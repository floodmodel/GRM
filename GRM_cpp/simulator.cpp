
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
extern map<int, vector<int>> cvaisToFA; //fa별 cv array idex 목록
extern vector<rainfallData> rfs;
extern flowControlCellAndData fccds;
extern wpinfo wpis;

extern cvAtt* cvsb; 
map<int, double> fcdAb;// <cvid, value> t-dt 시간에 적용된 flow control data 값
wpinfo wpisb;

int startSimulationSingleEvent()
{
    initThisSimulation();
    setCVStartingCondition(0);
    //int dtPrint_min = prj.printTimeStep_min;
    int dtRF_sec = prj.rfinterval_min * 60;
    //int dtrf_min = prj.rfinterval_min;
    //int isRFended = -1;
    //int tsec_tm1 = 0;
    //cvAtt Project_tm1;
    //int targetTtoPrint_min = 0;
    int rfOrder = 0;
    int nowTsec = ts.dtsec;
    double nowTmin;
    ts.dtsecUsed_tm1 = ts.dtsec;
    ts.zeroTimePrinted = -1;
    ts.targetTtoP_sec = prj.dtPrint_min;
    //int dtsec;
    int endingT_sec = ts.time_simEnding_sec + ts.dtsec;
    while (nowTsec <= endingT_sec) {// simulationTimeLimitSEC
        //dtsec = ts.dtsec;        
        // dtsec부터 시작해서, 첫번째 강우레이어를 이용한 모의결과를 0시간에 출력한다.
        /*if (isRFended == -1 && (nowRFOrder == 0 || (nowTsec > dtRF_sec* nowRFOrder))) {*/
        if (nowTsec > dtRF_sec* rfOrder) {
            if (rfOrder < ts.rfDataCountTotal) {
                rfOrder++; // 이렇게 하면 마지막 레이어 적용
                if (setCVRF(rfOrder) == -1) { return -1; }    //isRFended = -1;
            }
            else {
                setRFintensityAndDTrf_Zero();
                rfOrder = INT_MAX;   //isRFended = 1;
            }
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
            nowTsec = nowTsec + ts.dtsec; // dtsec 만큼 전진
            ts.dtsecUsed_tm1 = ts.dtsec;
            if (prj.IsFixedTimeStep == -1) {
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
        writeLog(fpnLog, "Simulation was completed.\n", 1, 1);
        return 1;
    }
    return 1;
}


int simulateRunoff(double nowTmin)
{
    int maxLimit = di.facMax + 1;
    ts.vMaxInThisStep = DBL_MIN;
    int numth = prj.mdp;
    double* uMax = new double[numth];
    for (int fac = 0; fac < maxLimit; ++fac) {
        if (cvaisToFA[fac].size() > 0) {
            int iterLimit = cvaisToFA[fac].size();
#pragma omp parallel
            {    // reduction으로 max, min 찾는 것은 openMP 3.1 이상부터 가능, 
                // VS2019는 openMP 2.0 지원, 그러므로 critical 사용한다.
                // critical은 속도 손실이 있으므로, 배열로 할당해서 직접 계산한다.
                int id = omp_get_thread_num();
                uMax[id] = 0;
#pragma omp for schedule(guided) 
                for (int i = 0; i < iterLimit; ++i) {
                    if (cvs[i].toBeSimulated == 1) {
                        //id = omp_get_thread_num();
                        simulateRunoffCore(i, nowTmin);
                        if (prj.IsFixedTimeStep == -1) {
                            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                                if (uMax[id] < cvs[i].uOF) {
                                    uMax[id] = cvs[i].uOF;
                                }
                            }
                            else {
                                if (uMax[id] < cvs[i].stream.uCH) {
                                    uMax[id] = cvs[i].stream.uCH;
                                }
                            }
                        }
                    }
                }
            }
            //#pragma omp critical(getVmax)
            if (prj.IsFixedTimeStep == -1) {//이 조건 적용 여부?
                for (int i = 0; i < numth; ++i) {
                    if (ts.vMaxInThisStep < uMax[i]) {
                        ts.vMaxInThisStep = uMax[i];
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
        fccds.fcDataAppliedNowT_m3Ps[i + 1] = 0;
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
        else { //cvs[i].flowType == cellFlowType::ChannelFlow  || cvs[i].flowType == cellFlowType::ChannelNOverlandFlow
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
          fccds.fcDataAppliedNowT_m3Ps[i+1] = 0;
          if (cvs[i].fcType == flowControlType::SinkFlow
              || cvs[i].fcType == flowControlType::SourceFlow) {
              calSinkOrSourceFlow(i, nowTmin);
          }
          if (prj.fcs[i + 1].roType != reservoirOperationType::None) {
              // rotype이 있으면, ro로 넘어간다.
              calReservoirOperation(i, nowTmin);
          }
      }
    }
}

void initThisSimulation()
{
    if (prj.simType != simulationType::RealTime) {
        ts.rfDataCountTotal = rfs.size();
    }
    else {
        ts.rfDataCountTotal = -1;
    }
    // 이렇게 해야 모의기간에 맞게 실행된다. 
    //왜냐하면, 첫번째 실행 결과가 0 시간으로 출력되기 때문에
    ts.time_simEnding_sec = (prj.simDuration_hr*60 + prj.dtPrint_min) * 60;     
    ts.setupGRMisNormal = 1;
    ts.grmStarted = 1;
    ts.stopSim = -1;
    ts.dtsec = prj.dtsec;
    ts.dtMaxLimit_sec = (int)prj.dtPrint_min * 60 / 2;
    ts.dtMinLimit_sec = 1;
    ts.vMaxInThisStep = DBL_MIN;
    //ts.iscvsb = -1;
    ts.cvsbT_sec = 0;

    time_t now = time(0);
    localtime_s(&ts.g_RT_tStart_from_MonitorEXE, &now);
    ts.time_thisSimStarted = COleDateTime::GetCurrentTime();;
}

void setCVStartingCondition(double iniflow)
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
        cvs[i].effCVnFlowintoCVw = 0;
        cvs[i].QSSF_m3Ps = 0;
        cvs[i].bfQ_m3Ps = 0;
        cvs[i].hUAQfromBedrock_m = CONST_UAQ_HEIGHT_FROM_BEDROCK;
        cvs[i].storageCumulative_m3 = 0;
        if (prj.simFlowControl == 1) {
            int acvid = i + 1;
            if (prj.fcs.size() > 0 && getVectorIndex(fccds.cvidsFCcell, acvid) != -1) {
                double iniS = prj.fcs[acvid].iniStorage_m3;
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


void outputManager(int nowTsec, int rfOrder)
{
    int dtP_min = prj.dtPrint_min;
    int dtrf_min = (int)prj.rfinterval_min;
    int dtrf_sec = dtrf_min * 60;
    int dtP_SEC = dtP_min * 60;
    double dtmin = ts.dtsec / 60.0;
    int timeToP_min;
    if (rfOrder == 1
        && dtP_min > dtrf_min
        && ((nowTsec + ts.dtsec) > dtrf_sec)) {
        // 첫번째 출력전에 다음 스텝에서 강우레이어가 바뀌는 경우는 첫번째 강우레이어 모델링이 끝났다는 얘기이므로 한번 출력한다.
        // 0 시간에서의 모델링 결과로 출력한다.
        double RFmeanForFirstLayer = ts.rfAveForDT_m / dtmin * dtrf_min;
        writeBySimType(0, 1);
        ts.zeroTimePrinted = 1;
        return; // 이경우는 모의시작 전에 설정된  ts.targetTtoP_min = dtP_min 을 유지
    }
    else if (nowTsec % dtP_SEC == 0) {
        if (ts.zeroTimePrinted == -1) {
            timeToP_min = ts.targetTtoP_sec * 60 - dtP_min; // 이렇게 해야 첫번째 모의 결과가 0시간에 출력된다.
        }
        else {
            timeToP_min = ts.targetTtoP_sec * 60;
        }
        writeBySimType(timeToP_min, 1);
        ts.targetTtoP_sec = ts.targetTtoP_sec + dtP_SEC;
        return;
    }
    else {
        if (nowTsec <  ts.targetTtoP_sec
            && (nowTsec + ts.dtsec) >ts.targetTtoP_sec) {
            // 만일 현재의 dtsec으로 한번더 전진해서 이 조건을 만족하면
            std::copy(cvs, cvs + di.cellNnotNull, cvsb);
            wpisb = wpis;
            if (prj.simFlowControl == 1) {
                fcdAb = fccds.fcDataAppliedNowT_m3Ps;
            }
            ts.cvsbT_sec = nowTsec;
            ts.isbak = 1;
            return; //벡업만 받고 나간다.
        }
        if (nowTsec > ts.targetTtoP_sec
            && (nowTsec - ts.dtsecUsed_tm1) <= ts.targetTtoP_sec) {
            double citerp;
            citerp = (ts.targetTtoP_sec - ts.cvsbT_sec) / (double)(nowTsec - ts.cvsbT_sec);
            timeToP_min = ts.targetTtoP_sec * 60 - dtP_min; // 이렇게 해야 첫번째 모의 결과가 0시간에 출력된다.
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
    writeSimStep(nowTP_min);
    int wpCount = wpis.wpCVIDs.size();
    simulationType simType = prj.simType;
    switch (simType) {
    case simulationType::SingleEvent: {
        if (prj.printOption == GRMPrintType::All) {
            writeSingleEvent(nowTP_min, cinterp);
        }
        if (prj.printOption == GRMPrintType::DischargeFileQ) {
            writeDischargeOnly(cinterp, -1);
        }
        if (prj.printOption == GRMPrintType::AllQ) {
            writeDischargeOnly(cinterp, 1);
        }
        break;
    }
    case simulationType::RealTime: {
        writeDBRealTime(nowTP_min, cinterp);
        break;
    }
    }
    if (ts.runByAnalyzer == 1)    {
        // 클래스나 전역 변수에 저장해서, 외부로 노출하고, analyzer에서 사용하는 방법은?
        //모의 종료시 혹은 1초마다 챠트 업데이트하는 방법?
        //SendQToAnalyzer(nowTP_min, cinterp);
    }
    if (prj.makeASCorIMGfile == 1)    {
        makeRasterOutput(nowTP_min);
    }
    ts.rfAveSumAllCells_dtP_m = 0;
    for (int cvid : wpis.wpCVIDs) {
        wpis.rfUpWSAveForDtP_mm[cvid] = 0;
        wpis.rfWPGridForDtP_mm[cvid] = 0;
    }
}