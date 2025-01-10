#include "stdafx.h"
#include "grm.h"
using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern int** cvais;
extern cvAtt* cvs;
extern domaininfo di;
extern vector<flowControlinfo> fcinfos;

extern flowControlCellAndData fccds;
extern thisSimulation ts;

int updateFCCellinfoAndData()
{
    fccds.cvidxsinlet.clear();
    fccds.cvidxsFCcell.clear();
    fccds.inputFlowDataFCType1_m3Ps.clear();
	fccds.inputFlowDataFCType2_m3Ps.clear();
	fccds.inputFlowDataFCType3_m3Ps.clear();
    fccds.curDorder.clear();
	fccds.inflowSumPT_m3.clear();
    prj.fcs.clear();
	
	for (int i = 0; i < fcinfos.size(); ++i) {  // 여기fcinfos에서는 일련번호가 key
		flowControlinfo afc = fcinfos[i];
		int aidx = cvais[afc.fcColX][afc.fcRowY];
		if (prj.fcs.count(aidx) < 1) {
			prj.fcs[aidx].push_back(afc); //여기서 cvidx를 키로 설정
			fccds.cvidxsFCcell.push_back(aidx);
			fccds.curDorder[aidx] = 0;
			if (afc.fcType == flowControlType::Inlet) {
				fccds.cvidxsinlet.push_back(aidx);
			}
			else {
				fccds.inflowSumPT_m3[aidx] = 0.0; // inlet 이 아닌 모든 fc 셀에 대해서 유입량 합 0으로 초기화.
			}
		}
		else {
			if (afc.fcType == flowControlType::SinkFlow
				|| afc.fcType == flowControlType::SourceFlow) {// 여기서 sink와 Source 를 후순위에 배치한다.
				prj.fcs[aidx].push_back(afc);
			}
			else {
				vector<flowControlinfo>::iterator it;
				it = prj.fcs[aidx].begin();
				prj.fcs[aidx].insert(it, afc);
			}
		}
	}

	// 위에서 우선순위 정리 된 것으로 시계열 자료 받는다.
	for (int idx : fccds.cvidxsFCcell) {
		for (int ifc = 0; ifc < prj.fcs[idx].size(); ++ifc) { // 최대값은 2, fc1, fc2, fc3
			flowControlinfo afc = prj.fcs[idx][ifc];
			if (afc.fcType == flowControlType::Inlet
				|| afc.fcType == flowControlType::ReservoirOutflow
				|| afc.fcType == flowControlType::SinkFlow
				|| afc.fcType == flowControlType::SourceFlow) {
				if (afc.fpnFCData == "" || _access(afc.fpnFCData.c_str(), 0) != 0) {
					string outstr = "ERROR : Flow control data file (" + afc.fpnFCData
						+ ") is invalid.\n";
					writeLog(fpnLog, outstr, 1, 1);
					return -1;
				}
				vector<double> vs;
				vs = readTextFileToDoubleVector(afc.fpnFCData);
				if (vs.size() == 0) {
					string outstr = "WARNNING : Flow control data file (" + afc.fpnFCData
						+ ") has no value.\n";
					writeLog(fpnLog, outstr, 1, 1);
					if (ts.enforceFCautoROM == -1) {
						return -1;
					}
				}
				for (int i = 0; i < vs.size(); ++i) {
					timeSeries ts;
					if (prj.isDateTimeFormat == 1) {
						ts.dataTime = timeElaspedToDateTimeFormat2(prj.simStartTime,
							afc.fcDT_min * 60 * i, timeUnitToShow::toM
							, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
					}
					else {
						ts.dataTime = afc.fcDT_min * i;
					}
					ts.value = vs[i];
					switch (ifc) {
					case 0: {
						fccds.inputFlowDataFCType1_m3Ps[idx].push_back(ts);
						break;
					}
					case 1: {
						fccds.inputFlowDataFCType2_m3Ps[idx].push_back(ts);
						break;
					}
					case 2: {
						fccds.inputFlowDataFCType3_m3Ps[idx].push_back(ts);
						break;
					}
					}
				}
			}
		}
    }
    if (fccds.cvidxsinlet.size() > 0) {
        prj.isinletExist = 1;
    }
    else { prj.isinletExist = -1; }
    return 1;
}

void calFCReservoirOutFlow(int i, double nowTmin)
{// nowTmin의 최소값은 dtsec/60이다.
    int dtfc = prj.fcs[i][0].fcDT_min;
	int calStreamFlow = 1;
	if (nowTmin >= dtfc * fccds.curDorder[i]) {
		if (fccds.curDorder[i] < fccds.inputFlowDataFCType1_m3Ps[i].size()) {
			fccds.curDorder[i]++;
		}
		else {
			// 아래 조건 주석처리하면, 마지막 자료가 끝까지 사용된다..
			if (ts.enforceFCautoROM == -1) {
				setNoFluxCVCH(i);
			}
			else if (ts.enforceFCautoROM == 1) {	
				double t_currentFC = dtfc * fccds.curDorder[i];
				convertFCtoAutoROM(dtos(t_currentFC, 2) +" min", i,0); // Reservoir 의 fc idx는 0
			}
			calStreamFlow = 0;
		}
	}
	if (calStreamFlow == 1) {
		int orderidx = fccds.curDorder[i] - 1;//vector index, 이 지점에서  fccds.curDorder[i]의 최소값은 1
		double v = fccds.inputFlowDataFCType1_m3Ps[i][orderidx].value;
		if (v < 0) { v = 0; }
		cvs[i].stream.QCH_m3Ps = v;
		cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i], cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
		cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
			cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
			cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
		cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
	}

    //아래에서  cvs[i].QsumCVw_dt_m3는 t-dt 에서의 값이 저장되어 있다.
    // 그리고, fcDataAppliedNowT_m3Ps 는 현재 셀에서 하류로 방류되는 값이므로, 빼준다.
	if (cvs[i].fcType1 == flowControlType::ReservoirOutflow && cvs[i].fac>0) {
		// 여기에서 w로 부터의 유입량 계산..
		if (cvs[i].flowType == cellFlowType::OverlandFlow) {
			getOverlandFlowDepthCVw(i);
		}
		else {
			getChCSAatCVW(i);
		}
		cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
			+ cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
			- cvs[i].stream.QCH_m3Ps * ts.dtsec;
		if (cvs[i].storageCumulative_m3 < 0) {
			cvs[i].storageCumulative_m3 = 0;
		}
	}
}


void calSinkOrSourceFlow(int i, double nowTmin, flowControlType fcType, int fcTypeOrder)
{// nowTmin의 최소값은 dtsec/60이다.
    int dtfc = prj.fcs[i][0].fcDT_min;
    double cellsize = di.cellSize;
	if (nowTmin > dtfc * fccds.curDorder[i]) {
		double t_currentFC = dtfc * fccds.curDorder[i];
		if (fccds.curDorder[i] < fccds.inputFlowDataFCType1_m3Ps[i].size()) {
			fccds.curDorder[i]++;
		}
		else {
			// 2023.11.23. Sinkflow, sourceflow에서는 저수지 제원 입력하지 않는다. 그러므로 AutoROM으로 강제 전환하는 기능 없다.
			//if (ts.enforceFCautoROM == -1) {
			//	setNoFluxCVCH(i);
			//}
			//else if (ts.enforceFCautoROM == 1) {
			//	convertFCtoAutoROM(dtos(t_currentFC, 2) + " min", i,0); // 첫번째 fc idx를 전환한다.
			//}
			return; //sink, source flow 모의하지 않는다
		}
	}
    int orderidx = fccds.curDorder[i]-1 ;//vector index, 이 지점에서 fccds.curDorder[i] 의 최소값은 1

	double QtoApp = 0.0;
	switch (fcTypeOrder) {
	case 1: {
		QtoApp = fccds.inputFlowDataFCType1_m3Ps[i][orderidx].value;
		break;
	}
	case 2: {
		QtoApp = fccds.inputFlowDataFCType2_m3Ps[i][orderidx].value;
		break;
	}
	case 3: {
		QtoApp = fccds.inputFlowDataFCType3_m3Ps[i][orderidx].value;
		break;
	}
	}
	
    if (cvs[i].flowType == cellFlowType::OverlandFlow) {
        switch (fcType) {
        case flowControlType::SinkFlow: {
            cvs[i].QOF_m3Ps = cvs[i].QOF_m3Ps - QtoApp;
            break;
        }
        case flowControlType::SourceFlow: {
            cvs[i].QOF_m3Ps = cvs[i].QOF_m3Ps + QtoApp;
            break;
        }
        }
        if (cvs[i].QOF_m3Ps < 0) {
            cvs[i].QOF_m3Ps = 0;
        }
        cvs[i].hOF = pow(cvs[i].rcOF * cvs[i].QOF_m3Ps 
            / cellsize / pow(cvs[i].slopeOF, 0.5), 0.6);
        cvs[i].csaOF = cvs[i].hOF * cellsize;
        cvs[i].stream.uCH = cvs[i].QOF_m3Ps / cvs[i].csaOF;
    }
    else {
        switch (fcType) {
        case flowControlType::SinkFlow: {
            cvs[i].stream.QCH_m3Ps = cvs[i].stream.QCH_m3Ps - QtoApp;
            break;
        }
        case flowControlType::SourceFlow: {
            cvs[i].stream.QCH_m3Ps = cvs[i].stream.QCH_m3Ps + QtoApp;
            break;
        }
        }
        if (cvs[i].stream.QCH_m3Ps < 0) {
            cvs[i].stream.QCH_m3Ps = 0;
        }
        cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i], 
            cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
        cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
            cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS,
            cvs[i].stream.chURBaseWidth_m, cvs[i].stream.chLRArea_m2,
            cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
        cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
    }
	int dtsec = ts.dtsec;
	if (fcType == flowControlType::SinkFlow) {
		cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
			+ cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
			- QtoApp * dtsec;
	}
	else if (fcType == flowControlType::SourceFlow) {//이전에 source flow 가 계산되었으면..
		cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
			+ cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
			+ QtoApp * dtsec;
	}
	if (cvs[i].storageCumulative_m3 < 0) {
		cvs[i].storageCumulative_m3 = 0;
	}
}


void calDetentionPond(int i, double nowTmin) {
	// 저류지는 댐과 다르게 하천 셀 옆에 있으므로, 유입량, 유출량, 저류량 변수를 별도로 배정한다.
	// DP_storageCumulative_m3, DP_inflow_m3Ps, DP_outflow_m3Ps
	int dtsec = ts.dtsec;
	double dy_m = di.cellSize;
	flowControlinfo afci;
	afci = prj.fcs[i][0];
	cvs[i].DP_inflow_m3Ps = 0.0;
	cvs[i].DP_outflow_m3Ps = 0.0;
	double Qflow = 0.0;
	if (cvs[i].flowType == cellFlowType::OverlandFlow) {
		if (cvs[i].DP_storageCumulative_m3 < afci.maxStorage_m3) {
			double Qi = 0.0;
			double rd2 = 0.0;
			Qflow = cvs[i].QOF_m3Ps;
			Qi = Qflow;
			rd2 = cvs[i].DP_storageCumulative_m3 + Qi * ts.dtsec;
			if (rd2 > afci.maxStorage_m3) {
				Qi = (afci.maxStorage_m3 - cvs[i].DP_storageCumulative_m3) / ts.dtsec;
			}
			Qflow = Qflow - Qi;
			cvs[i].QOF_m3Ps = Qflow;
			cvs[i].hOF = hByManningEqForOF(cvs[i].QOF_m3Ps, cvs[i].rcOF, cvs[i].slopeOF, dy_m);
			cvs[i].DP_inflow_m3Ps = Qi;
			cvs[i].DP_storageCumulative_m3 = cvs[i].DP_storageCumulative_m3 + Qi * ts.dtsec;
		}
		// Overland flow 셀에서는 저류지 유출을 고려하지 않는다.
	}
	else {
		Qflow = cvs[i].stream.QCH_m3Ps;
		// inflow to DP
		if (Qflow > afci.dp_QT_StoD_CMS) { // 기준 유량보다 하천 유량이 큰 경우
			if (cvs[i].DP_storageCumulative_m3 < afci.maxStorage_m3) {
				double rStD = 0.0;
				double Qi = 0.0;
				double Qdiff_max = 0.0;
				rStD = afci.dp_Wdi_m / (afci.dp_Wdi_m + afci.dp_Ws_m) * afci.dp_Cr_StoD;
				Qdiff_max = Qflow - afci.dp_QT_StoD_CMS;
				Qi = Qdiff_max * rStD;
				if (Qi > afci.dp_Qi_max_CMS) {
					Qi = afci.dp_Qi_max_CMS;
				}
				if (Qi > Qdiff_max) { // DP 유입 기준 유량을 초과하는 하천 최대 유량을 초과해서 유입되지 않게 한다.
					Qi = Qdiff_max;
				}
				double rd2 = 0.0;
				rd2 = cvs[i].DP_storageCumulative_m3 + Qi * ts.dtsec;
				if (rd2 > afci.maxStorage_m3) {
					Qi = (afci.maxStorage_m3 - cvs[i].DP_storageCumulative_m3) / ts.dtsec;
				}
				cvs[i].DP_inflow_m3Ps = Qi;
				Qflow = Qflow - Qi;
				cvs[i].stream.QCH_m3Ps = Qflow;
				calChannelAUH(i);
				cvs[i].DP_storageCumulative_m3 = cvs[i].DP_storageCumulative_m3 + Qi * ts.dtsec;
			}
		}
		else if (cvs[i].DP_storageCumulative_m3 > 0) { // 이때 저류량이 있으면, 저류지 유출이 발생한다.
			double rd2 = 0.0; //DP storage
			double QdpO = 0.0; // DP outflow
			rd2 = cvs[i].DP_storageCumulative_m3 - afci.dp_Qo_max_CMS * ts.dtsec;
			if (rd2 > 0.0) {
				QdpO = afci.dp_Qo_max_CMS;
			}
			else {
				QdpO = cvs[i].DP_storageCumulative_m3 / ts.dtsec;
			}
			double Qflow2 = 0.0;
			Qflow2 = Qflow + QdpO;
			if (Qflow2 > afci.dp_QT_StoD_CMS) { // QdpO==0 이면 이 조건에 들어오진 안는다.
				QdpO = afci.dp_QT_StoD_CMS - Qflow;
			}
			cvs[i].DP_outflow_m3Ps = QdpO;
			cvs[i].stream.QCH_m3Ps += QdpO;
			calChannelAUH(i);

			cvs[i].DP_storageCumulative_m3 = cvs[i].DP_storageCumulative_m3 - QdpO * ts.dtsec;
			if (cvs[i].DP_storageCumulative_m3 < 0.0) { cvs[i].DP_storageCumulative_m3 = 0.0; }
		}
	}
}


void calChannelAUH(int i) {
	cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i],
		cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
	cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
	cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
		cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS,
		cvs[i].stream.chURBaseWidth_m, cvs[i].stream.chLRArea_m2,
		cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
}


void calReservoirOperation(int i, double nowTmin)
{
    int dtsec = ts.dtsec;
	flowControlinfo afci;
	afci = prj.fcs[i][0];
    double QforDTbySinkOrSourceFlow = 0;
    double cellsize = di.cellSize;
   //이전에 sinkflow or source flow 였으면, calSinkOrSourceFlow(int i, double nowTmin) 에서 cell storage 미리 계산된다.
	//이전에 sinkflow or source flow 가 아니었으면, 현재상태에서의 cell storage 한번 계산한다.
	if (cvs[i].fcType1 != flowControlType::SinkFlow &&
	    cvs[i].fcType1 != flowControlType::SourceFlow){
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3
            + cvs[i].QsumCVw_dt_m3;
    }
	if (cvs[i].storageCumulative_m3 < 0) {
		cvs[i].storageCumulative_m3 = 0;
	}
    double Qout_cms = 0.0;
    double dy_m = di.cellSize;
	double maxStorageApp=0;
	maxStorageApp = afci.NormalHighStorage_m3;
	if (prj.isDateTimeFormat == 1) { // RestrictedStorage_m3>0 인 경우에만 적용
		//if (afci.RestrictedStorage_m3 > 0) { // 주석처리. 2024.08.14
			if (ts.tCurMonth >= afci.restricedP_SM
				&& ts.tCurDay >= afci.restricedP_SD) {
				if (ts.tCurMonth <= afci.restricedP_EM
					&& ts.tCurDay <= afci.restricedP_ED) {
					//maxStorageApp = afci.RestrictedStorage_m3; // 주석처리. 2024.08.14
					maxStorageApp = afci.maxStorage_m3; // 홍수기에서는 계획홍수위 저수량을 최대 저류량으로 설정. 2024.08.14
				}
			}
		//}
	}
	else {
		//if (afci.RestrictedStorage_m3 > 0) {// 주석처리. 2024.08.14
			if (afci.RestrictedPeriod_Start_min >= nowTmin
				&& afci.RestrictedPeriod_End_min <= nowTmin) {
				//maxStorageApp = afci.RestrictedStorage_m3;
				maxStorageApp = afci.maxStorage_m3; // 홍수기에서는 계획홍수위 저수량을 최대 저류량으로 설정. 2024.08.14
			}
		//}
	}
    switch (afci.roType) {
    case reservoirOperationType::AutoROM: {
        if (cvs[i].storageCumulative_m3 >= maxStorageApp) {
            Qout_cms = (cvs[i].storageCumulative_m3
                - maxStorageApp) / ts.dtsec; // 차이 있을 때 유출
			if (afci.autoROMmaxOutflow_cms > 0 &&
				Qout_cms > afci.autoROMmaxOutflow_cms) { // 2023.03.13 이거보다 작게 유출되게 한다.
				Qout_cms = afci.autoROMmaxOutflow_cms;
			}
			cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3 - Qout_cms * ts.dtsec;
			if (cvs[i].storageCumulative_m3 < 0) { 
				cvs[i].storageCumulative_m3 = 0; 
			}
        }
        if (Qout_cms < 0) { Qout_cms = 0; }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::RigidROM: {
		if (cvs[i].storageCumulative_m3 >= maxStorageApp) { // 현재 저류량이 최대 저류량보다 클때
			Qout_cms = (cvs[i].storageCumulative_m3
				- maxStorageApp) / ts.dtsec; // 차이만큼 모두 유출
			cvs[i].storageCumulative_m3 = maxStorageApp;
		}
		else  if (cvs[i].QsumCVw_m3Ps < afci.roConstQ_cms) {
			// 현재 유입량이 지정된 방류량보다 작은경우로 수정. 2024.05.21 <-- RigidROM에서 유입수문곡선의 첨두값을 미리 알수 없으므로, 지정된 유량을 기준으로 일정율을 적용한다.
			Qout_cms = cvs[i].QsumCVw_m3Ps * afci.roConstR;
		}
		//else  if (cvs[i].storageCumulative_m3 < afci.roConstQ_cms * ts.dtsec) {
		//    // 현재 저류량이 dtsec 유출량 보다 작은 경우, 저류된 모든 양이 유출
		//    Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
		//    cvs[i].storageCumulative_m3 = 0;
		//}
        else {
            Qout_cms = afci.roConstQ_cms;
            cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                - afci.roConstQ_cms * ts.dtsec;
            if (cvs[i].storageCumulative_m3 < 0) {
                cvs[i].storageCumulative_m3 = 0;
            }
        }
        if (Qout_cms < 0) { Qout_cms = 0; }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::ConstantQ: {
        if (nowTmin <= afci.roConstQDuration_hr * 60) {
            //여기서는 최대 저류 가능량에 상관 없이 일정량 방류            
            if (cvs[i].storageCumulative_m3 <= afci.roConstQ_cms * ts.dtsec) {
                // 이경우는 저류량이 모두 유출
                Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
                cvs[i].storageCumulative_m3 = 0;
            }
            else {
                Qout_cms = afci.roConstQ_cms * ts.dtsec;
                cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                    - afci.roConstQ_cms * ts.dtsec;
                if (cvs[i].storageCumulative_m3 < 0) {
                    cvs[i].storageCumulative_m3 = 0;
                }
            }
        }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::SDEqation: {
        break;
    }
    }
}

void calReservoirOutFlowInReservoirOperation(int i,
    double Qout_cms, double dy_m)
{
    if (Qout_cms > 0) {
        if (cvs[i].flowType == cellFlowType::OverlandFlow) {
            cvs[i].QOF_m3Ps = Qout_cms;
            cvs[i].hOF = hByManningEqForOF(cvs[i].QOF_m3Ps, cvs[i].rcOF, cvs[i].slopeOF, dy_m);
            cvs[i].stream.QCH_m3Ps = 0;
            cvs[i].stream.uCH = 0;
            cvs[i].stream.csaCH = 0;
            cvs[i].stream.hCH = 0;
            cvs[i].stream.uCH = 0;
        }
        else {
            cvs[i].QOF_m3Ps = 0;
            cvs[i].hOF = 0;
            cvs[i].stream.QCH_m3Ps = Qout_cms;
			calChannelAUH(i);
        }
    }
    else {
        cvs[i].hOF = 0;
        cvs[i].csaOF = 0;
        cvs[i].QOF_m3Ps = 0;
        cvs[i].stream.uCH = 0;
        cvs[i].stream.csaCH = 0;
        cvs[i].stream.hCH = 0;
        cvs[i].stream.uCH = 0;
        cvs[i].stream.QCH_m3Ps = 0;
    }
}

int getCVidxByFcName(string fcName)
{
    for (int idx : fccds.cvidxsFCcell) {
		for (int i = 0; i<prj.fcs[idx].size(); ++i) {
			if (fcName == prj.fcs[idx][i].fcName) {
				return idx;
			}
		}
    }
    return -1;
}

void convertFCtoAutoROM(string strDate, int cvidx, int ifc)
{
	reservoirOperationType rot_bak;
	rot_bak = prj.fcs[cvidx][ifc].roType;
	prj.fcs[cvidx][ifc].fcType = flowControlType::ReservoirOperation;
	prj.fcs[cvidx][ifc].roType = reservoirOperationType::AutoROM;
	cvs[cvidx].fcType1 = flowControlType::ReservoirOperation;
    string fcname = prj.fcs[cvidx][ifc].fcName;
	if (rot_bak != reservoirOperationType::AutoROM) {
		string msg = "WARNNING : Reservoir operation type was converted to AutoROM (FC Name:"
			+ fcname + ", (ColX, RowY):(" + to_string(prj.fcs[cvidx][ifc].fcColX)
			+", "+ to_string(prj.fcs[cvidx][ifc].fcRowY) + "), Time:" + strDate
			+ ").\n";
		writeLog(fpnLog, msg, 1, 1);
	}
	
}

