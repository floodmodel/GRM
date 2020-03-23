#include <omp.h>

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
    cvs[i].rfApp_dt_m = rfApp_dt_m(cvs[i].rfiRead_mPsec, dt_sec, di.cellSize, cvs[i].cvdx_m);
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
    cvs[i].QsumCVw_dt_m3 = 0;
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
    for (int iter = 0; iter < 20000; iter++) {
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
        double tolerance = Hp_n * CONST_TOLERANCE;
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


double getOverlandFlowDepthCVw(int i)
{
    double qSumToCViM1 = 0;
    double qCViM1;
    double qWn_i;
    int effCellCountFlowToCViW;
    effCellCountFlowToCViW = cvs[i].neighborCVIDsFlowIntoMe.size();
    for(int cvid : cvs[i].neighborCVIDsFlowIntoMe)    {
        qCViM1 = project.CVs[cvid - 1].QCVof_i_j_m3Ps / project.watershed.mCellSize; // 단위폭당 유량
        if (qCViM1 <= 0.0)
        {
            effCellCountFlowToCViW = effCellCountFlowToCViW - 1;
            qCViM1 = 0;
        }
        qSumToCViM1 = qSumToCViM1 + qCViM1;
        project.CVs[cvan].QsumCVw_dt_m3 = project.CVs[cvan].QsumCVw_dt_m3
            + project.CVs[cvid - 1].QCVof_i_j_m3Ps * sThisSimulation.dtsec;
    }
    if (effCellCountFlowToCViW < 1)
    {
        effCellCountFlowToCViW = 1;
    }
    project.CVs[cvan].effCVCountFlowINTOCViW = effCellCountFlowToCViW;
    qWn_i = Math.Pow(project.CVs[i].RoughnessCoeffOF * qSumToCViM1 / Math.Sqrt(project.CVs[i].SlopeOF), 0.6);
    return qWn_i;
}


double getChCSAbyFlowDepth(double LRBaseWidth,
    double chBankConst, double crossSectionDepth,
    bool isCompoundCS, double LRHeight,
    double LRArea, double URBaseWidth)
{
    if ((isCompoundCS == true)
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


double getChCSAbyQusingIteration(cvAtt cv, double CSAini, double Q_m3Ps)
{
    if (Q_m3Ps <= 0) { return 0; }
    double CSA_nP1 = 0;
    double cbw = cv.stream.chBaseWidth;
    double bc = cv.stream.bankCoeff;
    bool bCompound = cv.stream.isCompoundCS;
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
        double Hw_n = getChannelDepthUsingArea(cbw, CSA_n, bCompound,
            bwURegion, AreaLR, hLR, bc);
        ChCrossSecPer = getChannelCrossSectionPerimeter(cbw, cv.stream.chSideSlopeRight, 
            cv.stream.chSideSlopeLeft, Hw_n, bCompound, hLR, AreaLR, bwURegion);
        Fx = pow(CSA_n, 1.66667) * sqrt(cv.stream.chBedSlope)
            / (cv.stream.chRC * pow(ChCrossSecPer, 0.66667)) - Q_m3Ps;
        dFx = 1.66667 * pow(CSA_n, 0.66667) * sqrt(cv.stream.chBedSlope)
            / (cv.stream.chRC * pow(ChCrossSecPer, 0.66667));
        CSA_nP1 = CSA_n - Fx / dFx;
        double toler = CSA_n * CONST_TOLERANCE;
        double err = abs(CSA_nP1 - CSA_n);
        if (err < toler) {
            return CSA_nP1;
        }
        CSA_n = CSA_nP1;
    }
    return CSA_nP1;
}

double getChannelDepthUsingArea(double baseWidthLRegion,
    double chCSAinput, bool isCompoundCrossSection,
    double baseWidthURegion, double LRegionArea,
    double LRegionHeight, double chBankConst)
{
    double chCSDepth = 0;
    if (isCompoundCrossSection == true && chCSAinput > LRegionArea) {
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

double getChannelCrossSectionPerimeter(double LRegionBaseWidth,
    double sideSlopeRightBank, double sideSlopeLeftBank,
    double crossSectionDepth, bool isCompoundCrossSection,
    double LRegionHeight, double LRegionArea,
    double URegionBaseWidth)
{
    if (isCompoundCrossSection == true)    {
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
}


 int getDTsec(double cfln, double dx, 
     double vMax, int dtMax_min, int dtMin_min)
{
    if (vMax <= 0)    {
        return dtMax_min;
    }
    double dtsec_tmp = (int) (cfln * dx / (double)vMax);
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