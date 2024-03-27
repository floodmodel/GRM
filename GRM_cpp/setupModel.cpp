#include "stdafx.h"
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;

extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern int** cvais;
extern cvAtt* cvs;
extern cvpos* cvps;

extern map<int, int*> cvaisToFA; //fa별 cv array idex 목록
extern vector<int> fas;
extern map<int, int> faCount;
extern wpSimData wpSimValue;
extern flowControlCellAndData fccds;

int setupModelAfterOpenProjectFile()
{
	if (setDomainAndCVBasicinfo() == -1) { return -1; }
	if (initWPinfos() == -1) { return - 1; }
	if (prj.simFlowControl == 1) {
		if (updateFCCellinfoAndData() == -1) { return -1; }
	}
    if (setupByFAandNetwork() == -1) { return -1; }
    if (updateCVbyUserSettings() == -1) { return -1; }
	return 1;
}


int setDomainAndCVBasicinfo()
{
	if (readDomainFaFileAndSetupCV() == -1) { return -1; }
	if (checkWPpositions() == -1) { return -1; }
	if (checkWeatherDataByDomain() == -1) { return -1; }
	if (readSlopeFdirStreamCwCfSsrFileAndSetCV() == -1) { return -1; }
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

int checkWPpositions() {
	for (int i = 0; i < prj.wps.size(); ++i) {
		int cx = prj.wps[i].wpColX;
		int ry = prj.wps[i].wpRowY;
		if (cx<0 || cx>di.nCols - 1) {
			writeLog(fpnLog, "ERROR : The column index of the watch point ["
				+ prj.wps[i].wpName + "] is out of range.\n", 1, 1);
			return -1;
		}
		if (ry<0 || ry>di.nRows - 1) {
			writeLog(fpnLog, "ERROR : The row index of the watch point ["
				+ prj.wps[i].wpName + "] is out of range.\n", 1, 1);
			return -1;
		}
		if (cvais[cx][ry] == -1) {
			writeLog(fpnLog, "ERROR : The watch point ["
				+ prj.wps[i].wpName + "] is out of valid simulation region.\n", 1, 1);
			return -1;
		}
	}
}
int checkWeatherDataByDomain() {
	if (compareWeatherDataWithDomain(prj.fpnRainfallData, prj.rfDataType, "Precipitation") == -1) {
		return -1;
	}
	// continuous =================
	if (prj.simEvaportranspiration == 1) {
		if (compareWeatherDataWithDomain(prj.fpnTempMaxData, prj.tempMaxDataType, "Maximum temperature") == -1) {
			return -1;
		}
		if (compareWeatherDataWithDomain(prj.fpnTempMinData, prj.tempMinDataType, "Minimum temperature") == -1) {
			return -1;
		}
		if (compareWeatherDataWithDomain(prj.fpnDaytimeLengthData, prj.daytimeLengthDataType, "Daytime length") == -1) {
			return -1;
		}
		if (compareWeatherDataWithDomain(prj.fpnSolarRadData, prj.solarRadDataType, "Solar radiation") == -1) {
			return -1;
		}
	}
	if (prj.simSnowMelt == 1) {
		if (compareWeatherDataWithDomain(prj.fpnTempMaxData, prj.tempMaxDataType, "Maximum temperature") == -1) {
			return -1;
		}
		if (compareWeatherDataWithDomain(prj.fpnSnowpackTempData, prj.snowpackTempDataType, "Snowpack temperature") == -1) {
			return -1;
		}
	}
	// continuous =================
}


int compareWeatherDataWithDomain(string fpn_in_wd, weatherDataType wdType,
	string dataString) {

	if (wdType == weatherDataType::Mean_DividedArea) {
		vector<int> wsids;
		ifstream txtFile(fpn_in_wd);
		string aline;
		getline(txtFile, aline); // 첫줄만 읽는다
		aline = trim(aline);
		wsids = setWSIDsInWeatherDataFile(aline);
		int idCount = wsids.size();
		if (di.dmids.size() != idCount) {// 여기서는 id 개수 확인
			string err = "ERROR : The number of " + dataString
				+ " data column is not equal to the number of sub-domain ids.\n"
				+ "  "+ dataString +" data file name : " + fpn_in_wd + "\n";
			writeLog(fpnLog, err, 1, 1);
			return -1;
		}

		for (int i = 0; i < idCount; i++) { // 여기서는 id 있는지 확인
			if (std::find(di.dmids.begin(), di.dmids.end(), wsids[i]) == di.dmids.end()) {
				string err = "ERROR : Domain id [" + to_string(wsids[i])
					+ "] from "+ dataString +" data file was not found in sub-domain ids.\n"
					+ "  " + dataString + " data file name : " + fpn_in_wd + "\n";
				writeLog(fpnLog, err, 1, 1);
				return -1;
			}
		}		
	}
	return 1;
}


int initWPinfos()
{
	wpSimValue.prcpiReadSumUpWS_mPs.clear();
	wpSimValue.prcpUpWSAveForDt_mm.clear();
	wpSimValue.prcpUpWSAveForPT_mm.clear();
	wpSimValue.prcpUpWSAveTotal_mm.clear();
	wpSimValue.prcpWPGridForPT_mm.clear();
	wpSimValue.prcpWPGridTotal_mm.clear();
	wpSimValue.Q_sumPTforAVE_m3.clear();
	wpSimValue.q_cms_print.clear();
	wpSimValue.wpCVidxes.clear();

	for (int i = 0; i < prj.wps.size(); ++i) {
		int cx = prj.wps[i].wpColX;
		int ry = prj.wps[i].wpRowY;
		int idx = cvais[cx][ry];
		wpSimValue.wpCVidxes.push_back(idx);
		wpSimValue.wpNames[idx] = prj.wps[i].wpName;
	}
	return 1;
}

int setupByFAandNetwork()
{
    di.facMostUpChannelCell = di.cellNnotNull;//우선 최대값으로 초기화
    cvaisToFA.clear();
    fas.clear();
    faCount.clear();
    map<int, vector<int>> aisFA;
    for (int i = 0; i < di.cellNnotNull; i++) {
        double dxw;
        int curfa = cvs[i].fac;
        if (cvs[i].neighborCVidxFlowintoMe.size() > 0) {
            dxw = cvs[i].dxWSum / (double)cvs[i].neighborCVidxFlowintoMe.size();
        }
        else {
            dxw = cvs[i].dxDownHalf_m;
        }
        //cvs[i].cvdx_m = cvs[i].dxDownHalf_m + dxw; 이것 적용하지 않는 것으로 수정. 상류 유입량이 w 끝으로 들어오는 것으로 계산..2015.03.12
        cvs[i].cvdx_m = cvs[i].dxDownHalf_m * 2.0;
        // 하도 매개변수 받고
        if (cvs[i].flowType == cellFlowType::ChannelFlow &&
            curfa < di.facMostUpChannelCell) {
            di.facMostUpChannelCell = curfa;
        }
        // FA별 idx 저장
        aisFA[curfa].push_back(i);
        if (getVectorIndex(fas, curfa) == -1) {
            fas.push_back(curfa);
        }
        //셀별 하류 wp 정보 초기화
        cvs[i].downWPCVidx.clear();
    }
    sort(fas.begin(), fas.end());
    for (int i = 0; i<fas.size(); ++i) {
        int curFA = fas[i];
        vector<int> av = aisFA[curFA];
        cvaisToFA[curFA] = new int[av.size()];
        copy(av.begin(), av.end(), cvaisToFA[curFA]);
        faCount[curFA] = av.size();
    }
	
    // cross section 정보 wsid 오류 확인
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
                //저장된 css 키가 최하류 wsid 리스트에 없다면,
                string outstr = "ERROR : [" + to_string(i) + "] is not the most downstream watershed ID (it has the downstream watershedID "
					+ to_string(di.wsn.wsidNearbyDown[i])+").\n";
                writeLog(fpnLog, outstr, 1, 1);
                return -1;
            }
        }
    }

    // 셀별 하류 wp idx 정보 업데이트
    vector<int> idxesBase;
    vector<int> idxesNew;
    for (int curidx : wpSimValue.wpCVidxes) {
        idxesBase.push_back(curidx);
        //현재 셀을 출발점으로 한다.
        int aidx = curidx ;
        cvs[aidx].downWPCVidx.push_back(curidx);
        bool ended = false;
        while (ended != true) {
            idxesNew.clear();
            ended = true;
            for (int cvidBase : idxesBase) {
                aidx = cvidBase ;
                if (cvs[aidx].neighborCVidxFlowintoMe.size() > 0) {
                    ended = false;
                    for (int idx : cvs[aidx].neighborCVidxFlowintoMe) {
                        cvs[idx].downWPCVidx.push_back(curidx);
                        idxesNew.push_back(idx);
                    }
                }
            }
            idxesBase = idxesNew;
        }
    }
    return 1;
}


int updateCVbyUserSettings()
{
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int wid = cvps[i].wsid;
        swsParameters ups = prj.swps[wid];
        if (cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) {
            cvs[i].flowType = cellFlowType::ChannelFlow; //우선 channel flow로 초기화
        }
        // 지표면 경사
        if (cvs[i].slope < ups.minSlopeOF) {
            cvs[i].slopeOF = ups.minSlopeOF;
        }
        else {
            cvs[i].slopeOF = cvs[i].slope;
        }
        cvs[i].rcOF = cvs[i].rcOFori * ups.ccLCRoughness;
        // 하천
        if (prj.streamFileApplied == 1
            && cvs[i].flowType == cellFlowType::ChannelFlow) {
            int mdwsid = di.wsn.mdWSIDofCurrentWS[wid];
            cvs[i].stream.chRC = ups.chRoughness;
            cvs[i].stream.chSideSlopeLeft = prj.css[mdwsid].bankSlopeLeft;
            cvs[i].stream.chSideSlopeRight = prj.css[mdwsid].bankSlopeRight;
            cvs[i].stream.bankCoeff = 1 / prj.css[mdwsid].bankSlopeLeft
                + 1 / prj.css[mdwsid].bankSlopeRight;
            if (cvs[i].slope < ups.minSlopeChBed) {
                cvs[i].stream.slopeCH = ups.minSlopeChBed;
            }
            else {
                cvs[i].stream.slopeCH = cvs[i].slope;
            }
            if (prj.css[mdwsid].csType == crossSectionType::CSSingle) {//Single CS에서는 두 가지 방법을 이용해서 하폭을 계산
                channelSettingInfo cs = prj.css[mdwsid];
                if (cs.csWidthType == channelWidthType::CWEquation) {
                    double cellarea = di.cellSize * di.cellSize / 1000000.0;
                    double area = (cvs[i].fac + 1.0) * cellarea;
                    cvs[i].stream.chBaseWidth = cs.cwEQc * pow(area, cs.cwEQd)
                        / pow(cvs[i].stream.slopeCH, cs.cwEQe);
                }
                else {
                    int facMax_inMDWS = cvs[di.wsn.wsOutletidxs[mdwsid]].fac;
                    cvs[i].stream.chBaseWidth = cvs[i].fac
                        * cs.cwMostDownStream / (double)facMax_inMDWS;
                }
                if (prj.cwFileApplied == 1 && cvs[i].stream.chBaseWidthByLayer > 0) {
                    cvs[i].stream.chBaseWidth = cvs[i].stream.chBaseWidthByLayer;
                }
                cvs[i].stream.chURBaseWidth_m = 0;
                cvs[i].stream.isCompoundCS = -1;
                cvs[i].stream.chLRArea_m2 = 0;
            }
            else if (prj.css[mdwsid].csType == crossSectionType::CSCompound) {// Compound CS에서는 사용자가 입력한 재원을 이용해서 하폭 계산
                channelSettingInfo cs = prj.css[mdwsid];
                int facMax_inMDWS = cvs[di.wsn.wsOutletidxs[mdwsid]].fac;
                cvs[i].stream.chBaseWidth = cvs[i].fac
                    * cs.lowRBaseWidth / (double)facMax_inMDWS;
                if (cvs[i].stream.chBaseWidth < cs.compoundCSChannelWidthLimit) {
                    cvs[i].stream.isCompoundCS = -1;
                    cvs[i].stream.chURBaseWidth_m = 0;
                    cvs[i].stream.chLRHeight = 0;
                    cvs[i].stream.chLRArea_m2 = 0;
                }
                else {
                    cvs[i].stream.isCompoundCS = 1;
                    cvs[i].stream.chURBaseWidth_m = cvs[i].fac * cs.highRBaseWidth / (double)facMax_inMDWS;
                    cvs[i].stream.chLRHeight = cvs[i].fac * cs.lowRHeight / (double)facMax_inMDWS;
                    cvs[i].stream.chLRArea_m2 = getChCSAbyFlowDepth(cvs[i].stream.chBaseWidth,
                        cvs[i].stream.bankCoeff, cvs[i].stream.chLRHeight,
                        -1, cvs[i].stream.chLRHeight, cvs[i].stream.chLRArea_m2, 0);//처음에는 단단면으로 출발
                }
            }
            else {
				string cstype = "";
				if (prj.css[mdwsid].csType == crossSectionType::CSCompound) {
				cstype = "CSCompound";
				}
				else if (prj.css[mdwsid].csType == crossSectionType::CSSingle) {
					cstype = "CSSingle";
				}
				else if (prj.css[mdwsid].csType == crossSectionType::None) {
					cstype = "None";
				}
				else {
					cstype = "Unknown";
				}
                writeLog(fpnLog, "ERROR : Cross section type("+ cstype +") is invalid. Current watershed ID = "+ to_string(wid)
					+ ", most downstream ID = "+to_string(mdwsid)+" \n", 1, 1);
                return -1;
            }
            // 최소 하폭
            if (cvs[i].stream.chBaseWidth < ups.minChBaseWidth) {
                cvs[i].stream.chBaseWidth = ups.minChBaseWidth;
            }
            if (cvs[i].stream.chBaseWidth < di.cellSize) {
                cvs[i].flowType = cellFlowType::ChannelNOverlandFlow;
            }
        }

        // 토양
        if (prj.issrFileApplied == -1) {
            cvs[i].iniSSR = ups.iniSaturation;
        }

        if (cvs[i].flowType == cellFlowType::ChannelFlow
            || cvs[i].lcCode == landCoverCode::WATR
            || cvs[i].lcCode == landCoverCode::WTLD) {
            cvs[i].ssr = 1.0;
        }
        else { 
			cvs[i].ssr = cvs[i].iniSSR; 
		}

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
        cvs[i].effPorosity_ThetaE = cvs[i].effPorosity_ThetaEori * ups.ccPorosity;   // 유효 공극율의 보정은 공극률 보정계수를 함께 사용한다.
        if (cvs[i].effPorosity_ThetaE >= 1) { cvs[i].effPorosity_ThetaE = 0.99; }
        if (cvs[i].effPorosity_ThetaE <= 0) { cvs[i].effPorosity_ThetaE = 0.01; }
        cvs[i].wfsh_Psi_m = cvs[i].wfsh_PsiOri_m * ups.ccWFSuctionHead;
        cvs[i].hc_K_mPsec = cvs[i].hc_Kori_mPsec * ups.ccHydraulicK;
        cvs[i].sd_m = cvs[i].sdOri_m * ups.ccSoilDepth;
        cvs[i].sdEffAsWaterDepth_m = cvs[i].sd_m * cvs[i].effPorosity_ThetaE;
        cvs[i].soilWaterC_m = cvs[i].sdEffAsWaterDepth_m * cvs[i].ssr;
        cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_m;
        cvs[i].sdToBedrock_m = CONST_DEPTH_TO_BEDROCK; // 암반까지의 깊이를 20m로 가정, 산악지역에서는 5m
        if (cvs[i].lcCode == landCoverCode::FRST) {
            cvs[i].sdToBedrock_m = CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN;
        }
		
		// 차단
		cvs[i].intcpMethod= ups.interceptMethod;

		// 증발산
		cvs[i].petMethod = ups.potentialETMethod;
		cvs[i].etCoef = ups.etCoeff;
		
		// 융설
		cvs[i].smMethod = ups.snowMeltMethod;
		cvs[i].smeltTSR =ups.tempSnowRain;
		cvs[i].smeltingT =ups.smeltingTemp;
		cvs[i].snowCovR = ups.snowCovRatio;
		cvs[i].smeltCoef = ups.smeltCoef;
    }

    // Flow control
    if (prj.simFlowControl == 1 && prj.fcs.size() > 0) {
        for (int idx : fccds.cvidxsFCcell) {
			for (int i = 0; i<prj.fcs[idx].size(); ++i) {
				flowControlinfo afc = prj.fcs[idx][i];
				switch (afc.fcType) {
				case flowControlType::Inlet: {
					cvs[idx].fcType1 = flowControlType::Inlet;
					break;
				}
				case flowControlType::ReservoirOperation: {
					cvs[idx].fcType1 = flowControlType::ReservoirOperation;
					break;
				}
				case flowControlType::ReservoirOutflow: {
						cvs[idx].fcType1 = flowControlType::ReservoirOutflow;
						break;
				}
				case flowControlType::DetensionPond: {
					cvs[idx].fcType1 = flowControlType::DetensionPond;
					break;
				}
				case flowControlType::SinkFlow: {// Opne project에서 같은 속성은 없는게 보장된다. 
					if (cvs[idx].fcType1 == flowControlType::None) {
						cvs[idx].fcType1 = flowControlType::SinkFlow;
						break;
					}
					else if (cvs[idx].fcType2 == flowControlType::None) { 
						cvs[idx].fcType2 = flowControlType::SinkFlow;
						break;
					}
					if (cvs[idx].fcType3 == flowControlType::None) {
						cvs[idx].fcType3 = flowControlType::SinkFlow;
					}
					break;
				}
				case flowControlType::SourceFlow: {// Opne project에서 같은 속성은 없는게 보장된다. 
					if (cvs[idx].fcType1 == flowControlType::None) {
						cvs[idx].fcType1 = flowControlType::SourceFlow;
						break;
					}
					else if (cvs[idx].fcType2 == flowControlType::None) {
						cvs[idx].fcType2 = flowControlType::SourceFlow;
						break;
					}
					if (cvs[idx].fcType3 == flowControlType::None) {
						cvs[idx].fcType3 = flowControlType::SourceFlow;
					}
					break;
				}
				}
			}
        }
    }

    // Inlet 셀 상류는  toBeSimulated =-1 으로 설정
    if (prj.simFlowControl == 1 && prj.isinletExist == 1) {
        bool bEnded = false;
        vector<int> baseCVidxs;
        vector<int> newCVids;
        baseCVidxs = fccds.cvidxsinlet;
        while (!bEnded == true) {
            newCVids.clear();
            bEnded = true;
            for (int cvan : baseCVidxs) {
                //int cvan = cvidBase - 1;
                if (cvs[cvan].neighborCVidxFlowintoMe.size() > 0) {
                    bEnded = false;
                    for (int idFtoM : cvs[cvan].neighborCVidxFlowintoMe) {
                        cvs[idFtoM].toBeSimulated = -1;
                        newCVids.push_back(idFtoM);
                    }
                }
            }
            baseCVidxs.clear();
            baseCVidxs = newCVids;
        }
    }

    // wp 별로, 상류에 있는 cv 개수 설정
	di.cellNtobeSimulated = 0;
	for (int idx : wpSimValue.wpCVidxes) {
		wpSimValue.cvCountAllup[idx] =0;
	}
    for (int i = 0; i < di.cellNnotNull; i++) {
        // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
        //if (cvs[i].toBeSimulated == -1) { continue; }
        di.cellNtobeSimulated++;
        for (int idx : cvs[i].downWPCVidx) {
            wpSimValue.cvCountAllup[idx] ++;
        }
    }
    return 1;
}


