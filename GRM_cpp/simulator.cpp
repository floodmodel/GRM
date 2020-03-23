
#include <omp.h>
#include "gentle.h"
#include "grm.h"

using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern thisSimulation ts;

extern domaininfo di;
extern cvAtt* cvs;
extern map<int, vector<int>> cvaisToFA; //fa별 cv array idex 목록
extern vector<rainfallData> rfs;
extern flowControlCellAndData fccds;
extern wpinfo wpis;

int startSimulationSingleEvent()
{
    initThisSimulation();
    setCVStartingCondition(0);
    int dtPrint_min = prj.printTimeStep_min;
    int dtRF_sec = prj.rfinterval_min * 60;
    int dtRFinterval_min = prj.rfinterval_min;
    //int isRFended = -1;
    int tsec_tm1 = 0;
    cvAtt Project_tm1;
    int targetTtoPrint_min = 0;
    int nowRFOrder = 0;
    int nowTsec = ts.dtsec;
    double nowTmin;
    ts.dtsec_usedtoForwardToThisTime = ts.dtsec;
    ts.zeroTimePrinted = -1;
    int dtsec;
    while (nowTsec <= ts.time_simEnding_sec) {// simulationTimeLimitSEC
        dtsec = ts.dtsec;
        ts.vMaxInThisStep = DBL_MIN;
        // dtsec부터 시작해서, 첫번째 강우레이어를 이용한 모의결과를 0시간에 출력한다.
        /*if (isRFended == -1 && (nowRFOrder == 0
            || (nowTsec > dtRF_sec* nowRFOrder))) {*/
        if (nowTsec > dtRF_sec* nowRFOrder ) {
            if (nowRFOrder < ts.rfDataCountInThisEvent) {
                nowRFOrder++; // 이렇게 하면 마지막 레이어 적용
                if (setCVRF(nowRFOrder) == -1) { return -1; }
                //isRFended = -1;
            }
            else {
                setRFintensityAndDTrf_Zero();
                nowRFOrder = INT_MAX;
                //isRFended = 1;
            }
        }
        nowTmin = nowTsec / 60.0;
        if (simulateRunoff(nowTmin) == -1) { return -1; }
        calCumulativeRFDuringDTPrintOut(dtsec);
        WriteCurrentResultAndInitializeNextStep(mProject, nowTsec, dtsec, dTRFintervalSEC, dTPrint_MIN, wpCount,
            ref targetCalTtoPrint_MIN, ref mSEC_tm1, ref Project_tm1, mProject.mSimulationType, nowRFOrder);

        if (nowTsec < ts.time_simEnding_sec && nowTsec + dtsec > ts.time_simEnding_sec) {
            ts.dtsec = nowTsec + dtsec - ts.time_simEnding_sec;
            nowTsec = ts.time_simEnding_sec;
            ts.dtsec_usedtoForwardToThisTime = ts.dtsec;
        }
        else {
            nowTsec = nowTsec + dtsec; // dtsec 만큼 전진
            ts.dtsec_usedtoForwardToThisTime = ts.dtsec;
            if (prj.IsFixedTimeStep == -1)
            {
                ts.dtsec = getDTsec(CONST_CFL_NUMBER,
                    di.cellSize, ts.vMaxInThisStep, ts.dtMaxLimit_sec, ts.dtMinLimit_sec);
            }
        }
        if (ts.stopSim == 1) { break; }
    }
    if (ts.stopSim == 1) {
        writeLog(fpnLog, "Simulation was stopped.\n", 1, 1);
        return 1;
    }
    else {
        writeLog(fpnLog, "Simulation was completed.\n", 1, 1);
        return 1;
    }
    return 1;
}


void initThisSimulation()
{
    if (prj.simType != simulationType::RealTime) {
        ts.rfDataCountInThisEvent = rfs.size();
    }
    else {
        ts.rfDataCountInThisEvent = -1;
    }
    ts.time_simEnding_sec = (prj.simDuration_hr + prj.printTimeStep_min) * 60;
    ts.setupGRMisNormal = 1;
    ts.grmStarted = 1;
    ts.stopSim = -1;
    ts.dtsec = prj.dtsec;
    ts.dtMaxLimit_sec = (int)prj.printTimeStep_min * 60/2;
    ts.dtMinLimit_sec = 1;

    time_t now = time(0);
    localtime_s(&ts.g_RT_tStart_from_MonitorEXE, &now);
    ts.time_thisSimStarted = ts.g_RT_tStart_from_MonitorEXE;
}

int setCVStartingCondition(double iniflow)
{
    double hChCVini;
    double chCSAini;
    double qChCVini;
    double uChCVini;
    for (int i = 0; i < di.cellNnotNull; ++i) {
        double iniQAtwsOutlet = 0;
        int faAtBaseCV = di.facMax;
        int wsid = cvs[i].wsid;
        cvs[i].uOF = 0;
        cvs[i].hOF = 0;
        cvs[i].hOF_ori = 0;
        cvs[i].QOF_m3Ps = 0;
        cvs[i].hUAQfromChannelBed_m = 0;
        cvs[i].csaOF = 0;
        if (cvs[i].flowType == cellFlowType::ChannelFlow
            || cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) {
            int iniStreamFlowIWasSet = -1;
            if (prj.swps[wsid].iniFlow > 0) {//Apply ini. flow of current sws
                iniQAtwsOutlet = prj.swps[wsid].iniFlow;
                faAtBaseCV = cvs[di.wsn.wsOutletCVID[wsid] - 1].fac;
                iniStreamFlowIWasSet = true;
            }
            else {
                int baseWSid = wsid;
                for (int id = 0; id < di.wsn.wsidsAllDown[wsid].size(); id++) {
                    int downWSid = di.wsn.wsidNearbyDown[baseWSid];
                    if (prj.swps[downWSid].iniFlow > 0) {// If this condition is satisfied, apply ini. flow of downstream ws.
                        iniQAtwsOutlet = prj.swps[downWSid].iniFlow;
                        faAtBaseCV = cvs[di.wsn.wsOutletCVID[downWSid] - 1].fac;
                        iniStreamFlowIWasSet = 1;
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
            if (iniStreamFlowIWasSet == 1) {
                if (prj.simType == simulationType::SingleEventPE_SSR) {
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
                    double sngCAS_ini = qChCVini / (double)cvs[i].cvdx_m; // 초기값 설정
                    chCSAini = getChCSAbyQusingIteration(cvs[i], sngCAS_ini, qChCVini);
                    hChCVini = getChannelDepthUsingArea(cvs[i].stream.chBaseWidth, chCSAini,
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
                cvs[i].hUAQfromChannelBed_m = hChCVini; // 하도의 초기 수심을 비피압대수층의 초기 수심으로 설정 
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
        cvs[i].effCVCountFlowintoCViw = 0;
        cvs[i].QSSF_m3Ps = 0;
        cvs[i].bfQ_m3Ps = 0;
        cvs[i].hUAQfromBedrock_m = CONST_UAQ_HEIGHT_FROM_BEDROCK;
        cvs[i].storageCumulative_m3 = 0;
        if (prj.simFlowControl == 1) {
            int acvid = i + 1;
            if (prj.fcs.size() > 0 && getVectorIndex(fccds.cvidsFCcell, acvid) != -1) {
                double iniS = prj.fcs[acvid].iniStorage;
                if (iniS > 0) {
                    cvs[i].storageCumulative_m3 = iniS;
                }
                else {
                    cvs[i].storageCumulative_m3 = 0;
                }
            }
        }
    }
    for (int wpcvid : wpis.wpCVIDs) {
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

int simulateRunoff(double nowTmin)
{
    int maxLimit = di.facMax + 1;
    for (int fac = 0; fac < maxLimit; ++fac) {
        if (cvaisToFA[fac].size() > 0) {
            double uMax = 0;
#pragma omp parallel
            {
                int iterLimit = cvaisToFA[fac].size();
                omp_set_num_threads(prj.maxDegreeOfParallelism);
                // reduction으로 max, min 찾는 것은 openMP 3.1 이상부터 가능, 
                // VS2019는 openMP 2.0 지원, 그러므로 critical 사용한다.
#pragma omp for schedule(guided)
                for (int i = 0; i < iterLimit; ++i) {
                    if (cvs[i].toBeSimulated == 1) {
                        simulateRunoffCore(i, nowTmin);
                        if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                            uMax = cvs[i].uOF;
                        }
                        else {
                            uMax = cvs[i].stream.uCH;
                        }
                    }
                }
#pragma omp critical(getVmax)
                {
                    if (ts.vMaxInThisStep < uMax) {
                        ts.vMaxInThisStep = uMax;
                    }
                }
            }
        }
    }
    return 1;
}

void simulateRunoffCore(int i, double nowTmin)
{
    int fac = cvs[i].fac;
    int dtsec = ts.dtsec;
    double cellsize = di.cellSize;
    if (prj.simFlowControl == 1 &&
        (cvs[i].fcType == flowControlType::ReservoirOutflow ||
            cvs[i].fcType == flowControlType::Inlet)) {
        calFCReservoirOutFlow(nowTmin, i);
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
        else if (cvs[i].flowType == cellFlowType::ChannelFlow
            || cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) {
            double CSAchCVw_i_jP1 = 0;
            if (fac > 0) {
                CSAchCVw_i_jP1 = mFVMSolver.CalChCSA_CViW(cvs, i);
            }
            if (CSAchCVw_i_jP1 > 0 || cvs[i].mStreamAttr.hCH > 0) {
                mFVMSolver.CalculateChannelFlow(cvs[i], CSAchCVw_i_jP1);
            }
            else {
                setNoFluxCVCH(i);
            }
        }
    }
    if (prj.simFlowControl == 1
        && (cvs[i].fcType == flowControlType::SinkFlow
            || cvs[i].fcType == flowControlType::SourceFlow
            || cvs[i].fcType == flowControlType::ReservoirOperation)) {
        Dataset.GRMProject.FlowControlGridRow[] rows =
            (Dataset.GRMProject.FlowControlGridRow[])project.fcGrid.mdtFCGridInfo.Select("CVID = " + (cvan + 1));
        Dataset.GRMProject.FlowControlGridRow row = rows[0];
        double v;
        if (double.TryParse(row.MaxStorage, out v) == false || double.TryParse(row.MaxStorageR, out v) == false ||
            System.Convert.ToDouble(row.MaxStorage) * System.Convert.ToDouble(row.MaxStorageR) == 0) {
            cFlowControl.CalFCSinkOrSourceFlow(project, nowT_min, i);
        }
        else {
            cFlowControl.CalFCReservoirOperation(project, i, nowT_min);
        }
    }
}
