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
//extern vector<weatherData> rfs;
//extern vector<weatherData> tempMax;
//extern vector<weatherData> tempMin;
//extern vector<weatherData> solarRad;
//extern vector<weatherData> dayTimeLength;
//extern vector<weatherData> dewPointTemp;// �̽��� �µ� ���Ͽ��� ���� �ڷ�
//extern vector<weatherData> windSpeed;// ǳ�� ���Ͽ��� ���� �ڷ�
//extern vector<weatherData> userPET;// ����� �Է� �������߻귮
//extern vector<weatherData> snowpackTemp;

extern weatherData* rfs;
extern weatherData* tempMax;
extern weatherData* tempMin; // �ִ��� ���Ͽ��� ���� �ڷ�
extern weatherData* dayTimeLength; // �ִ��� ���Ͽ��� ���� �ڷ�
extern weatherData* solarRad; // �ִ��� ���Ͽ��� ���� �ڷ�
extern weatherData* dewPointTemp; // �ִ��� ���Ͽ��� ���� �ڷ�
extern weatherData* windSpeed; // �ִ��� ���Ͽ��� ���� �ڷ�
extern weatherData* userET; // �ִ��� ���Ͽ��� ���� �ڷ�
extern weatherData* snowpackTemp; // �ִ��� ���Ͽ��� ���� �ڷ�


extern flowControlCellAndData fccds;
extern wpSimData wpSimValue;
extern string msgFileProcess;

extern cvAtt* cvsb; 
map<int, double> fcInflowSumPT_m3_Bak;// <idx, value> t-dt �ð��� ����� flow control inflow ��
wpSimData wpSimValueB; // t-dt �ð��� ����� wp���� ������ �پ��� ����.  prj.wps ���Ͽ��� ���� ������� 2�� ���� ����
flowControlCellAndData fccdsb; // t-dt ���� ����� gmp ���Ͽ��� ���� fc ������ �̿��ؼ� 2�� ���� ����

int startSimulation()
{
	initThisSimulation();
	setCVStartingCondition(0);
	int dtRF_sec = prj.rfinterval_min * 60;
	int iRF = 0;

	int dtTempMax_sec = prj.tempMaxInterval_min * 60;
	int iTempMax = 0;
	int dtTempMin_sec = prj.tempMinInterval_min * 60;
	int iTempMin = 0;
	int dtDTL_sec = prj.DTLDataInterval_min * 60; // daytime length
	int iDTL = 0; // daytime length
	int dtSolarRMin_sec = prj.solarRadInterval_min * 60;
	int iSolarR = 0;

	int dtDewPointTemp_sec = prj.dewPointTempInterval_min * 60;
	int iDewPointT = 0;
	int dtWindSpeed_sec = prj.windSpeedInterval_min * 60;
	int iWindSpeed = 0;
	int dtUserET_sec = prj.userETInterval_min * 60;
	int iUserET = 0;
	weatherDataType userWdType_apply;
	if (ts.wdUsed_userET == 1) {
		userWdType_apply = prj.userPETDataType;
	}
	else if (ts.wdUsed_userET == 2) {
		userWdType_apply = prj.userAETDataType;
	}

	int dtSnowPackTemp = prj.snowpackTempInterval_min * 60;
	int iSnowPackT = 0;

	int nowTsec = ts.dtsec;
	double nowTmin = 0.0;;

	int simEndingT_sec = ts.simEnding_sec;
	int loopEndingT_sec = ts.simEnding_sec + 1;// ts.dtsec + 1; ���⼭ +1�� �ϴ� ���� while ������ �ε�ȣ�� ����ϱ� ����
	int nowTday = 1;
	int nowTday_bak = 0;
	while (nowTsec < loopEndingT_sec) {
		// dtsec���� �����ؼ�, ù��° ���췹�̾ �̿��� ���ǰ���� 0�ð��� ����Ѵ�.
		nowTmin = nowTsec / 60.0;
		if (nowTmin > 1440 * nowTday) {
			nowTday++;
		}

		if (nowTsec > dtRF_sec * iRF) {
			if (iRF < ts.dataNumTotal_rf) {
				iRF++; // �̷��� �ϸ� ������ ���̾� ����
				if (setCVRF(iRF) == -1) { return -1; }    //isRFended = -1;
			}
			else {
				// ������ �ڷ�� ������, ������ 0���� �ϰ�, 
				// �ٸ� ����ڷ�� 0���� �� �� �����Ƿ�, ���ǱⰣ ��ü�� ���ؼ� �ڷᰡ �־�� �Ѵ�.
				//  ==> ����ڷ�� �̸� ���� ������ �ϰ� ���� ��������
				setCVRFintensityAndDTrf_Zero();
				iRF = loopEndingT_sec / dtRF_sec + 100;// ����� ū�� ����.   // INT_MAX ����ϸ�  dtRF_sec * orderRF ���� �ִ밪 �ʰ��ؼ� - ������ �����ȴ�.
			}
		}

		if (prj.simEvaportranspiration == 1 || prj.simSnowMelt == 1) { //|| prj.simInterception == 1) {
			// ����ڷ�� ��ü ���� ������ openProjectFile()���� �̹� �Ϸ������Ƿ�, ���⼭�� ������� �б⸸ �Ѵ�. 
			if (ts.wdUsed_tempMax == 1) {
				if (nowTsec > dtTempMax_sec * iTempMax) {
					iTempMax++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iTempMax, "max. temperature",
						prj.tempMaxDataType, tempMax, whatWeatherData::TempMax) == -1) {
					return -1;
					}
				}
			}
			if (ts.wdUsed_tempMin == 1) {
				if (nowTsec > dtTempMin_sec * iTempMin) {
					iTempMin++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iTempMin, "min. temperature",
						prj.tempMinDataType, tempMin, whatWeatherData::TempMin) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_DTL == 1) {
				if (nowTsec > dtDTL_sec * iDTL) {//�����ð�
					iDTL++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iDTL, "daytime length",
						prj.DTLDataType, dayTimeLength, whatWeatherData::DaytimeLength) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_solarR == 1) {
				if (nowTsec > dtSolarRMin_sec * iSolarR) {//�ϻ緮
					iSolarR++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iSolarR, "solar radiation",
						prj.solarRadDataType, solarRad, whatWeatherData::SolarRad) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_dewPointTemp == 1) {
				if (nowTsec > dtDewPointTemp_sec * iDewPointT) {//�ϻ緮
					iDewPointT++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iDewPointT, "dew point temperature",
						prj.dewPointTempDataType, dewPointTemp, whatWeatherData::DewPointTemp) == -1) {
					return -1;
					}
				}
			}
			if (ts.wdUsed_windSpeed == 1) {
				if (nowTsec > dtWindSpeed_sec * iWindSpeed) {//�ϻ緮
					iWindSpeed++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iWindSpeed, "wind speed",
						prj.windSpeedDataType, windSpeed, whatWeatherData::WindSpeed) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_userET >0) {
				if (nowTsec > dtUserET_sec * iUserET) {//�ϻ緮
					iUserET++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iUserET, "user defined evapotranspiration",
						userWdType_apply, userET, whatWeatherData::UserET) == -1) {
					return -1;
					}
				}
			}
			if (ts.wdUsed_snowPackTemp == 1) {
				if (nowTsec > dtSnowPackTemp * iSnowPackT) {
					iSnowPackT++; // �̷��� �ϸ� ������ ���̾� ����
					if (setCVweatherData(iSnowPackT, "snow pack temperature",
						prj.snowpackTempDataType, snowpackTemp, whatWeatherData::SnowpackTemp) == -1) {
					return -1;
					}
				}
			}

			if (nowTday_bak != nowTday) {
				string tElapsedStr = timeElaspedToDateTimeFormat2(prj.simStartTime,
					nowTsec, timeUnitToShow::toM,
					dateTimeFormat::yyyymmddHHMMSS);
				tm tCurDate = stringToDateTime(tElapsedStr, true);
				//ts.tCurYear = tCurDate.tm_year;
				ts.tCurMonth = tCurDate.tm_mon;
				ts.tCurDay = tCurDate.tm_mday;
				ts.tDayOfYear = tCurDate.tm_yday;
			}
		}

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
		nowTday_bak = nowTday;
	}

	if (ts.stopSim == 1) {
		writeLogString(fpnLog, "Simulation was stopped.\n ", 1, 1);
		return 1;
	}
	else {
		tm tnow = getCurrentTimeAsLocal_tm(); //MP ����
		std::tm tsTotalSim = timeDifferecceTM_DHMS(ts.time_thisSimStarted, tnow);

		printf("\rCurrent progress: 100.00%%... ");
		writeLogString(fpnLog, "Simulation was completed. Run time: "
			+ to_string(tsTotalSim.tm_hour) + "h "
			+ to_string(tsTotalSim.tm_min) + "m "
			+ to_string(tsTotalSim.tm_sec) + "s.\n", 1, 1);
		return 1;
	}
	return 1;
}

int simulateRunoff(double nowTmin)
{
    ts.vMaxInThisStep = 0.0;

	// ������� parallel =============================
    int numth = prj.mdp;
    double* uMax = new double[numth]();// �̷��� �ϸ� 0 ���� �ʱ�ȭ �ȴ�.
    for (int fac : fas) {
        int nCVs = faCount[fac];
#pragma omp parallel
        {
            // reduction���� max, min ã�� ���� openMP 3.1 �̻���� ����, 
            // �迭 ����ϴ� ���� critical ���� ������..
            int nth = omp_get_thread_num();
            //uMax[nth] = DBL_MIN;
#pragma omp for //  guided �Ⱦ��°� �� ������..
            for (int e = 0; e < nCVs; ++e) {
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

	//if (ts.vMaxInThisStep > GRAVITY_ACC) {
	//	ts.vMaxInThisStep = GRAVITY_ACC;
	//}
    delete[] uMax;
	// parallel =============================

	//// ������� serial===========
	//double uMax_s = 0.0;
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
	//if (uMax_s > GRAVITY_ACC) { uMax_s = GRAVITY_ACC; }
	//ts.vMaxInThisStep = uMax_s;
	//// serial===========

    return 1;
}


void simulateRunoffCore(int i, double nowTmin)
{
	int fac = cvs[i].fac;
	int dtsec = ts.dtsec;
	double cellsize = di.cellSize;
	updateCVbyHydroComps(i);
	if (prj.simFlowControl == 1 &&
		(cvs[i].fcType1 == flowControlType::ReservoirOutflow ||
			cvs[i].fcType1 == flowControlType::Inlet)) {
		calFCReservoirOutFlow(i, nowTmin);
	}
	else {
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
		// ���� �� �ϳ�.  flowControlType::SinkFlow, flowControlType::SourceFlow, 
		// flowControlType::ReservoirOperation, flowControlType::DetentionPond

		// 2023.05.31 �Ʒ��� ���� ����. Sink, source �ߺ������� CV ���� ����
		if (cvs[i].fcType1 == flowControlType::DetentionPond) {
			calDetentionPond(i, nowTmin);
		}
		if (cvs[i].fcType1 == flowControlType::SinkFlow
			|| cvs[i].fcType1 == flowControlType::SourceFlow) {
			calSinkOrSourceFlow(i, nowTmin, cvs[i].fcType1, 1);
		}
		if (cvs[i].fcType2 == flowControlType::SinkFlow
			|| cvs[i].fcType2 == flowControlType::SourceFlow) {
			calSinkOrSourceFlow(i, nowTmin, cvs[i].fcType2, 2);
		}
		if (cvs[i].fcType3 == flowControlType::SinkFlow
			|| cvs[i].fcType3 == flowControlType::SourceFlow) {
			calSinkOrSourceFlow(i, nowTmin, cvs[i].fcType3, 3);
		}
		if (cvs[i].fcType1 == flowControlType::ReservoirOperation
			&& prj.fcs[i][0].roType != reservoirOperationType::None) {
			// rotype�� ������, ro�� �Ѿ��.  
			// sink, source�� AutoROM�� ���� ������ �� �����Ƿ�, sinkflow, sourceflow������ ���� ����.
			calReservoirOperation(i, nowTmin);
		}
	}
}

void initThisSimulation()
{
	//if (prj.simType != simulationType::RealTime) {
	//	ts.dataNumTotal_rf = (int)rfs.size();
	//	ts.dnTotal_tempMax = (int)tempMax.size();
	//	ts.dnTotal_tempMin = (int)tempMin.size();
	//	ts.dnTotal_solarR = (int)solarRad.size();
	//	ts.dnTotal_DTL = (int)dayTimeLength.size();
	//	ts.dnTotal_snowPackTemp = (int)snowpackTemp.size();
	//	ts.dnTotal_dewPointTemp = (int)dewPointTemp.size();
	//	ts.dnTotal_windSpeed = (int)windSpeed.size();
	//	ts.dnTotal_userPET = (int)userPET.size();
	//}
	//else {
	//	ts.dataNumTotal_rf = 0;
	//	ts.dnTotal_tempMax = 0;
	//	ts.dnTotal_tempMin = 0;
	//	ts.dnTotal_solarR = 0;
	//	ts.dnTotal_DTL = 0;
	//	ts.dnTotal_snowPackTemp = 0;
	//	ts.dnTotal_dewPointTemp = 0;
	//	ts.dnTotal_windSpeed = 0;
	//	ts.dnTotal_userPET = 0;
	//}

	initRFvars();

    // �̷��� �ؾ� ���ǱⰣ�� �°� ����ȴ�. 
    //�ֳ��ϸ�, ù��° �����ڷḦ �̿��� ���� ����� 0 �ð����� ��µǱ� ������
	// �����ڷ�(������ ǥ�õǴ� ����ڷ�)�� �����ð������� ����, �����ڷ�� ���� �ð������� ������
	ts.simDuration_min = (int)prj.simDuration_hr * 60;//        +prj.dtPrint_min;
    ts.simEnding_sec = ts.simDuration_min * 60;
    ts.grmStarted = 1;
    ts.stopSim = -1;
    ts.vMaxInThisStep = 0.0;
    ts.cvsbT_sec = 0;
    ts.dtMinLimit_sec = 1;
    ts.zeroTimePrinted = -1;
    //dtMaxLimit�� ��½ð�����, ����, fc�� �ð������� ��� ����ؼ� �� ������ �Ѵ�.
    int dtFromP = (int)(prj.dtPrint_min * 60 / 2);
    int dtFromR = (int)(prj.rfinterval_min * 60 / 2);
    ts.dtMaxLimit_sec = min(dtFromP, dtFromR);
	if (prj.simFlowControl == 1) {
		bool appFCdt = false;
		int mindtFC = INT_MAX;
		for (int i : fccds.cvidxsFCcell) {
			if (prj.fcs[i][0].fcDT_min > 0 && mindtFC > prj.fcs[i][0].fcDT_min) {
				mindtFC = int(prj.fcs[i][0].fcDT_min); //  *60 / 2); // �켱 �ּ� �ð������� �޴� ������ ����. 2024.08.07
				appFCdt = true;
			}
		}

		if (appFCdt == true) {
			mindtFC = (int)(mindtFC * 60 / 2); // FC �ּ� �ð������� 1/2 �� ���⼭ ���. 2024.08.07
			if (ts.dtMaxLimit_sec > mindtFC) {
				ts.dtMaxLimit_sec = mindtFC;
			}
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

    ts.time_thisSimStarted = getCurrentTimeAsLocal_tm(); //MP ����

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
        cvs[i].rfiRead_After_iniLoss_mPsec = 0;
        cvs[i].rfEff_dt_m = 0;
        cvs[i].rfApp_mPdt = 0;
        cvs[i].rf_PDT_m = 0;
        cvs[i].rfAccRead_fromStart_mm = 0;
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
		cvs[i].DP_storageCumulative_m3 = 0.0;
		cvs[i].DP_inflow_m3Ps = 0.0;
		cvs[i].DP_outflow_m3Ps = 0.0;

		cvs[i].pet_PDT_m = 0.0;
		cvs[i].aet_PDT_m = 0.0;

        if (prj.simFlowControl == 1) {
            if (prj.fcs.size() > 0 && getVectorIndex(fccds.cvidxsFCcell, i) != -1) {
				//  �ϳ��� ���� sink, source�� �ߺ� ������ �� ������, �̶� sink, source�� �ļ��� ��, [i][1]�̴�. 
				double iniS = prj.fcs[i][0].iniStorage_m3; 
                if (iniS > 0) {
					if (prj.fcs[i][0].fcType == flowControlType::DetentionPond) {
						cvs[i].DP_storageCumulative_m3 = iniS;
					}
					else {
						cvs[i].storageCumulative_m3 = iniS;
					}

                }
            }
        }
    }

    for (int wpcvid : wpSimValue.wpCVidxes) {
        wpSimValue.prcpWPGridTotal_mm[wpcvid] = 0;
        wpSimValue.prcpUpWSAveTotal_mm[wpcvid] = 0;
		wpSimValue.Q_sumPdTforAVE_m3[wpcvid] = 0.0;
    }
}


int outputManager(int nowTsec)
{
    int dtP_min = prj.dtPrint_min;
	int dtP_SEC = dtP_min * 60;
    int timeToP_min = 0;// = nowTsec / 60;
	int timeToP_AVE_min = 0;
	int printAveValueNow = 0;
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
			ts.rfAveSumAllCells_PT_m_bak = ts.rfAveSumAllCells_PdT_m;
			ts.rfAveSumAllCells_PTave_m_bak = ts.rfAveSumAllCells_PdTave_m;

            if (prj.simFlowControl == 1) {
				fcInflowSumPT_m3_Bak = fccds.inflowSumPdT_m3;
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
	return 1;
}

int  writeBySimType(int nowTP_min,
	double cinterp, int writeAVE, int timeToP_AVE_min) {
	double TS_FromStarting_sec = 0.0;
	double TS_FromStarting_min = 0.0;
	string tStrToPrint;
	string tStrToPrintAve;
	tm tnow = getCurrentTimeAsLocal_tm(); //MP ����
	TS_FromStarting_sec = timeDifferecceSEC(ts.time_thisSimStarted, tnow);
	TS_FromStarting_min = TS_FromStarting_sec / 60.0;
	string unitToP = "";
	if (prj.isDateTimeFormat == 1) {
		timeUnitToShow tUnit = timeUnitToShow::toM; // default�� �д��� ����
		if (prj.dtPrint_min % 1440 == 0) {// �ϴ����� ����� ���
			tUnit = timeUnitToShow::toDay;
		}
		if (prj.simType == simulationType::Normal) {
			tStrToPrint = timeElaspedToDateTimeFormat2(prj.simStartTime,
				nowTP_min * 60, tUnit,
				dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);

			if (writeAVE == 1) {
				tStrToPrintAve = timeElaspedToDateTimeFormat2(prj.simStartTime,
					timeToP_AVE_min * 60, tUnit,
					dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
			}
		}
		else {// real time �� ���
			tStrToPrint = timeElaspedToDateTimeFormat(prj.simStartTime,
				nowTP_min * 60, tUnit,
				dateTimeFormat::yyyymmddHHMMSS);
			if (writeAVE == 1) {
				tStrToPrintAve = timeElaspedToDateTimeFormat(prj.simStartTime,
					timeToP_AVE_min * 60, tUnit,
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
			+ dtos(TS_FromStarting_min, 2) + "min) \n";
		writeLogString(fpnLog, logStr, 1, -1);
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
#ifdef _WIN32
		writeRealTimeSimResults(tStrToPrint, cinterp, TS_FromStarting_sec);
#else
		writeLogString(fpnLog, "ERROR : Real time simulation is not supported in the GRM model for Linux. \n", 1, 1);
#endif
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
	ts.rfAveSumAllCells_PdT_m = 0.0;
	if (printAveValueNow == 1) {
		ts.rfAveSumAllCells_PdTave_m = 0.0;
	}
	for (int idx : wpSimValue.wpCVidxes) {
		wpSimValue.prcpUpWSAveForPT_mm[idx] = 0.0;
		wpSimValue.prcpWPGridForPdT_mm[idx] = 0.0;
		wpSimValue.pet_grid_sumPdT_mm[idx] = 0.0;
		wpSimValue.aet_grid_sumPdT_mm[idx] = 0.0;
		wpSimValue.snowM_grid_sumPdT_mm[idx] = 0.0;
		if (printAveValueNow == 1 ) {
			wpSimValue.Q_sumPdTforAVE_m3[idx] = 0.0;
		}
	}
	if (printAveValueNow == 1 ) {
		for (int idx : fccds.cvidxsFCcell) {
			if (prj.fcs[idx][0].fcType != flowControlType::Inlet) {
				fccds.inflowSumPdT_m3[idx] = 0.0;
			}
		}
		fcInflowSumPT_m3_Bak.clear();
	}
}


void calValuesDuringPT(int dtsec)
{
	ts.rfAveForDT_m = ts.rfiSumAllCellsInCurRFData_mPs * dtsec
		/ di.cellNtobeSimulated;
	ts.rfAveSumAllCells_PdT_m = ts.rfAveSumAllCells_PdT_m
		+ ts.rfAveForDT_m;
	if (prj.printAveValue == 1) {
		ts.rfAveSumAllCells_PdTave_m = ts.rfAveSumAllCells_PdTave_m
			+ ts.rfAveForDT_m;
	}

	for (int idx : wpSimValue.wpCVidxes) {
		wpSimValue.prcpUpWSAveForDt_mm[idx] = wpSimValue.prcpiReadSumUpWS_mPs[idx]
			* dtsec * 1000.0 / (double)wpSimValue.cvCountAllup[idx];
		wpSimValue.prcpUpWSAveForPT_mm[idx] = wpSimValue.prcpUpWSAveForPT_mm[idx]
			+ wpSimValue.prcpUpWSAveForDt_mm[idx];
		wpSimValue.prcpWPGridForPdT_mm[idx] = wpSimValue.prcpWPGridForPdT_mm[idx]
			+ cvs[idx].rfiRead_mPsec * 1000.0 * dtsec;
		wpSimValue.pet_grid_sumPdT_mm[idx] += cvs[idx].pet_mPdt * 1000.0;
		wpSimValue.aet_grid_sumPdT_mm[idx] += cvs[idx].aet_mPdt * 1000.0;
		wpSimValue.snowM_grid_sumPdT_mm[idx] += cvs[idx].smelt_mPdt * 1000.0;

		if (prj.printAveValue == 1) {
			if (cvs[idx].flowType == cellFlowType::OverlandFlow) {
				wpSimValue.Q_sumPdTforAVE_m3[idx] += cvs[idx].QOF_m3Ps * dtsec;
			}
			else {
				wpSimValue.Q_sumPdTforAVE_m3[idx] += cvs[idx].stream.QCH_m3Ps * dtsec;
			}
		}
	}

	if (prj.printAveValue == 1) {
		for (int idx : fccds.cvidxsFCcell) {
			if (prj.fcs[idx][0].fcType != flowControlType::Inlet) {
				// inlet �� �ƴ� ��� fc ���� ���ؼ� 	��±Ⱓ������  inflowSumPT_m3 ���
				fccds.inflowSumPdT_m3[idx] += cvs[idx].QsumCVw_m3Ps * dtsec;
			}
		}
	}
}

