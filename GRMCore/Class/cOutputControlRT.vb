Imports System.IO
Imports System.Text
Public Class cOutputControlRT
    Dim m_cpuUsage As New PerformanceCounter("Processor", "% Processor Time", "_Total") '성능 비교 용
    Dim m_diskUsageC As New PerformanceCounter("PhysicalDisk", "% Disk Time", "0 C:")
    Dim m_diskUsageD As New PerformanceCounter("PhysicalDisk", "% Disk Time", "1 D:")

    Sub WriteSimResultsToTextFileAndDBForRealTime(ByVal project As cProject, ByVal nowT_MIN As Integer,
                             ByVal interCoef As Single,
                             ByVal project_tm1 As cProjectBAK, RTProject As cRealTime)

        Dim intTimeNow As Date
        Dim lngTimeDiffFromStarting_SEC As Long
        Dim strNowTimeToPrintOut As String
        intTimeNow = Now()
        lngTimeDiffFromStarting_SEC = DateDiff(DateInterval.Second, cThisSimulation.mTimeThisSimulationStarted, intTimeNow)
        strNowTimeToPrintOut = Format(cRealTime.Current.mDateTimeStartRT.Add(New System.TimeSpan(0, nowT_MIN, 0)), "yyyy/MM/dd HH:mm")

        Dim strWPName As String
        Dim strFNP As String
        Dim vToPrint As String = ""
        Dim sngQobs As Single
        Dim strOutPutLine As String
        Dim strSQL_Server As String = ""       'SQL DB 에도 추가적으로 기입

        For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
            Dim cvan As Integer = row.CVID - 1
            If interCoef = 1 Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("@{0,8:#0.##}", project.CV(cvan).QCVof_i_j_m3Ps)
                    Case Else
                        vToPrint = String.Format("{0,8:#0.##}", project.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps)
                End Select
            ElseIf project_tm1 IsNot Nothing Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                               project_tm1.CV(cvan).QCVof_i_j_m3Ps,
                                               project.CV(cvan).QCVof_i_j_m3Ps, interCoef))
                    Case Else
                        vToPrint = String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                                project_tm1.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps,
                                                project.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps, interCoef))
                End Select
            End If

            '여기서 관측자료 받는다.. 직접 받을 수 있는 경우를 대비해서, 자리만 만들어줌..
            sngQobs = 0
            strWPName = Replace(row.Name, ",", "_")

            '실시간 모델링 텍스트 파일 쓰고
            strFNP = project.ProjectPath & "\" & project.ProjectNameOnly & "RealTime_" & strWPName & ".out"
            strFNP = IO_Path_ChangeDrive(cRealTime.CONST_Output_File_Target_DISK, strFNP)
            strOutPutLine = strNowTimeToPrintOut _
                                  + vbTab + Format(project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID), "#0.00") _
                                  + vbTab + Format(sngQobs, "#0.00") + vbTab + vToPrint _
                                  + vbTab + Format(lngTimeDiffFromStarting_SEC / 60, "#0.00") + vbCrLf

            If Not IO.Directory.Exists(IO.Path.GetDirectoryName(strFNP)) Then IO.Directory.CreateDirectory(IO.Path.GetDirectoryName(strFNP))
            IO.File.AppendAllText(strFNP, strOutPutLine, Encoding.Default)
            Call Log_Performance_data(project.ProjectNameOnly, strWPName, strNowTimeToPrintOut, lngTimeDiffFromStarting_SEC / 60.0) '성능 비교 분석용 정보 수집

            If cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem Then
                Dim oSQLCon As New System.Data.SqlClient.SqlConnection(g_strDBMSCnn)
                If oSQLCon.State = ConnectionState.Closed Then oSQLCon.Open()
                Dim intPos As Integer = row.Name.IndexOf("_"c)
                Dim strGaugeCode As String = row.Name.Substring(intPos + 1)

                '2017년 방식
                'Dim strSQL As String = String.Format("insert into Qwatershed_CAL ([WSCODE], GaugeCode, time ,[Value], [RFMean_mm]) values('{0}','{1}',{2},{3},{4})",
                '    project.ProjectNameOnly,
                '    strGaugeCode,
                '    cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut),
                '    vToPrint,
                '    project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID),
                '    cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME.ToString)

                '2018년 방식 by 원 2018.8.8 
                '2018.8.10 이전에 9999 였고. 이제 test 위해 g_performance_log_GUID 사용중
                Dim strSQL As String = String.Format("insert into [Q_CAL] (runid,WPName, [Time],[value], RFMean_mm) values({0},'{1}','{2}',{3},{4})",
                    g_RunID,
                    strWPName,
                    cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut),
                    vToPrint,
                    project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID))

                Dim oSQLCMD As New SqlClient.SqlCommand(strSQL, oSQLCon)
                Dim intRetVal As Integer = oSQLCMD.ExecuteNonQuery()
                If intRetVal <> 1 Then
                    System.Console.WriteLine(strSQL + vbCrLf + project.ProjectPath)
                    Throw New ApplicationException("Error : " & strSQL)
                End If
                oSQLCon.Close()

            Else '항상 db를 사용
                'strOutPutLine = String.Format("{0},{1},{2}{3}", RTProject.mDWSS_CVID_toConnectUWSS.ToString,
                '                                  cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut),
                '                                  vToPrint.Trim, vbCrLf)
                'If RTProject.mbIsDWSS = True Then
                '    If RTProject.mFPNDWssFCData IsNot Nothing AndAlso cFile.IsFileLocked(RTProject.mFPNDWssFCData, 10) = False Then
                '        Dim checkError As Boolean = True
                '        While checkError
                '            Try
                '                IO.File.AppendAllText(RTProject.mFPNDWssFCData, strOutPutLine, Encoding.Default)
                '                checkError = False
                '            Catch ex2 As IOException
                '                Console.WriteLine(String.Format("{0} : {1} File 기록 동시 접근상황", Now.ToLongTimeString, RTProject.mFPNDWssFCData))
                '                checkError = True
                '            Catch e As Exception
                '                Throw New ApplicationException("IO.File.AppendAllText(RTProject.mFPNDWssFCData, strOutPutLine, Encoding.Default)")
                '            End Try
                '        End While
                '    Else
                '        Dim msgstr As String
                '        msgstr = "Calculated discharge cannot be written in the downstream watershed flow control file." + vbCrLf
                '        msgstr = msgstr + String.Format("File name : {0}{1}", RTProject.mFPNDWssFCData, vbCrLf)
                '        msgstr = msgstr + "The file may be locked by another process."
                '        Throw New ApplicationException(msgstr)
                '    End If
                'End If
            End If
        Next
    End Sub

    Sub Log_Performance_data(strBasin As String, strTag As String, strDataTime As String, dblElapTime As Double)
        '성능 비교 용도. 개별 프로세스. launcher .exe 에 집중. 머신 전체는 monitor에서 측정하도록함
        Dim oSQLCon As New System.Data.SqlClient.SqlConnection(g_strDBMSCnn)
        If oSQLCon.State = ConnectionState.Closed Then oSQLCon.Open()
        Dim strSQL As String = String.Format("insert into run_perf ([basin],[RainfallDataCompleted],[ElapsedTime_Min],[net_process_PrivateMemorySize64],[run_meta_guid],[OutputDrive],[tag]) values('{0}','{1}',{2},{3},'{4}','{5}','{6}')",
                                                         strBasin, strDataTime, dblElapTime, Process.GetCurrentProcess.PrivateMemorySize64 / 1024 / 1024 / 1024, g_RunID, cRealTime.CONST_Output_File_Target_DISK, strTag)
        Dim oSQLCMD As New SqlClient.SqlCommand(strSQL, oSQLCon)
        Dim intRetVal As Integer = oSQLCMD.ExecuteNonQuery()
        If intRetVal <> 1 Then
            Console.WriteLine("perf logging error")
        End If
        oSQLCon.Close()
    End Sub

    Public Shared Function CreateNewOutputTextFileRT(ByVal project As cProject, RTproject As cRealTime) As Boolean
        Try
            Dim strWPName As String
            Dim strFNP As String
            Dim strOutPutLine As String
            Dim FPNs As List(Of String)
            Console.Write("Deleting previous output files... ")
            For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
                strWPName = Replace(row.Name, ",", "_")
                strFNP = project.ProjectPath & "\" & project.ProjectNameOnly & "RealTime_" & strWPName & ".out"
                FPNs = New List(Of String)
                FPNs.Add(strFNP)
                cFile.ConfirmDeleteFiles(FPNs)
                IO.File.Delete(strFNP)
                strOutPutLine = "GRM real time simulation results for watchpoint " + strWPName + ": Discharge[CMS]  by " _
                                        & cGRM.BuildInfo.ProductName & vbCrLf
                strOutPutLine = strOutPutLine + cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + vbTab +
                    "RFMean[mm]" + vbTab + "Qobs[CMS]" + vbTab + "Qsim[CMS]" + vbTab + "TimeFormStarting[MIN]" + vbCrLf
                IO.File.AppendAllText(strFNP, strOutPutLine, Encoding.Default)
            Next
            If project.GeneralSimulEnv.mbMakeRasterOutput = True Then
                Dim Dpath As New List(Of String)
                Dpath.Add(project.OFPSSRDistribution)
                Dpath.Add(project.OFPRFDistribution)
                Dpath.Add(project.OFPRFAccDistribution)
                Dpath.Add(project.OFPFlowDistribution)
                If Dpath.Count > 0 Then
                    If cFile.ConfirmDeleteDirectory(Dpath) = False Then
                        cThisSimulation.mGRMSetupIsNormal = False
                        Return False
                    End If
                    If cFile.ConfirmCreateDirectory(Dpath) = False Then
                        cThisSimulation.mGRMSetupIsNormal = False
                        Return False
                    End If
                End If
            End If
            Console.WriteLine("completed. ")
            If cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem = False Then
                'If RTproject.mbIsDWSS = True AndAlso Not IO.File.Exists(RTproject.mFPNDWssFCData) Then
                '    strOutPutLine = "CVID," + cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + ",Value" & vbCrLf
                '    IO.File.AppendAllText(RTproject.mFPNDWssFCData, strOutPutLine, Encoding.Default)
                'End If
            End If

            Return True
        Catch ex As Exception
            Return False
        End Try
        Return True
    End Function

End Class
