using System;
using System.Collections.Generic;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Diagnostics;
using gentle;

namespace GRMCore
{
    public class cOutputControlRT
    {
        private PerformanceCounter m_cpuUsage = new PerformanceCounter("Processor", "% Processor Time", "_Total"); // 성능 비교 용
        private PerformanceCounter m_diskUsageC = new PerformanceCounter("PhysicalDisk", "% Disk Time", "0 C:");
        private PerformanceCounter m_diskUsageD = new PerformanceCounter("PhysicalDisk", "% Disk Time", "1 D:");

        public void WriteSimResultsToTextFileAndDBForRealTime(cProject project, int nowT_MIN, double interCoef, cProjectBAK project_tm1, cRealTime RTProject)
        {
            DateTime timeNow;
            long lngTimeDiffFromStarting_SEC;
            string strNowTimeToPrintOut;
            timeNow = DateTime.Now;
            TimeSpan tsTotalSim = timeNow - sThisSimulation.mTimeThisSimulationStarted;
            lngTimeDiffFromStarting_SEC = (long)tsTotalSim.TotalSeconds;
            strNowTimeToPrintOut = cComTools.GetTimeToPrintOut(project.generalSimulEnv.mIsDateTimeFormat, project.generalSimulEnv.mSimStartDateTime, nowT_MIN);


            string strWPName;
            string strFNP;
            string vToPrint = "";
            double sngQobs;
            string strOutPutLine;
            string strSQL_Server = "";       // SQL DB 에도 추가적으로 기입

            foreach (Dataset.GRMProject.WatchPointsRow row in project.watchPoint.mdtWatchPointInfo)
            {
                int cvan = row.CVID - 1;
                if (interCoef == 1)
                {
                    switch (project.CVs[cvan].FlowType)
                    {
                        case  cGRM.CellFlowType.OverlandFlow:
                            {
                                vToPrint = string.Format("@{0,8:#0.##}", project.CVs[cvan].QCVof_i_j_m3Ps);
                                break;
                            }

                        default:
                            {
                                vToPrint = project.CVs[cvan].mStreamAttr.QCVch_i_j_m3Ps.ToString("F2");
                                break;
                            }
                    }
                }
                else if (project_tm1 != null)
                {
                    switch (project.CVs[cvan].FlowType)
                    {
                        case  cGRM.CellFlowType.OverlandFlow:
                            {
                                vToPrint = string.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].QCVof_i_j_m3Ps, project.CVs[cvan].QCVof_i_j_m3Ps, interCoef));
                                break;
                            }

                        default:
                            {
                                vToPrint = string.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(project_tm1.CVs[cvan].mStreamAttr.QCVch_i_j_m3Ps, project.CVs[cvan].mStreamAttr.QCVch_i_j_m3Ps, interCoef));
                                break;
                            }
                    }
                }

                // 여기서 관측자료 받는다.. 직접 받을 수 있는 경우를 대비해서, 자리만 만들어줌..
                sngQobs = 0;
                strWPName = row.Name.ToString().Replace(",", "_");

                // 실시간 모델링 텍스트 파일 쓰고
                strFNP = project.ProjectPath + @"\" + project.ProjectNameOnly + "RealTime_" + strWPName + ".out";
                strFNP = cRealTime_Common.IO_Path_ChangeDrive(cRealTime.CONST_Output_File_Target_DISK, strFNP);
                strOutPutLine = strNowTimeToPrintOut
                                      + "\t" + string.Format(project.watchPoint.RFUpWsMeanForDtPrintout_mm[row.CVID].ToString(), "#0.00")
                                      + "\t" + String.Format(sngQobs.ToString (), "#0.00") + "\t" + vToPrint
                                      + "\t" + String.Format((lngTimeDiffFromStarting_SEC / (double) 60).ToString(), "#0.00") + "\r\n";

                if (!System.IO.Directory.Exists(System.IO.Path.GetDirectoryName(strFNP)))
                    System.IO.Directory.CreateDirectory(System.IO.Path.GetDirectoryName(strFNP));
                System.IO.File.AppendAllText(strFNP, strOutPutLine, Encoding.Default);
                Log_Performance_data(project.ProjectNameOnly, strWPName, strNowTimeToPrintOut, lngTimeDiffFromStarting_SEC / 60.0); // 성능 비교 분석용 정보 수집

                if (cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem)
                {
                    SqlConnection oSQLCon = new SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
                    if (oSQLCon.State == ConnectionState.Closed)
                        oSQLCon.Open();
                    int intPos = row.Name.IndexOf('_');
                    string strGaugeCode = row.Name.Substring(intPos + 1);

                    // 2017년 방식
                    // Dim strSQL As String = String.Format("insert into Qwatershed_CAL ([WSCODE], GaugeCode, time ,[Value], [RFMean_mm]) values('{0}','{1}',{2},{3},{4})",
                    // project.ProjectNameOnly,
                    // strGaugeCode,
                    // cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut),
                    // vToPrint,
                    // project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID),
                    // cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME.ToString)

                    // 2018년 방식 by 원 2018.8.8 
                    // 2018.8.10 이전에 9999 였고. 이제 test 위해 g_performance_log_GUID 사용중
                    string strSQL = string.Format("insert into [Q_CAL] (runid,WPName, [Time],[value], RFMean_mm) values({0},'{1}','{2}',{3},{4})", 
                        cRealTime_DBMS.g_RunID, strWPName, cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut), vToPrint, project.watchPoint.RFUpWsMeanForDtPrintout_mm[row.CVID]);

                    SqlCommand oSQLCMD = new SqlCommand(strSQL, oSQLCon);
                    int intRetVal = oSQLCMD.ExecuteNonQuery();
                    if (intRetVal != 1)
                    {
                        System.Console.WriteLine(strSQL + "\r\n" + project.ProjectPath);
                        throw new ApplicationException("Error : " + strSQL);
                    }
                    oSQLCon.Close();
                }
                else
                {
                }
            }
        }

        public void Log_Performance_data(string strBasin, string strTag, string strDataTime, double dblElapTime)
        {
            // 성능 비교 용도. 개별 프로세스. launcher .exe 에 집중. 머신 전체는 monitor에서 측정하도록함
           SqlConnection oSQLCon = new SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
            if (oSQLCon.State == ConnectionState.Closed)
                oSQLCon.Open();
            string strSQL = string.Format("insert into run_perf ([basin],[RainfallDataCompleted],[ElapsedTime_Min],[net_process_PrivateMemorySize64],[run_meta_guid],[OutputDrive],[tag]) values('{0}','{1}',{2},{3},'{4}','{5}','{6}')", 
                strBasin, strDataTime, dblElapTime, Process.GetCurrentProcess().PrivateMemorySize64 / (double)1024 / 1024 / 1024, cRealTime_DBMS.g_RunID, cRealTime.CONST_Output_File_Target_DISK, strTag);
            System.Data.SqlClient.SqlCommand oSQLCMD = new System.Data.SqlClient.SqlCommand(strSQL, oSQLCon);
            int intRetVal = oSQLCMD.ExecuteNonQuery();
            if (intRetVal != 1)
                Console.WriteLine("perf logging error");
            oSQLCon.Close();
        }

        public static bool CreateNewOutputTextFileRT(cProject project, cRealTime RTproject)
        {
            try
            {
                string strWPName;
                string strFNP;
                string strOutPutLine;
                List<string> FPNs;
                Console.Write("Deleting previous output files... ");
                foreach (Dataset.GRMProject.WatchPointsRow row in project.watchPoint.mdtWatchPointInfo)
                {
                    strWPName = row.Name.ToString().Replace(",", "_");
                    strFNP = project.ProjectPath + @"\" + project.ProjectNameOnly + "RealTime_" + strWPName + ".out";
                    FPNs = new List<string>();
                    FPNs.Add(strFNP);
                    cFile.ConfirmDeleteFiles(FPNs);
                    System.IO.File.Delete(strFNP);
                    strOutPutLine = "GRM real time simulation results for watchpoint " + strWPName + ": Discharge[CMS]  by "
                                            + cGRM.BuildInfo.ProductName + "\r\n";
                    strOutPutLine = strOutPutLine + cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + "\t" + "RFMean[mm]" + "\t" + "Qobs[CMS]" + "\t" + "Qsim[CMS]" + "\t" + "TimeFormStarting[MIN]" + "\r\n";
                    System.IO.File.AppendAllText(strFNP, strOutPutLine, Encoding.Default);
                }
                if (project.generalSimulEnv.mbMakeRasterOutput == true)
                {
                    List<string> Dpath = new List<string>();
                    Dpath.Add(project.OFPSSRDistribution);
                    Dpath.Add(project.OFPRFDistribution);
                    Dpath.Add(project.OFPRFAccDistribution);
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
                            return false;
                        }
                    }
                }
                Console.WriteLine("completed. ");
                if (cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem == false)
                {
                }

                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return false;
            }
            return true;
        }
    }
}
