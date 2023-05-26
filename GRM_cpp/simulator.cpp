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
extern vector<weatherData> rfs;
extern vector<weatherData> tempMax;
extern vector<weatherData> tempMin;
extern vector<weatherData> solarRad;
extern vector<weatherData> dayTimeLength;
extern vector<weatherData> snowpackTemp;
extern flowControlCellAndData fccds;
extern wpSimData wpSimValue;
extern string msgFileProcess;

extern cvAtt* cvsb; 
//map<int, double> fcDataAppliedBak;// <idx, value> t-dt �ð��� ����� flow control data ��
map<int, double> fcInflowSumPT_m3_Bak;// <idx, value> t-dt �ð��� ����� flow control inflow ��
wpSimData wpSimValueB; // t-dt �ð��� ����� wp���� ������ �پ��� ����.  prj.wps ���Ͽ��� ���� ������� 2�� ���� ����
flowControlCellAndData fccdsb; // t-dt ���� ����� gmp ���Ͽ��� ���� fc ������ �̿��ؼ� 2�� ���� ����

int startSimulation()
{
	initThisSimulation();
	setCVStartingCondition(0);
	int dtRF_sec = prj.rfinterval_min * 60;
	int orderRF = 0;

	int dtTempMax_sec = prj.tempMaxInterval_min * 60;
	int orderTempMax = 0;
	int dtTempMin_sec = prj.tempMinInterval_min * 60;
	int orderTempMin = 0;
	int dtSolarRMin_sec = prj.solarRadInterval_min * 60;
	int orderSolarR = 0;
	int dtSunDur_sec = prj.daytimeLengthDataInterval_min * 60;
	int orderSunDur = 0;
	int dtSnowPackTemp = prj.snowpackTempInterval_min * 60;
	int orderSnowPackTemp = 0;

	int nowTsec = ts.dtsec;
	double nowTmin = 0.0;;

	int simEndingT_sec = ts.simEnding_sec;
	int loopEndingT_sec = ts.simEnding_sec + 1;// ts.dtsec + 1; ���⼭ +1�� �ϴ� ���� while ������ �ε�ȣ�� ����ϱ� ����
	while (nowTsec < loopEndingT_sec) {
		// dtsec���� �����ؼ�, ù��° ���췹�̾ �̿��� ���ǰ���� 0�ð��� ����Ѵ�.
		if (nowTsec > dtRF_sec * orderRF) {
			if (orderRF < ts.dataNumTotal_rf) {
				orderRF++; // �̷��� �ϸ� ������ ���̾� ����
				if (setCVRF(orderRF) == -1) { return -1; }    //isRFended = -1;
			}
			else {
				setCVRFintensityAndDTrf_Zero();
				orderRF = loopEndingT_sec / dtRF_sec +100;// ����� ū�� ����.   // INT_MAX ����ϸ�  dtRF_sec * orderRF ���� �ִ밪 �ʰ��ؼ� - ������ �����ȴ�.
			}
		}

		if (prj.simEvaportranspiration == 1 || prj.simSnowMelt == 1 
			|| prj.simInterception==1) {// �̰�� ����ڷ� �����Ѵ�.
			if (nowTsec > dtTempMax_sec * orderTempMax) {
				if (orderTempMax < ts.dataNumTotal_tempMax) {
					orderTempMax++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVTempMax(orderTempMax) == -1) { return -1; }    //isEnded = -1;
				}
				else {
					setCVTempMaxZero();		
					orderTempMax = loopEndingT_sec / dtTempMax_sec + 100;
				}
			}
			if (nowTsec > dtTempMin_sec * orderTempMin) {
				if (orderTempMin < ts.dataNumTotal_tempMin) {
					orderTempMin++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVTempMin(orderTempMin) == -1) { return -1; }    //isEnded = -1;
				}
				else {
					setCVTempMinZero();			
					orderTempMin = loopEndingT_sec / dtTempMin_sec + 100;
				}
			}
			if (nowTsec > dtSolarRMin_sec * orderSolarR) {
				if (orderSolarR < ts.dataNumTotal_solarR) {
					orderSolarR++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVSolarRad(orderSolarR) == -1) { return -1; }    //isEnded = -1;
				}
				else {
					setCVSolarRZero();		
					orderSolarR = loopEndingT_sec / dtSolarRMin_sec + 100;
				}
			}
			if (nowTsec > dtSunDur_sec * orderSunDur) {
				if (orderSunDur < ts.dataNumTotal_sunDur) {
					orderSunDur++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVSunDur(orderSunDur) == -1) { return -1; }    //isEnded = -1;
				}
				else {
					setCVSunDurZero();
					orderSunDur = loopEndingT_sec / dtSunDur_sec + 100;
				}
			}
			if (nowTsec > dtSnowPackTemp * orderSnowPackTemp) {
				if (orderSnowPackTemp < ts.dataNumTotal_snowPackTemp) {
					orderSnowPackTemp++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVSnowpackTemp(orderSnowPackTemp) == -1) { return -1; }    //isEnded = -1;
				}
				else {
					setCVSnowpackTempZero();
					orderSnowPackTemp = loopEndingT_sec / dtSnowPackTemp + 100;
				}
			}
			if (prj.isDateTimeFormat == 1) {
				string tElapsedStr = timeElaspedToDateTimeFormat2(prj.simStartTime,
					nowTsec, timeUnitToShow::toM,
					dateTimeFormat::yyyymmddHHMMSS);
				tm tCurDate = stringToDateTime(tElapsedStr);
				ts.tCurMonth = tCurDate.tm_mon;
				ts.tCurDay = tCurDate.tm_mday;
			}
		}
		nowTmin = nowTsec / 60.0;

		if (simulateRunoff(nowTmin) == -1) { return -1; }
		calValuesDuringPT(ts.dtsec);
		if (outputManager(nowTsec) == -1) { return -1; }
		if (nowTsec == simEndingT_sec) { break; }
		if (nowTsec + ts.dtsec > simEndingT_sec) {
			ts.dtsec = simEndingT_sec - nowTsec;
			nowTsec = simEndingT_sec; // �̷��� �ϸ� ts.simEnding_sec �� ��������.
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
		//if (ts.stopSim == 1) { break; }
	}

	if (ts.stopSim == 1) {
		writeLog(fpnLog, "Simulation was stopped.\n", 1, 1);
		return 1;
	}
	else {
		COleDateTime  timeNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
		printf("\rCurrent progress: 100.00%%... ");
		writeLog(fpnLog, "Simulation was completed. Run time: "
			+ to_string(tsTotalSim.GetHours()) + "h "
			+ to_string(tsTotalSim.GetMinutes()) + "m "
			+ to_string(tsTotalSim.GetSeconds()) + "s.\n", 1, 1);
		return 1;
	}
	return 1;
}

int simulateRunoff(double nowTmin)
{
    ts.vMaxInThisStep = 0.0;

//	// ������� parallel =============================
//    int numth = prj.mdp;
//    double* uMax = new double[numth]();// �̷��� �ϸ� 0 ���� �ʱ�ȭ �ȴ�.
//    for (int fac : fas) {
//        int nCVs = faCount[fac];
//#pragma omp parallel
//        {
//            // reduction���� max, min ã�� ���� openMP 3.1 �̻���� ����, 
//            // �迭 ����ϴ� ���� critical ���� ������..
//            int nth = omp_get_thread_num();
//            //uMax[nth] = DBL_MIN;
//#pragma omp for //  guided �Ⱦ��°� �� ������..
//            for (int e = 0; e < nCVs; ++e) {
//                int i = cvaisToFA[fac][e];
//                if (cvs[i].toBeSimulated == 1) {
//                    simulateRunoffCore(i, nowTmin);
//                    if (cvs[i].flowType == cellFlowType::OverlandFlow) {
//                        if (uMax[nth] < cvs[i].uOF) {
//                            uMax[nth] = cvs[i].uOF;
//                        }
//                    }
//                    else {
//                        if (uMax[nth] < cvs[i].stream.uCH) {
//                            uMax[nth] = cvs[i].stream.uCH;
//                        }
//                    }
//                }
//            }
//        }
//    }
//    for (int i = 0; i < prj.mdp; ++i) {
//        if (ts.vMaxInThisStep < uMax[i]) {
//            ts.vMaxInThisStep = uMax[i];
//        }
//    }
//
//	if (ts.vMaxInThisStep > GRAVITY_ACC) {
//		ts.vMaxInThisStep = GRAVITY_ACC;
//	}
//    delete[] uMax;
//	// parallel =============================

	// ������� serial===========
	double uMax_s = 0.0;
	for (int fac : fas) {
		int iterLimit = faCount[fac];
		for (int e = 0; e < iterLimit; ++e) {
			int i = cvaisToFA[fac][e];
			if (cvs[i].toBeSimulated == 1) {
				simulateRunoffCore(i, nowTmin);
				if (cvs[i].flowType == cellFlowType::OverlandFlow) {
					if (uMax_s < cvs[i].uOF) {
						uMax_s = cvs[i].uOF;
					}
				}
				else {
					if (uMax_s < cvs[i].stream.uCH) {
						uMax_s = cvs[i].stream.uCH;
					}
				}
			}
		}
	}
	if (uMax_s > GRAVITY_ACC) { uMax_s = GRAVITY_ACC; }
	ts.vMaxInThisStep = uMax_s;
	// serial===========

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
		updateCVbyHydroComps(i);
        //fccds.fcDataAppliedNowT_m3Ps[i] = 0;  // 2022.10.17 �ּ�ó��
        calFCReservoirOutFlow(i, nowTmin);
    }
    else {
        updateCVbyHydroComps(i);
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
          //fccds.fcDataAppliedNowT_m3Ps[i] = 0;  // 2022.10.17 �ּ�ó��
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
        ts.dataNumTotal_rf = (int)rfs.size();
		ts.dataNumTotal_tempMax = (int)tempMax.size();
		ts.dataNumTotal_tempMin = (int)tempMin.size();
		ts.dataNumTotal_solarR = (int)solarRad.size();
		ts.dataNumTotal_sunDur = (int)dayTimeLength.size();
		ts.dataNumTotal_snowPackTemp = (int)snowpackTemp.size();
    }
	else {
		ts.dataNumTotal_rf = 0;
		ts.dataNumTotal_tempMax = 0;
		ts.dataNumTotal_tempMin = 0;
		ts.dataNumTotal_solarR = 0;
		ts.dataNumTotal_sunDur = 0;
	}

	initRFvars();

    // �̷��� �ؾ� ���ǱⰣ�� �°� ����ȴ�. 
    //�ֳ��ϸ�, ù��° �����ڷḦ �̿��� ���� ����� 0 �ð����� ��µǱ� ������
	// �����ڷ�(������ ǥ�õǴ� ����ڷ�)�� �����ð������� ����, �����ڷ�� ���� �ð������� ������
	ts.simDuration_min = (int)prj.simDuration_hr * 60;//        +prj.dtPrint_min;
    ts.simEnding_sec = ts.simDuration_min * 60;
    ts.grmStarted = 1;
    ts.stopSim = -1;
    ts.vMaxInThisStep = 0.0;
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
	ts.dtsecUsed_tm1 = ts.dtsec;
	ts.targetTtoP_sec = (int)prj.dtPrint_min * 60;
	ts.targetTtoP_AVE_sec = (int)prj.dtPrintAveValue_min * 60;
	ts.TtoP_ave_check_sec = ts.targetTtoP_AVE_sec + prj.dtPrintAveValue_sec;

    time_t now = time(0);
    ts.time_thisSimStarted = COleDateTime::GetCurrentTime();
    //tsrt.g_RT_tStart_from_MonitorEXE = COleDateTime::GetCurrentTime();
	ts.showFileProgress = -1;
	if (msgFileProcess != "") {
		ts.showFileProgress = 1;
	}
}

void setCVStartingCondition(double iniflow)
{
    //#pragma omp parallel for //schedule(guided)
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
        cvs[i].QOF_m3Ps = 0;
        cvs[i].hUAQfromChannelBed_m = 0;
        cvs[i].csaOF = 0;
        if (cvs[i].flowType == cellFlowType::ChannelFlow
            || cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) {

			if (cvs[i].stream.cellValue > prj.swps[wsid].dryStreamOrder) {// ��õ�� �ƴϸ�,

				int iniStreamFlowWasSet = -1;
				if (prj.swps[wsid].userSet == 1 && prj.swps[wsid].iniFlow >= 0) {//Apply ini. flow of current sws
					iniQAtwsOutlet = prj.swps[wsid].iniFlow;
					faAtBaseCV = cvs[di.wsn.wsOutletidxs[wsid]].fac;
					iniStreamFlowWasSet = 1;
				}
				else {
					int baseWSid = wsid;
					for (int id = 0; id < di.wsn.wsidsAllDown[wsid].size(); id++) {
						int downWSid = di.wsn.wsidNearbyDown[baseWSid];
						if (downWSid > 0 && prj.swps[downWSid].iniFlow > 0) {// If this condition is satisfied, apply ini. flow of downstream ws.
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
					if (prj.icfFileApplied != 1) {
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
						double csPeri = cvs[i].stream.chBaseWidth + hChCVini * 2;
						double HRch = chCSAini / csPeri; // �ʱⰪ�� �������� ������ ������ �̿��Ѵ�.
						uChCVini = vByManningEq(HRch, cvs[i].stream.slopeCH, cvs[i].stream.chRC);
					}
				}
				cvs[i].stream.hCH = hChCVini;
				cvs[i].stream.csaCH = chCSAini;
				cvs[i].stream.hCH_ori = hChCVini;
				cvs[i].stream.csaCH_ori = chCSAini;
				cvs[i].stream.QCH_m3Ps = qChCVini;
				cvs[i].stream.uCH = uChCVini;
			}
			else { // ��õ�̸� 2023.04.12
				cvs[i].stream.hCH = 0.0;
				cvs[i].stream.csaCH = 0.0;
				cvs[i].stream.hCH_ori = 0.0;
				cvs[i].stream.csaCH_ori = 0.0;
				cvs[i].stream.QCH_m3Ps = 0.0;
				cvs[i].stream.uCH = 0.0;
			}

            if (prj.simBaseFlow == 1) {
                cvs[i].hUAQfromChannelBed_m = hChCVini; // �ϵ��� �ʱ� ������ ���Ǿд������ �ʱ� �������� ���� 
            }
            else {
                cvs[i].hUAQfromChannelBed_m = 0;
            }
        }
        cvs[i].rfiRead_tm1_mPsec = 0;
        cvs[i].rfiRead_mPsec = 0;
        cvs[i].rfEff_dt_m = 0;
        cvs[i].rfApp_mPdt = 0;
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

    for (int wpcvid : wpSimValue.wpCVidxes) {
        wpSimValue.prcpWPGridTotal_mm[wpcvid] = 0;
        wpSimValue.prcpUpWSAveTotal_mm[wpcvid] = 0;
		wpSimValue.Q_sumPTforAVE_m3[wpcvid] = 0.0;
    }
}


int outputManager(int nowTsec)//, int rfOrder)
{
    int dtP_min = prj.dtPrint_min;
	int dtP_SEC = dtP_min * 60;
    int timeToP_min = 0;// = nowTsec / 60;
	int timeToP_AVE_min = 0;
	int printAveValueNow = 0;
  //  if (rfOrder == 1
  //      && dtP_min > dtrf_min
  //      && ((nowTsec + ts.dtsec) > dtrf_sec)) {
  //      // ù��° ������� ���� ���ܿ��� ���췹�̾ �ٲ�� ���� ù��° ���췹�̾� �𵨸��� �����ٴ� ����̹Ƿ� �ѹ� ����Ѵ�.
  //      // 0 �ð������� �𵨸� ����� ����Ѵ�.
  //      double RFmeanForFirstLayer = ts.rfAveForDT_m / dtmin * dtrf_min;
		//if (writeBySimType(0, 1) == -1) {
		//	return -1;
		//}
  //      ts.zeroTimePrinted = 1;
  //      ts.isbak = -1;
  //      return 1; // �̰��� ���ǽ��� ���� ������  ts.targetTtoP_min = dtP_min �� ����
  //  }
  //  else if (nowTsec % dtP_SEC == 0) {
	if (nowTsec % dtP_SEC == 0) { // ���⼭ ���
		timeToP_min = ts.targetTtoP_sec / 60 - dtP_min; // �̷��� �ؾ� ù��° ���� ����� 0�ð��� ��µȴ�.
		if (prj.printAveValue == 1) {
			if (nowTsec % prj.dtPrintAveValue_sec == 0) { // ���⼭ ���
				timeToP_AVE_min = ts.targetTtoP_AVE_sec / 60 - prj.dtPrintAveValue_min;
				printAveValueNow = 1;
			}
		}

		if (writeBySimType(timeToP_min, 1, printAveValueNow, timeToP_AVE_min) == -1) {
			return -1;
		}
        ts.targetTtoP_sec = ts.targetTtoP_sec + dtP_SEC;
		if (printAveValueNow == 1) {
			ts.targetTtoP_AVE_sec = ts.targetTtoP_AVE_sec + prj.dtPrintAveValue_sec;
			ts.TtoP_ave_check_sec = ts.targetTtoP_AVE_sec + prj.dtPrint_min * 60;
		}

        ts.isbak = -1;
        return 1;
    }
    else {
		// ���⼭ ���
        if (nowTsec <  ts.targetTtoP_sec
            && (nowTsec + ts.dtsec) >ts.targetTtoP_sec) {

            // ���� ������ dtsec���� �ѹ��� �����ؼ� �� ������ �����ϸ�
            std::copy(cvs, cvs + di.cellNnotNull, cvsb);
            wpSimValueB = wpSimValue; // �����Ͱ� �ƴϹǷ�..
			ts.rfAveSumAllCells_PT_m_bak = ts.rfAveSumAllCells_PT_m;
			ts.rfAveSumAllCells_PTave_m_bak = ts.rfAveSumAllCells_PTave_m;

            if (prj.simFlowControl == 1) {
                //fcDataAppliedBak = fccds.fcDataAppliedNowT_m3Ps; // 2022.10.17 �ּ�ó��
				fcInflowSumPT_m3_Bak = fccds.inflowSumPT_m3;
            }
            ts.cvsbT_sec = nowTsec;
            ts.isbak = 1;
            return 1; //������ �ް� ������.
        }
		// ���⼭ ���
        if (nowTsec > ts.targetTtoP_sec
            && (nowTsec - ts.dtsecUsed_tm1) <= ts.targetTtoP_sec) {
            double citerp=0.0;
            citerp = (ts.targetTtoP_sec - ts.cvsbT_sec) / (double)(nowTsec - ts.cvsbT_sec);
            timeToP_min = (int)(ts.targetTtoP_sec / 60) - dtP_min; // �̷��� �ؾ� ù��° ���� ����� 0�ð��� ��µȴ�.
			
			if (prj.printAveValue == 1) {
				if (nowTsec >= ts.targetTtoP_AVE_sec && nowTsec <= ts.TtoP_ave_check_sec) { // ==�� ���� ���� ������ ����. ��� �ð����� ���� üũ�ϹǷ�, �ð� ���ϴ� ũ�� ���� ��. 
					timeToP_AVE_min = (int)(ts.targetTtoP_AVE_sec / 60) - prj.dtPrintAveValue_min; // �̷��� �ؾ� ù��° ���� ����� 0�ð��� ��µȴ�.
					printAveValueNow = 1;

				}
			}

			if (writeBySimType(timeToP_min, citerp, printAveValueNow, timeToP_AVE_min) == -1) {
				return -1;
			}
            ts.targetTtoP_sec = ts.targetTtoP_sec + dtP_SEC;

			if (printAveValueNow == 1) {
				ts.targetTtoP_AVE_sec = ts.targetTtoP_AVE_sec + prj.dtPrintAveValue_sec;
				ts.TtoP_ave_check_sec = ts.targetTtoP_AVE_sec + prj.dtPrint_min * 60;
			}
            ts.isbak = -1;
            return 1;
        }
    }
}

int  writeBySimType(int nowTP_min,
	double cinterp, int writeAVE, int timeToP_AVE_min) {
	COleDateTime timeNow;
	double TS_FromStarting_sec = 0.0;
	double TS_FromStarting_min = 0.0;
	string tStrToPrint;
	string tStrToPrintAve;
	timeNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
	TS_FromStarting_sec = tsTotalSim.GetTotalSeconds();
	TS_FromStarting_min = TS_FromStarting_sec / 60.0;
	string unitToP = "";
	if (prj.isDateTimeFormat == 1) {
		if (prj.simType == simulationType::Normal) {
			tStrToPrint = timeElaspedToDateTimeFormat2(prj.simStartTime,
				nowTP_min * 60, timeUnitToShow::toM,
				dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
			if (writeAVE == 1) {
				tStrToPrintAve = timeElaspedToDateTimeFormat2(prj.simStartTime,
					timeToP_AVE_min * 60, timeUnitToShow::toM,
					dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
			}
		}
		else {// real time �� ���
			tStrToPrint = timeElaspedToDateTimeFormat(prj.simStartTime,
				nowTP_min * 60, timeUnitToShow::toM,
				dateTimeFormat::yyyymmddHHMMSS);
			if (writeAVE == 1) {
				tStrToPrintAve = timeElaspedToDateTimeFormat(prj.simStartTime,
					timeToP_AVE_min * 60, timeUnitToShow::toM,
					dateTimeFormat::yyyymmddHHMMSS);
			}
		}
	}
	else {
		tStrToPrint = dtos(nowTP_min / 60.0, 2);
		tStrToPrintAve = dtos(timeToP_AVE_min / 60.0, 2);
		unitToP = " (hr)";
	}

	if (prj.writeLog == 1) {
		string logStr = "Sim. time" + unitToP + " : " + tStrToPrint + ", "
			+ "dt : " + to_string(ts.dtsec) + ", "
			+ "vMax : " + to_string(ts.vMaxInThisStep) + ", "
			+ "T from starting : " + dtos(TS_FromStarting_sec, 0) + "s ("
			+ dtos(TS_FromStarting_min, 2) + "min) \n ";
		writeLog(fpnLog, logStr, 1, -1);
	}

	simulationType simType = prj.simType;
	switch (simType) {
	case simulationType::Normal: {
		writeSimProgress(nowTP_min);
		if (prj.printOption == GRMPrintType::All) {
			writeDischargeFile(tStrToPrint, cinterp);
			if (writeAVE ==1) {
				writeDischargeAveFile(tStrToPrintAve, cinterp);
			}
			writeWPoutputFile(tStrToPrint, cinterp);
			if (prj.simFlowControl == 1) {
				writeFCOutputFile(tStrToPrint, cinterp);
				if (writeAVE == 1) {
					writeFCAveOutputFile(tStrToPrintAve, cinterp);
				}
			}
			writeRainfallOutputFile(tStrToPrint, cinterp);
		}
		else if (prj.printOption == GRMPrintType::DischargeAndFcFile) {
			writeDischargeFile(tStrToPrint, cinterp);
			if (writeAVE == 1) {
				writeDischargeAveFile(tStrToPrintAve, cinterp);
			}
			if (prj.simFlowControl == 1) {
				writeFCOutputFile(tStrToPrint, cinterp);
				if (writeAVE == 1)  {
					writeFCAveOutputFile(tStrToPrintAve, cinterp);
				}
			}
		}
		else if (prj.printOption == GRMPrintType::DischargeFile) {
			writeDischargeFile(tStrToPrint, cinterp);
			if (writeAVE == 1) {
				writeDischargeAveFile(tStrToPrintAve, cinterp);
			}
		} 
		else if (prj.printOption == GRMPrintType::AverageFile && writeAVE == 1) {
				writeDischargeAveFile(tStrToPrintAve, cinterp);
				if (prj.simFlowControl == 1) {
					writeFCAveOutputFile(tStrToPrintAve, cinterp);
			}
		}
		// �Ʒ��� Q �� ��� =======================
		else if (prj.printOption == GRMPrintType::AllQ) {
			writeDischargeFile("", cinterp);
			if (writeAVE == 1) {
				writeDischargeAveFile("", cinterp);
			}
			writeWPoutputFile("", cinterp);
			if (prj.simFlowControl == 1) {
				writeFCOutputFile("", cinterp);
				if (writeAVE == 1) {
					writeFCAveOutputFile("", cinterp);
				}
			}
		}
		else if (prj.printOption == GRMPrintType::DischargeFileQ) {
			writeDischargeFile("", cinterp);
			if (writeAVE == 1) {
				writeDischargeAveFile("", cinterp);
			}
		}
		else if (prj.printOption == GRMPrintType::AverageFileQ) {
			writeDischargeAveFile("", cinterp);
			if (prj.simFlowControl == 1) {
				writeFCAveOutputFile("", cinterp);
			}
		}
		//============================
		break;
	}
	case simulationType::RealTime: {
		writeRealTimeSimResults(tStrToPrint, cinterp, TS_FromStarting_sec);
		break;
	}
	}
	if (ts.runByAnalyzer == 1) {
		// Ŭ������ ���� ������ �����ؼ�, �ܺη� �����ϰ�, analyzer���� ����ϴ� �����?
		//���� ����� Ȥ�� 1�ʸ��� íƮ ������Ʈ�ϴ� ���?
		//SendQToAnalyzer(nowTP_min, cinterp);
	}
	if (prj.makeASCorIMGfile == 1) {
		if (makeRasterOutput(nowTP_min) == -1) {
			return -1;
		}
	}
	initValuesAfterPrinting(nowTP_min, writeAVE);
	return 1;
}

// ���⼭ ����� �� ���� �ʱ�ȭ �Ǿ�� �ϴ� ������ ó��
void initValuesAfterPrinting(int nowTP_min, int printAveValueNow) {
	ts.rfAveSumAllCells_PT_m = 0.0;
	if (printAveValueNow == 1) {
		ts.rfAveSumAllCells_PTave_m = 0.0;
	}
	for (int idx : wpSimValue.wpCVidxes) {
		wpSimValue.prcpUpWSAveForPT_mm[idx] = 0.0;
		wpSimValue.prcpWPGridForPT_mm[idx] = 0.0;
		wpSimValue.pet_sumPT_mm[idx] = 0.0;
		wpSimValue.aet_sumPT_mm[idx] = 0.0;
		wpSimValue.snowM_sumPT_mm[idx] = 0.0;
		if (printAveValueNow == 1 ) {
			wpSimValue.Q_sumPTforAVE_m3[idx] = 0.0;
		}
	}
	//fcDataAppliedBak.clear(); // 2022.10.17 �ּ�ó��
	if (printAveValueNow == 1 ) {
		for (int idx : fccds.cvidxsFCcell) {
			if (prj.fcs[idx].fcType != flowControlType::Inlet) {
				fccds.inflowSumPT_m3[idx] = 0.0;
			}
		}
		fcInflowSumPT_m3_Bak.clear();
	}
}

void calValuesDuringPT(int dtsec)
{
	ts.rfAveForDT_m = ts.rfiSumAllCellsInCurRFData_mPs * dtsec
		/ di.cellNtobeSimulated;
	ts.rfAveSumAllCells_PT_m = ts.rfAveSumAllCells_PT_m
		+ ts.rfAveForDT_m;
	if (prj.printAveValue == 1) {
		ts.rfAveSumAllCells_PTave_m = ts.rfAveSumAllCells_PTave_m
			+ ts.rfAveForDT_m;
	}

	for (int idx : wpSimValue.wpCVidxes) {
		wpSimValue.prcpUpWSAveForDt_mm[idx] = wpSimValue.prcpiReadSumUpWS_mPs[idx]
			* dtsec * 1000.0 / (double)wpSimValue.cvCountAllup[idx]; //(double)(cvs[idx].fac + 1);
		wpSimValue.prcpUpWSAveForPT_mm[idx] = wpSimValue.prcpUpWSAveForPT_mm[idx]
			+ wpSimValue.prcpUpWSAveForDt_mm[idx];
		wpSimValue.prcpWPGridForPT_mm[idx] = wpSimValue.prcpWPGridForPT_mm[idx]
			+ cvs[idx].rfiRead_mPsec * 1000.0 * dtsec;
		wpSimValue.pet_sumPT_mm[idx] += cvs[idx].pet_mPdt * 1000.0 ;
		wpSimValue.aet_sumPT_mm[idx] += cvs[idx].aet_mPdt * 1000.0 ;
		wpSimValue.snowM_sumPT_mm[idx] += cvs[idx].smelt_mPdt * 1000.0;

		if (prj.printAveValue == 1) {
			if (cvs[idx].flowType == cellFlowType::OverlandFlow) {
				wpSimValue.Q_sumPTforAVE_m3[idx] += cvs[idx].QOF_m3Ps * dtsec;
			}
			else {
				wpSimValue.Q_sumPTforAVE_m3[idx] += cvs[idx].stream.QCH_m3Ps * dtsec;
			}
		}
	}

	if (prj.printAveValue == 1) {
		for (int idx : fccds.cvidxsFCcell) {
			if (prj.fcs[idx].fcType != flowControlType::Inlet) {
				fccds.inflowSumPT_m3[idx] += cvs[idx].QsumCVw_m3Ps * dtsec; // inlet �� �ƴ� ��� fc ���� ���ؼ� 	��±Ⱓ������  inflowSumPT_m3 ���
			}
		}
	}
}