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
