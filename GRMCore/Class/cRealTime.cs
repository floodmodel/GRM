using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Data;
using System.Data.SqlClient;

namespace GRMCore
{
    public class cRealTime
    {
        public const bool CONST_bUseDBMS_FOR_RealTimeSystem = true;    // Real time에서는 true로 설정
        public static char CONST_Output_File_Target_DISK = System.Convert.ToChar("?"); // png 등의 모의 결과를 c ,.d  어디에 기입할지. 구분 
        public event RTStatusEventHandler RTStatus;

        public delegate void RTStatusEventHandler(string strMSG);

        public cRainfall.RainfallDataType mRainfallDataTypeRT;
        public string mRfFilePathRT;
        public int mDtPrintOutRT_min;
        public int mSimDurationrRT_Hour;
        public string mRFStartDateTimeRT;
        public bool mbNewRFAddedRT;
        public Dictionary<int, bool> mdicBNewFCdataAddedRT;   // CVID 로 구분
       // Public mdicBNewFCdataAddedRT_v2018 As Dictionary(Of String, Boolean)    'DAM 으로 구분 . 굳이 이리할 필요 없어서. 도입하지 않음. 2018.8.28
        public List<cRainfall.RainfallData> mlstRFdataRT;
        /// <summary>
        /// Get data count by Cvid
        /// </summary>
        /// <remarks></remarks>
        public Dictionary<int, int> mdicFCDataCountForEachCV;
        public Dictionary<int, string> mdicFCNameForEachCV;
        public Dictionary<int, int> mdicFCDataOrder;
        public int mRFLayerCountToApply_RT;
        public DateTime mDateTimeStartRT;
        public bool mbSimulationRTisOngoing;
        private static cRealTime mGRMRT;
        private cProject mRTProject;
        // Public mFPNFcData As String
        public float mPicWidth;
        public float mPicHeight;
        private cRasterOutput mRasterFileOutput;

        public bool mbIsDWSS;
        public int mCWCellColX_ToConnectDW;
        public int mCWCellRowY_ToConnectDW;
        public int mDWCellColX_ToConnectCW;
        public int mDWCellRowY_ToConnectCW;

        private cSimulator mSimul = new cSimulator();
        private DataTable m_odt_flowcontrolinfo;

        public static void InitializeGRMRT()
        {
            string strTmp = File.ReadAllText(@"C:\Nakdong\outputDrive.txt");
            if (strTmp.ToUpper() != "C" & strTmp.ToUpper() != "D" & strTmp.ToUpper() != "S")
            {
                Console.WriteLine("Can not Read " + strTmp);
                System.Diagnostics.Debugger.Break();
            }
            CONST_Output_File_Target_DISK = System.Convert.ToChar(strTmp);
            mGRMRT = new cRealTime();

            mGRMRT.RTStatus += new RTStatusEventHandler(mGRMRT.cRealTime_RTStatus);
        }

        public void SetupGRM(string FPNprj) // fc 자료는 항상 db를 사용하는 것으로 수정, Optional FPNfcdata As String = "")
        {
            if (cProject.OpenProject(FPNprj, true) == false)
            {
                RTStatus("모형 설정 실패.");
                if (CONST_bUseDBMS_FOR_RealTimeSystem)
                    cRealTime_DBMS.Add_Log_toDBMS(System.IO.Path.GetFileName(FPNprj), "Fail in Model Setting"); //  '모형 설정 실패.  2018.11.20 문구 수정함
                return;
            }
            mRTProject = cProject.Current;
            mRTProject.mSimulationType = cGRM.SimulationType.RealTime;
            if (mRTProject.SetupModelParametersAfterProjectFileWasOpened() == false)
            {
                cGRM.writelogAndConsole("GRM setup was failed !!!", true, true);
                return;
            }
            if (cOutPutControl.CreateNewOutputFiles(cProject.Current, true) == false)
                cGRM.writelogAndConsole("Deleting single event output files were failed !!!", true, true);

            RTStatus("모형 설정 완료.");
            if (CONST_bUseDBMS_FOR_RealTimeSystem)
                cRealTime_DBMS.Add_Log_toDBMS(mRTProject.ProjectNameOnly, "Model Setting Completed."); //'모형 설정 완료. 2018.11.20 문구 수정
        }

        public void RunGRMRT()
        {
            sThisSimulation.mGRMSetupIsNormal = true;
            if (mRTProject.generalSimulEnv.mbMakeRasterOutput == true)
            {
                mRTProject.mImgFPN_dist_Flow = new List<string>();
                mRTProject.mImgFPN_dist_RF = new List<string>();
                mRTProject.mImgFPN_dist_RFAcc = new List<string>();
                mRTProject.mImgFPN_dist_SSR = new List<string>();
                mRasterFileOutput = new cRasterOutput(mRTProject);
            }
            int dateY = 0;
            int dateM = 0;
            int dateD = 0;
            int timeH = 0;
            int timeM = 0;
            int v = 0;
            if (int.TryParse(mRFStartDateTimeRT.Substring(0, 4), out v) == true) { dateY = v; }
            if (int.TryParse(mRFStartDateTimeRT.Substring(4, 2), out v) == true) { dateM = v; }
            if (int.TryParse(mRFStartDateTimeRT.Substring(6, 2), out v) == true) { dateD = v; }
            if (int.TryParse(mRFStartDateTimeRT.Substring(8, 2), out v) == true) { timeH = v; }
            if (int.TryParse(mRFStartDateTimeRT.Substring(10, 2), out v) == true) { timeM = v; }
            mDateTimeStartRT = new DateTime(dateY, dateM, dateD, timeH, timeM, 0);
            //mDateTimeStartRT = new DateTime((int)mRFStartDateTimeRT.Substring(1, 4), (int)mRFStartDateTimeRT.Substring(5, 2), (int)mRFStartDateTimeRT.Substring (7, 2), 
            //    (int)mRFStartDateTimeRT.Substring(9, 2), (int)mRFStartDateTimeRT.Substring(11, 2), 0);
            mSimDurationrRT_Hour = 24 * 100; // 충분히 100일 동안 모의하는 것으로 설정
            mbSimulationRTisOngoing = true;
            mlstRFdataRT = new List<cRainfall.RainfallData>();
            mRFLayerCountToApply_RT = 0;
            if (CONST_bUseDBMS_FOR_RealTimeSystem)
            {
                bool doit = true;
                if (doit == false)//'2018.8 부터 이제 과거 분석 기록은 보존됨..그래서 삭제 code는 미수행.
                {
                    Clear_DBMS_Table_Qwatershed(mRTProject.ProjectNameOnly);
                    RTStatus("DBMS [Q_CAL] Table Cleared");
                    cRealTime_DBMS.Add_Log_toDBMS(mRTProject.ProjectNameOnly, "DBMS [Q_CAL] Table Cleared");
                }
            }

            RTStatus("RealTime Rainall Runoff Start.."); // '실시간 유출해석 시작.. 2018.11.20 한글 -> 영문 변경함
            if (CONST_bUseDBMS_FOR_RealTimeSystem)
                cRealTime_DBMS.Add_Log_toDBMS(mRTProject.ProjectNameOnly, "RealTime Rainall Runoff Start..");

            mSimul = new cSimulator();
            if (CreateNewOutputFilesRT() == false)
                return;

            string ascFPN;
            
            if (cRealTime_Common.g_strModel == "")
            {
                ascFPN = mRfFilePathRT + @"\" + GetYearAndMonthFromyyyyMMddHHmm(mRFStartDateTimeRT) + @"\" + mRFStartDateTimeRT + ".asc";
            }
            else
            {
                //mRFStartDateTimeRT  시작 시점
                string strDIFF = (DateTime.ParseExact(mRFStartDateTimeRT, "yyyyMMddHHmm", null)-DateTime.ParseExact(cRealTime_Common.g_strTimeTagBase_KST, "yyyyMMddHH",null)  ).TotalHours.ToString("000");
                // 가정.. ref의 시작 시간을.. l030_v070_m00_h004.2016100400.gb2_1_clip.asc,,  에 맞추고... h000만 조정...
                // 즉 KST로 시작 시간 지정은. 201610040900 + 4  즉 2016100413이 됨
                
                string strFileLEns = string.Format("l030_v070_{0}_h{1}.{2}.gb2_1_clip.asc",   cRealTime_Common.g_strModel , strDIFF, cRealTime_Common.g_strTimeTagBase_UCT);
                ascFPN = mRfFilePathRT + @"\" + strFileLEns ;
            }

            if (System.IO.File.Exists(ascFPN) == false)
            {
                RTStatus("유출해석 시작 시간에서의 강우자료가 없습니다.");
                RTStatus("강우자료와 유출해석 시작 시간을 확인하시길 바랍니다.");
            }

            if (mRTProject.fcGrid.IsSet == true)
            {
                mdicFCDataCountForEachCV = new Dictionary<int, int>();
                mdicFCNameForEachCV = new Dictionary<int, string>();
                mdicFCDataOrder = new Dictionary<int, int>();
                mdicBNewFCdataAddedRT = new Dictionary<int, bool>();
                foreach (int id in mRTProject.fcGrid.FCGridCVidList)
                {
                    mdicFCDataCountForEachCV.Add(id, 0);
                    mdicBNewFCdataAddedRT.Add(id, true);
                    mdicFCDataOrder.Add(id, 0);
                }
                // If mGRMRT.mFPNFcData.Trim <> "" AndAlso IO.File.Exists(mGRMRT.mFPNFcData) = False Then
                // System.Console.WriteLine(String.Format("Realtime flow control data file is not valid. {0} {1}", vbCrLf, mGRMRT.mFPNFcData))
                // Exit Sub
                // End If
                mRTProject.fcGrid.mdtFCFlowData = new DataTable();
                ReadDBorCSVandMakeFCdataTableForRealTime_v2018(mRFStartDateTimeRT);
                if (mRTProject.fcGrid.mdtFCFlowData.Rows.Count < 1)
                {
                    RTStatus("유출해석 시작 시간에서의 flow control 자료가 없습니다.");
                    RTStatus("유출해석 시작시간과 댐방류량, inlet 자료 등 flow control 자료를 확인하시길 바랍니다.");
                }
            }
            mSimul.SimulateRT(mRTProject, this);
        }

        // 2018.8.8 이제 부터는 과거  run 도 보존 . 그래서 이 함수는 미사용됨.
        private void Clear_DBMS_Table_Qwatershed(string strName)
        {
            System.Data.SqlClient.SqlConnection oSQLCon = new System.Data.SqlClient.SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
            if (oSQLCon.State == ConnectionState.Closed)
                oSQLCon.Open();

            // Dim strSQL As String = String.Format("delete [Q_CAL] where wscode='{0}'", strName)
            string strSQL = string.Format("delete [Q_CAL] where runid={0}", cRealTime_Common.g_performance_log_GUID);   // '2018.8.8 부터 임시 적용. 한시적

            SqlCommand oSQLCMD = new SqlCommand(strSQL, oSQLCon);
            int intAffectedRecords = oSQLCMD.ExecuteNonQuery();
            string strMsg = string.Format("[Q_CAL] Table에서 {0} 유역 {1}건 Data 삭제됨. 초기화 완료.", strName, intAffectedRecords);
            RTStatus(strMsg);
            cRealTime_DBMS.Add_Log_toDBMS(strName, strMsg);
        }

        private bool CreateNewOutputFilesRT()
        {
            if (!cOutputControlRT.CreateNewOutputTextFileRT(mRTProject, mGRMRT))
                return false;
            return true;
        }

        public void UpdateFcDatainfoGRMRT(string strDate, int cvid, int previousOrder, int dtMIN)
        {
            string fcname = mRTProject.fcGrid.GetFCName(cvid);
            DataRow[] drs = mRTProject.fcGrid.mdtFCFlowData.Select(string.Format("CVID = {0} and datetime={1}", cvid, strDate));
            if (drs.Length > 0)
            {
                mdicBNewFCdataAddedRT[cvid] = true;
                RTStatus(string.Format("  FC Data 입력완료...({3} {0}, CVID={1}, Value={2})", fcname.PadRight(13), cvid.ToString().PadLeft(5), drs[0].Field<Single>("value").ToString().PadLeft(8), strDate));
            }
            else
            {
                mdicBNewFCdataAddedRT[cvid] = false;
                RTStatus(string.Format("FC 자료({0}, CVID={1}, {2}) 수신대기 중...", fcname, cvid, strDate));
            }
        }

        // 2018.8 CVID 아닌 DAM 이름 방식으로 변경하려던 시도. 현재 이방법 채택하지 않음
        // Public Sub UpdateFcDatainfoGRMRT_v2018(ByVal strDate As String, GName As String,
        // previousOrder As Integer, dtMIN As Integer)

        // 'Dim drs As DataRow() = mRTProject.FCGrid.mdtFCFlowData.Select(String.Format("CVID = {0} and datetime={1}", cvid, strDate))  '2017년 방식은 CVID로 한정 
        // Dim drs As DataRow() = mRTProject.FCGrid.mdtFCFlowData.Select(String.Format("gname = '{0}' and datetime={1}", GName, strDate))  '2018년 방식은 DAM 정보를 받아야 하는거 아니가?

        // If drs.Count > 0 Then
        // mdicBNewFCdataAddedRT_v2018(GName) = True
        // RaiseEvent RTStatus(String.Format("  FC Data 입력완료...({2}, GName={0}, Value={1})",
        // GName, drs(0).Item("value").ToString.PadLeft(8), strDate))
        // Else
        // mdicBNewFCdataAddedRT_v2018(GName) = False
        // RaiseEvent RTStatus(String.Format("FC 자료(GName={0}, {1}) 수신대기 중...",
        // GName, strDate))
        // End If
        // End Sub
        
        public void UpdateRainfallInformationGRMRT(string strDate)
        {
            TimeSpan tsTotalSim = DateTime.Now - sThisSimulation.mTimeThisSimulationStarted;
            //long lngTimeDiffFromStarting_SEC = DateDiff(DateInterval.Second, cThisSimulation.mTimeThisSimulationStarted, DateTime.Now);
            long lngTimeDiffFromStarting_SEC = (long)tsTotalSim.TotalSeconds;
            string tFromStart = (lngTimeDiffFromStarting_SEC / (double)60).ToString("#0.00");

            if (mRFLayerCountToApply_RT > 0 & mbNewRFAddedRT == true)
            {
                cRainfall rf = new cRainfall();
                cRainfall.RainfallData row = rf.GetRFdataByOrder(mlstRFdataRT, mRFLayerCountToApply_RT);
                RTStatus(row.FileName + " 분석완료 .. " + tFromStart + "분 경과");
                //System.Windows.Forms.Application.DoEvents();
                mbNewRFAddedRT = false;
            }
            switch (mRainfallDataTypeRT)
            {
                case cRainfall.RainfallDataType.TextFileASCgrid_mmPhr:
                case cRainfall.RainfallDataType.TextFileASCgrid:
                    {
                        // Dim strFilenameOnly As String = strDate   '2017년 방식
                        string strFilenameOnly;    // 2018년 8.8 현재 산출 naming
                        string ascFPN;        // 2018년 8.8 현재 산출 naming

                        if (cRealTime_Common.g_strModel == "")
                        {
                            strFilenameOnly = "RDR_COMP_ADJ_" + strDate + ".RKDP.bin";    
                            ascFPN = mRfFilePathRT + @"\" + GetYearAndMonthFromyyyyMMddHHmm(strDate) + @"\" + strFilenameOnly + ".asc";       
                        }
                        else
                        {
                            string strDIFF = (DateTime.ParseExact(strDate, "yyyyMMddHHmm", null) - DateTime.ParseExact(cRealTime_Common.g_strTimeTagBase_KST, "yyyyMMddHH", null)).TotalHours.ToString("000");
                            // 가정.. ref의 시작 시간을.. l030_v070_m00_h004.2016100400.gb2_1_clip.asc,,  에 맞추고... h000만 조정...
                            // 즉 KST로 시작 시간 지정은. 201610040900 + 4  즉 2016100413이 됨

                            if (strDIFF == "073") { Console.WriteLine("LENS : completed"); Environment.Exit(0); }

                            string strFileLEns = string.Format("l030_v070_{0}_h{1}.{2}.gb2_1_clip", cRealTime_Common.g_strModel, strDIFF, cRealTime_Common.g_strTimeTagBase_UCT);
                            strFilenameOnly = strFileLEns;
                            ascFPN = mRfFilePathRT + @"\" +  strFilenameOnly + ".asc";
                        }

                        if (System.IO.File.Exists(ascFPN) == true)
                        {
                            mRFLayerCountToApply_RT += 1;
                            sThisSimulation.mRFDataCountInThisEvent = mRFLayerCountToApply_RT;
                            cRainfall.RainfallData nr;
                            nr.Order = mRFLayerCountToApply_RT;
                            nr.DataTime = strDate;
                            nr.Rainfall = strFilenameOnly + ".asc"; // 

                            //LENS 인 경우 yyyymm 폴더 구분 하지 않음
                            if (cRealTime_Common.g_strModel == "")
                                { nr.FilePath = mRfFilePathRT + @"\" + GetYearAndMonthFromyyyyMMddHHmm(strDate); }
                            else
                                { nr.FilePath = mRfFilePathRT ; }
                                
                            nr.FileName = strFilenameOnly + ".asc";
                            mlstRFdataRT.Add(nr);
                            RTStatus(nr.FileName + " 입력완료(강우)");
                            //System.Windows.Forms.Application.DoEvents();
                            mbNewRFAddedRT = true;
                            return;
                        }

                        break;
                    }
            }
            RTStatus(string.Format("강우자료({0}) 수신대기 중..", strDate));
        }

        public void ReadDBorCSVandMakeFCdataTableForRealTime_v2017(string TargetDateTime, string CSVFPNsource = "")
        {
            DataTable dt = new DataTable();
            if (CONST_bUseDBMS_FOR_RealTimeSystem == true)
            {
                if (m_odt_flowcontrolinfo == null)
                {
                    m_odt_flowcontrolinfo = new DataTable();
                    string strSQL = string.Format("select * from flowcontrolinfo where WScode='{0}'", cProject.Current.ProjectNameOnly);
                    SqlDataAdapter oSqlDataAdapter = new SqlDataAdapter(strSQL, cRealTime_DBMS.g_strDBMSCnn);
                    oSqlDataAdapter.SelectCommand.CommandTimeout = 60;
                    try
                    {
                        oSqlDataAdapter.Fill(m_odt_flowcontrolinfo);
                        oSqlDataAdapter.Dispose();
                    }
                    catch (SqlException ex1)
                    {
                        Console.WriteLine(ex1.ToString());
                        Console.ReadLine();
                        System.Diagnostics.Debugger.Break();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        Console.ReadLine();
                        System.Diagnostics.Debugger.Break();
                    }
                }

                foreach (DataRow oDR in m_odt_flowcontrolinfo.Rows)
                {
                    string strCVID = oDR.Field<string>("CVID");
                    string strSourceName = "";
                    string strSourceTableName = "";
                    string strSourceTarget = oDR.Field<string>("SourceType");
                    string strFieldTarget = "";
                    string strWhereGage = "";
                    switch (strSourceTarget)
                    {
                        case "DAM":
                            {
                                strSourceTableName = "QDAM_OBS";
                                strSourceName = oDR.Field<string>("SourceGaugeCode");
                                strFieldTarget = "GaugeCode";
                                break;
                            }

                        case "WS":
                            {
                                strSourceTableName = "QWatershed_CAL";
                                strSourceName = oDR.Field<string>("SourceWSCode");
                                strFieldTarget = "WSCode";
                                if (strSourceName.Trim() == "GM" | strSourceName.Trim() == "EB")
                                    strWhereGage = string.Format("gaugecode='{0}' and", strSourceName);
                                else
                                    strWhereGage = "gaugecode='md' and";
                                break;
                            }

                        default:
                            {
                                System.Diagnostics.Debugger.Break();
                                break;
                            }
                    }

                    // SQL for time series
                    string strSQL_TS = string.Format("select  {0} as cvid, time as datetime, value from {1} where {4} {3} ='{2}' and time='{5}'", strCVID, strSourceTableName, strSourceName, strFieldTarget, strWhereGage, TargetDateTime);
                    System.Data.DataTable odt_TS = new System.Data.DataTable();
                    SqlDataAdapter oSqlDataAdapter_TS = new SqlDataAdapter(strSQL_TS, cRealTime_DBMS.g_strDBMSCnn);
                    oSqlDataAdapter_TS.SelectCommand.CommandTimeout = 60;
                    oSqlDataAdapter_TS.Fill(odt_TS);
                    oSqlDataAdapter_TS.Dispose();
                    dt.Merge(odt_TS);
                }
                cProject.Current.fcGrid.mdtFCFlowData = dt;
                bool doit = true;
                if (doit == false)
                {
                    mRTProject.fcGrid.mdtFCFlowData.TableName = "tmp";
                    mRTProject.fcGrid.mdtFCFlowData.WriteXml(@"C:\temp\read_method_" + cProject.Current.ProjectNameOnly + "_" + DateTime.Now.ToString("yyMMddHHmmss") + "grm.xml");
                }
            }
            else
            {
                dt.Columns.Add(new global::System.Data.DataColumn("CVID", typeof(int), null, global::System.Data.MappingType.Element));
                dt.Columns.Add(new global::System.Data.DataColumn("DataTime", typeof(string), null, global::System.Data.MappingType.Element));
                dt.Columns.Add(new global::System.Data.DataColumn("Value", typeof(float), null, global::System.Data.MappingType.Element));
                int intL = 0;
                StreamReader reader = new StreamReader(CSVFPNsource, System.Text.Encoding.Default);
                while (!reader.EndOfStream)
                {
                    string aLine = reader.ReadLine();
                    string[] parts = aLine.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries);

                    if (parts[0].Trim() == "")
                        break;

                    int nFieldCount = parts.Length;
                    if (intL > 0 && parts[1] == TargetDateTime)
                    {
                        DataRow nr = dt.NewRow();
                        nr["CVID"] = System.Convert.ToInt32(parts[0]);
                        nr["DataTime"] = parts[1];
                        nr["Value"] = System.Convert.ToDouble(parts[2]);
                        mRTProject.fcGrid.mdtFCFlowData.Rows.Add(nr);
                    }
                    intL += 1;
                }
            }
        }




        public void ReadDBorCSVandMakeFCdataTableForRealTime_v2018(string TargetDateTime, string CSVFPNsource = "")
        {
            DataTable dt = new DataTable();

            if (CONST_bUseDBMS_FOR_RealTimeSystem)
            {
                // 2018.8.28 원 : CVID 이건 DB에 없슴. 격자 규격 col, row 위치가 변경시에 CVID도 변경될 것임. 그래서 정적인 DB에 구성 안하는 의도로 예상됨.  최초 gmp road 등 하는 단계에서 temp table로 구성해도 좋겠슴.

                string strSQL = string.Format("Select  w.name, 999 as cvid ,[Time] as datetime ,[QValue] AS VALUE  From [QDam_OBS] d , WatchPoint w Where d.Gname=w.Gname and w.fc=1 And TIME ='{0}' ", TargetDateTime);

                System.Data.DataTable odt = new System.Data.DataTable();
                SqlDataAdapter oSqlDataAdapter = new SqlDataAdapter(strSQL, cRealTime_DBMS.g_strDBMSCnn);
                oSqlDataAdapter.SelectCommand.CommandTimeout = 60;
                oSqlDataAdapter.Fill(odt);

                foreach (DataRow oDR in odt.Rows)
                {
                    DataRow oDR_Target = mRTProject.fcGrid.mdtFCGridInfo.Select(string.Format("Name='{0}'", oDR.Field<string>("NAME"))).FirstOrDefault();
                    //string strCVID = oDR_Target.Field<string>("CVID");
                    string strCVID = oDR_Target.Field<Int32>("CVID").ToString();
                    oDR["CVID"] = strCVID;
                    //Debug.Print(strCVID);
                }
                dt.Merge(odt);

                // 2018.9.3 원  : auto rom 목록도 추가 해주는 처리 시도... 가 필요하다고 생각 했는데. 없어도 잘됨.  
                // Dim oDR_AutoROMs() As DataRow = mRTProject.FCGrid.mdtFCGridInfo.Select(String.Format("rotype='AutoROM'"))
                // Dim odt_auto As New Data.DataTable
                // For Each oDR As DataRow In oDR_AutoROMs
                // Dim nr As DataRow = dt.NewRow
                // nr("CVID") = oDR.Item("CVID").ToString
                // nr("datetime") = TargetDateTime
                // nr("Value") = ""
                // odt_auto.Rows.Add(nr)
                // Next
                // dt.Merge(odt_auto)

                //경천DAM 처리.영주댐 처리 부분 인데... 너무 지엽적인 예외 조치 라서... 배제하는시도를 2018.10.12 원,안 하고 있슴
                if (false)
                {
                    // Dim strSpcealDams As String = "'경천댐'"       '2018.8.29 원 : 여기서 n 개 기입... 이건 추후 DB 등으로 이동되어야 함
                    string strSpcealDams = "'경천댐','영주댐'";       // 2018.8.29 원 : 여기서 n 개 기입... 이건 추후 DB 등으로 이동되어야 함..  2018.10/11 원 : 영주댐 추가
                    string strSQL2 = string.Format("Select  w.name, 999 as cvid ,[Time] as datetime ,[QValue] AS VALUE From QStream_OBS_ht d , WatchPoint w  Where  d.GName in({1}) and  TIME ='{0}' and d.Gname=w.Gname ", TargetDateTime, strSpcealDams);

                    System.Data.DataTable odt2 = new System.Data.DataTable();
                    SqlDataAdapter oSqlDataAdapter2 = new SqlDataAdapter(strSQL2, cRealTime_DBMS.g_strDBMSCnn);
                    oSqlDataAdapter2.SelectCommand.CommandTimeout = 60;
                    oSqlDataAdapter2.Fill(odt2);

                    if (odt2.Rows.Count != 2)
                        // Stop   '2018.10.11 까지는 stop 이었슴
                        cGRM.writelogAndConsole(strSpcealDams + " 의 data가 2건이 아님!", false, true);

                    foreach (DataRow oDR2 in odt2.Rows)
                    {
                        DataRow oDR_Target2 = mRTProject.fcGrid.mdtFCGridInfo.Select(string.Format("Name='{0}'", oDR2["NAME"].ToString())).FirstOrDefault();
                        string strCVID2 = oDR_Target2["CVID"].ToString();
                        oDR2["CVID"] = strCVID2;
                        //Debug.Print(strCVID2);
                    }

                    dt.Merge(odt2);
                }


                cProject.Current.fcGrid.mdtFCFlowData = dt;
            }
            else
            {
                dt.Columns.Add(new global::System.Data.DataColumn("CVID", typeof(int), null, global::System.Data.MappingType.Element));
                dt.Columns.Add(new global::System.Data.DataColumn("DataTime", typeof(string), null, global::System.Data.MappingType.Element));
                dt.Columns.Add(new global::System.Data.DataColumn("Value", typeof(float), null, global::System.Data.MappingType.Element));
                int intL = 0;
                StreamReader reader = new StreamReader(CSVFPNsource, System.Text.Encoding.Default);
                while (!reader.EndOfStream)
                {
                    string aLine = reader.ReadLine();
                    string[] parts = aLine.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries);

                    if (parts[0].Trim() == "")
                        break;

                    int nFieldCount = parts.Length;
                    if (intL > 0 && parts[1] == TargetDateTime)
                    {
                        DataRow nr = dt.NewRow();
                        nr["CVID"] = System.Convert.ToInt32(parts[0]);
                        nr["DataTime"] = parts[1];
                        nr["Value"] = System.Convert.ToDouble(parts[2]);
                        mRTProject.fcGrid.mdtFCFlowData.Rows.Add(nr);
                    }
                    intL += 1;
                }
            }
        }

        // Public Sub ReadDBorCSVandMakeFCdataTableForRealTime_v2018_old(TargetDateTime As String, Optional CSVFPNsource As String = "")
        // '2018.8.28 원 : 이 방법은. DB 에서 건건히 가져오는 방법임.. 비효율 

        // Dim dt As New DataTable

        // If CONST_bUseDBMS_FOR_RealTimeSystem Then
        // Dim strSQL As String

        // If False Then   '// 2018.8.28 원 : DB에서 FC 만 정확히 관리해 주면 이부분 불필요함
        // Dim strNameS As String = ""
        // For Each oDR As DataRow In mRTProject.FCGrid.mdtFCGridInfo.Rows
        // Dim strName As String = oDR.Item("Name").ToString  '"ADS_WP4"
        // strNameS = strNameS + String.Format(",'{0}'", strName)
        // Next
        // If strNameS.StartsWith(",") Then strNameS = strNameS.Substring(1)

        // '// GNAME 얻기
        // strSQL = String.Format("SELECT distinct GName,name  FROM WatchPoint where [name] in ({0})", strNameS)
        // End If

        // strSQL = "SELECT GName,name  from  [WatchPoint] where fc=1"

        // Dim odt As New Data.DataTable
        // Dim oSqlDataAdapter As New SqlClient.SqlDataAdapter(strSQL, g_strDBMSCnn)
        // oSqlDataAdapter.SelectCommand.CommandTimeout = 60
        // oSqlDataAdapter.Fill(odt)
        // Dim strGNameS As String = ""

        // For Each oDR As DataRow In odt.Rows
        // Dim strGName As String = oDR.Item(0).ToString
        // Dim strName As String = oDR.Item(1).ToString
        // 'strGNameS = strGNameS + String.Format(",'{0}'", strName)

        // 'If strGNameS.StartsWith(",") Then strGNameS = strGNameS.Substring(1)
        // 'Dim strSQL_TS As String = String.Format("select  {0} as cvid, time as datetime, value from {1} where {4} {3} ='{2}' and time='{5}'",
        // Dim drS As DataRow() = mRTProject.FCGrid.mdtFCGridInfo.Select(String.Format("Name='{0}'", strName))
        // Dim strCVID As String = drS(0).Item("CVID").ToString

        // Dim strSQL_TS As String = String.Format("Select name,  {0} as cvid     ,[Time] as datetime      ,[QValue] AS VALUE  From [QDam_OBS] Where GName = '{1}' And TIME ='{2}' ", strCVID, strGName, TargetDateTime)
        // '
        // Dim odt_TS As New Data.DataTable
        // Dim oSqlDataAdapter_TS As New SqlClient.SqlDataAdapter(strSQL_TS, g_strDBMSCnn)
        // oSqlDataAdapter_TS.SelectCommand.CommandTimeout = 60
        // oSqlDataAdapter_TS.Fill(odt_TS)
        // oSqlDataAdapter_TS.Dispose()
        // dt.Merge(odt_TS)
        // Next

        // cProject.Current.FCGrid.mdtFCFlowData = dt

        // Else    'DBMS 방식이 아닌 경우
        // dt.Columns.Add(New Global.System.Data.DataColumn("CVID", GetType(Integer), Nothing, Global.System.Data.MappingType.Element))
        // dt.Columns.Add(New Global.System.Data.DataColumn("DataTime", GetType(String), Nothing, Global.System.Data.MappingType.Element))
        // dt.Columns.Add(New Global.System.Data.DataColumn("Value", GetType(Single), Nothing, Global.System.Data.MappingType.Element))
        // Dim intL As Integer = 0
        // Using oTextReader As New FileIO.TextFieldParser(CSVFPNsource, Encoding.Default)
        // oTextReader.TextFieldType = FileIO.FieldType.Delimited
        // oTextReader.SetDelimiters(",")
        // oTextReader.TrimWhiteSpace = True
        // Dim TextIncurrentRow As String()
        // While Not oTextReader.EndOfData
        // TextIncurrentRow = oTextReader.ReadFields
        // For Each ele As String In TextIncurrentRow
        // If Trim(TextIncurrentRow(0)).ToString = "" Then Exit While
        // Next
        // Dim nFieldCount As Integer = TextIncurrentRow.Length
        // If intL > 0 AndAlso TextIncurrentRow(1) = TargetDateTime Then
        // Dim nr As DataRow = dt.NewRow
        // nr("CVID") = CInt(TextIncurrentRow(0))
        // nr("DataTime") = TextIncurrentRow(1)
        // nr("Value") = CSng(TextIncurrentRow(2))
        // mRTProject.FCGrid.mdtFCFlowData.Rows.Add(nr)
        // End If
        // intL += 1
        // End While
        // End Using
        // End If
        // End Sub

        public void StopGRM()
        {
            mSimul.StopSimulation();
            RTStatus("실시간 모델링 종료");
        }


        /// <summary>
        /// 모델링 기간과 출력 시간간격을 이용해서 모델링 끝나는 시간을 계산[sec]
        /// </summary>
        /// <remarks> </remarks>
        public int EndingTime_SEC
        {
            get
            {
                return System.Convert.ToInt32(mSimDurationrRT_Hour * 3600);
            }
        }

        private string GetYearAndMonthFromyyyyMMddHHmm(string INPUTyyyyMMddHHmm)
        {
            //return INPUTyyyyMMddHHmm.Substring(1, 6);
            return INPUTyyyyMMddHHmm.Substring(0, 6);
        }

        public static cRealTime Current
        {
            get
            {
                return mGRMRT;
            }
        }

        private void mSimul_SimulationComplete(cSimulator sender)
        {
            RTStatus("분석종료");
        }

        private void mSimul_SimulationRaiseError(cSimulator sender, cSimulator.SimulationErrors simulError, object erroData)
        {
            switch (simulError)
            {
                case cSimulator.SimulationErrors.OutputFileCreateError:
                    {
                        RTStatus("출력파일 생성오류");
                        break;
                    }
            }
        }

        private void mSimul_SimulationStop(cSimulator sender)
        {
            RTStatus("분석종료");
        }

        private void mSimul_CallAnalyzer(cSimulator sender, int nowTtoPrint_MIN)
        {
            if (mRTProject.generalSimulEnv.mbMakeRasterOutput == true)
                mRasterFileOutput.MakeDistributionFiles(nowTtoPrint_MIN, mRasterFileOutput.ImgWidth, mRasterFileOutput.ImgHeight, true);
        }

        private void cRealTime_RTStatus(string strMSG)
        {
            cGRM.writelogAndConsole(strMSG, cGRM.bwriteLog, true);    //2018.11.29 원 : 임시 console log 위한 사용. 추후 event 받는 측에서 조치해야함 
        }

    }
}
