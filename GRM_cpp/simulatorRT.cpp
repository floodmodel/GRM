
#include <omp.h>
#include <string>
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

map<int, double> fcdAb;// <idx, value> t-dt �ð��� ����� flow control data ��
wpinfo wpisb;

int startSimulationRT()
{
    initThisSimulation();
    setCVStartingCondition(0);
    int dtRF_sec = prj.rfinterval_min * 60;
    int rfOrder = 0;
    int nowTsec = ts.dtsec;
    double nowTmin;
    ts.dtsecUsed_tm1 = ts.dtsec;
    ts.targetTtoP_sec = (int)prj.dtPrint_min * 60;
    int endingT_sec = ts.simEnding_sec + ts.dtsec + 1;
    int curRForder = 0;
    while (nowTsec < endingT_sec) {
        //dtsec = ts.dtsec;        
        // dtsec���� �����ؼ�, ù��° ���췹�̾ �̿��� ���ǰ���� 0�ð��� ����Ѵ�.
        if (curRForder==0|| nowTsec > dtRF_sec * ts.rfDataCountTotal) {
            string targetRFLayerTime;
            int et_sec = curRForder * dtRF_sec;
            targetRFLayerTime = timeElaspedToDateTimeFormat(prj.simStartTime, et_sec, 
                timeUnitToShow::toMinute, dateTimeFormat::yyyymmddHHMMSS);
            do
            {
                if (mStop == true) { return; }
                mRealTime.UpdateRainfallInformationGRMRT(targetRFLayerTime);
                if (curRForder < tsrt.mRFDataCountInThisEvent) { break; }
                Thread.Sleep(2000);
            } while (true);  // 2�� ���� ������
                curRForder = curRForder + 1; // �̷��� �ϸ� ������ ���̾� ����
            cRainfall.ReadRainfall(mProject, mRealTime.mRainfallDataTypeRT, mRealTime.mlstRFdataRT, System.Convert.ToInt32(mProject.rainfall.mRainfallinterval), nowRFLayerOrder, sThisSimulation.IsParallel);
            bRainfallisEnded = false;
        }
        nowTmin = nowTsec / 60.0;
        if (simulateRunoff(nowTmin) == -1) { return -1; }
        calCumulRFduringDTP(ts.dtsec);
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
                ts.dtsec = getDTsec(CONST_CFL_NUMBER,
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
            + to_string(tsTotalSim.GetHours()) + "hrs "
            + to_string(tsTotalSim.GetMinutes()) + "min "
            + to_string(tsTotalSim.GetSeconds()) + "sec.\n", 1, 1);
        return 1;
    }
    return 1;
}

