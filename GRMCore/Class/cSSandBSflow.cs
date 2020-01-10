using System;

namespace GRMCore
{
    public class cSSandBSflow
    {
        /// <summary>
        ///   상류셀에서 들어오는 ssf에 의한 현재 셀의 overland flow 기여분 계산. 반환 m2
        ///   현재 셀에서의 지표하 유출 계산
        ///   </summary>
        ///   <param name="cvan"></param>
        ///   <param name="dtSEC"></param>
        ///   <param name="Dy_m"></param>
        ///   <returns></returns>
        ///   <remarks></remarks>
        public double GetRFlowOFAreaAddedBySSflowCVwAndCalSSflowAtNowCV(cCVAttribute[] CVs, int cvan, int dtSEC, double Dy_m, double Dx_m)
        {
            {
                cCVAttribute cv = CVs[cvan];
                // todo : 지표하 유출관련.. 포화되지 않아도 기여, 2013.05.20
                double SSFfromCVw;
                double SSFAddedToMeFromCVw_m;
                double RflowBySSFfromCVw_m2; // 이건 return flow
                SSFfromCVw = GetTotalSSFfromCVwOFcell_m3Ps(CVs, cvan) * dtSEC / (Dy_m * Dx_m);

                // 상류의 지표하 유출에 의한 현재 셀의 rf 기여분 계산
                if (cInfiltration.GetSoilSaturationRaito(cv.soilWaterContent_tM1_m, cv.SoilDepthEffectiveAsWaterDepth_m, cv.FlowType) >= 0.99)
                {
                    RflowBySSFfromCVw_m2 = SSFfromCVw * Dy_m;
                    SSFAddedToMeFromCVw_m = 0;
                }
                else
                {
                    RflowBySSFfromCVw_m2 = SSFfromCVw * cv.soilSaturationRatio * Dy_m;
                    SSFAddedToMeFromCVw_m = SSFfromCVw * (1 - cv.soilSaturationRatio);
                }

                // 상류에서 유입된 지표하 유출 깊이 더하고..
                cv.soilWaterContent_tM1_m = cv.soilWaterContent_tM1_m + SSFAddedToMeFromCVw_m;

                // 현재의 침투 깊이를 이용해서 rf 계산하고..

                if (cv.soilWaterContent_tM1_m > cv.SoilDepthEffectiveAsWaterDepth_m)
                {
                    // 하류로 이송된 지표하 유출량이 현재 셀의 포화가능 깊이를 초과하면, 초과분은 returnflow
                    RflowBySSFfromCVw_m2 = (cv.soilWaterContent_tM1_m - cv.SoilDepthEffectiveAsWaterDepth_m) * Dy_m
                                            + RflowBySSFfromCVw_m2;
                    cv.soilWaterContent_tM1_m = cv.SoilDepthEffectiveAsWaterDepth_m;
                }

                if (cv.soilWaterContent_tM1_m < 0)
                {
                    cv.soilWaterContent_tM1_m = 0;
                }
                // 이건 현재 셀의 지표하 유출. 지표하 유출이 발생하는 부분은 물이 차있는 부분이므로.. 포화수리전도도 적용
                cv.SSF_Q_m3Ps = cv.soilWaterContent_tM1_m / cv.porosityEta
                          * cv.hydraulicConductK_mPsec * Math.Sin(Math.Atan(cv.SlopeOF)) * Dy_m;

                // 이건 하류로 유출한 이후의 현재 누가침투심
                cv.soilWaterContent_tM1_m = cv.soilWaterContent_tM1_m
                        - cv.SSF_Q_m3Ps / (Dy_m * cv.CVDeltaX_m) * dtSEC;
                return RflowBySSFfromCVw_m2; // + .subSurfaceflow_Q_m3Ps
            }
        }


        public double GetTotalSSFfromCVwOFcell_m3Ps(cCVAttribute[] CVs, int cvan)
        {
            double cumulativeSSF_m3Ps = 0;
            foreach (int objCellid in CVs[cvan].NeighborCVidFlowIntoMe)
            {
                if (CVs[objCellid - 1].FlowType == cGRM.CellFlowType.OverlandFlow)
                // 즉, GRM에서 Green-Ampt 모형을 이용해서 침투된 양을 계산할때, 포화된 깊이로 계산됨. 즉, 현재 침투률로 얼만큼 깊이 들어갔는지는 계산하지 않는다..
                // 따라서, 하도로 기여하는 지표하 유출량 계산할때는 누가침투깊이가 .saturatedSoildepth_m 이 된다.
                {
                    cumulativeSSF_m3Ps = cumulativeSSF_m3Ps + CVs[objCellid - 1].SSF_Q_m3Ps;
                }
            }
            return cumulativeSSF_m3Ps;
        }

        /// <summary>
        ///  지표하 유출을 하폭이 격자크기보다 큰 경우의 셀에 대해서 모의 2009.09.12
        ///  하도셀에서는 침투가 없으므로, 하도셀 자체에서 dunne type flow는 발생하지 않는다.
        ///  상류셀에서 발생된 dunne type flow는 상류셀의 overlandflow 유출 계산에서 반영되어 있음.
        ///  m2, dt 시간에 대해서 평균된 면적을 반환함.
        ///   </summary>
        ///   <param name="cvan"></param>
        ///   <returns></returns>
        ///   <remarks></remarks>
        public double GetChCSAaddedBySSFlowInChlCell(cCVAttribute[] CVs, int cvan)
        {
            double cumulativeSSFlow_cms = 0;
            cumulativeSSFlow_cms = GetTotalSSFfromCVwOFcell_m3Ps(CVs, cvan);
            if (cumulativeSSFlow_cms > 0)
            {
                // todo : 여기 발산 가능성 있음.
                if (CVs[cvan].mStreamAttr.uCVch_i_j > 0)
                {
                    double chCSA;
                    chCSA = cumulativeSSFlow_cms / (double)CVs[cvan].mStreamAttr.uCVch_i_j; // 이전 시간에 계산된 유속

                    // 유속이 작을 경우.. 발산 가능성 있으므로.. 기존 단면적 보다 큰 단면적으로 유입될 경우.. 
                    // 기존 단면적으로 유입되는 것으로 설정..
                    // 즉, 지표하 유출의 기여 단면적은 기존 하천단면적보다 클수 없음.
                    if (chCSA > CVs[cvan].mStreamAttr.CSAch_i_j)
                        chCSA = CVs[cvan].mStreamAttr.CSAch_i_j;
                    return chCSA;
                }
            }
            // 이경우에는 하도에 기여되지 않는 것으로 가정
            // 즉 강우에 의한 하도의 유량 발생이 시작되지 않았는데(하도가 말라있을 경우).. 
            // 지표하 유출에 의해 기여가 있을 수 없음.
            return 0;
        }



        // 지표하 유출을 하폭이 격자크기보다 작은 경우의 셀에 대해서 모의 2009.09.12
        public double GetChCSAaddedBySSFlowInChNOfCell(cCVAttribute[] CVs, int cvan)
        {
            // 현재 셀의 overlandflow 구간에서 발생된 subsurface flow 유량을 받는다.
            if (CVs[cvan].SSF_Q_m3Ps > 0)
            {
                // todo : 여기 발산 가능성 있음.
                if (CVs[cvan].mStreamAttr.uCVch_i_j > 0)
                {
                    double chCSA;
                    chCSA = CVs[cvan].SSF_Q_m3Ps / (double)CVs[cvan].mStreamAttr.uCVch_i_j;
                    // 유속이 작을 경우.. 발산 가능성 있으므로.. 기존 단면적 보다 큰 단면적으로 유입될 경우.. 기존 단면적으로 유입되는 것으로 설정..
                    // 즉, 지표하 유출의 기여 단면적은 기존 하천단면적보다 클수 없음.
                    if (chCSA > CVs[cvan].mStreamAttr.CSAch_i_j)
                    {
                        chCSA = CVs[cvan].mStreamAttr.CSAch_i_j;
                    }
                    return chCSA;
                }
            }
            // 이경우에는 하도에 기여되지 않는 것으로 가정
            // 즉 강우에 의한 하도의 유량 발생이 시작되지 않았는데(하도가 말라있을 경우).. 지표하 유출에 의해 기여가 있을 수 없음.
            return 0;
        }



        /// <summary>
        ///   상류로부터 유입된 기저유출에 의한 현재 검사체적의 지하수위 상승분과 기저유출을 계산
        ///   </summary>
        ///   <param name="cvan"></param>
        ///   <param name="DX_m"></param>
        ///   <param name="DY_m"></param>
        ///   <param name="DTSec"></param>
        ///   <remarks></remarks>
        public void GetBaseflowInputDepthAndCalculateLateralMovement(cCVAttribute[] CVs, int cvan, int FacMin, double dX_m, double dY_m, double dTSec)
        {
            double cumulBFq_m3Ps = 0;
            double dhUAQ_m;
            foreach (int cvid in CVs[cvan].NeighborCVidFlowIntoMe)
            {
                if (CVs[cvid - 1].FlowType == cGRM.CellFlowType.OverlandFlow)
                {
                    cumulBFq_m3Ps = cumulBFq_m3Ps + CVs[cvid - 1].baseflow_Q_m3Ps;// 수두경사가 셀의 지표면 경사와 같은 것으로 가정
                }
            }

            if (CVs[cvan].FAc == FacMin)
            {
                dX_m = dX_m / (double)2;
            }
            // sngDhUAQ_m = sngCumulativeBFq_m3Ps * intDTSec / (sngDeltaY_m * sngDeltaX_m)
            dhUAQ_m = cumulBFq_m3Ps * dTSec / (dY_m * dX_m) / CVs[cvan].effectivePorosityThetaE;  // 이건 검사체적에 균질하게 퍼져 있는 수위변화분
            CVs[cvan].hUAQfromBedrock_m = CVs[cvan].hUAQfromBedrock_m + dhUAQ_m; // 이건 지하수대의 토양 깊이 변화
            if (CVs[cvan].hUAQfromBedrock_m > (CVs[cvan].SoilDepthToBedrock_m - CVs[cvan].soilDepth_m))
            {
                CVs[cvan].hUAQfromBedrock_m = CVs[cvan].SoilDepthToBedrock_m - CVs[cvan].soilDepth_m;
            }
            // 기저유출 계산에서는 포화수리전도도 적용. 포화된 영역에서 발생
            CVs[cvan].baseflow_Q_m3Ps = CVs[cvan].hUAQfromBedrock_m * CVs[cvan].hydraulicConductK_mPsec * Math.Sin(Math.Atan(CVs[cvan].SlopeOF)) * dY_m;
            if (CVs[cvan].baseflow_Q_m3Ps < 0)
            {
                CVs[cvan].baseflow_Q_m3Ps = 0;
            }
        }


        public double CalBFlowAndGetCSAAddedByBFlow(cProject project, int cvan, int dtsec, double cellSize_m)
        {
            double deltaSoilDepthofUAQ_m; // B층(비피압 대수층)의 높이 변화. A층이 포화될 경우, B층으로 침누되면서 발생, 연직방향 침투깊이
            double soilDepthPercolated_m;
            double waterDepthPercolated_m;
            double csa = 0;
            deltaSoilDepthofUAQ_m = 0;
            cCVAttribute cv = project.CVs[cvan];
            // 토양의 포화 상태와 상관없이 침누가 발생한다. 토양포화도가 상승 및 하강한다. 포화도가 0 이면 침누 발생 안한다.
            if (cv.soilSaturationRatio > 0)
            { soilDepthPercolated_m = cInfiltration.Kunsaturated(cv) * dtsec; }
            else
            { soilDepthPercolated_m = 0; }

            if (cv.soilDepth_m < soilDepthPercolated_m)
            { soilDepthPercolated_m = cv.soilDepth_m; }
            // If .FlowType <> cGRM.CellFlowType.ChannelFlow AndAlso
            // project.CVs[cvan].LandCoverCode <> cSetLandcover.LandCoverCode.WATR AndAlso
            // project.CVs[cvan].LandCoverCode <> cSetLandcover.LandCoverCode.WTLD Then 'channel flow 혹은 수역에서는 토양수분함량 변하지 않는다.
            // 'If .soilDepth_m = soilDepthPercolated_m Then 'B층의 최대 증가 가능 높이는 토양심을 초과하지 않는다.
            // '    .CumulativeInfiltrationF_tM1_m = 0 '이전시간에 침투된 모든 양이 B 층으로 침누된 경우이므로..
            // 'Else
            // '    waterDepthPercolated_m = soilDepthPercolated_m * .effectivePorosityThetaE
            // '    .CumulativeInfiltrationF_tM1_m = .CumulativeInfiltrationF_tM1_m - waterDepthPercolated_m
            // '    If .CumulativeInfiltrationF_tM1_m < 0 Then .CumulativeInfiltrationF_tM1_m = 0
            // 'End If
            // End If


            // If .CumulativeInfiltrationF_tM1_m > .SoilDepthEffectiveAsWaterDepth_m Then _
            // .CumulativeInfiltrationF_tM1_m = .SoilDepthEffectiveAsWaterDepth_m
            if ((cv.FlowType == cGRM.CellFlowType.ChannelFlow && cv.mStreamAttr.ChStrOrder > project.subWSPar.userPars[cv.WSID].dryStreamOrder)
                || cv.LandCoverCode == cSetLandcover.LandCoverCode.WATR || cv.LandCoverCode == cSetLandcover.LandCoverCode.WTLD)
            // 이조건에서는 항상 포화상태, 침누있음, 강우에 의한 침투량 없음. 대신 지표면 저류량에 의한 침투는 항상 있음
            {
                cInfiltration.SetWaterAreaInfiltrationParameters(project.CVs[cvan]);
            }
            else if (cv.soilDepth_m == soilDepthPercolated_m)
            { cv.soilWaterContent_tM1_m = 0; } // 이전시간에 침투된 모든 양이 B 층으로 침누된 경우이므로..
            else
            {
                waterDepthPercolated_m = soilDepthPercolated_m * cv.effectivePorosityThetaE;
                cv.soilWaterContent_tM1_m = cv.soilWaterContent_tM1_m - waterDepthPercolated_m;
                if (cv.soilWaterContent_tM1_m < 0)
                {
                    cv.soilWaterContent_tM1_m = 0;
                }
            }
            cv.hUAQfromBedrock_m = cv.hUAQfromBedrock_m + soilDepthPercolated_m;
            if (cv.hUAQfromBedrock_m > (cv.SoilDepthToBedrock_m - cv.soilDepth_m))
            {
                cv.hUAQfromBedrock_m = (cv.SoilDepthToBedrock_m - cv.soilDepth_m);
            }
            if (cv.FlowType == cGRM.CellFlowType.OverlandFlow)
            {
                GetBaseflowInputDepthAndCalculateLateralMovement(project.CVs, cvan,  project.FacMin , cv.CVDeltaX_m, cellSize_m, dtsec);
            }
            else
            {
                if (cv.mStreamAttr.hCVch_i_j > 0)
                {
                    // 하천 수심보다 더 많은 양이 침누되었을 경우, 하천 수심만큼만 침누된 것으로 한다.
                    double dHinUAQ_m = soilDepthPercolated_m * cv.porosityEta;
                    if (dHinUAQ_m > cv.mStreamAttr.hCVch_i_j)
                    {
                        dHinUAQ_m = cv.mStreamAttr.hCVch_i_j;
                    }
                    cv.hUAQfromChannelBed_m = cv.hUAQfromChannelBed_m + dHinUAQ_m;
                    if (cv.hUAQfromChannelBed_m > 0)
                    {
                        // 기저유출은 물이 차있는 영역에서 발생하므로.. 포화수리전도도 적용
                        if (cv.hUAQfromChannelBed_m > cv.mStreamAttr.hCVch_i_j)
                        {
                            csa = cv.hydraulicConductK_mPsec * (cv.hUAQfromChannelBed_m - cv.mStreamAttr.hCVch_i_j)
                                / cv.mStreamAttr.hCVch_i_j * cv.mStreamAttr.ChBaseWidth * dtsec;
                        }
                        else
                        {
                            csa = cv.hydraulicConductK_mPsec * (cv.hUAQfromChannelBed_m - cv.mStreamAttr.hCVch_i_j) * dtsec;
                        }
                    }
                    else
                    { csa = 0; }
                }
                else
                { csa = 0; }
                cv.hUAQfromChannelBed_m = cv.hUAQfromChannelBed_m - csa / (double)cv.mStreamAttr.ChBaseWidth / (double)cv.porosityEta;
                cv.hUAQfromBedrock_m = cv.hUAQfromBedrock_m - csa / (double)cv.mStreamAttr.ChBaseWidth / (double)cv.porosityEta;
                if (cv.hUAQfromChannelBed_m < 0)
                {
                    cv.hUAQfromChannelBed_m = 0;
                }
                if (cv.hUAQfromBedrock_m < 0)
                {
                    cv.hUAQfromBedrock_m = 0;
                }
            }
            return csa;
        }
    }
}
