#include <string>

#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;

extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern int** cvais;
extern cvAtt* cvs;

extern map<int, vector<int>> cvaisToFA; //fa�� cv array idex ���
extern wpinfo wpis;
extern flowControlCellAndData fccds;
extern vector<rainfallData> rfs;

extern thisProcess tp;

int setupModelAfterOpenProjectFile()
{
	if (setDomainAndCVBasicinfo() == -1) { return -1; }
	if (initWPinfos() == -1) { return - 1; }
	if (prj.simFlowControl == 1) {
		if (updateFCCellinfoAndData() == -1) { return -1; }
	}
    if (setupByFAandNetwork() == -1) { return -1; }
    if (updateCVbyUserSettings() == -1) { return -1; }
    if (prj.simType != simulationType::RealTime) {
        tp.rfDataCountInThisEvent = rfs.size();
    }
    else {
        tp.rfDataCountInThisEvent = -1;
    }
    tp.setupGRMisNormal = 1;
    tp.grmStarted = 1;
	return 1;
}

int setDomainAndCVBasicinfo()
{
	if (readDomainFileAndSetupCV() == -1) { return -1; }
	if (readSlopeFdirFacStreamCwCfSsrFileAndSetCV() == -1) { return -1; }
	if (prj.lcDataType == fileOrConstant::File) {
		if (readLandCoverFileAndSetCVbyVAT() == -1) { return -1; }
	}
	else if (prj.lcDataType == fileOrConstant::Constant) {
		if (setCVbyLCConstant() == -1) { return -1; }
	}
	if (prj.stDataType == fileOrConstant::File) {
		if (readSoilTextureFileAndSetCVbyVAT() == -1) { return -1; }
	}
	else if (prj.stDataType == fileOrConstant::Constant) {
		if (setCVbySTConstant() == -1) { return -1; }
	}
	if (prj.sdDataType == fileOrConstant::File) {
		if (readSoilDepthFileAndSetCVbyVAT() == -1) { return -1; }
	}
	else if (prj.sdDataType == fileOrConstant::Constant) {
		if (setCVbySDConstant() == -1) { return -1; }
	}
	if (setFlowNetwork() == -1) { return -1; }
	return 1;
}

int initWPinfos()
{
	int isnormal = -1;
	wpis.rfReadIntensitySumUpWS_mPs.clear();
	wpis.rfUpWSAveForDt_mm.clear();
	wpis.rfUpWSAveForDtPrintout_mm.clear();
	wpis.rfUpWSAveTotal_mm.clear();
	wpis.rfWPGridForDtPrintout_mm.clear();
	wpis.rfWPGridTotal_mm.clear();
	wpis.mTotalFlow_cms.clear();
	wpis.mTotalDepth_m.clear();
	wpis.maxFlow_cms.clear();
	wpis.maxDepth_m.clear();
	wpis.maxFlowTime.clear();
	wpis.maxDepthTime.clear();
	wpis.qFromFCData_cms.clear();
	wpis.qprint_cms.clear();
	wpis.FpnWpOut.clear();
	wpis.wpCVIDs.clear();

	for (int i = 0; i < prj.wps.size(); ++i) {
		int cx = prj.wps[i].wpColX;
		int ry = prj.wps[i].wpRowY;
		int cvid = cvais[cx][ry] + 1;
		wpis.wpCVIDs.push_back(cvid);
	}
	isnormal = 1;
	return isnormal;
}


int setupByFAandNetwork()
{
    di.facMostUpChannelCell = di.cellCountNotNull;//�켱 �ִ밪���� �ʱ�ȭ
    di.facMax = -1;
    di.facMin = INT_MAX;
    cvaisToFA.clear();
    for (int i = 0; i < di.cellCountNotNull; i++) {
        //cvs[i].fcType = flowControlType::None;
        double dxw;
        if (cvs[i].neighborCVIDsFlowIntoMe.size() > 0) {
            dxw = cvs[i].dxWSum / (double)cvs[i].neighborCVIDsFlowIntoMe.size();
        }
        else {
            dxw = cvs[i].dxDownHalf_m;
        }
        //cvs[i].cvdx_m = cvs[i].dxDownHalf_m + dxw; �̰� �������� �ʴ� ������ ����. ��� ���Է��� w ������ ������ ������ ���..2015.03.12
        cvs[i].cvdx_m = cvs[i].dxDownHalf_m * 2.0;
        if (cvs[i].fac > di.facMax) {
            di.facMax = cvs[i].fac;
            di.cvidxMaxFac = i;
        }
        if (cvs[i].fac < di.facMin) {
            di.facMin = cvs[i].fac;
        }
        // �ϵ� �Ű����� �ް�
        if (cvs[i].flowType == cellFlowType::ChannelFlow &&
            cvs[i].fac < di.facMostUpChannelCell) {
            di.facMostUpChannelCell = cvs[i].fac;
        }
        // FA�� cvid ����
        cvaisToFA[cvs[i].fac].push_back(i);

        //���� �Ϸ� wp ���� �ʱ�ȭ
        cvs[i].downWPCVIDs.clear();
    }

    // cross section ���� wsid ���� Ȯ��
    if (prj.css.size() > 0) {
        vector<int> ks;
        map<int, channelSettingInfo>::iterator iter;
        map<int, channelSettingInfo>::iterator iter_end;
        iter_end = prj.css.end();
        for (iter = prj.css.begin(); iter != iter_end; ++iter) {
            ks.push_back(iter->first);
        }
        for (int i : ks) {
            if (getVectorIndex(di.wsn.mdWSIDs, i) == -1) {
                //����� css Ű�� ���Ϸ� wsid ����Ʈ�� ���ٸ�,
                string outstr = "[" + to_string(i) + "] is not most downstream watershed ID.\n";
                writeLog(fpnLog, outstr, 1, 1);
                return -1;
            }
        }
    }

    // ���� �Ϸ� wp cvid ���� ������Ʈ
    vector<int> cvidsBase;
    vector<int> cvidsNew;
    for (int curCVid : wpis.wpCVIDs) {
        cvidsBase.push_back(curCVid);
        //���� ���� ��������� �Ѵ�.
        int aidx = curCVid - 1;// array index�� cvid-1
        cvs[aidx].downWPCVIDs.push_back(curCVid);
        bool ended = false;
        while (ended != true) {
            ended = true;
            for (int cvidBase : cvidsBase) {
                aidx = cvidBase - 1;
                if (cvs[aidx].neighborCVIDsFlowIntoMe.size() > 0) {
                    ended = false;
                    for (int cvid : cvs[aidx].neighborCVIDsFlowIntoMe) {
                        cvs[cvid - 1].downWPCVIDs.push_back(curCVid);
                        cvidsNew.push_back(cvid);
                    }
                }
            }
            cvidsBase = cvidsNew;
        }
    }
    return 1;
}


int updateCVbyUserSettings()
{
    for (int i = 0; i < di.cellCountNotNull; ++i) {
        int wid = cvs[i].wsid;
        swsParameters ups = prj.swps[wid];
        if (cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) {
            cvs[i].flowType = cellFlowType::ChannelFlow; //�켱 channel flow�� �ʱ�ȭ
        }
        // ��ǥ�� ���
        if (cvs[i].slope < ups.minSlopeOF) {
            cvs[i].slopeOF = ups.minSlopeOF;
        }
        else {
            cvs[i].slopeOF = cvs[i].slope;
        }
        cvs[i].rcOF = cvs[i].rcOFori * ups.ccLCRoughness;

        // ��õ
        if (prj.streamFileApplied == 1
            && cvs[i].flowType == cellFlowType::ChannelFlow) {
            int mdwsid = di.wsn.mdWSIDofCurrentWS[wid];
            cvs[i].stream.chRC = ups.chRoughness;
            cvs[i].stream.chSideSlopeLeft = prj.css[mdwsid].bankSlopeLeft;
            cvs[i].stream.chSideSlopeRight = prj.css[mdwsid].bankSlopeRight;
            cvs[i].stream.chBankCoeff = 1 / prj.css[mdwsid].bankSlopeLeft
                + 1 / prj.css[mdwsid].bankSlopeRight;
            if (cvs[i].slope < ups.minSlopeChBed) {
                cvs[i].stream.chBedSlope = ups.minSlopeChBed;
            }
            else {
                cvs[i].stream.chBedSlope = cvs[i].slope;
            }
            if (prj.css[mdwsid].csType == crossSectionType::CSSingle) {//Single CS������ �� ���� ����� �̿��ؼ� ������ ���
                channelSettingInfo cs = prj.css[mdwsid];
                if (cs.csWidthType == channelWidthType::CWEquation) {
                    double cellarea = di.cellSize * di.cellSize / 1000000.0;
                    double area = (cvs[i].fac + 1.0) * cellarea;
                    cvs[i].stream.chBaseWidth = cs.cwEQc * pow(area, cs.cwEQd)
                        / pow(cvs[i].stream.chBedSlope, cs.cwEQe);
                }
                else {
                    int facMax_inMDWS = cvs[di.wsn.wsOutletCVID[mdwsid] - 1].fac;
                    cvs[i].stream.chBaseWidth = cvs[i].fac
                        * cs.cwMostDownStream / (double)facMax_inMDWS;
                }
                if (prj.cwFileApplied == 1 && cvs[i].stream.chBaseWidthByLayer > 0) {
                    cvs[i].stream.chBaseWidth = cvs[i].stream.chBaseWidthByLayer;
                }
                cvs[i].stream.chHighRBaseWidth_m = 0;
                cvs[i].stream.isCompoundCS = -1;
                cvs[i].stream.chLowRArea_m2 = 0;
            }
            else if (prj.css[mdwsid].csType == crossSectionType::CSCompound) {// Compound CS������ ����ڰ� �Է��� ����� �̿��ؼ� ���� ���
                channelSettingInfo cs = prj.css[mdwsid];
                int facMax_inMDWS = cvs[di.wsn.wsOutletCVID[mdwsid] - 1].fac;
                cvs[i].stream.chBaseWidth = cvs[i].fac
                    * cs.lowRBaseWidth / (double)facMax_inMDWS;
                if (cvs[i].stream.chBaseWidth < cs.compoundCSChannelWidthLimit) {
                    cvs[i].stream.isCompoundCS = false;
                    cvs[i].stream.chHighRBaseWidth_m = 0;
                    cvs[i].stream.chLowRHeight = 0;
                    cvs[i].stream.chLowRArea_m2 = 0;
                }
                else {
                    cvs[i].stream.isCompoundCS = true;
                    cvs[i].stream.chHighRBaseWidth_m = cvs[i].fac * cs.highRBaseWidth / (double)facMax_inMDWS;
                    cvs[i].stream.chLowRHeight = cvs[i].fac * cs.lowRHeight / (double)facMax_inMDWS;
                    cvs[i].stream.chLowRArea_m2 = getChCSAbyFlowDepth(cvs[i].stream.chBaseWidth,
                        cvs[i].stream.chBankCoeff, cvs[i].stream.chLowRHeight,
                        false, cvs[i].stream.chLowRHeight, cvs[i].stream.chLowRArea_m2, 0);
                }
            }
            else {
                writeLog(fpnLog, "Cross section type is invalid. \n", 1, 1);
                return -1;
            }
            // �ּ� ����
            if (cvs[i].stream.chBaseWidth < ups.minChBaseWidth) {
                cvs[i].stream.chBaseWidth = ups.minChBaseWidth;
            }
            if (cvs[i].stream.chBaseWidth < di.cellSize) {
                cvs[i].flowType = cellFlowType::ChannelNOverlandFlow;
            }
        }

        // ���
        if (prj.issrFileApplied == -1) {
            cvs[i].iniSSR = ups.iniSaturation;
        }
        if (cvs[i].flowType == cellFlowType::ChannelFlow
            || cvs[i].lcCode == landCoverCode::WATR
            || cvs[i].lcCode == landCoverCode::WTLD) {
            cvs[i].ssr = 1.0;
        }
        else { cvs[i].ssr = cvs[i].iniSSR; }

        cvs[i].ukType = unSaturatedKType::Linear;
        if (ups.unSatKType == unSaturatedKType::Linear) {
            cvs[i].ukType = unSaturatedKType::Linear;
        }
        if (ups.unSatKType == unSaturatedKType::Exponential) {
            cvs[i].ukType = unSaturatedKType::Exponential;
        }
        if (ups.unSatKType == unSaturatedKType::Constant) {
            cvs[i].ukType = unSaturatedKType::Constant;
        }

        cvs[i].coefUK = ups.coefUnsaturatedK;
        cvs[i].porosity_Eta = cvs[i].porosity_EtaOri * ups.ccPorosity;
        if (cvs[i].porosity_Eta >= 1) { cvs[i].porosity_Eta = 0.99; }
        if (cvs[i].porosity_Eta <= 0) { cvs[i].porosity_Eta = 0.01; }
        cvs[i].effPorosity_ThetaE = cvs[i].effPorosity_ThetaEori * ups.ccPorosity;   // ��ȿ �������� ������ ���ط� ��������� �Բ� ����Ѵ�.
        if (cvs[i].effPorosity_ThetaE >= 1) { cvs[i].effPorosity_ThetaE = 0.99; }
        if (cvs[i].effPorosity_ThetaE <= 0) { cvs[i].effPorosity_ThetaE = 0.01; }
        cvs[i].wfsh_Psi_m = cvs[i].wfsh_PsiOri_m * ups.ccWFSuctionHead;
        cvs[i].hydraulicC_K_mPsec = cvs[i].HydraulicC_Kori_mPsec * ups.ccHydraulicK;
        cvs[i].sd_m = cvs[i].sdOri_m * ups.ccSoilDepth;
        cvs[i].sdEffAsWaterDepth_m = cvs[i].sd_m * cvs[i].effPorosity_ThetaE;
        cvs[i].soilWaterContent_m = cvs[i].sdEffAsWaterDepth_m * cvs[i].ssr;
        cvs[i].soilWaterContent_tm1_m = cvs[i].soilWaterContent_m;
        cvs[i].sdToBedrock_m = CONST_DEPTH_TO_BEDROCK; // �Ϲݱ����� ���̸� 20m�� ����, ������������� 5m
        if (cvs[i].lcCode == landCoverCode::FRST) {
            cvs[i].sdToBedrock_m = CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN;
        }
    }

    // Flow control
    if (prj.simFlowControl == 1 && prj.fcs.size() > 0) {
        for (int cvid : fccds.cvidsFCcell) {
            flowControlinfo afc = prj.fcs[cvid];
            switch (afc.fcType) {
            case flowControlType::Inlet: {
                cvs[cvid - 1].fcType = flowControlType::Inlet;
                break;
            }
            case flowControlType::ReservoirOperation: {
                cvs[cvid - 1].fcType = flowControlType::ReservoirOperation;
                break;
            }
            case flowControlType::ReservoirOutflow: {
                cvs[cvid - 1].fcType = flowControlType::ReservoirOutflow;
                break;
            }
            case flowControlType::SinkFlow: {
                cvs[cvid - 1].fcType = flowControlType::SinkFlow;
                break;
            }
            case flowControlType::SourceFlow: {
                cvs[cvid - 1].fcType = flowControlType::SourceFlow;
                break;
            }
            }
        }
    }

    // Inlet �� �����  toBeSimulated =-1 ���� ����
    if (prj.simFlowControl == 1 && prj.isinletExist == 1) {
        bool bEnded = false;
        vector<int> baseCVids;
        vector<int> newCVids;
        baseCVids = fccds.cvidsinlet;
        while (!bEnded == true) {
            bEnded = true;
            for (int cvidBase : baseCVids) {
                int cvan = cvidBase - 1;
                if (cvs[cvan].neighborCVIDsFlowIntoMe.size() > 0) {
                    bEnded = false;
                    for (int cvidFtoM : cvs[cvan].neighborCVIDsFlowIntoMe) {
                        cvs[cvidFtoM - 1].toBeSimulated = -1;
                        newCVids.push_back(cvidFtoM);
                    }
                }
            }
            baseCVids.clear();
            baseCVids = newCVids;
        }
    }
    return 1;
}

