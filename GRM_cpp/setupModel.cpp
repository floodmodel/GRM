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

int setupModelAfterOpenProjectFile()
{
	if (setDomainAndCVBasicinfo() == -1) { return -1; }
	if (initWPinfos() == -1) { return - 1; }
	if (prj.simFlowControl == 1) {
		if (initFCCellinfoAndData() == -1) { return -1; }
	}
    if (setupByFAandNetwork() == -1) { return -1; }
    updateCVbyUserSettings();



	//cProject.Current.UpdateDownstreamWPforAllCVs(); //�̰� setupByFAandNetwork���� ���� �̵�
	//cGRM.Start();
	//sThisSimulation.mGRMSetupIsNormal = true;
	//if (mProject.mSimulationType != cGRM.SimulationType.RealTime)
	//{
	//	sThisSimulation.mRFDataCountInThisEvent = mProject.rainfall.mlstRainfallData.Count;
	//}
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
    //for (int ir = 0; ir < di.nRows; ++ir)    {
//    for (int ic = 0; ic < di.nCols; ++ic)        {
    for (int i = 0; i < di.cellCountNotNull; ++i) {
        //if (WSCells[ic, ir] == null) { continue; }
        //cCVAttribute cell = WSCells[ic, ir];
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
                    cvs[i].stream.chLowRArea_m2 = mFVMSolver.GetChannelCrossSectionAreaUsingChannelFlowDepth
                    (cvs[i].stream.chBaseWidth, cvs[i].stream.chBankCoeff, cvs[i].stream.chLowRHeight,
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

        //// ���
        //if (watershed.mFPN_initialSoilSaturationRatio == "" || File.Exists(watershed.mFPN_initialSoilSaturationRatio) == false)
        //{
        //    cvs[i].InitialSaturation = ups.iniSaturation;
        //}
        //else
        //{
        //}
        //if (cvs[i].FlowType == cGRM.CellFlowType.ChannelFlow
        //    || cvs[i].LandCoverCode == cSetLandcover.LandCoverCode.WATR
        //    || cvs[i].LandCoverCode == cSetLandcover.LandCoverCode.WTLD)
        //{
        //    cvs[i].soilSaturationRatio = 1;
        //}
        //else { cvs[i].soilSaturationRatio = cvs[i].InitialSaturation; }

        //cvs[i].UKType = cGRM.UnSaturatedKType.Linear;
        //if (ups.UKType.ToLower() == cGRM.UnSaturatedKType.Linear.ToString().ToLower())
        //{
        //    cvs[i].UKType = cGRM.UnSaturatedKType.Linear;
        //}
        //if (ups.UKType.ToLower() == cGRM.UnSaturatedKType.Exponential.ToString().ToLower())
        //{
        //    cvs[i].UKType = cGRM.UnSaturatedKType.Exponential;
        //}
        //if (ups.UKType.ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower())
        //{
        //    cvs[i].UKType = cGRM.UnSaturatedKType.Constant;
        //}

        //cvs[i].coefUK = ups.coefUK;
        //cvs[i].porosityEta = cvs[i].PorosityEtaOri * ups.ccPorosity;
        //if (cvs[i].porosityEta >= 1) { cvs[i].porosityEta = 0.99; }
        //if (cvs[i].porosityEta <= 0) { cvs[i].porosityEta = 0.01; }
        //cvs[i].effectivePorosityThetaE = cvs[i].EffectivePorosityThetaEori * ups.ccPorosity;   // ��ȿ �������� ������ ���ط� ��������� �Բ� ����Ѵ�.
        //if (cvs[i].effectivePorosityThetaE >= 1) { cvs[i].effectivePorosityThetaE = 0.99; }
        //if (cvs[i].effectivePorosityThetaE <= 0) { cvs[i].effectivePorosityThetaE = 0.01; }
        //cvs[i].wettingFrontSuctionHeadPsi_m = cvs[i].WettingFrontSuctionHeadPsiOri_m * ups.ccWFSuctionHead;
        //cvs[i].hydraulicConductK_mPsec = cvs[i].HydraulicConductKori_mPsec * ups.ccHydraulicK;
        //cvs[i].soilDepth_m = cvs[i].SoilDepthOri_m * ups.ccSoilDepth;
        //cvs[i].SoilDepthEffectiveAsWaterDepth_m = cvs[i].soilDepth_m * cvs[i].effectivePorosityThetaE;
        //cvs[i].soilWaterContent_m = cvs[i].SoilDepthEffectiveAsWaterDepth_m * cvs[i].soilSaturationRatio;
        //cvs[i].soilWaterContent_tM1_m = cvs[i].soilWaterContent_m;

        //cvs[i].SoilDepthToBedrock_m = cGRM.CONST_DEPTH_TO_BEDROCK; // �Ϲݱ����� ���̸� 20m�� ����, ������������� 5m
        //if (cvs[i].LandCoverCode == cSetLandcover.LandCoverCode.FRST)
        //{
        //    cvs[i].SoilDepthToBedrock_m = cGRM.CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN;
        //}
    //}
    }

    // Flow control
    if (prj.simFlowControl == 1 && prj.fcs.size() > 0) {
        //foreach(int cvid in fcGrid.FCGridCVidList)
        //{
        //    DataRow[] rows = fcGrid.mdtFCGridInfo.Select("CVID = " + cvid);
        //    Dataset.GRMProject.FlowControlGridRow row;
        //    row = (Dataset.GRMProject.FlowControlGridRow)rows[0];
        //    switch (row.ControlType)
        //    {
        //    case nameof(cFlowControl.FlowControlType.Inlet):
        //    {
        //        CVs[cvid - 1].FCType = cFlowControl.FlowControlType.Inlet;
        //        break;
        //    }

        //    case nameof(cFlowControl.FlowControlType.ReservoirOperation):
        //    {
        //        CVs[cvid - 1].FCType = cFlowControl.FlowControlType.ReservoirOperation;
        //        break;
        //    }

        //    case nameof(cFlowControl.FlowControlType.ReservoirOutflow):
        //    {
        //        CVs[cvid - 1].FCType = cFlowControl.FlowControlType.ReservoirOutflow;
        //        break;
        //    }

        //    case nameof(cFlowControl.FlowControlType.SinkFlow):
        //    {
        //        CVs[cvid - 1].FCType = cFlowControl.FlowControlType.SinkFlow;
        //        break;
        //    }

        //    case nameof(cFlowControl.FlowControlType.SourceFlow):
        //    {
        //        CVs[cvid - 1].FCType = cFlowControl.FlowControlType.SourceFlow;
        //        break;
        //    }

        //    default:
        //    {
        //        throw new InvalidDataException();
        //    }
        //    }
        //}
    }

    // Inlet
    if (prj.simFlowControl == 1 && prj.isinletExisted == 1) {
        //bool bEnded = false;
        //List<int> lBaseCVid;
        //List<int> lNewCVid;
        //lBaseCVid = new List<int>();
        //lBaseCVid = fcGrid.InletCVidList;
        //while (!bEnded == true)
        //{
        //    lNewCVid = new List<int>();
        //    bEnded = true;
        //    foreach(int cvidBase in lBaseCVid)
        //    {
        //        int cvan = cvidBase - 1;
        //        if (CVs[cvan].NeighborCVidFlowIntoMe.Count > 0)
        //        {
        //            bEnded = false;
        //            foreach(int cvidFlowIntoMe in CVs[cvan].NeighborCVidFlowIntoMe)
        //            {
        //                CVs[cvidFlowIntoMe - 1].toBeSimulated = -1;
        //                lNewCVid.Add(cvidFlowIntoMe);
        //            }
        //        }
        //    }
        //    lBaseCVid = new List<int>();
        //    lBaseCVid = lNewCVid;
        //}
    }
    return 1;
}