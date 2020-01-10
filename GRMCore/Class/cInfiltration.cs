using System;

namespace GRMCore
{
    public class cInfiltration
    {
        /// <summary>
        ///   침투를 계산하고, 유효강우량을 산정
        ///   </summary>
        ///   <param name="cvan"></param>
        ///   <param name="RFintervalSEC"></param>
        ///   <param name="DtSEC"></param>
        ///   <remarks></remarks>
        public void CalEffectiveRainfall(cProject project, int cvan, int rfIntervalSEC, int dtSEC)
        {
            cCVAttribute cv = project.CVs[cvan];
            if (project.generalSimulEnv.mbSimulateInfiltration == false)
            {
                SetNoInfiltrationParameters(cv);
                return;
            }
            if ((cv.FlowType == cGRM.CellFlowType.ChannelFlow) && (cv.mStreamAttr.ChStrOrder > project.subWSPar.userPars[cv.WSID].dryStreamOrder))
            {
                SetWaterAreaInfiltrationParameters(cv);
                return;
            }
            if (cv.LandCoverCode == cSetLandcover.LandCoverCode.WATR || cv.LandCoverCode == cSetLandcover.LandCoverCode.WTLD)
            {
                SetWaterAreaInfiltrationParameters(cv);
                return;
            }
            if (cv.ImperviousRatio == 1)
            {
                SetNoInfiltrationParameters(cv);
                return;
            }

            double CONSTGreenAmpt;
            double residualMoistContentThetaR;
            //cCVAttribute cv = project.CVs[cvan];
            cv.soilSaturationRatio = GetSoilSaturationRaito(cv.soilWaterContent_tM1_m, cv.SoilDepthEffectiveAsWaterDepth_m, cv.FlowType);
            if (cv.soilSaturationRatio > 0.99 || (project.mSimulationType == cGRM.SimulationType.SingleEventPE_SSR && cv.soilSaturationRatio == 1))
            {
                if (cv.soilSaturationRatio == 1)
                    cv.soilWaterContent_m = cv.SoilDepthEffectiveAsWaterDepth_m;// .CumulativeInfiltrationF_tM1_m
                cv.bAfterSaturated = true;
                cv.InfiltrationF_mPdt = 0;
                cv.InfiltrationRatef_mPsec = 0;
                cv.EffRFCV_dt_meter = cv.RFApp_dt_meter;
                cv.soilWaterContent_m = cv.soilWaterContent_tM1_m;
            }
            else
            {
                residualMoistContentThetaR = cv.porosityEta - cv.effectivePorosityThetaE;
                if (residualMoistContentThetaR < 0)
                { residualMoistContentThetaR = 0; }
                cv.EffectiveSaturationSe = (cv.porosityEta * cv.InitialSaturation - residualMoistContentThetaR) / (cv.porosityEta - residualMoistContentThetaR);

                if (cv.EffectiveSaturationSe < 0)
                { cv.EffectiveSaturationSe = 0; }
                cv.SoilMoistureChangeDeltaTheta = (1 - cv.EffectiveSaturationSe) * cv.effectivePorosityThetaE;
                CONSTGreenAmpt = cv.SoilMoistureChangeDeltaTheta * cv.wettingFrontSuctionHeadPsi_m;
                double infiltrationF_mPdt_max;
                bool beingPonding = false;
                if (cv.InfiltrationRatef_tM1_mPsec >= cv.RFReadintensity_tM1_mPsec)
                // 이전 시간에서의 침투률이 이전 시간에서의 강우강도보다 컸다면, 모든 강우는 침투됨
                { infiltrationF_mPdt_max = cv.RFApp_dt_meter; }
                else
                {
                    // 이전 시간에서의 침투률이 이전 시간에서의 강우강도보다 같거나 작았다면 ponding이 발생한 경우
                    infiltrationF_mPdt_max = GetInfiltrationForDtAfterPonding(dtSEC, cvan, CONSTGreenAmpt, cv.hydraulicConductK_mPsec);
                    beingPonding = true;
                }
                cv.InfiltrationF_mPdt = WaterDepthCanBeInfiltrated(cv.soilWaterContent_tM1_m, cv.SoilDepthEffectiveAsWaterDepth_m, infiltrationF_mPdt_max);
                // 누가 침투량으로 dt 동안에 추가된 침투량을 더한다.
                cv.soilWaterContent_m = cv.soilWaterContent_tM1_m + cv.InfiltrationF_mPdt;
                // 현재까지의 누가 침투량을 이용해서 이에 대한 포텐셜 침투률을 계산한다.
                if (cv.soilWaterContent_m <= 0)
                {
                    cv.InfiltrationRatef_mPsec = 0;
                    cv.soilWaterContent_m = 0;
                }
                else
                { cv.InfiltrationRatef_mPsec = cv.hydraulicConductK_mPsec * (1 + CONSTGreenAmpt / cv.soilWaterContent_m); }

                // 이경우에는 침투는 있지만.. 강우는 모두 직접 유출.. 침투는 지표면 저류 상태에서 발생
                if (cv.bAfterSaturated == true && (beingPonding == true || cv.soilSaturationRatio > 0.99))
                { cv.EffRFCV_dt_meter = cv.RFApp_dt_meter; }
                else
                {
                    double effRF_dt_m = cv.RFApp_dt_meter - cv.InfiltrationF_mPdt;
                    if (effRF_dt_m > cv.RFApp_dt_meter)
                    { effRF_dt_m = cv.RFApp_dt_meter; }
                    if (effRF_dt_m < 0)
                    { effRF_dt_m = 0; }
                    cv.EffRFCV_dt_meter = effRF_dt_m;
                    if (cv.ImperviousRatio < 1)
                    {
                        cv.EffRFCV_dt_meter = cv.EffRFCV_dt_meter * (1 - cv.ImperviousRatio) + cv.RFApp_dt_meter * cv.ImperviousRatio;
                        if (cv.EffRFCV_dt_meter > cv.RFApp_dt_meter)
                            cv.EffRFCV_dt_meter = cv.RFApp_dt_meter;
                    }
                }
            }
            // 유효강우량의 계산이 끝났으므로, 현재까지 계산된 침투, 강우강도 등을 tM1 변수로 저장한다.
            //cCVAttribute cv = project.CVs[cvan];
            cv.soilSaturationRatio = GetSoilSaturationRaito(cv.soilWaterContent_m, cv.SoilDepthEffectiveAsWaterDepth_m, cv.FlowType);
            if (cv.soilSaturationRatio == 1)
            { cv.bAfterSaturated = true; }
            cv.soilWaterContent_tM1_m = cv.soilWaterContent_m;
            cv.InfiltrationRatef_tM1_mPsec = cv.InfiltrationRatef_mPsec;
            cv.RFReadintensity_tM1_mPsec = cv.RFReadintensity_mPsec;
        }



        /// <summary>
        ///   Ponding 발생 후의 침투계산
        ///   </summary>
        ///   <param name="DtSEC"></param>
        ///   <param name="cvan"></param>
        ///   <param name="CONSTGreenAmpt"></param>
        ///   <remarks>
        ///   여기서는 누가 침투량과, 침투률을 계산한다. -> 침투률은 향후 강우강도와 비교하여 유효 강우량 계산에 이용됨.
        ///   여기서 계산되는 침투량과 침투률은 , j=1에서 적용하기 위한 t=0에서 부터 dt 시간동안의 잠재침투량과 잠재 침투률이다.
        ///   t=2 부터는 t=t-1에서 계산된 실제 누가침투량과 그에 따른 잠재 침투률을 계산한다.
        ///   </remarks>
        private double GetInfiltrationForDtAfterPonding(int dtSEC, int cvan, double CONSTGreenAmpt, double Kapp)
        {
            double CI_n;
            double CI_nP1;
            double err;
            double conCriteria;
            double Fx;
            double dFx;
            double constCI_tm1 = 0.0;
            cCVAttribute cv = cProject.Current.CVs[cvan];
            CI_n = cv.soilWaterContent_tM1_m + cv.InfiltrationRatef_tM1_mPsec * dtSEC;
            constCI_tm1 = cv.soilWaterContent_tM1_m;
            for (int intiterlationN = 0; intiterlationN < 20000; intiterlationN++)
            {
                // Newton-Raphson
                Fx = CI_n - constCI_tm1 - Kapp * dtSEC
                        - CONSTGreenAmpt * Math.Log((CI_n + CONSTGreenAmpt)
                                                    / (cv.soilWaterContent_tM1_m + CONSTGreenAmpt));
                dFx = 1 - CONSTGreenAmpt / (CI_n + CONSTGreenAmpt);
                CI_nP1 = CI_n - Fx / dFx;
                err = Math.Abs(CI_nP1 - CI_n);
                conCriteria = (CI_n * cGRM.CONST_TOLERANCE);
                if (err < conCriteria)
                {
                    double dF = CI_nP1 - constCI_tm1;
                    if (dF < 0)
                        dF = 0;
                    return dF;
                }
                CI_n = CI_nP1;
            }
            double dFr = CI_n - constCI_tm1;
            if (dFr < 0)
            { return 0; }
            else
            { return dFr; }
            //System.Console.WriteLine("[ColX=" + cProject.Current.CVs[cvan].XCol + ", RowY=" + cProject.Current.CVs[cvan].YRow + "] 에서 침투량이 수렴되지 않았습니다.   " + "\r\n" + "초기 침투률:" + cProject.Current.CVs[cvan].InfiltrationRatef_mPsec + "mm   ");
        }

        public void SetNoInfiltrationParameters(cCVAttribute cv)
        {
                cv.InfiltrationF_mPdt = 0;
                cv.InfiltrationRatef_mPsec = 0;
                cv.soilSaturationRatio = 0;
                cv.soilWaterContent_m = 0;
                cv.soilWaterContent_tM1_m = 0;
                cv.EffRFCV_dt_meter = cv.RFApp_dt_meter;
        }

        private double WaterDepthCanBeInfiltrated(double preDepth, double maxDepth, double maxINFILbeCalculated)
        {
            if (maxINFILbeCalculated <= 0) { return 0; }
            double dF = maxDepth - preDepth;
            if (dF < 0) { dF = 0; }
            if (dF < maxINFILbeCalculated) { maxINFILbeCalculated = dF; }
            return maxINFILbeCalculated;
        }

        public static void SetWaterAreaInfiltrationParameters(cCVAttribute cv)
        {
            cv.InfiltrationF_mPdt = 0;
            cv.InfiltrationRatef_mPsec = 0;
            // .CumulativeInfiltrationF_m = 0 '이건 침투된 깊이가 아니라, 실제로는 토양수분함량이다.
            // .CumulativeInfiltrationF_tM1_m = 0
            cv.soilSaturationRatio = 1;
            cv.EffRFCV_dt_meter = cv.RFApp_dt_meter;
        }

        public static double GetSoilSaturationRaito(double cumulativeInfiltration, double effSoilDepth, cGRM.CellFlowType flowType)

        {
            if (flowType == cGRM.CellFlowType.ChannelFlow) { return 1; }
            if (effSoilDepth <= 0) { return 1; }
            if (cumulativeInfiltration <= 0) { return 0; }
            double SSR = cumulativeInfiltration / effSoilDepth;
            if (SSR > 1) { return 1; }
            return SSR;
        }

        public static double GetSoilSaturationRaito(double curssr, double cumulativeInfiltration, double effSoilDepth, cGRM.CellFlowType flowType)
        {
            if (flowType == cGRM.CellFlowType.ChannelFlow)
            { return 1; }
            if (effSoilDepth <= 0)
            { return 1; }
            if (cumulativeInfiltration <= 0)
            { return 0; }
            return curssr;
        }


        public static double Kunsaturated(cCVAttribute cv)
        {
            // Dim ca As Single = 0.2 '0.24
            double ssr = GetSoilSaturationRaito(cv.soilSaturationRatio, cv.soilWaterContent_tM1_m, cv.SoilDepthEffectiveAsWaterDepth_m, cv.FlowType);
            // Dim ssr As Single = GetSoilSaturationRaito(cv.CumulativeInfiltrationF_tM1_m, cv.SoilDepthEffectiveAsWaterDepth_m, cv.FlowType)
            double Ks = cv.hydraulicConductK_mPsec;
            cGRM.UnSaturatedKType uKType = cv.UKType;
            double CoefUnsaturatedK = cv.coefUK;
            if (CoefUnsaturatedK <= 0)
            { return Ks * 0.1; }
            if (ssr > 0.99)
            { return Ks; }
            else
                switch (uKType)
                {
                    case cGRM.UnSaturatedKType.Linear:
                        {
                            return Ks * ssr * CoefUnsaturatedK;
                        }

                    case cGRM.UnSaturatedKType.Exponential:
                        {
                            return Ks * Math.Pow(ssr, CoefUnsaturatedK);
                        }

                    case cGRM.UnSaturatedKType.Constant:
                        {
                            return Ks * CoefUnsaturatedK;
                        }

                    default:
                        {
                            return Ks * ssr * CoefUnsaturatedK;
                        }
                }
        }
    }
}
