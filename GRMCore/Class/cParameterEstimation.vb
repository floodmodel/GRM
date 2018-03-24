Imports System.Threading
Imports System.Math

Public Class cParameterEstimation

    Public Enum PEtype
        INI_SOIL_SATURATION_RATIO
        SOIL_DEPTH
    End Enum

#Region "Events"
    Public Event CallAnalyzer(ByVal sender As cParameterEstimation, ByVal project As cProject, ByVal nowTtoPrint_MIN As Integer, createImgFile As Boolean, createASCFile As Boolean)
    Public Event PEiteration(ByVal sender As cParameterEstimation, ByVal nowiter As Integer)
    Public Event PEcomplete(ByVal sender As cParameterEstimation)
    Public Event PEstop(ByVal sender As cParameterEstimation)
#End Region

#Region "SSR"
    Private mPESSRuniformRFori_mm As Single
    Private mPESSRuniformRFtoApply_mm As Single
    Private mCompletedWSid As Dictionary(Of Integer, PE_SS_Result)
    Private mSWScountToEstSS As Integer
    Private mCVresult_SSR_IniFlow As List(Of cCVAttribute)

#End Region

    Private mPEtype As PEtype
    Private mProject As cProject
    Private mSimulator As cSimulator
    Private mStopPE As Boolean
    Private mCompletePE As Boolean
    Private mPrintOutStep_min As Integer
    Private mbAfterAllSSRbeenOne As Boolean

    Private Structure PE_SS_Result
        Public INI_SSR As Single
        Public INI_FLOW As Single
    End Structure


    Public Sub StartPEiniSS(ByVal project As cProject, ByVal iniRF As Single, Optional ByVal printoutStep_min As Integer = 0)
        mPEtype = cParameterEstimation.PEtype.INI_SOIL_SATURATION_RATIO
        mPESSRuniformRFori_mm = iniRF
        mPESSRuniformRFtoApply_mm = iniRF
        mProject = project
        If printoutStep_min = 0 Then
            mPrintOutStep_min = CInt(cThisSimulation.dtsec / 60)
            If mPrintOutStep_min < 1 Then mPrintOutStep_min = 1
        Else
            mPrintOutStep_min = printoutStep_min
        End If
        mSWScountToEstSS = SWScountsetIniFlow()
        SimulatePEiniSS()
    End Sub


    Private Sub PE_SSR_setIniSoilConditionAndSetRFwithUniformValueForCV()
        Dim rfInterval_sec As Integer = mProject.Rainfall.RFIntervalSEC
        Dim calinterval_sec As Integer = cThisSimulation.dtsec
        Dim cellSize As Single = mProject.Watershed.mCellSize
        For n As Integer = 0 To mProject.CVCount - 1
            With mProject.CV(n)
                .InitialSaturation = 0
                .soilSaturationRatio = 0
                .soilWaterContent_m = 0
                .soilWaterContent_tM1_m = 0

                If (.FlowType = cGRM.CellFlowType.ChannelFlow AndAlso
                    .mStreamAttr.ChStrOrder > cProject.Current.SubWSPar.userPars(.WSID).dryStreamOrder) Then
                    .soilSaturationRatio = 1
                End If
            End With
            cRainfall.CalRFintensity_mPsec(mProject.CV(n), mPESSRuniformRFtoApply_mm,
                                                                   rfInterval_sec)
        Next
    End Sub


    Private Sub SimulatePEiniSS()
        Dim ts As New ThreadStart(AddressOf SimulatePEiniSSinner)
        Dim th As New Thread(ts)
        th.Start()
    End Sub

    Private Sub SimulatePEiniSSinner()
        Dim wpCount As Integer = mProject.WatchPoint.WPCount
        Dim simulator As New cSimulator
        Dim outputControl As New cOutPutControl
        If Not cThisSimulation.mGRMSetupIsNormal Then Exit Sub
        Dim nowiterForPrint As Integer = 0
        mCompletedWSid = New Dictionary(Of Integer, PE_SS_Result)
        mCVresult_SSR_IniFlow = New List(Of cCVAttribute)
        Dim bContinue As Boolean = True
        Dim niter As Integer = 0
        mCompletePE = False
        mStopPE = False
        simulator.Initialize()
        simulator.SetCVStartingCondition(mProject, wpCount, 0)
        PE_SSR_setIniSoilConditionAndSetRFwithUniformValueForCV()
        Do Until bContinue = False
            niter += 1
            Dim nowTmin As Integer = (niter - 1) * CInt(cThisSimulation.dtsec / 60)
            PE_SSR_SetPreEstimatedParameter()
            cThisSimulation.mRFMeanForDT_m = mPESSRuniformRFtoApply_mm / 1000 / mProject.Rainfall.RFIntervalSEC * cThisSimulation.dtsec
            cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = cThisSimulation.mRFMeanForDT_m
            Call simulator.SimulateRunoff(mProject, nowTmin)
            If (nowiterForPrint) * cThisSimulation.dtsec / 60 = mPrintOutStep_min OrElse niter = 1 Then
                outputControl.WriteSimResultsToTextFileForSingleEvent(mProject, wpCount, nowTmin, cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m, 1, Nothing)
                If mProject.GeneralSimulEnv.mbMakeRasterOutput = True Then
                    RaiseEvent CallAnalyzer(Me, mProject, nowTmin, mProject.GeneralSimulEnv.mbCreateImageFile, mProject.GeneralSimulEnv.mbCreateASCFile)

                End If
                nowiterForPrint = 0
            End If
            RaiseEvent PEiteration(Me, niter)
            nowiterForPrint += 1
            If mbAfterAllSSRbeenOne = True Then
                PESS_CheckSatisfyToleranceAndSetCVatt()
            Else
                PESSR_CheckSSRisOne()
                If mbAfterAllSSRbeenOne = True Then Call NoRFcondition()
            End If

            If mSWScountToEstSS = mCompletedWSid.Count Then
                RaiseEvent PEcomplete(Me)
                bContinue = False
            End If
            If mStopPE = True Then
                RaiseEvent PEstop(Me)
                bContinue = False
            End If
        Loop
    End Sub

    Private Sub PESSR_CheckSSRisOne()
        Dim sum As Single = 0
        For n As Integer = 0 To mProject.CVCount - 1
            sum = sum + mProject.CV(n).soilSaturationRatio
        Next
        Dim ave As Single = sum / mProject.CVCount
        If ave > 0.99 Then
            mbAfterAllSSRbeenOne = True
        Else
            mbAfterAllSSRbeenOne = False
        End If

    End Sub

    Private Sub NoRFcondition()
        Dim rfInterval_sec As Integer = mProject.Rainfall.RFIntervalSEC
        Dim calinterval_sec As Integer = cThisSimulation.dtsec
        Dim cellSize As Single = mProject.Watershed.mCellSize
        For n As Integer = 0 To mProject.CVCount - 1
            cRainfall.CalRFintensity_mPsec(mProject.CV(n), 0, rfInterval_sec)
        Next
        mPESSRuniformRFtoApply_mm = 0
    End Sub

    Private Sub PESS_CheckSatisfyToleranceAndSetCVatt()
        For Each id As Integer In mProject.Watershed.WSIDList
            If mCompletedWSid.ContainsKey(id) = False Then
                If mProject.SubWSPar.userPars(id).isUserSet = True AndAlso mProject.SubWSPar.userPars(id).iniFlow > 0 Then
                    Dim wsCVarrayNum As Integer = mProject.WSNetwork.WSoutletCVID(id) - 1
                    Dim err As Double = Abs(mProject.CV(wsCVarrayNum).mStreamAttr.QCVch_i_j_m3Ps - mProject.SubWSPar.userPars(id).iniFlow)
                    If err < mProject.SubWSPar.userPars(id).iniFlow / 100 Then
                        mCompletedWSid.Add(id, New PE_SS_Result)
                        Dim results As New PE_SS_Result
                        If mProject.CV(wsCVarrayNum).FlowType = cGRM.CellFlowType.OverlandFlow Then
                            results.INI_FLOW = mProject.CV(wsCVarrayNum).QCVof_i_j_m3Ps
                        Else
                            results.INI_FLOW = mProject.CV(wsCVarrayNum).mStreamAttr.QCVch_i_j_m3Ps
                        End If
                        results.INI_SSR = mProject.CV(wsCVarrayNum).soilSaturationRatio
                        mCompletedWSid(id) = results
                        SetCVattWithPEresult_SSR_IniFLOW(id)
                    End If
                End If
            End If
        Next
    End Sub

    ''' <summary>
    ''' 여기서는 매개변수 추정 과정에서 결정된 매개변수를 다시 매개변수 추정과정에서 설정해 주는것.
    ''' 다지점 보정에서 이미 추정 완료된 유역에 대해서는 매개변수 추정하지 않고, 
    ''' 그 하류만 반복적으로 추정하게 되므로.. 상류에서 설정된 값을 매먼 설정해 준다.
    ''' </summary>
    ''' <remarks></remarks>
    Private Sub PE_SSR_SetPreEstimatedParameter()
        For Each id As Integer In mCompletedWSid.Keys
            Dim wsCVarrayNum As Integer = mProject.WSNetwork.WSoutletCVID(id) - 1
            With mProject.CV(wsCVarrayNum)
                If .FlowType = cGRM.CellFlowType.OverlandFlow Then
                    .QCVof_i_j_m3Ps = mCompletedWSid(id).INI_FLOW
                Else
                    .mStreamAttr.QCVch_i_j_m3Ps = mCompletedWSid(id).INI_FLOW
                End If
                .soilSaturationRatio = mCompletedWSid(id).INI_SSR
            End With
        Next
    End Sub

    ''' <summary>
    ''' 초기포화도와 하천 초기유량을 추정된 값으로 설정하고, 해당 유역을 PE 모델링 대상에서 제외시킨다.
    ''' </summary>
    ''' <param name="wsid"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Private Function SetCVattWithPEresult_SSR_IniFLOW(ByVal wsid As Integer) As Boolean
        '우선 현재 추정된된 초기포화도와 초기유량 값을 초기조건으로 확정할 유역을 선택한다.
        Dim wsidToSet As New List(Of Integer)
        wsidToSet = GetUpWSIDlistToSet(mProject, wsid)
        If wsidToSet.Contains(wsid) = False Then wsidToSet.Add(wsid)
        For Each swsid As Integer In wsidToSet
            For Each cvid As Integer In mProject.Watershed.mCVidListForEachWS(swsid)
                Dim cv As New cCVAttribute
                cv.WSID = wsid
                cv.CVID = cvid
                cv.InitialSaturation = mProject.CV(cvid - 1).soilSaturationRatio
                If cv.FlowType = cGRM.CellFlowType.OverlandFlow Then
                    cv.QCVof_i_j_m3Ps = mProject.CV(cvid - 1).QCVof_i_j_m3Ps
                Else
                    cv.mStreamAttr.QCVch_i_j_m3Ps = mProject.CV(cvid - 1).mStreamAttr.QCVch_i_j_m3Ps
                End If
                mCVresult_SSR_IniFlow.Add(cv)
                '여기서 이렇게 설정하더라도.. 실제 모델링에서는 inlet의 상류만 false로 설정됨
                mProject.CV(cvid - 1).toBeSimulated = False
            Next
        Next
    End Function


    Private Function GetUpWSIDlistToSet(ByVal project As cProject, ByVal baseWSID As Integer) As List(Of Integer)
        GetUpWSIDlistToSet = New List(Of Integer)
        With project
            Dim wsidToExclude As New List(Of Integer)
            For Each upsid As Integer In .WSNetwork.WSIDsAllUps(baseWSID)
                If .SubWSPar.userPars(upsid).isUserSet = True AndAlso .SubWSPar.userPars(upsid).iniFlow > 0 Then
                    If Not wsidToExclude.Contains(upsid) Then
                        wsidToExclude.Add(upsid)
                    End If
                    For Each upupID As Integer In .WSNetwork.WSIDsAllUps(upsid)
                        If Not wsidToExclude.Contains(upupID) Then
                            wsidToExclude.Add(upupID)
                        End If
                    Next
                End If
            Next

            For Each upsid As Integer In .WSNetwork.WSIDsAllUps(baseWSID)
                If wsidToExclude.Contains(upsid) = False Then
                    GetUpWSIDlistToSet.Add(upsid)
                End If
            Next
        End With
    End Function

    Public Sub StopParameterEstimation()
        mStopPE = True
    End Sub

    Private ReadOnly Property SWScountsetIniFlow() As Integer
        Get
            Dim count As Integer = 0
            For Each id As Integer In mProject.Watershed.WSIDList
                If mProject.SubWSPar.userPars(id).iniFlow > 0 Then
                    count += 1
                End If
            Next
            Return count
        End Get
    End Property
End Class
