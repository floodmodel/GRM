Public Class cSSandBSflow

    ''' <summary>
    ''' 상류셀에서 들어오는 ssf에 의한 현재 셀의 overland flow 기여분 계산. 반환 m2
    ''' 현재 셀에서의 지표하 유출 계산
    ''' </summary>
    ''' <param name="cvan"></param>
    ''' <param name="dtSEC"></param>
    ''' <param name="Dy_m"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Function GetRFlowOFAreaAddedBySSflowCVwAndCalSSflowAtNowCV(ByVal project As cProject,
                               ByVal cvan As Integer, ByVal dtSEC As Integer,
                               ByVal Dy_m As Single, ByVal Dx_m As Single) As Single

        With project.CV(cvan)
            'todo : 지표하 유출관련.. 포화되지 않아도 기여, 2013.05.20
            Dim SSFfromCVw As Single
            Dim SSFAddedToMeFromCVw_m As Single
            Dim RflowBySSFfromCVw_m2 As Single '이건 return flow
            SSFfromCVw = GetTotalSSFfromCVwOFcell_m3Ps(project, cvan) * dtSEC / (Dy_m * Dx_m)

            '상류의 지표하 유출에 의한 현재 셀의 rf 기여분 계산
            If cInfiltration.GetSoilSaturationRaito(.soilWaterContent_tM1_m, .SoilDepthEffectiveAsWaterDepth_m, .FlowType) >= 0.99 Then
                RflowBySSFfromCVw_m2 = SSFfromCVw * Dy_m
                SSFAddedToMeFromCVw_m = 0
            Else
                RflowBySSFfromCVw_m2 = SSFfromCVw * .soilSaturationRatio * Dy_m
                SSFAddedToMeFromCVw_m = SSFfromCVw * (1 - .soilSaturationRatio)
            End If

            ' 상류에서 유입된 지표하 유출 깊이 더하고..
            .soilWaterContent_tM1_m = .soilWaterContent_tM1_m + SSFAddedToMeFromCVw_m

            '현재의 침투 깊이를 이용해서 rf 계산하고..

            If .soilWaterContent_tM1_m > .SoilDepthEffectiveAsWaterDepth_m Then
                '하류로 이송된 지표하 유출량이 현재 셀의 포화가능 깊이를 초과하면, 초과분은 returnflow
                RflowBySSFfromCVw_m2 = (.soilWaterContent_tM1_m - .SoilDepthEffectiveAsWaterDepth_m) * Dy_m _
                                        + RflowBySSFfromCVw_m2
                .soilWaterContent_tM1_m = .SoilDepthEffectiveAsWaterDepth_m
            End If

            If .soilWaterContent_tM1_m < 0 Then .soilWaterContent_tM1_m = 0
            '이건 현재 셀의 지표하 유출. 지표하 유출이 발생하는 부분은 물이 차있는 부분이므로.. 포화수리전도도 적용
            .SSF_Q_m3Ps = (.soilWaterContent_tM1_m / .porosityEta) _
                      * .hydraulicConductK_mPsec * CSng(Math.Sin(Math.Atan(.SlopeOF))) * Dy_m

            '이건 하류로 유출한 이후의 현재 누가침투심
            .soilWaterContent_tM1_m = .soilWaterContent_tM1_m _
                    - .SSF_Q_m3Ps / (Dy_m * .CVDeltaX_m) * dtSEC
            Return RflowBySSFfromCVw_m2 ' + .subSurfaceflow_Q_m3Ps
        End With
    End Function


    Public Function GetTotalSSFfromCVwOFcell_m3Ps(ByVal project As cProject, ByVal cvan As Integer) As Single
        Dim cumulativeSSF_m3Ps As Single
        For Each objCellid As Integer In project.CV(cvan).NeighborCVidFlowIntoMe
            With project.CV(objCellid - 1)
                If .FlowType = cGRM.CellFlowType.OverlandFlow Then '상류셀 중 overlandflow type에 대해서만 받는다.
                    '즉, GRM에서 Green-Ampt 모형을 이용해서 침투된 양을 계산할때, 포화된 깊이로 계산됨. 즉, 현재 침투률로 얼만큼 깊이 들어갔는지는 계산하지 않는다..
                    '따라서, 하도로 기여하는 지표하 유출량 계산할때는 누가침투깊이가 .saturatedSoildepth_m 이 된다.
                    cumulativeSSF_m3Ps = cumulativeSSF_m3Ps + .SSF_Q_m3Ps
                End If
            End With
        Next
        Return cumulativeSSF_m3Ps
    End Function

    ''' <summary>
    '''지표하 유출을 하폭이 격자크기보다 큰 경우의 셀에 대해서 모의 2009.09.12
    '''하도셀에서는 침투가 없으므로, 하도셀 자체에서 dunne type flow는 발생하지 않는다.
    '''상류셀에서 발생된 dunne type flow는 상류셀의 overlandflow 유출 계산에서 반영되어 있음.
    '''m2, dt 시간에 대해서 평균된 면적을 반환함.
    ''' </summary>
    ''' <param name="cvan"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Function GetChCSAaddedBySSFlowInChlCell(ByVal project As cProject, ByVal cvan As Integer) As Single
        Dim cumulativeSSFlow_cms As Single
        cumulativeSSFlow_cms = GetTotalSSFfromCVwOFcell_m3Ps(project, cvan)
        If cumulativeSSFlow_cms > 0 Then
            'todo : 여기 발산 가능성 있음.
            If project.CV(cvan).mStreamAttr.uCVch_i_j > 0 Then
                Dim chCSA As Single
                chCSA = cumulativeSSFlow_cms / project.CV(cvan).mStreamAttr.uCVch_i_j '이전 시간에 계산된 유속

                '유속이 작을 경우.. 발산 가능성 있으므로.. 기존 단면적 보다 큰 단면적으로 유입될 경우.. 
                '기존 단면적으로 유입되는 것으로 설정..
                '즉, 지표하 유출의 기여 단면적은 기존 하천단면적보다 클수 없음.
                If chCSA > project.CV(cvan).mStreamAttr.CSAch_i_j Then
                    chCSA = project.CV(cvan).mStreamAttr.CSAch_i_j
                End If
                Return chCSA
            End If
        End If
        '이경우에는 하도에 기여되지 않는 것으로 가정
        '즉 강우에 의한 하도의 유량 발생이 시작되지 않았는데(하도가 말라있을 경우).. 
        '지표하 유출에 의해 기여가 있을 수 없음.
        Return 0
        '이것으로 반복계산하면.. 작은 유량에 대해 수렴해야 하므로.. 계산시간 많이 걸린다.
        '따라서 위에서 유속으로 나눈값으로 근사한다.
        '실제 계산해보면 첨두 1000cms 정도되는 수문곡선에서 최대 0.1cms 정도의 차이가 난다.
        'sngCSAaddedBySubsurfaceflowFromCVw = CalculateChannelCrossSectionAreaFromQbyIteration(intCVArrayNumber, sngCSAini, sngCumulativeSubsurfaceDischarge_m3Ps)
    End Function



    ' 지표하 유출을 하폭이 격자크기보다 작은 경우의 셀에 대해서 모의 2009.09.12
    Public Function GetChCSAaddedBySSFlowInChNOfCell(ByVal project As cProject, ByVal cvan As Integer) As Single
        With project.CV(cvan)
            '현재 셀의 overlandflow 구간에서 발생된 subsurface flow 유량을 받는다.
            If .SSF_Q_m3Ps > 0 Then
                'todo : 여기 발산 가능성 있음.
                If .mStreamAttr.uCVch_i_j > 0 Then
                    Dim chCSA As Single
                    chCSA = .SSF_Q_m3Ps / .mStreamAttr.uCVch_i_j
                    '유속이 작을 경우.. 발산 가능성 있으므로.. 기존 단면적 보다 큰 단면적으로 유입될 경우.. 기존 단면적으로 유입되는 것으로 설정..
                    '즉, 지표하 유출의 기여 단면적은 기존 하천단면적보다 클수 없음.
                    If chCSA > project.CV(cvan).mStreamAttr.CSAch_i_j Then
                        chCSA = project.CV(cvan).mStreamAttr.CSAch_i_j
                    End If
                    Return chCSA
                End If
            End If
            '이경우에는 하도에 기여되지 않는 것으로 가정
            '즉 강우에 의한 하도의 유량 발생이 시작되지 않았는데(하도가 말라있을 경우).. 지표하 유출에 의해 기여가 있을 수 없음.
            Return 0
            '이것으로 반복계산하면.. 작은 유량에 대해 수렴해야 하므로.. 계산시간 많이 걸린다. 따라서 위에서 유속으로 나눈값으로 근사한다.
            '실제 계산해보면 첨두 1000cms 정도되는 수문곡선에서 최대 1cms 정도의 차이가 난다.
            'sngCSAaddedBySubsurfaceflowFromCVw = CalculateChannelCrossSectionAreaFromQbyIteration(intCVArrayNumber, sngCSAini, sngSubsurfaceflowOccurredInThisCell_m3Ps)
        End With
    End Function



    ''' <summary>
    ''' 상류로부터 유입된 기저유출에 의한 현재 검사체적의 지하수위 상승분과 기저유출을 계산
    ''' </summary>
    ''' <param name="cvan"></param>
    ''' <param name="DX_m"></param>
    ''' <param name="DY_m"></param>
    ''' <param name="DTSec"></param>
    ''' <remarks></remarks>
    Public Sub GetBaseflowInputDepthAndCalculateLateralMovement(ByVal project As cProject, ByVal cvan As Integer, ByVal dX_m As Single, ByVal dY_m As Single, ByVal dTSec As Single)
        Dim cumulBFq_m3Ps As Single = 0
        Dim dhUAQ_m As Single
        With project
            For Each cvid As Integer In .CV(cvan).NeighborCVidFlowIntoMe
                If .CV(cvid - 1).FlowType = cGRM.CellFlowType.OverlandFlow Then
                    cumulBFq_m3Ps = cumulBFq_m3Ps + .CV(cvid - 1).baseflow_Q_m3Ps  '수두경사가 셀의 지표면 경사와 같은 것으로 가정
                End If
            Next
            With .CV(cvan)
                If .FAc = project.FacMin Then dX_m = dX_m / 2
                'sngDhUAQ_m = sngCumulativeBFq_m3Ps * intDTSec / (sngDeltaY_m * sngDeltaX_m)
                dhUAQ_m = cumulBFq_m3Ps * dTSec / (dY_m * dX_m) / .effectivePorosityThetaE  ' 이건 검사체적에 균질하게 퍼져 있는 수위변화분
                .hUAQfromBedrock_m = .hUAQfromBedrock_m + dhUAQ_m '이건 지하수대의 토양 깊이 변화
                If .hUAQfromBedrock_m > (.SoilDepthToBedrock_m - .soilDepth_m) Then .hUAQfromBedrock_m = .SoilDepthToBedrock_m - .soilDepth_m
                '기저유출 계산에서는 포화수리전도도 적용. 포화된 영역에서 발생
                .baseflow_Q_m3Ps = .hUAQfromBedrock_m * .hydraulicConductK_mPsec * CSng(Math.Sin(Math.Atan(.SlopeOF))) * dY_m
                If .baseflow_Q_m3Ps < 0 Then .baseflow_Q_m3Ps = 0
            End With
        End With
    End Sub


    Function CalBFlowAndGetCSAAddedByBFlow(ByVal project As cProject, ByVal cvan As Integer,
                                           ByVal dtsec As Integer, ByVal cellSize_m As Single) As Single
        Dim deltaSoilDepthofUAQ_m As Single ' B층(비피압 대수층)의 높이 변화. A층이 포화될 경우, B층으로 침누되면서 발생, 연직방향 침투깊이
        Dim soilDepthPercolated_m As Single
        Dim waterDepthPercolated_m As Single
        Dim csa As Single = 0
        deltaSoilDepthofUAQ_m = 0
        With project.CV(cvan)
            '토양의 포화 상태와 상관없이 침누가 발생한다. 토양포화도가 상승 및 하강한다.
            If .soilSaturationRatio > 0 Then
                soilDepthPercolated_m = cInfiltration.Kunsaturated(project.CV(cvan), .UKType, .coefUK) * dtsec
            Else
                soilDepthPercolated_m = 0
            End If
            If .soilDepth_m < soilDepthPercolated_m Then 'B층의 최대 증가 가능 높이는 토양심을 초과하지 않는다.
                soilDepthPercolated_m = .soilDepth_m
            End If
            'If .FlowType <> cGRM.CellFlowType.ChannelFlow AndAlso
            '  project.CV(cvan).LandCoverCode <> cSetLandcover.LandCoverCode.WATR AndAlso
            '  project.CV(cvan).LandCoverCode <> cSetLandcover.LandCoverCode.WTLD Then 'channel flow 혹은 수역에서는 토양수분함량 변하지 않는다.
            '    'If .soilDepth_m = soilDepthPercolated_m Then 'B층의 최대 증가 가능 높이는 토양심을 초과하지 않는다.
            '    '    .CumulativeInfiltrationF_tM1_m = 0 '이전시간에 침투된 모든 양이 B 층으로 침누된 경우이므로..
            '    'Else
            '    '    waterDepthPercolated_m = soilDepthPercolated_m * .effectivePorosityThetaE
            '    '    .CumulativeInfiltrationF_tM1_m = .CumulativeInfiltrationF_tM1_m - waterDepthPercolated_m
            '    '    If .CumulativeInfiltrationF_tM1_m < 0 Then .CumulativeInfiltrationF_tM1_m = 0
            '    'End If
            'End If


            'If .CumulativeInfiltrationF_tM1_m > .SoilDepthEffectiveAsWaterDepth_m Then _
            '   .CumulativeInfiltrationF_tM1_m = .SoilDepthEffectiveAsWaterDepth_m
            If (.FlowType = cGRM.CellFlowType.ChannelFlow AndAlso
            .mStreamAttr.ChStrOrder > project.SubWSPar.userPars(.WSID).dryStreamOrder) OrElse
            .LandCoverCode = cSetLandcover.LandCoverCode.WATR OrElse
            .LandCoverCode = cSetLandcover.LandCoverCode.WTLD Then
                '이조건에서는 항상 포화상태, 침누있음, 강우에 의한 침투량 없음. 대신 지표면 저류량에 의한 침투는 항상 있음
                cInfiltration.SetWaterAreaInfiltrationParameters(project.CV(cvan))
            Else
                If .soilDepth_m = soilDepthPercolated_m Then 'B층의 최대 증가 가능 높이는 토양심을 초과하지 않는다.
                    .soilWaterContent_tM1_m = 0 '이전시간에 침투된 모든 양이 B 층으로 침누된 경우이므로..
                Else
                    waterDepthPercolated_m = soilDepthPercolated_m * .effectivePorosityThetaE
                    .soilWaterContent_tM1_m = .soilWaterContent_tM1_m - waterDepthPercolated_m
                    If .soilWaterContent_tM1_m < 0 Then .soilWaterContent_tM1_m = 0
                End If
            End If
            .hUAQfromBedrock_m = .hUAQfromBedrock_m + soilDepthPercolated_m
            If .hUAQfromBedrock_m > (.SoilDepthToBedrock_m - .soilDepth_m) Then _
                .hUAQfromBedrock_m = (.SoilDepthToBedrock_m - .soilDepth_m)
            If .FlowType = cGRM.CellFlowType.OverlandFlow Then 'overland flow 셀에 대해서만 b 층의 횡방향 이동 해석
                Call GetBaseflowInputDepthAndCalculateLateralMovement(project, cvan, .CVDeltaX_m, cellSize_m, dtsec)
            Else
                If .mStreamAttr.hCVch_i_j > 0 Then
                    '하천 수심보다 더 많은 양이 침누되었을 경우, 하천 수심만큼만 침누된 것으로 한다.
                    Dim dHinUAQ_m As Single = soilDepthPercolated_m * .porosityEta
                    If dHinUAQ_m > .mStreamAttr.hCVch_i_j Then dHinUAQ_m = .mStreamAttr.hCVch_i_j
                    .hUAQfromChannelBed_m = .hUAQfromChannelBed_m + dHinUAQ_m
                    If .hUAQfromChannelBed_m > 0 Then
                        '기저유출은 물이 차있는 영역에서 발생하므로.. 포화수리전도도 적용
                        If .hUAQfromChannelBed_m > .mStreamAttr.hCVch_i_j Then
                            csa = CSng(.hydraulicConductK_mPsec *
                                                                        (.hUAQfromChannelBed_m - .mStreamAttr.hCVch_i_j) / .mStreamAttr.hCVch_i_j) _
                                                                    * .mStreamAttr.ChBaseWidth * dtsec
                        Else
                            csa = .hydraulicConductK_mPsec * (.hUAQfromChannelBed_m - .mStreamAttr.hCVch_i_j) * dtsec
                        End If
                    Else
                        csa = 0
                    End If
                Else
                    csa = 0
                End If
                .hUAQfromChannelBed_m = .hUAQfromChannelBed_m - csa / .mStreamAttr.ChBaseWidth / .porosityEta
                .hUAQfromBedrock_m = .hUAQfromBedrock_m - csa / .mStreamAttr.ChBaseWidth / .porosityEta
                If .hUAQfromChannelBed_m < 0 Then .hUAQfromChannelBed_m = 0
                If .hUAQfromBedrock_m < 0 Then .hUAQfromBedrock_m = 0
            End If
        End With
        Return csa
    End Function
End Class
