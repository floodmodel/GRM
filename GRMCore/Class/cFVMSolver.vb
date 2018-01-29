Imports System.Math

Public Class cFVMSolver
    Public Sub CalculateOverlandFlow(project As cProject, ByVal CVAN As Integer,
                                                ByVal hCVw_i_jP1 As Single, ByVal effDy_m As Single)
        With project.CV(CVAN)
            Dim constHp_j As Single = .hCVof_i_j
            Dim CONST_DtPDx As Single = CInt(cThisSimulation.dtsec) / .CVDeltaX_m
            Dim Hp_n As Single = constHp_j

            Dim hCVw_n As Single = hCVw_i_jP1
            Dim uCVw_n As Single = 0
            Dim cCVw_n As Single = 0
            If hCVw_n > 0 Then
                uCVw_n = GetFlowVelocityByManningEq(hCVw_n, .SlopeOF, .RoughnessCoeffOF)
                cCVw_n = uCVw_n * CONST_DtPDx * hCVw_n
            End If
            For iter As Integer = 0 To 20000
                Dim hCVe_n As Single = Hp_n
                Dim uCVe_n As Single = GetFlowVelocityByManningEq(hCVe_n, .SlopeOF, .RoughnessCoeffOF)
                Dim cCVe_n As Single = uCVe_n * CONST_DtPDx * hCVe_n
                'Newton-Raphson
                Dim Fx As Single = Hp_n - cCVw_n + cCVe_n - constHp_j
                Dim dFx As Single
                dFx = 1 + CSng(1.66667 * CONST_DtPDx _
                         * (hCVe_n) ^ (0.66667) * (.SlopeOF ^ 0.5) / .RoughnessCoeffOF)
                Dim Hp_nP1 As Single = CSng(Hp_n - Fx / dFx)
                If Hp_nP1 <= 0 Then
                    SetNoFluxOverlandFlowCV(project.CV(CVAN))
                    Exit For
                End If
                Dim tolerance As Single = Hp_n * cGRM.CONST_TOLERANCE
                Dim err As Single = Abs(Hp_nP1 - Hp_n)
                If err < tolerance Then
                    .hCVof_i_j = Hp_nP1
                    .uCVof_i_j = GetFlowVelocityByManningEq(Hp_nP1, .SlopeOF, .RoughnessCoeffOF)
                    .CSAof_i_j = Hp_nP1 * effDy_m
                    .QCVof_i_j_m3Ps = .CSAof_i_j * .uCVof_i_j
                    If cThisSimulation.vMaxInThisStep < .uCVof_i_j Then
                        cThisSimulation.vMaxInThisStep = .uCVof_i_j
                    End If
                    Exit For
                End If
                Hp_n = Hp_nP1
            Next iter
        End With
    End Sub

    Public Sub CalculateChannelFlow(project As cProject, ByVal cvan As Integer,
                                               ByVal CSAchCVw_i_jP1 As Single)
        With project.CV(cvan)
            Dim CONST_DtPDx As Single _
                = CInt(cThisSimulation.dtsec) / .CVDeltaX_m
            Dim HRch As Single
            Dim CSPerCh As Single
            Dim hChCVw_i_jP1 As Single
            Dim uCVw_n As Single
            Dim cChCVw_n As Single
            If CSAchCVw_i_jP1 = 0 Then
                hChCVw_i_jP1 = 0
                HRch = 0
                CSPerCh = 0
                uCVw_n = 0
                cChCVw_n = 0
            Else
                hChCVw_i_jP1 = GetChannelDepthUsingArea(.mStreamAttr.ChBaseWidth,
                                    CSAchCVw_i_jP1, .mStreamAttr.chIsCompoundCS,
                                   .mStreamAttr.chUpperRBaseWidth_m, .mStreamAttr.chLowerRArea_m2,
                                   .mStreamAttr.chLowerRHeight, .mStreamAttr.mChBankCoeff)
                CSPerCh = GetChannelCrossSectionPerimeter(.mStreamAttr.ChBaseWidth,
                            .mStreamAttr.chSideSlopeRight, .mStreamAttr.chSideSlopeLeft,
                            hChCVw_i_jP1, .mStreamAttr.chIsCompoundCS, .mStreamAttr.chLowerRHeight,
                           .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chUpperRBaseWidth_m)
                HRch = CSAchCVw_i_jP1 / CSPerCh
                uCVw_n = GetFlowVelocityByManningEq(HRch, .mStreamAttr.chBedSlope, .mStreamAttr.RoughnessCoeffCH)
                cChCVw_n = uCVw_n * CONST_DtPDx * CSAchCVw_i_jP1
            End If

            Dim constCSAchCVp_j As Single = .mStreamAttr.CSAch_i_j
            Dim CSAp_n As Single = constCSAchCVp_j
            Dim hChp_n As Single = .mStreamAttr.hCVch_i_j

            For iter As Integer = 0 To 20000
                Dim hChCVe_n As Single = hChp_n
                Dim CSAChCVe_n As Single = CSAp_n
                CSPerCh = GetChannelCrossSectionPerimeter(.mStreamAttr.ChBaseWidth,
                                .mStreamAttr.chSideSlopeRight, .mStreamAttr.chSideSlopeLeft,
                                hChCVe_n, .mStreamAttr.chIsCompoundCS, .mStreamAttr.chLowerRHeight,
                                .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chUpperRBaseWidth_m)
                HRch = CSAChCVe_n / CSPerCh
                Dim u_n As Single = GetFlowVelocityByManningEq(HRch, .mStreamAttr.chBedSlope, .mStreamAttr.RoughnessCoeffCH)
                Dim cChCVe_n As Single = u_n * CONST_DtPDx * CSAChCVe_n
                'Newton-Raphson
                Dim Fx As Single = CSAp_n - cChCVw_n + cChCVe_n - constCSAchCVp_j
                Dim dFx As Single
                dFx = 1 + CSng(1.66667 * (CSAChCVe_n) ^ (0.66667) * (.mStreamAttr.chBedSlope ^ 0.5) *
                               CONST_DtPDx / (.mStreamAttr.RoughnessCoeffCH * CSPerCh ^ 0.66667))
                Dim CSAch_nP1 As Single = CSAp_n - Fx / dFx
                If CSAch_nP1 <= 0 Then
                    SetNoFluxChannelFlowCV(project.CV(cvan))
                    Exit For
                End If
                Dim Qn As Single = u_n * CSAp_n
                HRch = CSAch_nP1 / CSPerCh
                Dim u_nP1 As Single = GetFlowVelocityByManningEq(HRch, .mStreamAttr.chBedSlope, .mStreamAttr.RoughnessCoeffCH)
                Dim QnP1 As Single = u_nP1 * CSAch_nP1
                Dim tolerance As Single = CSng(Qn * cGRM.CONST_TOLERANCE)
                Dim err As Single = Abs(Qn - QnP1)
                Dim hCh_nP1 As Single = GetChannelDepthUsingArea(.mStreamAttr.ChBaseWidth, CSAch_nP1,
                                                .mStreamAttr.chIsCompoundCS,
                                   .mStreamAttr.chUpperRBaseWidth_m,
                                   .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chLowerRHeight,
                                   .mStreamAttr.mChBankCoeff)
                If err < tolerance Then
                    .mStreamAttr.hCVch_i_j = hCh_nP1
                    .mStreamAttr.uCVch_i_j = u_nP1
                    .mStreamAttr.CSAch_i_j = CSAch_nP1
                    .mStreamAttr.QCVch_i_j_m3Ps = QnP1
                    If cThisSimulation.vMaxInThisStep < u_nP1 Then
                        cThisSimulation.vMaxInThisStep = u_nP1
                    End If
                    Exit For
                End If
                CSAp_n = CSAch_nP1
                hChp_n = hCh_nP1
            Next iter
        End With
    End Sub


    Public Function CalChCSA_CViW(ByVal project As cProject, ByVal cvan As Integer) As Single
        'w의 단면적 계산
        Dim CSAe_iM1 As Single
        Dim CSAeSum_iM1 As Single = 0
        Dim qSumCViM1_m3Ps As Single = 0
        Dim effCellCountFlowToCV_iM1 As Byte = CByte(project.CV(cvan).NeighborCVidFlowIntoMe.Count)
        For Each cvid As Integer In project.CV(cvan).NeighborCVidFlowIntoMe
            Dim qCViM1_m3Ps As Single
            With project.CV(cvid - 1)
                Select Case .FlowType
                    Case cGRM.CellFlowType.OverlandFlow
                        CSAe_iM1 = .CSAof_i_j
                        qCViM1_m3Ps = .QCVof_i_j_m3Ps
                    Case cGRM.CellFlowType.ChannelFlow, cGRM.CellFlowType.ChannelNOverlandFlow
                        CSAe_iM1 = .mStreamAttr.CSAch_i_j
                        qCViM1_m3Ps = .mStreamAttr.QCVch_i_j_m3Ps
                        If .FlowType = cGRM.CellFlowType.ChannelNOverlandFlow Then
                            CSAe_iM1 = CSAe_iM1 + .mStreamAttr.CSAchAddedByOFinCHnOFcell
                            qCViM1_m3Ps = qCViM1_m3Ps + .QCVof_i_j_m3Ps
                        End If
                End Select
                qSumCViM1_m3Ps = qSumCViM1_m3Ps + qCViM1_m3Ps
                CSAeSum_iM1 = CSAeSum_iM1 + CSAe_iM1
                If qCViM1_m3Ps <= 0.0 Then
                    effCellCountFlowToCV_iM1 = CByte(effCellCountFlowToCV_iM1 - 1)
                End If
            End With
        Next
        project.CV(cvan).QsumCVw_dt_m3 = project.CV(cvan).QsumCVw_dt_m3 +
                   qSumCViM1_m3Ps * CInt(cThisSimulation.dtsec)

        If effCellCountFlowToCV_iM1 < 1 Then effCellCountFlowToCV_iM1 = 1
        project.CV(cvan).effCVCountFlowINTOCViW = effCellCountFlowToCV_iM1

        If CSAeSum_iM1 < cGRM.CONST_WET_AND_DRY_CRITERIA Then
            CalChCSA_CViW = 0
            Exit Function
        End If

        Dim CSAw_n As Single = CSAeSum_iM1
        Dim CSAw_nP1 As Single
        With project.CV(cvan)
            For iter As Integer = 0 To 20000
                Dim hWn_i As Single = GetChannelDepthUsingArea(.mStreamAttr.ChBaseWidth, CSAw_n,
                                                      .mStreamAttr.chIsCompoundCS, .mStreamAttr.chUpperRBaseWidth_m,
                                           .mStreamAttr.chLowerRArea_m2, .mStreamAttr.chLowerRHeight, .mStreamAttr.mChBankCoeff)
                Dim chCSPeri As Single = GetChannelCrossSectionPerimeter(.mStreamAttr.ChBaseWidth,
                                                                         .mStreamAttr.chSideSlopeRight, .mStreamAttr.chSideSlopeLeft, hWn_i,
                                                                        .mStreamAttr.chIsCompoundCS,
                                                                        .mStreamAttr.chLowerRHeight,
                                                                        .mStreamAttr.chLowerRArea_m2,
                                                                        .mStreamAttr.chUpperRBaseWidth_m)
                Dim Fx As Single = CSng((CSAw_n ^ 1.66667 * .mStreamAttr.chBedSlope ^ 0.5) _
                                                / (.mStreamAttr.RoughnessCoeffCH * chCSPeri ^ 0.66667) - qSumCViM1_m3Ps)
                Dim dFx As Single = CSng(1.66667 * (CSAw_n ^ 0.66667) * (.mStreamAttr.chBedSlope ^ 0.5) _
                                               / (.mStreamAttr.RoughnessCoeffCH * chCSPeri ^ 0.66667))
                CSAw_nP1 = CSAw_n - Fx / dFx
                Dim err As Single = Abs(CSAw_nP1 - CSAw_n) / CSAw_n
                If err < cGRM.CONST_TOLERANCE Then
                    Return CSAw_nP1
                End If
                CSAw_n = CSAw_nP1
            Next iter
            Return CSAw_nP1
        End With
    End Function



    Public Function CalChCSAFromQbyIteration(ByVal cv As cCVAttribute, ByVal CASini As Single,
                                             ByVal Q_m3Ps As Single) As Single
        Dim CSA_nP1 As Single
        With cv
            Dim cbw As Single = .mStreamAttr.ChBaseWidth
            Dim bc As Single = .mStreamAttr.mChBankCoeff
            Dim bCompound As Boolean = .mStreamAttr.chIsCompoundCS
            Dim hLR As Single = .mStreamAttr.chLowerRHeight
            Dim AreaLR As Single = .mStreamAttr.chLowerRArea_m2
            Dim bwUpperRegion As Single = .mStreamAttr.chUpperRBaseWidth_m
            Dim CSA_n As Single
            CSA_n = CASini
            Dim Fx As Single
            Dim dFx As Single
            Dim ChCrossSecPer As Single
            For iter As Integer = 0 To 20000
                Dim Hw_n As Single = GetChannelDepthUsingArea(cbw, CSA_n, .mStreamAttr.chIsCompoundCS,
                               bwUpperRegion, AreaLR, hLR, bc)
                ChCrossSecPer = GetChannelCrossSectionPerimeter(cbw,
                                                                   .mStreamAttr.chSideSlopeRight, .mStreamAttr.chSideSlopeLeft,
                                                                   Hw_n, bCompound,
                                                                   hLR, AreaLR, bwUpperRegion)
                Fx = CSng((CSA_n ^ 1.66667 * .mStreamAttr.chBedSlope ^ 0.5) _
                            / (.mStreamAttr.RoughnessCoeffCH * ChCrossSecPer ^ 0.66667) - Q_m3Ps)
                dFx = CSng(1.66667 * (CSA_n ^ 0.66667) * (.mStreamAttr.chBedSlope ^ 0.5) _
                            / (.mStreamAttr.RoughnessCoeffCH * ChCrossSecPer ^ 0.66667))
                CSA_nP1 = CSA_n - Fx / dFx
                Dim toler As Single = CSng(CSA_n * cGRM.CONST_TOLERANCE)
                Dim err As Single = Abs(CSA_nP1 - CSA_n)
                If err < toler Then
                    Return CSA_nP1
                End If
                CSA_n = CSA_nP1
            Next iter
        End With
        Return CSA_nP1
    End Function

    ''' <summary>
    ''' 유량 보전 연속방정식을 적용하여 새로 계산된 w의 수위를 반환. 최상류셀은 계산하지 않는다
    ''' </summary>
    ''' <param name="cvan"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Function CalculateOverlandWaterDepthCViW(ByVal project As cProject, ByVal cvan As Integer) As Single
        Dim qSumToCViM1 As Single
        Dim qCViM1 As Single
        Dim qWn_i As Single
        Dim effCellCountFlowToCViW As Byte
        effCellCountFlowToCViW = CByte(project.CV(cvan).NeighborCVidFlowIntoMe.Count)
        For Each cvid As Integer In project.CV(cvan).NeighborCVidFlowIntoMe
            With project.CV(cvid - 1)
                qCViM1 = .QCVof_i_j_m3Ps / project.Watershed.mCellSize ' 단위폭당 유량
                If qCViM1 <= 0.0 Then
                    effCellCountFlowToCViW = CByte(effCellCountFlowToCViW - 1)
                    qCViM1 = 0
                End If
                qSumToCViM1 = qSumToCViM1 + qCViM1
                project.CV(cvan).QsumCVw_dt_m3 = project.CV(cvan).QsumCVw_dt_m3 _
                    + .QCVof_i_j_m3Ps * cThisSimulation.dtsec
            End With
        Next
        With project.CV(cvan)
            If effCellCountFlowToCViW < 1 Then effCellCountFlowToCViW = 1
            .effCVCountFlowINTOCViW = effCellCountFlowToCViW
            qWn_i = CSng(((.RoughnessCoeffOF * qSumToCViM1) / (.SlopeOF ^ 0.5)) ^ 0.6)
        End With
        Return qWn_i
    End Function

    Public Function GetFlowVelocityByManningEq(ByVal hydraulicRaidus As Single, ByVal slope As Double, ByVal nCoeff As Single) As Single
        Dim vManning As Single = CSng(((hydraulicRaidus) ^ 0.66667) * (slope) ^ 0.5 / nCoeff)
        Return vManning
    End Function



    ''' <summary>
    ''' 하도의 흐름 단면
    ''' </summary>
    ''' <param name="lowerRegionBaseWidth"></param>
    ''' <param name="chBankConst"></param>
    ''' <param name="crossSectionDepth"></param>
    ''' <param name="isCompoundCrossSection"></param>
    ''' <param name="lowerRegionHeight"></param>
    ''' <param name="lowerRegionArea"></param>
    ''' <param name="upperRegionBaseWidth"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Function GetChannelCrossSectionAreaUsingDepth(ByVal lowerRegionBaseWidth As Single, _
                                               ByVal chBankConst As Single, _
                                               ByVal crossSectionDepth As Single, _
                                               ByVal isCompoundCrossSection As Boolean, _
                                               ByVal lowerRegionHeight As Single, _
                                               ByVal lowerRegionArea As Single, _
                                               ByVal upperRegionBaseWidth As Single) As Single
        If (isCompoundCrossSection = True) Then
            If (crossSectionDepth > lowerRegionHeight) Then
                Dim highFlowDepth As Single = crossSectionDepth - lowerRegionHeight
                If highFlowDepth < 0 Then highFlowDepth = 0
                Dim sngHighFlowArea As Single = (upperRegionBaseWidth + chBankConst * highFlowDepth / 2) * highFlowDepth
                Return (lowerRegionArea + sngHighFlowArea)
            End If
        End If
        Return (lowerRegionBaseWidth + chBankConst * crossSectionDepth / 2) * crossSectionDepth
    End Function

    ''' <summary>
    ''' 윤변계산
    ''' </summary>
    ''' <param name="lowerRegionBaseWidth"></param>
    ''' <param name="sideSlopeRightBank"></param>
    ''' <param name="sideSlopeLeftBank"></param>
    ''' <param name="crossSectionDepth"></param>
    ''' <param name="isCompoundCrossSection"></param>
    ''' <param name="lowerRegionHeight"></param>
    ''' <param name="lowerRegionArea"></param>
    ''' <param name="upperRegionBaseWidth"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Function GetChannelCrossSectionPerimeter(ByVal lowerRegionBaseWidth As Single, _
                                                    ByVal sideSlopeRightBank As Single, _
                                                    ByVal sideSlopeLeftBank As Single, _
                                                    ByVal crossSectionDepth As Single, _
                                                    ByVal isCompoundCrossSection As Boolean, _
                                                    ByVal lowerRegionHeight As Single, _
                                                    ByVal lowerRegionArea As Single, _
                                                    ByVal upperRegionBaseWidth As Single) As Single
        If isCompoundCrossSection = True Then
            If crossSectionDepth > lowerRegionHeight Then
                Dim lowFlowPerimeter As Single = CSng((lowerRegionBaseWidth + Sqrt(lowerRegionHeight ^ 2 + (lowerRegionHeight / sideSlopeRightBank) ^ 2) _
                                             + Sqrt(lowerRegionHeight ^ 2 + (lowerRegionHeight / sideSlopeLeftBank) ^ 2)))
                Dim highFlowDepth As Single = crossSectionDepth - lowerRegionHeight
                Dim highFlowPerimeter As Single = CSng(Sqrt(highFlowDepth ^ 2 + (highFlowDepth / sideSlopeRightBank) ^ 2) _
                                          + Sqrt(highFlowDepth ^ 2 + (highFlowDepth / sideSlopeLeftBank) ^ 2) + upperRegionBaseWidth - lowerRegionBaseWidth)
                Return (lowFlowPerimeter + highFlowPerimeter)
            End If
        End If

        Return CSng(lowerRegionBaseWidth + Sqrt(crossSectionDepth ^ 2 + (crossSectionDepth / sideSlopeRightBank) ^ 2) _
                                                  + Sqrt(crossSectionDepth ^ 2 + (crossSectionDepth / sideSlopeLeftBank) ^ 2))
    End Function


    ''' <summary>
    ''' 근의 공식으로 하도 단면의 수심 계산
    ''' </summary>
    ''' <param name="baseWidthLowerRegion"></param>
    ''' <param name="chCSAinput"></param>
    ''' <param name="bIsCompoundCrossSection"></param>
    ''' <param name="baseWidthUpperRegion"></param>
    ''' <param name="dblLowerRegionArea"></param>
    ''' <param name="lowerRegionHeight"></param>
    ''' <param name="chBankConst"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Function GetChannelDepthUsingArea(ByVal baseWidthLowerRegion As Single, ByVal chCSAinput As Single,
                                    ByVal bIsCompoundCrossSection As Boolean, ByVal baseWidthUpperRegion As Single,
                                    ByVal dblLowerRegionArea As Single, ByVal lowerRegionHeight As Single, ByVal chBankConst As Single) As Single
        Dim sngChCSDepth As Single
        If bIsCompoundCrossSection = True AndAlso chCSAinput > dblLowerRegionArea Then
            Dim sngHighRegionArea As Single = chCSAinput - dblLowerRegionArea
            sngChCSDepth = CSng((Sqrt(baseWidthUpperRegion ^ 2 + 2 * chBankConst * sngHighRegionArea) - baseWidthUpperRegion) / chBankConst)
            sngChCSDepth = sngChCSDepth + lowerRegionHeight
        Else
            sngChCSDepth = CSng((Sqrt(baseWidthLowerRegion ^ 2 + 2 * chBankConst * chCSAinput) - baseWidthLowerRegion) / chBankConst)
        End If
        GetChannelDepthUsingArea = sngChCSDepth
    End Function

    Public Sub SetNoFluxOverlandFlowCV(cv As cCVAttribute)
        With cv
            .hCVof_i_j = 0
            .uCVof_i_j = 0
            .CSAof_i_j = 0
            .QCVof_i_j_m3Ps = 0
        End With
    End Sub

    Public Sub SetNoFluxChannelFlowCV(cv As cCVAttribute)
        With cv
            .mStreamAttr.hCVch_i_j = 0
            .mStreamAttr.uCVch_i_j = 0
            .mStreamAttr.CSAch_i_j = 0
            .mStreamAttr.QCVch_i_j_m3Ps = 0
        End With
    End Sub

End Class
