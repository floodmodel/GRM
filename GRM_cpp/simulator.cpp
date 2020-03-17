
#include "gentle.h"
#include "grm.h"

using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern thisProcess tp;

extern domaininfo di;
extern cvAtt* cvs;
extern vector<rainfallData> rfs;
extern flowControlCellAndData fccds;
extern wpinfo wpis;

int startSimulationSingleEvent()
{
    initThisProcess();
    setCVStartingCondition(0);
    int stop = -1;
    int dtPrint_min = prj.printTimeStep_min;
    int dtRF_sec = prj.rfinterval_min * 60;
    int dtRFinterval_min = prj.rfinterval_min;
    int isRFended = -1;
    int tsec_tm1 = 0;
    cvAtt Project_tm1 ;
    int targetTtoPrint_min = 0;
    int nowRFOrder = 0;
    int nowTsec = tp.dtsec;
    tp.dtsec_usedtoForwardToThisTime = tp.dtsec;
    tp.zeroTimePrinted = -1;
    int dtsec;
    // CVid의 값은 1부터 시작함. 
    while (nowTsec <= tp.time_simEnding_sec) {// simulationTimeLimitSEC
        dtsec = tp.dtsec;
        tp.vMaxInThisStep = DBL_MIN;
        // dtsec부터 시작해서, 첫번째 강우레이어를 이용한 모의결과를 0시간에 출력한다.
        if (isRFended ==-1 && (nowRFOrder == 0 || (nowTsec > dtRF_sec* nowRFOrder))) {
            if (nowRFOrder < tp.rfDataCountInThisEvent)            {
                nowRFOrder = nowRFOrder + 1; // 이렇게 하면 마지막 레이어 적용
                setCVRF(nowRFOrder);
                //cRainfall.ReadRainfall(project, eRainfallDataType, dtRFinfo, dTRFintervalMIN, nowRFOrder, sThisSimulation.IsParallel);
                isRFended = -1;
            }
            else {
                cRainfall.SetRainfallintensity_mPsec_And_Rainfall_dt_meter_Zero(mProject);
                nowRFOrder = int.MaxValue;
                sThisSimulation.mRFMeanForDT_m = 0;
                mbRFisEnded = true;
            }
        }
        int nowT_MIN = nowTsec / 60;
        SimulateRunoff(mProject, nowT_MIN);
        cRainfall.CalCumulativeRFDuringDTPrintOut(mProject, dtsec);
        WriteCurrentResultAndInitializeNextStep(mProject, nowTsec, dtsec, dTRFintervalSEC, dTPrint_MIN, wpCount,
            ref targetCalTtoPrint_MIN, ref mSEC_tm1, ref Project_tm1, mProject.mSimulationType, nowRFOrder);
        if (nowTsec < endingTimeSEC && nowTsec + dtsec > endingTimeSEC) {
            sThisSimulation.dtsec = nowTsec + dtsec - endingTimeSEC;
            nowTsec = endingTimeSEC;
            sThisSimulation.dtsec_usedtoForwardToThisTime = sThisSimulation.dtsec;
        }
        else {
            nowTsec = nowTsec + dtsec; // dtsec 만큼 전진
            sThisSimulation.dtsec_usedtoForwardToThisTime = sThisSimulation.dtsec;
            if (sThisSimulation.IsFixedTimeStep == false)
            {
                sThisSimulation.dtsec = cHydroCom.getDTsec(cGRM.CONST_CFL_NUMBER,
                    project.watershed.mCellSize, sThisSimulation.vMaxInThisStep, dTPrint_MIN);
            }
        }
        if (mStop == true) { break; }
    }
    if (mStop == true) {
        SimulationStop();
    }
    else {
        cGRM.writelogAndConsole("Simulation was completed.", cGRM.bwriteLog, false);
        SimulationComplete();
    }
    return 1;
}


int initThisProcess()
{
    if (prj.simType != simulationType::RealTime) {
        tp.rfDataCountInThisEvent = rfs.size();
    }
    else {
        tp.rfDataCountInThisEvent = -1;
    }
    tp.time_simEnding_sec = (prj.simDuration_hr + prj.printTimeStep_min) * 60;
    tp.setupGRMisNormal = 1;
    tp.grmStarted = 1;

    tp.dtsec = prj.dtsec;

    time_t now = time(0);
    localtime_s(&tp.g_RT_tStart_from_MonitorEXE, &now);
    tp.time_thisSimStarted = tp.g_RT_tStart_from_MonitorEXE;
    return 1;
}

int setCVStartingCondition(double iniflow)
{
    double hChCVini;
    double chCSAini;
    double qChCVini;
    double uChCVini;
    for (int i = 0; i < di.cellCountNotNull; ++i) {
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
                        cvs[i].stream.isCompoundCS, cvs[i].stream.chHighRBaseWidth_m,
                        cvs[i].stream.chLowRArea_m2, cvs[i].stream.chLowRHeight, cvs[i].stream.bankCoeff);
                }
            }
            cvs[i].stream.hCH = hChCVini;
            cvs[i].stream.csaCh = chCSAini;
            cvs[i].stream.hCH_ori = hChCVini;
            cvs[i].stream.csaCh_ori = chCSAini;
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
        cvs[i].rfintensityRead_tm1_mPsec = 0;
        cvs[i].rfintensityRead_mPsec = 0;
        cvs[i].rfEff_dt_m = 0;
        cvs[i].rfApp_dt_m = 0;
        cvs[i].rf_dtPrintOut_m = 0;
        cvs[i].rfAcc_FromStartToNow_m = 0;
        cvs[i].soilMoistureChange_DTheta = 0;
        cvs[i].infiltrationF_mPdt = 0;
        cvs[i].infiltRatef_mPsec = 0;
        cvs[i].infiltRatef_tM1_mPsec = 0;
        cvs[i].effSR_Se = 0;
        cvs[i].isAfterSaturated = -1;
        cvs[i].StorageAddedForDTfromRF_m3 = 0;
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

