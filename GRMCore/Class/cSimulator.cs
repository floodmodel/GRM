using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Threading;
using System.Collections.Concurrent;

namespace GRMCore
{
    public class cSimulator
    {
        public enum SimulationErrors
        {
            OutputFileCreateError,
            ReadRainfallDataError,
            NoWatchPointError
        }

        public event SimulationStepEventHandler SimulationStep;
        public delegate void SimulationStepEventHandler(cSimulator sender, int elapsedMinutes);
        public event SimulationStopEventHandler SimulationStop;
        public delegate void SimulationStopEventHandler(cSimulator sender);
        public event SimulationCompleteEventHandler SimulationComplete;
        public delegate void SimulationCompleteEventHandler(cSimulator sender);
        public event SimulationRaiseErrorEventHandler SimulationRaiseError;
        public delegate void SimulationRaiseErrorEventHandler(cSimulator sender, SimulationErrors simulError, object erroData);
        public event SimulationMultiEventStepEventHandler SimulationMultiEventStep;
        public delegate void SimulationMultiEventStepEventHandler(cSimulator sender, int eventOrder);
        public event MakeRasterOutputEventHandler MakeRasterOutput;
        public delegate void MakeRasterOutputEventHandler(cSimulator sender, int nowTtoPrint_MIN);
        public event SendQToAnalyzerEventHandler SendQToAnalyzer;
        public delegate void SendQToAnalyzerEventHandler(cSimulator sender, int nowTtoPrint_MIN, double interCoef);


        private bool mStop;

        /// <summary>
        /// 강우입력자료가 끝났는지 여부
        /// </summary>
        /// <remarks></remarks>
        private bool mbRFisEnded;

        private cHydroCom mHydroCom = new cHydroCom();
        private cFVMSolver mFVMSolver = new cFVMSolver();
        private cInfiltration mInfiltration = new cInfiltration();
        private cFlowControl mFC = new cFlowControl();
        private cSSandBSflow mSSnBS = new cSSandBSflow();

        private cOutPutControl mOutputControl = new cOutPutControl();
        private cOutputControlRT mOutputControlRT;

        private cProject mProject;
        private cRealTime mRealTime;

        public cOutputControlRT OutputControlRT
        {
            get
            {
                return mOutputControlRT;
            }
        }


        public void Initialize()
        {
            mbRFisEnded = false;
            sThisSimulation.mTimeThisSimulationStarted = cRealTime_Common.g_dtStart_from_MonitorEXE;
            sThisSimulation.mRFMeanForDT_m = 0;
            sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = 0;

            if (sThisSimulation.IsParallel == true)
                cGRM.writelogAndConsole(string.Format("IsParallel : {0}, Max degree of parallelism : {1}", sThisSimulation.IsParallel.ToString(), sThisSimulation.MaxDegreeOfParallelism), cGRM.bwriteLog, true);
            else
                cGRM.writelogAndConsole(string.Format("IsParallel : {0}", sThisSimulation.IsParallel.ToString()), cGRM.bwriteLog, true);
        }

        public void SimulateSingleEvent(cProject project)
        {
            mProject = project;
            mStop = false;
            cRainfall.RainfallDataType eRainfallDataType = (cRainfall.RainfallDataType)mProject.rainfall.mRainfallDataType;
            List<cRainfall.RainfallData> dtRFinfo = mProject.rainfall.mlstRainfallData;
            int endingTimeSEC = mProject.generalSimulEnv.EndingTimeSec;
            int dTPrint_MIN = System.Convert.ToInt32(mProject.generalSimulEnv.mPrintOutTimeStepMIN);
            int dTPrint_SEC = dTPrint_MIN * 60;
            double dtmin = sThisSimulation.dtsec / (double)60;
            int dTRFintervalSEC = mProject.rainfall.RFIntervalSEC;
            int dTRFintervalMIN = System.Convert.ToInt32(mProject.rainfall.mRainfallinterval);
            int wpCount = mProject.watchPoint.WPCount;
            cRealTime_Common.g_dtStart_from_MonitorEXE = DateTime.Now;
            Initialize();
            SetCVStartingCondition(mProject, wpCount);
            if (!sThisSimulation.mGRMSetupIsNormal) { return; }
            int mSEC_tm1 = 0;
            cProjectBAK Project_tm1 = new cProjectBAK();
            //int targetToPrintMIN = 0;
            //bool bBAKdata = false;
            int targetCalTtoPrint_MIN = 0;
            int nowRFOrder = 0;
            int nowTsec = sThisSimulation.dtsec;
            sThisSimulation.dtsec_usedtoForwardToThisTime = sThisSimulation.dtsec;
            int dtsec;
            // CVid의 값은 1부터 시작함. 
            int simulationTimeLimitSEC = endingTimeSEC + sThisSimulation.dtsec;
            while (nowTsec <= endingTimeSEC) // simulationTimeLimitSEC
            {
                dtsec = sThisSimulation.dtsec;
                sThisSimulation.vMaxInThisStep = float.MinValue;
                // dtsec부터 시작해서, 첫번째 강우레이어를 이용한 모의결과를 0시간에 출력한다.
                if (!mbRFisEnded && (nowRFOrder == 0 || (nowTsec > dTRFintervalSEC * nowRFOrder)))
                {
                    if (nowRFOrder < sThisSimulation.mRFDataCountInThisEvent)
                    {
                        nowRFOrder = nowRFOrder + 1; // 이렇게 하면 마지막 레이어 적용
                        cRainfall.ReadRainfall(project, eRainfallDataType, dtRFinfo, dTRFintervalMIN, nowRFOrder, sThisSimulation.IsParallel);
                        mbRFisEnded = false;
                    }
                    else
                    {
                        cRainfall.SetRainfallintensity_mPsec_And_Rainfall_dt_meter_Zero(mProject);
                        nowRFOrder = int.MaxValue;
                        sThisSimulation.mRFMeanForDT_m = 0;
                        mbRFisEnded = true;
                    }
                }
                int nowT_MIN =nowTsec / 60;
                SimulateRunoff(mProject, nowT_MIN);
                cRainfall.CalCumulativeRFDuringDTPrintOut(mProject, dtsec);
                WriteCurrentResultAndInitializeNextStep(mProject, nowTsec, dtsec, dTRFintervalSEC, dTPrint_MIN, wpCount, 
                    ref targetCalTtoPrint_MIN, ref mSEC_tm1, ref Project_tm1, mProject.mSimulationType, nowRFOrder);
                // nowTsec = nowTsec + dtsec 'dtsec 만큼 전진

                // cThisSimulation.dtsec_usedtoForwardToThisTime = dtsec
                // If cThisSimulation.IsFixedTimeStep = False Then
                // cThisSimulation.dtsec = cHydroCom.getDTsec(cGRM.CONST_CFL_NUMBER,
                // project.Watershed.mCellSize, cThisSimulation.vMaxInThisStep, dTPrint_MIN)
                // End If

                if (nowTsec < endingTimeSEC && nowTsec + dtsec > endingTimeSEC)
                {
                    sThisSimulation.dtsec = nowTsec + dtsec - endingTimeSEC;
                    nowTsec = endingTimeSEC;
                    sThisSimulation.dtsec_usedtoForwardToThisTime = sThisSimulation.dtsec;
                }
                else
                {
                    nowTsec = nowTsec + dtsec; // dtsec 만큼 전진
                    sThisSimulation.dtsec_usedtoForwardToThisTime = sThisSimulation.dtsec;
                    if (sThisSimulation.IsFixedTimeStep == false)
                        sThisSimulation.dtsec = cHydroCom.getDTsec(cGRM.CONST_CFL_NUMBER, project.watershed.mCellSize, sThisSimulation.vMaxInThisStep, dTPrint_MIN);
                }

                if (mStop == true) { break; }
            }

            if (mStop==true)
            {
                SimulationStop(this);
            }
            else
            {
                cGRM.writelogAndConsole("Simulation was completed.", cGRM.bwriteLog, false);
                SimulationComplete(this);
            }
        }

        public void SimulateRT(cProject project, cRealTime realTime)
        {
            mProject = project;
            mRealTime = realTime;
            ThreadStart ts = new ThreadStart(SimulateRTInner);
            Thread th = new Thread(ts);
            th.Start();
        }

        public void SimulateRTInner()
        {
            // 2018.8 원 : 이건 single event에서는 사용 아닌 것으로 가정함

            Console.WriteLine("..debug.. Sub SimulateRTInner()");
            mStop = false;
            mOutputControlRT = new cOutputControlRT();
            bool bRainfallisEnded;
            int nowT_MIN;
            int targetCalTtoPrint_MIN = 0;
            cProjectBAK project_tm1 = new cProjectBAK ();
            //bool bBAKdata = false;
            int dTPrint_MIN = mRealTime.mDtPrintOutRT_min;
            int dtPrint_SEC = dTPrint_MIN * 60;
            int EndingT_SEC = mRealTime.EndingTime_SEC;
            int EndingT_HOUR = System.Convert.ToInt32(EndingT_SEC / 3600.0);
            int dTRFintervalSEC = System.Convert.ToInt32(mProject.rainfall.RFIntervalSEC);
            int sec_tm1 = 0;
            Initialize();
            SetCVStartingCondition(mProject, mProject.watchPoint.WPCount);
            int nowRFLayerOrder = 0;
            int nowTsec = sThisSimulation.dtsec;
            sThisSimulation.dtsec_usedtoForwardToThisTime = sThisSimulation.dtsec;
            int dtsec = 0;
            int simulationTimeLimitSEC = EndingT_SEC + dtsec + 1;
            while (nowTsec < simulationTimeLimitSEC)
            {
                dtsec = sThisSimulation.dtsec;
                nowT_MIN = System.Convert.ToInt32(nowTsec / (double)60);
                sThisSimulation.vMaxInThisStep = float.MinValue;
                if (nowRFLayerOrder == 0 || nowTsec > (dTRFintervalSEC * sThisSimulation.mRFDataCountInThisEvent))
                {
                    // 신규 강우자료 검색
                    string TargetRFLayerTime;
                    TargetRFLayerTime = string.Format(mRealTime.mDateTimeStartRT.Add
                                                  (new System.TimeSpan(0, (int)(nowRFLayerOrder * dTRFintervalSEC / 60.0), 0)).ToString(), "yyyyMMddHHmm");
                    do
                    {
                        if (mStop == true)
                            return;
                        mRealTime.UpdateRainfallInformationGRMRT(TargetRFLayerTime);
                        if (nowRFLayerOrder < sThisSimulation.mRFDataCountInThisEvent)
                            break;
                        Thread.Sleep(2000);
                    }
                    while (true)  // 2초 지연 적절함
    ;
                    nowRFLayerOrder = nowRFLayerOrder + 1; // 이렇게 하면 마지막 레이어 적용
                    cRainfall.ReadRainfall(mProject, mRealTime.mRainfallDataTypeRT, mRealTime.mlstRFdataRT, System.Convert.ToInt32(mProject.rainfall.mRainfallinterval), nowRFLayerOrder, sThisSimulation.IsParallel);
                    bRainfallisEnded = false;
                }
                if (mProject.fcGrid.IsSet == true)
                {
                    // 신규 fc 자료 검색 조건
                    string ReadDBorCSVandMakeFCdataTableForRealTime_TargetDataTime_Previous = "";
                    for (int nfc = 0; nfc < mProject.fcGrid.FCCellCount ; nfc++)
                    {
                        Dataset.GRMProject.FlowControlGridRow r = (Dataset.GRMProject.FlowControlGridRow)mProject.fcGrid.mdtFCGridInfo.Rows[nfc];
                        int cvid = r.CVID;
                        if (r.ControlType.ToString() != cFlowControl.FlowControlType.ReservoirOperation.ToString())
                        {
                            int dt_MIN = System.Convert.ToInt32(r.DT);
                            if (r.ControlType.ToString() != cFlowControl.FlowControlType.ReservoirOperation.ToString())
                            {
                                if (nowTsec > dt_MIN * 60 * mRealTime.mdicFCDataOrder[cvid] || mRealTime.mdicFCDataOrder[cvid] == 0)
                                {
                                    string TargetDataTime;
                                    TargetDataTime = string.Format(mRealTime.mDateTimeStartRT.Add
                                     (new System.TimeSpan(0, (int)(mRealTime.mdicFCDataOrder[cvid] * dt_MIN), 0)).ToString(), "yyyyMMddHHmm");
                                    bool bAfterSleep = false;
                                    do
                                    {
                                        if (ReadDBorCSVandMakeFCdataTableForRealTime_TargetDataTime_Previous != TargetDataTime | bAfterSleep)
                                            mRealTime.ReadDBorCSVandMakeFCdataTableForRealTime_v2018(TargetDataTime);
                                        ReadDBorCSVandMakeFCdataTableForRealTime_TargetDataTime_Previous = TargetDataTime;
                                        if (mStop == true)
                                            return;
                                        mRealTime.UpdateFcDatainfoGRMRT(TargetDataTime, cvid, mRealTime.mdicFCDataOrder[cvid], dt_MIN);
                                        if (mRealTime.mdicBNewFCdataAddedRT[cvid] == true)
                                            break;
                                        Thread.Sleep(2000);
                                        bAfterSleep = true;
                                    }
                                    while (true);
                                    mRealTime.mdicFCDataOrder[cvid] += 1;
                                }
                            }
                        }
                    }
                }
                // cThisSimulation.vMaxInThisStep = Single.MinValue
                SimulateRunoff(mProject, nowT_MIN);
                cRainfall.CalCumulativeRFDuringDTPrintOut(mProject, dtsec);
                int wpCount = mProject.watchPoint.WPCount;
                WriteCurrentResultAndInitializeNextStep(mProject, nowTsec, dtsec, dTRFintervalSEC, dTPrint_MIN, wpCount, ref targetCalTtoPrint_MIN, ref sec_tm1, ref project_tm1, mProject.mSimulationType, nowRFLayerOrder);
                nowTsec = nowTsec + dtsec;
                sThisSimulation.dtsec_usedtoForwardToThisTime = dtsec;
                if (sThisSimulation.IsFixedTimeStep == false)
                    sThisSimulation.dtsec = cHydroCom.getDTsec(cGRM.CONST_CFL_NUMBER, mProject.watershed.mCellSize, sThisSimulation.vMaxInThisStep, dTPrint_MIN);
                if (mStop == true)
                    break;
            }
            if (mStop == true)
                SimulationStop(this);
            else
                SimulationComplete(this);
        }


        public void SimulateRunoff(cProject project, int nowT_MIN)
        {
            double cellsize = project.watershed.mCellSize;
            int dtsec = sThisSimulation.dtsec;
            if (sThisSimulation.IsParallel == true)
            {
                for (int fac = 0; fac < project.watershed.mFacMax + 1; fac++)
                {
                    if (project.mCVANsForEachFA[fac] != null)
                    {
                        //int nfac = fac;
                        //int ncv = 0;
                        //int[] cvans = new int[project.mCVANsForEachFA[fac].Length];
                        //project.mCVANsForEachFA[fac].CopyTo(cvans, 0);
                        int[] cvans = project.mCVANsForEachFA[fac];
                        ParallelOptions options = new ParallelOptions();
                        options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                        Parallel.ForEach(Partitioner.Create(0, cvans.Length), options, range =>
                        {
                            for (int i = range.Item1; i < range.Item2; i++)
                            {
                                int cvan = cvans[i];
                                if (project.CVs[cvan].toBeSimulated == 1)
                                { simulateRunoffCore(project, fac, cvan, dtsec, nowT_MIN, cellsize); }
                            }
                        });
                    }
                }

                foreach (cCVAttribute icv in project.CVs)
                {
                    if (icv.toBeSimulated == 1)
                    {
                        if (icv.FlowType == cGRM.CellFlowType.OverlandFlow)
                        {
                            if (sThisSimulation.vMaxInThisStep < icv.uCVof_i_j)
                                sThisSimulation.vMaxInThisStep = icv.uCVof_i_j;
                        }
                        else if (sThisSimulation.vMaxInThisStep < icv.mStreamAttr.uCVch_i_j)
                            sThisSimulation.vMaxInThisStep = icv.mStreamAttr.uCVch_i_j;
                    }
                }
            }
            else
            {
                for (int fac = 0; fac < project.watershed.mFacMax + 1; fac++)
                {
                    if (project.mCVANsForEachFA[fac] != null)
                    {
                        foreach (int cvan in project.mCVANsForEachFA[fac])
                        {
                            if (project.CVs[cvan].toBeSimulated == 1)
                            {
                                simulateRunoffCore(project, fac, cvan, dtsec, nowT_MIN, cellsize);
                                if (project.CVs[cvan].FlowType == cGRM.CellFlowType.OverlandFlow)
                                {
                                    if (sThisSimulation.vMaxInThisStep < project.CVs[cvan].uCVof_i_j)
                                        sThisSimulation.vMaxInThisStep = project.CVs[cvan].uCVof_i_j;
                                }
                                else if (sThisSimulation.vMaxInThisStep < project.CVs[cvan].mStreamAttr.uCVch_i_j)
                                    sThisSimulation.vMaxInThisStep = project.CVs[cvan].mStreamAttr.uCVch_i_j;
                            }
                        }
                    }
                }
            }
        }


        private void simulateRunoffCore(cProject project, int fac, int cvan, int dtsec, int nowT_min, double cellsize)
        {
            cRainfall.CalRF_mPdt(project.CVs[cvan], dtsec, cellsize);
            if (project.generalSimulEnv.mbSimulateFlowControl == true &&
                (project.CVs[cvan].FCType == cFlowControl.FlowControlType.ReservoirOutflow || project.CVs[cvan].FCType == cFlowControl.FlowControlType.Inlet))
                mFC.CalFCReservoirOutFlow(project, nowT_min, cvan);
            else
            {
                InitializeCVForThisStep(project, cvan);
                if (project.CVs[cvan].FlowType == cGRM.CellFlowType.OverlandFlow)
                {
                    double hCVw_i_jP1 = 0;
                    if (fac > 0)
                    {
                        hCVw_i_jP1 = mFVMSolver.CalculateOverlandWaterDepthCViW(project, cvan);
                    }
                    if (hCVw_i_jP1 > 0 || project.CVs[cvan].hCVof_i_j > 0)
                    {
                        mFVMSolver.CalculateOverlandFlow(project.CVs[cvan], hCVw_i_jP1, project.watershed.mCellSize);
                    }
                    else
                    {
                        mFVMSolver.SetNoFluxOverlandFlowCV(project.CVs[cvan]);
                    }
                }
                else if (project.CVs[cvan].FlowType == cGRM.CellFlowType.ChannelFlow || project.CVs[cvan].FlowType == cGRM.CellFlowType.ChannelNOverlandFlow)
                {
                    double CSAchCVw_i_jP1 = 0;
                    if (fac > 0)
                    {
                        CSAchCVw_i_jP1 = mFVMSolver.CalChCSA_CViW(project.CVs, cvan);
                    }
                    if (CSAchCVw_i_jP1 > 0 || project.CVs[cvan].mStreamAttr.hCVch_i_j > 0)
                    {
                        mFVMSolver.CalculateChannelFlow(project.CVs[cvan], CSAchCVw_i_jP1);
                    }
                    else
                    {
                        mFVMSolver.SetNoFluxChannelFlowCV(project.CVs[cvan]);
                    }
                }
            }
            if (project.generalSimulEnv.mbSimulateFlowControl == true && (project.CVs[cvan].FCType == cFlowControl.FlowControlType.SinkFlow
                || project.CVs[cvan].FCType == cFlowControl.FlowControlType.SourceFlow || project.CVs[cvan].FCType == cFlowControl.FlowControlType.ReservoirOperation))
            {
                Dataset.GRMProject.FlowControlGridRow[] rows =
                    (Dataset.GRMProject.FlowControlGridRow[])project.fcGrid.mdtFCGridInfo.Select("CVID = " + (cvan + 1));
                Dataset.GRMProject.FlowControlGridRow row = rows[0];
                if (System.Convert.ToDouble(row.MaxStorage) * System.Convert.ToDouble(row.MaxStorageR) == 0)
                {
                    mFC.CalFCSinkOrSourceFlow(project, nowT_min, cvan);
                }
                else
                {
                    mFC.CalFCReservoirOperation(project, cvan, nowT_min);
                }
            }
        }

        /// <summary>
        /// 강우, 지표하 유출, 기저유출에 의한 생성항 계산 및 t시간에서의 모의를 위한 초기값을 설정한다.
        /// 또한 [Channel and overland flow] 속성 셀에서의 overland flow 부분을 계산해서 하도로 측방 유입시킨다.
        /// </summary>
        /// <param name="project"></param>
        /// <param name="cvan"></param>
        /// <remarks></remarks>
        private void InitializeCVForThisStep(cProject project, int cvan)
        {
            int rfInterval_SEC = project.rainfall.RFIntervalSEC;
            double dY_m = project.watershed.mCellSize;
            double effOFdYinCHnOFcell = 0;
            int dTSEC = sThisSimulation.dtsec;
            double CVdx_m = project.CVs[cvan].CVDeltaX_m;
            double chCSAAddedByBFlow_m2 = 0;
            double ofDepthAddedByRFlow_m2 = 0;
            double chCSAAddedBySSFlow_m2 = 0;
            if (project.CVs[cvan].FlowType == cGRM.CellFlowType.ChannelNOverlandFlow)
            { effOFdYinCHnOFcell = dY_m - project.CVs[cvan].mStreamAttr.ChBaseWidth; }
            if (project.generalSimulEnv.mbSimulateBFlow == true)
            { chCSAAddedByBFlow_m2 = mSSnBS.CalBFlowAndGetCSAAddedByBFlow(project, cvan, dTSEC, dY_m); }
            cCVAttribute cv = project.CVs[cvan];
            if (project.generalSimulEnv.mbSimulateSSFlow == true)
            {
                switch (cv.FlowType)
                {
                    case cGRM.CellFlowType.OverlandFlow:
                        {
                            ofDepthAddedByRFlow_m2 = mSSnBS.GetRFlowOFAreaAddedBySSflowCVwAndCalSSflowAtNowCV(project.CVs, cvan, dTSEC, dY_m, cv.CVDeltaX_m);
                            break;
                        }
                    case cGRM.CellFlowType.ChannelFlow:
                        {
                            chCSAAddedBySSFlow_m2 = mSSnBS.GetChCSAaddedBySSFlowInChlCell(project.CVs, cvan);
                            break;
                        }
                    case cGRM.CellFlowType.ChannelNOverlandFlow:
                        {
                            ofDepthAddedByRFlow_m2 = mSSnBS.GetRFlowOFAreaAddedBySSflowCVwAndCalSSflowAtNowCV(project.CVs, 
                                cvan, dTSEC, effOFdYinCHnOFcell, cv.CVDeltaX_m);
                            chCSAAddedBySSFlow_m2 = mSSnBS.GetChCSAaddedBySSFlowInChNOfCell(project.CVs, cvan);
                            break;
                        }
                }
            }
            mInfiltration.CalEffectiveRainfall(project, cvan, rfInterval_SEC, dTSEC);
            switch (cv.FlowType)
            {
                case cGRM.CellFlowType.OverlandFlow:
                    {
                        cv.hCVof_i_j_ori = cv.hCVof_i_j + cv.EffRFCV_dt_meter + ofDepthAddedByRFlow_m2;
                        cv.hCVof_i_j = cv.hCVof_i_j_ori;
                        cv.CSAof_i_j = cv.hCVof_i_j_ori * dY_m;
                        cv.StorageAddedForDTfromRF_m3 = cv.EffRFCV_dt_meter * dY_m * CVdx_m;
                        break;
                    }
                case cGRM.CellFlowType.ChannelFlow:
                    {
                        double ChWidth = cv.mStreamAttr.ChBaseWidth;
                        cv.mStreamAttr.hCVch_i_j_ori = cv.mStreamAttr.hCVch_i_j + cv.RFApp_dt_meter
                                                     + chCSAAddedBySSFlow_m2 / ChWidth
                                                     + chCSAAddedByBFlow_m2 / ChWidth;
                        cv.mStreamAttr.CSAch_i_j_ori = mFVMSolver.GetChannelCrossSectionAreaUsingChannelFlowDepth(ChWidth, 
                            cv.mStreamAttr.mChBankCoeff, cv.mStreamAttr.hCVch_i_j_ori,
                            cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chLowerRHeight, 
                            cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chUpperRBaseWidth_m);
                        cv.mStreamAttr.hCVch_i_j = cv.mStreamAttr.hCVch_i_j_ori;
                        cv.mStreamAttr.CSAch_i_j = cv.mStreamAttr.CSAch_i_j_ori;
                        cv.StorageAddedForDTfromRF_m3 = cv.RFApp_dt_meter * dY_m * CVdx_m;
                        break;
                    }
                case cGRM.CellFlowType.ChannelNOverlandFlow:
                    {
                        double chCSAAddedByOFInChCell_m2;
                        double ChWidth = cv.mStreamAttr.ChBaseWidth;
                        cv.hCVof_i_j_ori = cv.hCVof_i_j + cv.EffRFCV_dt_meter + ofDepthAddedByRFlow_m2;
                        cv.hCVof_i_j = cv.hCVof_i_j_ori;
                        cv.CSAof_i_j = cv.hCVof_i_j_ori * effOFdYinCHnOFcell;
                        if (cv.hCVof_i_j > 0)
                        {
                            mFVMSolver.CalculateOverlandFlow(project.CVs[cvan], 0, effOFdYinCHnOFcell);
                        }
                        else
                        {
                            cv.hCVof_i_j = 0;
                            cv.uCVof_i_j = 0;
                            cv.CSAof_i_j = 0;
                            cv.QCVof_i_j_m3Ps = 0;
                        }
                        if (cv.QCVof_i_j_m3Ps > 0)
                        // chCSAAddedByOFInChCell_m2 = mFVMSolver.CalChCSAFromQbyIteration(project.CVs[cvan], .CSAof_i_j, .QCVof_i_j_m3Ps)
                        {
                            chCSAAddedByOFInChCell_m2 = cv.CSAof_i_j;
                        } // 이렇게 단순화 해도, 별 차이 없다. 계산시간은 좀 준다. 2018.03.12.최
                        else
                        {
                            chCSAAddedByOFInChCell_m2 = 0;
                        }
                        cv.mStreamAttr.CSAchAddedByOFinCHnOFcell = chCSAAddedByOFInChCell_m2;
                        cv.mStreamAttr.hCVch_i_j_ori = cv.mStreamAttr.hCVch_i_j + cv.RFApp_dt_meter + chCSAAddedBySSFlow_m2 / ChWidth 
                            + chCSAAddedByBFlow_m2 / ChWidth;
                        if (cv.mStreamAttr.hCVch_i_j_ori < 0) { cv.mStreamAttr.hCVch_i_j_ori = 0; }
                        cv.mStreamAttr.CSAch_i_j_ori = mFVMSolver.GetChannelCrossSectionAreaUsingChannelFlowDepth(ChWidth, cv.mStreamAttr.mChBankCoeff, 
                            cv.mStreamAttr.hCVch_i_j_ori, cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chLowerRHeight, cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chUpperRBaseWidth_m);
                        cv.mStreamAttr.hCVch_i_j = cv.mStreamAttr.hCVch_i_j_ori;
                        cv.mStreamAttr.CSAch_i_j = cv.mStreamAttr.CSAch_i_j_ori;
                        cv.StorageAddedForDTfromRF_m3 = cv.RFApp_dt_meter * ChWidth * CVdx_m + cv.EffRFCV_dt_meter * effOFdYinCHnOFcell * CVdx_m;
                        break;
                    }
            }
            cv.QsumCVw_dt_m3 = 0;
        }


        public void SetCVStartingCondition(cProject project, int intWPCount, double iniflow = 0)
        {
            double hChCVini;
            double chCSAini;
            double qChCVini;
            double uChCVini;
            for (int cvan = 0; cvan < project.CVCount; cvan++)
            {
                double iniQAtWP = 0;
                int faAtBaseWP = project.watershed.mFacMax;
                cCVAttribute cv = project.CVs[cvan]; // 이거 잘 될까? 2018.11.09
                cv.uCVof_i_j = 0;
                cv.hCVof_i_j = 0;
                cv.hCVof_i_j_ori = 0;
                cv.QCVof_i_j_m3Ps = 0;
                cv.hUAQfromChannelBed_m = 0;
                cv.CSAof_i_j = 0;
                if (cv.FlowType == cGRM.CellFlowType.ChannelFlow || cv.FlowType == cGRM.CellFlowType.ChannelNOverlandFlow)
                {
                    bool bApplyIniStreamFlowIsSet = false;
                    if (project.subWSPar.userPars[cv.WSID].iniFlow > 0)
                    {
                        iniQAtWP = project.subWSPar.userPars[cv.WSID].iniFlow;
                        faAtBaseWP = project.CVs[project.WSNetwork.WSoutletCVID(cv.WSID) - 1].FAc;
                        bApplyIniStreamFlowIsSet = true;
                    }
                    else
                    {
                        int baseWSid = cv.WSID;
                        for (int id = 0; id < project.WSNetwork.WSIDsAllDowns(cv.WSID).Count; id++)
                        {
                            int downWSid = project.WSNetwork.WSIDsNearbyDown(baseWSid);
                            if (project.subWSPar.userPars[downWSid].iniFlow > 0)
                            {
                                iniQAtWP = project.subWSPar.userPars[downWSid].iniFlow;
                                faAtBaseWP = project.CVs[project.WSNetwork.WSoutletCVID(downWSid) - 1].FAc;
                                bApplyIniStreamFlowIsSet = true;
                                break;
                            }
                            else
                            {
                                baseWSid = downWSid;
                            }
                        }
                    }
                    chCSAini = 0;
                    hChCVini = 0;
                    qChCVini = 0;
                    uChCVini = 0;
                    if (bApplyIniStreamFlowIsSet == true)
                    {
                        if (project.mSimulationType == cGRM.SimulationType.SingleEventPE_SSR)
                        {
                            qChCVini = iniflow * (cv.FAc - project.watershed.mFacMostUpChannelCell)
                                  / (double)(faAtBaseWP - project.watershed.mFacMostUpChannelCell);
                        }
                        else if (project.watershed.mFPN_initialChannelFlow == "")
                        {
                            qChCVini = iniQAtWP * (cv.FAc - project.watershed.mFacMostUpChannelCell)
                                  / (double)(faAtBaseWP - project.watershed.mFacMostUpChannelCell);
                        }
                        else
                        {
                            qChCVini = cv.mStreamAttr.initialQCVch_i_j_m3Ps;
                        }
                        if (qChCVini > 0)
                        {
                            double sngCAS_ini = qChCVini / (double)cv.CVDeltaX_m; // 초기값 설정
                            chCSAini = mFVMSolver.CalChCSAFromQbyIteration(project.CVs[cvan], sngCAS_ini, qChCVini);
                            hChCVini = mFVMSolver.GetChannelDepthUsingArea(cv.mStreamAttr.ChBaseWidth, chCSAini,
                                cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chUpperRBaseWidth_m,
                                cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chLowerRHeight, cv.mStreamAttr.mChBankCoeff);
                        }
                    }
                    cv.mStreamAttr.hCVch_i_j = hChCVini;
                    cv.mStreamAttr.CSAch_i_j = chCSAini;
                    cv.mStreamAttr.hCVch_i_j_ori = hChCVini;
                    cv.mStreamAttr.CSAch_i_j_ori = chCSAini;
                    cv.mStreamAttr.QCVch_i_j_m3Ps = qChCVini;
                    cv.mStreamAttr.uCVch_i_j = uChCVini;
                    if (project.generalSimulEnv.mbSimulateBFlow == true)
                    {
                        cv.hUAQfromChannelBed_m = hChCVini; // 하도의 초기 수심을 비피압대수층의 초기 수심으로 설정 
                    }
                    else
                    {
                        cv.hUAQfromChannelBed_m = 0;
                    }
                }
                //cv.Qprint_cms = 0;
                cv.RFReadintensity_tM1_mPsec = 0;
                cv.EffRFCV_dt_meter = 0;
                cv.RFApp_dt_meter = 0;
                cv.RF_dtPrintOut_meter = 0;
                cv.RFAcc_FromStartToNow_meter = 0;
                cv.RFReadintensity_mPsec = 0;
                cv.RFReadintensity_tM1_mPsec = 0;
                cv.SoilMoistureChangeDeltaTheta = 0;
                cv.InfiltrationF_mPdt = 0;
                cv.InfiltrationRatef_mPsec = 0;
                cv.InfiltrationRatef_tM1_mPsec = 0;
                cv.EffectiveSaturationSe = 0;
                cv.bAfterSaturated = false;
                cv.StorageAddedForDTfromRF_m3 = 0;
                cv.QsumCVw_dt_m3 = 0;
                cv.effCVCountFlowINTOCViW = 0;
                cv.SSF_Q_m3Ps = 0;
                cv.baseflow_Q_m3Ps = 0;
                cv.hUAQfromBedrock_m = cGRM.CONST_UAQ_HEIGHT_FROM_BEDROCK;
                cv.StorageCumulative_m3 = 0;
                if (project.generalSimulEnv.mbSimulateFlowControl == true)
                {
                    if (project.fcGrid.IsSet == true && project.fcGrid.FCGridCVidList.Contains(cvan + 1))
                    {
                        Dataset.GRMProject.FlowControlGridRow[] rows = (Dataset.GRMProject.FlowControlGridRow[])project.fcGrid.mdtFCGridInfo.Select("CVID = " + (cvan + 1));
                        Dataset.GRMProject.FlowControlGridRow row = rows[0];
                        if (!row.IsIniStorageNull() && row.IniStorage != "")
                        {
                            cv.StorageCumulative_m3 = System.Convert.ToDouble(row.IniStorage);
                        }
                    }
                }
            }
            foreach (int wpcvid in project.watchPoint.WPCVidList)
            {
                project.watchPoint.MaxDepth_m[wpcvid] = 0;
                project.watchPoint.MaxDepthTime[wpcvid] = "";
                project.watchPoint.MaxFlow_cms[wpcvid] = 0;
                project.watchPoint.MaxFlowTime[wpcvid] = "";
                project.watchPoint.RFWPGridTotal_mm[wpcvid] = 0;
                project.watchPoint.RFUpWsMeanTotal_mm[wpcvid] = 0;
                project.watchPoint.mTotalDepth_m[wpcvid] = 0;
                project.watchPoint.mTotalFlow_cms[wpcvid] = 0;
            }
        }


        private void WriteCurrentResultAndInitializeNextStep(cProject project, int mNowTsec, int dtsec, int dTRFintervalSEC, int dTPrint_MIN, int wpCount,
            ref int targetCalTtoPrint_MIN, ref int mSEC_tm1, ref cProjectBAK Project_tm1, cGRM.SimulationType SimType, int mNowRFLayerNumber)
        {
            int dTRFintervalMIN = System.Convert.ToInt32(dTRFintervalSEC / (double)60);
            int dTPrint_SEC = dTPrint_MIN * 60;
            double dtmin = dtsec / (double)60;
            int timeToPrint_MIN;
            if (targetCalTtoPrint_MIN == 0)
            {
                targetCalTtoPrint_MIN = dTPrint_MIN;
            }
            if (mNowRFLayerNumber == 1 && dTPrint_MIN > dTRFintervalMIN && ((mNowTsec + dtsec) > dTRFintervalSEC))
            {
                // 첫번째 출력전에 다음 스텝에서 강우레이어가 바뀌는 경우는 첫번째 강우레이어 모델링이 끝났다는 얘기이므로 한번 출력한다.
                // 0 시간에서의 모델링 결과로 출력한다.
                timeToPrint_MIN = 0;
                double RFmeanForFirstLayer = sThisSimulation.mRFMeanForDT_m / dtmin * dTRFintervalMIN;
                OutputProcessManagerBySimType(timeToPrint_MIN, wpCount, RFmeanForFirstLayer, 1, null, SimType);
            }
            else if (mNowTsec > 0 && (mNowTsec % dTPrint_SEC) == 0)
            {
                timeToPrint_MIN = targetCalTtoPrint_MIN - dTPrint_MIN;
                OutputProcessManagerBySimType(timeToPrint_MIN, wpCount, sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m, 1, null, SimType);
                targetCalTtoPrint_MIN = targetCalTtoPrint_MIN + dTPrint_MIN;
            }
            else
            {
                if (mNowTsec < targetCalTtoPrint_MIN * 60 && (mNowTsec + dtsec) > (targetCalTtoPrint_MIN) * 60)
                {
                    if (Project_tm1.isSet == false)
                    {
                        mSEC_tm1 = mNowTsec;
                        Project_tm1 = new cProjectBAK();
                        Project_tm1.SetCloneUsingCurrentProject(project);
                    }
                }
                if (mNowTsec > (targetCalTtoPrint_MIN * 60) && (mNowTsec - sThisSimulation.dtsec_usedtoForwardToThisTime) <= (targetCalTtoPrint_MIN * 60))
                {
                    double coeffInterpolation;
                    coeffInterpolation = (targetCalTtoPrint_MIN * 60 - mSEC_tm1) / (double)(mNowTsec - mSEC_tm1);
                    timeToPrint_MIN = targetCalTtoPrint_MIN - dTPrint_MIN;
                    OutputProcessManagerBySimType(timeToPrint_MIN, wpCount, sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m, coeffInterpolation, Project_tm1, SimType);
                    targetCalTtoPrint_MIN = targetCalTtoPrint_MIN + dTPrint_MIN;
                    Project_tm1.CVs = null;
                    Project_tm1.isSet = false;
                }
            }
        }


        private void OutputProcessManagerBySimType(int nowTtoPrint_MIN, int wpCount, double SumRFMeanForDTprintOut_m, double coeffInterpolation, cProjectBAK Project_tm1, cGRM.SimulationType simType)
        {
            switch (simType)
            {
                case cGRM.SimulationType.SingleEvent:
                    {
                        if (SimulationStep != null) { SimulationStep(this, nowTtoPrint_MIN); }
                        if (mProject.generalSimulEnv.mPrintOption == cGRM.GRMPrintType.All)
                        {
                            mOutputControl.WriteSimResultsToTextFileForSingleEvent(mProject, wpCount, nowTtoPrint_MIN, SumRFMeanForDTprintOut_m, coeffInterpolation, Project_tm1);
                        }
                        if (mProject.generalSimulEnv.mPrintOption == cGRM.GRMPrintType.DischargeFileQ)
                        {
                            mOutputControl.WriteDischargeOnlyToDischargeFile(mProject, coeffInterpolation, Project_tm1);
                        }
                        if (mProject.generalSimulEnv.mPrintOption == cGRM.GRMPrintType.AllQ)
                        {
                            mOutputControl.WriteDischargeOnlyToDischargeFile(mProject, coeffInterpolation, Project_tm1);
                            mOutputControl.WriteDischargeOnlyToWPFile(mProject, coeffInterpolation, Project_tm1);
                        }
                        cGRM.writelogAndConsole(string.Format("Time(min) dt(sec), {0}{1}{2}", nowTtoPrint_MIN, "\t", sThisSimulation.dtsec), cGRM.bwriteLog, false);
                        break;
                    }

                case cGRM.SimulationType.RealTime:
                    {
                        mOutputControlRT.WriteSimResultsToTextFileAndDBForRealTime(mProject, nowTtoPrint_MIN, coeffInterpolation, Project_tm1, mRealTime);
                        break;
                    }
            }

            if (mProject.generalSimulEnv.mbRunAanlyzer == true)
            // RaiseEvent SendQToAnalyzer(Me, mProject, Project_tm1, nowTtoPrint_MIN, coeffInterpolation)
            {
                SendQToAnalyzer(this, nowTtoPrint_MIN, coeffInterpolation);
            }
            if (mProject.generalSimulEnv.mbMakeRasterOutput == true)
            {
                MakeRasterOutput(this, nowTtoPrint_MIN);
            }

            sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = 0;
            foreach (Dataset.GRMProject.WatchPointsRow row in mProject.watchPoint.mdtWatchPointInfo)
            {
                mProject.watchPoint.RFUpWsMeanForDtPrintout_mm[row.CVID] = 0;
                mProject.watchPoint.RFWPGridForDtPrintout_mm[row.CVID] = 0;
            }
            if (mProject.generalSimulEnv.mbShowRFdistribution == true)
            {
                for (int cvan = 0; cvan < cProject.Current.CVCount; cvan++)
                { cProject.Current.CVs[cvan].RF_dtPrintOut_meter = 0; }
            }
        }

        public void StopSimulation()
        {
            mStop = true;
        }
    }
}
