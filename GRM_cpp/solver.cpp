#include <omp.h>

#include "gentle.h"
#include "grm.h"

extern projectFile prj;
extern domaininfo di;
extern cvAtt* cvs;
extern map<int, vector<int>> cvaisToFA; //fa별 cv array idex 목록

extern thisSimulation ts;

int simulateRunoff(double nowTmin)
{
    int maxLimit = di.facMax + 1;
    for (int fac = 0; fac < maxLimit; ++fac) {
        if (cvaisToFA[fac].size() > 0) {
            double uMax = 0;
#pragma omp parallel
            {
                int iterLimit = cvaisToFA[fac].size();
                omp_set_num_threads(prj.maxDegreeOfParallelism);
                // reduction으로 max, min 찾는 것은 openMP 3.1 이상부터 가능, 
                // VS2019는 openMP 2.0 지원, 그러므로 critical 사용한다.
#pragma omp for schedule(guided)
                for (int i = 0; i < iterLimit; ++i) {
                    if (cvs[i].toBeSimulated == 1) {
                        simulateRunoffCore( i, nowTmin);
                        if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                            uMax = cvs[i].uOF;
                        }
                        else {
                            uMax = cvs[i].stream.uCH;
                        }
                    }
                }
#pragma omp critical(getVmax)
                {
                    if (ts.vMaxInThisStep < uMax) {
                        ts.vMaxInThisStep = uMax;
                    }
                }
            }

        }
    }
    return 1;
}

void simulateRunoffCore(int i, double nowTmin)
{
    int fac = cvs[i].fac;
    int dtsec = ts.dtsec;
    double cellsize = di.cellSize;
    //이거 InitializeCVForThisStep()여기로 옮길 수 있는지?
    cvs[i].rfApp_dt_m = rfApp_dt_m(cvs[i].rfiRead_mPsec, dtsec, cellsize, cvs[i].cvdx_m);
    if (project.generalSimulEnv.mbSimulateFlowControl == true &&
        (project.CVs[cvan].FCType == cFlowControl.FlowControlType.ReservoirOutflow ||
            project.CVs[cvan].FCType == cFlowControl.FlowControlType.Inlet))
    {
        cFlowControl.CalFCReservoirOutFlow(project, nowT_min, cvan);
    }
    else
    {
        InitializeCVForThisStep(project, cvan);
        if (project.CVs[cvan].FlowType == cGRM.CellFlowType.OverlandFlow)
        {
            double hCVw_i_jP1 = 0;
            if (fac > 0)
            {
                hCVw_i_jP1 = mFVMSolver.CalculateOverlandWaterDepthCViW(project, cvan);
            }
            if (hCVw_i_jP1 > 0 || project.CVs[cvan].hCVof_i_j > 0)
            {
                mFVMSolver.CalculateOverlandFlow(project.CVs[cvan], hCVw_i_jP1, project.watershed.mCellSize);
            }
            else
            {
                mFVMSolver.SetNoFluxOverlandFlowCV(project.CVs[cvan]);
            }
        }
        else if (project.CVs[cvan].FlowType == cGRM.CellFlowType.ChannelFlow
            || project.CVs[cvan].FlowType == cGRM.CellFlowType.ChannelNOverlandFlow)
        {
            double CSAchCVw_i_jP1 = 0;
            if (fac > 0)
            {
                CSAchCVw_i_jP1 = mFVMSolver.CalChCSA_CViW(project.CVs, cvan);
            }
            if (CSAchCVw_i_jP1 > 0 || project.CVs[cvan].mStreamAttr.hCVch_i_j > 0)
            {
                mFVMSolver.CalculateChannelFlow(project.CVs[cvan], CSAchCVw_i_jP1);
            }
            else
            {
                mFVMSolver.SetNoFluxChannelFlowCV(project.CVs[cvan]);
            }
        }
    }
    if (project.generalSimulEnv.mbSimulateFlowControl == true
        && (project.CVs[cvan].FCType == cFlowControl.FlowControlType.SinkFlow
            || project.CVs[cvan].FCType == cFlowControl.FlowControlType.SourceFlow
            || project.CVs[cvan].FCType == cFlowControl.FlowControlType.ReservoirOperation))
    {
        Dataset.GRMProject.FlowControlGridRow[] rows =
            (Dataset.GRMProject.FlowControlGridRow[])project.fcGrid.mdtFCGridInfo.Select("CVID = " + (cvan + 1));
        Dataset.GRMProject.FlowControlGridRow row = rows[0];
        double v;
        if (double.TryParse(row.MaxStorage, out v) == false || double.TryParse(row.MaxStorageR, out v) == false ||
            System.Convert.ToDouble(row.MaxStorage) * System.Convert.ToDouble(row.MaxStorageR) == 0)
        {
            cFlowControl.CalFCSinkOrSourceFlow(project, nowT_min, cvan);
        }
        else
        {
            cFlowControl.CalFCReservoirOperation(project, cvan, nowT_min);
        }
    }
}

double getChCSAbyFlowDepth(double LRBaseWidth, double chBankConst,
    double crossSectionDepth, bool isCompoundCS, double LRHeight,
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