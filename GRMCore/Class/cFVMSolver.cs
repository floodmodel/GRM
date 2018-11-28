using System;


namespace GRMCore
{
    public class cFVMSolver
    {
        public void CalculateOverlandFlow(cCVAttribute cv, double hCVw_i_jP1, double effDy_m)
        {
            double constHp_j = cv.hCVof_i_j;
            double CONST_DtPDx = System.Convert.ToInt32(sThisSimulation.dtsec) / (double)cv.CVDeltaX_m;
            double Hp_n = constHp_j;

            double hCVw_n = hCVw_i_jP1;
            double uCVw_n = 0;
            double cCVw_n = 0;
            if (hCVw_n > 0)
            {
                uCVw_n = GetFlowVelocityByManningEq(hCVw_n, cv.SlopeOF, cv.RoughnessCoeffOF);
                cCVw_n = uCVw_n * CONST_DtPDx * hCVw_n;
            }
            for (int iter = 0; iter < 20000; iter++)
            {
                double hCVe_n = Hp_n;
                double uCVe_n = GetFlowVelocityByManningEq(hCVe_n, cv.SlopeOF, cv.RoughnessCoeffOF);
                double cCVe_n = uCVe_n * CONST_DtPDx * hCVe_n;
                // Newton-Raphson
                double Fx = Hp_n - cCVw_n + cCVe_n - constHp_j;
                double dFx;
                dFx = 1 + (1.66667 * CONST_DtPDx
                         * Math.Pow((hCVe_n), (0.66667)) * (Math.Pow(cv.SlopeOF, 0.5)) /cv.RoughnessCoeffOF);
                double Hp_nP1 = (Hp_n - Fx / dFx);
                if (Hp_nP1 <= 0)
                {
                    SetNoFluxOverlandFlowCV(cv);
                    break;
                }
                double tolerance = Hp_n * cGRM.CONST_TOLERANCE;
                double err = Math.Abs(Hp_nP1 - Hp_n);
                if (err < tolerance)
                {
                    cv.hCVof_i_j = Hp_nP1;
                    cv.uCVof_i_j = GetFlowVelocityByManningEq(Hp_nP1, cv.SlopeOF, cv.RoughnessCoeffOF);
                    cv.CSAof_i_j = Hp_nP1 * effDy_m;
                    cv.QCVof_i_j_m3Ps = cv.CSAof_i_j * cv.uCVof_i_j;
                    break;
                }
                Hp_n = Hp_nP1;
            }

        }

        public void CalculateChannelFlow(cCVAttribute cv, double CSAchCVw_i_jP1)
        {
            double CONST_DtPDx = System.Convert.ToInt32(sThisSimulation.dtsec) / cv.CVDeltaX_m;
            double HRch;
            double CSPerCh;
            double hChCVw_i_jP1;
            double uCVw_n;
            double cChCVw_n;
            if (CSAchCVw_i_jP1 == 0)
            {
                hChCVw_i_jP1 = 0;
                HRch = 0;
                CSPerCh = 0;
                uCVw_n = 0;
                cChCVw_n = 0;
            }
            else
            {
                hChCVw_i_jP1 = GetChannelDepthUsingArea(cv.mStreamAttr.ChBaseWidth, CSAchCVw_i_jP1,
                    cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chUpperRBaseWidth_m,
                    cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chLowerRHeight, cv.mStreamAttr.mChBankCoeff);
                CSPerCh = GetChannelCrossSectionPerimeter(cv.mStreamAttr.ChBaseWidth,
                    cv.mStreamAttr.chSideSlopeRight, cv.mStreamAttr.chSideSlopeLeft, hChCVw_i_jP1,
                    cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chLowerRHeight,
                    cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chUpperRBaseWidth_m);
                HRch = CSAchCVw_i_jP1 / CSPerCh;
                uCVw_n = GetFlowVelocityByManningEq(HRch, cv.mStreamAttr.chBedSlope, cv.mStreamAttr.RoughnessCoeffCH);
                cChCVw_n = uCVw_n * CONST_DtPDx * CSAchCVw_i_jP1;
            }

            double constCSAchCVp_j = cv.mStreamAttr.CSAch_i_j;
            double CSAp_n = constCSAchCVp_j;
            double hChp_n = cv.mStreamAttr.hCVch_i_j;

            for (int iter = 0; iter < 20000; iter++)
            {
                double hChCVe_n = hChp_n;
                double CSAChCVe_n = CSAp_n;
                CSPerCh = GetChannelCrossSectionPerimeter(cv.mStreamAttr.ChBaseWidth, cv.mStreamAttr.chSideSlopeRight,
                    cv.mStreamAttr.chSideSlopeLeft, hChCVe_n, cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chLowerRHeight,
                    cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chUpperRBaseWidth_m);
                HRch = CSAChCVe_n / CSPerCh;
                double u_n = GetFlowVelocityByManningEq(HRch, cv.mStreamAttr.chBedSlope, cv.mStreamAttr.RoughnessCoeffCH);
                double cChCVe_n = u_n * CONST_DtPDx * CSAChCVe_n;
                // Newton-Raphson
                double Fx = CSAp_n - cChCVw_n + cChCVe_n - constCSAchCVp_j;
                double dFx;
                dFx = 1 + (1.66667 * Math.Pow(CSAChCVe_n, 0.66667)
                    * Math.Sqrt(cv.mStreamAttr.chBedSlope) * CONST_DtPDx /(cv.mStreamAttr.RoughnessCoeffCH * Math.Pow(CSPerCh, 0.66667)));
                double CSAch_nP1 = CSAp_n - Fx / dFx;
                if (CSAch_nP1 <= 0)
                {
                    SetNoFluxChannelFlowCV(cv);
                    break;
                }
                double Qn = u_n * CSAp_n;
                HRch = CSAch_nP1 / CSPerCh;
                double u_nP1 = GetFlowVelocityByManningEq(HRch, cv.mStreamAttr.chBedSlope, cv.mStreamAttr.RoughnessCoeffCH);
                double QnP1 = u_nP1 * CSAch_nP1;
                double tolerance = Qn * cGRM.CONST_TOLERANCE;
                double err = Math.Abs(Qn - QnP1);
                double hCh_nP1 = GetChannelDepthUsingArea(cv.mStreamAttr.ChBaseWidth, CSAch_nP1, 
                    cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chUpperRBaseWidth_m, 
                    cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chLowerRHeight, cv.mStreamAttr.mChBankCoeff);
                if (err < tolerance)
                {
                    cv.mStreamAttr.hCVch_i_j = hCh_nP1;
                    cv.mStreamAttr.uCVch_i_j = u_nP1;
                    cv.mStreamAttr.CSAch_i_j = CSAch_nP1;
                    cv.mStreamAttr.QCVch_i_j_m3Ps = QnP1;
                    break;
                }
                CSAp_n = CSAch_nP1;
                hChp_n = hCh_nP1;
            }
        }


        public double CalChCSA_CViW(cCVAttribute[] CVs, int cvan)
        {
            // w의 단면적 계산
            double CSAe_iM1 = 0;
            double CSAeSum_iM1 = 0;
            double qSumCViM1_m3Ps = 0;
            cCVAttribute cv = CVs[cvan];
            int effCellCountFlowToCV_iM1 = cv.NeighborCVidFlowIntoMe.Count;
            foreach (int cvid in cv.NeighborCVidFlowIntoMe)
            {
                double qCViM1_m3Ps = 0;
                if (CVs[cvid - 1].FlowType == cGRM.CellFlowType.OverlandFlow)
                {
                    CSAe_iM1 = CVs[cvid - 1].CSAof_i_j;
                    qCViM1_m3Ps = CVs[cvid - 1].QCVof_i_j_m3Ps;

                }
                else if (CVs[cvid - 1].FlowType == cGRM.CellFlowType.ChannelFlow || CVs[cvid - 1].FlowType == cGRM.CellFlowType.ChannelNOverlandFlow)
                {
                    CSAe_iM1 = CVs[cvid - 1].mStreamAttr.CSAch_i_j;
                    qCViM1_m3Ps = CVs[cvid - 1].mStreamAttr.QCVch_i_j_m3Ps;
                    if (CVs[cvid - 1].FlowType == cGRM.CellFlowType.ChannelNOverlandFlow)
                    {
                        CSAe_iM1 = CSAe_iM1 + CVs[cvid - 1].mStreamAttr.CSAchAddedByOFinCHnOFcell;
                        qCViM1_m3Ps = qCViM1_m3Ps + CVs[cvid - 1].QCVof_i_j_m3Ps;
                    }

                }
                qSumCViM1_m3Ps = qSumCViM1_m3Ps + qCViM1_m3Ps;
                CSAeSum_iM1 = CSAeSum_iM1 + CSAe_iM1;
                if (qCViM1_m3Ps <= 0.0)
                { effCellCountFlowToCV_iM1 = effCellCountFlowToCV_iM1 - 1; }
            }
            cv.QsumCVw_dt_m3 = cv.QsumCVw_dt_m3 + qSumCViM1_m3Ps * sThisSimulation.dtsec;

            if (effCellCountFlowToCV_iM1 < 1) { effCellCountFlowToCV_iM1 = 1; }
            cv.effCVCountFlowINTOCViW = effCellCountFlowToCV_iM1;

            if (CSAeSum_iM1 < cGRM.CONST_WET_AND_DRY_CRITERIA)
            {
                return 0;
            }

            double CSAw_n = CSAeSum_iM1;
            double CSAw_nP1 = 0;
            for (int iter = 0; iter < 100; iter++)
            {
                double hWn_i = GetChannelDepthUsingArea(cv.mStreamAttr.ChBaseWidth, CSAw_n,
                   cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chUpperRBaseWidth_m,
                   cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chLowerRHeight, cv.mStreamAttr.mChBankCoeff);
                double chCSPeri = GetChannelCrossSectionPerimeter(cv.mStreamAttr.ChBaseWidth, cv.mStreamAttr.chSideSlopeRight,
                   cv.mStreamAttr.chSideSlopeLeft, hWn_i, cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chLowerRHeight,
                   cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chUpperRBaseWidth_m);
                double Fx = Math.Pow(CSAw_n, 1.66667) * Math.Pow(cv.mStreamAttr.chBedSlope, 0.5)
                                                / (cv.mStreamAttr.RoughnessCoeffCH * Math.Pow(chCSPeri, 0.66667)) - qSumCViM1_m3Ps;
                double dFx = 1.66667 * Math.Pow(CSAw_n, 0.66667) * Math.Pow(cv.mStreamAttr.chBedSlope, 0.5)
                                               / (cv.mStreamAttr.RoughnessCoeffCH * Math.Pow(chCSPeri, 0.66667));
                CSAw_nP1 = CSAw_n - Fx / dFx;
                double err = Math.Abs(CSAw_nP1 - CSAw_n) / CSAw_n;
                if (err < cGRM.CONST_TOLERANCE)
                    return CSAw_nP1;
                CSAw_n = CSAw_nP1;
            }
            return CSAw_nP1;
        }


        public double CalChCSAFromQbyIteration(cCVAttribute cv, double CSAini, double Q_m3Ps)
        {
            if (Q_m3Ps <= 0) { return 0; }
            double CSA_nP1 = 0;
            double cbw = cv.mStreamAttr.ChBaseWidth;
            double bc = cv.mStreamAttr.mChBankCoeff;
            bool bCompound = cv.mStreamAttr.chIsCompoundCS;
            double hLR = cv.mStreamAttr.chLowerRHeight;
            double AreaLR = cv.mStreamAttr.chLowerRArea_m2;
            double bwUpperRegion = cv.mStreamAttr.chUpperRBaseWidth_m;
            double CSA_n;
            if (CSAini > 0)
                CSA_n = CSAini;
            else
                CSA_n = Q_m3Ps / cv.CVDeltaX_m;
            double Fx;
            double dFx;
            double ChCrossSecPer;
            for (int iter = 0; iter < 100; iter++)
            {
                double Hw_n = GetChannelDepthUsingArea(cbw, CSA_n, bCompound, bwUpperRegion, AreaLR, hLR, bc);
                ChCrossSecPer = GetChannelCrossSectionPerimeter(cbw, cv.mStreamAttr.chSideSlopeRight, cv.mStreamAttr.chSideSlopeLeft, Hw_n, bCompound, hLR, AreaLR, bwUpperRegion);
                Fx = Math.Pow(CSA_n, 1.66667) * Math.Sqrt(cv.mStreamAttr.chBedSlope)
                            / (cv.mStreamAttr.RoughnessCoeffCH * Math.Pow(ChCrossSecPer, 0.66667)) - Q_m3Ps;
                dFx = 1.66667 * Math.Pow(CSA_n, 0.66667) * Math.Sqrt(cv.mStreamAttr.chBedSlope)
                            / (cv.mStreamAttr.RoughnessCoeffCH * Math.Pow(ChCrossSecPer, 0.66667));
                CSA_nP1 = CSA_n - Fx / dFx;
                double toler = CSA_n * cGRM.CONST_TOLERANCE;
                double err = Math.Abs(CSA_nP1 - CSA_n);
                if (err < toler)
                    return CSA_nP1;
                CSA_n = CSA_nP1;
            }
            return CSA_nP1;
        }

        /// <summary>
        /// 유량 보전 연속방정식을 적용하여 새로 계산된 w의 수위를 반환. 최상류셀은 계산하지 않는다
        /// </summary>
        /// <param name="cvan"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public double CalculateOverlandWaterDepthCViW(cProject project, int cvan)
        {
            double qSumToCViM1 = 0;
            double qCViM1;
            double qWn_i;
            int effCellCountFlowToCViW;
            effCellCountFlowToCViW = project.CVs[cvan].NeighborCVidFlowIntoMe.Count;
            foreach (int cvid in project.CVs[cvan].NeighborCVidFlowIntoMe)
            {
                qCViM1 = project.CVs[cvid - 1].QCVof_i_j_m3Ps / project.watershed.mCellSize; // 단위폭당 유량
                if (qCViM1 <= 0.0)
                {
                    effCellCountFlowToCViW =effCellCountFlowToCViW - 1;
                    qCViM1 = 0;
                }
                qSumToCViM1 = qSumToCViM1 + qCViM1;
                project.CVs[cvan].QsumCVw_dt_m3 = project.CVs[cvan].QsumCVw_dt_m3
                    + project.CVs[cvid - 1].QCVof_i_j_m3Ps * sThisSimulation.dtsec;
            }
            if (effCellCountFlowToCViW < 1)
                effCellCountFlowToCViW = 1;
            project.CVs[cvan].effCVCountFlowINTOCViW = effCellCountFlowToCViW;
            qWn_i = Math.Pow(project.CVs[cvan].RoughnessCoeffOF * qSumToCViM1 / Math.Sqrt(project.CVs[cvan].SlopeOF), 0.6);
            return qWn_i;
        }

        public double GetFlowVelocityByManningEq(double hydraulicRaidus, double slope, double nCoeff)
        {
            double vManning = Math.Pow(hydraulicRaidus, 0.66667) * Math.Sqrt(slope) / nCoeff;
            return vManning;
        }



        /// <summary>
        /// 하도의 흐름 단면
        /// </summary>
        /// <param name="lowerRegionBaseWidth"></param>
        /// <param name="chBankConst"></param>
        /// <param name="crossSectionDepth"></param>
        /// <param name="isCompoundCrossSection"></param>
        /// <param name="lowerRegionHeight"></param>
        /// <param name="lowerRegionArea"></param>
        /// <param name="upperRegionBaseWidth"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public double GetChannelCrossSectionAreaUsingChannelFlowDepth(double lowerRegionBaseWidth, double chBankConst, double crossSectionDepth, bool isCompoundCrossSection, double lowerRegionHeight, double lowerRegionArea, double upperRegionBaseWidth)
        {
            if ((isCompoundCrossSection == true))
            {
                if ((crossSectionDepth > lowerRegionHeight))
                {
                    double highFlowDepth = crossSectionDepth - lowerRegionHeight;
                    if (highFlowDepth < 0)
                        highFlowDepth = 0;
                    double sngHighFlowArea = (upperRegionBaseWidth + chBankConst * highFlowDepth / (double)2) * highFlowDepth;
                    return (lowerRegionArea + sngHighFlowArea);
                }
            }
            return (lowerRegionBaseWidth + chBankConst * crossSectionDepth / (double)2) * crossSectionDepth;
        }

        /// <summary>
        /// 윤변계산
        /// </summary>
        /// <param name="lowerRegionBaseWidth"></param>
        /// <param name="sideSlopeRightBank"></param>
        /// <param name="sideSlopeLeftBank"></param>
        /// <param name="crossSectionDepth"></param>
        /// <param name="isCompoundCrossSection"></param>
        /// <param name="lowerRegionHeight"></param>
        /// <param name="lowerRegionArea"></param>
        /// <param name="upperRegionBaseWidth"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public double GetChannelCrossSectionPerimeter(double lowerRegionBaseWidth, double sideSlopeRightBank, 
            double sideSlopeLeftBank, double crossSectionDepth, bool isCompoundCrossSection, 
            double lowerRegionHeight, double lowerRegionArea, double upperRegionBaseWidth)
        {
            if (isCompoundCrossSection == true)
            {
                if (crossSectionDepth > lowerRegionHeight)
                {
                    double lowFlowPerimeter = lowerRegionBaseWidth + Math.Sqrt(Math.Pow(lowerRegionHeight, 2) + Math.Pow(lowerRegionHeight /sideSlopeRightBank, 2))
                        + Math.Sqrt(Math.Pow(lowerRegionHeight, 2) + Math.Pow(lowerRegionHeight / sideSlopeLeftBank, 2));
                    double highFlowDepth = crossSectionDepth - lowerRegionHeight;
                    double highFlowPerimeter = Math.Sqrt(Math.Pow(highFlowDepth, 2) + Math.Pow(highFlowDepth /sideSlopeRightBank, 2))
                                              + Math.Sqrt(Math.Pow(highFlowDepth, 2) + Math.Pow(highFlowDepth / sideSlopeLeftBank, 2)) + upperRegionBaseWidth - lowerRegionBaseWidth;
                    return (lowFlowPerimeter + highFlowPerimeter);
                }
            }

            return lowerRegionBaseWidth + Math.Sqrt(Math.Pow(crossSectionDepth, 2) + Math.Pow(crossSectionDepth / sideSlopeRightBank, 2))
                                                      + Math.Sqrt(Math.Pow(crossSectionDepth, 2) + Math.Pow(crossSectionDepth / sideSlopeLeftBank, 2));
        }


        /// <summary>
        /// 근의 공식으로 하도 단면의 수심 계산
        /// </summary>
        /// <param name="baseWidthLowerRegion"></param>
        /// <param name="chCSAinput"></param>
        /// <param name="bIsCompoundCrossSection"></param>
        /// <param name="baseWidthUpperRegion"></param>
        /// <param name="dblLowerRegionArea"></param>
        /// <param name="lowerRegionHeight"></param>
        /// <param name="chBankConst"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public double GetChannelDepthUsingArea(double baseWidthLowerRegion, double chCSAinput, 
            bool bIsCompoundCrossSection, double baseWidthUpperRegion, 
            double dblLowerRegionArea, double lowerRegionHeight, double chBankConst)
        {
            double chCSDepth = 0;
            if (bIsCompoundCrossSection == true && chCSAinput > dblLowerRegionArea)
            {
                double sngHighRegionArea = chCSAinput - dblLowerRegionArea;
                chCSDepth = (Math.Sqrt(Math.Pow(baseWidthUpperRegion, 2) + 2 * chBankConst * sngHighRegionArea) - baseWidthUpperRegion) / chBankConst;
                chCSDepth = chCSDepth + lowerRegionHeight;
            }
            else
                chCSDepth = (Math.Sqrt(Math.Pow(baseWidthLowerRegion, 2) + 2 * chBankConst * chCSAinput) - baseWidthLowerRegion) / chBankConst;
            return chCSDepth;
        }

        public void SetNoFluxOverlandFlowCV(cCVAttribute cv)
        {
            cv.hCVof_i_j = 0;
            cv.uCVof_i_j = 0;
            cv.CSAof_i_j = 0;
            cv.QCVof_i_j_m3Ps = 0;

        }

        public void SetNoFluxChannelFlowCV(cCVAttribute cv)
        {
            cv.mStreamAttr.hCVch_i_j = 0;
            cv.mStreamAttr.uCVch_i_j = 0;
            cv.mStreamAttr.CSAch_i_j = 0;
            cv.mStreamAttr.QCVch_i_j_m3Ps = 0;
        }
    }
}
