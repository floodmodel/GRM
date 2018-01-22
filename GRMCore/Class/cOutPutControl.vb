Imports System.IO
Imports System.Diagnostics

'Imports Microsoft.Office.Interop
Imports System.Text

Imports System.Drawing
Imports System.Drawing.Drawing2D
Imports System.Drawing.Imaging


Public Class cOutPutControl
    Public Shared Event ProcessText(ByVal pText As String)
    Public Shared Event ProcessValue(ByVal pValue As Integer)

    Public Sub WriteSimResultsToTextFileForSingleEvent(ByVal project As cProject,
                                                ByVal wpCount As Integer,
                                                ByVal nowT_MIN As Integer,
                                                ByVal meanRainfallSumToPrintOut_m As Double,
                                                ByVal interCoef As Single,
                                                ByVal project_tm1 As cProjectBAK)
        Dim timeNow As Date
        'Dim lngTimeDiff As Integer
        Dim lngTimeDiffFromStarting_SEC As Long
        Dim strNowTimeToPrintOut As String
        Dim cvan As Integer
        Dim sngMeanRainfallSumForPrintoutTime_mm As Double
        Dim strFNPDischarge As String
        Dim strFNPDepth As String
        Dim strFNPRFGrid As String
        Dim strFNPRFMean As String
        Dim strFNPFCData As String
        Dim strFNPFCStorage As String
        Dim lineToPrint As String = ""
        Dim vToPrint As String = ""

        sngMeanRainfallSumForPrintoutTime_mm = meanRainfallSumToPrintOut_m * 1000
        timeNow = Now()
        lngTimeDiffFromStarting_SEC = DateDiff(DateInterval.Second, cThisSimulation.mTimeThisSimulationStarted, timeNow)
        'lngTimeDiff = cTimeChecker.GetTimeDiffereceAsMilliSEC(cThisSimulation.mTimeThisStepStarted, timeNow)
        strNowTimeToPrintOut = cComTools.GetTimeToPrintOut(project.GeneralSimulEnv.mIsDateTimeFormat, project.GeneralSimulEnv.mSimStartDateTime, nowT_MIN)

        '파일 이름, 경로 받고
        strFNPDischarge = project.OFNPDischarge
        strFNPDepth = project.OFNPDepth
        strFNPRFGrid = project.OFNPRFGrid
        strFNPRFMean = project.OFNPRFMean
        strFNPFCData = project.OFNPFCData
        strFNPFCStorage = project.OFNPFCStorage

        '===================================================================================================
        '이건 유량
        lineToPrint = strNowTimeToPrintOut
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            cvan = wpcvid - 1
            If interCoef = 1 Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("{0,8:#0.##}", project.CV(cvan).QCVof_i_j_m3Ps) 'CV의 배열 번호는 CVid -1과 같으므로..
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
            'vToPrint = CSng(String.Format("{#0.00000}", vToPrint))
            lineToPrint = lineToPrint + vbTab + vToPrint.Trim  'CV의 배열 번호는 CVid -1과 같으므로..
            Dim sv As Single = CSng(vToPrint)
            With project.WatchPoint
                .mTotalFlow_cms(wpcvid) = .mTotalFlow_cms(wpcvid) + sv ' * CInt(project.GeneralSimulEnv.mPrintOutTimeStep * 60)
                If .mMaxFlow_cms(wpcvid) < sv Then
                    .mMaxFlow_cms(wpcvid) = sv
                    .mMaxFlowTime(wpcvid) = strNowTimeToPrintOut
                End If
            End With
        Next

        'lineToPrint = lineToPrint + vbTab + Format(sngMeanRainfallSumForPrintoutTime_mm, "#0.00") + vbTab + Str(lngTimeDiff) + vbTab + CStr(lngTimeDiffFromStarting_SEC) & vbCrLf
        lineToPrint = lineToPrint + vbTab + Format(sngMeanRainfallSumForPrintoutTime_mm, "#0.00") + vbTab + CStr(lngTimeDiffFromStarting_SEC) & vbCrLf
        IO.File.AppendAllText(strFNPDischarge, lineToPrint, Encoding.Default)
        ''===================================================================================================
        '''삭제 대상
        'If CInt(strNowTimeToPrintOut) = 24 Then cThisSimulation.tmp_24H_RunTime = CStr(lngTimeDiffFromStarting_SEC)
        'If CInt(strNowTimeToPrintOut) = 48 Then cThisSimulation.tmp_48H_RunTime = CStr(lngTimeDiffFromStarting_SEC)

        '''===================================================================================================
        ''이건 수심. 주석처리 2017.04.26
        'lineToPrint = strNowTimeToPrintOut
        ''For intCVCount As Integer = 0 To intWPCount - 1
        'For Each wpcvid As Integer In project.WatchPoint.WPCVidList
        '    cvan = wpcvid - 1

        '    If interCoef = 1 Then
        '        Select Case project.CV(cvan).FlowType
        '            Case cGRM.CellFlowType.OverlandFlow
        '                vToPrint = String.Format("{0,8:#0.#####}", project.CV(cvan).hCVof_i_j) 'CV의 배열 번호는 CVid -1과 같으므로..
        '            Case Else
        '                vToPrint = String.Format("{0,8:#0.#####}", project.CV(cvan).mStreamAttr.hCVch_i_j)
        '        End Select
        '    ElseIf project_tm1 IsNot Nothing Then
        '        Select Case project.CV(cvan).FlowType
        '            Case cGRM.CellFlowType.OverlandFlow
        '                vToPrint = String.Format("{0,8:#0.#####}", cHydroCom.GetInterpolatedValueLinear(
        '                                       project_tm1.CV(cvan).hCVof_i_j,
        '                                       project.CV(cvan).hCVof_i_j, interCoef)) 'CV의 배열 번호는 CVid -1과 같으므로..
        '            Case Else
        '                vToPrint = String.Format("{0,8:#0.#####}", cHydroCom.GetInterpolatedValueLinear(
        '                                       project_tm1.CV(cvan).mStreamAttr.hCVch_i_j,
        '                                       project.CV(cvan).mStreamAttr.hCVch_i_j, interCoef))
        '        End Select
        '    End If

        '    lineToPrint = lineToPrint + vbTab + vToPrint.Trim
        '    Dim sv As Single = CSng(vToPrint)
        '    With project.WatchPoint
        '        .mTotalDepth_m(wpcvid) = .mTotalDepth_m(wpcvid) + sv
        '        If .mMaxDepth_m(wpcvid) < sv Then
        '            .mMaxDepth_m(wpcvid) = sv
        '            .mMaxDepthTime(wpcvid) = strNowTimeToPrintOut
        '        End If
        '    End With
        'Next
        'lineToPrint = lineToPrint + vbTab + Format(sngMeanRainfallSumForPrintoutTime_mm, "#0.00") + vbTab +
        '                Str(lngTimeDiff) + vbTab + CStr(lngTimeDiffFromStarting_SEC) & vbCrLf
        'IO.File.AppendAllText(strFNPDepth, lineToPrint, Encoding.Default)
        ''===================================================================================================
        ''===================================================================
        ''이건 격자별 강우 for watchpoint. 주석처리 2017.04.26
        'lineToPrint = strNowTimeToPrintOut
        'If interCoef = 1 Then
        '    For Each wpcvid As Integer In project.WatchPoint.WPCVidList
        '        With project.WatchPoint
        '            lineToPrint = lineToPrint + vbTab + CStr(.mRFWPGridForDtPrintout_mm(wpcvid))
        '            .mRFWPGridTotal_mm(wpcvid) = CSng(Format(.mRFWPGridTotal_mm(wpcvid) + .mRFWPGridForDtPrintout_mm(wpcvid), "#0.00000"))
        '        End With
        '    Next
        'ElseIf project_tm1 IsNot Nothing Then
        '    For Each wpcvid As Integer In project.WatchPoint.WPCVidList
        '        With project.WatchPoint
        '            lineToPrint = lineToPrint + vbTab + CStr(cHydroCom.GetInterpolatedValueLinear(
        '                                                  CSng(project_tm1.WatchPoint.mRFWPGridForDtPrintout_mm(wpcvid)),
        '                                                  CSng(.mRFWPGridForDtPrintout_mm(wpcvid)), interCoef))
        '            .mRFWPGridTotal_mm(wpcvid) = CSng(Format(.mRFWPGridTotal_mm(wpcvid) +
        '                                                     cHydroCom.GetInterpolatedValueLinear(
        '                                                     CSng(project_tm1.WatchPoint.mRFWPGridForDtPrintout_mm(wpcvid)),
        '                                                     CSng(.mRFWPGridForDtPrintout_mm(wpcvid)), interCoef) _
        '                                                      , "#0.00000"))
        '        End With
        '    Next
        'End If

        'lineToPrint = lineToPrint + vbCrLf
        'IO.File.AppendAllText(strFNPRFGrid, lineToPrint, Encoding.Default)
        ''===================================================================

        ' 이거 주석처리. 별로 필요없는 것 같다. 2018.1.18. 최
        ''===================================================================
        ''이건 watchpoint 상류 격자의 평균 강우량
        'lineToPrint = strNowTimeToPrintOut

        'If interCoef = 1 Then
        '    For Each wpcvid As Integer In project.WatchPoint.WPCVidList
        '        lineToPrint = lineToPrint + vbTab + CStr(Format(project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(wpcvid), "#0.0000"))
        '        With project.WatchPoint
        '            .mRFUpWsMeanTotal_mm(wpcvid) = CSng(Format(.mRFUpWsMeanTotal_mm(wpcvid) + .mRFUpWsMeanForDtPrintout_mm(wpcvid), "#0.0000"))
        '        End With
        '    Next
        'ElseIf project_tm1 IsNot Nothing Then
        '    For Each wpcvid As Integer In project.WatchPoint.WPCVidList
        '        With project.WatchPoint
        '            lineToPrint = lineToPrint + vbTab + Format(cHydroCom.GetInterpolatedValueLinear(
        '                                                 CSng(project_tm1.WatchPoint.mRFUpWsMeanForDtPrintout_mm(wpcvid)),
        '                                                 CSng(.mRFUpWsMeanForDtPrintout_mm(wpcvid)), interCoef), "#0.0000")
        '            .mRFUpWsMeanTotal_mm(wpcvid) = CSng(Format(.mRFUpWsMeanTotal_mm(wpcvid) +
        '                                                       cHydroCom.GetInterpolatedValueLinear(
        '                                                       CSng(project_tm1.WatchPoint.mRFUpWsMeanForDtPrintout_mm(wpcvid)),
        '                                                       CSng(.mRFUpWsMeanForDtPrintout_mm(wpcvid)), interCoef) _
        '                                                       , "#0.00000"))
        '        End With
        '    Next
        'End If

        'lineToPrint = lineToPrint + vbCrLf
        'IO.File.AppendAllText(strFNPRFMean, lineToPrint, Encoding.Default)
        ''===================================================================
        '===================================================================
        '여긴 watchpoint별 모든 자료 출력
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            Dim cvanWP As Integer = wpcvid - 1
            'Dim intNowFCArrayNum As Integer = project.CV(intNowCVArrayNumForWP).FCArrayNum
            Dim strL As String
            strL = strNowTimeToPrintOut + vbTab

            If interCoef = 1 Then
                '유량, 수심만 우선 포멧 적용
                If project.CV(cvanWP).FlowType = cGRM.CellFlowType.OverlandFlow Then
                    strL = strL + String.Format("{0,8:#0.##}", project.CV(cvanWP).QCVof_i_j_m3Ps) + vbTab
                    'strL = strL + String.Format("{0,8:#0.#####}", project.CV(cvanWP).hCVof_i_j) + vbTab
                Else
                    strL = strL + String.Format("{0,8:#0.##}", project.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps) + vbTab
                    'strL = strL + String.Format("{0,8:#0.#####}", project.CV(cvanWP).mStreamAttr.hCVch_i_j) + vbTab
                End If
                strL = strL + Format(project.CV(cvanWP).hUAQfromChannelBed_m, "#0.0000") + vbTab
                strL = strL + Format(project.CV(cvanWP).CumulativeInfiltrationF_m, "#0.0000") + vbTab
                strL = strL + Format(project.CV(cvanWP).soilSaturationRatio, "#0.0000") + vbTab
                strL = strL + Format(project.WatchPoint.mRFWPGridForDtPrintout_mm(wpcvid), "#0.0000") + vbTab
                strL = strL + Format(project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(wpcvid), "#0.0000") + vbTab
                strL = strL + Format(project.WatchPoint.mQfromFCDataCMS(wpcvid), "#0.00") + vbTab
                strL = strL + Format(project.CV(cvanWP).StorageCumulative_m3, "#0.00") + vbCrLf
            ElseIf project_tm1 IsNot Nothing Then
                If project.CV(cvanWP).FlowType = cGRM.CellFlowType.OverlandFlow Then
                    strL = strL + String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                   project_tm1.CV(cvanWP).QCVof_i_j_m3Ps,
                                   project.CV(cvanWP).QCVof_i_j_m3Ps, interCoef)) + vbTab
                    'strL = strL + String.Format("{0,8:#0.#####}", cHydroCom.GetInterpolatedValueLinear(
                    '                    project_tm1.CV(cvanWP).hCVof_i_j,
                    '                    project.CV(cvanWP).hCVof_i_j, interCoef)) + vbTab
                Else
                    strL = strL + String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                   project_tm1.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps,
                                   project.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps, interCoef)) + vbTab
                    'strL = strL + String.Format("{0,8:#0.#####}", cHydroCom.GetInterpolatedValueLinear(
                    '                    project_tm1.CV(cvanWP).mStreamAttr.hCVch_i_j,
                    '                    project.CV(cvanWP).mStreamAttr.hCVch_i_j, interCoef)) + vbTab
                End If

                strL = strL + Format(cHydroCom.GetInterpolatedValueLinear(
                                    project_tm1.CV(cvanWP).hUAQfromChannelBed_m,
                                    project.CV(cvanWP).hUAQfromChannelBed_m, interCoef), "#0.0000") + vbTab
                strL = strL + Format(cHydroCom.GetInterpolatedValueLinear(
                                    project_tm1.CV(cvanWP).CumulativeInfiltrationF_m,
                                    project.CV(cvanWP).CumulativeInfiltrationF_m, interCoef), "#0.0000") + vbTab
                Dim ssv As Single = cHydroCom.GetInterpolatedValueLinear(
                                                    project_tm1.CV(cvanWP).soilSaturationRatio,
                                                    project.CV(cvanWP).soilSaturationRatio, interCoef)

                strL = strL + Format(cHydroCom.GetInterpolatedValueLinear(
                                                    project_tm1.CV(cvanWP).soilSaturationRatio,
                                                    project.CV(cvanWP).soilSaturationRatio, interCoef), "#0.0000") + vbTab

                strL = strL + Format(cHydroCom.GetInterpolatedValueLinear(
                                    CSng(project_tm1.WatchPoint.mRFWPGridForDtPrintout_mm(wpcvid)),
                                    CSng(project.WatchPoint.mRFWPGridForDtPrintout_mm(wpcvid)), interCoef), "#0.0000") + vbTab
                strL = strL + Format(cHydroCom.GetInterpolatedValueLinear(
                                    CSng(project_tm1.WatchPoint.mRFUpWsMeanForDtPrintout_mm(wpcvid)),
                                    CSng(project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(wpcvid)), interCoef), "#0.0000") + vbTab
                strL = strL + Format(cHydroCom.GetInterpolatedValueLinear(
                                    CSng(project_tm1.WatchPoint.mQfromFCDataCMS(wpcvid)),
                                    CSng(project.WatchPoint.mQfromFCDataCMS(wpcvid)), interCoef), "#0.00") + vbTab
                strL = strL + Format(cHydroCom.GetInterpolatedValueLinear(
                                    project_tm1.CV(cvanWP).StorageCumulative_m3,
                                    project.CV(cvanWP).StorageCumulative_m3, interCoef), "#0.00") + vbCrLf
            End If
            IO.File.AppendAllText(project.WatchPoint.mFpnWpOut(wpcvid), strL, Encoding.Default)
            ''여기서 출력 시간간격에 대한 강우 값 초기화
            'project.WatchPoint.mRFWPGridForDtPrintout_mm(wpcvid) = 0
            'project.WatchPoint.mRFUpWsMeanForDtPrintout_mm(wpcvid) = 0
        Next

        '===================================================================
        '이건 FCAppFlow, FCStorage
        If project.GeneralSimulEnv.mbSimulateFlowControl = True AndAlso project.FCGrid.FCCellCount > 0 Then
            Dim strLFlow As String = strNowTimeToPrintOut
            Dim strLStorage As String = strNowTimeToPrintOut
            If interCoef = 1 Then
                For Each fcCvid As Integer In project.FCGrid.FCGridCVidList
                    strLFlow = strLFlow + vbTab + Format(project.FCGrid.mFCdataToApplyNowT(fcCvid), "#0.00")
                    strLStorage = strLStorage + vbTab + Format(project.CV(fcCvid - 1).StorageCumulative_m3, "#0.00")
                Next
            ElseIf project_tm1 IsNot Nothing Then
                For Each fcCvid As Integer In project.FCGrid.FCGridCVidList
                    strLFlow = strLFlow + vbTab + Format(cHydroCom.GetInterpolatedValueLinear(
                                                        CSng(project_tm1.FCGrid.mFCdataToApplyNowT(fcCvid)),
                                                        CSng(project.FCGrid.mFCdataToApplyNowT(fcCvid)), interCoef), "#0.00")
                    strLStorage = strLStorage + vbTab + Format(cHydroCom.GetInterpolatedValueLinear(
                                                             project_tm1.CV(fcCvid - 1).StorageCumulative_m3,
                                                             project.CV(fcCvid - 1).StorageCumulative_m3, interCoef), "#0.00")
                Next
            End If
            strLFlow = strLFlow + vbCrLf
            strLStorage = strLStorage + vbCrLf
            IO.File.AppendAllText(strFNPFCData, strLFlow, Encoding.Default)
            IO.File.AppendAllText(strFNPFCStorage, strLStorage, Encoding.Default)
        End If
        '===================================================================

        If nowT_MIN = CInt(project.GeneralSimulEnv.mSimDurationHOUR * 60) Then
            project.GeneralSimulEnv.mEndingTimeToPrint = strNowTimeToPrintOut
        End If

        'cThisSimulation.mPrintOutResultRowCount = cThisSimulation.mPrintOutResultRowCount + 1
        'cThisSimulation.mTimeThisStepStarted = timeNow  '2017.6.1 원 : 저의 생각은 이거 불필요
    End Sub


    Public Sub WriteDischargeOnlyToDischargeFile(ByVal project As cProject,
                                                ByVal interCoef As Single,
                                                ByVal project_tm1 As cProjectBAK)
        Dim cvan As Integer
        Dim strFNPDischarge As String
        Dim lineToPrint As String = ""
        Dim vToPrint As String = ""
        strFNPDischarge = project.OFNPDischarge
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            cvan = wpcvid - 1
            If interCoef = 1 Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("{0,8:#0.#####}", project.CV(cvan).QCVof_i_j_m3Ps)
                    Case Else
                        vToPrint = String.Format("{0,8:#0.#####}", project.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps)
                End Select
            ElseIf project_tm1 IsNot Nothing Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("{0,8:#0.#####}", cHydroCom.GetInterpolatedValueLinear(
                                               project_tm1.CV(cvan).QCVof_i_j_m3Ps,
                                               project.CV(cvan).QCVof_i_j_m3Ps, interCoef))
                    Case Else
                        vToPrint = String.Format("{0,8:#0.#####}", cHydroCom.GetInterpolatedValueLinear(
                                                project_tm1.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps,
                                                project.CV(cvan).mStreamAttr.QCVch_i_j_m3Ps, interCoef))
                End Select
            End If
            If lineToPrint.Trim = "" Then
                lineToPrint = vToPrint.Trim
            Else
                lineToPrint = lineToPrint.Trim + vbTab + vToPrint.Trim
            End If

        Next
        IO.File.AppendAllText(strFNPDischarge, lineToPrint & vbCrLf, Encoding.Default)
    End Sub

    Public Sub WriteDischargeOnlyToWPFile(ByVal project As cProject,
                                                ByVal interCoef As Single,
                                                ByVal project_tm1 As cProjectBAK)
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            Dim cvanWP As Integer = wpcvid - 1
            Dim strL As String = ""
            If interCoef = 1 Then
                '유량, 수심만 우선 포멧 적용
                If project.CV(cvanWP).FlowType = cGRM.CellFlowType.OverlandFlow Then
                    strL = String.Format("{0,8:#0.##}", project.CV(cvanWP).QCVof_i_j_m3Ps) + vbCrLf
                Else
                    strL = String.Format("{0,8:#0.##}", project.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps) + vbCrLf
                End If
            ElseIf project_tm1 IsNot Nothing Then
                If project.CV(cvanWP).FlowType = cGRM.CellFlowType.OverlandFlow Then
                    strL = String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                   project_tm1.CV(cvanWP).QCVof_i_j_m3Ps,
                                   project.CV(cvanWP).QCVof_i_j_m3Ps, interCoef)) + vbCrLf
                Else
                    strL = String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                   project_tm1.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps,
                                   project.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps, interCoef)) + vbCrLf
                End If
            End If
            Dim wpName As String = project.WatchPoint.wpName(wpcvid)
            Dim nFPN As String = Path.Combine(project.ProjectPath, project.ProjectNameOnly & "WP_" & wpName & ".out")
            IO.File.AppendAllText(nFPN, strL, Encoding.Default)
        Next
    End Sub

    Public Shared Function CreateNewOutputFiles(ByVal project As cProject, deleteOnly As Boolean) As Boolean
        '모의 시작시에는 다 지우고 새로 생성
        Try
            cThisSimulation.mGRMSetupIsNormal = True
            Dim wpCount As Integer = project.WatchPoint.WPCount
            Dim strFNPDischarge As String = project.OFNPDischarge
            Dim strFNPDepth As String = project.OFNPDepth
            Dim strFNPRFGrid As String = project.OFNPRFGrid
            Dim strFNPRFMean As String = project.OFNPRFMean
            Dim strFNPFCData As String = project.OFNPFCData
            Dim strFNPFCStorage As String = project.OFNPFCStorage
            Dim FPNs As New List(Of String)
            With FPNs
                .Add(strFNPDischarge)
                .Add(strFNPDepth)
                .Add(strFNPRFGrid)
                .Add(strFNPRFMean)
                .Add(strFNPFCData)
                .Add(strFNPFCStorage)
                For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo.Rows
                    '여긴 wp 별 출력파일
                    Dim wpName As String = Replace(row.Name, ",", "_")
                    Dim wpfpn As String = Path.Combine(project.ProjectPath, project.ProjectNameOnly & "WP_" & wpName & ".out")
                    .Add(wpfpn)
                Next
            End With
            If cFile.ConfirmDeleteFiles(FPNs) = False Then
                cThisSimulation.mGRMSetupIsNormal = False
                Exit Function
            End If

            If deleteOnly = True Then Return True

            If cProject.Current.GeneralSimulEnv.mPrintOption = cGRM.GRMPrintType.All Then
                If project.GeneralSimulEnv.mbEnableAnalyzer = True Then
                    Dim Dpath As New List(Of String)
                    If project.GeneralSimulEnv.mbShowSoilSaturation = True Then
                        Dpath.Add(project.OFPSSRDistribution)
                    End If

                    If project.GeneralSimulEnv.mbShowRFdistribution = True Then
                        Dpath.Add(project.OFPRFDistribution)
                    End If

                    If project.GeneralSimulEnv.mbShowRFaccDistribution = True Then
                        Dpath.Add(project.OFPRFAccDistribution)
                    End If

                    If project.GeneralSimulEnv.mbShowFlowDistribution = True Then
                        Dpath.Add(project.OFPFlowDistribution)
                    End If

                    If Dpath.Count > 0 Then
                        If cFile.ConfirmDeleteDirectory(Dpath) = False Then
                            cThisSimulation.mGRMSetupIsNormal = False
                            Exit Function
                        End If
                        If cFile.ConfirmCreateDirectory(Dpath) = False Then
                            cThisSimulation.mGRMSetupIsNormal = False
                            Exit Function
                        End If
                    End If
                End If

                '===========================================================
                '여긴 해더
                Dim strOutPutLine As String
                Dim strOutputCommonHeader As String
                strOutputCommonHeader = String.Format("Project name : {0} {1} {2} {3} by {4}{5}",
                                                      project.ProjectNameWithExtension,
                                                      vbTab, Format(Now, "yyyy/MM/dd HH:mm"),
                                                      vbTab, cGRM.BuildInfo.ProductName, vbCrLf)
                Dim strLTime_WPName As String
                strLTime_WPName = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME

                For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo.Rows
                    '여긴 시간필드와 wp 이름 필드 설정
                    strLTime_WPName = strLTime_WPName + vbTab + "[" & row.Name & "]"

                    '----------------------------------------------------
                    '여긴 wp 별 모든 결과를 출력하기 위한 wp 별 출력파일 설정
                    Dim wpName As String = Replace(row.Name, ",", "_")
                    Dim nFPN As String = Path.Combine(project.ProjectPath, project.ProjectNameOnly & "WP_" & wpName & ".out")
                    FPNs.Clear()
                    FPNs.Add(nFPN)
                    'cComTools.ConfirmDeleteFiles(FPNs)
                    If cFile.ConfirmDeleteFiles(FPNs) = False Then
                        cThisSimulation.mGRMSetupIsNormal = False
                        Exit Function
                    End If
                    'Dim strL As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + vbTab + "Discharge[cms]" + vbTab + "Depth[m]" + vbTab + "BaseFlowDepth[m]" + vbTab +
                    '                     "CumulativeInfiltrion[m]" + vbTab + "SSR" + vbTab + "RFGrid" + vbTab + "RFUpMean" + vbTab + "FCData" + vbTab + "FCResStor" + vbCrLf
                    Dim strL As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + vbTab + "Discharge[cms]" + vbTab + "BaseFlowDepth[m]" + vbTab +
                                         "CumulativeInfiltrion[m]" + vbTab + "SSR" + vbTab + "RFGrid" + vbTab + "RFUpMean" + vbTab + "FCData" + vbTab + "FCResStor" + vbCrLf
                    IO.File.AppendAllText(nFPN, strOutputCommonHeader, Encoding.Default)
                    IO.File.AppendAllText(nFPN, "Output data : All the results for watch point '" + wpName + "'" + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(nFPN, strL, Encoding.Default)
                    project.WatchPoint.mFpnWpOut(row.CVID) = nFPN
                Next

                '----------------------------------------------------
                '이건 유량
                If Not IO.Directory.Exists(IO.Path.GetDirectoryName(strFNPDischarge)) Then IO.Directory.CreateDirectory(IO.Path.GetDirectoryName(strFNPDischarge))  '2017.5.31 원 추가
                IO.File.AppendAllText(strFNPDischarge, strOutputCommonHeader, Encoding.Default)
                strOutPutLine = "Output data : " & "Discharge[CMS]" & vbCrLf & vbCrLf
                IO.File.AppendAllText(strFNPDischarge, strOutPutLine, Encoding.Default)

                'strOutPutLine = strLTime_WPName + vbTab + "Rainfall_Mean" + vbTab + "ThisStep[msec]" + vbTab + "FromStarting[sec]" & vbCrLf
                strOutPutLine = strLTime_WPName + vbTab + "Rainfall_Mean" + vbTab + "FromStarting[sec]" & vbCrLf
                IO.File.AppendAllText(strFNPDischarge, strOutPutLine, Encoding.Default)

                ''----------------------------------------------------
                ''이건 수심
                'IO.File.AppendAllText(strFNPDepth, strOutputCommonHeader, Encoding.Default)
                'strOutPutLine = "Output data : " & "Depth[m]" & vbCrLf & vbCrLf
                'IO.File.AppendAllText(strFNPDepth, strOutPutLine, Encoding.Default)

                'strOutPutLine = strLTime_WPName + vbTab + "Rainfall_Mean" + vbTab + "ThisStep[msec]" + vbTab + "FromStarting[sec]" & vbCrLf
                'IO.File.AppendAllText(strFNPDepth, strOutPutLine, Encoding.Default)
                ''----------------------------------------------------

                '----------------------------------------------------
                ''이건 강우. 주석처리. 2017.04.26. 최
                'IO.File.AppendAllText(strFNPRFGrid, strOutputCommonHeader, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFGrid, "Output data : Rainfall for each watchpoint[mm]" + vbCrLf & vbCrLf, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFGrid, strLTime_WPName & vbCrLf, Encoding.Default)

                ''이건 wp별 유역 평균강우량 주석처리. 2018.1.18. 최
                'IO.File.AppendAllText(strFNPRFMean, strOutputCommonHeader, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFMean, "Output data : Mean rainfall for upstream of each watchpoint[mm]" + vbCrLf & vbCrLf, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFMean, strLTime_WPName & vbCrLf, Encoding.Default)
                ''----------------------------------------------------

                '----------------------------------------------------
                '여기는 flow control 모듈 관련
                Dim strNameFCApp As String
                Dim strTypeFCApp As String
                Dim strSourceDT As String
                Dim strResOperation As String

                Dim strROiniStorage As String
                Dim strROmaxStorage As String
                Dim strROmaxStorageRatio As String
                Dim strROmaxStorageApp As String
                Dim strROType As String
                Dim strROConstQ As String
                Dim strROConstQduration As String

                strNameFCApp = "FCName:"
                strTypeFCApp = "FCType:"
                strSourceDT = "SourceDT[min]:"
                strResOperation = "ResOperation:"
                strROiniStorage = "StorageINI:"
                strROmaxStorage = "StorageMax:"
                strROmaxStorageRatio = "StorageMaxRatio:"
                strROmaxStorageApp = "StorageMaxApp:"
                strROType = "ROType:"
                strROConstQ = "ConstQ:"
                strROConstQduration = "ConstQDuration:"
                Dim strFCDataField As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME
                'Dim strFCResOperationField As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME
                If cProject.Current.GeneralSimulEnv.mbSimulateFlowControl = True AndAlso project.FCGrid.FCCellCount > 0 Then
                    For Each row As GRMProject.FlowControlGridRow In project.FCGrid.mdtFCGridInfo
                        strFCDataField = strFCDataField + vbTab + row.Name
                        strNameFCApp = strNameFCApp + vbTab + row.Name
                        strTypeFCApp = strTypeFCApp + vbTab + row.ControlType
                        If row.ControlType = cFlowControl.FlowControlType.ReservoirOutflow.ToString _
                            Or row.ControlType = cFlowControl.FlowControlType.Inlet.ToString Then
                            strSourceDT = strSourceDT + vbTab + CStr(row.DT)
                            strResOperation = strResOperation + vbTab + "FALSE"
                        Else
                            strSourceDT = strSourceDT + vbTab + "NONE"
                            If Not row.IsMaxStorageNull AndAlso Not row.IsMaxStorageRNull Then
                                If row.MaxStorage * row.MaxStorageR > 0 Then
                                    strResOperation = strResOperation + vbTab + "TRUE"
                                Else
                                    strResOperation = strResOperation + vbTab + "FALSE"
                                End If
                            End If
                        End If
                        If Not row.IsIniStorageNull Then strROiniStorage = strROiniStorage + vbTab + CStr(row.IniStorage)
                        If Not row.IsMaxStorageNull Then strROmaxStorage = strROmaxStorage + vbTab + CStr(row.MaxStorage)
                        If Not row.IsMaxStorageRNull Then strROmaxStorageRatio = strROmaxStorageRatio + vbTab + CStr(row.MaxStorageR)
                        If Not row.IsMaxStorageRNull Then strROmaxStorageApp = strROmaxStorageApp + vbTab + CStr(row.MaxStorage * row.MaxStorageR)
                        If Not row.IsROTypeNull Then strROType = strROType + vbTab + row.ROType
                        If Not row.IsROConstQNull Then strROConstQ = strROConstQ + vbTab + CStr(row.ROConstQ)
                        If Not row.IsROConstQDurationNull Then strROConstQduration = strROConstQduration + vbTab + CStr(row.ROConstQDuration)
                    Next

                    '여긴 FCApp - flow control data
                    IO.File.AppendAllText(strFNPFCData, strOutputCommonHeader, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, "Output data : Flow control data input[CMS]" + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strNameFCApp & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strTypeFCApp & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strSourceDT & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strResOperation + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strFCDataField & vbCrLf, Encoding.Default)

                    '여긴 reservoir operation
                    IO.File.AppendAllText(strFNPFCStorage, strOutputCommonHeader, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, "Output data : Storage data[m^3]" + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strNameFCApp & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strTypeFCApp & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strROiniStorage & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strROmaxStorage & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strROmaxStorageRatio & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strROmaxStorageApp & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strROType & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strROConstQ & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strROConstQduration + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCStorage, strFCDataField & vbCrLf, Encoding.Default)
                    '----------------------------------------------------
                End If
            End If
        Catch ex As Exception
            System.Console.WriteLine("Error: Some output files are not deleted.        " &
                   vbCrLf & "Click 'Start simulation' button anagin.")
            Return False
        End Try

        Return True
    End Function


    '''' <summary>
    '''' 현재까지 구동된 유역 매개변수 출력, 매번 런할때 마다, 그리고, 저장할때도 출력
    '''' </summary>
    '''' <param name="project"></param>
    '''' <remarks></remarks>
    'Public Shared Sub SaveSWSParsTextFile(ByVal project As cProject)
    '    Dim fPathName As String = project.OFNPSwsPars
    '    If IO.File.Exists(fPathName) Then IO.File.Delete(fPathName)

    '    Dim LtoWrite As String = ""
    '    LtoWrite = String.Format("Project name : {0} {1} {2}{3}{3}", _
    '                              project.ProjectNameWithExtension, _
    '                             vbTab, Format(Now, "yyyy/MM/dd HH:mm"), vbCrLf)
    '    LtoWrite = LtoWrite + "ID" + vbTab + _
    '                            "IsUserSet" + vbTab + _
    '                            "IniSaturation" + vbTab + _
    '                            "MinSlopeOF" + vbTab + _
    '                            "MinSlopeChBed" + vbTab + _
    '                            "MinChBaseWidth" + vbTab + _
    '                            "ChRoughness" + vbTab + _
    '                            "DryStreamOrder" + vbTab + _
    '                            "IniFlow" + vbTab + _
    '                            "CalCoefLCRoughness" + vbTab + _
    '                            "CalCoefPorosity" + vbTab + _
    '                            "CalCoefWFSuctionHead" + vbTab + _
    '                            "CalCoefHydraulicK" + vbTab + _
    '                            "CalCoefSoilDepth" + vbCrLf

    '    For Each wsid As Integer In project.Watershed.WSIDList
    '        With project.SubWSPar.userPars(wsid)
    '            LtoWrite = LtoWrite + wsid.ToString & vbTab
    '            LtoWrite = LtoWrite + .isUserSet.ToString & vbTab
    '            LtoWrite = LtoWrite + .iniSaturation.ToString & vbTab
    '            LtoWrite = LtoWrite + .minSlopeOF.ToString & vbTab
    '            LtoWrite = LtoWrite + .minSlopeChBed.ToString & vbTab
    '            LtoWrite = LtoWrite + .minChBaseWidth.ToString & vbTab
    '            LtoWrite = LtoWrite + .chRoughness.ToString & vbTab
    '            LtoWrite = LtoWrite + .dryStreamOrder.ToString & vbTab
    '            If .iniFlow Is Nothing Then
    '                LtoWrite = LtoWrite + "  " & vbTab
    '            Else
    '                LtoWrite = LtoWrite + .iniFlow.Value.ToString & vbTab
    '            End If
    '            LtoWrite = LtoWrite + .ccLCRoughness.ToString & vbTab
    '            LtoWrite = LtoWrite + .ccPorosity.ToString & vbTab
    '            LtoWrite = LtoWrite + .ccWFSuctionHead.ToString & vbTab
    '            LtoWrite = LtoWrite + .ccHydraulicK.ToString & vbTab
    '            LtoWrite = LtoWrite + .ccSoilDepth.ToString & vbTab & vbCrLf
    '        End With
    '    Next
    '    IO.File.AppendAllText(fPathName, LtoWrite, Encoding.Default)
    'End Sub


#Region "Output after stop or complete simulation"
    'Public Shared Sub saveSimulationResultToMDBandXLS(ByVal curProject As cProject)
    '    If cThisSimulation.mGRMSetupIsNormal = True AndAlso cProject.Current.WatchPoint.WPCount > 0 Then
    '        SaveGRMOutputToDB(curProject)
    '    End If

    '    If IO.File.Exists(cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xlsx") = True Or _
    '       IO.File.Exists(cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xls") = True Or _
    '       IO.File.Exists(cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xlsm") = True Then

    '        Dim strFPNExel As String = ""

    '        If IO.File.Exists(cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xlsx") = True Then _
    '             strFPNExel = cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xlsx"

    '        If IO.File.Exists(cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xls") = True Then _
    '             strFPNExel = cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xls"

    '        If IO.File.Exists(cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xlsm") = True Then _
    '             strFPNExel = cProject.Current.ProjectPath & "\" & cProject.Current.ProjectNameOnly & ".xlsm"

    '        'Call SaveToProjectExcelFile_Discharge(gstrOFNPDischarge, strFPNExel, gintWPCount)
    '    End If
    'End Sub

    'Private Shared Sub SaveGRMOutputToDB(ByVal project As cProject) 'ByVal intWPCountToPrintOut As Integer, ByVal dlgSetNRunGRM As frmSetupRunGRM)
    '    Try
    '        Dim selectQuery As String
    '        'Dim bDropTable As Boolean
    '        Dim strTimeFiledName As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME
    '        Dim strMeanRainfallFieldName As String = cGRM.CONST_OUTPUT_TABLE_MEAN_RAINFALL_FIELD_NAME

    '        RaiseEvent ProcessText("Saving..")
    '        If cThisSimulation.mPrintOutResultRowCount = 0 Then Exit Sub

    '        project.TSSummary.OpenDB(project.ProjectDBxmlFpn)
    '        Dim rows As DataRow() = project.TSSummary.mdtTSSummary.Select("TS_source = 'Simulated'")
    '        For Each row As DataRow In rows
    '            Dim drow As GRMDynamicDB.TimeSeriesDataSummaryRow = CType(row, GRMDynamicDB.TimeSeriesDataSummaryRow)
    '            project.TSSummary.mdtTSSummary.RemoveTimeSeriesDataSummaryRow(drow)
    '        Next

    '        'todo : 이건 챠팅부분과 함께 검토
    '        'If project.gintNCountRemovedWP > 0 Then
    '        '    Call DeleteRecordInTimeSeriesDataSummary(project.gintNCountRemovedWP)
    '        'End If
    '        'If project.OdbCnnDynamic.State = ConnectionState.Closed Then _
    '        '    project.OdbCnnDynamic.Open()
    '        '===================================================================
    '        '여기서는 유량
    '        RaiseEvent ProcessText("DischargeToDB...")
    '        selectQuery = "select * from OutputDischarge"
    '        'bDropTable = cComTools.ConfirmMDBStates(selectQuery, project.OdbCnnDynamic)
    '        'Call DropAndCreateTableDischargeOrDepth(project, cGRM.OutputTableName.OutputDischarge, strMeanRainfallFieldName, bDropTable)
    '        'Call ReadOutputTextAndSaveToDB(project.OFNPDischarge, cGRM.OutputTableName.OutputDischarge.ToString, project.OdbCnnDynamic)
    '        Call AddSimulatedRowToTSSummaryTable(project, "Discharge", "Simulated", "CMS")
    '        RaiseEvent ProcessValue(1)
    '        '===================================================================

    '        '===================================================================
    '        '여기서는 수심
    '        RaiseEvent ProcessText("DepthToDB...")
    '        selectQuery = "select * from OutputDepth"
    '        bDropTable = cComTools.ConfirmMDBStates(selectQuery, project.OdbCnnDynamic)
    '        Call DropAndCreateTableDischargeOrDepth(project, cGRM.OutputTableName.OutputDepth, strMeanRainfallFieldName, bDropTable)
    '        Call ReadOutputTextAndSaveToDB(project.OFNPDepth, cGRM.OutputTableName.OutputDepth.ToString, project.OdbCnnDynamic)
    '        Call AddSimulatedRowToTSSummaryTable(project, "Depth", "Simulated", "m")
    '        RaiseEvent ProcessValue(2)
    '        '===================================================================

    '        '===================================================================
    '        '여기서는 격자별강우
    '        RaiseEvent ProcessText("RFGridToDB...")
    '        selectQuery = "select * from RainfallWPGrid"
    '        bDropTable = cComTools.ConfirmMDBStates(selectQuery, project.OdbCnnDynamic)
    '        Call DropAndCreateTable(project, cGRM.OutputTableName.RainfallWPGrid, bDropTable)
    '        Call ReadOutputTextAndSaveToDB(project.OFNPRFGrid, cGRM.OutputTableName.RainfallWPGrid.ToString, project.OdbCnnDynamic)
    '        RaiseEvent ProcessValue(3)
    '        '===================================================================

    '        '===================================================================
    '        '여기서는 격자별 상류유역 평균강우
    '        RaiseEvent ProcessText("RFUpMeanToDB...")
    '        selectQuery = "select * from RainfallWPUpMean"
    '        bDropTable = cComTools.ConfirmMDBStates(selectQuery, project.OdbCnnDynamic)
    '        Call DropAndCreateTable(project, cGRM.OutputTableName.RainfallWPUpMean, bDropTable)
    '        Call ReadOutputTextAndSaveToDB(project.OFNPRFMean, cGRM.OutputTableName.RainfallWPUpMean.ToString, project.OdbCnnDynamic)
    '        RaiseEvent ProcessValue(4)
    '        '===================================================================

    '        '===================================================================
    '        ''여기서는 FCData
    '        'RaiseEvent ProcessText("FCDataToDB...")
    '        'selectQuery = "select * from FCDataWP"
    '        'bDropTable = cComTools.ConfirmMDBStates(selectQuery, project.OdbCnnDynamic)
    '        'If IO.File.Exists(project.OFNPFCData) Then
    '        '    Call DropAndCreateTable(project, "FCDataWP", bDropTable)
    '        '    Call ReadOutputTextAndSaveToDB(project.OFNPFCData, "FCDataWP", project.OdbCnnDynamic)
    '        'End If
    '        'RaiseEvent ProcessValue(5)
    '        ''===================================================================

    '        '===================================================================
    '        ''여기서는 저수지 저수량 자료
    '        'RaiseEvent ProcessText("ResStorageToDB...")
    '        'selectQuery = "select * from FCResStorage"
    '        'bDropTable = cComTools.ConfirmMDBStates(selectQuery, project.OdbCnnDynamic)
    '        'If IO.File.Exists(project.OFNPFCStorage) Then
    '        '    Call DropAndCreateTable(project, "FCResStorage", bDropTable)
    '        '    Call ReadOutputTextAndSaveToDB(project.OFNPFCStorage, "FCResStorage", project.OdbCnnDynamic)
    '        'End If
    '        'RaiseEvent ProcessValue(6)
    '        '===================================================================

    '        RaiseEvent ProcessText("Create WPTS tables...")
    '        Call DropAndCreateWPTSTable(project)
    '        For Each row As GRMDynamicDB.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
    '            Dim strWPTableName As String = "WPTS_" + Replace(row.Name, ",", "_")
    '            Call ReadOutputTextAndSaveToDB(project.WatchPoint.mFpnWpOut(row.CVID), strWPTableName, project.OdbCnnDynamic)
    '        Next

    '        project.TSSummary.SaveMDB(project.OdbCnnDynamic)
    '        project.OdbCnnDynamic.Close()
    '        RaiseEvent ProcessValue(7)
    '        '===================================================================
    '    Catch ex As Exception
    '        MsgBox(String.Format("An error was occured while saving {0}.", project.ProjectDBxmlFpn), _
    '               MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
    '        If project.OdbCnnDynamic.State = ConnectionState.Open Then
    '            project.OdbCnnDynamic.Close()
    '        End If
    '    End Try
    'End Sub

    'Private Shared Sub DropAndCreateTableDischargeOrDepth(ByVal project As cProject, ByVal TableName As cGRM.OutputTableName, ByVal strMeanRainfallFieldName As String, ByVal bDropTable As Boolean)
    '    Dim strCreateTableQuery As String
    '    Dim odbCMD As New OleDb.OleDbCommand
    '    Dim strTimeFiledName As String

    '    Try
    '        strTimeFiledName = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME
    '        odbCMD = project.OdbCnnDynamic.CreateCommand

    '        If bDropTable = True Then
    '            strCreateTableQuery = "drop table " & TableName.ToString
    '            odbCMD.CommandText = strCreateTableQuery
    '            odbCMD.ExecuteNonQuery()
    '        End If

    '        strCreateTableQuery = "create table " & TableName.ToString & "( DataNo long, [" + strTimeFiledName + "] string "
    '        For Each row As GRMDynamicDB.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
    '            strCreateTableQuery = strCreateTableQuery + ", " + Replace(row.Name, ",", "_") + " single"
    '        Next

    '        strCreateTableQuery = strCreateTableQuery + ", " + strMeanRainfallFieldName + " single, ThisStep_sec single,	FromStarting_sec single );"
    '        odbCMD.CommandText = strCreateTableQuery
    '        odbCMD.ExecuteNonQuery()

    '    Catch ex As Exception
    '        Throw
    '    End Try

    'End Sub



    'Private Shared Sub DropAndCreateTable(ByVal project As cProject, ByVal TableName As cGRM.OutputTableName, ByVal bDropTable As Boolean)
    '    Try
    '        Dim strTimeFiledName As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME
    '        Dim odbCMD As New OleDb.OleDbCommand
    '        odbCMD = project.OdbCnnDynamic.CreateCommand
    '        Dim strCreateTableQuery As String
    '        If bDropTable = True Then
    '            strCreateTableQuery = "drop table " & TableName.ToString
    '            odbCMD.CommandText = strCreateTableQuery
    '            odbCMD.ExecuteNonQuery()
    '        End If

    '        strCreateTableQuery = "create table " & TableName.ToString & "( DataNo long, [" + strTimeFiledName + "] string "
    '        For Each row As GRMDynamicDB.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
    '            strCreateTableQuery = strCreateTableQuery + ", " + Replace(row.Name, ",", "_") + " single"
    '        Next
    '        strCreateTableQuery = strCreateTableQuery + " );"
    '        odbCMD.CommandText = strCreateTableQuery
    '        odbCMD.ExecuteNonQuery()
    '    Catch ex As Exception
    '        Throw
    '    End Try
    'End Sub


    'Sub ReadOutputTextAndSaveToDB(ByVal strFPathNameTxtFile As String, ByVal dtInput As Data.DataTable, ByVal strInputTableName As String)
    'Private Shared Sub ReadOutputTextAndSaveToDB(ByVal strFPathNameTxtFile As String, ByVal strInputTableName As String, ByVal projectCNN As OleDb.OleDbConnection)

    '    Dim intNRowTxt As Integer
    '    Dim intColindex As Integer
    '    Try

    '        If strFPathNameTxtFile <> "" Then
    '            Dim commandString As String = "select * from " & strInputTableName
    '            Dim dtInput As New Data.DataTable(strInputTableName)
    '            Dim DBAdapter As New OleDbDataAdapter(commandString, projectCNN)
    '            Dim oCommandBuilder As New OleDbCommandBuilder(DBAdapter)  '이거 주석처리하면 밑의 DBAdapter.Update(DSinput, strInputTableName) 여기서 insert 문 오류 남. 2008.12.02
    '            DBAdapter.Fill(dtInput) ', strInputTableName)
    '            Dim TxtReader As New FileIO.TextFieldParser(strFPathNameTxtFile)
    '            TxtReader.TextFieldType = FileIO.FieldType.Delimited '필드가 구분됨
    '            TxtReader.SetDelimiters(vbTab)
    '            Dim currentRow As String()
    '            intNRowTxt = 0
    '            dtInput.Rows.Clear()
    '            Dim intLineNumberToSkip As Integer
    '            Dim strOutputType As String = strInputTableName
    '            If Left(strOutputType, 4) = "WPTS" Then strOutputType = "WPTS"
    '            Select Case strOutputType
    '                Case cGRM.OutputTableName.OutputDischarge.ToString : intLineNumberToSkip = 3
    '                Case cGRM.OutputTableName.OutputDepth.ToString : intLineNumberToSkip = 3
    '                Case cGRM.OutputTableName.RainfallWPGrid.ToString : intLineNumberToSkip = 3
    '                Case cGRM.OutputTableName.RainfallWPUpMean.ToString : intLineNumberToSkip = 3
    '                Case cGRM.OutputTableName.FCDataWP.ToString : intLineNumberToSkip = 7
    '                Case cGRM.OutputTableName.FCResStorage.ToString : intLineNumberToSkip = 12
    '                Case "WPTS" : intLineNumberToSkip = 3
    '                Case Else
    '            End Select
    '            While Not TxtReader.EndOfData
    '                intNRowTxt += 1
    '                currentRow = TxtReader.ReadFields()
    '                If intNRowTxt > intLineNumberToSkip Then
    '                    Dim newRow As DataRow = dtInput.NewRow
    '                    newRow(0) = intNRowTxt - intLineNumberToSkip
    '                    intColindex = 1
    '                    For Each currentField As String In currentRow
    '                        newRow(intColindex) = currentField
    '                        intColindex += 1
    '                    Next
    '                    dtInput.Rows.Add(newRow)
    '                End If
    '            End While
    '            DBAdapter.Update(dtInput) ', strInputTableName)

    '            TxtReader.Close()
    '            TxtReader.Dispose()
    '        End If
    '    Catch ex As Exception
    '        MsgBox(ex.ToString)
    '    End Try
    'End Sub


    'Private Shared Sub AddSimulatedRowToTSSummaryTable(ByVal project As cProject, ByVal strDataType As String, ByVal strDataSource As String, ByVal strUnit As String)

    '    Try
    '        For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
    '            Dim cvid As Integer = row.CVID
    '            Dim strTimeToSave As String
    '            If project.Rainfall.mIsDateTimeFormat = True Then
    '                strTimeToSave = CStr(project.Rainfall.mRFStartDateTime) '.TIME_START
    '            Else
    '                strTimeToSave = "0"
    '            End If

    '            Dim tsSum As Double
    '            Dim max As Double
    '            Dim maxTime As String = ""
    '            Select Case strDataType
    '                Case "Discharge"
    '                    tsSum = project.WatchPoint.mTotalFlow_cms(cvid)
    '                    max = project.WatchPoint.mMaxFlow_cms(cvid)
    '                    maxTime = project.WatchPoint.mMaxFlowTime(cvid)
    '                Case "Depth"
    '                    tsSum = project.WatchPoint.mTotalDepth_m(cvid)
    '                    max = project.WatchPoint.mMaxDepth_m(cvid)
    '                    maxTime = project.WatchPoint.mMaxDepthTime(cvid)
    '            End Select

    '            Dim newRow As GRMProject.TimeSeriesDataSummaryRow _
    '                            = project.TSSummary.mdtTSSummary.NewTimeSeriesDataSummaryRow
    '            With newRow
    '                .CVID = row.CVID
    '                .WATCHPOINT = CStr(row.ColX & "_" & row.RowY)
    '                .WPNAME = row.Name
    '                .TS_TYPE = strDataType
    '                .UNIT = strUnit
    '                .TS_SOURCE = strDataSource
    '                .TIME_START = strTimeToSave
    '                .TIME_END = project.GeneralSimulEnv.EndingTimeToPrint
    '                .TS_COUNT = cThisSimulation.mPrintOutResultRowCount
    '                .MISSING_COUNT = 0
    '                .TS_SUM = tsSum
    '                .MAXIMUM = max
    '                .MAX_TIME = maxTime
    '                If strDataSource = "Simulated" Then
    '                    .RFGrid_SUM_mm = project.WatchPoint.mRFWPGridTotal_mm(cvid)
    '                    .RFMean_SUM_mm = project.WatchPoint.mRFUpWsMeanTotal_mm(cvid)
    '                Else
    '                    .SetRFGrid_SUM_mmNull()
    '                    .SetRFMean_SUM_mmNull()
    '                End If
    '            End With
    '            project.TSSummary.mdtTSSummary.Rows.Add(newRow)
    '        Next
    '    Catch ex As Exception
    '        MsgBox(ex.ToString)
    '    End Try
    'End Sub


    'Private Shared Sub DropAndCreateWPTSTable(ByVal project As cProject)
    '    Try
    '        Dim myTables As New Data.DataTable
    '        Dim restricstions() As String = New String(3) {}
    '        restricstions(3) = "table"
    '        myTables = project.OdbCnnDynamic.GetOleDbSchemaTable(OleDb.OleDbSchemaGuid.Tables, restricstions)

    '        Dim strQuery As String
    '        Dim odbCMD As New OleDb.OleDbCommand
    '        Dim strNowTableName As String
    '        Dim strNowTableNameHeader As String
    '        odbCMD = project.OdbCnnDynamic.CreateCommand

    '        '여기서 테이블 지우고
    '        For intN As Integer = 0 To myTables.Rows.Count - 1
    '            strNowTableName = CStr(myTables.Rows(intN).Item(2))
    '            strNowTableNameHeader = Mid(strNowTableName, 1, 4)
    '            If strNowTableNameHeader = "WPTS" Then
    '                strQuery = "delete * from " & strNowTableName
    '                odbCMD.CommandText = strQuery
    '                odbCMD.ExecuteNonQuery()

    '                strQuery = "drop table " & strNowTableName
    '                odbCMD.CommandText = strQuery
    '                odbCMD.ExecuteNonQuery()
    '            End If
    '        Next intN

    '        Dim strTimeFiledName As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME
    '        For Each row As GRMDynamicDB.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo
    '            Dim strWPName As String = Replace(row.Name, ",", "_")
    '            Dim strTargetTableName As String = "WPTS_" & strWPName
    '            Dim strCreateQuery As String = _
    '                             "create table " & strTargetTableName + _
    '                             "( " + _
    '                             "DataNo long, " + _
    '                             strTimeFiledName + " string, " + _
    '                             "Discharge single, " + _
    '                             "Depth single, " + _
    '                             "BaseFlowDepth single, " + _
    '                             "CumulativeInfiltrion single, " + _
    '                             "SSR single, " + _
    '                             "RFGrid single, RFUpMean single, " + _
    '                             "FCData single, " + _
    '                             "FCResStor single" + _
    '                             " );"
    '            odbCMD.CommandText = strCreateQuery
    '            odbCMD.ExecuteNonQuery()
    '        Next
    '    Catch ex As Exception
    '        MsgBox(ex.ToString, , cGRM.BuildInfo.ProductName)
    '    End Try
    'End Sub

    'Private Sub SaveToProjectExcelFile_Discharge(ByVal strOFNPDischarge As String, ByVal strFPNExel As String, ByVal intWPCount As Integer)

    '    Dim intCountL As Integer
    '    Dim intCountCol As Integer = intWPCount + 4

    '    Dim strLines() As String = System.IO.File.ReadAllLines(strOFNPDischarge)

    '    intCountL = strLines.Length

    '    Dim saRet(intCountL - 1, intCountCol - 1) As Object
    '    Dim TxtReader As New FileIO.TextFieldParser(strOFNPDischarge)
    '    Dim intNRowTxt As Integer = 0

    '    TxtReader.TextFieldType = FileIO.FieldType.Delimited '필드가 구분됨
    '    TxtReader.TrimWhiteSpace = False
    '    TxtReader.SetDelimiters(vbTab)

    '    Dim currentRow As String()

    '    While Not TxtReader.EndOfData

    '        If intNRowTxt = 10 Or intNRowTxt = 17 Or intNRowTxt = 22 Or intNRowTxt = 29 Or intNRowTxt = 31 Then
    '            saRet(intNRowTxt, 0) = ""
    '        Else
    '            currentRow = TxtReader.ReadFields()
    '            Dim currentField As String
    '            Dim intColindex As Integer
    '            intColindex = 0
    '            For Each currentField In currentRow
    '                saRet(intNRowTxt, intColindex) = CStr(currentField)
    '                intColindex += 1
    '            Next

    '        End If
    '        intNRowTxt += 1
    '    End While

    '    Dim objBooks As Excel.Workbooks
    '    Dim objSheets As Excel.Sheets
    '    Dim objSheet As Excel._Worksheet
    '    Dim range As Excel.Range

    '    ' Create a new instance of Excel and start a new workbook.
    '    mExcelApp = New Excel.Application()
    '    mExcelBook = mExcelApp.Workbooks.Open(Filename:=strFPNExel, UpdateLinks:=False, ReadOnly:=False)
    '    objSheets = mExcelBook.Worksheets
    '    objSheet = CType(objSheets(2), Excel._Worksheet)

    '    'Get the range where the starting cell has the address
    '    'm_sStartingCell and its dimensions are m_iNumRows x m_iNumCols.
    '    objSheet.Range("a1", "z5000").Clear()
    '    range = objSheet.Range("A1", Reflection.Missing.Value)
    '    range = range.Resize(intCountL, intCountCol)

    '    'Set the range value to the array.
    '    range.Value = saRet

    '    'Return control of Excel to the user.
    '    mExcelApp.Visible = True
    '    mExcelApp.UserControl = True

    '    'Clean up a little.
    '    range = Nothing
    '    objSheet = Nothing
    '    objSheets = Nothing
    '    objBooks = Nothing
    'End Sub
#End Region
End Class

