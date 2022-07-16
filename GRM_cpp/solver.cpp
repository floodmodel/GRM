#include "stdafx.h"
#include "gentle.h"
#include "grm.h"

extern projectFile prj;
extern domaininfo di;
extern cvAtt* cvs;

extern thisSimulation ts;

void updatetCVbyRFandSoil(int i)
{
    int dtrf_sec = prj.rfinterval_min * 60;
    double dY_m = di.cellSize;
    double effOFdYinCHnOFcell = 0;
    int dt_sec = ts.dtsec;
    double CVdx_m = cvs[i].cvdx_m;
    double chCSAaddedByBFlow_m2 = 0;
    double ofDepthAddedByRFlow_m2 = 0;
    double chCSAaddedBySSFlow_m2 = 0;
    cvs[i].rfApp_dt_m = cvs[i].rfiRead_mPsec * dt_sec * (di.cellSize / cvs[i].cvdx_m);
    if (cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) {
        effOFdYinCHnOFcell = dY_m - cvs[i].stream.chBaseWidth;
    }
    if (prj.simBaseFlow == 1) {
        chCSAaddedByBFlow_m2 = calBFlowAndGetCSAaddedByBFlow(i, dt_sec, dY_m);
    }
    if (prj.simSubsurfaceFlow == 1) {
        switch (cvs[i].flowType) {
        case cellFlowType::OverlandFlow: {
            ofDepthAddedByRFlow_m2 = calRFlowAndSSFlow(i, dt_sec, dY_m);
            break;
        }
        case cellFlowType::ChannelFlow: {
            chCSAaddedBySSFlow_m2 = getChCSAaddedBySSFlow(i);
            break;
        }
        case cellFlowType::ChannelNOverlandFlow: {
            ofDepthAddedByRFlow_m2 = calRFlowAndSSFlow(i,
                dt_sec, effOFdYinCHnOFcell);
            chCSAaddedBySSFlow_m2 = getChCSAaddedBySSFlow(i);
            break;
        }
        }
    }
    calEffectiveRainfall(i, dtrf_sec, dt_sec);
    switch (cvs[i].flowType) {
    case cellFlowType::OverlandFlow: {
		cvs[i].hOF_ori = cvs[i].hOF + cvs[i].rfEff_dt_m + ofDepthAddedByRFlow_m2;
        cvs[i].hOF = cvs[i].hOF_ori;
        cvs[i].csaOF = cvs[i].hOF_ori * dY_m;
        cvs[i].storageAddedForDTbyRF_m3 = cvs[i].rfEff_dt_m * dY_m * CVdx_m;
        break;
    }
    case cellFlowType::ChannelFlow: {
        double ChWidth = cvs[i].stream.chBaseWidth;
        cvs[i].stream.hCH_ori = cvs[i].stream.hCH + cvs[i].rfApp_dt_m
            + chCSAaddedBySSFlow_m2 / ChWidth
            + chCSAaddedByBFlow_m2 / ChWidth;
        cvs[i].stream.csaCH_ori = getChCSAbyFlowDepth(ChWidth,
            cvs[i].stream.bankCoeff, cvs[i].stream.hCH_ori,
            cvs[i].stream.isCompoundCS, cvs[i].stream.chLRHeight,
            cvs[i].stream.chLRArea_m2, cvs[i].stream.chURBaseWidth_m);
        cvs[i].stream.hCH = cvs[i].stream.hCH_ori;
        cvs[i].stream.csaCH = cvs[i].stream.csaCH_ori;
        cvs[i].storageAddedForDTbyRF_m3 = cvs[i].rfApp_dt_m * dY_m * CVdx_m;
        break;
    }
    case cellFlowType::ChannelNOverlandFlow: {
        double chCSAAddedByOFInChCell_m2;
        double ChWidth = cvs[i].stream.chBaseWidth;
		cvs[i].hOF_ori = cvs[i].hOF + cvs[i].rfEff_dt_m + ofDepthAddedByRFlow_m2;
        cvs[i].hOF = cvs[i].hOF_ori;
        cvs[i].csaOF = cvs[i].hOF_ori * effOFdYinCHnOFcell;
        if (cvs[i].hOF > 0) {
            calOverlandFlow(i, 0, effOFdYinCHnOFcell);
        }
        else {
            cvs[i].hOF = 0;
            cvs[i].uOF = 0;
            cvs[i].csaOF = 0;
            cvs[i].QOF_m3Ps = 0;
        }
        if (cvs[i].QOF_m3Ps > 0) {
            chCSAAddedByOFInChCell_m2 = cvs[i].csaOF;
        }
        else {
            chCSAAddedByOFInChCell_m2 = 0;
        }
        cvs[i].stream.csaChAddedByOFinCHnOFcell = chCSAAddedByOFInChCell_m2;
        cvs[i].stream.hCH_ori = cvs[i].stream.hCH + cvs[i].rfApp_dt_m
            + chCSAaddedBySSFlow_m2 / ChWidth
            + chCSAaddedByBFlow_m2 / ChWidth;
        if (cvs[i].stream.hCH_ori < 0) { cvs[i].stream.hCH_ori = 0; }
        cvs[i].stream.csaCH_ori = getChCSAbyFlowDepth(ChWidth,
            cvs[i].stream.bankCoeff, cvs[i].stream.hCH_ori,
            cvs[i].stream.isCompoundCS, cvs[i].stream.chLRHeight,
            cvs[i].stream.chLRArea_m2, cvs[i].stream.chURBaseWidth_m);
        cvs[i].stream.hCH = cvs[i].stream.hCH_ori;
        cvs[i].stream.csaCH = cvs[i].stream.csaCH_ori;
        cvs[i].storageAddedForDTbyRF_m3 = cvs[i].rfApp_dt_m * ChWidth * CVdx_m
            + cvs[i].rfEff_dt_m * effOFdYinCHnOFcell * CVdx_m;
        break;
    }
    }
    //cvs[i].QsumCVw_dt_m3 = 0; 
	//cvs[i].QsumCVw_m3Ps = 0;
}

void calOverlandFlow(int i, double hCVw_tp1, double effDy_m)
{
    double constHp_j = cvs[i].hOF;
    double CONST_DtPDx = ts.dtsec / cvs[i].cvdx_m;
    double Hp_n = constHp_j;
    double hCVw_n = hCVw_tp1;
    double uCVw_n = 0;
    double cCVw_n = 0;
    if (hCVw_n > 0) {
        uCVw_n = vByManningEq(hCVw_n, cvs[i].slopeOF, cvs[i].rcOF);
        cCVw_n = uCVw_n * CONST_DtPDx * hCVw_n;
    }
    for (int iter = 0; iter < ITER_NR; iter++) {
        double hCVe_n = Hp_n;
        double uCVe_n = vByManningEq(hCVe_n, cvs[i].slopeOF, cvs[i].rcOF);
        double cCVe_n = uCVe_n * CONST_DtPDx * hCVe_n;
        // Newton-Raphson
        double Fx = Hp_n - cCVw_n + cCVe_n - constHp_j;
        double dFx;
        dFx = 1 + (1.66667 * CONST_DtPDx
            * pow((hCVe_n), (0.66667)) * (pow(cvs[i].slopeOF, 0.5)) 
            / cvs[i].rcOF);
        double Hp_nP1 = (Hp_n - Fx / dFx);
        if (Hp_nP1 <= 0) {
            setNoFluxCVOF(i);
            break;
        }
        double tolerance = Hp_n * TOLERANCE;
        double err = abs(Hp_nP1 - Hp_n);
        if (err < tolerance) {
            cvs[i].hOF = Hp_nP1;
            cvs[i].uOF = vByManningEq(Hp_nP1, cvs[i].slopeOF, cvs[i].rcOF);
            cvs[i].csaOF = Hp_nP1 * effDy_m;
            cvs[i].QOF_m3Ps = cvs[i].csaOF * cvs[i].uOF;
            break;
        }
        Hp_n = Hp_nP1;
    }
}

void calChannelFlow(int i, double chCSACVw_tp1)
{
    double dtPdx = ts.dtsec / cvs[i].cvdx_m;
    double HRch = 0.0;
    double CSPer = 0.0;
    double hChw_tp1 = 0.0;
    double uw_n = 0.0;
    double cCHw_n = 0.0;
    if (chCSACVw_tp1 > 0) {
        hChw_tp1 = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
            chCSACVw_tp1, cvs[i].stream.isCompoundCS,
            cvs[i].stream.chURBaseWidth_m, cvs[i].stream.chLRArea_m2,
            cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
        CSPer = getChCrossSectionPerimeter(cvs[i].stream.chBaseWidth,
            cvs[i].stream.chSideSlopeRight, cvs[i].stream.chSideSlopeLeft,
            hChw_tp1, cvs[i].stream.isCompoundCS,
            cvs[i].stream.chLRHeight, cvs[i].stream.chLRArea_m2,
            cvs[i].stream.chURBaseWidth_m);
        HRch = chCSACVw_tp1 / CSPer;
        uw_n = vByManningEq(HRch, cvs[i].stream.slopeCH,
            cvs[i].stream.chRC);
        cCHw_n = uw_n * dtPdx * chCSACVw_tp1;
    }
    double CSAchCVp_j = cvs[i].stream.csaCH;
    double CSAp_n = CSAchCVp_j;
    double hChp_n = cvs[i].stream.hCH;
    for (int iter = 0; iter < ITER_NR; iter++) {
        double hChCVe_n = hChp_n;
        double CSAChCVe_n = CSAp_n;
        CSPer = getChCrossSectionPerimeter(cvs[i].stream.chBaseWidth,
            cvs[i].stream.chSideSlopeRight, cvs[i].stream.chSideSlopeLeft,
            hChCVe_n, cvs[i].stream.isCompoundCS, cvs[i].stream.chLRHeight,
            cvs[i].stream.chLRArea_m2, cvs[i].stream.chURBaseWidth_m);
        HRch = CSAChCVe_n / CSPer;
        double u_n = vByManningEq(HRch, cvs[i].stream.slopeCH,
            cvs[i].stream.chRC);
        double cChCVe_n = u_n * dtPdx * CSAChCVe_n;
        // Newton-Raphson
        double Fx = CSAp_n - cCHw_n + cChCVe_n - CSAchCVp_j;
        double dFx;
        dFx = 1 + (1.66667 * pow(CSAChCVe_n, 0.66667)
            * sqrt(cvs[i].stream.slopeCH) * dtPdx
            / (cvs[i].stream.chRC * pow(CSPer, 0.66667)));
        double CSAch_nP1 = CSAp_n - Fx / dFx;
        if (CSAch_nP1 <= 0) {
            setNoFluxCVCH(i);
            break;
        }
        double Qn = u_n * CSAp_n;
        HRch = CSAch_nP1 / CSPer;
        double u_nP1 = vByManningEq(HRch, cvs[i].stream.slopeCH,
            cvs[i].stream.chRC);
        double QnP1 = u_nP1 * CSAch_nP1;
        double tolerance = Qn * TOLERANCE;
        double err = abs(Qn - QnP1);
        double hCh_nP1 = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
            CSAch_nP1, cvs[i].stream.isCompoundCS,
            cvs[i].stream.chURBaseWidth_m, cvs[i].stream.chLRArea_m2,
            cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
        if (err < tolerance) {
            cvs[i].stream.hCH = hCh_nP1;
            cvs[i].stream.uCH = u_nP1;
            cvs[i].stream.csaCH = CSAch_nP1;
            cvs[i].stream.QCH_m3Ps = QnP1;
            break;
        }
        CSAp_n = CSAch_nP1;
        hChp_n = hCh_nP1;
    }
}

double getOverlandFlowDepthCVw(int i)
{
    double qSumFromCViM1 = 0;
	double QsumCViM1_m3Ps = 0;
    double qCViM1;
    double hWn_i;
    int nEffCVFlowintoCVw;
	cvs[i].QsumCVw_dt_m3 = 0;
	cvs[i].QsumCVw_m3Ps = 0;
    nEffCVFlowintoCVw = (int)cvs[i].neighborCVidxFlowintoMe.size();
    for (int iw : cvs[i].neighborCVidxFlowintoMe) {
        qCViM1 = cvs[iw].QOF_m3Ps / di.cellSize; // 단위폭당 유량
        if (qCViM1 <= 0.0) {
            nEffCVFlowintoCVw = nEffCVFlowintoCVw - 1;
            qCViM1 = 0;
        }
        qSumFromCViM1 = qSumFromCViM1 + qCViM1;
		QsumCViM1_m3Ps += cvs[iw].QOF_m3Ps;

    }
	cvs[i].QsumCVw_dt_m3 = QsumCViM1_m3Ps * ts.dtsec;
	cvs[i].QsumCVw_m3Ps = QsumCViM1_m3Ps;

    if (nEffCVFlowintoCVw < 1) {
        nEffCVFlowintoCVw = 1;
    }
    cvs[i].effCVnFlowintoCVw = nEffCVFlowintoCVw;
    hWn_i = pow(cvs[i].rcOF * qSumFromCViM1 / sqrt(cvs[i].slopeOF), 0.6);
    return hWn_i;
}

double getChCSAatCVW(int i)
{    // w의 단면적 계산
    double CSAe_iM1 = 0;
    double CSAeSum_iM1 = 0;
    double QsumCViM1_m3Ps = 0;
	cvs[i].QsumCVw_dt_m3 = 0;
	cvs[i].QsumCVw_m3Ps = 0;
    int nEffCVFlowintoCVw = (int)cvs[i].neighborCVidxFlowintoMe.size();
    for (int iw : cvs[i].neighborCVidxFlowintoMe) {
        double Qcv_iM1_m3Ps = 0;
        if (cvs[iw].flowType == cellFlowType::OverlandFlow) {
            CSAe_iM1 = cvs[iw].csaOF;
            Qcv_iM1_m3Ps = cvs[iw].QOF_m3Ps;

        }
        else if (cvs[iw].flowType == cellFlowType::ChannelFlow
            || cvs[iw].flowType == cellFlowType::ChannelNOverlandFlow) {
            CSAe_iM1 = cvs[iw].stream.csaCH;
            Qcv_iM1_m3Ps = cvs[iw].stream.QCH_m3Ps;
            if (cvs[iw].flowType == cellFlowType::ChannelNOverlandFlow) {
                CSAe_iM1 = CSAe_iM1
                    + cvs[iw].stream.csaChAddedByOFinCHnOFcell;
                Qcv_iM1_m3Ps = Qcv_iM1_m3Ps + cvs[iw].QOF_m3Ps;
            }
        }
		if (Qcv_iM1_m3Ps <= 0.0) {
			nEffCVFlowintoCVw = nEffCVFlowintoCVw - 1;
			Qcv_iM1_m3Ps = 0;
		}
		QsumCViM1_m3Ps = QsumCViM1_m3Ps + Qcv_iM1_m3Ps;
        CSAeSum_iM1 = CSAeSum_iM1 + CSAe_iM1;
    }
    cvs[i].QsumCVw_dt_m3 = QsumCViM1_m3Ps * ts.dtsec;
	cvs[i].QsumCVw_m3Ps = QsumCViM1_m3Ps;

    if (nEffCVFlowintoCVw < 1) { nEffCVFlowintoCVw = 1; }
    cvs[i].effCVnFlowintoCVw = nEffCVFlowintoCVw;
    if (CSAeSum_iM1 < WETDRY_CRITERIA) {
        return 0;
    }
    double CSAw_n = CSAeSum_iM1;
    double CSAw_nP1 = 0;
    for (int iter = 0; iter < 100; ++iter) {
        double hWn_i = getChDepthUsingCSA(cvs[i].stream.chBaseWidth, CSAw_n,
            cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
            cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight,
            cvs[i].stream.bankCoeff);
        double chCSPeri = getChCrossSectionPerimeter(cvs[i].stream.chBaseWidth,
            cvs[i].stream.chSideSlopeRight, cvs[i].stream.chSideSlopeLeft,
            hWn_i, cvs[i].stream.isCompoundCS, cvs[i].stream.chLRHeight,
            cvs[i].stream.chLRArea_m2, cvs[i].stream.chURBaseWidth_m);
        double Fx = pow(CSAw_n, 1.66667) 
            * pow(cvs[i].stream.slopeCH, 0.5)
            / (cvs[i].stream.chRC * pow(chCSPeri, 0.66667)) 
            - QsumCViM1_m3Ps;
        double dFx = 1.66667 * pow(CSAw_n, 0.66667) 
            * pow(cvs[i].stream.slopeCH, 0.5)
            / (cvs[i].stream.chRC * pow(chCSPeri, 0.66667));
        CSAw_nP1 = CSAw_n - Fx / dFx;
        double err = abs(CSAw_nP1 - CSAw_n) / CSAw_n;
        if (err < TOLERANCE) {
            return CSAw_nP1;
        }
        CSAw_n = CSAw_nP1;
    }
    return CSAw_nP1;
}

double getChCSAbyFlowDepth(double LRBaseWidth,
    double chBankConst, double crossSectionDepth,
    int isCompoundCS, double LRHeight,
    double LRArea, double URBaseWidth)
{
    if ((isCompoundCS == 1)
        && (crossSectionDepth > LRHeight)) {
        double uFlowDepth = crossSectionDepth - LRHeight;
        if (uFlowDepth < 0) {
            uFlowDepth = 0;
        }
        double uFlowArea;
        uFlowArea = (URBaseWidth + chBankConst * uFlowDepth / 2.0)
            * uFlowDepth;
        return (LRArea + uFlowArea);
    }
    else {
        return (LRBaseWidth + chBankConst * crossSectionDepth / 2.0)
            * crossSectionDepth;
    }
}


double getChCSAusingQbyiteration(cvAtt cv, double CSAini, double Q_m3Ps)
{
    if (Q_m3Ps <= 0) { return 0; }
    double CSA_nP1 = 0;
    double cbw = cv.stream.chBaseWidth;
    double bc = cv.stream.bankCoeff;
    int bCompound = cv.stream.isCompoundCS;
    double hLR = cv.stream.chLRHeight;
    double AreaLR = cv.stream.chLRArea_m2;
    double bwURegion = cv.stream.chURBaseWidth_m;
    double CSA_n;
    if (CSAini > 0) {
        CSA_n = CSAini;
    }
    else {
        CSA_n = Q_m3Ps / cv.cvdx_m;
    }
    double Fx;
    double dFx;
    double ChCrossSecPer;
    for (int iter = 0; iter < 100; ++iter) {
        double Hw_n = getChDepthUsingCSA(cbw, CSA_n, bCompound,
            bwURegion, AreaLR, hLR, bc);
        ChCrossSecPer = getChCrossSectionPerimeter(cbw, cv.stream.chSideSlopeRight, 
            cv.stream.chSideSlopeLeft, Hw_n, bCompound, hLR, AreaLR, bwURegion);
        Fx = pow(CSA_n, 1.66667) * sqrt(cv.stream.slopeCH)
            / (cv.stream.chRC * pow(ChCrossSecPer, 0.66667)) - Q_m3Ps;
        dFx = 1.66667 * pow(CSA_n, 0.66667) * sqrt(cv.stream.slopeCH)
            / (cv.stream.chRC * pow(ChCrossSecPer, 0.66667));
        CSA_nP1 = CSA_n - Fx / dFx;
        double toler = CSA_n * TOLERANCE;
        double err = abs(CSA_nP1 - CSA_n);
        if (err < toler) {
            return CSA_nP1;
        }
        CSA_n = CSA_nP1;
    }
    return CSA_nP1;
}

double getChDepthUsingCSA(double baseWidthLRegion,
    double chCSAinput, int isCompoundCS,
    double baseWidthURegion, double LRegionArea,
    double LRegionHeight, double chBankConst)
{
    double chCSDepth = 0;
    if (isCompoundCS == 1 && chCSAinput > LRegionArea) {
        double uRegionArea = chCSAinput - LRegionArea;
        chCSDepth = (sqrt(pow(baseWidthURegion, 2)
            + 2 * chBankConst * uRegionArea) - baseWidthURegion) / chBankConst;
        chCSDepth = chCSDepth + LRegionHeight;
    }
    else {
        chCSDepth = (sqrt(pow(baseWidthLRegion, 2)
            + 2 * chBankConst * chCSAinput) - baseWidthLRegion) / chBankConst;
    }
    return chCSDepth;
}

double getChCrossSectionPerimeter(double LRegionBaseWidth,
    double sideSlopeRightBank, double sideSlopeLeftBank,
    double crossSectionDepth, int isCompoundCS,
    double LRegionHeight, double LRegionArea,
    double URegionBaseWidth)
{
    if (isCompoundCS == 1)    {
        if (crossSectionDepth > LRegionHeight)        {
            double LFlowPerimeter = LRegionBaseWidth 
                + sqrt(pow(LRegionHeight, 2) + pow(LRegionHeight / sideSlopeRightBank, 2))
                + sqrt(pow(LRegionHeight, 2) + pow(LRegionHeight / sideSlopeLeftBank, 2));
            double uFlowDepth = crossSectionDepth - LRegionHeight;
            double uFlowPerimeter = sqrt(pow(uFlowDepth, 2) 
                + pow(uFlowDepth / sideSlopeRightBank, 2))
                + sqrt(pow(uFlowDepth, 2) + pow(uFlowDepth / sideSlopeLeftBank, 2)) 
                + URegionBaseWidth - LRegionBaseWidth;
            return (LFlowPerimeter + uFlowPerimeter);
        }
    }
    else {
        return LRegionBaseWidth
            + sqrt(pow(crossSectionDepth, 2) + pow(crossSectionDepth / sideSlopeRightBank, 2))
            + sqrt(pow(crossSectionDepth, 2) + pow(crossSectionDepth / sideSlopeLeftBank, 2));
    }
    return 0;
}

 int getDTsec(double dx, 
     double vMax, int dtMax_min, int dtMin_min)
{
    if (vMax <= 0)    {
        return dtMax_min;
    }
    double dtsec_tmp = (int) (CFL_NUMBER * dx / (double)vMax);
    int dtsec =(int)floor(dtsec_tmp);
    if (dtsec > dtMax_min)    {
        dtsec = dtMax_min;
    }
    if (dtsec < dtMin_min)    {
        dtsec = dtMin_min;
    }
    return dtsec;
}

 inline  double vByManningEq(double hydraulicRaidus,
     double slope, double nCoeff)
 {
     return pow(hydraulicRaidus, 0.66667) * sqrt(slope) / nCoeff;
 }

inline void setNoFluxCVCH(int i)
 {
    cvs[i].stream.hCH = 0;
    cvs[i].stream.uCH = 0;
    cvs[i].stream.csaCH = 0;
    cvs[i].stream.QCH_m3Ps = 0;
 }

 inline void setNoFluxCVOF(int i)
 {
     cvs[i].hOF = 0;
     cvs[i].uOF = 0;
     cvs[i].csaOF = 0;
     cvs[i].QOF_m3Ps = 0;
 }