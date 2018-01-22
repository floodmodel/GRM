Imports System.Text
Public Class cRealTime

    'Public Const CONST_bUseDBMS_FOR_RealTimeSystem As Boolean = False   '2015.11.5 원,최,문... : DBMS 매개로 하는 방식
    Public Const CONST_bUseDBMS_FOR_RealTimeSystem As Boolean = True    '2017.5.31 최,원 : 여긴 real time 방식에서는 true로 해야함. 매우 중요함!

    Public Shared CONST_Output_File_Target_DISK As Char = CChar("?") '2017.6.2 원 : png 등을 c ,.d  어디에 기입할지. 구분 

    'Public Const CONST_Output_File_Ori_DISK As String = "d:\"   '2015.11.5 원,최,문... : DBMS 매개로 하는 방식
    '2017.6.1 원 : 기존 방식은. 애초 어디에 쓰려고 하는지. 그것을 어느 drive로 변경할지. 소스코딩. 지금은 대상 drive만 기입하도록 변경함.

    Event RTStatus(ByVal strMSG As String)
    Public mRainfallDataTypeRT As cRainfall.RainfallDataType
    Public mRfFilePathRT As String
    Public mDtPrintOutRT_min As Integer
    Public mSimDurationrRT_Hour As Integer
    Public mRFStartDateTimeRT As String
    Public mbNewRFAddedRT As Boolean
    Public mdicBNewFCdataAddedRT As Dictionary(Of Integer, Boolean)
    Public mlstRFdataRT As List(Of cRainfall.RainfallData) ' TimeSeriesDS.RainfallDataTable
    ''' <summary>
    ''' Get data count by Cvid
    ''' </summary>
    ''' <remarks></remarks>
    Public mdicFCDataCountForEachCV As Dictionary(Of Integer, Integer)
    Public mdicFCNameForEachCV As Dictionary(Of Integer, String)
    Public mdicFCDataOrder As Dictionary(Of Integer, Integer) 'Get current data order for each CVID
    Public mRFLayerCountToApply_RT As Integer
    Public mDateTimeStartRT As DateTime
    Public mbSimulationRTisOngoing As Boolean
    Private Shared mGRMRT As cRealTime
    Public mFPNFcData As String
    Public mPicWidth As Single
    Public mPicHeight As Single
    'Private mfAnalyzer As fAnalyzer
    Private mGrmAnalyzer As cGRMAnalyzer

    Public mbIsDWSS As Boolean
    Public mDWSS_CVID_toConnectUWSS As Integer
    Public mCWSS_CVID_toConnectWithDWSS As Integer
    Public mFPNDWssFCData As String

    'Public g_bUseDBMS_for_RealTimeSystem As Boolean
    'Public g_strDBMSCnn As String

    Private WithEvents mSimul As cSimulator

    Private m_odt_flowcontrolinfo As Data.DataTable '2017.6.20 원영진 : scope 상향


    Public Shared Sub InitializeGRMRT()

        'Public CONST_Output_File_Target_DISK As Char = CChar("?") '2017.6.2 원 : png 등을 c ,.d  어디에 기입할지. 구분 

        Dim strTmp As String =File.ReadAllText("C:\Nakdong\outputDrive.txt")
        If strTmp.ToUpper() <> "C" And strTmp.ToUpper() <> "D" Then
            Console.WriteLine("Can not Read " + strTmp)
            Stop  '2017.6.2 원 : 심각 상황이므로 
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
        mlstRFdataRT = New List(Of cRainfall.RainfallData) 'TimeSeriesDS.RainfallDataTable ' 분석 시작할때 마다 새로 생성
        mRFLayerCountToApply_RT = 0
        If CONST_bUseDBMS_FOR_RealTimeSystem Then
            Call Clear_DBMS_Table_Qwatershed(cProject.Current.ProjectNameOnly)
            RaiseEvent RTStatus("DBMS Qwatershed_CAL Table Cleared")

            Call Add_Log_toDBMS(cProject.Current.ProjectNameOnly, "DBMS Qwatershed Table Cleared")
            '2015.11.5 원 : 
            '   file 방식 ofcd_rt.txt는 초기 가동시 mn 유역 에서 초기화 함. 
            '   DBMS 방식은 여기서 초기화함. 
            '   이를 통하여 모든 유역 일괄 다시 가동이 아닌 특정 유역만다시 돌리기도 지원 모색
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

        'Call UpdateRainfallInformationGRMRT(mRFStartDateTimeRT)

        'If mRFLayerCountToApply_RT = 0 Then
        '    RaiseEvent RTStatus("유출해석 시작 시간에서의 강우자료가 없습니다.")
        '    RaiseEvent RTStatus("강우자료와 유출해석 시작 시간을 확인하시길 바랍니다.")
        '    'RaiseEvent RTStatus("분석종료")
        '    'Exit Sub
        'End If

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
            cProject.Current.FCGrid.mdtFCFlowData = New DataTable       '원 :2017.6.20 부터 사용
            Call ReadDBorCSVandMakeFCdataTableForRealTime(mRFStartDateTimeRT)
            If cProject.Current.FCGrid.mdtFCFlowData.Rows.Count < 1 Then
                RaiseEvent RTStatus("유출해석 시작 시간에서의 flow control 자료가 없습니다.")
                RaiseEvent RTStatus("유출해석 시작시간과 댐방류량, inlet 자료 등 flow control 자료를 확인하시길 바랍니다.")
                'RaiseEvent RTStatus("분석종료")
                'Exit Sub
            End If
            'Else
            '    For nfc As Integer = 0 To cProject.Current.FCGrid.FCCellCount - 1
            '            Dim r As GRMProject.FlowControlGridRow = CType(cProject.Current.FCGrid.mdtFCGridInfo.Rows(nfc),
            '                          GRMProject.FlowControlGridRow)
            '            If cProject.Current.FCGrid.mdtFCFlowData.Select("cvid= " & r.CVID.ToString).Length = 0 Then
            '                mdicFCDataOrder.Add(r.CVID, 0) '첫번째 자료가 없는경우
            '            Else
            '                mdicFCDataOrder.Add(r.CVID, 1) '첫번째 자료가 있는 경우
            '            End If
            '            'If mdicFCDataOrder(r.CVID) = 0 Then
            '            '    Call SearchFCdataRT(dicFCDataOrder, 0)
            '            'End If

            '            Dim dt_MIN As Integer = r.DT '이건 분단위
            '            Dim cvid As Integer = r.CVID
            '            Call UpdateFcDatainfoGRMRT(mRFStartDateTimeRT, cvid, 0, dt_MIN)
            '        Next
            '    End If
        End If
        mSimul.SimulateRT(cProject.Current, Me)
    End Sub

    Private Sub Clear_DBMS_Table_Qwatershed(strName As String)
        '2015.11.5 원 : 추가.. 

        Dim oSQLCon As New System.Data.SqlClient.SqlConnection(g_strDBMSCnn)
        If oSQLCon.State = ConnectionState.Closed Then oSQLCon.Open()
        '2016.10.10 안주영 QWatershed ->QWatershed_CAL, name->wscode 
        'Dim strSQL As String = String.Format("delete QWatershed where name='{0}'", strName)
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
        'Dim lngTimeDiffFromStarting_SEC As Long = DateDiff(DateInterval.Second, cThisSimulation.mTimeThisSimulationStarted, Now)
        'Dim tFromStart As String = Format(lngTimeDiffFromStarting_SEC / 60, "#0.00")
        Dim fcname As String = cProject.Current.FCGrid.GetFCName(cvid)


        'If mdicFCDataCountForEachCV(cvid) > 0 AndAlso mdicBNewFCdataAddedRT(cvid) = True Then   '//2017.7.3 최,원 : 이거  역활 없다고 보임
        '    Dim tmpDateTime As String
        '    tmpDateTime = Format(mDateTimeStartRT.Add _
        '             (New System.TimeSpan(0, (previousOrder - 1) * dtMIN, 0)), "yyyyMMddHHmm")
        '    'RaiseEvent RTStatus(String.Format("FC 자료({0}, CVID={1}, {2}) 분석완료... {3}분 경과",fcname, cvid, tmpDateTime, tFromStart))  '2017/6/20 원 : 생략 가능
        '    'Windows.Forms.Application.DoEvents()
        '    mdicBNewFCdataAddedRT(cvid) = False
        'End If

        'Dim drs As DataRow() = cProject.Current.FCGrid.mdtFCFlowData.Select(String.Format("CVID = {0}", cvid))  '2017.7.2 이전 
        Dim drs As DataRow() = cProject.Current.FCGrid.mdtFCFlowData.Select(String.Format("CVID = {0} and datetime={1}", cvid, strDate))  '2017.7.2 이후. 이부분 생략시 2016.12 영강 사례 처럼 시점 매칭 불일치 

        'Dim count As Integer = drs.Length
        'If count > previousOrder Then   ' 2017.7.2 이전  방식
        If drs.Count > 0 Then   ''2017.7.2 이후
            mdicBNewFCdataAddedRT(cvid) = True
            'mdicFCDataCountForEachCV(cvid) = count '//2017.7.3 최,원 : 이거 역활 없다고 보임
            RaiseEvent RTStatus(String.Format("  FC Data 입력완료...({3} {0}, CVID={1}, Value={2})",
                                              fcname.PadRight(13), cvid.ToString.PadLeft(5), drs(0).Item("value").ToString.PadLeft(8), strDate))
            '2017.7.2 원 : index 0 사용 이유. 2017.7.2 현재 동일 data 가 n건(4건~6건 ) 반환됨. 중복 제거가 안된 것임.  임의 0번 채택
        Else
            mdicBNewFCdataAddedRT(cvid) = False '2017.7.2 원 : 중요 추가... 외부 에서 mdicBNewFCdataAddedRT(cvid)의 T/F를 사용하여 준비여부 판정하므로. 설정함.        If mdicFCDataCountForEachCV(cvid) block 안에서 flase 하는 부분도 필요한지는  최박사님과 같이 상의 예정 
            RaiseEvent RTStatus(String.Format("FC 자료({0}, CVID={1}, {2}) 수신대기 중...",
                                  fcname, cvid, strDate))
        End If
    End Sub

    'Public Sub ReadFCdataFromDBorCSVandUpdateDataTableForRealTime()
    '    'cProject.Current.FCGrid.mdtFCFlowData = New TimeSeriesDS.FlowControlDataDataTable  '원 :2017.6.20 까지 사용


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
                Dim ascFPN As String = mRfFilePathRT + "\" + GetYearAndMonthFromyyyyMMddHHmm(strDate) + "\" + strDate + ".asc"
                If IO.File.Exists(ascFPN) = True Then
                    mRFLayerCountToApply_RT += 1
                    cThisSimulation.mRFDataCountInThisEvent = mRFLayerCountToApply_RT
                    Dim nr As cRainfall.RainfallData 'TimeSeriesDS.RainfallRow = mdtRFinfoRT.NewRainfallRow
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

    Public Sub ReadDBorCSVandMakeFCdataTableForRealTime(TargetDateTime As String, Optional CSVFPNsource As String = "") 'As DataTable '  TimeSeriesDS.FlowControlDataTable

        'CSVFPNsource    '2017.7.2 원 : 실시간 시스템에서는 이게 항상 빈 문자열

        'cProject.Current.FCGrid.mdtFCFlowData = ReadDBorCSVandMakeFCdataTable(mFPNFcData)

        Dim dt As New DataTable ' TimeSeriesDS.FlowControlDataDataTable
        If CONST_bUseDBMS_FOR_RealTimeSystem Then 'Qwatershed Table에 기록
            'flowcontrol table에서... 
            If m_odt_flowcontrolinfo Is Nothing Then    '2017.6.20 원 : FC 구조는 실시간 유역 분석시 변경 없으므로 계속 DB에 문의하지 않도록 함
                m_odt_flowcontrolinfo = New DataTable()
                Dim strSQL As String = String.Format("select * from flowcontrolinfo where WScode='{0}'", cProject.Current.ProjectNameOnly)
                Dim oSqlDataAdapter As New SqlClient.SqlDataAdapter(strSQL, g_strDBMSCnn)
                oSqlDataAdapter.SelectCommand.CommandTimeout = 60   '기본 30에서 60으로 변경 했음
                Try 'DB 접근 장애시. 여기서 오류 2016.5.3
                    oSqlDataAdapter.Fill(m_odt_flowcontrolinfo)
                    oSqlDataAdapter.Dispose()
                Catch ex1 As SqlClient.SqlException
                    Console.WriteLine(ex1.ToString)
                    Console.ReadLine()
                    'Console.WriteLine("10초후 재개 시도")    
                    'Threading.Thread.Sleep(10000)
                    'Exit Function
                    '2016.5.3 원 : n초후 재시도를 모색했으나. 불가 왜냐면 여기 정보 입수 안되면. CalFCReservoirOutFlow 여기서 오류 발생
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
                        ' 2016.12.6 원
                        '   2016.12월초 최윤석 박사,. 원영진 이메일 문답 관련 사항    
                        '   유역에서 유량 넘겨 받는 경우 최하류 지점에서 받아야 함. 유역 중간의 관측소에서 유량 수치 넘겨 받으면 안됨.  
                        '   예를 들어 영강 유역은 외부 유입이 없어서. 여기 소스코드 지나가지 않음
                        '   예를 들어 구담 유역은 DAM 에서 유입이고. 그 경우 상기 소스에서 분기되어 GaugeCode 가 명시적 지정됨
                        '   현재 md 를 기입한 경우 본류명 기입한 경우 혼재하고 있슴.. db modeling 조정이 필요할 듯
                        If strSourceName.Trim() = "GM" Or strSourceName.Trim() = "EB" Then
                            strWhereGage = String.Format("gaugecode='{0}' and", strSourceName)
                        Else
                            strWhereGage = "gaugecode='md' and"
                        End If
                    Case Else
                        Stop
                End Select

                'SQL for time siries
                'Dim strSQL_TS As String = String.Format("select  {0} as cvid, datatime, value from {1} where {3} ='{2}' order by datatime", strCVID, strSourceTableName, strSourceName, strFieldTarget)
                '2016.10.10 안주영 기존 mssql은 datatime 컬럼 사용하지않음 
                'Dim strSQL_TS As String = String.Format("select  {0} as cvid, time, value from {1} where {3} ='{2}' order by time", strCVID, strSourceTableName, strSourceName, strFieldTarget) '2016.12.6 까지 사용하던 코드
                'Dim strSQL_TS As String = String.Format("select  {0} as cvid, time, value from {1} where {4} {3} ='{2}' order by time", strCVID, strSourceTableName, strSourceName, strFieldTarget, strWhereGage) '2016.12.6 부터 사용하는 코드
                'Dim strSQL_TS As String = String.Format("select  {0} as cvid, time as datetime, value from {1} where {4} {3} ='{2}' order by time", strCVID, strSourceTableName, strSourceName, strFieldTarget, strWhereGage) '2017.6.20  부터 사용하는 코드 . by 원
                Dim strSQL_TS As String = String.Format("select  {0} as cvid, time as datetime, value from {1} where {4} {3} ='{2}' and time='{5}'",
                                                        strCVID, strSourceTableName, strSourceName, strFieldTarget, strWhereGage, TargetDateTime) '2017.6.21  부터 사용하는 코드 . by 원

                Dim odt_TS As New Data.DataTable
                Dim oSqlDataAdapter_TS As New SqlClient.SqlDataAdapter(strSQL_TS, g_strDBMSCnn)
                oSqlDataAdapter_TS.SelectCommand.CommandTimeout = 60  '원 2016.5.3 추가 
                oSqlDataAdapter_TS.Fill(odt_TS)
                oSqlDataAdapter_TS.Dispose()

                dt.Merge(odt_TS)    '2017.6.20 원 : 'dt = odt_TS.Copy() 최초는 copy 이후는 merge 해도 되고 바로 0+1 방식의 merge 부터 해도 됨 

                'cProject.Current.FCGrid.mdtFCFlowData.Merge(odt_TS) ' 2017.6.20 최,원 : 한줄로 받아서 세팅하자.  만약 GD 유역의 경우와 같이 상류 2개면 cvid 가 두개, 두줄이 각각 한줄식 추가됨
                '2017.7.2  원  : 이렇게 하니까. 시간에 따라 datatable이 무한정 누적됨. . 그래서 일단 dt 에 담은후 for loop 밖에서 한번에 넘겨주도록 함

                If False Then   '2017.6.20 상기로 1줄 처리후 주석화. by 원
                    'For Each oDR_TS As DataRow In odt_TS.Rows
                    '    Dim nr As TimeSeriesDS.FlowControlDataRow = dt.NewFlowControlDataRow
                    '    nr.CVID = CInt(oDR_TS.Item("CVID"))
                    '    '2016.10.10 안주영 datatime->time 

                    '    ' nr.DataTime = oDR_TS.Item("datatime").ToString
                    '    nr.DataTime = oDR_TS.Item("time").ToString
                    '    nr.Value = CSng(oDR_TS.Item("value"))
                    '    dt.Rows.Add(nr)
                    'Next
                End If
            Next oDR

            cProject.Current.FCGrid.mdtFCFlowData = dt  '2017.7.2 원 : 6/20 코드에선 왜 merge?  그동안 왜 누적 한거지? target time 시점의 정보만 처리하면 예전 시점 data 는 불필요함

            If False Then   '2017.7.2 원 : 디버깅중에 연계 정보를 File 로 확인시 활용
                cProject.Current.FCGrid.mdtFCFlowData.TableName = "tmp"
                cProject.Current.FCGrid.mdtFCFlowData.WriteXml("C:\temp\read_method_" + cProject.Current.ProjectNameOnly + "_" + Now.ToString("yyMMddHHmmss") + "grm.xml")

                'dt.AsEnumerable().Distinct((New DataRowComparer).Dump() ' 참고한 code,. 중복 제거 시도 
                'Dim xx As IEnumerable(Of DataRow) = cProject.Current.FCGrid.mdtFCFlowData.AsEnumerable.Distinct()
                '            Dim xxx As DataTable = xx.CopyToDataTable()

            End If

        Else

            '2017.6.20 원 : 기존  XSD 방식에서 수행하던 스키마 생성은 여기서 해결함
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
                        'Dim nr As TimeSeriesDS.FlowControlDataRow = dt.NewFlowControlDataRow
                        Dim nr As DataRow = dt.NewRow
                        nr("CVID") = CInt(TextIncurrentRow(0))
                        nr("DataTime") = TextIncurrentRow(1)
                        nr("Value") = CSng(TextIncurrentRow(2))
                        'dt.Rows.Add(nr)
                        cProject.Current.FCGrid.mdtFCFlowData.Rows.Add(nr)
                    End If
                    intL += 1
                End While
            End Using
        End If

        'Return dt
        'cProject.Current.FCGrid.mdtFCFlowData = dt
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
            Return CInt(mSimDurationrRT_Hour * 3600) '초로 바꾼다. 
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
