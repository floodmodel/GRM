#include "stdafx.h"
#include <omp.h>
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

extern thisSimulationRT tsrt;
extern realTimeEnvFile rtef;

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
        //dtsec = ts.dtsec;        
        // dtsec���� �����ؼ�, ù��° ���췹�̾ �̿��� ���ǰ���� 0�ð��� ����Ѵ�.
        if (rfOrder == 0 || nowTsec > dtRF_sec * ts.rfDataCountTotal) {
            string targetRFTime;
            int rft_sec = rfOrder * dtRF_sec;
            targetRFTime = timeElaspedToDateTimeFormat(prj.simStartTime, rft_sec,
                timeUnitToShow::toM, dateTimeFormat::yyyymmddHHMMSS);
            while (ts.stopSim == -1) {
                updateRFdataGRMRT(targetRFTime);
                if (rfOrder < ts.rfDataCountTotal) { break; }
                Sleep(2000);// 2�� ���� ������
            }
            if (ts.stopSim == 1) {
                writeLog(fpnLog, "Real time simulation was stopped.\n", 1, 1);
                return 1;
            }
            rfOrder = rfOrder + 1; // �̷��� �ϸ� ������ ���̾� ����
            setCVRF(rfOrder);
        }
        if (prj.simFlowControl == 1) {// �ű� fc �ڷ� �˻� ����
            //string targetDataTime_Previous = "";
            for (int idx : fccds.cvidxsFCcell) {
                flowControlinfo afc = prj.fcs[idx];
                if (afc.fcType != flowControlType::ReservoirOperation) {
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
                            if (tsrt.newFcDataAddedRT[idx] == 1) { break; }//2019.10.01. ��. prediction ����
                            if (ts.enforceFCautoROM == 1 && tsrt.newFcDataAddedRT[idx] == -1)//2019.10.01. ��. prediction ����
                            {// 2019.10.01. ��. prediction ����. �ѹ� ���� ��������, 
                            // �����ͷ� �𵨸� �ϴ� ���� ��� ReservoirOperation,  AutoROM���� �����
                             // fc �����Ͱ� �ִ� �� ������ ������ �̿�, ������, autoROM
                                convertFCtypeToAutoROM(targetDataTime, idx);
                                break;
                            }
                            Sleep(2000);
                        }
                        fccds.curDorder[idx] += 1;
                        if (ts.stopSim == 1) {
                            writeLog(fpnLog, "Real time simulation was stopped.\n", 1, 1);
                            return 1;
                        }
                    }
                }
            }
        }
        nowTmin = nowTsec / 60.0;
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
                ts.dtsec = getDTsec(di.cellSize, 
					ts.vMaxInThisStep, ts.dtMaxLimit_sec,
                    ts.dtMinLimit_sec);
            }
        }
        if (ts.stopSim == 1) { break; }
    }
    if (ts.stopSim == 1) {
        writeLog(fpnLog, "Real time simulation was stopped.\n", 1, 1);
        return 1;
    }
    else {
        COleDateTime  timeNow = COleDateTime::GetCurrentTime();
        COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
        writeLog(fpnLog, "Simulation was completed. Run time: "
            + to_string(tsTotalSim.GetHours()) + "hrs "
            + to_string(tsTotalSim.GetMinutes()) + "min "
            + to_string(tsTotalSim.GetSeconds()) + "sec.\n", 1, 1);
        return 1;
    }
    return 1;
}

