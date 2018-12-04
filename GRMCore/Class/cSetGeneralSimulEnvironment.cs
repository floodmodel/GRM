using System;

namespace GRMCore
{
    public class cSetGeneralSimulEnvironment
    {
        public bool mIsDateTimeFormat = false;
        public string mSimStartDateTime = null;
        /// <summary>
        ///   모의 기간[hour]
        ///   </summary>
        ///   <remarks></remarks>
        public Nullable<int> mSimDurationHOUR;
        /// <summary>
        ///   Printout time step[minute]
        ///   </summary>
        ///   <remarks></remarks>
        public Nullable<int> mPrintOutTimeStepMIN;

        public bool mbSimulateInfiltration = true;
        public bool mbSimulateSSFlow = true;
        public bool mbSimulateFlowControl = false;
        public bool mbSimulateBFlow = true;

        public bool mbRunAanlyzer = false;
        public bool mbMakeRasterOutput = false;
        public bool mbShowSoilSaturation = false;
        public bool mbShowRFdistribution = false;
        public bool mbShowRFaccDistribution = false;
        public bool mbShowFlowDistribution = false;
        public bool mbCreateImageFile = false;
        public bool mbCreateASCFile = false;

        public cGRM.GRMPrintType mPrintOption;

        public string mAboutThisProject;
        public string mAboutWatershed;
        public string mAboutLandCoverMap;
        public string mAboutSoilMap;
        public string mAboutSoilDepthMap;
        public string mAboutRainfall;

        /// <summary>
        ///   모델링 종료시간을 출력하기 위한 문자형 변수
        ///   </summary>
        ///   <remarks></remarks>
        public string mEndingTimeToPrint;

        public cSetGeneralSimulEnvironment()
        {
            mSimDurationHOUR = null;
            mPrintOutTimeStepMIN = null;
        }

        public void GetValues(Dataset.GRMProject prjDB)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
            if (!row.IsSimulationDurationNull())
            {
                if (!row.IsSimulStartingTimeNull()) { mSimStartDateTime = row.SimulStartingTime; }
                else { mSimStartDateTime = "0"; }
                int vi = 0;
                if (int.TryParse(mSimStartDateTime, out vi) == true)
                { mIsDateTimeFormat = false; }
                else
                { mIsDateTimeFormat = true; }

                if (int.TryParse(row.SimulationDuration, out vi) == true) { mSimDurationHOUR = vi; }
                if (int.TryParse(row.OutputTimeStep, out vi) == true) { mPrintOutTimeStepMIN = vi; }

                sThisSimulation.dtMaxLimit_sec = (int)mPrintOutTimeStepMIN / 2 * 60;
                bool v = true;
                if (bool.TryParse(row.SimulateBaseFlow, out v) == true) { mbSimulateBFlow = v; }
                if (bool.TryParse(row.SimulateSubsurfaceFlow, out v) == true) { mbSimulateSSFlow = v; }
                if (bool.TryParse(row.SimulateFlowControl, out v) == true) { mbSimulateFlowControl = v; }
                if (bool.TryParse(row.SimulateInfiltration, out v) == true) { mbSimulateInfiltration = v; }

                mbCreateASCFile = false;
                if (!row.IsMakeASCFileNull() && row.MakeASCFile.ToLower() == "true")
                {
                    mbCreateASCFile = true;
                }

                mbCreateImageFile = false;
                if (!row.IsMakeIMGFileNull() && row.MakeIMGFile.ToLower() == "true")
                {
                    mbCreateImageFile = true;
                }

                mbShowFlowDistribution = false;
                if (!row.IsMakeFlowDistFileNull() && row.MakeFlowDistFile.ToLower() == "true")
                {
                    if (bool.TryParse(row.MakeFlowDistFile, out v) == true)
                    {
                        mbShowFlowDistribution = v;
                    }
                }

                mbShowRFaccDistribution = false;
                if (!row.IsMakeRFaccDistFileNull() && row.MakeRFaccDistFile.ToLower() == "true")
                {
                    mbShowRFaccDistribution = true;
                }

                mbShowRFdistribution = false;
                if (!row.IsMakeRfDistFileNull() && row.MakeRfDistFile.ToLower() == "true")
                {
                    mbShowRFdistribution = true;
                }

                mbShowSoilSaturation = false;
                if (!row.IsMakeSoilSaturationDistFileNull() && row.MakeSoilSaturationDistFile.ToLower() == "true")
                {
                    mbShowSoilSaturation = true;
                }

                if (mbCreateImageFile == true || mbCreateASCFile == true)
                {
                    mbMakeRasterOutput = true;
                }
                else
                { mbMakeRasterOutput = false; }
                if (!row.IsPrintOptionNull())
                {
                    mPrintOption = cGRM.GRMPrintType.All;
                    if (row.PrintOption.ToString().ToLower() == cGRM.GRMPrintType.AllQ.ToString().ToLower())
                    {
                        mPrintOption = cGRM.GRMPrintType.AllQ;
                    }
                    else if (row.PrintOption.ToString().ToLower() == cGRM.GRMPrintType.DischargeFileQ.ToString().ToLower())
                    {
                        mPrintOption = cGRM.GRMPrintType.DischargeFileQ;
                    }
                }
                else
                {
                    mPrintOption = cGRM.GRMPrintType.All;
                }
                if (!row.IsAboutThisProjectNull())
                {
                    mAboutThisProject = row.AboutThisProject;
                }
                if (!row.IsAboutWatershedNull())
                {
                    mAboutWatershed = row.AboutWatershed;
                }
                if (!row.IsAboutLandCoverMapNull())
                {
                    mAboutLandCoverMap = row.AboutLandCoverMap;
                }
                if (!row.IsAboutSoilMapNull())
                {
                    mAboutSoilMap = row.AboutSoilMap;
                }
                if (!row.IsAboutSoilDepthMapNull())
                {
                    mAboutSoilDepthMap = row.AboutSoilDepthMap;
                }
                if (!row.IsAboutRainfallNull())
                { mAboutRainfall = row.AboutRainfall; }
            }
        }

        public bool IsSet
        {
            get
            {
                return mSimDurationHOUR.HasValue;
            }
        }

        public void SetValues(Dataset.GRMProject prjDB)
        {
            if (mSimDurationHOUR != null)
            {
                Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
                row.SimulationDuration = mSimDurationHOUR.Value.ToString();
                row.OutputTimeStep = mPrintOutTimeStepMIN.Value.ToString();
                row.SimulateBaseFlow = mbSimulateBFlow.ToString();
                row.SimulateSubsurfaceFlow = mbSimulateSSFlow.ToString();
                row.SimulateFlowControl = mbSimulateFlowControl.ToString();
                row.SimulateInfiltration = mbSimulateInfiltration.ToString();
                row.MakeASCFile = mbCreateASCFile.ToString();
                row.MakeIMGFile = mbCreateImageFile.ToString();
                row.PrintOption = mPrintOption.ToString();
                if (mAboutThisProject != "")
                {
                    row.AboutThisProject = mAboutThisProject;
                }
                if (mAboutWatershed != "")
                {
                    row.AboutWatershed = mAboutWatershed;
                }
                if (mAboutLandCoverMap != "")
                {
                    row.AboutLandCoverMap = mAboutLandCoverMap;
                }
                if (mAboutSoilMap != "")
                {
                    row.AboutSoilMap = mAboutSoilMap;
                }
                if (mAboutSoilDepthMap != "")
                {
                    row.AboutSoilDepthMap = mAboutSoilDepthMap;
                }
                if (mAboutRainfall != "")
                { row.AboutRainfall = mAboutRainfall; }
                row.ProjectSavedTime = DateTime.Now.ToString("yyyy/MM/dd HH:mm");
                row.ComputerName = Environment.MachineName;
                row.ComputerUserName = Environment.UserName;
                row.GRMVersion = cGRM.BuildInfo.ProductVersion;
            }
        }

        /// <summary>
        ///   모델링 종료시간을 출력하기 위한 문자형 변수
        ///   </summary>
        ///   <remarks></remarks>
        public string EndingTimeToPrint
        {
            get
            {
                return mEndingTimeToPrint;
            }
        }


        /// <summary>
        ///   모델링 기간과 출력 시간간격을 이용해서 모델링 끝나는 시간을 계산[sec]
        ///   </summary>
        ///   <remarks>'이때 사용자가 지정한 기간보다 한시간 더 모의한다.
        ///   왜냐하면, 시작시 0으로 출력되는 것이 실제로는 1시간 모의가 되므로, 
        ///   10시간 입력시, 실제로는 9시간이 마지막 출력이 된다.
        ///   그러므로 혼동을 없애기 위해 사용자가 지정한 시간으로 모의결과를 마치기 위해서 1시간을 더 모의해 준다. </remarks>
        public int EndingTimeSec
        {
            get
            {
                return System.Convert.ToInt32((mSimDurationHOUR * 60 + mPrintOutTimeStepMIN) * 60);
            }
        }
    }
}
