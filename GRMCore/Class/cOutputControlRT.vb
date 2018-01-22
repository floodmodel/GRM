Imports System.IO
Imports System.Text
Public Class cOutputControlRT

    '2017.6.1 원 :  성능 비교 편의용도  (단 아직 신뢰할 수 없슴. perf mon 에서는 disk % 가 5% 미만인데. log 에서는 80 % 정도임)
    Dim m_cpuUsage As New PerformanceCounter("Processor", "% Processor Time", "_Total")
    'perf mon 에서는 % User Time 과 큰차이가 없슴

    'Dim diskUsage As New PerformanceCounter("PhysicalDisk", "% Disk Time", "_Total")
    Dim m_diskUsageC As New PerformanceCounter("PhysicalDisk", "% Disk Time", "0 C:")
    Dim m_diskUsageD As New PerformanceCounter("PhysicalDisk", "% Disk Time", "1 D:")



    'Private mSimul As cSimulator

    'Public Sub New(ByVal simul As cSimulator)
    '    mSimul = simul
    'End Sub

    Sub WriteSimResultsToTextFileAndDBForRealTime(ByVal project As cProject, ByVal nowT_MIN As Integer, _
                             ByVal interCoef As Single, _
                             ByVal project_tm1 As cProjectBAK, RTProject As cRealTime)

        Dim intTimeNow As Date
        'Dim lngTimeDiff As Long
        Dim lngTimeDiffFromStarting_SEC As Long

        Dim strNowTimeToPrintOut As String
        intTimeNow = Now()
        'lngTimeDiff = DateDiff(DateInterval.Second, cThisSimulation.mTimeThisStepStarted, intTimeNow)
        lngTimeDiffFromStarting_SEC = DateDiff(DateInterval.Second, cThisSimulation.mTimeThisSimulationStarted, intTimeNow)
        strNowTimeToPrintOut = Format(cRealTime.Current.mDateTimeStartRT.Add(New System.TimeSpan(0, nowT_MIN, 0)), "yyyy/MM/dd HH:mm")

        Dim strWPName As String
        Dim strFNP As String
        Dim vToPrint As String = ""
        Dim sngQobs As Single
        Dim strOutPutLine As String
        Dim strSQL_Server As String = ""       '여기서 SQL DB 에도 추가적으로 기입함. 2011.6.7 원영진,문정록

        For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
            Dim cvan As Integer = row.CVID - 1
            If interCoef = 1 Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("@{0,8:#0.##}", project.CV(cvan).QCVof_i_j_m3Ps) 'CV의 배열 번호는 CVid -1과 같으므로..
                    Case Else
                        vToPrint = String.Format("{0,8:#0.##}", project.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps)
                End Select
            ElseIf project_tm1 IsNot Nothing Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                               project_tm1.CV(cvan).QCVof_i_j_m3Ps,
                                               project.CV(cvan).QCVof_i_j_m3Ps, interCoef)) 'CV의 배열 번호는 CVid -1과 같으므로..
                    Case Else
                        vToPrint = String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                                project_tm1.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps,
                                                project.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps, interCoef))
                End Select
            End If

            '여기서 관측자료 받는다.. 직접 받을 수 있는 경우를 대비해서, 자리만 만들어줌..
            sngQobs = 0
            strWPName = Replace(row.Name, ",", "_")

            '여기서 실시간 모델링 텍스트 파일 쓰고
            strFNP = project.ProjectPath & "\" & project.ProjectNameOnly & "RealTime_" & strWPName & ".out"
            strFNP = IO_Path_ChangeDrive(cRealTime.CONST_Output_File_Target_DISK, strFNP)    '2017.6.1 원 : 대상 drive 조정

            strOutPutLine = strNowTimeToPrintOut _
                                  + vbTab + Format(project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID), "#0.00") _
                                  + vbTab + Format(sngQobs, "#0.00") + vbTab + vToPrint _
                                  + vbTab + Format(lngTimeDiffFromStarting_SEC / 60, "#0.00") + vbCrLf

            If Not IO.Directory.Exists(IO.Path.GetDirectoryName(strFNP)) Then IO.Directory.CreateDirectory(IO.Path.GetDirectoryName(strFNP))  '2017.6.1 원 추가
            IO.File.AppendAllText(strFNP, strOutPutLine, Encoding.Default)
            Call Log_Performance_data(project.ProjectNameOnly, strNowTimeToPrintOut, lngTimeDiffFromStarting_SEC / 60.0) '2017.5.31 원 : 성능 비교 분석용 정보 수집

            'If (RTProject.mbIsDWSS = False AndAlso row.Name = "mn_md") Or (RTProject.mbIsDWSS = True AndAlso RTProject.mCWSS_CVI D_toConnectWithDWSS = row.CVID) Then        ' 2016.4.26까지 사용한 코드

            'If RTProject.mbIsDWSS = True AndAlso RTProject.mCWSS_CVID_toConnectWithDWSS = row.CVID Then
            '-->     2015.11.11 최 : 본류를 위해 아래와 같이 수정함. mn_md 이건 우선 하드코딩
            ' 여기서.. 모든 wp 결과 db 적제 하는 것으로 수정 필요.. 2015.12.01 최

            '2015.12.09. 하류로 전달할 유량을 DB화 할지, txt file(OFCD.txt)로 쓸지를 결정
            If cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem Then 'Qwatershed Table에 기록
                Dim oSQLCon As New System.Data.SqlClient.SqlConnection(g_strDBMSCnn)
                If oSQLCon.State = ConnectionState.Closed Then oSQLCon.Open()
                '2015.11.11 최 : row.Name 이게 watchpoint name 
                '2016.4.25 최 : gaugeinfo table의 gaugecode 선정은. watchpoint 에서 처음 등장 _ 의 다음 부분으로 함

                Dim intPos As Integer = row.Name.IndexOf("_"c)
                Dim strGaugeCode As String = row.Name.Substring(intPos + 1)

                ' Dim strSQL As String = String.Format("insert into Qwatershed_CAL ([WSCODE], GaugeCode, [Time] ,[Value], [RFMean_mm]) values('{0}','{1}',{2},{3},{4})",
                'project.ProjectNameOnly, strGaugeCode, cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut),
                'vToPrint, project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID), cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME.ToString)
                ' 2016.10.10 안주영
                ' mssql 은daattime컬럼을 사용하지않고 time으로 유지한다. 
                'Dim strSQL As String = String.Format("insert into Qwatershed_CAL ([WSCODE], GaugeCode, {5} ,[Value], [RFMean_mm]) values('{0}','{1}',{2},{3},{4})",
                Dim strSQL As String = String.Format("insert into Qwatershed_CAL ([WSCODE], GaugeCode, time ,[Value], [RFMean_mm]) values('{0}','{1}',{2},{3},{4})",
                                                         project.ProjectNameOnly, strGaugeCode, cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut),
                                                         vToPrint, project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID), cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME.ToString)
                '  
                Dim oSQLCMD As New SqlClient.SqlCommand(strSQL, oSQLCon)
                Dim intRetVal As Integer = oSQLCMD.ExecuteNonQuery()
                If intRetVal <> 1 Then
                    '2016.11.9 원 : 무시할 오류가 아니므로 명시적으로 중단시킴. 2016.11.9 에서는 관측소  code가 없어서 발생. 
                    '               최, 원 협의한  "가상지점" 으로 문제 해결함
                    System.Console.WriteLine(strSQL + vbCrLf + project.ProjectPath)
                    Throw New ApplicationException("Error : " & strSQL)
                End If
                oSQLCon.Close()

            Else
                strOutPutLine = String.Format("{0},{1},{2}{3}", RTProject.mDWSS_CVID_toConnectUWSS.ToString,
                                                  cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut),
                                                  vToPrint.Trim, vbCrLf)
                If RTProject.mbIsDWSS = True Then
                    If RTProject.mFPNDWssFCData IsNot Nothing AndAlso cFile.IsFileLocked(RTProject.mFPNDWssFCData, 10) = False Then
                        Dim checkError As Boolean = True
                        While checkError
                            Try
                                'File이 없으면 여기서 생성됨
                                IO.File.AppendAllText(RTProject.mFPNDWssFCData, strOutPutLine, Encoding.Default)
                                checkError = False
                            Catch ex2 As IOException ' 다른 프로세스에서 사용중
                                Console.WriteLine(String.Format("{0} : {1} File 기록 동시 접근상황", Now.ToLongTimeString, RTProject.mFPNDWssFCData))
                                checkError = True
                            Catch e As Exception
                                Throw New ApplicationException("IO.File.AppendAllText(RTProject.mFPNDWssFCData, strOutPutLine, Encoding.Default)")
                            End Try
                        End While
                    Else
                        Dim msgstr As String
                        msgstr = "Calculated discharge cannot be written in the downstream watershed flow control file." + vbCrLf
                        msgstr = msgstr + String.Format("File name : {0}{1}", RTProject.mFPNDWssFCData, vbCrLf)
                        msgstr = msgstr + "The file may be locked by another process."
                        Throw New ApplicationException(msgstr)
                    End If
                End If
            End If
            'End If

        Next
        'cRealTime.Current.gofrmGRMRT.RaiseMessageForOneOutputRecordAdded()
    End Sub

    Sub Log_Performance_data(strBasin As String, strDataTime As String, dblElapTime As Double)
        '2017.6.1 원 :  성능 비교 편의용도. 개별 프로세스. launcher .exe 에 집중. 머신 전체는 monitor에서 측정하도록함

        Dim oSQLCon As New System.Data.SqlClient.SqlConnection(g_strDBMSCnn)
        If oSQLCon.State = ConnectionState.Closed Then oSQLCon.Open()

        'Dim sngCPU As Single
        'sngCPU = m_cpuUsage.NextValue()  'Console.WriteLine(sngCPU)    '잘 안맞음
        'Dim sngDISKC As Single
        'sngDISKC = m_diskUsageC.NextValue  'Console.WriteLine(sngDISKC)
        'Dim sngDISKD As Single
        'sngDISKD = m_diskUsageD.NextValue  'Console.WriteLine(sngDISKD)

        Dim strSQL As String = String.Format("insert into run_perf ([basin],[RainfallDataCompleted],[ElapsedTime_Min],[net_process_PrivateMemorySize64],[run_meta_guid],[OutputDrive]) values('{0}','{1}',{2},{3},'{4}','{5}')",
                                                         strBasin, strDataTime, dblElapTime, Process.GetCurrentProcess.PrivateMemorySize64 / 1024 / 1024 / 1024, g_performance_log_GUID, cRealTime.CONST_Output_File_Target_DISK)

        Dim oSQLCMD As New SqlClient.SqlCommand(strSQL, oSQLCon)
        Dim intRetVal As Integer = oSQLCMD.ExecuteNonQuery()
        If intRetVal <> 1 Then
            Console.WriteLine("perf logging error")
            'Throw New ApplicationException("Error : " & strSQL)    '분석은 계속 되어야 함
        End If
        oSQLCon.Close()

    End Sub

    Public Shared Function CreateNewOutputTextFileRT(ByVal project As cProject, RTproject As cRealTime) As Boolean
        Try
            Dim strWPName As String
            Dim strFNP As String
            Dim strOutPutLine As String
            Dim FPNs As List(Of String)
            For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
                strWPName = Replace(row.Name, ",", "_")
                strFNP = project.ProjectPath & "\" & project.ProjectNameOnly & "RealTime_" & strWPName & ".out"
                FPNs = New List(Of String)
                FPNs.Add(strFNP)
                cFile.ConfirmDeleteFiles(FPNs)
                IO.File.Delete(strFNP)
                '여기서 해더 기록
                strOutPutLine = "GRM real time simulation results for watchpoint " + strWPName + ": Discharge[CMS]  by " _
                                        & cGRM.BuildInfo.ProductName & vbCrLf
                strOutPutLine = strOutPutLine + cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + vbTab +
                    "RFMean[mm]" + vbTab + "Qobs[CMS]" + vbTab + "Qsim[CMS]" + vbTab + "TimeFormStarting[MIN]" + vbCrLf
                IO.File.AppendAllText(strFNP, strOutPutLine, Encoding.Default)
            Next
            If project.GeneralSimulEnv.mbEnableAnalyzer = True Then
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

            If cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem = False Then
                If RTproject.mbIsDWSS = True AndAlso Not IO.File.Exists(RTproject.mFPNDWssFCData) Then
                    strOutPutLine = "CVID," + cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + ",Value" & vbCrLf
                    IO.File.AppendAllText(RTproject.mFPNDWssFCData, strOutPutLine, Encoding.Default)
                End If
            End If

            Return True
        Catch ex As Exception
            Return False
        End Try
        Return True
    End Function

    ''모의결과 db 출력은 없으므로, 이 프로세스는 사용되지 않음. 2016.12.21. 최
    'Public Shared Function DropAndCreateOutputTableRT(ByVal project As cProject) As Boolean
    '    Try
    '        'If project.OdbCnnDynamic.State = ConnectionState.Closed Then project.OdbCnnDynamic.Open()
    '        Dim restricstions() As String = New String(3) {}
    '        Dim myTables As New Data.DataTable
    '        restricstions(3) = "table"
    '        'myTables = project.OdbCnnDynamic.GetOleDbSchemaTable(OleDb.OleDbSchemaGuid.Tables, restricstions)
    '        Dim strQuery As String
    '        Dim odbCMD As New OleDb.OleDbCommand
    '        Dim intTableCount As Integer
    '        Dim strNowTableName As String
    '        Dim strNowTableNameHeader As String
    '        intTableCount = myTables.Rows.Count
    '        'odbCMD = project.OdbCnnDynamic.CreateCommand
    '        '여기서 테이블 지우고 삭제
    '        For intN As Integer = 0 To intTableCount - 1
    '            strNowTableName = CStr(myTables.Rows(intN).Item(2))
    '            strNowTableNameHeader = Mid(strNowTableName, 1, 8)
    '            If strNowTableNameHeader = "RealTime" Then
    '                strQuery = "delete * from " & strNowTableName
    '                odbCMD.CommandText = strQuery
    '                odbCMD.ExecuteNonQuery()
    '                strQuery = "drop table " & strNowTableName
    '                odbCMD.CommandText = strQuery
    '                odbCMD.ExecuteNonQuery()
    '            End If
    '        Next intN
    '        Dim strTableName As String
    '        For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
    '            '여기서 WatchPoint 별로 하나씩의 테이블 생성, 필드는 강우 이벤트 번호
    '            strTableName = "RealTime_" & Replace(row.Name, ",", "_")
    '            strQuery = String.Format("create table {0} ( DataNo long CONSTRAINT DataNo PRIMARY KEY, {1} string, Rainfall string, Qsim single);",
    '                                     strTableName, cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME.ToString)
    '            odbCMD.CommandText = strQuery
    '            odbCMD.ExecuteNonQuery()
    '        Next
    '        Return True
    '    Catch ex As Exception
    '        Return False
    '    End Try
    'End Function
End Class
