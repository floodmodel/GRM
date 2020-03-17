#include "gentle.h"
#include "grm.h"




double getChCSAbyFlowDepth(double lowRBaseWidth, double chBankConst,
    double crossSectionDepth, bool isCompoundCS, double lowRHeight,
    double lowRArea, double highRBaseWidth)
{
    if ((isCompoundCS == true)
        && (crossSectionDepth > lowRHeight)) {
        double highFlowDepth = crossSectionDepth - lowRHeight;
        if (highFlowDepth < 0) {
            highFlowDepth = 0;
        }
        double sngHighFlowArea;
        sngHighFlowArea = (highRBaseWidth + chBankConst * highFlowDepth / 2.0)
            * highFlowDepth;
        return (lowRArea + sngHighFlowArea);
    }
    else {
        return (lowRBaseWidth + chBankConst * crossSectionDepth / 2.0)
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
    double hLR = cv.stream.chLowRHeight;
    double AreaLR = cv.stream.chLowRArea_m2;
    double bwUpperRegion = cv.stream.chHighRBaseWidth_m;
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
            bwUpperRegion, AreaLR, hLR, bc);
        ChCrossSecPer = getChannelCrossSectionPerimeter(cbw, cv.stream.chSideSlopeRight, 
            cv.stream.chSideSlopeLeft, Hw_n, bCompound, hLR, AreaLR, bwUpperRegion);
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

double getChannelDepthUsingArea(double baseWidthLowRegion,
    double chCSAinput, bool isCompoundCrossSection,
    double baseWidthHighRegion, double lowRegionArea,
    double lowRegionHeight, double chBankConst)
{
    double chCSDepth = 0;
    if (isCompoundCrossSection == true && chCSAinput > lowRegionArea) {
        double highRegionArea = chCSAinput - lowRegionArea;
        chCSDepth = (sqrt(pow(baseWidthHighRegion, 2)
            + 2 * chBankConst * highRegionArea) - baseWidthHighRegion) / chBankConst;
        chCSDepth = chCSDepth + lowRegionHeight;
    }
    else {
        chCSDepth = (sqrt(pow(baseWidthLowRegion, 2)
            + 2 * chBankConst * chCSAinput) - baseWidthLowRegion) / chBankConst;
    }
    return chCSDepth;
}

double getChannelCrossSectionPerimeter(double lowRegionBaseWidth,
    double sideSlopeRightBank, double sideSlopeLeftBank,
    double crossSectionDepth, bool isCompoundCrossSection,
    double lowRegionHeight, double lowRegionArea,
    double highRegionBaseWidth)
{
    if (isCompoundCrossSection == true)    {
        if (crossSectionDepth > lowRegionHeight)        {
            double lowFlowPerimeter = lowRegionBaseWidth 
                + sqrt(pow(lowRegionHeight, 2) + pow(lowRegionHeight / sideSlopeRightBank, 2))
                + sqrt(pow(lowRegionHeight, 2) + pow(lowRegionHeight / sideSlopeLeftBank, 2));
            double highFlowDepth = crossSectionDepth - lowRegionHeight;
            double highFlowPerimeter = sqrt(pow(highFlowDepth, 2) 
                + pow(highFlowDepth / sideSlopeRightBank, 2))
                + sqrt(pow(highFlowDepth, 2) + pow(highFlowDepth / sideSlopeLeftBank, 2)) 
                + highRegionBaseWidth - lowRegionBaseWidth;
            return (lowFlowPerimeter + highFlowPerimeter);
        }
    }
    else {
        return lowRegionBaseWidth
            + sqrt(pow(crossSectionDepth, 2) + pow(crossSectionDepth / sideSlopeRightBank, 2))
            + sqrt(pow(crossSectionDepth, 2) + pow(crossSectionDepth / sideSlopeLeftBank, 2));
    }
}