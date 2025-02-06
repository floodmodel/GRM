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
extern vector<weatherData> rfs;
extern flowControlCellAndData fccds;
extern wpSimData wpSimValue;
extern cvAtt* cvsb;

extern thisSimulationRT tsrt;
extern realTimeEnvFile rtef;

/// 현재는 강우-유출 사상만 실시간 모의가 가능하다. 연속형 모의는 실시간 자료 처리 적용 안되어 있음. 2023.03.06
int startSimulationRT()
{ 
    initThisSimulation();
    setCVStartingCondition(0);
    int dtRF_sec = prj.rfinterval_min * 60;
    //int rfOrder = 0;
    int nowTsec = ts.dtsec;
    double nowTmin;
    ts.dtsecUsed_tm1 = ts.dtsec;
    ts.targetTtoP_sec = (int)prj.dtPrint_min * 60;
    int endingT_sec = ts.simEnding_sec + ts.dtsec + 1;
    int rfOrder = 0;
    while (nowTsec < endingT_sec) {
        // dtsec부터 시작해서, 첫번째 강우레이어를 이용한 모의결과를 0시간에 출력한다.
        if (rfOrder == 0 || nowTsec > dtRF_sec * ts.dataNumTotal_rf) {
            string targetRFTime;
            int rft_sec = rfOrder * dtRF_sec;
            targetRFTime = timeElaspedToDateTimeFormat(prj.simStartTime, rft_sec,
                timeUnitToShow::toM, dateTimeFormat::yyyymmddHHMMSS);
            while (ts.stopSim == -1) {
                updateRFdataGRMRT(targetRFTime);
                if (rfOrder < ts.dataNumTotal_rf) { break; }
                Sleep(2000);// 2초 지연 적절함
            }
            if (ts.stopSim == 1) {
                writeLogString(fpnLog, "Real time simulation was stopped.\n", 1, 1);
                return 1;
            }
            rfOrder = rfOrder + 1; // 이렇게 하면 마지막 레이어 적용
            setCVRF(rfOrder);
        }
        if (prj.simFlowControl == 1) {// 신규 fc 자료 검색 조건
            //string targetDataTime_Previous = "";
            for (int idx : fccds.cvidxsFCcell) {
                flowControlinfo afc = prj.fcs[idx][0];
                if (afc.fcType == flowControlType::ReservoirOutflow) {
                    int dt_min = afc.fcDT_min;
                    if (nowTsec > dt_min * 60 * fccds.curDorder[idx]
                        || fccds.curDorder[idx] == 0) {
                        string targetDataTime;
                        int fcdt_sec = fccds.curDorder[idx] * dt_min * 60;
                        targetDataTime = timeElaspedToDateTimeFormat(prj.simStartTime, fcdt_sec,
                            timeUnitToShow::toM, dateTimeFormat::yyyymmddHHMMSS);
                        while (ts.stopSim == -1) {
                            if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
                                if (readDBandFillFCdataForRealTime(targetDataTime) == -1) {
                                    return -1;
                                }
                            }
                            else {
                                if (readCSVandFillFCdataForRealTime(rtef.fpnRTFCdata,
                                    targetDataTime) == -1) {
                                    return -1;
                                }
                            }
                            //targetDataTime_Previous = targetDataTime;
                            updateFcDataStatusForEachFCcellGRMRT(targetDataTime, idx);//, mRealTime.mdicFCDataOrder[cvid], dt_MIN);
                            if (tsrt.newFcDataAddedRT[idx] == 1) { break; }//2019.10.01. 최. prediction 관련
                            if (ts.enforceFCautoROM == 1 && tsrt.newFcDataAddedRT[idx] == -1)//2019.10.01. 최. prediction 관련
                            {// 2019.10.01. 최. prediction 관련. 한번 여기 지나가면, 
                            // 데이터로 모델링 하던 것은 모두 ReservoirOperation,  AutoROM으로 변경됨
                             // fc 데이터가 있는 것 까지는 데이터 이용, 없으면, autoROM
                                convertFCtoAutoROM(targetDataTime, idx,0);
                                break;
                            }
                            Sleep(2000);
                        }
                        fccds.curDorder[idx] += 1;
                        if (ts.stopSim == 1) {
                            writeLogString(fpnLog, "Real time simulation was stopped.\n", 1, 1);
                            return 1;
                        }
                    }
                }
            }
        }
        nowTmin = nowTsec / 60.0;
		if (prj.isDateTimeFormat == 1) {
			string tElapsedStr = timeElaspedToDateTimeFormat2(prj.simStartTime,
				nowTsec, timeUnitToShow::toM,
				dateTimeFormat::yyyymmddHHMMSS);
			tm tCurDate = stringToDateTime(tElapsedStr, true);
			ts.tCurMonth = tCurDate.tm_mon;
			ts.tCurDay = tCurDate.tm_mday;
		}
        if (simulateRunoff(nowTmin) == -1) { return -1; }
        calValuesDuringPT(ts.dtsec);
		outputManager(nowTsec);
        if (nowTsec + ts.dtsec > endingT_sec) {
            ts.dtsec = nowTsec + ts.dtsec - endingT_sec;
            nowTsec = endingT_sec;
            ts.dtsecUsed_tm1 = ts.dtsec;
        }
        else {
            nowTsec = nowTsec + ts.dtsec; // dtsec 만큼 전진
            ts.dtsecUsed_tm1 = ts.dtsec;
            if (prj.isFixedTimeStep == -1) {
                ts.dtsec = getDTsec(di.cellSize, 
					ts.vMaxInThisStep, ts.dtMaxLimit_sec,
                    ts.dtMinLimit_sec);
            }
        }
        if (ts.stopSim == 1) { break; }
    }
    if (ts.stopSim == 1) {
        writeLogString(fpnLog, "Real time simulation was stopped.\n", 1, 1);
        return 1;
    }
    else {
        tm tnow = getCurrentTimeAsLocal_tm(); //MP 수정
        std::tm tsTotalSim = timeDifferecceTM_DHMS(ts.time_thisSimStarted, tnow);
        writeLogString(fpnLog, "Simulation was completed. Run time: "
            + to_string(tsTotalSim.tm_hour) + "h "
            + to_string(tsTotalSim.tm_min) + "m "
            + to_string(tsTotalSim.tm_sec) + "s.\n", 1, 1);

        return 1;
    }
    return 1;
}

