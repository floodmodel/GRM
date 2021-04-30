#include "stdafx.h"
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern thisSimulation ts;

extern domaininfo di;
extern cvAtt* cvs;
extern cvpos* cvps;
extern map<int, int*> cvaisToFA; //fa�� cv array idex ���
extern vector<int> fas;
extern map<int, int> faCount;
extern vector<rainfallData> rfs;
extern flowControlCellAndData fccds;
extern wpinfo wpis;

extern cvAtt* cvsb; 
map<int, double> fcdAb;// <idx, value> t-dt �ð��� ����� flow control data ��
wpinfo wpisb;

int startSimulation()
{
    initThisSimulation();
    setCVStartingCondition(0);
    int dtRF_sec = prj.rfinterval_min * 60;
    int rfOrder = 0;
    int nowTsec = ts.dtsec;
    double nowTmin;
    ts.dtsecUsed_tm1 = ts.dtsec;
    ts.targetTtoP_sec = (int)prj.dtPrint_min * 60;
    int endingT_sec = ts.simEnding_sec + ts.dtsec+1;
    while (nowTsec < endingT_sec) {
        //dtsec = ts.dtsec;        
        // dtsec���� �����ؼ�, ù��° ���췹�̾ �̿��� ���ǰ���� 0�ð��� ����Ѵ�.
        if (nowTsec > dtRF_sec* rfOrder) {
            if (rfOrder < ts.rfDataCountTotal) {
                rfOrder++; // �̷��� �ϸ� ������ ���̾� ����
                if (setCVRF(rfOrder) == -1) { return -1; }    //isRFended = -1;
            }
            else {
                setRFintensityAndDTrf_Zero();
                rfOrder = INT_MAX;   //isRFended = 1;
            }
        }
        nowTmin = nowTsec / 60.0;
		if (prj.isDateTimeFormat == 1 && prj.simFlowControl == 1) {
			string tElapsedStr = timeElaspedToDateTimeFormat2(prj.simStartTime,
				nowTmin * 60, timeUnitToShow::toM,
				dateTimeFormat::yyyymmddHHMMSS);
			tm tElapsed = stringToDateTime(tElapsedStr);
			ts.tElapsed_DateTime_Month = tElapsed.tm_mon;
			ts.tElapsed_DateTime_Day = tElapsed.tm_mday;
		}
        if (simulateRunoff(nowTmin) == -1) { return -1; }
        calWPCumulRFduringDTP(ts.dtsec);
        outputManager(nowTsec, rfOrder);
        if (nowTsec + ts.dtsec > endingT_sec) {
            ts.dtsec = nowTsec + ts.dtsec - endingT_sec;
            nowTsec = endingT_sec;
            ts.dtsecUsed_tm1 = ts.dtsec;
        }
        else {
            nowTsec = nowTsec + ts.dtsec; // dtsec ��ŭ ����
            ts.dtsecUsed_tm1 = ts.dtsec;
            if (prj.isFixedTimeStep == -1) {
                ts.dtsec = getDTsec(
                    di.cellSize, ts.vMaxInThisStep, ts.dtMaxLimit_sec, 
                    ts.dtMinLimit_sec);
            }
        }
        if (ts.stopSim == 1) { break; }
    }
    if (ts.stopSim == 1) {
        writeLog(fpnLog, "Simulation was stopped.\n", 1, 1);
        return 1;
    }
    else {
        COleDateTime  timeNow = COleDateTime::GetCurrentTime();
        COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
            writeLog(fpnLog, "Simulation was completed. Run time: "
                +to_string(tsTotalSim.GetHours())+"hrs "
                +to_string(tsTotalSim.GetMinutes())+"min "
                +to_string(tsTotalSim.GetSeconds())+"sec.\n", 1, 1);
        return 1;
    }
    return 1;
}

int simulateRunoff(double nowTmin)
{
    ts.vMaxInThisStep = DBL_MIN;
	// ������� parallel =============================
    int numth = prj.mdp;
    double* uMax = new double[numth];// �̷��� �ϸ� ���� ���� ������ �ʱ�ȭ �ȴ�.
    for (int fac : fas) {
        int iterLimit = faCount[fac];
#pragma omp parallel
        {
            // reduction���� max, min ã�� ���� openMP 3.1 �̻���� ����, 
            // �迭 ����ϴ� ���� critical ���� ������..
            int nth = omp_get_thread_num();
            uMax[nth] = DBL_MIN;
#pragma omp for//  guided �Ⱦ��°� �� ������..
            for (int e = 0; e < iterLimit; ++e) {
                int i = cvaisToFA[fac][e];
                if (cvs[i].toBeSimulated == 1) {
                    simulateRunoffCore(i, nowTmin);
                    if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                        if (uMax[nth] < cvs[i].uOF) {
                            uMax[nth] = cvs[i].uOF;
                        }
                    }
                    else {
                        if (uMax[nth] < cvs[i].stream.uCH) {
                            uMax[nth] = cvs[i].stream.uCH;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < prj.mdp; ++i) {
        if (ts.vMaxInThisStep < uMax[i]) {
            ts.vMaxInThisStep = uMax[i];
        }
    }
    delete[] uMax;
	// parallel =============================

	//// ������� serial===========
	//double uMax_s = DBL_MIN;
	//for (int fac : fas) {
	//	int iterLimit = faCount[fac];
	//	for (int e = 0; e < iterLimit; ++e) {
	//		int i = cvaisToFA[fac][e];
	//		if (cvs[i].toBeSimulated == 1) {
	//			simulateRunoffCore(i, nowTmin);
	//			if (cvs[i].flowType == cellFlowType::OverlandFlow) {
	//				if (uMax_s < cvs[i].uOF) {
	//					uMax_s = cvs[i].uOF;
	//				}
	//			}
	//			else {
	//				if (uMax_s < cvs[i].stream.uCH) {
	//					uMax_s = cvs[i].stream.uCH;
	//				}
	//			}
	//		}
	//	}
	//}
	//ts.vMaxInThisStep = uMax_s;
	//// serial===========
    return 1;
}


void simulateRunoffCore(int i, double nowTmin)
{
    int fac = cvs[i].fac;
    int dtsec = ts.dtsec;
    double cellsize = di.cellSize;
    if (prj.makeRFraster == 1){
        cvs[i].rf_dtPrint_m = cvs[i].rf_dtPrint_m
            + cvs[i].rfiRead_mPsec * dtsec;
        cvs[i].rfAcc_fromStart_m = cvs[i].rfAcc_fromStart_m
            + cvs[i].rfiRead_mPsec * dtsec;
    }
    if (prj.simFlowControl == 1 &&
        (cvs[i].fcType == flowControlType::ReservoirOutflow ||
            cvs[i].fcType == flowControlType::Inlet)) {
        fccds.fcDataAppliedNowT_m3Ps[i] = 0;
        calFCReservoirOutFlow(i, nowTmin);
    }
    else {
        updatetCVbyRFandSoil(i);
        if (cvs[i].flowType == cellFlowType::OverlandFlow) {
            double hCVw_tp1 = 0;
            if (fac > 0) {
                hCVw_tp1 = getOverlandFlowDepthCVw(i);
            }
            if (hCVw_tp1 > 0 || cvs[i].hOF > 0) {
                calOverlandFlow(i, hCVw_tp1, cellsize);
            }
            else {
                setNoFluxCVOF(i);
            }
        }
        else { 
            double CSAchCVw_i_jP1 = 0;
            if (fac > 0) {
                CSAchCVw_i_jP1 = getChCSAatCVW(i);
            }
            if (CSAchCVw_i_jP1 > 0 || cvs[i].stream.hCH > 0) {
                calChannelFlow(i, CSAchCVw_i_jP1);
            }
            else {
                setNoFluxCVCH(i);
            }
        }
    }
    if (prj.simFlowControl == 1) {
      if (cvs[i].fcType== flowControlType::SinkFlow
          || cvs[i].fcType == flowControlType::SourceFlow
          || cvs[i].fcType == flowControlType::ReservoirOperation) {
          fccds.fcDataAppliedNowT_m3Ps[i] = 0;
          if (cvs[i].fcType == flowControlType::SinkFlow
              || cvs[i].fcType == flowControlType::SourceFlow) {
              calSinkOrSourceFlow(i, nowTmin);
          }
          if (prj.fcs[i].roType != reservoirOperationType::None) {
              // rotype�� ������, ro�� �Ѿ��.
              calReservoirOperation(i, nowTmin);
          }
      }
    }
}


void initThisSimulation()
{
    if (prj.simType != simulationType::RealTime) {
        ts.rfDataCountTotal = (int)rfs.size();
    }
    else {
        ts.rfDataCountTotal = 0;
    }
	initRFvars();
    // �̷��� �ؾ� ���ǱⰣ�� �°� ����ȴ�. 
    //�ֳ��ϸ�, ù��° ���� ����� 0 �ð����� ��µǱ� ������
    ts.simDuration_min = (int)prj.simDuration_hr * 60
        + prj.dtPrint_min;
    ts.simEnding_sec = ts.simDuration_min * 60;
    ts.setupGRMisNormal = 1;
    ts.grmStarted = 1;
    ts.stopSim = -1;
    ts.vMaxInThisStep = DBL_MIN;
    //ts.iscvsb = -1;
    ts.cvsbT_sec = 0;
    ts.dtMinLimit_sec = 1;
    ts.zeroTimePrinted = -1;
    //dtMaxLimit�� ��½ð�����, ����, fc�� �ð������� ��� ����ؼ� �� ������ �Ѵ�.
    int dtFromP = (int)(prj.dtPrint_min * 60 / 2);
    int dtFromR = (int)(prj.rfinterval_min * 60 / 2);
    ts.dtMaxLimit_sec = min(dtFromP, dtFromR);
    if (prj.simFlowControl == 1) {
        int mindtFC = INT_MAX;
        for (int i :fccds.cvidxsFCcell) {
            if (mindtFC > prj.fcs[i].fcDT_min) {
                mindtFC = int(prj.fcs[i].fcDT_min *60 / 2);
            }
        }
        if (ts.dtMaxLimit_sec > mindtFC) {
            ts.dtMaxLimit_sec = mindtFC;
        }
    }
    if (ts.dtMaxLimit_sec < ts.dtMinLimit_sec) {
        ts.dtMaxLimit_sec = ts.dtMinLimit_sec;
    }
    if (ts.dtMaxLimit_sec < prj.dtsec) {
        prj.dtsec = ts.dtMaxLimit_sec;
    }
    ts.dtsec = prj.dtsec;

    time_t now = time(0);
    ts.time_thisSimStarted = COleDateTime::GetCurrentTime();
    //tsrt.g_RT_tStart_from_MonitorEXE = COleDateTime::GetCurrentTime();
}

void setCVStartingCondition(double iniflow)
{
    #pragma omp parallel for //schedule(guided)
    for (int i = 0; i < di.cellNnotNull; ++i) {
        double hChCVini;
        double chCSAini;
        double qChCVini;
        double uChCVini;
        double iniQAtwsOutlet = 0;
        int faAtBaseCV = di.facMax;
        int wsid = cvps[i].wsid;
        cvs[i].uOF = 0;
        cvs[i].hOF = 0;
        cvs[i].hOF_ori = 0;
        cvs[i].QOF_m3Ps = 0;
        cvs[i].hUAQfromChannelBed_m = 0;
        cvs[i].csaOF = 0;
        if (cvs[i].flowType == cellFlowType::ChannelFlow
            || cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) {
            int iniStreamFlowWasSet = -1;
            if (prj.swps[wsid].iniFlow > 0) {//Apply ini. flow of current sws
                iniQAtwsOutlet = prj.swps[wsid].iniFlow;
                faAtBaseCV = cvs[di.wsn.wsOutletidxs[wsid]].fac;
                iniStreamFlowWasSet = 1;
            }
            else {
                int baseWSid = wsid;
                for (int id = 0; id < di.wsn.wsidsAllDown[wsid].size(); id++) {
                    int downWSid = di.wsn.wsidNearbyDown[baseWSid];
                    if (prj.swps[downWSid].iniFlow > 0) {// If this condition is satisfied, apply ini. flow of downstream ws.
                        iniQAtwsOutlet = prj.swps[downWSid].iniFlow;
                        faAtBaseCV = cvs[di.wsn.wsOutletidxs[downWSid]].fac;
                        iniStreamFlowWasSet = 1;
                        break;
                    }
                    else {// Search next downstream ws
                        baseWSid = downWSid;
                    }
                }
            }
            chCSAini = 0;
            hChCVini = 0;
            qChCVini = 0;
            uChCVini = 0;
            if (iniStreamFlowWasSet == 1) {
                if (prj.simType == simulationType::Normal_PE_SSR) {
                    qChCVini = iniflow * (cvs[i].fac - di.facMostUpChannelCell)
                        / (double)(faAtBaseCV - di.facMostUpChannelCell);
                }
                else if (prj.icfFileApplied != 1) {
                    qChCVini = iniQAtwsOutlet * (cvs[i].fac - di.facMostUpChannelCell)
                        / (double)(faAtBaseCV - di.facMostUpChannelCell);
                }
                else {
                    qChCVini = cvs[i].stream.iniQCH_m3Ps;
                }
                if (qChCVini > 0) {
                    double sngCAS_ini = qChCVini / (double)cvs[i].cvdx_m; // �ʱⰪ ����
                    chCSAini = getChCSAusingQbyiteration(cvs[i], sngCAS_ini, qChCVini);
                    hChCVini = getChDepthUsingCSA(cvs[i].stream.chBaseWidth, chCSAini,
                        cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
                        cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight,
                        cvs[i].stream.bankCoeff);
                }
            }
            cvs[i].stream.hCH = hChCVini;
            cvs[i].stream.csaCH = chCSAini;
            cvs[i].stream.hCH_ori = hChCVini;
            cvs[i].stream.csaCH_ori = chCSAini;
            cvs[i].stream.QCH_m3Ps = qChCVini;
            cvs[i].stream.uCH = uChCVini;
            if (prj.simBaseFlow == 1) {
                cvs[i].hUAQfromChannelBed_m = hChCVini; // �ϵ��� �ʱ� ������ ���Ǿд������ �ʱ� �������� ���� 
            }
            else {
                cvs[i].hUAQfromChannelBed_m = 0;
            }
        }
        //cv.Qprint_cms = 0;
        cvs[i].rfiRead_tm1_mPsec = 0;
        cvs[i].rfiRead_mPsec = 0;
        cvs[i].rfEff_dt_m = 0;
        cvs[i].rfApp_dt_m = 0;
        cvs[i].rf_dtPrint_m = 0;
        cvs[i].rfAcc_fromStart_m = 0;
        cvs[i].soilMoistureChange_DTheta = 0;
        cvs[i].ifF_mPdt = 0;
        cvs[i].ifRatef_mPsec = 0;
        cvs[i].ifRatef_tm1_mPsec = 0;
        cvs[i].effSR_Se = 0;
        cvs[i].isAfterSaturated = -1;
        cvs[i].storageAddedForDTbyRF_m3 = 0;
        cvs[i].QsumCVw_dt_m3 = 0;
		cvs[i].QsumCVw_m3Ps = 0;
        cvs[i].effCVnFlowintoCVw = 0;
        cvs[i].QSSF_m3Ps = 0;
        cvs[i].bfQ_m3Ps = 0;
        cvs[i].hUAQfromBedrock_m = CONST_UAQ_HEIGHT_FROM_BEDROCK;
        cvs[i].storageCumulative_m3 = 0;
        if (prj.simFlowControl == 1) {
            //int i = i + 1;
            if (prj.fcs.size() > 0 && getVectorIndex(fccds.cvidxsFCcell, i) != -1) {
                double iniS = prj.fcs[i].iniStorage_m3;
                if (iniS > 0) {
                    cvs[i].storageCumulative_m3 = iniS;
                }
                else {
                    cvs[i].storageCumulative_m3 = 0;
                }
            }
        }
    }

    for (int wpcvid : wpis.wpCVidxes) {
        wpis.maxDepth_m[wpcvid] = 0;
        wpis.maxDepthTime[wpcvid] = "";
        wpis.maxFlow_cms[wpcvid] = 0;
        wpis.maxFlowTime[wpcvid] = "";
        wpis.rfWPGridTotal_mm[wpcvid] = 0;
        wpis.rfUpWSAveTotal_mm[wpcvid] = 0;
        wpis.totalDepth_m[wpcvid] = 0;
        wpis.totalFlow_cms[wpcvid] = 0;
    }
}


void outputManager(int nowTsec, int rfOrder)
{
    int dtP_min = prj.dtPrint_min;
    int dtrf_min = (int)prj.rfinterval_min;
    int dtrf_sec = dtrf_min * 60;
    int dtP_SEC = dtP_min * 60;
    double dtmin = ts.dtsec / 60.0;
    int timeToP_min = 0;// = nowTsec / 60;
    if (rfOrder == 1
        && dtP_min > dtrf_min
        && ((nowTsec + ts.dtsec) > dtrf_sec)) {
        // ù��° ������� ���� ���ܿ��� ���췹�̾ �ٲ�� ���� ù��° ���췹�̾� �𵨸��� �����ٴ� ����̹Ƿ� �ѹ� ����Ѵ�.
        // 0 �ð������� �𵨸� ����� ����Ѵ�.
        double RFmeanForFirstLayer = ts.rfAveForDT_m / dtmin * dtrf_min;
        writeBySimType(0, 1);
        ts.zeroTimePrinted = 1;
        ts.isbak = -1;
        return; // �̰��� ���ǽ��� ���� ������  ts.targetTtoP_min = dtP_min �� ����
    }
    else if (nowTsec % dtP_SEC == 0) {
        if (ts.zeroTimePrinted == -1) {
            timeToP_min = ts.targetTtoP_sec / 60 - dtP_min; // �̷��� �ؾ� ù��° ���� ����� 0�ð��� ��µȴ�.
        }
        else {
            timeToP_min = ts.targetTtoP_sec / 60;
        }
        writeBySimType(timeToP_min, 1);
        ts.targetTtoP_sec = ts.targetTtoP_sec + dtP_SEC;
        ts.isbak = -1;
        return;
    }
    else {
        if (nowTsec <  ts.targetTtoP_sec
            && (nowTsec + ts.dtsec) >ts.targetTtoP_sec) {
            // ���� ������ dtsec���� �ѹ��� �����ؼ� �� ������ �����ϸ�
            std::copy(cvs, cvs + di.cellNnotNull, cvsb);
            wpisb = wpis;
            if (prj.simFlowControl == 1) {
                fcdAb = fccds.fcDataAppliedNowT_m3Ps;
            }
            ts.cvsbT_sec = nowTsec;
            ts.isbak = 1;
            return; //������ �ް� ������.
        }
        if (nowTsec > ts.targetTtoP_sec
            && (nowTsec - ts.dtsecUsed_tm1) <= ts.targetTtoP_sec) {
            double citerp;
            citerp = (ts.targetTtoP_sec - ts.cvsbT_sec) / (double)(nowTsec - ts.cvsbT_sec);
            timeToP_min = (int)(ts.targetTtoP_sec / 60) - dtP_min; // �̷��� �ؾ� ù��° ���� ����� 0�ð��� ��µȴ�.
            writeBySimType(timeToP_min, citerp);
            ts.targetTtoP_sec = ts.targetTtoP_sec + dtP_SEC;
            ts.isbak = -1;
            return;
        }
    }
}

void writeBySimType(int nowTP_min,
	double cinterp)
{
	simulationType simType = prj.simType;
	switch (simType) {
	case simulationType::Normal: {
		writeSimStep(nowTP_min);
		if (prj.printOption == GRMPrintType::All
			|| prj.printOption == GRMPrintType::DischargeFile
			|| prj.printOption == GRMPrintType::DischargeAndFcFile) {
			writeSingleEvent(nowTP_min, cinterp);
		}
		else if (prj.printOption == GRMPrintType::DischargeFileQ) {
			writeDischargeOnly(cinterp, -1, -1);
		}
		else if (prj.printOption == GRMPrintType::AllQ) {
			writeDischargeOnly(cinterp, 1, 1);
		}
		break;
	}
	case simulationType::RealTime: {
		writeRealTimeSimResults(nowTP_min, cinterp);
		break;
	}
	}
	if (ts.runByAnalyzer == 1) {
		// Ŭ������ ���� ������ �����ؼ�, �ܺη� �����ϰ�, analyzer���� ����ϴ� �����?
		//���� ����� Ȥ�� 1�ʸ��� íƮ ������Ʈ�ϴ� ���?
		//SendQToAnalyzer(nowTP_min, cinterp);
	}
	if (prj.makeASCorIMGfile == 1) {
		makeRasterOutput(nowTP_min);
	}
	ts.rfAveSumAllCells_dtP_m = 0;
	for (int idx : wpis.wpCVidxes) {
		wpis.rfUpWSAveForDtP_mm[idx] = 0;
		wpis.rfWPGridForDtP_mm[idx] = 0;
	}
}