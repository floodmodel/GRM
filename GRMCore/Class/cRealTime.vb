Imports System.Text
Public Class cRealTime

    Public Const CONST_bUseDBMS_FOR_RealTimeSystem As Boolean = True    'Real time에서는 true로 설정
    Public Shared CONST_Output_File_Target_DISK As Char = CChar("?") 'png 등의 모의 결과를 c ,.d  어디에 기입할지. 구분 
    Event RTStatus(ByVal strMSG As String)
    Public mRainfallDataTypeRT As cRainfall.RainfallDataType
    Public mRfFilePathRT As String
    Public mDtPrintOutRT_min As Integer
    Public mSimDurationrRT_Hour As Integer
    Public mRFStartDateTimeRT As String
    Public mbNewRFAddedRT As Boolean
    Public mdicBNewFCdataAddedRT As Dictionary(Of Integer, Boolean)   'CVID 로 구분
    'Public mdicBNewFCdataAddedRT_v2018 As Dictionary(Of String, Boolean)    'DAM 으로 구분 . 굳이 이리할 필요 없어서. 도입하지 않음. 2018.8.28
    Public mlstRFdataRT As List(Of cRainfall.RainfallData)
    ''' <summary>
    ''' Get data count by Cvid
    ''' </summary>
    ''' <remarks></remarks>
    Public mdicFCDataCountForEachCV As Dictionary(Of Integer, Integer)
    Public mdicFCNameForEachCV As Dictionary(Of Integer, String)
    Public mdicFCDataOrder As Dictionary(Of Integer, Integer)
    Public mRFLayerCountToApply_RT As Integer
    Public mDateTimeStartRT As DateTime
    Public mbSimulationRTisOngoing As Boolean
    Private Shared mGRMRT As cRealTime
    Private mRTProject As cProject
    'Public mFPNFcData As String
    Public mPicWidth As Single
    Public mPicHeight As Single
    Private mRasterFileOutput As cRasterOutput

    Public mbIsDWSS As Boolean
    Public mCWCellColX_ToConnectDW As Integer
    Public mCWCellRowY_ToConnectDW As Integer
    Public mDWCellColX_ToConnectCW As Integer
    Public mDWCellRowY_ToConnectCW As Integer
    'Public mFPNDWssFCData As String

    Private WithEvents mSimul As cSimulator
    Private m_odt_flowcontrolinfo As Data.DataTable

    Public Shared Sub InitializeGRMRT()
        Dim strTmp As String = File.ReadAllText("C:\Nakdong\outputDrive.txt")
        If strTmp.ToUpper() <> "C" And strTmp.ToUpper() <> "D" And strTmp.ToUpper() <> "S" Then
            Console.WriteLine("Can not Read " + strTmp)
            Stop
        End If
        CONST_Output_File_Target_DISK = CChar(strTmp)
        mGRMRT = New cRealTime
    End Sub

    Public Sub SetupGRM(ByVal FPNprj As String) 'fc 자료는 항상 db를 사용하는 것으로 수정, Optional FPNfcdata As String = "")
        'mFPNFcData = FPNfcdata
        If cProject.OpenProject(FPNprj, True) = False Then
            RaiseEvent RTStatus("모형 설정 실패.")
            If CONST_bUseDBMS_FOR_RealTimeSystem Then Call Add_Log_toDBMS(IO.Path.GetFileName(FPNprj), "모형 설정 실패.")
            Exit Sub
        End If
        'mRTProject = New cProject
        mRTProject = cProject.Current
        mRTProject.mSimulationType = cGRM.SimulationType.RealTime
        If mRTProject.SetupModelParametersAfterProjectFileWasOpened() = False Then
            cGRM.writelogAndConsole("GRM setup was failed !!!", True, True)
            Exit Sub
        End If
        If cOutPutControl.CreateNewOutputFiles(cProject.Current, True) = False Then ' 기존의 single 이벤트에서의 출력파일을 지운다.
            cGRM.writelogAndConsole("Deleting single event output files were failed !!!", True, True)
        End If

        RaiseEvent RTStatus("모형 설정 완료.")
        If CONST_bUseDBMS_FOR_RealTimeSystem Then Call Add_Log_toDBMS(mRTProject.ProjectNameOnly, "모형 설정 완료.")
    End Sub

    Public Sub RunGRMRT()
        cThisSimulation.mGRMSetupIsNormal = True
        If mRTProject.GeneralSimulEnv.mbMakeRasterOutput = True Then
            With mRTProject
                .mImgFPN_dist_Flow = New List(Of String)
                .mImgFPN_dist_RF = New List(Of String)
                .mImgFPN_dist_RFAcc = New List(Of String)
                .mImgFPN_dist_SSR = New List(Of String)
            End With
            mRasterFileOutput = New cRasterOutput(mRTProject)
        End If
        mDateTimeStartRT = New DateTime(CInt(Mid(mRFStartDateTimeRT, 1, 4)),
                                             CInt(Mid(mRFStartDateTimeRT, 5, 2)),
                                             CInt(Mid(mRFStartDateTimeRT, 7, 2)),
                                             CInt(Mid(mRFStartDateTimeRT, 9, 2)),
                                             CInt(Mid(mRFStartDateTimeRT, 11, 2)), 0)
        mSimDurationrRT_Hour = 24 * 100 '충분히 100일 동안 모의하는 것으로 설정
        mbSimulationRTisOngoing = True
        mlstRFdataRT = New List(Of cRainfall.RainfallData)
        mRFLayerCountToApply_RT = 0
        If CONST_bUseDBMS_FOR_RealTimeSystem Then
            If False Then   '2018.8 부터 이제 과거 분석 기록은 보존됨..그래서 삭제 code는 미수행.
                Call Clear_DBMS_Table_Qwatershed(mRTProject.ProjectNameOnly)
                RaiseEvent RTStatus("DBMS [Q_CAL] Table Cleared")
                Call Add_Log_toDBMS(mRTProject.ProjectNameOnly, "DBMS [Q_CAL] Table Cleared")
            End If
        End If

        RaiseEvent RTStatus("실시간 유출해석 시작..")
        If CONST_bUseDBMS_FOR_RealTimeSystem Then Call Add_Log_toDBMS(mRTProject.ProjectNameOnly, "실시간 유출해석 시작..")

        mSimul = New cSimulator
        If CreateNewOutputFilesRT() = False Then Exit Sub

        Dim ascFPN As String = mRfFilePathRT + "\" + GetYearAndMonthFromyyyyMMddHHmm(mRFStartDateTimeRT) + "\" + mRFStartDateTimeRT + ".asc"
        If IO.File.Exists(ascFPN) = False Then
            RaiseEvent RTStatus("유출해석 시작 시간에서의 강우자료가 없습니다.")
            RaiseEvent RTStatus("강우자료와 유출해석 시작 시간을 확인하시길 바랍니다.")
        End If

        If mRTProject.FCGrid.IsSet = True Then
            mdicFCDataCountForEachCV = New Dictionary(Of Integer, Integer)
            mdicFCNameForEachCV = New Dictionary(Of Integer, String)
            mdicFCDataOrder = New Dictionary(Of Integer, Integer)
            mdicBNewFCdataAddedRT = New Dictionary(Of Integer, Boolean)
            For Each id As Integer In mRTProject.FCGrid.FCGridCVidList
                mdicFCDataCountForEachCV.Add(id, 0)
                mdicBNewFCdataAddedRT.Add(id, True)
                mdicFCDataOrder.Add(id, 0)
            Next
            'If mGRMRT.mFPNFcData.Trim <> "" AndAlso IO.File.Exists(mGRMRT.mFPNFcData) = False Then
            '    System.Console.WriteLine(String.Format("Realtime flow control data file is not valid. {0} {1}", vbCrLf, mGRMRT.mFPNFcData))
            '    Exit Sub
            'End If
            mRTProject.FCGrid.mdtFCFlowData = New DataTable
            Call ReadDBorCSVandMakeFCdataTableForRealTime_v2018(mRFStartDateTimeRT)
            If mRTProject.FCGrid.mdtFCFlowData.Rows.Count < 1 Then
                RaiseEvent RTStatus("유출해석 시작 시간에서의 flow control 자료가 없습니다.")
                RaiseEvent RTStatus("유출해석 시작시간과 댐방류량, inlet 자료 등 flow control 자료를 확인하시길 바랍니다.")
            End If
        End If
        mSimul.SimulateRT(mRTProject, Me)
    End Sub

    '2018.8.8 이제 부터는 과거  run 도 보존 . 그래서 이 함수는 미사용됨.
    Private Sub Clear_DBMS_Table_Qwatershed(strName As String)
        Dim oSQLCon As New System.Data.SqlClient.SqlConnection(g_strDBMSCnn)
        If oSQLCon.State = ConnectionState.Closed Then oSQLCon.Open()

        'Dim strSQL As String = String.Format("delete [Q_CAL] where wscode='{0}'", strName)
        Dim strSQL As String = String.Format("delete [Q_CAL] where runid={0}", g_performance_log_GUID)   ''2018.8.8 부터 임시 적용. 한시적

        Dim oSQLCMD As New SqlClient.SqlCommand(strSQL, oSQLCon)
        Dim intAffectedRecords As Integer = oSQLCMD.ExecuteNonQuery()
        Dim strMsg As String = String.Format("[Q_CAL] Table에서 {0} 유역 {1}건 Data 삭제됨. 초기화 완료.", strName, intAffectedRecords)
        RaiseEvent RTStatus(strMsg)
        Call Add_Log_toDBMS(strName, strMsg)
    End Sub

    Private Function CreateNewOutputFilesRT() As Boolean
        If Not cOutputControlRT.CreateNewOutputTextFileRT(mRTProject, mGRMRT) Then Return False
        Return True
    End Function

    Public Sub UpdateFcDatainfoGRMRT(ByVal strDate As String, cvid As Integer,
                                     previousOrder As Integer, dtMIN As Integer)
        Dim fcname As String = mRTProject.FCGrid.GetFCName(cvid)
        Dim drs As DataRow() = mRTProject.FCGrid.mdtFCFlowData.Select(String.Format("CVID = {0} and datetime={1}", cvid, strDate))
        If drs.Count > 0 Then
            mdicBNewFCdataAddedRT(cvid) = True
            RaiseEvent RTStatus(String.Format("  FC Data 입력완료...({3} {0}, CVID={1}, Value={2})",
                                              fcname.PadRight(13), cvid.ToString.PadLeft(5), drs(0).Item("value").ToString.PadLeft(8), strDate))
        Else
            mdicBNewFCdataAddedRT(cvid) = False
            RaiseEvent RTStatus(String.Format("FC 자료({0}, CVID={1}, {2}) 수신대기 중...",
                                  fcname, cvid, strDate))
        End If
    End Sub

    '2018.8 CVID 아닌 DAM 이름 방식으로 변경하려던 시도. 현재 이방법 채택하지 않음
    'Public Sub UpdateFcDatainfoGRMRT_v2018(ByVal strDate As String, GName As String,
    '                                 previousOrder As Integer, dtMIN As Integer)

    '    'Dim drs As DataRow() = mRTProject.FCGrid.mdtFCFlowData.Select(String.Format("CVID = {0} and datetime={1}", cvid, strDate))  '2017년 방식은 CVID로 한정 
    '    Dim drs As DataRow() = mRTProject.FCGrid.mdtFCFlowData.Select(String.Format("gname = '{0}' and datetime={1}", GName, strDate))  '2018년 방식은 DAM 정보를 받아야 하는거 아니가?

    '    If drs.Count > 0 Then
    '        mdicBNewFCdataAddedRT_v2018(GName) = True
    '        RaiseEvent RTStatus(String.Format("  FC Data 입력완료...({2}, GName={0}, Value={1})",
    '                                           GName, drs(0).Item("value").ToString.PadLeft(8), strDate))
    '    Else
    '        mdicBNewFCdataAddedRT_v2018(GName) = False
    '        RaiseEvent RTStatus(String.Format("FC 자료(GName={0}, {1}) 수신대기 중...",
    '                               GName, strDate))
    '    End If
    'End Sub


    Public Sub UpdateRainfallInformationGRMRT(ByVal strDate As String)
        Dim lngTimeDiffFromStarting_SEC As Long = DateDiff(DateInterval.Second, cThisSimulation.mTimeThisSimulationStarted, Now)
        Dim tFromStart As String = Format(lngTimeDiffFromStarting_SEC / 60, "#0.00")
        If mRFLayerCountToApply_RT > 0 And mbNewRFAddedRT = True Then
            Dim rf As New cRainfall
            Dim row As cRainfall.RainfallData = rf.GetRFdataByOrder(mlstRFdataRT, mRFLayerCountToApply_RT)
            RaiseEvent RTStatus(row.FileName & " 분석완료 .. " & tFromStart & "분 경과")
            System.Windows.Forms.Application.DoEvents()
            mbNewRFAddedRT = False
        End If
        Select Case mRainfallDataTypeRT
            Case cRainfall.RainfallDataType.TextFileASCgrid_mmPhr, cRainfall.RainfallDataType.TextFileASCgrid
                'Dim strFilenameOnly As String = strDate   '2017년 방식
                Dim strFilenameOnly As String = "RDR_COMP_ADJ_" + strDate + ".RKDP.bin"    '2018년 8.8 현재 산출 naming
                Dim ascFPN As String = mRfFilePathRT + "\" + GetYearAndMonthFromyyyyMMddHHmm(strDate) + "\" + strFilenameOnly + ".asc"        '2018년 8.8 현재 산출 naming
                If IO.File.Exists(ascFPN) = True Then
                    mRFLayerCountToApply_RT += 1
                    cThisSimulation.mRFDataCountInThisEvent = mRFLayerCountToApply_RT
                    Dim nr As cRainfall.RainfallData
                    With nr
                        .Order = mRFLayerCountToApply_RT
                        .DataTime = strDate
                        .Rainfall = strFilenameOnly + ".asc" ' 
                        .FilePath = mRfFilePathRT + "\" + GetYearAndMonthFromyyyyMMddHHmm(strDate)
                        .FileName = strFilenameOnly + ".asc"
                    End With
                    mlstRFdataRT.Add(nr)
                    RaiseEvent RTStatus(nr.FileName & " 입력완료(강우)")
                    System.Windows.Forms.Application.DoEvents()
                    mbNewRFAddedRT = True
                    Exit Sub
                End If
        End Select
        RaiseEvent RTStatus(String.Format("강우자료({0}) 수신대기 중..", strDate))
    End Sub

    Public Sub ReadDBorCSVandMakeFCdataTableForRealTime_v2017(TargetDateTime As String, Optional CSVFPNsource As String = "")
        Dim dt As New DataTable
        If CONST_bUseDBMS_FOR_RealTimeSystem Then
            If m_odt_flowcontrolinfo Is Nothing Then
                m_odt_flowcontrolinfo = New DataTable()
                Dim strSQL As String = String.Format("select * from flowcontrolinfo where WScode='{0}'", cProject.Current.ProjectNameOnly)
                Dim oSqlDataAdapter As New SqlClient.SqlDataAdapter(strSQL, g_strDBMSCnn)
                oSqlDataAdapter.SelectCommand.CommandTimeout = 60
                Try
                    oSqlDataAdapter.Fill(m_odt_flowcontrolinfo)
                    oSqlDataAdapter.Dispose()
                Catch ex1 As SqlClient.SqlException
                    Console.WriteLine(ex1.ToString)
                    Console.ReadLine()
                    Stop
                Catch ex As Exception
                    Console.WriteLine(ex.ToString)
                    Console.ReadLine()
                    Stop
                End Try
            End If

            For Each oDR As DataRow In m_odt_flowcontrolinfo.Rows
                Dim strCVID As String = oDR.Item("CVID").ToString
                Dim strSourceName As String = ""
                Dim strSourceTableName As String = ""
                Dim strSourceTarget As String = oDR.Item("SourceType").ToString
                Dim strFieldTarget As String = ""
                Dim strWhereGage As String = ""
                Select Case strSourceTarget
                    Case "DAM"
                        strSourceTableName = "QDAM_OBS"
                        strSourceName = oDR.Item("SourceGaugeCode").ToString
                        strFieldTarget = "GaugeCode"
                    Case "WS"
                        strSourceTableName = "QWatershed_CAL"
                        strSourceName = oDR.Item("SourceWSCode").ToString
                        strFieldTarget = "WSCode"
                        If strSourceName.Trim() = "GM" Or strSourceName.Trim() = "EB" Then
                            strWhereGage = String.Format("gaugecode='{0}' and", strSourceName)
                        Else
                            strWhereGage = "gaugecode='md' and"
                        End If
                    Case Else
                        Stop
                End Select

                'SQL for time series
                Dim strSQL_TS As String = String.Format("select  {0} as cvid, time as datetime, value from {1} where {4} {3} ='{2}' and time='{5}'",
                                                        strCVID, strSourceTableName, strSourceName, strFieldTarget, strWhereGage, TargetDateTime)
                Dim odt_TS As New Data.DataTable
                Dim oSqlDataAdapter_TS As New SqlClient.SqlDataAdapter(strSQL_TS, g_strDBMSCnn)
                oSqlDataAdapter_TS.SelectCommand.CommandTimeout = 60
                oSqlDataAdapter_TS.Fill(odt_TS)
                oSqlDataAdapter_TS.Dispose()
                dt.Merge(odt_TS)
            Next oDR
            cProject.Current.FCGrid.mdtFCFlowData = dt
            If False Then
                mRTProject.FCGrid.mdtFCFlowData.TableName = "tmp"
                mRTProject.FCGrid.mdtFCFlowData.WriteXml("C:\temp\read_method_" + cProject.Current.ProjectNameOnly + "_" + Now.ToString("yyMMddHHmmss") + "grm.xml")
            End If
        Else
            dt.Columns.Add(New Global.System.Data.DataColumn("CVID", GetType(Integer), Nothing, Global.System.Data.MappingType.Element))
            dt.Columns.Add(New Global.System.Data.DataColumn("DataTime", GetType(String), Nothing, Global.System.Data.MappingType.Element))
            dt.Columns.Add(New Global.System.Data.DataColumn("Value", GetType(Single), Nothing, Global.System.Data.MappingType.Element))
            Dim intL As Integer = 0
            Using oTextReader As New FileIO.TextFieldParser(CSVFPNsource, Encoding.Default)
                oTextReader.TextFieldType = FileIO.FieldType.Delimited
                oTextReader.SetDelimiters(",")
                oTextReader.TrimWhiteSpace = True
                Dim TextIncurrentRow As String()
                While Not oTextReader.EndOfData
                    TextIncurrentRow = oTextReader.ReadFields
                    For Each ele As String In TextIncurrentRow
                        If Trim(TextIncurrentRow(0)).ToString = "" Then Exit While
                    Next
                    Dim nFieldCount As Integer = TextIncurrentRow.Length
                    If intL > 0 AndAlso TextIncurrentRow(1) = TargetDateTime Then
                        Dim nr As DataRow = dt.NewRow
                        nr("CVID") = CInt(TextIncurrentRow(0))
                        nr("DataTime") = TextIncurrentRow(1)
                        nr("Value") = CSng(TextIncurrentRow(2))
                        mRTProject.FCGrid.mdtFCFlowData.Rows.Add(nr)
                    End If
                    intL += 1
                End While
            End Using
        End If
    End Sub




    Public Sub ReadDBorCSVandMakeFCdataTableForRealTime_v2018(TargetDateTime As String, Optional CSVFPNsource As String = "")

        Dim dt As New DataTable

        If CONST_bUseDBMS_FOR_RealTimeSystem Then
            '2018.8.28 원 : CVID 이건 DB에 없슴. 격자 규격 col, row 위치가 변경시에 CVID도 변경될 것임. 그래서 정적인 DB에 구성 안하는 의도로 예상됨.  최초 gmp road 등 하는 단계에서 temp table로 구성해도 좋겠슴.

            Dim strSQL As String = String.Format("Select  w.name, 999 as cvid ,[Time] as datetime ,[QValue] AS VALUE  From [QDam_OBS] d , WatchPoint w Where d.Gname=w.Gname and w.fc=1 And TIME ='{0}' ", TargetDateTime)

            Dim odt As New Data.DataTable
            Dim oSqlDataAdapter As New SqlClient.SqlDataAdapter(strSQL, g_strDBMSCnn)
            oSqlDataAdapter.SelectCommand.CommandTimeout = 60
            oSqlDataAdapter.Fill(odt)

            For Each oDR As DataRow In odt.Rows
                Dim oDR_Target As DataRow = mRTProject.FCGrid.mdtFCGridInfo.Select(String.Format("Name='{0}'", oDR.Item("NAME").ToString)).FirstOrDefault
                Dim strCVID As String = oDR_Target.Item("CVID").ToString
                oDR.Item("CVID") = strCVID
                Debug.Print(strCVID)
            Next
            dt.Merge(odt)

            '2018.9.3 원  : auto rom 목록도 추가 해주는 처리 시도... 가 필요하다고 생각 했는데. 없어도 잘됨.  
            'Dim oDR_AutoROMs() As DataRow = mRTProject.FCGrid.mdtFCGridInfo.Select(String.Format("rotype='AutoROM'"))
            'Dim odt_auto As New Data.DataTable
            'For Each oDR As DataRow In oDR_AutoROMs
            '    Dim nr As DataRow = dt.NewRow
            '    nr("CVID") = oDR.Item("CVID").ToString
            '    nr("datetime") = TargetDateTime
            '    nr("Value") = ""
            '    odt_auto.Rows.Add(nr)
            'Next
            'dt.Merge(odt_auto)

            '경천DAM 처리
            '            Dim strSpcealDams As String = "'경천댐'"       '2018.8.29 원 : 여기서 n 개 기입... 이건 추후 DB 등으로 이동되어야 함
            Dim strSpcealDams As String = "'경천댐','영주댐'"       '2018.8.29 원 : 여기서 n 개 기입... 이건 추후 DB 등으로 이동되어야 함..  2018.10/11 원 : 영주댐 추가
            Dim strSQL2 As String = String.Format("Select  w.name, 999 as cvid ,[Time] as datetime ,[QValue] AS VALUE From QStream_OBS_ht d , WatchPoint w  Where  d.GName in({1}) and  TIME ='{0}' and d.Gname=w.Gname ", TargetDateTime, strSpcealDams)

            Dim odt2 As New Data.DataTable
            Dim oSqlDataAdapter2 As New SqlClient.SqlDataAdapter(strSQL2, g_strDBMSCnn)
            oSqlDataAdapter2.SelectCommand.CommandTimeout = 60
            oSqlDataAdapter2.Fill(odt2)

            If odt2.Rows.Count <> 2 Then
                'Stop   '2018.10.11 까지는 stop 이었슴
                cGRM.writelogAndConsole(strSpcealDams + " 의 data가 2건이 아님!", False, True)
            End If

            For Each oDR2 As DataRow In odt2.Rows
                Dim oDR_Target2 As DataRow = mRTProject.FCGrid.mdtFCGridInfo.Select(String.Format("Name='{0}'", oDR2.Item("NAME").ToString)).FirstOrDefault
                Dim strCVID2 As String = oDR_Target2.Item("CVID").ToString
                oDR2.Item("CVID") = strCVID2
                Debug.Print(strCVID2)
            Next

            dt.Merge(odt2)

            cProject.Current.FCGrid.mdtFCFlowData = dt

        Else    'DBMS 방식이 아닌 경우
            dt.Columns.Add(New Global.System.Data.DataColumn("CVID", GetType(Integer), Nothing, Global.System.Data.MappingType.Element))
            dt.Columns.Add(New Global.System.Data.DataColumn("DataTime", GetType(String), Nothing, Global.System.Data.MappingType.Element))
            dt.Columns.Add(New Global.System.Data.DataColumn("Value", GetType(Single), Nothing, Global.System.Data.MappingType.Element))
            Dim intL As Integer = 0
            Using oTextReader As New FileIO.TextFieldParser(CSVFPNsource, Encoding.Default)
                oTextReader.TextFieldType = FileIO.FieldType.Delimited
                oTextReader.SetDelimiters(",")
                oTextReader.TrimWhiteSpace = True
                Dim TextIncurrentRow As String()
                While Not oTextReader.EndOfData
                    TextIncurrentRow = oTextReader.ReadFields
                    For Each ele As String In TextIncurrentRow
                        If Trim(TextIncurrentRow(0)).ToString = "" Then Exit While
                    Next
                    Dim nFieldCount As Integer = TextIncurrentRow.Length
                    If intL > 0 AndAlso TextIncurrentRow(1) = TargetDateTime Then
                        Dim nr As DataRow = dt.NewRow
                        nr("CVID") = CInt(TextIncurrentRow(0))
                        nr("DataTime") = TextIncurrentRow(1)
                        nr("Value") = CSng(TextIncurrentRow(2))
                        mRTProject.FCGrid.mdtFCFlowData.Rows.Add(nr)
                    End If
                    intL += 1
                End While
            End Using
        End If
    End Sub

    'Public Sub ReadDBorCSVandMakeFCdataTableForRealTime_v2018_old(TargetDateTime As String, Optional CSVFPNsource As String = "")
    '    '2018.8.28 원 : 이 방법은. DB 에서 건건히 가져오는 방법임.. 비효율 

    '    Dim dt As New DataTable

    '    If CONST_bUseDBMS_FOR_RealTimeSystem Then
    '        Dim strSQL As String

    '        If False Then   '// 2018.8.28 원 : DB에서 FC 만 정확히 관리해 주면 이부분 불필요함
    '            Dim strNameS As String = ""
    '            For Each oDR As DataRow In mRTProject.FCGrid.mdtFCGridInfo.Rows
    '                Dim strName As String = oDR.Item("Name").ToString  '"ADS_WP4"
    '                strNameS = strNameS + String.Format(",'{0}'", strName)
    '            Next
    '            If strNameS.StartsWith(",") Then strNameS = strNameS.Substring(1)

    '            '// GNAME 얻기
    '            strSQL = String.Format("SELECT distinct GName,name  FROM WatchPoint where [name] in ({0})", strNameS)
    '        End If

    '        strSQL = "SELECT GName,name  from  [WatchPoint] where fc=1"

    '        Dim odt As New Data.DataTable
    '        Dim oSqlDataAdapter As New SqlClient.SqlDataAdapter(strSQL, g_strDBMSCnn)
    '        oSqlDataAdapter.SelectCommand.CommandTimeout = 60
    '        oSqlDataAdapter.Fill(odt)
    '        Dim strGNameS As String = ""

    '        For Each oDR As DataRow In odt.Rows
    '            Dim strGName As String = oDR.Item(0).ToString
    '            Dim strName As String = oDR.Item(1).ToString
    '            'strGNameS = strGNameS + String.Format(",'{0}'", strName)

    '            'If strGNameS.StartsWith(",") Then strGNameS = strGNameS.Substring(1)
    '            'Dim strSQL_TS As String = String.Format("select  {0} as cvid, time as datetime, value from {1} where {4} {3} ='{2}' and time='{5}'",
    '            Dim drS As DataRow() = mRTProject.FCGrid.mdtFCGridInfo.Select(String.Format("Name='{0}'", strName))
    '            Dim strCVID As String = drS(0).Item("CVID").ToString

    '            Dim strSQL_TS As String = String.Format("Select name,  {0} as cvid     ,[Time] as datetime      ,[QValue] AS VALUE  From [QDam_OBS] Where GName = '{1}' And TIME ='{2}' ", strCVID, strGName, TargetDateTime)
    '            '
    '            Dim odt_TS As New Data.DataTable
    '            Dim oSqlDataAdapter_TS As New SqlClient.SqlDataAdapter(strSQL_TS, g_strDBMSCnn)
    '            oSqlDataAdapter_TS.SelectCommand.CommandTimeout = 60
    '            oSqlDataAdapter_TS.Fill(odt_TS)
    '            oSqlDataAdapter_TS.Dispose()
    '            dt.Merge(odt_TS)
    '        Next

    '        cProject.Current.FCGrid.mdtFCFlowData = dt

    '    Else    'DBMS 방식이 아닌 경우
    '        dt.Columns.Add(New Global.System.Data.DataColumn("CVID", GetType(Integer), Nothing, Global.System.Data.MappingType.Element))
    '        dt.Columns.Add(New Global.System.Data.DataColumn("DataTime", GetType(String), Nothing, Global.System.Data.MappingType.Element))
    '        dt.Columns.Add(New Global.System.Data.DataColumn("Value", GetType(Single), Nothing, Global.System.Data.MappingType.Element))
    '        Dim intL As Integer = 0
    '        Using oTextReader As New FileIO.TextFieldParser(CSVFPNsource, Encoding.Default)
    '            oTextReader.TextFieldType = FileIO.FieldType.Delimited
    '            oTextReader.SetDelimiters(",")
    '            oTextReader.TrimWhiteSpace = True
    '            Dim TextIncurrentRow As String()
    '            While Not oTextReader.EndOfData
    '                TextIncurrentRow = oTextReader.ReadFields
    '                For Each ele As String In TextIncurrentRow
    '                    If Trim(TextIncurrentRow(0)).ToString = "" Then Exit While
    '                Next
    '                Dim nFieldCount As Integer = TextIncurrentRow.Length
    '                If intL > 0 AndAlso TextIncurrentRow(1) = TargetDateTime Then
    '                    Dim nr As DataRow = dt.NewRow
    '                    nr("CVID") = CInt(TextIncurrentRow(0))
    '                    nr("DataTime") = TextIncurrentRow(1)
    '                    nr("Value") = CSng(TextIncurrentRow(2))
    '                    mRTProject.FCGrid.mdtFCFlowData.Rows.Add(nr)
    '                End If
    '                intL += 1
    '            End While
    '        End Using
    '    End If
    'End Sub

    Sub StopGRM()
        mSimul.StopSimulation()
        RaiseEvent RTStatus("실시간 모델링 종료")
    End Sub


    ''' <summary>
    ''' 모델링 기간과 출력 시간간격을 이용해서 모델링 끝나는 시간을 계산[sec]
    ''' </summary>
    ''' <remarks> </remarks>
    Public ReadOnly Property EndingTime_SEC() As Integer
        Get
            Return CInt(mSimDurationrRT_Hour * 3600)
        End Get
    End Property

    Private Function GetYearAndMonthFromyyyyMMddHHmm(ByVal INPUTyyyyMMddHHmm As String) As String
        Return Mid(INPUTyyyyMMddHHmm, 1, 6)
    End Function

    Public Shared ReadOnly Property Current() As cRealTime
        Get
            Return mGRMRT
        End Get
    End Property

    Private Sub mSimul_SimulationComplete(ByVal sender As cSimulator) Handles mSimul.SimulationComplete
        RaiseEvent RTStatus("분석종료")
    End Sub

    Private Sub mSimul_SimulationRaiseError(ByVal sender As cSimulator, ByVal simulError As cSimulator.SimulationErrors, ByVal erroData As Object) Handles mSimul.SimulationRaiseError
        Select Case simulError
            Case cSimulator.SimulationErrors.OutputFileCreateError
                RaiseEvent RTStatus("출력파일 생성오류")

        End Select
    End Sub

    Private Sub mSimul_SimulationStop(ByVal sender As cSimulator) Handles mSimul.SimulationStop
        RaiseEvent RTStatus("분석종료")
    End Sub

    Private Sub mSimul_CallAnalyzer(sender As cSimulator,
                                    nowTtoPrint_MIN As Integer) Handles mSimul.MakeRasterOutput
        If mRTProject.GeneralSimulEnv.mbMakeRasterOutput = True Then _
        mRasterFileOutput.MakeDistributionFiles(nowTtoPrint_MIN, mRasterFileOutput.ImgWidth, mRasterFileOutput.ImgHeight, True)
    End Sub

    Private Sub cRealTime_RTStatus(strMSG As String) Handles Me.RTStatus

    End Sub
End Class
