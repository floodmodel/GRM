Imports System.Threading
Imports System.Threading.Tasks
Imports System.Collections.Concurrent
Imports System.Linq
Imports System.Xml.Linq
Imports System.Text

Public Class cSimulator

    Public Enum SimulationErrors
        OutputFileCreateError
        ReadRainfallDataError
        NoWatchPointError
    End Enum

#Region "EVENTS"
    Public Event SimulationStep(ByVal sender As cSimulator, ByVal elapsedMinutes As Integer)
    Public Event SimulationStop(ByVal sender As cSimulator)
    Public Event SimulationComplete(ByVal sender As cSimulator)
    Public Event SimulationRaiseError(ByVal sender As cSimulator, ByVal simulError As SimulationErrors, ByVal erroData As Object)
    Public Event SimulationMultiEventStep(ByVal sender As cSimulator, ByVal eventOrder As Integer)
    'Public Event MakeRasterOutput(ByVal sender As cSimulator, ByVal project As cProject, ByVal nowTtoPrint_MIN As Integer)
    Public Event MakeRasterOutput(ByVal sender As cSimulator, ByVal nowTtoPrint_MIN As Integer)
    'Public Event SendQToAnalyzer(ByVal sender As cSimulator, ByVal project As cProject, project_tm1 As cProjectBAK, ByVal nowTtoPrint_MIN As Integer, interCoef As Single)
    Public Event SendQToAnalyzer(ByVal sender As cSimulator, project_tm1 As cProjectBAK, ByVal nowTtoPrint_MIN As Integer, interCoef As Single)

#End Region

#Region "Exceptions"
    Public Class SimulatorCreateOutputFilesException
        Inherits Exception
    End Class

#End Region

    Private mStop As Boolean

    ''' <summary>
    ''' 강우입력자료가 끝났는지 여부
    ''' </summary>
    ''' <remarks></remarks>
    Private mbRFisEnded As Boolean

    Private mHydroCom As New cHydroCom
    Private mFVMSolver As New cFVMSolver
    Private mInfiltration As New cInfiltration
    Private mFC As New cFlowControl
    Private mSSnBS As New cSSandBSflow

    Private WithEvents mOutputControl As New cOutPutControl()
    Private mOutputControlRT As cOutputControlRT

    Private mProject As cProject
    Private mRealTime As cRealTime

    Public ReadOnly Property OutputControlRT As cOutputControlRT
        Get
            Return mOutputControlRT
        End Get
    End Property


    Public Sub Initialize()
        mbRFisEnded = False
        cThisSimulation.mTimeThisSimulationStarted = g_dtStart_from_MonitorEXE
        cThisSimulation.mRFMeanForDT_m = 0
        cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = 0

        If cThisSimulation.IsParallel = True Then
            cGRM.writelogAndConsole(String.Format("IsParallel : {0}, Max degree of parallelism : {1}",
                                              cThisSimulation.IsParallel.ToString, cThisSimulation.MaxDegreeOfParallelism), cGRM.bwriteLog, True)

        Else
            cGRM.writelogAndConsole(String.Format("IsParallel : {0}", cThisSimulation.IsParallel.ToString), cGRM.bwriteLog, True)
        End If
    End Sub

    ''2017.04.24. GUI와 별개 exe로 실행되기 때문에 쓰레드 생성 실행 아래의 부분은 필요 없음.
    'Public Sub SimulateSingleEventWithNewThread(ByVal project As cProject)
    '    mProject = project
    '    Dim ts As New ThreadStart(AddressOf SimulateSingleEventInner)
    '    Dim th As New Thread(ts)
    '    th.Start()
    'End Sub

    'Private Sub SimulateSingleEventInner()
    Public Sub SimulateSingleEvent(ByVal project As cProject)
        mProject = project
        mStop = False
        Dim eRainfallDataType As cRainfall.RainfallDataType = CType(mProject.Rainfall.mRainfallDataType, cRainfall.RainfallDataType)
        Dim dtRFinfo As List(Of cRainfall.RainfallData) = mProject.Rainfall.mlstRainfallData
        Dim endingTimeSEC As Integer = mProject.GeneralSimulEnv.EndingTimeSec
        Dim dTPrint_MIN As Integer = CInt(mProject.GeneralSimulEnv.mPrintOutTimeStepMIN)
        Dim dTPrint_SEC As Integer = dTPrint_MIN * 60
        Dim dtmin As Single = CSng(cThisSimulation.dtsec / 60)
        Dim dTRFintervalSEC As Integer = mProject.Rainfall.RFIntervalSEC
        Dim dTRFintervalMIN As Integer = CInt(mProject.Rainfall.mRainfallinterval)
        Dim wpCount As Integer = mProject.WatchPoint.WPCount
        g_dtStart_from_MonitorEXE = DateTime.Now
        Initialize()
        SetCVStartingCondition(mProject, wpCount)
        If Not cThisSimulation.mGRMSetupIsNormal Then Exit Sub
        Dim mSEC_tm1 As Integer
        Dim Project_tm1 As cProjectBAK = Nothing
        Dim targetToPrintMIN As Integer = 0
        Dim bBAKdata As Boolean = False
        Dim targetCalTtoPrint_MIN As Integer = 0
        Dim nowRFOrder As Integer = 0
        Dim nowTsec As Integer = cThisSimulation.dtsec
        cThisSimulation.dtsec_usedtoForwardToThisTime = cThisSimulation.dtsec
        Dim dtsec As Integer
        'CVid의 값은 1부터 시작함. 
        Dim simulationTimeLimitSEC As Integer = endingTimeSEC + dtsec
        Do While nowTsec <= simulationTimeLimitSEC
            dtsec = cThisSimulation.dtsec
            cThisSimulation.vMaxInThisStep = Single.MinValue
            'dtsec부터 시작해서, 첫번째 강우레이어를 이용한 모의결과를 0시간에 출력한다.
            If Not mbRFisEnded AndAlso (nowRFOrder = 0 OrElse (nowTsec > dTRFintervalSEC * nowRFOrder)) Then
                If nowRFOrder < cThisSimulation.mRFDataCountInThisEvent Then '현재까지 적용된 레이어가 전체 개수보다 작으면,,
                    nowRFOrder = nowRFOrder + 1 '이렇게 하면 마지막 레이어 적용
                    cRainfall.ReadRainfall(project, eRainfallDataType, dtRFinfo, dTRFintervalMIN, nowRFOrder, cThisSimulation.IsParallel)
                    mbRFisEnded = False
                Else
                    cRainfall.SetRainfallintensity_mPsec_And_Rainfall_dt_meter_Zero(mProject)
                    nowRFOrder = Integer.MaxValue
                    cThisSimulation.mRFMeanForDT_m = 0
                    mbRFisEnded = True
                End If
            End If
            Dim nowT_MIN As Integer = CInt(Int(nowTsec / 60))
            SimulateRunoff(mProject, nowT_MIN)
            cRainfall.CalCumulativeRFDuringDTPrintOut(mProject, dtsec)
            WriteCurrentResultAndInitializeNextStep(mProject, nowTsec, dtsec, dTRFintervalSEC, dTPrint_MIN,
                                       wpCount, targetCalTtoPrint_MIN,
                                        mSEC_tm1,
                                         Project_tm1, mProject.mSimulationType, nowRFOrder)
            nowTsec = nowTsec + dtsec 'dtsec 만큼 전진
            cThisSimulation.dtsec_usedtoForwardToThisTime = dtsec
            If cThisSimulation.IsFixedTimeStep = False Then
                cThisSimulation.dtsec = cHydroCom.getDTsec(cGRM.CONST_CFL_NUMBER,
                                                           project.Watershed.mCellSize, cThisSimulation.vMaxInThisStep, dTPrint_MIN)
            End If
            If mStop Then Exit Do
        Loop

        If mStop Then
            RaiseEvent SimulationStop(Me)
        Else
            cGRM.writelogAndConsole("Simulation was completed.", cGRM.bwriteLog, False)
            RaiseEvent SimulationComplete(Me)
            MyBase.Finalize()
        End If
    End Sub

    Public Sub SimulateRT(ByVal project As cProject, ByVal realTime As cRealTime)
        mProject = project
        mRealTime = realTime
        Dim ts As New ThreadStart(AddressOf SimulateRTInner)
        Dim th As New Thread(ts)
        th.Start()
        'SimulateRTInner()
    End Sub

    Public Sub SimulateRTInner()
        Console.WriteLine("..debug.. Sub SimulateRTInner()")
        mStop = False
        mOutputControlRT = New cOutputControlRT()
        Dim bRainfallisEnded As Boolean
        Dim nowT_MIN As Integer
        Dim targetCalTtoPrint_MIN As Integer = 0
        Dim project_tm1 As cProjectBAK = Nothing
        Dim bBAKdata As Boolean = False
        Dim dTPrint_MIN As Integer = mRealTime.mDtPrintOutRT_min
        Dim dtPrint_SEC As Integer = dTPrint_MIN * 60
        Dim EndingT_SEC As Integer = mRealTime.EndingTime_SEC
        Dim EndingT_HOUR As Integer = CInt(EndingT_SEC / 3600)
        Dim dTRFintervalSEC As Integer = CInt(mProject.Rainfall.RFIntervalSEC)
        Dim sec_tm1 As Integer
        Initialize()
        SetCVStartingCondition(mProject, mProject.WatchPoint.WPCount)
        Dim nowRFLayerOrder As Integer = 0
        Dim nowTsec As Integer = cThisSimulation.dtsec
        cThisSimulation.dtsec_usedtoForwardToThisTime = cThisSimulation.dtsec
        Dim dtsec As Integer
        Dim simulationTimeLimitSEC As Integer = EndingT_SEC + dtsec + 1
        Do While nowTsec < simulationTimeLimitSEC
            dtsec = cThisSimulation.dtsec
            nowT_MIN = CInt(nowTsec / 60)
            cThisSimulation.vMaxInThisStep = Single.MinValue
            If nowRFLayerOrder = 0 OrElse nowTsec > (dTRFintervalSEC * cThisSimulation.mRFDataCountInThisEvent) Then
                '신규 강우자료 검색
                Dim TargetRFLayerTime As String
                TargetRFLayerTime = Format(mRealTime.mDateTimeStartRT.Add _
                                              (New System.TimeSpan(0, CInt(nowRFLayerOrder * dTRFintervalSEC / 60), 0)),
                                              "yyyyMMddHHmm")
                Do
                    If mStop = True Then Exit Sub
                    Call mRealTime.UpdateRainfallInformationGRMRT(TargetRFLayerTime)
                    If nowRFLayerOrder < cThisSimulation.mRFDataCountInThisEvent Then Exit Do
                    Thread.Sleep(2000)  '2초 지연 적절함
                Loop
                nowRFLayerOrder = nowRFLayerOrder + 1 '이렇게 하면 마지막 레이어 적용
                cRainfall.ReadRainfall(mProject, mRealTime.mRainfallDataTypeRT, mRealTime.mlstRFdataRT,
                                                  CInt(mProject.Rainfall.mRainfallinterval), nowRFLayerOrder, cThisSimulation.IsParallel)
                bRainfallisEnded = False
            End If
            If mProject.FCGrid.IsSet = True Then
                '신규 fc 자료 검색 조건
                Dim ReadDBorCSVandMakeFCdataTableForRealTime_TargetDataTime_Previous As String = ""
                For nfc As Integer = 0 To mProject.FCGrid.FCCellCount - 1
                    Dim r As GRMProject.FlowControlGridRow = CType(mProject.FCGrid.mdtFCGridInfo.Rows(nfc),
                          GRMProject.FlowControlGridRow)
                    Dim cvid As Integer = r.CVID
                    If r.ControlType.ToString <> cFlowControl.FlowControlType.ReservoirOperation.ToString Then
                        Dim dt_MIN As Integer = CInt(r.DT)
                        If r.ControlType.ToString <> cFlowControl.FlowControlType.ReservoirOperation.ToString Then
                            If nowTsec > dt_MIN * 60 * mRealTime.mdicFCDataOrder(cvid) OrElse mRealTime.mdicFCDataOrder(cvid) = 0 Then
                                Dim TargetDataTime As String
                                TargetDataTime = Format(mRealTime.mDateTimeStartRT.Add _
                                 (New System.TimeSpan(0, CInt(mRealTime.mdicFCDataOrder(cvid) * dt_MIN), 0)),
                                 "yyyyMMddHHmm")
                                Dim bAfterSleep As Boolean = False
                                Do
                                    If ReadDBorCSVandMakeFCdataTableForRealTime_TargetDataTime_Previous <> TargetDataTime Or bAfterSleep Then
                                        mRealTime.ReadDBorCSVandMakeFCdataTableForRealTime(TargetDataTime)
                                    End If
                                    ReadDBorCSVandMakeFCdataTableForRealTime_TargetDataTime_Previous = TargetDataTime
                                    If mStop = True Then Exit Sub
                                    Call mRealTime.UpdateFcDatainfoGRMRT(TargetDataTime, cvid, mRealTime.mdicFCDataOrder(cvid), dt_MIN)
                                    If mRealTime.mdicBNewFCdataAddedRT(cvid) = True Then Exit Do
                                    Thread.Sleep(2000)
                                    bAfterSleep = True
                                Loop
                                mRealTime.mdicFCDataOrder(cvid) += 1
                            End If
                        End If
                    End If
                Next
            End If
            'cThisSimulation.vMaxInThisStep = Single.MinValue
            SimulateRunoff(mProject, nowT_MIN)
            cRainfall.CalCumulativeRFDuringDTPrintOut(mProject, dtsec)
            Dim wpCount As Integer = mProject.WatchPoint.WPCount
            Call WriteCurrentResultAndInitializeNextStep(mProject, nowTsec, dtsec, dTRFintervalSEC, dTPrint_MIN,
                          wpCount,
                          targetCalTtoPrint_MIN,
                         sec_tm1,
                         project_tm1, mProject.mSimulationType, nowRFLayerOrder)
            nowTsec = nowTsec + dtsec
            cThisSimulation.dtsec_usedtoForwardToThisTime = dtsec
            If cThisSimulation.IsFixedTimeStep = False Then
                cThisSimulation.dtsec = cHydroCom.getDTsec(cGRM.CONST_CFL_NUMBER,
                                                           mProject.Watershed.mCellSize, cThisSimulation.vMaxInThisStep, dTPrint_MIN)
            End If
            If mStop = True Then Exit Do
        Loop
        If mStop = True Then
            RaiseEvent SimulationStop(Me)
        Else
            RaiseEvent SimulationComplete(Me)
        End If
    End Sub


    Public Sub SimulateRunoff(ByVal project As cProject, ByVal nowT_MIN As Integer)
        Dim cellsize As Integer = project.Watershed.mCellSize
        Dim dtsec As Integer = cThisSimulation.dtsec
        If cThisSimulation.IsParallel = True Then
            For fac As Integer = 0 To project.Watershed.mFacMax
                If project.mCVANsForEachFA(fac) IsNot Nothing Then
                    Dim nfac As Integer = fac
                    Dim ncv As Integer = Nothing
                    Dim cvans(project.mCVANsForEachFA(nfac).Length - 1) As Integer
                    project.mCVANsForEachFA(nfac).CopyTo(cvans, 0)
                    Dim options As ParallelOptions = New ParallelOptions()
                    options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                    Parallel.ForEach(Partitioner.Create(ncv, cvans.Length), options,
                                     Sub(range)
                                         For i As Integer = range.Item1 To range.Item2 - 1
                                             Dim cvan As Integer = cvans(i)
                                             If project.CV(cvan).toBeSimulated = True Then
                                                 simulateRunoffCore(project, nfac, cvan, dtsec, nowT_MIN, cellsize)
                                             End If
                                         Next
                                     End Sub)
                End If
            Next fac
        Else
            For fac As Integer = 0 To project.Watershed.mFacMax
                If project.mCVANsForEachFA(fac) IsNot Nothing Then
                    For Each cvan As Integer In project.mCVANsForEachFA(fac)
                        If project.CV(cvan).toBeSimulated = True Then
                            simulateRunoffCore(project, fac, cvan, dtsec, nowT_MIN, cellsize)
                            'Dim rf_mPs As Single = project.CV(cvan).RFReadintensity_mPsec
                        End If
                    Next
                End If
            Next fac
        End If
        For Each icv As cCVAttribute In project.CVs
            If icv.FlowType = cGRM.CellFlowType.OverlandFlow Then
                If cThisSimulation.vMaxInThisStep < icv.uCVof_i_j Then
                    cThisSimulation.vMaxInThisStep = icv.uCVof_i_j
                End If
            Else
                If cThisSimulation.vMaxInThisStep < icv.mStreamAttr.uCVch_i_j Then
                    cThisSimulation.vMaxInThisStep = icv.mStreamAttr.uCVch_i_j
                End If
            End If
        Next
    End Sub


    Private Sub simulateRunoffCore(project As cProject, fac As Integer, cvan As Integer, dtsec As Integer, nowT_min As Integer, cellsize As Single)
        cRainfall.CalRF_mPdt(project.CV(cvan), dtsec, cellsize)
        If project.GeneralSimulEnv.mbSimulateFlowControl = True AndAlso
                (project.CV(cvan).FCType = cFlowControl.FlowControlType.ReservoirOutflow _
               OrElse project.CV(cvan).FCType = cFlowControl.FlowControlType.Inlet) Then
            mFC.CalFCReservoirOutFlow(project, nowT_min, cvan)
        Else
            InitializeCVForThisStep(project, cvan)
            Select Case project.CV(cvan).FlowType
                Case cGRM.CellFlowType.OverlandFlow
                    Dim hCVw_i_jP1 As Single = 0
                    If fac > 0 Then
                        hCVw_i_jP1 = mFVMSolver.CalculateOverlandWaterDepthCViW(project, cvan)
                    End If
                    With project
                        If hCVw_i_jP1 > 0 OrElse .CV(cvan).hCVof_i_j > 0 Then
                            mFVMSolver.CalculateOverlandFlow(project, cvan, hCVw_i_jP1, .Watershed.mCellSize)
                        Else
                            mFVMSolver.SetNoFluxOverlandFlowCV(.CV(cvan))
                        End If
                    End With
                Case cGRM.CellFlowType.ChannelFlow, cGRM.CellFlowType.ChannelNOverlandFlow
                    Dim CSAchCVw_i_jP1 As Single = 0
                    If fac > 0 Then
                        CSAchCVw_i_jP1 = mFVMSolver.CalChCSA_CViW(project, cvan)
                    End If
                    With project
                        If CSAchCVw_i_jP1 > 0 OrElse .CV(cvan).mStreamAttr.hCVch_i_j > 0 Then
                            mFVMSolver.CalculateChannelFlow(project, cvan, CSAchCVw_i_jP1)
                        Else
                            mFVMSolver.SetNoFluxChannelFlowCV(.CV(cvan))
                        End If
                    End With
            End Select
        End If
        If project.GeneralSimulEnv.mbSimulateFlowControl = True AndAlso
                   (project.CV(cvan).FCType = cFlowControl.FlowControlType.SinkFlow _
                   OrElse project.CV(cvan).FCType = cFlowControl.FlowControlType.SourceFlow _
                   OrElse project.CV(cvan).FCType = cFlowControl.FlowControlType.ReservoirOperation) Then
            Dim rows As GRMProject.FlowControlGridRow() =
                    CType(project.FCGrid.mdtFCGridInfo.Select("CVID = " & (cvan + 1)), GRMProject.FlowControlGridRow())
            Dim row As GRMProject.FlowControlGridRow = rows(0)
            If CSng(row.MaxStorage) * CSng(row.MaxStorageR) = 0 Then '
                mFC.CalFCSinkOrSourceFlow(project, nowT_min, cvan)
            Else
                mFC.CalFCReservoirOperation(project, cvan, nowT_min)
            End If
        End If

    End Sub

    ''' <summary>
    ''' 강우, 지표하 유출, 기저유출에 의한 생성항 계산 및 t시간에서의 모의를 위한 초기값을 설정한다.
    ''' 또한 [Channel and overland flow] 속성 셀에서의 overland flow 부분을 계산해서 하도로 측방 유입시킨다.
    ''' </summary>
    ''' <param name="project"></param>
    ''' <param name="cvan"></param>
    ''' <remarks></remarks>
    Private Sub InitializeCVForThisStep(ByVal project As cProject, ByVal cvan As Integer)
        Dim rfInterval_SEC As Integer = project.Rainfall.RFIntervalSEC
        Dim dY_m As Single = project.Watershed.mCellSize
        Dim effOFdYinCHnOFcell As Single = 0
        Dim dTSEC As Integer = cThisSimulation.dtsec
        Dim CVdx_m As Single = project.CV(cvan).CVDeltaX_m
        Dim chCSAAddedByBFlow_m2 As Single = 0
        Dim ofDepthAddedByRFlow_m2 As Single = 0
        Dim chCSAAddedBySSFlow_m2 As Single = 0
        If project.CV(cvan).FlowType = cGRM.CellFlowType.ChannelNOverlandFlow Then
            effOFdYinCHnOFcell = dY_m - project.CV(cvan).mStreamAttr.ChBaseWidth
        End If
        If project.GeneralSimulEnv.mbSimulateBFlow = True Then
            chCSAAddedByBFlow_m2 = mSSnBS.CalBFlowAndGetCSAAddedByBFlow(project, cvan, dTSEC, dY_m)
        End If
        If project.GeneralSimulEnv.mbSimulateSSFlow = True Then
            With project.CV(cvan)
                Select Case .FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        ofDepthAddedByRFlow_m2 = mSSnBS.GetRFlowOFAreaAddedBySSflowCVwAndCalSSflowAtNowCV(project, cvan, dTSEC, dY_m, .CVDeltaX_m)
                    Case cGRM.CellFlowType.ChannelFlow
                        chCSAAddedBySSFlow_m2 = mSSnBS.GetChCSAaddedBySSFlowInChlCell(project, cvan)
                    Case cGRM.CellFlowType.ChannelNOverlandFlow
                        ofDepthAddedByRFlow_m2 = mSSnBS.GetRFlowOFAreaAddedBySSflowCVwAndCalSSflowAtNowCV(project, cvan, dTSEC, effOFdYinCHnOFcell, .CVDeltaX_m)
                        chCSAAddedBySSFlow_m2 = mSSnBS.GetChCSAaddedBySSFlowInChNOfCell(project, cvan)
                End Select
            End With
        End If
        Call mInfiltration.CalEffectiveRainfall(project, cvan, rfInterval_SEC, dTSEC)
        With project.CV(cvan)
            Select Case .FlowType
                Case cGRM.CellFlowType.OverlandFlow
                    .hCVof_i_j_ori = .hCVof_i_j + .EffRFCV_dt_meter + ofDepthAddedByRFlow_m2
                    .hCVof_i_j = .hCVof_i_j_ori
                    .CSAof_i_j = .hCVof_i_j_ori * dY_m
                    .StorageAddedForDTfromRF_m3 = .EffRFCV_dt_meter * dY_m * CVdx_m
                Case cGRM.CellFlowType.ChannelFlow
                    Dim ChWidth As Single = .mStreamAttr.ChBaseWidth
                    .mStreamAttr.hCVch_i_j_ori = .mStreamAttr.hCVch_i_j + .RFApp_dt_meter _
                                                 + chCSAAddedBySSFlow_m2 / ChWidth _
                                                 + chCSAAddedByBFlow_m2 / ChWidth
                    .mStreamAttr.CSAch_i_j_ori = mFVMSolver.GetChannelCrossSectionAreaUsingChannelFlowDepth(ChWidth,
                                                         .mStreamAttr.mChBankCoeff, .mStreamAttr.hCVch_i_j_ori,
                                                         .mStreamAttr.chIsCompoundCS, .mStreamAttr.chLowerRHeight,
                                                         .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chUpperRBaseWidth_m)
                    .mStreamAttr.hCVch_i_j = .mStreamAttr.hCVch_i_j_ori
                    .mStreamAttr.CSAch_i_j = .mStreamAttr.CSAch_i_j_ori
                    .StorageAddedForDTfromRF_m3 = .RFApp_dt_meter * dY_m * CVdx_m
                Case cGRM.CellFlowType.ChannelNOverlandFlow
                    Dim chCSAAddedByOFInChCell_m2 As Single
                    Dim ChWidth As Single = .mStreamAttr.ChBaseWidth
                    .hCVof_i_j_ori = .hCVof_i_j + .EffRFCV_dt_meter + ofDepthAddedByRFlow_m2
                    .hCVof_i_j = .hCVof_i_j_ori
                    .CSAof_i_j = .hCVof_i_j_ori * effOFdYinCHnOFcell
                    If .hCVof_i_j > 0 Then
                        Call mFVMSolver.CalculateOverlandFlow(project, cvan, 0, effOFdYinCHnOFcell)
                    Else
                        .hCVof_i_j = 0
                        .uCVof_i_j = 0
                        .CSAof_i_j = 0
                        .QCVof_i_j_m3Ps = 0
                    End If
                    If .QCVof_i_j_m3Ps > 0 Then
                        'chCSAAddedByOFInChCell_m2 = mFVMSolver.CalChCSAFromQbyIteration(project.CV(cvan), .CSAof_i_j, .QCVof_i_j_m3Ps)
                        chCSAAddedByOFInChCell_m2 = .CSAof_i_j '이렇게 단순화 해도, 별 차이 없다. 계산시간은 좀 준다. 2018.03.12.최
                    Else
                        chCSAAddedByOFInChCell_m2 = 0
                    End If
                    .mStreamAttr.CSAchAddedByOFinCHnOFcell = chCSAAddedByOFInChCell_m2
                    .mStreamAttr.hCVch_i_j_ori = .mStreamAttr.hCVch_i_j + .RFApp_dt_meter +
                                                 chCSAAddedBySSFlow_m2 / ChWidth +
                                                 chCSAAddedByBFlow_m2 / ChWidth
                    If .mStreamAttr.hCVch_i_j_ori < 0 Then .mStreamAttr.hCVch_i_j_ori = 0
                    .mStreamAttr.CSAch_i_j_ori = mFVMSolver.GetChannelCrossSectionAreaUsingChannelFlowDepth(ChWidth,
                                                          .mStreamAttr.mChBankCoeff, .mStreamAttr.hCVch_i_j_ori,
                                                          .mStreamAttr.chIsCompoundCS, .mStreamAttr.chLowerRHeight,
                                                          .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chUpperRBaseWidth_m)
                    .mStreamAttr.hCVch_i_j = .mStreamAttr.hCVch_i_j_ori
                    .mStreamAttr.CSAch_i_j = .mStreamAttr.CSAch_i_j_ori
                    .StorageAddedForDTfromRF_m3 = .RFApp_dt_meter * ChWidth * CVdx_m + .EffRFCV_dt_meter * effOFdYinCHnOFcell * CVdx_m
            End Select
            .QsumCVw_dt_m3 = 0
        End With
    End Sub


    Public Sub SetCVStartingCondition(ByVal project As cProject, ByVal intWPCount As Integer,
                                      Optional ByVal iniflow As Single = 0)
        Dim hChCVini As Single
        Dim chCSAini As Single
        Dim qChCVini As Single
        Dim uChCVini As Single
        For cvan As Integer = 0 To project.CVCount - 1
            Dim iniQAtWP As Single = 0
            Dim faAtBaseWP As Integer = project.Watershed.mFacMax
            With project.CV(cvan)
                .uCVof_i_j = 0
                .hCVof_i_j = 0
                .hCVof_i_j_ori = 0
                .QCVof_i_j_m3Ps = 0
                .hUAQfromChannelBed_m = 0
                .CSAof_i_j = 0
                If .FlowType = cGRM.CellFlowType.ChannelFlow OrElse
                    .FlowType = cGRM.CellFlowType.ChannelNOverlandFlow Then
                    Dim bApplyIniStreamFlowIsSet As Boolean = False
                    If project.SubWSPar.userPars(.WSID).iniFlow > 0 Then
                        iniQAtWP = project.SubWSPar.userPars(.WSID).iniFlow
                        faAtBaseWP = project.CV(project.WSNetwork.WSoutletCVID(.WSID) - 1).FAc
                        bApplyIniStreamFlowIsSet = True
                    Else
                        Dim baseWSid As Integer = .WSID
                        For id As Integer = 0 To project.WSNetwork.WSIDsAllDowns(.WSID).Count - 1
                            Dim downWSid As Integer = project.WSNetwork.WSIDsNearbyDown(baseWSid)
                            If project.SubWSPar.userPars(downWSid).iniFlow > 0 Then
                                iniQAtWP = project.SubWSPar.userPars(downWSid).iniFlow
                                faAtBaseWP = project.CV(project.WSNetwork.WSoutletCVID(downWSid) - 1).FAc
                                bApplyIniStreamFlowIsSet = True
                                Exit For
                            Else
                                baseWSid = downWSid
                            End If
                        Next
                    End If
                    chCSAini = 0
                    hChCVini = 0
                    qChCVini = 0
                    uChCVini = 0
                    If bApplyIniStreamFlowIsSet = True Then
                        If project.mSimulationType = cGRM.SimulationType.SingleEventPE_SSR Then
                            qChCVini = iniflow * (.FAc - project.Watershed.mFacMostUpChannelCell) _
                                      / (faAtBaseWP - project.Watershed.mFacMostUpChannelCell)
                        Else
                            If project.Watershed.mFPN_initialChannelFlow = "" Then
                                qChCVini = iniQAtWP * (.FAc - project.Watershed.mFacMostUpChannelCell) _
                                          / (faAtBaseWP - project.Watershed.mFacMostUpChannelCell)

                            Else
                                qChCVini = .mStreamAttr.initialQCVch_i_j_m3Ps
                            End If
                        End If
                        If qChCVini > 0 Then
                            Dim sngCAS_ini As Single = qChCVini / .CVDeltaX_m '초기값 설정
                            chCSAini = mFVMSolver.CalChCSAFromQbyIteration(project.CV(cvan), sngCAS_ini, qChCVini)
                            hChCVini = mFVMSolver.GetChannelDepthUsingArea(.mStreamAttr.ChBaseWidth, chCSAini, .mStreamAttr.chIsCompoundCS,
                                                          .mStreamAttr.chUpperRBaseWidth_m, .mStreamAttr.chLowerRArea_m2,
                                                          .mStreamAttr.chLowerRHeight, .mStreamAttr.mChBankCoeff)
                        End If
                    End If
                    .mStreamAttr.hCVch_i_j = hChCVini
                    .mStreamAttr.CSAch_i_j = chCSAini
                    .mStreamAttr.hCVch_i_j_ori = hChCVini
                    .mStreamAttr.CSAch_i_j_ori = chCSAini
                    .mStreamAttr.QCVch_i_j_m3Ps = qChCVini
                    .mStreamAttr.uCVch_i_j = uChCVini
                    If project.GeneralSimulEnv.mbSimulateBFlow = True Then
                        .hUAQfromChannelBed_m = hChCVini '하도의 초기 수심을 비피압대수층의 초기 수심으로 설정 
                    Else
                        .hUAQfromChannelBed_m = 0
                    End If
                End If
                .RFReadintensity_tM1_mPsec = 0
                .EffRFCV_dt_meter = 0
                .RFApp_dt_meter = 0
                .RF_dtPrintOut_meter = 0
                .RFAcc_FromStartToNow_meter = 0
                .RFReadintensity_mPsec = 0
                .RFReadintensity_tM1_mPsec = 0
                .SoilMoistureChangeDeltaTheta = 0
                .InfiltrationF_mPdt = 0
                .InfiltrationRatef_mPsec = 0
                .InfiltrationRatef_tM1_mPsec = 0
                .EffectiveSaturationSe = 0
                .bAfterSaturated = False
                .StorageAddedForDTfromRF_m3 = 0
                .QsumCVw_dt_m3 = 0
                .effCVCountFlowINTOCViW = 0
                .SSF_Q_m3Ps = 0
                .baseflow_Q_m3Ps = 0
                .hUAQfromBedrock_m = cGRM.CONST_UAQ_HEIGHT_FROM_BEDROCK
                .StorageCumulative_m3 = 0
                If project.GeneralSimulEnv.mbSimulateFlowControl = True Then
                    If project.FCGrid.IsSet = True AndAlso project.FCGrid.FCGridCVidList.Contains(cvan + 1) Then
                        Dim rows As GRMProject.FlowControlGridRow() = CType(project.FCGrid.mdtFCGridInfo.Select("CVID = " & (cvan + 1)), GRMProject.FlowControlGridRow())
                        Dim row As GRMProject.FlowControlGridRow = rows(0)
                        If Not row.IsIniStorageNull AndAlso row.IniStorage <> "" Then
                            .StorageCumulative_m3 = CSng(row.IniStorage)
                        End If
                    End If
                End If
            End With
        Next cvan
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            With project.WatchPoint
                .mMaxDepth_m(wpcvid) = 0
                .mMaxDepthTime(wpcvid) = ""
                .mMaxFlow_cms(wpcvid) = 0
                .mMaxFlowTime(wpcvid) = ""
                .mRFWPGridTotal_mm(wpcvid) = 0
                .mRFUpWsMeanTotal_mm(wpcvid) = 0
                .mTotalDepth_m(wpcvid) = 0
                .mTotalFlow_cms(wpcvid) = 0
            End With
        Next
    End Sub


    Private Sub WriteCurrentResultAndInitializeNextStep(project As cProject, mNowTsec As Integer,
                              dtsec As Integer, dTRFintervalSEC As Integer, dTPrint_MIN As Integer,
                              wpCount As Integer, ByRef targetCalTtoPrint_MIN As Integer,
                             ByRef mSEC_tm1 As Integer,
                             ByRef Project_tm1 As cProjectBAK, SimType As cGRM.SimulationType, mNowRFLayerNumber As Integer)
        Dim dTRFintervalMIN As Integer = CInt(dTRFintervalSEC / 60)
        Dim dTPrint_SEC As Integer = dTPrint_MIN * 60
        Dim dtmin As Single = CSng(dtsec / 60)
        Dim timeToPrint_MIN As Integer
        If targetCalTtoPrint_MIN = 0 Then targetCalTtoPrint_MIN = dTPrint_MIN
        If mNowRFLayerNumber = 1 AndAlso dTPrint_MIN > dTRFintervalMIN AndAlso
            ((mNowTsec + dtsec) > dTRFintervalSEC) Then ' 첫번째 강우레이어 모델링 끝나는 경우
            '첫번째 출력전에 다음 스텝에서 강우레이어가 바뀌는 경우는 첫번째 강우레이어 모델링이 끝났다는 얘기이므로 한번 출력한다.
            '0 시간에서의 모델링 결과로 출력한다.
            timeToPrint_MIN = 0
            Dim RFmeanForFirstLayer As Single = CSng(cThisSimulation.mRFMeanForDT_m / dtmin * dTRFintervalMIN)
            OutputProcessManagerBySimType(timeToPrint_MIN, wpCount, RFmeanForFirstLayer,
                                                     1, Nothing, SimType)
        Else
            If mNowTsec > 0 AndAlso (mNowTsec Mod dTPrint_SEC) = 0 Then
                timeToPrint_MIN = targetCalTtoPrint_MIN - dTPrint_MIN
                OutputProcessManagerBySimType(timeToPrint_MIN, wpCount,
                                                         cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m,
                                                         1, Nothing, SimType)
                targetCalTtoPrint_MIN = targetCalTtoPrint_MIN + dTPrint_MIN
            Else
                If mNowTsec < targetCalTtoPrint_MIN * 60 AndAlso (mNowTsec + dtsec) > (targetCalTtoPrint_MIN) * 60 Then
                    If Project_tm1 Is Nothing Then
                        mSEC_tm1 = mNowTsec
                        Project_tm1 = New cProjectBAK
                        Project_tm1.SetCloneUsingCurrentProject(project)
                    End If
                End If
                If mNowTsec > (targetCalTtoPrint_MIN * 60) AndAlso
                    (mNowTsec - cThisSimulation.dtsec_usedtoForwardToThisTime) <= (targetCalTtoPrint_MIN * 60) Then
                    Dim coeffInterpolation As Single
                    coeffInterpolation = CSng((targetCalTtoPrint_MIN * 60 - mSEC_tm1) / (mNowTsec - mSEC_tm1))
                    timeToPrint_MIN = targetCalTtoPrint_MIN - dTPrint_MIN
                    OutputProcessManagerBySimType(timeToPrint_MIN, wpCount,
                                                             cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m,
                                                             coeffInterpolation, Project_tm1, SimType)
                    targetCalTtoPrint_MIN = targetCalTtoPrint_MIN + dTPrint_MIN
                    Project_tm1 = Nothing
                End If
            End If
        End If
    End Sub


    Private Sub OutputProcessManagerBySimType(ByVal nowTtoPrint_MIN As Integer,
                                  ByVal wpCount As Integer, ByVal SumRFMeanForDTprintOut_m As Double,
                                  ByVal coeffInterpolation As Single, ByVal Project_tm1 As cProjectBAK, simType As cGRM.SimulationType)
        Select Case simType
            Case cGRM.SimulationType.SingleEvent
                RaiseEvent SimulationStep(Me, nowTtoPrint_MIN)
                If mProject.GeneralSimulEnv.mPrintOption = cGRM.GRMPrintType.All Then
                    mOutputControl.WriteSimResultsToTextFileForSingleEvent(mProject, wpCount,
                                                                       nowTtoPrint_MIN, SumRFMeanForDTprintOut_m, coeffInterpolation, Project_tm1)
                End If
                If mProject.GeneralSimulEnv.mPrintOption = cGRM.GRMPrintType.DischargeFileQ Then
                    mOutputControl.WriteDischargeOnlyToDischargeFile(mProject, coeffInterpolation, Project_tm1)
                End If
                If mProject.GeneralSimulEnv.mPrintOption = cGRM.GRMPrintType.AllQ Then
                    mOutputControl.WriteDischargeOnlyToDischargeFile(mProject, coeffInterpolation, Project_tm1)
                    mOutputControl.WriteDischargeOnlyToWPFile(mProject, coeffInterpolation, Project_tm1)
                End If
                cGRM.writelogAndConsole(String.Format("Time(min) dt(sec), {0}{1}{2}", nowTtoPrint_MIN, vbTab, cThisSimulation.dtsec), cGRM.bwriteLog, False)
            Case cGRM.SimulationType.RealTime
                mOutputControlRT.WriteSimResultsToTextFileAndDBForRealTime(mProject,
                                                                           nowTtoPrint_MIN, coeffInterpolation, Project_tm1, mRealTime)
        End Select

        If mProject.GeneralSimulEnv.mbRunAanlyzer = True Then
            'RaiseEvent SendQToAnalyzer(Me, mProject, Project_tm1, nowTtoPrint_MIN, coeffInterpolation)
            RaiseEvent SendQToAnalyzer(Me, Project_tm1, nowTtoPrint_MIN, coeffInterpolation)
        End If
        If mProject.GeneralSimulEnv.mbMakeRasterOutput = True Then
            RaiseEvent MakeRasterOutput(Me, nowTtoPrint_MIN)
        End If

        cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = 0
        For Each row As GRMProject.WatchPointsRow In mProject.WatchPoint.mdtWatchPointInfo
            mProject.WatchPoint.mRFUpWsMeanForDtPrintout_mm(row.CVID) = 0
            mProject.WatchPoint.mRFWPGridForDtPrintout_mm(row.CVID) = 0
        Next
        If mProject.GeneralSimulEnv.mbShowRFdistribution = True Then
            With cProject.Current
                For cvan As Integer = 0 To cProject.Current.CVCount - 1
                    .CV(cvan).RF_dtPrintOut_meter = 0
                Next
            End With
        End If
    End Sub

    Public Sub StopSimulation()
        mStop = True
    End Sub

End Class


