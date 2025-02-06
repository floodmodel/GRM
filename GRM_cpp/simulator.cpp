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
extern map<int, int*> cvaisToFA; //fa별 cv array idex 목록
extern vector<int> fas;
extern map<int, int> faCount;
//extern vector<weatherData> rfs;
//extern vector<weatherData> tempMax;
//extern vector<weatherData> tempMin;
//extern vector<weatherData> solarRad;
//extern vector<weatherData> dayTimeLength;
//extern vector<weatherData> dewPointTemp;// 이슬점 온도 파일에서 읽은 자료
//extern vector<weatherData> windSpeed;// 풍속 파일에서 읽은 자료
//extern vector<weatherData> userPET;// 사용자 입력 잠재증발산량
//extern vector<weatherData> snowpackTemp;

extern weatherData* rfs;
extern weatherData* tempMax;
extern weatherData* tempMin; // 최대기온 파일에서 읽은 자료
extern weatherData* dayTimeLength; // 최대기온 파일에서 읽은 자료
extern weatherData* solarRad; // 최대기온 파일에서 읽은 자료
extern weatherData* dewPointTemp; // 최대기온 파일에서 읽은 자료
extern weatherData* windSpeed; // 최대기온 파일에서 읽은 자료
extern weatherData* userET; // 최대기온 파일에서 읽은 자료
extern weatherData* snowpackTemp; // 최대기온 파일에서 읽은 자료


extern flowControlCellAndData fccds;
extern wpSimData wpSimValue;
extern string msgFileProcess;

extern cvAtt* cvsb; 
map<int, double> fcInflowSumPT_m3_Bak;// <idx, value> t-dt 시간에 적용된 flow control inflow 합
wpSimData wpSimValueB; // t-dt 시간에 저장된 wp별로 저장할 다양한 정보.  prj.wps 파일에서 읽은 순서대로 2차 정보 저장
flowControlCellAndData fccdsb; // t-dt 에서 저장된 gmp 파일에서 읽은 fc 정보를 이용해서 2차 정보 저장

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
	int loopEndingT_sec = ts.simEnding_sec + 1;// ts.dtsec + 1; 여기서 +1을 하는 것은 while 문에서 부등호를 사용하기 위함
	int nowTday = 1;
	int nowTday_bak = 0;
	while (nowTsec < loopEndingT_sec) {
		// dtsec부터 시작해서, 첫번째 강우레이어를 이용한 모의결과를 0시간에 출력한다.
		nowTmin = nowTsec / 60.0;
		if (nowTmin > 1440 * nowTday) {
			nowTday++;
		}

		if (nowTsec > dtRF_sec * iRF) {
			if (iRF < ts.dataNumTotal_rf) {
				iRF++; // 이렇게 하면 마지막 레이어 적용
				if (setCVRF(iRF) == -1) { return -1; }    //isRFended = -1;
			}
			else {
				// 강수량 자료는 없으면, 강수량 0으로 하고, 
				// 다른 기상자료는 0으로 할 수 없으므로, 모의기간 전체에 대해서 자료가 있어야 한다.
				//  ==> 기상자료는 미리 개수 검증을 하고 모의 시작하자
				setCVRFintensityAndDTrf_Zero();
				iRF = loopEndingT_sec / dtRF_sec + 100;// 충분히 큰값 설정.   // INT_MAX 사용하면  dtRF_sec * orderRF 에서 최대값 초과해서 - 값으로 설정된다.
			}
		}

		if (prj.simEvaportranspiration == 1 || prj.simSnowMelt == 1) { //|| prj.simInterception == 1) {
			// 기상자료는 전체 개수 검증을 openProjectFile()에서 이미 완료했으므로, 여기서는 순서대로 읽기만 한다. 
			if (ts.wdUsed_tempMax == 1) {
				if (nowTsec > dtTempMax_sec * iTempMax) {
					iTempMax++; // 이렇게 하면 마지막 레이어 적용
					if (setCVweatherData(iTempMax, "max. temperature",
						prj.tempMaxDataType, tempMax, whatWeatherData::TempMax) == -1) {
					return -1;
					}
				}
			}
			if (ts.wdUsed_tempMin == 1) {
				if (nowTsec > dtTempMin_sec * iTempMin) {
					iTempMin++; // 이렇게 하면 마지막 레이어 적용
					if (setCVweatherData(iTempMin, "min. temperature",
						prj.tempMinDataType, tempMin, whatWeatherData::TempMin) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_DTL == 1) {
				if (nowTsec > dtDTL_sec * iDTL) {//일조시간
					iDTL++; // 이렇게 하면 마지막 레이어 적용
					if (setCVweatherData(iDTL, "daytime length",
						prj.DTLDataType, dayTimeLength, whatWeatherData::DaytimeLength) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_solarR == 1) {
				if (nowTsec > dtSolarRMin_sec * iSolarR) {//일사량
					iSolarR++; // 이렇게 하면 마지막 레이어 적용
					if (setCVweatherData(iSolarR, "solar radiation",
						prj.solarRadDataType, solarRad, whatWeatherData::SolarRad) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_dewPointTemp == 1) {
				if (nowTsec > dtDewPointTemp_sec * iDewPointT) {//일사량
					iDewPointT++; // 이렇게 하면 마지막 레이어 적용
					if (setCVweatherData(iDewPointT, "dew point temperature",
						prj.dewPointTempDataType, dewPointTemp, whatWeatherData::DewPointTemp) == -1) {
					return -1;
					}
				}
			}
			if (ts.wdUsed_windSpeed == 1) {
				if (nowTsec > dtWindSpeed_sec * iWindSpeed) {//일사량
					iWindSpeed++; // 이렇게 하면 마지막 레이어 적용
					if (setCVweatherData(iWindSpeed, "wind speed",
						prj.windSpeedDataType, windSpeed, whatWeatherData::WindSpeed) == -1) {
						return -1;
					}
				}
			}
			if (ts.wdUsed_userET >0) {
				if (nowTsec > dtUserET_sec * iUserET) {//일사량
					iUserET++; // 이렇게 하면 마지막 레이어 적용
					if (setCVweatherData(iUserET, "user defined evapotranspiration",
						userWdType_apply, userET, whatWeatherData::UserET) == -1) {
					return -1;
					}
				}
			}
			if (ts.wdUsed_snowPackTemp == 1) {
				if (nowTsec > dtSnowPackTemp * iSnowPackT) {
					iSnowPackT++; // 이렇게 하면 마지막 레이어 적용
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
			nowTsec = simEndingT_sec; // 이렇게 하면 ts.simEnding_sec 와 같아진다.
			ts.dtsecUsed_tm1 = ts.dtsec;
		}
		else {
			nowTsec = nowTsec + ts.dtsec; // dtsec 만큼 전진
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
		tm tnow = getCurrentTimeAsLocal_tm(); //MP 수정
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

	// 여기부터 parallel =============================
    int numth = prj.mdp;
    double* uMax = new double[numth]();// 이렇게 하면 0 으로 초기화 된다.
    for (int fac : fas) {
        int nCVs = faCount[fac];
#pragma omp parallel
        {
            // reduction으로 max, min 찾는 것은 openMP 3.1 이상부터 가능, 
            // 배열 사용하는 것이 critical 보다 빠르다..
            int nth = omp_get_thread_num();
            //uMax[nth] = DBL_MIN;
#pragma omp for //  guided 안쓰는게 더 빠르다..
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

	//// 여기부터 serial===========
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
		// 다음 중 하나.  flowControlType::SinkFlow, flowControlType::SourceFlow, 
		// flowControlType::ReservoirOperation, flowControlType::DetentionPond

		// 2023.05.31 아래와 같이 수정. Sink, source 중복설정된 CV 모의 가능
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
			// rotype이 있으면, ro로 넘어간다.  
			// sink, source는 AutoROM과 같이 설정될 수 있으므로, sinkflow, sourceflow에서도 여기 들어간다.
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

    // 이렇게 해야 모의기간에 맞게 실행된다. 
    //왜냐하면, 첫번째 강우자료를 이용한 모의 결과가 0 시간으로 출력되기 때문에
	// 강우자료(양으로 표시되는 기상자료)는 이전시간까지의 누적, 유량자료는 현재 시간에서의 계측값
	ts.simDuration_min = (int)prj.simDuration_hr * 60;//        +prj.dtPrint_min;
    ts.simEnding_sec = ts.simDuration_min * 60;
    ts.grmStarted = 1;
    ts.stopSim = -1;
    ts.vMaxInThisStep = 0.0;
    ts.cvsbT_sec = 0;
    ts.dtMinLimit_sec = 1;
    ts.zeroTimePrinted = -1;
    //dtMaxLimit은 출력시간간격, 강우, fc의 시간간격을 모두 고려해서 그 반으로 한다.
    int dtFromP = (int)(prj.dtPrint_min * 60 / 2);
    int dtFromR = (int)(prj.rfinterval_min * 60 / 2);
    ts.dtMaxLimit_sec = min(dtFromP, dtFromR);
	if (prj.simFlowControl == 1) {
		bool appFCdt = false;
		int mindtFC = INT_MAX;
		for (int i : fccds.cvidxsFCcell) {
			if (prj.fcs[i][0].fcDT_min > 0 && mindtFC > prj.fcs[i][0].fcDT_min) {
				mindtFC = int(prj.fcs[i][0].fcDT_min); //  *60 / 2); // 우선 최소 시간간격을 받는 것으로 수정. 2024.08.07
				appFCdt = true;
			}
		}

		if (appFCdt == true) {
			mindtFC = (int)(mindtFC * 60 / 2); // FC 최소 시간간격의 1/2 은 여기서 계산. 2024.08.07
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

    ts.time_thisSimStarted = getCurrentTimeAsLocal_tm(); //MP 수정

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

			if (cvs[i].stream.cellValue > prj.swps[wsid].dryStreamOrder) {// 건천이 아니면,

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
						double sngCAS_ini = qChCVini / (double)cvs[i].cvdx_m; // 초기값 설정
						chCSAini = getChCSAusingQbyiteration(cvs[i], sngCAS_ini, qChCVini);
						hChCVini = getChDepthUsingCSA(cvs[i].stream.chBaseWidth, chCSAini,
							cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
							cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight,
							cvs[i].stream.bankCoeff);
						double csPeri = cvs[i].stream.chBaseWidth + hChCVini * 2;
						double HRch = chCSAini / csPeri; // 초기값은 윤변으로 하폭과 수심을 이용한다.
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
			else { // 건천이면 2023.04.12
				cvs[i].stream.hCH = 0.0;
				cvs[i].stream.csaCH = 0.0;
				cvs[i].stream.hCH_ori = 0.0;
				cvs[i].stream.csaCH_ori = 0.0;
				cvs[i].stream.QCH_m3Ps = 0.0;
				cvs[i].stream.uCH = 0.0;
			}

            if (prj.simBaseFlow == 1) {
                cvs[i].hUAQfromChannelBed_m = hChCVini; // 하도의 초기 수심을 비피압대수층의 초기 수심으로 설정 
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
				//  하나의 셀에 sink, source가 중복 설정될 수 있으며, 이때 sink, source는 후순위 즉, [i][1]이다. 
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
	if (nowTsec % dtP_SEC == 0) { // 여기서 출력
		timeToP_min = ts.targetTtoP_sec / 60 - dtP_min; // 이렇게 해야 첫번째 모의 결과가 0시간에 출력된다.
		if (prj.printAveValue == 1) {
			if (nowTsec % prj.dtPrintAveValue_sec == 0) { // 여기서 출력
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
		// 여기서 백업
        if (nowTsec <  ts.targetTtoP_sec
            && (nowTsec + ts.dtsec) >ts.targetTtoP_sec) {

            // 만일 현재의 dtsec으로 한번더 전진해서 이 조건을 만족하면
            std::copy(cvs, cvs + di.cellNnotNull, cvsb);
            wpSimValueB = wpSimValue; // 포인터가 아니므로..
			ts.rfAveSumAllCells_PT_m_bak = ts.rfAveSumAllCells_PdT_m;
			ts.rfAveSumAllCells_PTave_m_bak = ts.rfAveSumAllCells_PdTave_m;

            if (prj.simFlowControl == 1) {
				fcInflowSumPT_m3_Bak = fccds.inflowSumPdT_m3;
            }
            ts.cvsbT_sec = nowTsec;
            ts.isbak = 1;
            return 1; //벡업만 받고 나간다.
        }
		// 여기서 출력
        if (nowTsec > ts.targetTtoP_sec
            && (nowTsec - ts.dtsecUsed_tm1) <= ts.targetTtoP_sec) {
            double citerp=0.0;
            citerp = (ts.targetTtoP_sec - ts.cvsbT_sec) / (double)(nowTsec - ts.cvsbT_sec);
            timeToP_min = (int)(ts.targetTtoP_sec / 60) - dtP_min; // 이렇게 해야 첫번째 모의 결과가 0시간에 출력된다.
			
			if (prj.printAveValue == 1) {
				if (nowTsec >= ts.targetTtoP_AVE_sec && nowTsec <= ts.TtoP_ave_check_sec) { // ==인 경우는 없을 것으로 예상. 출력 시간각격 마다 체크하므로, 시간 부하는 크지 않을 듯. 
					timeToP_AVE_min = (int)(ts.targetTtoP_AVE_sec / 60) - prj.dtPrintAveValue_min; // 이렇게 해야 첫번째 모의 결과가 0시간에 출력된다.
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
	tm tnow = getCurrentTimeAsLocal_tm(); //MP 수정
	TS_FromStarting_sec = timeDifferecceSEC(ts.time_thisSimStarted, tnow);
	TS_FromStarting_min = TS_FromStarting_sec / 60.0;
	string unitToP = "";
	if (prj.isDateTimeFormat == 1) {
		timeUnitToShow tUnit = timeUnitToShow::toM; // default는 분단위 까지
		if (prj.dtPrint_min % 1440 == 0) {// 일단위로 출력할 경우
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
		else {// real time 인 경우
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
		// 아래는 Q 만 출력 =======================
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
		// 클래스나 전역 변수에 저장해서, 외부로 노출하고, analyzer에서 사용하는 방법은?
		//모의 종료시 혹은 1초마다 챠트 업데이트하는 방법?
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

// 여기서 출력할 때 마다 초기화 되어야 하는 변수들 처리
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
				// inlet 이 아닌 모든 fc 셀에 대해서 	출력기간에서의  inflowSumPT_m3 계산
				fccds.inflowSumPdT_m3[idx] += cvs[idx].QsumCVw_m3Ps * dtsec;
			}
		}
	}
}

