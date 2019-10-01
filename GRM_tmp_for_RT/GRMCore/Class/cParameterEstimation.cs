using System;
using System.Collections.Generic;
using System.Threading;

namespace GRMCore
{
    public class cParameterEstimation
    {
        public enum PEtype
        {
            INI_SOIL_SATURATION_RATIO,
            SOIL_DEPTH
        }

        public event CallAnalyzerEventHandler CallAnalyzer;
        public delegate void CallAnalyzerEventHandler(cParameterEstimation sender, cProject project, int nowTtoPrint_MIN, bool createImgFile, bool createASCFile);
        public event PEiterationEventHandler PEiteration;
        public delegate void PEiterationEventHandler(cParameterEstimation sender, int nowiter);
        public event PEcompleteEventHandler PEcomplete;
        public delegate void PEcompleteEventHandler(cParameterEstimation sender);
        public event PEstopEventHandler PEstop;
        public delegate void PEstopEventHandler(cParameterEstimation sender);

        private double mPESSRuniformRFori_mm;
        private double mPESSRuniformRFtoApply_mm;
        private Dictionary<int, PE_SS_Result> mCompletedWSid;
        private int mSWScountToEstSS;
        private List<cCVAttribute> mCVresult_SSR_IniFlow;


        private PEtype mPEtype;
        private cProject mProject;
        private cSimulator mSimulator;
        private bool mStopPE;
        private bool mCompletePE;
        private int mPrintOutStep_min;
        private bool mbAfterAllSSRbeenOne;

        private struct PE_SS_Result
        {
            public double INI_SSR;
            public double INI_FLOW;
        }


        public void StartPEiniSS(cProject project, double iniRF, int printoutStep_min = 0)
        {
            mPEtype = cParameterEstimation.PEtype.INI_SOIL_SATURATION_RATIO;
            mPESSRuniformRFori_mm = iniRF;
            mPESSRuniformRFtoApply_mm = iniRF;
            mProject = project;
            if (printoutStep_min == 0)
            {
                mPrintOutStep_min = System.Convert.ToInt32(sThisSimulation.dtsec / (double)60);
                if (mPrintOutStep_min < 1)
                {
                    mPrintOutStep_min = 1;
                }
            }
            else
            {
                mPrintOutStep_min = printoutStep_min;
            }
            mSWScountToEstSS = SWScountsetIniFlow;
            SimulatePEiniSS();
        }


        private void PE_SSR_setIniSoilConditionAndSetRFwithUniformValueForCV()
        {
            int rfInterval_sec = mProject.rainfall.RFIntervalSEC;
            int calinterval_sec = sThisSimulation.dtsec;
            double cellSize = mProject.watershed.mCellSize;
            for (int n = 0; n < mProject.CVCount; n++)
            {
                cCVAttribute cv = mProject.CVs[n];
                cv.InitialSaturation = 0;
                cv.soilSaturationRatio = 0;
                cv.soilWaterContent_m = 0;
                cv.soilWaterContent_tM1_m = 0;

                if ((cv.FlowType == cGRM.CellFlowType.ChannelFlow && cv.mStreamAttr.ChStrOrder > cProject.Current.subWSPar.userPars[cv.WSID].dryStreamOrder))
                {
                    cv.soilSaturationRatio = 1;
                }
                cRainfall.CalRFintensity_mPsec(mProject.CVs[n], mPESSRuniformRFtoApply_mm, rfInterval_sec);
            }
        }


        private void SimulatePEiniSS()
        {
            ThreadStart ts = new ThreadStart(SimulatePEiniSSinner);
            Thread th = new Thread(ts);
            th.Start();
        }

        private void SimulatePEiniSSinner()
        {
            int wpCount = mProject.watchPoint.WPCount;
            cSimulator simulator = new cSimulator();
            cOutPutControl outputControl = new cOutPutControl();
            if (!sThisSimulation.mGRMSetupIsNormal)
            {
                return;
            }
            int nowiterForPrint = 0;
            mCompletedWSid = new Dictionary<int, PE_SS_Result>();
            mCVresult_SSR_IniFlow = new List<cCVAttribute>();
            bool bContinue = true;
            int niter = 0;
            mCompletePE = false;
            mStopPE = false;
            simulator.Initialize();
            simulator.SetCVStartingCondition(mProject, wpCount, 0);
            PE_SSR_setIniSoilConditionAndSetRFwithUniformValueForCV();
            while (!bContinue == false)
            {
                niter += 1;
                int nowTmin = (niter - 1) * System.Convert.ToInt32(sThisSimulation.dtsec / (double)60);
                PE_SSR_SetPreEstimatedParameter();
                sThisSimulation.mRFMeanForDT_m = mPESSRuniformRFtoApply_mm / (double)1000 / (double)mProject.rainfall.RFIntervalSEC * sThisSimulation.dtsec;
                sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = sThisSimulation.mRFMeanForDT_m;
                simulator.SimulateRunoff(mProject, nowTmin);
                if ((nowiterForPrint) * sThisSimulation.dtsec / (double)60 == mPrintOutStep_min || niter == 1)
                {
                    outputControl.WriteSimResultsToTextFileForSingleEvent(mProject, wpCount, nowTmin, sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m, 1, null);
                    if (mProject.generalSimulEnv.mbMakeRasterOutput == true)
                    {
                        CallAnalyzer(this, mProject, nowTmin, mProject.generalSimulEnv.mbCreateImageFile, mProject.generalSimulEnv.mbCreateASCFile);
                    }
                    nowiterForPrint = 0;
                }
                PEiteration(this, niter);
                nowiterForPrint += 1;
                if (mbAfterAllSSRbeenOne == true)
                {
                    PESS_CheckSatisfyToleranceAndSetCVatt();
                }
                else
                {
                    PESSR_CheckSSRisOne();
                    if (mbAfterAllSSRbeenOne == true)
                    {
                        NoRFcondition();
                    }
                }

                if (mSWScountToEstSS == mCompletedWSid.Count)
                {
                    PEcomplete(this);
                    bContinue = false;
                }
                if (mStopPE == true)
                {
                    PEstop(this);
                    bContinue = false;
                }
            }
        }

        private void PESSR_CheckSSRisOne()
        {
            double sum = 0;
            for (int n = 0; n < mProject.CVCount; n++)
            {
                sum = sum + mProject.CVs[n].soilSaturationRatio;
            }
            double ave = sum / (double)mProject.CVCount;
            if (ave > 0.99)
            { mbAfterAllSSRbeenOne = true; }
            else
            { mbAfterAllSSRbeenOne = false; }
        }

        private void NoRFcondition()
        {
            int rfInterval_sec = mProject.rainfall.RFIntervalSEC;
            int calinterval_sec = sThisSimulation.dtsec;
            double cellSize = mProject.watershed.mCellSize;
            for (int n = 0; n < mProject.CVCount; n++)
            {
                cRainfall.CalRFintensity_mPsec(mProject.CVs[n], 0, rfInterval_sec);
            }
            mPESSRuniformRFtoApply_mm = 0;
        }

        private void PESS_CheckSatisfyToleranceAndSetCVatt()
        {
            foreach (int id in mProject.watershed.WSIDList)
            {
                if (mCompletedWSid.ContainsKey(id) == false)
                {
                    if (mProject.subWSPar.userPars[id].isUserSet == true && mProject.subWSPar.userPars[id].iniFlow > 0)
                    {
                        int wsCVarrayNum = mProject.WSNetwork.WSoutletCVID(id) - 1;
                        double err = Math.Abs(mProject.CVs[wsCVarrayNum].mStreamAttr.QCVch_i_j_m3Ps - mProject.subWSPar.userPars[id].iniFlow);
                        if (err < mProject.subWSPar.userPars[id].iniFlow / (double)100)
                        {
                            mCompletedWSid.Add(id, new PE_SS_Result());
                            PE_SS_Result results = new PE_SS_Result();
                            if (mProject.CVs[wsCVarrayNum].FlowType == cGRM.CellFlowType.OverlandFlow)
                            {
                                results.INI_FLOW = mProject.CVs[wsCVarrayNum].QCVof_i_j_m3Ps;
                            }
                            else
                            {
                                results.INI_FLOW = mProject.CVs[wsCVarrayNum].mStreamAttr.QCVch_i_j_m3Ps;
                            }
                            results.INI_SSR = mProject.CVs[wsCVarrayNum].soilSaturationRatio;
                            mCompletedWSid[id] = results;
                            SetCVattWithPEresult_SSR_IniFLOW(id);
                        }
                    }
                }
            }
        }

        /// <summary>
        ///   여기서는 매개변수 추정 과정에서 결정된 매개변수를 다시 매개변수 추정과정에서 설정해 주는것.
        ///   다지점 보정에서 이미 추정 완료된 유역에 대해서는 매개변수 추정하지 않고, 
        ///   그 하류만 반복적으로 추정하게 되므로.. 상류에서 설정된 값을 매먼 설정해 준다.
        ///   </summary>
        ///   <remarks></remarks>
        private void PE_SSR_SetPreEstimatedParameter()
        {
            foreach (int id in mCompletedWSid.Keys)
            {
                int wsCVarrayNum = mProject.WSNetwork.WSoutletCVID(id) - 1;
                {
                    cCVAttribute cv = mProject.CVs[wsCVarrayNum];
                    if (cv.FlowType == cGRM.CellFlowType.OverlandFlow)
                    { cv.QCVof_i_j_m3Ps = mCompletedWSid[id].INI_FLOW; }
                    else
                    { cv.mStreamAttr.QCVch_i_j_m3Ps = mCompletedWSid[id].INI_FLOW; }
                    cv.soilSaturationRatio = mCompletedWSid[id].INI_SSR;
                }
            }
        }

        /// <summary>
        ///   초기포화도와 하천 초기유량을 추정된 값으로 설정하고, 해당 유역을 PE 모델링 대상에서 제외시킨다.
        ///   </summary>
        ///   <param name="wsid"></param>
        ///   <returns></returns>
        ///   <remarks></remarks>
        private bool SetCVattWithPEresult_SSR_IniFLOW(int wsid)
        {
            // 우선 현재 추정된된 초기포화도와 초기유량 값을 초기조건으로 확정할 유역을 선택한다.
            List<int> wsidToSet = new List<int>();
            wsidToSet = GetUpWSIDlistToSet(mProject, wsid);
            if (wsidToSet.Contains(wsid) == false) { wsidToSet.Add(wsid); }                
            foreach (int swsid in wsidToSet)
            {
                foreach (int cvid in mProject.watershed.mCVidListForEachWS[swsid])
                {
                    cCVAttribute cv = new cCVAttribute();
                    cv.WSID = wsid;
                    cv.CVID = cvid;
                    cv.InitialSaturation = mProject.CVs[cvid - 1].soilSaturationRatio;
                    if (cv.FlowType == cGRM.CellFlowType.OverlandFlow)
                    {
                        cv.QCVof_i_j_m3Ps = mProject.CVs[cvid - 1].QCVof_i_j_m3Ps;
                    }
                    else
                    {
                        cv.mStreamAttr.QCVch_i_j_m3Ps = mProject.CVs[cvid - 1].mStreamAttr.QCVch_i_j_m3Ps;
                    }
                    mCVresult_SSR_IniFlow.Add(cv);
                    // 여기서 이렇게 설정하더라도.. 실제 모델링에서는 inlet의 상류만 false로 설정됨
                    mProject.CVs[cvid - 1].toBeSimulated = -1;
                }
            }
            return true;
        }


        private List<int> GetUpWSIDlistToSet(cProject project, int baseWSID)
        {
            List<int>upWSIDlistToSet = new List<int>();
            {
                List<int> wsidToExclude = new List<int>();
                foreach (int upsid in project.WSNetwork.WSIDsAllUps(baseWSID))
                {
                    if (project.subWSPar.userPars[upsid].isUserSet == true && project.subWSPar.userPars[upsid].iniFlow > 0)
                    {
                        if (!wsidToExclude.Contains(upsid))
                        {
                            wsidToExclude.Add(upsid);
                        }
                        foreach (int upupID in project.WSNetwork.WSIDsAllUps(upsid))
                        {
                            if (!wsidToExclude.Contains(upupID))
                            {
                                wsidToExclude.Add(upupID);
                            }
                        }
                    }
                }

                foreach (int upsid in project.WSNetwork.WSIDsAllUps(baseWSID))
                {
                    if (wsidToExclude.Contains(upsid) == false)
                    {
                        upWSIDlistToSet.Add(upsid);
                    }
                }
            }
            return upWSIDlistToSet;
        }

        public void StopParameterEstimation()
        {
            mStopPE = true;
        }

        private int SWScountsetIniFlow
        {
            get
            {
                int count = 0;
                foreach (int id in mProject.watershed.WSIDList)
                {
                    if (mProject.subWSPar.userPars[id].iniFlow > 0)
                    {
                        count += 1;
                    }
                }
                return count;
            }
        }
    }
}
