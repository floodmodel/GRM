Public Class cInfiltration

    ''' <summary>
    ''' 침투를 계산하고, 유효강우량을 산정
    ''' </summary>
    ''' <param name="cvan"></param>
    ''' <param name="RFintervalSEC"></param>
    ''' <param name="DtSEC"></param>
    ''' <remarks></remarks>
    Public Sub CalEffectiveRainfall(ByVal project As cProject, ByVal cvan As Integer, ByVal rfIntervalSEC As Integer, ByVal dtSEC As Integer)
        '이경우은 침투를 모의하지 않는 옵션의 경우
        '하도셀 중 건천차수 보다 큰 하천, 즉 침투모의하지 않은 부분
        '불투수율이 1인 경우

        With project.CV(cvan)
            If project.GeneralSimulEnv.mbSimulateInfiltration = False Then
                'subsurface flow도 모의하지 않는다.
                SetNoInfiltrationParameters(project.CV(cvan))
                Exit Sub
            End If
            If (.FlowType = cGRM.CellFlowType.ChannelFlow) AndAlso
                (.mStreamAttr.ChStrOrder > project.SubWSPar.userPars(.WSID).dryStreamOrder) Then
                SetWaterAreaInfiltrationParameters(project.CV(cvan))
                Exit Sub
            End If
            'If project.CV(cvan).ImperviousRatio = 1 Then
            If project.CV(cvan).LandCoverCode = cSetLandcover.LandCoverCode.WATR OrElse
           project.CV(cvan).LandCoverCode = cSetLandcover.LandCoverCode.WTLD Then
                SetWaterAreaInfiltrationParameters(project.CV(cvan))
                Exit Sub
            End If
            If .ImperviousRatio = 1 Then
                SetNoInfiltrationParameters(project.CV(cvan))
                Exit Sub
            End If
        End With

        Dim CONSTGreenAmpt As Single
        Dim residualMoistContentThetaR As Single

        With project.CV(cvan)
            .soilSaturationRatio = CSng(GetSoilSaturationRaito(.CumulativeInfiltrationF_tM1_m, .SoilDepthEffectiveAsWaterDepth_m, .FlowType))
            If .soilSaturationRatio > 0.99 _
                OrElse (project.mSimulationType = cGRM.SimulationType.SingleEventPE_SSR AndAlso .soilSaturationRatio = 1) _
                Then
                If .soilSaturationRatio = 1 Then
                    .CumulativeInfiltrationF_m = .SoilDepthEffectiveAsWaterDepth_m '.CumulativeInfiltrationF_tM1_m
                End If
                .bAfterSaturated = True
                .InfiltrationF_mPdt = 0
                .InfiltrationRatef_mPsec = 0
                .EffRFCV_dt_meter = .RFApp_dt_meter
                .CumulativeInfiltrationF_m = .CumulativeInfiltrationF_tM1_m
            Else
                residualMoistContentThetaR = .porosityEta - .effectivePorosityThetaE
                If residualMoistContentThetaR < 0 Then residualMoistContentThetaR = 0
                .EffectiveSaturationSe = (.porosityEta * .InitialSaturation - residualMoistContentThetaR) / (.porosityEta - residualMoistContentThetaR)

                If .EffectiveSaturationSe < 0 Then .EffectiveSaturationSe = 0
                .SoilMoistureChangeDeltaTheta = (1 - .EffectiveSaturationSe) * .effectivePorosityThetaE
                CONSTGreenAmpt = .SoilMoistureChangeDeltaTheta * .wettingFrontSuctionHeadPsi_m
                Dim infiltrationF_mPdt_max As Single
                Dim beingPonding As Boolean = False
                If .InfiltrationRatef_tM1_mPsec >= .RFReadintensity_tM1_mPsec Then
                    ' 이전 시간에서의 침투률이 이전 시간에서의 강우강도보다 컸다면, 모든 강우는 침투됨
                    '2015.03.23 불투수율은 유효강우에서 반영하는 것으로 수정.. 
                    infiltrationF_mPdt_max = .RFApp_dt_meter '* (1 - .ImperviousRatio)
                Else
                    ' 이전 시간에서의 침투률이 이전 시간에서의 강우강도보다 같거나 작았다면 ponding이 발생한 경우
                    infiltrationF_mPdt_max = GetInfiltrationForDtAfterPonding(dtSEC, cvan, CONSTGreenAmpt, .hydraulicConductK_mPsec)
                    beingPonding = True
                End If
                .InfiltrationF_mPdt = WaterDepthCanBeInfiltrated(.CumulativeInfiltrationF_tM1_m, .SoilDepthEffectiveAsWaterDepth_m, infiltrationF_mPdt_max)
                '누가 침투량으로 dt 동안에 추가된 침투량을 더한다.
                .CumulativeInfiltrationF_m = .CumulativeInfiltrationF_tM1_m + .InfiltrationF_mPdt
                '현재까지의 누가 침투량을 이용해서 이에 대한 포텐셜 침투률을 계산한다.
                If .CumulativeInfiltrationF_m <= 0 Then
                    .InfiltrationRatef_mPsec = 0
                    .CumulativeInfiltrationF_m = 0
                Else
                    .InfiltrationRatef_mPsec = .hydraulicConductK_mPsec * (1 + CONSTGreenAmpt / .CumulativeInfiltrationF_m)
                End If

                '이경우에는 침투는 있지만.. 강우는 모두 직접 유출.. 침투는 지표면 저류 상태에서 발생
                If .bAfterSaturated = True AndAlso
                    (beingPonding = True OrElse .soilSaturationRatio > 0.99) Then
                    .EffRFCV_dt_meter = .RFApp_dt_meter
                Else
                    Dim effRF_dt_m As Single = CSng(.RFApp_dt_meter - .InfiltrationF_mPdt) '+ pR
                    If effRF_dt_m > .RFApp_dt_meter Then effRF_dt_m = .RFApp_dt_meter
                    If effRF_dt_m < 0 Then effRF_dt_m = 0
                    .EffRFCV_dt_meter = effRF_dt_m
                    '2015.03.23 불투수율은 유효강우에서 반영하는 것으로 수정.. 
                    If .ImperviousRatio < 1 Then
                        .EffRFCV_dt_meter = .EffRFCV_dt_meter * (1 - .ImperviousRatio) + .RFApp_dt_meter * .ImperviousRatio
                        If .EffRFCV_dt_meter > .RFApp_dt_meter Then
                            .EffRFCV_dt_meter = .RFApp_dt_meter
                        End If
                    End If
                End If
            End If

            '유효강우량의 계산이 끝났으므로, 현재까지 계산된 침투, 강우강도 등을 tM1 변수로 저장한다.
        End With
        With project.CV(cvan)
            .soilSaturationRatio = CSng(GetSoilSaturationRaito(.CumulativeInfiltrationF_m, .SoilDepthEffectiveAsWaterDepth_m, .FlowType))
            If .soilSaturationRatio = 1 Then .bAfterSaturated = True
            .CumulativeInfiltrationF_tM1_m = .CumulativeInfiltrationF_m
            .InfiltrationRatef_tM1_mPsec = .InfiltrationRatef_mPsec
            .RFReadintensity_tM1_mPsec = .RFReadintensity_mPsec
        End With
    End Sub



    ''' <summary>
    ''' Ponding 발생 후의 침투계산
    ''' </summary>
    ''' <param name="DtSEC"></param>
    ''' <param name="cvan"></param>
    ''' <param name="CONSTGreenAmpt"></param>
    ''' <remarks>
    ''' 여기서는 누가 침투량과, 침투률을 계산한다. -> 침투률은 향후 강우강도와 비교하여 유효 강우량 계산에 이용됨.
    ''' 여기서 계산되는 침투량과 침투률은 , j=1에서 적용하기 위한 t=0에서 부터 dt 시간동안의 잠재침투량과 잠재 침투률이다.
    ''' t=2 부터는 t=t-1에서 계산된 실제 누가침투량과 그에 따른 잠재 침투률을 계산한다.
    ''' </remarks>
    Private Function GetInfiltrationForDtAfterPonding(ByVal dtSEC As Integer, ByVal cvan As Integer,
                                                      ByVal CONSTGreenAmpt As Single, ByVal Kapp As Single) As Single
        'todo : 나중에는 무강우 조건에서 토양별 초기 침투률을 look up 테이블로 만들어서 이를 바로 이용하는 것 검토
        Dim CI_n As Single
        Dim CI_nP1 As Single
        Dim err As Single
        Dim conCriteria As Single
        Dim Fx As Single
        Dim dFx As Single
        Dim constCI_tm1 As Single = 0
        '침투율 계산에서는 포화수리전도도 적용
        With cProject.Current.CV(cvan)
            '이 방법으로 초기값을 설정한다. 이전에서의 침투률로 dt 시간동안 침투량을 더한 것이므로, 실제 계산값보다 좀 크게 초기값이 주어진다.
            CI_n = .CumulativeInfiltrationF_tM1_m + .InfiltrationRatef_tM1_mPsec * dtSEC
            constCI_tm1 = .CumulativeInfiltrationF_tM1_m
            For intiterlationN As Integer = 0 To 20000
                '이건 Newton-Raphson 방법
                Fx = CSng(CI_n - constCI_tm1 - Kapp * dtSEC _
                        - CONSTGreenAmpt * Math.Log((CI_n + CONSTGreenAmpt) _
                                                    / (.CumulativeInfiltrationF_tM1_m + CONSTGreenAmpt)))
                dFx = 1 - CONSTGreenAmpt / (CI_n + CONSTGreenAmpt)
                CI_nP1 = CI_n - CSng(Fx / dFx)
                err = Math.Abs(CI_nP1 - CI_n)
                conCriteria = (CI_n * cGRM.CONST_TOLERANCE)
                If err < conCriteria Then
                    Dim dF As Single = CI_nP1 - constCI_tm1
                    If dF < 0 Then dF = 0
                    Return dF
                End If
                CI_n = CI_nP1
            Next intiterlationN
            Dim dFr As Single = CI_n - constCI_tm1
            If dFr < 0 Then
                Return 0
            Else
                Return dFr
            End If
        End With
        System.Console.WriteLine("[ColX=" & cProject.Current.CV(cvan).XCol & ", RowY=" & cProject.Current.CV(cvan).YRow & "] 에서 침투량이 수렴되지 않았습니다.   " + vbCrLf +
               "초기 침투률:" & cProject.Current.CV(cvan).InfiltrationRatef_mPsec & "mm   ")
    End Function

    Public Sub SetNoInfiltrationParameters(ByVal cv As cCVAttribute)
        With cv
            .InfiltrationF_mPdt = 0  ' 침투량 0
            .InfiltrationRatef_mPsec = 0 '침투률도 0
            '.bSaturated = False
            .soilSaturationRatio = 0
            .CumulativeInfiltrationF_m = 0
            .CumulativeInfiltrationF_tM1_m = 0
            .EffRFCV_dt_meter = .RFApp_dt_meter '모든 강우가 유효강우
        End With
    End Sub

    Private Function WaterDepthCanBeInfiltrated(ByVal preDepth As Single,
                                            ByVal maxDepth As Single, ByVal maxINFILbeCalculated As Single) As Single
        If maxINFILbeCalculated <= 0 Then Return 0
        Dim dF As Single = maxDepth - preDepth
        If dF < 0 Then dF = 0
        If dF < maxINFILbeCalculated Then
            maxINFILbeCalculated = dF
        End If

        Return maxINFILbeCalculated

    End Function

    Public Shared Sub SetWaterAreaInfiltrationParameters(ByVal CV As cCVAttribute)
        With CV
            .InfiltrationF_mPdt = 0  ' 침투량 0
            .InfiltrationRatef_mPsec = 0 '침투률도 0
            .CumulativeInfiltrationF_m = 0 '.SoilDepthEffectiveAsWaterDepth_m
            .CumulativeInfiltrationF_tM1_m = 0 ' .SoilDepthEffectiveAsWaterDepth_m
            '.bSaturated = True
            .soilSaturationRatio = 1
            .EffRFCV_dt_meter = .RFApp_dt_meter '모든 강우가 유효강우
        End With
    End Sub

    Public Shared Function GetSoilSaturationRaito(ByVal cumulativeInfiltration As Single,
                                                  ByVal effSoilDepth As Single, ByVal flowType As cGRM.CellFlowType) As Single
        Dim SSR As Single = 0

        If flowType = cGRM.CellFlowType.ChannelFlow Then 'OrElse flowType = cGRM.CellFlowType.ChannelNOverlandFlow Then
            SSR = 1
        End If
        If effSoilDepth <= 0 Then SSR = 1
        If cumulativeInfiltration <= 0 Then SSR = 0
        SSR = cumulativeInfiltration / effSoilDepth
        If SSR > 1 Then SSR = 1
        Return SSR

    End Function


    Public Shared Function Kunsaturated(ByVal cv As cCVAttribute, powCUnsaturatedK As Single) As Single
        'Dim ca As Single = 0.2472 '0.2466 '0.2269 '60cm : 0.2288 '50cm : 0.3776, 90cm : 0.0854
        Dim ca As Single = 0.2 '0.24
        'Dim cb As Single = 1 '-0.498
        'Dim cc As Single = 0 '0.5024
        Dim ssr As Single = GetSoilSaturationRaito(cv.CumulativeInfiltrationF_tM1_m, cv.SoilDepthEffectiveAsWaterDepth_m, cv.FlowType)
        Dim Ks As Single = cv.hydraulicConductK_mPsec
        If ssr > 0.99 Then
            Return Ks
        Else
            Dim Kus As Single
            Kus = CSng(ca * Ks * ssr)
            'Kus = CSng(Ks * ssr ^ 6.4)
            'Kus = CSng(Ks * ssr ^ powCUnsaturatedK)
            Return Kus
        End If
    End Function

End Class
