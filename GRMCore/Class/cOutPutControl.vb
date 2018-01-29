Imports System.IO
Imports System.Diagnostics
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
        strNowTimeToPrintOut = cComTools.GetTimeToPrintOut(project.GeneralSimulEnv.mIsDateTimeFormat, project.GeneralSimulEnv.mSimStartDateTime, nowT_MIN)
        strFNPDischarge = project.OFNPDischarge
        strFNPDepth = project.OFNPDepth
        strFNPRFGrid = project.OFNPRFGrid
        strFNPRFMean = project.OFNPRFMean
        strFNPFCData = project.OFNPFCData
        strFNPFCStorage = project.OFNPFCStorage

        '===================================================================================================
        '유량
        lineToPrint = strNowTimeToPrintOut
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            cvan = wpcvid - 1
            If interCoef = 1 Then
                Select Case project.CV(cvan).FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        vToPrint = String.Format("{0,8:#0.##}", project.CV(cvan).QCVof_i_j_m3Ps)
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
            lineToPrint = lineToPrint + vbTab + vToPrint.Trim
            Dim sv As Single = CSng(vToPrint)
            With project.WatchPoint
                .mTotalFlow_cms(wpcvid) = .mTotalFlow_cms(wpcvid) + sv
                If .mMaxFlow_cms(wpcvid) < sv Then
                    .mMaxFlow_cms(wpcvid) = sv
                    .mMaxFlowTime(wpcvid) = strNowTimeToPrintOut
                End If
            End With
        Next
        lineToPrint = lineToPrint + vbTab + Format(sngMeanRainfallSumForPrintoutTime_mm, "#0.00") + vbTab + CStr(lngTimeDiffFromStarting_SEC) & vbCrLf
        IO.File.AppendAllText(strFNPDischarge, lineToPrint, Encoding.Default)

        ''삭제 대상
        'If CInt(strNowTimeToPrintOut) = 24 Then cThisSimulation.tmp_24H_RunTime = CStr(lngTimeDiffFromStarting_SEC)
        'If CInt(strNowTimeToPrintOut) = 48 Then cThisSimulation.tmp_48H_RunTime = CStr(lngTimeDiffFromStarting_SEC)

        ''이건 수심. 
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
        '격자별 강우 for watchpoint
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

        ''===================================================================
        ' watchpoint 상류 격자의 평균 강우량
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
        ' watchpoint별 모든 자료 출력
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            Dim cvanWP As Integer = wpcvid - 1
            Dim strL As String
            strL = strNowTimeToPrintOut + vbTab

            If interCoef = 1 Then
                If project.CV(cvanWP).FlowType = cGRM.CellFlowType.OverlandFlow Then
                    strL = strL + String.Format("{0,8:#0.##}", project.CV(cvanWP).QCVof_i_j_m3Ps) + vbTab
                Else
                    strL = strL + String.Format("{0,8:#0.##}", project.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps) + vbTab
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
                Else
                    strL = strL + String.Format("{0,8:#0.##}", cHydroCom.GetInterpolatedValueLinear(
                                   project_tm1.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps,
                                   project.CV(cvanWP).mStreamAttr.QCVch_i_j_m3Ps, interCoef)) + vbTab
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
        Next

        '===================================================================
        'FCAppFlow, FCStorage
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

                '해더
                Dim strOutPutLine As String
                Dim strOutputCommonHeader As String
                strOutputCommonHeader = String.Format("Project name : {0} {1} {2} {3} by {4}{5}",
                                                      project.ProjectNameWithExtension,
                                                      vbTab, Format(Now, "yyyy/MM/dd HH:mm"),
                                                      vbTab, cGRM.BuildInfo.ProductName, vbCrLf)
                Dim strLTime_WPName As String
                strLTime_WPName = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME

                For Each row As GRMProject.WatchPointsRow In project.WatchPoint.mdtWatchPointInfo.Rows
                    strLTime_WPName = strLTime_WPName + vbTab + "[" & row.Name & "]"
                    'wp 별 출력파일 설정
                    Dim wpName As String = Replace(row.Name, ",", "_")
                    Dim nFPN As String = Path.Combine(project.ProjectPath, project.ProjectNameOnly & "WP_" & wpName & ".out")
                    FPNs.Clear()
                    FPNs.Add(nFPN)
                    If cFile.ConfirmDeleteFiles(FPNs) = False Then
                        cThisSimulation.mGRMSetupIsNormal = False
                        Exit Function
                    End If
                    Dim strL As String = cGRM.CONST_OUTPUT_TABLE_TIME_FIELD_NAME + vbTab + "Discharge[cms]" + vbTab + "BaseFlowDepth[m]" + vbTab +
                                         "CumulativeInfiltrion[m]" + vbTab + "SSR" + vbTab + "RFGrid" + vbTab + "RFUpMean" + vbTab + "FCData" + vbTab + "FCResStor" + vbCrLf
                    IO.File.AppendAllText(nFPN, strOutputCommonHeader, Encoding.Default)
                    IO.File.AppendAllText(nFPN, "Output data : All the results for watch point '" + wpName + "'" + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(nFPN, strL, Encoding.Default)
                    project.WatchPoint.mFpnWpOut(row.CVID) = nFPN
                Next

                '----------------------------------------------------
                '이건 유량
                If Not IO.Directory.Exists(IO.Path.GetDirectoryName(strFNPDischarge)) Then IO.Directory.CreateDirectory(IO.Path.GetDirectoryName(strFNPDischarge))
                IO.File.AppendAllText(strFNPDischarge, strOutputCommonHeader, Encoding.Default)
                strOutPutLine = "Output data : " & "Discharge[CMS]" & vbCrLf & vbCrLf
                IO.File.AppendAllText(strFNPDischarge, strOutPutLine, Encoding.Default)
                strOutPutLine = strLTime_WPName + vbTab + "Rainfall_Mean" + vbTab + "FromStarting[sec]" & vbCrLf
                IO.File.AppendAllText(strFNPDischarge, strOutPutLine, Encoding.Default)

                ''----------------------------------------------------
                ''이건 수심
                'IO.File.AppendAllText(strFNPDepth, strOutputCommonHeader, Encoding.Default)
                'strOutPutLine = "Output data : " & "Depth[m]" & vbCrLf & vbCrLf
                'IO.File.AppendAllText(strFNPDepth, strOutPutLine, Encoding.Default)

                'strOutPutLine = strLTime_WPName + vbTab + "Rainfall_Mean" + vbTab + "ThisStep[msec]" + vbTab + "FromStarting[sec]" & vbCrLf
                'IO.File.AppendAllText(strFNPDepth, strOutPutLine, Encoding.Default)

                ''강우. 
                'IO.File.AppendAllText(strFNPRFGrid, strOutputCommonHeader, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFGrid, "Output data : Rainfall for each watchpoint[mm]" + vbCrLf & vbCrLf, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFGrid, strLTime_WPName & vbCrLf, Encoding.Default)

                'wp별 유역 평균강우량
                'IO.File.AppendAllText(strFNPRFMean, strOutputCommonHeader, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFMean, "Output data : Mean rainfall for upstream of each watchpoint[mm]" + vbCrLf & vbCrLf, Encoding.Default)
                'IO.File.AppendAllText(strFNPRFMean, strLTime_WPName & vbCrLf, Encoding.Default)

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

                    'FCApp - flow control data
                    IO.File.AppendAllText(strFNPFCData, strOutputCommonHeader, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, "Output data : Flow control data input[CMS]" + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strNameFCApp & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strTypeFCApp & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strSourceDT & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strResOperation + vbCrLf & vbCrLf, Encoding.Default)
                    IO.File.AppendAllText(strFNPFCData, strFCDataField & vbCrLf, Encoding.Default)

                    'reservoir operation
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
End Class

