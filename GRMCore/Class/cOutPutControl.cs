using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using gentle;

namespace GRMCore
{
    public class cOutPutControl
    {
        public static event ProcessTextEventHandler ProcessText;
        public  delegate void ProcessTextEventHandler(string pText);
        public static event ProcessValueEventHandler ProcessValue;
        public delegate void ProcessValueEventHandler(int pValue);

        public void WriteSimResultsToTextFileForSingleEvent(cProject project, int wpCount, int nowT_MIN, double meanRainfallSumToPrintOut_m, double interCoef, cProjectBAK project_tm1)
        {
            DateTime timeNow;
            long lngTimeDiffFromStarting_SEC;
            string strNowTimeToPrintOut;
            double meanRFSumForPrintoutTime_mm;
            string strFNPDischarge;
            string strFNPDepth;
            string strFNPRFGrid;
            string strFNPRFMean;
            string strFNPFCData;
            string strFNPFCStorage;
            // Dim lineToPrint As String = ""
            string vToPrint = "";

            meanRFSumForPrintoutTime_mm = meanRainfallSumToPrintOut_m * 1000;
            timeNow = DateTime.Now;
            TimeSpan tsTotalSim = timeNow - sThisSimulation.mTimeThisSimulationStarted;
            lngTimeDiffFromStarting_SEC = (long) tsTotalSim.TotalSeconds;// .Seconds;//DateDiff(DateInterval.Second, cThisSimulation.mTimeThisSimulationStarted, timeNow);
            strNowTimeToPrintOut = cComTools.GetTimeToPrintOut(project.generalSimulEnv.mIsDateTimeFormat, project.generalSimulEnv.mSimStartDateTime, nowT_MIN);

            strFNPDischarge = project.OFNPDischarge;
            strFNPDepth = project.OFNPDepth;
            strFNPRFGrid = project.OFNPRFGrid;
            strFNPRFMean = project.OFNPRFMean;
            strFNPFCData = project.OFNPFCData;
            strFNPFCStorage = project.OFNPFCStorage;

            // ===================================================================================================
            // 유량
            StringBuilder sbQ = new StringBuilder();
            // lineToPrint = strNowTimeToPrintOut
            sbQ.Append(strNowTimeToPrintOut);
            foreach (int wpcvid in project.watchPoint.WPCVidList)
            {
                int cvan = wpcvid - 1;
                cCVAttribute cv = project.CVs[cvan];
                if (interCoef == 1)
                {
                    switch (cv.FlowType)
                    {
                        case cGRM.CellFlowType.OverlandFlow:
                            {
                                vToPrint = cv.QCVof_i_j_m3Ps.ToString("F2");
                                break;
                            }
                        default:
                            {
                                vToPrint = cv.mStreamAttr.QCVch_i_j_m3Ps.ToString("F2");
                                break;
                            }
                    }
                }
                else if (project_tm1.CVs[cvan] != null)
                {
                    switch (cv.FlowType)
                    {
                        case cGRM.CellFlowType.OverlandFlow:
                            {
                                vToPrint = cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].QCVof_i_j_m3Ps, cv.QCVof_i_j_m3Ps, interCoef).ToString("F2");
                                break;
                            }

                        default:
                            {
                                vToPrint = cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].mStreamAttr.QCVch_i_j_m3Ps, cv.mStreamAttr.QCVch_i_j_m3Ps, interCoef).ToString("F2");
                                break;
                            }
                    }
                }
                else
                    vToPrint = "0";
                 
                // lineToPrint = lineToPrint + vbTab + vToPrint.Trim
                sbQ.Append("\t" + vToPrint.Trim());
                double sv = System.Convert.ToDouble(vToPrint);
                project.watchPoint.mTotalFlow_cms[wpcvid] = project.watchPoint.mTotalFlow_cms[wpcvid] + sv;
                project.watchPoint.Qprint_cms[wpcvid] = System.Convert.ToDouble(vToPrint);
                if (project.watchPoint.MaxFlow_cms[wpcvid] < sv)
                {
                    project.watchPoint.MaxFlow_cms[wpcvid] = sv;
                    project.watchPoint.MaxFlowTime[wpcvid] = strNowTimeToPrintOut;
                }
                WriteWPouputs(strNowTimeToPrintOut, cvan, interCoef, project, project_tm1);
            }
            sbQ.Append("\t" + meanRFSumForPrintoutTime_mm.ToString("F2") + "\t" + lngTimeDiffFromStarting_SEC.ToString() + "\r\n");
            File.AppendAllText(strFNPDischarge, sbQ.ToString(), Encoding.Default);

            // ===================================================================
            // FCAppFlow, FCStorage
            if (project.generalSimulEnv.mbSimulateFlowControl == true && project.fcGrid.FCCellCount > 0)
            {
                StringBuilder sbFCFlow = new StringBuilder();
                StringBuilder sbFCStorage = new StringBuilder();
                sbFCFlow.Append(strNowTimeToPrintOut);
                sbFCStorage.Append(strNowTimeToPrintOut);
                if (interCoef == 1)
                {
                    foreach (int fcCvid in project.fcGrid.FCGridCVidList)
                    {
                        sbFCFlow.Append("\t" + project.fcGrid.mFCdataToApplyNowT[fcCvid].ToString("F2"));
                        sbFCStorage.Append("\t" + project.CVs[fcCvid - 1].StorageCumulative_m3.ToString("F2"));
                    }
                }
                else 
                {
                    foreach (int fcCvid in project.fcGrid.FCGridCVidList)
                    {
                        if (project_tm1.CVs[fcCvid - 1] != null)
                        {
                            sbFCFlow.Append("\t" + cHydroCom.GetInterpolatedValueLinear(project_tm1.fcGrid.mFCdataToApplyNowT[fcCvid],
                                project.fcGrid.mFCdataToApplyNowT[fcCvid], interCoef).ToString("F2"));
                            sbFCStorage.Append("\t" + cHydroCom.GetInterpolatedValueLinear(project_tm1.CV(fcCvid - 1).StorageCumulative_m3,
                                project.CVs[fcCvid - 1].StorageCumulative_m3, interCoef).ToString("F2"));
                        }
                    }
                }
                sbFCFlow.Append("\r\n");
                sbFCStorage.Append("\r\n");
                System.IO.File.AppendAllText(strFNPFCData, sbFCFlow.ToString(), Encoding.Default);
                System.IO.File.AppendAllText(strFNPFCStorage, sbFCStorage.ToString(), Encoding.Default);
            }
            // ===================================================================
            if (nowT_MIN == System.Convert.ToInt32(project.generalSimulEnv.mSimDurationHOUR * 60))
                project.generalSimulEnv.mEndingTimeToPrint = strNowTimeToPrintOut;
        }



        private void WriteWPouputs(string strNowTimeToPrintOut, int cvan, double interCoef, cProject project, cProjectBAK project_tm1)
        {
            // watchpoint별 모든 자료 출력
            int wpcvid = cvan + 1;
            StringBuilder sbWP = new StringBuilder();
            sbWP.Append(strNowTimeToPrintOut + "\t");
            sbWP.Append(project.watchPoint.Qprint_cms[wpcvid].ToString("F2") + "\t");
            if (interCoef == 1)
            {
                sbWP.Append(project.CVs[cvan].hUAQfromChannelBed_m.ToString("F4") + "\t");
                sbWP.Append(project.CVs[cvan].soilWaterContent_m.ToString("F4") + "\t");
                sbWP.Append(project.CVs[cvan].soilSaturationRatio.ToString("F4") + "\t");
                sbWP.Append(project.watchPoint.RFWPGridForDtPrintout_mm[wpcvid].ToString("F2") + "\t");
                sbWP.Append(project.watchPoint.RFUpWsMeanForDtPrintout_mm[wpcvid].ToString("F2")+ "\t");
                sbWP.Append(project.watchPoint.QfromFCDataCMS[wpcvid].ToString("F2")+ "\t");
                sbWP.Append(project.CVs[cvan].StorageCumulative_m3.ToString("F2") + "\r\n");
            }
            else if (project_tm1.CVs[cvan] != null)
            {
                sbWP.Append(cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].hUAQfromChannelBed_m, project.CVs[cvan].hUAQfromChannelBed_m, interCoef).ToString("F4") + "\t");
                sbWP.Append(cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].soilWaterContent_m, project.CVs[cvan].soilWaterContent_m, interCoef).ToString("F4") + "\t");
                double ssv = cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].soilSaturationRatio, project.CVs[cvan].soilSaturationRatio, interCoef);
                sbWP.Append(cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].soilSaturationRatio, project.CVs[cvan].soilSaturationRatio, interCoef).ToString("F4") + "\t");
                sbWP.Append(cHydroCom.GetInterpolatedValueLinear(project_tm1.watchPoint.RFWPGridForDtPrintout_mm[wpcvid], project.watchPoint.RFWPGridForDtPrintout_mm[wpcvid], interCoef).ToString("F2") + "\t");
                sbWP.Append(cHydroCom.GetInterpolatedValueLinear(project_tm1.watchPoint.RFUpWsMeanForDtPrintout_mm[wpcvid], project.watchPoint.RFUpWsMeanForDtPrintout_mm[wpcvid], interCoef).ToString("F2") + "\t");
                sbWP.Append(cHydroCom.GetInterpolatedValueLinear(project_tm1.watchPoint.QfromFCDataCMS[wpcvid], project.watchPoint.QfromFCDataCMS[wpcvid], interCoef).ToString("F2")+ "\t");
                sbWP.Append(cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].StorageCumulative_m3, project.CVs[cvan].StorageCumulative_m3, interCoef).ToString("F2")+ "\r\n");
            }
            System.IO.File.AppendAllText(project.watchPoint.FpnWpOut[wpcvid], sbWP.ToString(), Encoding.Default);
        }


        public void WriteDischargeOnlyToDischargeFile(cProject project, double interCoef, cProjectBAK project_tm1)
        {
            string strFNPDischarge;
            StringBuilder sbQ = new StringBuilder();
            string vToPrint = "";
            strFNPDischarge = project.OFNPDischarge;
            foreach (int wpcvid in project.watchPoint.WPCVidList)
            {
                int cvan = wpcvid - 1;
                if (interCoef == 1)
                {
                    switch (project.CVs[cvan].FlowType)
                    {
                        case cGRM.CellFlowType.OverlandFlow:
                            {
                                vToPrint = project.CVs[cvan].QCVof_i_j_m3Ps.ToString("F2");
                                break;
                            }

                        default:
                            {
                                vToPrint = project.CVs[cvan].mStreamAttr.QCVch_i_j_m3Ps.ToString("F2");
                                break;
                            }
                    }
                }
                else if (project_tm1.CVs[cvan] != null)
                {
                    switch (project.CVs[cvan].FlowType)
                    {
                        case cGRM.CellFlowType.OverlandFlow:
                            {
                                vToPrint = cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].QCVof_i_j_m3Ps, project.CVs[cvan].QCVof_i_j_m3Ps, interCoef).ToString("F2");
                                break;
                            }

                        default:
                            {
                                vToPrint = cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].mStreamAttr.QCVch_i_j_m3Ps, project.CVs[cvan].mStreamAttr.QCVch_i_j_m3Ps, interCoef).ToString("F2");
                                break;
                            }
                    }
                }
                else
                { vToPrint = "0"; }
                project.watchPoint.Qprint_cms[wpcvid] = System.Convert.ToDouble(vToPrint);
                if (sbQ.ToString().Trim() == "")
                    sbQ.Append(vToPrint.Trim());
                else
                    sbQ.Append("\t" + vToPrint.Trim());
            }
            sbQ.Append("\r\n");
            System.IO.File.AppendAllText(strFNPDischarge, sbQ.ToString(), Encoding.Default);
        }

        public void WriteDischargeOnlyToWPFile(cProject project, double interCoef, cProjectBAK project_tm1)
        {
            foreach (int wpcvid in project.watchPoint.WPCVidList)
            {
                int cvanWP = wpcvid - 1;
                string strL = "";
                if (interCoef == 1)
                {
                    if (project.CVs[cvanWP].FlowType == cGRM.CellFlowType.OverlandFlow)
                        strL = project.CVs[cvanWP].QCVof_i_j_m3Ps.ToString("F2") + "\r\n";
                    else
                        strL = project.CVs[cvanWP].mStreamAttr.QCVch_i_j_m3Ps.ToString("F2") + "\r\n";
                }
                else if (project_tm1.CVs [cvanWP] != null)
                {
                    if (project.CVs[cvanWP].FlowType == cGRM.CellFlowType.OverlandFlow)
                        strL = cHydroCom.GetInterpolatedValueLinear(project_tm1.CV(cvanWP).QCVof_i_j_m3Ps, project.CVs[cvanWP].QCVof_i_j_m3Ps, interCoef).ToString("F2") + "\r\n";
                    else
                        strL = cHydroCom.GetInterpolatedValueLinear(project_tm1.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps, project.CVs[cvanWP].mStreamAttr.QCVch_i_j_m3Ps, interCoef).ToString("F2") + "\r\n";
                }
                project.watchPoint.Qprint_cms[wpcvid] = System.Convert.ToDouble(strL);
                string wpName = project.watchPoint.wpName(wpcvid);
                string nFPN = Path.Combine(project.ProjectPath, project.ProjectNameOnly + "WP_" + wpName + ".out");
                System.IO.File.AppendAllText(nFPN, strL, Encoding.Default);
            }
        }

        public static bool CreateNewOutputFiles(cProject project, bool deleteOnly)
        {
            try
            {
                sThisSimulation.mGRMSetupIsNormal = true;
                int wpCount = project.watchPoint.WPCount;
                string strFNPDischarge = project.OFNPDischarge;
                string strFNPDepth = project.OFNPDepth;
                string strFNPRFGrid = project.OFNPRFGrid;
                string strFNPRFMean = project.OFNPRFMean;
                string strFNPFCData = project.OFNPFCData;
                string strFNPFCStorage = project.OFNPFCStorage;
                List<string> FPNs = new List<string>();
                    FPNs.Add(strFNPDischarge);
                    FPNs.Add(strFNPDepth);
                    FPNs.Add(strFNPRFGrid);
                    FPNs.Add(strFNPRFMean);
                    FPNs.Add(strFNPFCData);
                    FPNs.Add(strFNPFCStorage);
                    foreach (Dataset.GRMProject.WatchPointsRow row in project.watchPoint.mdtWatchPointInfo.Rows)
                    {
                        string wpName = row.Name.ToString().Replace(",", "_");
                        string wpfpn = Path.Combine(project.ProjectPath, project.ProjectNameOnly + "WP_" + wpName + ".out");
                        FPNs.Add(wpfpn);
                    }

                bool beenRun = false;
                foreach (string fpn in FPNs)
                {
                    if (File.Exists(fpn))
                    {
                        beenRun = true;
                        break;
                    }
                }

                if (beenRun == true)
                {
                    Console.Write("Deleting previous output files... ");
                    if (cFile.ConfirmDeleteFiles(FPNs) == false)
                    {
                        sThisSimulation.mGRMSetupIsNormal = false;
                        return false;
                    }
                }

                if (deleteOnly == true)
                    return true;

                if (cProject.Current.generalSimulEnv.mPrintOption == cGRM.GRMPrintType.All)
                {
                    if (project.generalSimulEnv.mbMakeRasterOutput == true)
                    {
                        List<string> Dpath = new List<string>();
                        if (project.generalSimulEnv.mbShowSoilSaturation == true)
                            Dpath.Add(project.OFPSSRDistribution);

                        if (project.generalSimulEnv.mbShowRFdistribution == true)
                            Dpath.Add(project.OFPRFDistribution);

                        if (project.generalSimulEnv.mbShowRFaccDistribution == true)
                            Dpath.Add(project.OFPRFAccDistribution);

                        if (project.generalSimulEnv.mbShowFlowDistribution == true)
                            Dpath.Add(project.OFPFlowDistribution);

                        if (Dpath.Count > 0)
                        {
                            if (cFile.ConfirmDeleteDirectory(Dpath) == false)
                            {
                                sThisSimulation.mGRMSetupIsNormal = false;
                                return false;
                            }
                            if (cFile.ConfirmCreateDirectory(Dpath) == false)
                            {
                                sThisSimulation.mGRMSetupIsNormal = false;
                                return false ;
                            }
                        }
                    }
                    if (beenRun == true)
                        Console.WriteLine("completed. ");

                    // 해더
                    string strOutPutLine;
                    string strOutputCommonHeader;
                    strOutputCommonHeader = string.Format("Project name : {0} {1} {2} {3} by {4}{5}", project.ProjectNameWithExtension, "\t", 
                        DateTime.Now.ToString("yyyy/MM/dd HH:mm") , "\t", cGRM.BuildInfo.ProductName, "\r\n");
                    string strLTime_WPName;
                    strLTime_WPName = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME;

                    foreach (Dataset.GRMProject.WatchPointsRow row in project.watchPoint.mdtWatchPointInfo.Rows)
                    {
                        strLTime_WPName = strLTime_WPName + "\t" + "[" + row.Name + "]";
                        // wp 별 출력파일 설정
                        string wpName = row.Name.ToString().Replace(",", "_");
                        string nFPN = Path.Combine(project.ProjectPath, project.ProjectNameOnly + "WP_" + wpName + ".out");
                        FPNs.Clear();
                        FPNs.Add(nFPN);
                        if (cFile.ConfirmDeleteFiles(FPNs) == false)
                        {
                            sThisSimulation.mGRMSetupIsNormal = false;
                            return false;
                        }
                        string strL = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + "\t" + "Discharge[cms]" + "\t" + "BaseFlowDepth[m]" + "\t" + "SoilWaterContent[m]" + "\t" + "SoilSatR" + "\t" + "RFGrid" + "\t" + "RFUpMean" + "\t" + "FCData" + "\t" + "FCResStor" + "\r\n";
                        System.IO.File.AppendAllText(nFPN, strOutputCommonHeader, Encoding.Default);
                        System.IO.File.AppendAllText(nFPN, "Output data : All the results for watch point '" + wpName + "'" + "\r\n" + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(nFPN, strL, Encoding.Default);
                        project.watchPoint.FpnWpOut[row.CVID] = nFPN;
                    }

                    // ----------------------------------------------------
                    // 이건 유량
                    if (!System.IO.Directory.Exists(System.IO.Path.GetDirectoryName(strFNPDischarge)))
                        System.IO.Directory.CreateDirectory(System.IO.Path.GetDirectoryName(strFNPDischarge));
                    System.IO.File.AppendAllText(strFNPDischarge, strOutputCommonHeader, Encoding.Default);
                    strOutPutLine = "Output data : " + "Discharge[CMS]" + "\r\n" + "\r\n";
                    System.IO.File.AppendAllText(strFNPDischarge, strOutPutLine, Encoding.Default);
                    strOutPutLine = strLTime_WPName + "\t" + "Rainfall_Mean" + "\t" + "FromStarting[sec]" + "\r\n";
                    System.IO.File.AppendAllText(strFNPDischarge, strOutPutLine, Encoding.Default);

                    // '----------------------------------------------------
                    // '이건 수심
                    // IO.File.AppendAllText(strFNPDepth, strOutputCommonHeader, Encoding.Default)
                    // strOutPutLine = "Output data : " & "Depth[m]" & vbCrLf & vbCrLf
                    // IO.File.AppendAllText(strFNPDepth, strOutPutLine, Encoding.Default)

                    // strOutPutLine = strLTime_WPName + vbTab + "Rainfall_Mean" + vbTab + "ThisStep[msec]" + vbTab + "FromStarting[sec]" & vbCrLf
                    // IO.File.AppendAllText(strFNPDepth, strOutPutLine, Encoding.Default)

                    // '강우. 
                    // IO.File.AppendAllText(strFNPRFGrid, strOutputCommonHeader, Encoding.Default)
                    // IO.File.AppendAllText(strFNPRFGrid, "Output data : Rainfall for each watchpoint[mm]" + vbCrLf & vbCrLf, Encoding.Default)
                    // IO.File.AppendAllText(strFNPRFGrid, strLTime_WPName & vbCrLf, Encoding.Default)

                    // wp별 유역 평균강우량
                    // IO.File.AppendAllText(strFNPRFMean, strOutputCommonHeader, Encoding.Default)
                    // IO.File.AppendAllText(strFNPRFMean, "Output data : Mean rainfall for upstream of each watchpoint[mm]" + vbCrLf & vbCrLf, Encoding.Default)
                    // IO.File.AppendAllText(strFNPRFMean, strLTime_WPName & vbCrLf, Encoding.Default)

                    // ----------------------------------------------------
                    // 여기는 flow control 모듈 관련
                    string strNameFCApp;
                    string strTypeFCApp;
                    string strSourceDT;
                    string strResOperation;

                    string strROiniStorage;
                    string strROmaxStorage;
                    string strROmaxStorageRatio;
                    string strROmaxStorageApp;
                    string strROType;
                    string strROConstQ;
                    string strROConstQduration;

                    strNameFCApp = "FCName:";
                    strTypeFCApp = "FCType:";
                    strSourceDT = "SourceDT[min]:";
                    strResOperation = "ResOperation:";
                    strROiniStorage = "StorageINI:";
                    strROmaxStorage = "StorageMax:";
                    strROmaxStorageRatio = "StorageMaxRatio:";
                    strROmaxStorageApp = "StorageMaxApp:";
                    strROType = "ROType:";
                    strROConstQ = "ConstQ:";
                    strROConstQduration = "ConstQDuration:";
                    string strFCDataField = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME;
                    if (cProject.Current.generalSimulEnv.mbSimulateFlowControl == true && project.fcGrid.FCCellCount > 0)
                    {
                        foreach (Dataset.GRMProject.FlowControlGridRow row in project.fcGrid.mdtFCGridInfo)
                        {
                            strFCDataField = strFCDataField + "\t" + row.Name;
                            strNameFCApp = strNameFCApp + "\t" + row.Name;
                            strTypeFCApp = strTypeFCApp + "\t" + row.ControlType;
                            if (row.ControlType == cFlowControl.FlowControlType.ReservoirOutflow.ToString() | row.ControlType == cFlowControl.FlowControlType.Inlet.ToString())
                            {
                                strSourceDT = strSourceDT + "\t" + System.Convert.ToString(row.DT);
                                strResOperation = strResOperation + "\t" + "FALSE";
                            }
                            else
                            {
                                strSourceDT = strSourceDT + "\t" + "NONE";
                                if (!row.IsMaxStorageNull() && !row.IsMaxStorageRNull())
                                {
                                    if (System.Convert.ToDouble(row.MaxStorage) * System.Convert.ToDouble(row.MaxStorageR) > 0)
                                        strResOperation = strResOperation + "\t" + "TRUE";
                                    else
                                        strResOperation = strResOperation + "\t" + "FALSE";
                                }
                            }
                            if (!row.IsIniStorageNull() && row.IniStorage != "")
                                strROiniStorage = strROiniStorage + "\t" + row.IniStorage;
                            if (!row.IsMaxStorageNull() && row.MaxStorage != "")
                                strROmaxStorage = strROmaxStorage + "\t" + row.MaxStorage;
                            if (!row.IsMaxStorageRNull() && row.MaxStorageR != "")
                                strROmaxStorageRatio = strROmaxStorageRatio + "\t" + row.MaxStorageR;
                            if (!row.IsMaxStorageRNull() && row.MaxStorageR != "")
                                strROmaxStorageApp = strROmaxStorageApp + "\t" + System.Convert.ToString(System.Convert.ToDouble(row.MaxStorage) * System.Convert.ToDouble(row.MaxStorageR));
                            if (!row.IsROTypeNull() && row.ROType != "")
                                strROType = strROType + "\t" + row.ROType;
                            if (!row.IsROConstQNull() && row.ROConstQ != "")
                                strROConstQ = strROConstQ + "\t" + row.ROConstQ;
                            if (!row.IsROConstQDurationNull() && row.ROConstQDuration != "")
                                strROConstQduration = strROConstQduration + "\t" + row.ROConstQDuration;
                        }

                        // FCApp - flow control data
                        System.IO.File.AppendAllText(strFNPFCData, strOutputCommonHeader, Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCData, "Output data : Flow control data input[CMS]" + "\r\n" + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCData, strNameFCApp + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCData, strTypeFCApp + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCData, strSourceDT + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCData, strResOperation + "\r\n" + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCData, strFCDataField + "\r\n", Encoding.Default);

                        // reservoir operation
                        System.IO.File.AppendAllText(strFNPFCStorage, strOutputCommonHeader, Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, "Output data : Storage data[m^3]" + "\r\n" + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strNameFCApp + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strTypeFCApp + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strROiniStorage + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strROmaxStorage + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strROmaxStorageRatio + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strROmaxStorageApp + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strROType + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strROConstQ + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strROConstQduration + "\r\n" + "\r\n", Encoding.Default);
                        System.IO.File.AppendAllText(strFNPFCStorage, strFCDataField + "\r\n", Encoding.Default);
                    }
                }
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("Error: Some output files are not deleted.        " + "\r\n" + "Click 'Start simulation' button anagin.");
                Console.WriteLine(ex.ToString());
                return false;
            }

            return true;
        }
    }
}
