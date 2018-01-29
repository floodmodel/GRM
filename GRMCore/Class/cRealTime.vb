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
    Public mdicBNewFCdataAddedRT As Dictionary(Of Integer, Boolean)
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
    Public mFPNFcData As String
    Public mPicWidth As Single
    Public mPicHeight As Single
    Private mGrmAnalyzer As cGRMAnalyzer

    Public mbIsDWSS As Boolean
    Public mDWSS_CVID_toConnectUWSS As Integer
    Public mCWSS_CVID_toConnectWithDWSS As Integer
    Public mFPNDWssFCData As String

    Private WithEvents mSimul As cSimulator
    Private m_odt_flowcontrolinfo As Data.DataTable

    Public Shared Sub InitializeGRMRT()
        Dim strTmp As String = File.ReadAllText("C:\Nakdong\outputDrive.txt")
        If strTmp.ToUpper() <> "C" And strTmp.ToUpper() <> "D" Then
            Console.WriteLine("Can not Read " + strTmp)
            Stop
        End If
        CONST_Output_File_Target_DISK = CChar(strTmp)
        mGRMRT = New cRealTime
    End Sub

    Public Sub SetupGRM(ByVal FPNprj As String, Optional FPNfcdata As String = "")
        mFPNFcData = FPNfcdata
        If cProject.OpenProject(FPNprj, True) = False Then
            RaiseEvent RTStatus("모형 설정 실패.")
            If CONST_bUseDBMS_FOR_RealTimeSystem Then Call Add_Log_toDBMS(cProject.Current.ProjectNameOnly, "모형 설정 실패.")
            Exit Sub
        End If
        cProject.Current.mSimulationType = cGRM.SimulationType.RealTime
        If cProject.Current.SetupModelParametersAfterProjectFileWasOpened() = False Then
            cGRM.writelogAndConsole("GRM setup was failed !!!", True, True)
            Exit Sub
        End If
        RaiseEvent RTStatus("모형 설정 완료.")
        If CONST_bUseDBMS_FOR_RealTimeSystem Then Call Add_Log_toDBMS(cProject.Current.ProjectNameOnly, "모형 설정 완료.")
    End Sub

    Public Sub RunGRMRT()
        cThisSimulation.mGRMSetupIsNormal = True
        If cProject.Current.GeneralSimulEnv.mbEnableAnalyzer = True Then
            With cProject.Current
                .mImgFPN_dist_Flow = New List(Of String)
                .mImgFPN_dist_RF = New List(Of String)
                .mImgFPN_dist_RFAcc = New List(Of String)
                .mImgFPN_dist_SSR = New List(Of String)
            End With
            mGrmAnalyzer = New cGRMAnalyzer(cProject.Current)
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
            Call Clear_DBMS_Table_Qwatershed(cProject.Current.ProjectNameOnly)
            RaiseEvent RTStatus("DBMS Qwatershed_CAL Table Cleared")
            Call Add_Log_toDBMS(cProject.Current.ProjectNameOnly, "DBMS Qwatershed Table Cleared")
        End If

        RaiseEvent RTStatus("실시간 유출해석 시작..")
        If CONST_bUseDBMS_FOR_RealTimeSystem Then Call Add_Log_toDBMS(cProject.Current.ProjectNameOnly, "실시간 유출해석 시작..")

        mSimul = New cSimulator
        If CreateNewOutputFilesRT() = False Then Exit Sub

        Dim ascFPN As String = mRfFilePathRT + "\" + GetYearAndMonthFromyyyyMMddHHmm(mRFStartDateTimeRT) + "\" + mRFStartDateTimeRT + ".asc"
        If IO.File.Exists(ascFPN) = False Then
            RaiseEvent RTStatus("유출해석 시작 시간에서의 강우자료가 없습니다.")
            RaiseEvent RTStatus("강우자료와 유출해석 시작 시간을 확인하시길 바랍니다.")
        End If

        If cProject.Current.FCGrid.IsSet = True Then
            mdicFCDataCountForEachCV = New Dictionary(Of Integer, Integer)
            mdicFCNameForEachCV = New Dictionary(Of Integer, String)
            mdicFCDataOrder = New Dictionary(Of Integer, Integer)
            mdicBNewFCdataAddedRT = New Dictionary(Of Integer, Boolean)
            For Each id As Integer In cProject.Current.FCGrid.FCGridCVidList
                mdicFCDataCountForEachCV.Add(id, 0)
                mdicBNewFCdataAddedRT.Add(id, True)
                mdicFCDataOrder.Add(id, 0)
            Next
            If mGRMRT.mFPNFcData.Trim <> "" AndAlso IO.File.Exists(mGRMRT.mFPNFcData) = False Then
                System.Console.WriteLine(String.Format("Realtime flow control data file is not valid. {0} {1}", vbCrLf, mGRMRT.mFPNFcData))
                Exit Sub
            End If
            cProject.Current.FCGrid.mdtFCFlowData = New DataTable
            Call ReadDBorCSVandMakeFCdataTableForRealTime(mRFStartDateTimeRT)
            If cProject.Current.FCGrid.mdtFCFlowData.Rows.Count < 1 Then
                RaiseEvent RTStatus("유출해석 시작 시간에서의 flow control 자료가 없습니다.")
                RaiseEvent RTStatus("유출해석 시작시간과 댐방류량, inlet 자료 등 flow control 자료를 확인하시길 바랍니다.")
            End If
        End If
        mSimul.SimulateRT(cProject.Current, Me)
    End Sub

    Private Sub Clear_DBMS_Table_Qwatershed(strName As String)
        Dim oSQLCon As New System.Data.SqlClient.SqlConnection(g_strDBMSCnn)
        If oSQLCon.State = ConnectionState.Closed Then oSQLCon.Open()
        Dim strSQL As String = String.Format("delete QWatershed_CAL where wscode='{0}'", strName)
        Dim oSQLCMD As New SqlClient.SqlCommand(strSQL, oSQLCon)
        Dim intAffectedRecords As Integer = oSQLCMD.ExecuteNonQuery()
        Dim strMsg As String = String.Format("QWatershed_CAL Table에서 {0} 유역 {1}건 Data 삭제됨. 초기화 완료.", strName, intAffectedRecords)
        RaiseEvent RTStatus(strMsg)
        Call Add_Log_toDBMS(strName, strMsg)
    End Sub

    Private Function CreateNewOutputFilesRT() As Boolean
        If Not cOutputControlRT.CreateNewOutputTextFileRT(cProject.Current, mGRMRT) Then Return False
        Return True
    End Function

    Public Sub UpdateFcDatainfoGRMRT(ByVal strDate As String, cvid As Integer,
                                     previousOrder As Integer, dtMIN As Integer)
        Dim fcname As String = cProject.Current.FCGrid.GetFCName(cvid)
        Dim drs As DataRow() = cProject.Current.FCGrid.mdtFCFlowData.Select(String.Format("CVID = {0} and datetime={1}", cvid, strDate))
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
                Dim ascFPN As String = mRfFilePathRT + "\" + GetYearAndMonthFromyyyyMMddHHmm(strDate) + "\" + strDate + ".asc"
                If IO.File.Exists(ascFPN) = True Then
                    mRFLayerCountToApply_RT += 1
                    cThisSimulation.mRFDataCountInThisEvent = mRFLayerCountToApply_RT
                    Dim nr As cRainfall.RainfallData
                    With nr
                        .Order = mRFLayerCountToApply_RT
                        .DataTime = strDate
                        .Rainfall = strDate + ".asc"
                        .FilePath = mRfFilePathRT + "\" + GetYearAndMonthFromyyyyMMddHHmm(strDate)
                        .FileName = strDate + ".asc"
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

    Public Sub ReadDBorCSVandMakeFCdataTableForRealTime(TargetDateTime As String, Optional CSVFPNsource As String = "")
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
                cProject.Current.FCGrid.mdtFCFlowData.TableName = "tmp"
                cProject.Current.FCGrid.mdtFCFlowData.WriteXml("C:\temp\read_method_" + cProject.Current.ProjectNameOnly + "_" + Now.ToString("yyMMddHHmmss") + "grm.xml")
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
                        cProject.Current.FCGrid.mdtFCFlowData.Rows.Add(nr)
                    End If
                    intL += 1
                End While
            End Using
        End If
    End Sub

    Sub StopGRM()
        mSimul.StopSimulate()
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

    Private Sub mSimul_CallAnalyzer(sender As cSimulator, project As cProject, nowTtoPrint_MIN As Integer,
                                    createImgFile As Boolean, createASCFile As Boolean) Handles mSimul.CallAnalyzer
        If project.GeneralSimulEnv.mbEnableAnalyzer = True Then _
        mGrmAnalyzer.CreateDistributionFiles(nowTtoPrint_MIN, mGrmAnalyzer.ImgWidth, mGrmAnalyzer.ImgHeight)
    End Sub

    Private Sub cRealTime_RTStatus(strMSG As String) Handles Me.RTStatus

    End Sub
End Class
