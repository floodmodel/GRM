
#include <math.h>
#include "grm.h"

extern projectFile prj;
extern cvAtt* cvs;
extern domaininfo di;



void calEffectiveRainfall(int i, int dtrf_sec, int dtsec)
{
    if (prj.simInfiltration == -1) {
        setNoInfiltrationParameters(i);
        return;
    }
    if (cvs[i].flowType == cellFlowType::ChannelFlow
        && cvs[i].stream.chStrOrder > prj.swps[cvs[i].wsid].dryStreamOrder) {
        setWaterAreaInfiltrationPars(i);
        return;
    }
    if (cvs[i].lcCode == landCoverCode::WATR
        || cvs[i].lcCode == landCoverCode::WTLD) {
        setWaterAreaInfiltrationPars(i);
        return;
    }
    if (cvs[i].imperviousR == 1) {
        setNoInfiltrationParameters(i);
        return;
    }
    double CONSTGreenAmpt;
    double residuMC_ThetaR;
    cvs[i].ssr = soilSaturationRaitoByCumulF(cvs[i].soilWaterC_tm1_m,
        cvs[i].sdEffAsWaterDepth_m, cvs[i].flowType);
    if (cvs[i].ssr > 0.99 
        || (prj.simType == simulationType::SingleEventPE_SSR 
            && cvs[i].ssr == 1))
    {
        if (cvs[i].ssr == 1)
            cvs[i].soilWaterC_m = cvs[i].sdEffAsWaterDepth_m;
        cvs[i].isAfterSaturated = 1;
        cvs[i].ifF_mPdt = 0;
        cvs[i].ifRatef_mPsec = 0;
        cvs[i].rfEff_dt_m = cvs[i].rfApp_dt_m;
        cvs[i].soilWaterC_m = cvs[i].soilWaterC_tm1_m;
    }
    else
    {
        residuMC_ThetaR = cvs[i].porosity_Eta - cvs[i].effPorosity_ThetaE;
        if (residuMC_ThetaR < 0) {
            residuMC_ThetaR = 0;
        }
        cvs[i].effSR_Se = (cvs[i].porosity_Eta * cvs[i].iniSSR - residuMC_ThetaR)
            / (cvs[i].porosity_Eta - residuMC_ThetaR);

        if (cvs[i].effSR_Se < 0) {
            cvs[i].effSR_Se = 0;
        }
        cvs[i].soilMoistureChange_DTheta = (1 - cvs[i].effSR_Se) * cvs[i].effPorosity_ThetaE;
        CONSTGreenAmpt = cvs[i].soilMoistureChange_DTheta * cvs[i].wfsh_Psi_m;
        double infiltrationF_mPdt_max;
        bool beingPonding = false;
        if (cvs[i].ifRatef_tm1_mPsec >= cvs[i].rfiRead_tm1_mPsec) {
            // 이전 시간에서의 침투률이 이전 시간에서의 강우강도보다 컸다면, 모든 강우는 침투됨
            infiltrationF_mPdt_max = cvs[i].rfApp_dt_m;
        }
        else {
            // 이전 시간에서의 침투률이 이전 시간에서의 강우강도보다 같거나 작았다면 ponding이 발생한 경우
            infiltrationF_mPdt_max = getinfiltrationForDtAfterPonding(i, 
                dtsec, CONSTGreenAmpt, cvs[i].hc_K_mPsec);
            beingPonding = true;
        }
        if (infiltrationF_mPdt_max <= 0) {
            cvs[i].ifF_mPdt = 0; 
        }
        else {
            double dF = cvs[i].sdEffAsWaterDepth_m - cvs[i].soilWaterC_tm1_m;
            if (dF < 0) {
                cvs[i].ifF_mPdt = 0;
            }
            else if (dF < infiltrationF_mPdt_max) {
                cvs[i].ifF_mPdt = dF;
            }
        }
        // 누가 침투량으로 dt 동안에 추가된 침투량을 더한다.
        cvs[i].soilWaterC_m = cvs[i].soilWaterC_tm1_m + cvs[i].ifF_mPdt;
        // 현재까지의 누가 침투량을 이용해서 이에 대한 포텐셜 침투률을 계산한다.
        if (cvs[i].soilWaterC_m <= 0) {
            cvs[i].ifRatef_mPsec = 0;
            cvs[i].soilWaterC_m = 0;
        }
        else {
            cvs[i].ifRatef_mPsec = cvs[i].hc_K_mPsec 
                * (1 + CONSTGreenAmpt / cvs[i].soilWaterC_m);
        }

        // 이경우에는 침투는 있지만.. 강우는 모두 직접 유출.. 침투는 지표면 저류 상태에서 발생
        if (cvs[i].isAfterSaturated == 1 
            && (beingPonding == true || cvs[i].ssr > 0.99)) {
            cvs[i].rfEff_dt_m = cvs[i].rfApp_dt_m;
        }
        else {
            double effRF_dt_m = cvs[i].rfApp_dt_m - cvs[i].ifF_mPdt;
            if (effRF_dt_m > cvs[i].rfApp_dt_m) {
                effRF_dt_m = cvs[i].rfApp_dt_m;
            }
            if (effRF_dt_m < 0) {
                effRF_dt_m = 0;
            }
            cvs[i].rfEff_dt_m = effRF_dt_m;
            if (cvs[i].imperviousR < 1) {
                cvs[i].rfEff_dt_m = cvs[i].rfEff_dt_m * (1 - cvs[i].imperviousR) 
                    + cvs[i].rfApp_dt_m * cvs[i].imperviousR;
                if (cvs[i].rfEff_dt_m > cvs[i].rfApp_dt_m) {
                    cvs[i].rfEff_dt_m = cvs[i].rfApp_dt_m;
                }
            }
        }
    }
    // 유효강우량의 계산이 끝났으므로, 현재까지 계산된 침투, 강우강도 등을 tM1 변수로 저장한다.    
    cvs[i].ssr = soilSaturationRaitoByCumulF(cvs[i].soilWaterC_m, 
        cvs[i].sdEffAsWaterDepth_m, cvs[i].flowType);
    if (cvs[i].ssr == 1) {
        cvs[i].isAfterSaturated = true;
    }
    cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_m;
    cvs[i].ifRatef_tm1_mPsec = cvs[i].ifRatef_mPsec;
    cvs[i].rfiRead_tm1_mPsec = cvs[i].rfiRead_mPsec;
}

double getinfiltrationForDtAfterPonding(int i, int dtSEC,
    double CONSTGreenAmpt, double Kapp)
{
    double CI_n;
    double CI_nP1;
    double err;
    double conCriteria;
    double Fx;
    double dFx;
    double constCI_tm1 = 0.0;
    CI_n = cvs[i].soilWaterC_tm1_m + cvs[i].ifRatef_tm1_mPsec * dtSEC;
    constCI_tm1 = cvs[i].soilWaterC_tm1_m;
    for (int iter = 0; iter < 20000; ++iter) {
        // Newton-Raphson
        Fx = CI_n - constCI_tm1 - Kapp * dtSEC
            - CONSTGreenAmpt * log((CI_n + CONSTGreenAmpt)
                / (cvs[i].soilWaterC_tm1_m + CONSTGreenAmpt));
        dFx = 1 - CONSTGreenAmpt / (CI_n + CONSTGreenAmpt);
        CI_nP1 = CI_n - Fx / dFx;
        err = abs(CI_nP1 - CI_n);
        conCriteria = (CI_n * CONST_TOLERANCE);
        if (err < conCriteria) {
            double dF = CI_nP1 - constCI_tm1;
            if (dF < 0)
                dF = 0;
            return dF;
        }
        CI_n = CI_nP1;
    }
    double dFr = CI_n - constCI_tm1;
    if (dFr < 0) {
        return 0;
    }
    else {
        return dFr;
    }
}


// 현재 cv의 Return flow는 상류에서 유입되는 ssflwo로 계산하고, 
// 현재 cv의 ssf는 현재 셀의 수분함량으로 계산한다.
double calRFlowAndSSFlow(int i,
    int dtsec, double dy_m)
{
    double dx_m = cvs[i].cvdx_m;
    double SSFfromCVw;
    double SSFAddedToMeFromCVw_m;
    double RflowBySSFfromCVw_m2; // 이건 return flow
    SSFfromCVw = totalSSFfromCVwOFcell_m3Ps(i) * dtsec / (dy_m * dx_m);
    // 상류의 지표하 유출에 의한 현재 셀의 Rflow 기여분 계산
    double ssr = soilSaturationRaitoByCumulF(cvs[i].soilWaterC_tm1_m,
        cvs[i].sdEffAsWaterDepth_m, cvs[i].flowType);
    if (ssr >= 0.99) {
        RflowBySSFfromCVw_m2 = SSFfromCVw * dy_m;
        SSFAddedToMeFromCVw_m = 0;
    }
    else {
        RflowBySSFfromCVw_m2 = SSFfromCVw * cvs[i].ssr * dy_m;
        SSFAddedToMeFromCVw_m = SSFfromCVw * (1 - cvs[i].ssr);
    }
    // 상류에서 유입된 지표하 유출 깊이 더하고..
    cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m
        + SSFAddedToMeFromCVw_m;
    // 현재의 침투 깊이를 이용해서 rf 계산하고..
    if (cvs[i].soilWaterC_tm1_m > cvs[i].sdEffAsWaterDepth_m) {
        // 하류로 이송된 지표하 유출량이 현재 셀의 포화가능 깊이를 초과하면, 초과분은 returnflow
        RflowBySSFfromCVw_m2 = (cvs[i].soilWaterC_tm1_m
            - cvs[i].sdEffAsWaterDepth_m)
            * dy_m + RflowBySSFfromCVw_m2;
        cvs[i].soilWaterC_tm1_m = cvs[i].sdEffAsWaterDepth_m;
    }
    if (cvs[i].soilWaterC_tm1_m < 0) {
        cvs[i].soilWaterC_tm1_m = 0;
    }
    
    // 현재 셀의 지표하 유출. 지표하 유출이 발생하는 부분은 물이 차있는 부분이므로.. 포화수리전도도 적용
    cvs[i].QSSF_m3Ps = cvs[i].soilWaterC_tm1_m / cvs[i].porosity_Eta
        * cvs[i].hc_K_mPsec * sin(atan(cvs[i].slopeOF)) * dy_m;
    // 하류로 유출한 이후의 현재 누가침투심
    cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m
        - cvs[i].QSSF_m3Ps / (dy_m * cvs[i].cvdx_m) * dtsec;
    return RflowBySSFfromCVw_m2; 
}


double calBFlowAndGetCSAaddedByBFlow(int i, int dtsec, double cellSize_m)
{
    double deltaSoilDepthofUAQ_m; // B층(비피압 대수층)의 높이 변화. A층이 포화될 경우, B층으로 침누되면서 발생, 연직방향 침투깊이
    double soilDepthPercolated_m;
    double waterDepthPercolated_m;
    double csa = 0;
    int wsid = cvs[i].wsid;
    deltaSoilDepthofUAQ_m = 0;
    // 토양의 포화 상태와 상관없이 침누가 발생한다. 토양포화도가 상승 및 하강한다. 포화도가 0 이면 침누 발생 안한다.
    if (cvs[i].ssr > 0) {
        soilDepthPercolated_m = Kunsaturated(cvs[i]) * dtsec;
    }
    else {
        soilDepthPercolated_m = 0;
    }
    if (cvs[i].sd_m < soilDepthPercolated_m) {
        soilDepthPercolated_m = cvs[i].sd_m;
    }
    if ((cvs[i].flowType == cellFlowType::ChannelFlow
        && cvs[i].stream.chStrOrder > prj.swps[wsid].dryStreamOrder)
        || cvs[i].lcCode == landCoverCode::WATR
        || cvs[i].lcCode == landCoverCode::WTLD) {
        // 이조건에서는 항상 포화상태, 침누있음, 강우에 의한 침투량 없음. 대신 지표면 저류량에 의한 침투는 항상 있음
        setWaterAreaInfiltrationPars(i);
    }
    else if (cvs[i].sd_m == soilDepthPercolated_m) {
        cvs[i].soilWaterC_tm1_m = 0;
    } // 이전시간에 침투된 모든 양이 B 층으로 침누된 경우이므로..
    else {
        waterDepthPercolated_m = soilDepthPercolated_m * cvs[i].effPorosity_ThetaE;
        cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m
            - waterDepthPercolated_m;
        if (cvs[i].soilWaterC_tm1_m < 0) {
            cvs[i].soilWaterC_tm1_m = 0;
        }
    }
    cvs[i].hUAQfromBedrock_m = cvs[i].hUAQfromBedrock_m + soilDepthPercolated_m;
    double toBed = cvs[i].sdToBedrock_m - cvs[i].sd_m;
    if (cvs[i].hUAQfromBedrock_m > toBed) {
        cvs[i].hUAQfromBedrock_m = toBed;
    }
    if (cvs[i].flowType == cellFlowType::OverlandFlow) {
        calBFLateralMovement(i, di.facMin, cellSize_m, dtsec);
    }
    else {
        if (cvs[i].stream.hCH > 0) {
            // 하천 수심보다 더 많은 양이 침누되었을 경우, 하천 수심만큼만 침누된 것으로 한다.
            double dHinUAQ_m = soilDepthPercolated_m
                * cvs[i].porosity_Eta;
            if (dHinUAQ_m > cvs[i].stream.hCH) {
                dHinUAQ_m = cvs[i].stream.hCH;
            }
            cvs[i].hUAQfromChannelBed_m = cvs[i].hUAQfromChannelBed_m
                + dHinUAQ_m;
            if (cvs[i].hUAQfromChannelBed_m > 0) {
                // 기저유출은 물이 차있는 영역에서 발생하므로.. 포화수리전도도 적용
                if (cvs[i].hUAQfromChannelBed_m > cvs[i].stream.hCH) {
                    csa = cvs[i].hc_K_mPsec
                        * (cvs[i].hUAQfromChannelBed_m - cvs[i].stream.hCH)
                        / cvs[i].stream.hCH
                        * cvs[i].stream.chBaseWidth * dtsec;
                }
                else {
                    csa = cvs[i].hc_K_mPsec
                        * (cvs[i].hUAQfromChannelBed_m - cvs[i].stream.hCH)
                        * dtsec;
                }
            }
            else {
                csa = 0;
            }
        }
        else {
            csa = 0;
        }
        cvs[i].hUAQfromChannelBed_m = cvs[i].hUAQfromChannelBed_m - csa
            / cvs[i].stream.chBaseWidth / cvs[i].porosity_Eta;
        cvs[i].hUAQfromBedrock_m = cvs[i].hUAQfromBedrock_m
            - csa / cvs[i].stream.chBaseWidth
            / cvs[i].porosity_Eta;
        if (cvs[i].hUAQfromChannelBed_m < 0) {
            cvs[i].hUAQfromChannelBed_m = 0;
        }
        if (cvs[i].hUAQfromBedrock_m < 0) {
            cvs[i].hUAQfromBedrock_m = 0;
        }
    }
    return csa;
}

double getChCSAaddedBySSFlow(int i)
{
    double SSFlow_cms = 0;
    if (cvs[i].flowType == cellFlowType::ChannelFlow) {
        SSFlow_cms = totalSSFfromCVwOFcell_m3Ps(i);
    }
    else if (cvs[i].flowType == cellFlowType::ChannelNOverlandFlow) { 
        SSFlow_cms = cvs[i].QSSF_m3Ps;
    }    
    if (SSFlow_cms > 0) {
        if (cvs[i].stream.uCH > 0) {     
            double chCSA;
            chCSA = SSFlow_cms / cvs[i].stream.uCH; // 이전 시간에 계산된 유속
            // 유속이 작을 경우.. 발산 가능성 있으므로.. 기존 단면적 보다 큰 단면적으로 유입될 경우.. 
            // 기존 단면적으로 유입되는 것으로 설정..
            // 즉, 지표하 유출의 기여 단면적은 기존 하천단면적보다 클수 없음.
            if (chCSA > cvs[i].stream.csaCH)
                chCSA = cvs[i].stream.csaCH;
            return chCSA;
        }
    }
    else {
        // 이경우에는 하도에 기여되지 않는 것으로 가정
        // 즉 강우에 의한 하도의 유량 발생이 시작되지 않았는데(하도가 말라있을 경우).. 
        // 지표하 유출에 의해 기여가 있을 수 없음.
        return 0;
    }
}


double Kunsaturated(cvAtt cv)
{
    // double ca = 0.2 '0.24
    double ssr = cv.ssr;
    if (cv.flowType == cellFlowType::ChannelFlow) {
        ssr= 1;
    }
    if (cv.sdEffAsWaterDepth_m <= 0) {
        ssr = 1;
    }
    if (cv.soilWaterC_tm1_m <= 0) {
        ssr = 0;
    }
    double Ks = cv.hc_K_mPsec;
    unSaturatedKType uKType = cv.ukType;
    double CoefUnsaturatedK = cv.coefUK;
    if (CoefUnsaturatedK <= 0) {
        return Ks * 0.1;
    }
    if (ssr > 0.99) {
        return Ks;
    }
    else {
        switch (uKType) {
        case unSaturatedKType::Linear: {
            return Ks * ssr * CoefUnsaturatedK;
        }
        case unSaturatedKType::Exponential: {
            return Ks * pow(ssr, CoefUnsaturatedK);
        }
        case unSaturatedKType::Constant: {
            return Ks * CoefUnsaturatedK;
        }
        default: {
            return Ks * ssr * CoefUnsaturatedK;
        }
        }
    }
}

void calBFLateralMovement(int i,
    int facMin, double dY_m, double dtsec)
{
    double cumulBFq_m3Ps = 0;
    double dhUAQ_m;
    double dX_m = cvs[i].cvdx_m;
    for (int cvid : cvs[i].neighborCVIDsFlowintoMe) {
        if (cvs[cvid - 1].flowType == cellFlowType::OverlandFlow) {
            cumulBFq_m3Ps = cumulBFq_m3Ps 
                + cvs[cvid - 1].bfQ_m3Ps;// 수두경사가 셀의 지표면 경사와 같은 것으로 가정
        }
    }
    if (cvs[i].fac == facMin) {
        dX_m = dX_m / (double)2;
    }
    dhUAQ_m = cumulBFq_m3Ps * dtsec / (dY_m * dX_m)
        / cvs[i].effPorosity_ThetaE;  // 이건 검사체적에 균질하게 퍼져 있는 수위변화분
    cvs[i].hUAQfromBedrock_m = cvs[i].hUAQfromBedrock_m 
        + dhUAQ_m; // 이건 지하수대의 토양 깊이 변화
    double toBed = cvs[i].sdToBedrock_m - cvs[i].sd_m;
    if (cvs[i].hUAQfromBedrock_m > toBed) {
        cvs[i].hUAQfromBedrock_m = toBed;
    }
    // 기저유출 계산에서는 포화수리전도도 적용. 포화된 영역에서 발생
    cvs[i].bfQ_m3Ps = cvs[i].hUAQfromBedrock_m 
        * cvs[i].hc_K_mPsec * sin(atan(cvs[i].slopeOF)) * dY_m;
    if (cvs[i].bfQ_m3Ps < 0) {
        cvs[i].bfQ_m3Ps = 0;
    }
}


double totalSSFfromCVwOFcell_m3Ps(int i)
{
    double SSF_m3Ps = 0;
    for(int cvid: cvs[i].neighborCVIDsFlowintoMe)    {
        if (cvs[cvid - 1].flowType == cellFlowType::OverlandFlow) {
            // 즉, GRM에서 Green-Ampt 모형을 이용해서 침투된 양을 계산할때, 
            // 포화된 깊이로 계산됨. 즉, 현재 침투률로 얼만큼 깊이 들어갔는지는 계산하지 않는다..
            // 따라서, 하도로 기여하는 지표하 유출량 계산할때는 누가침투깊이가 .saturatedSoildepth_m 이 된다.
            SSF_m3Ps = SSF_m3Ps + cvs[cvid - 1].QSSF_m3Ps;
        }
    }
    return SSF_m3Ps;
}

inline double soilSaturationRaitoByCumulF(double cumulinfiltration,
    double effSoilDepth, cellFlowType flowType)
{
    if (flowType == cellFlowType::ChannelFlow) { return 1; }
    if (effSoilDepth <= 0) { return 1; }
    if (cumulinfiltration <= 0) { return 0; }
    double ssr = cumulinfiltration / effSoilDepth;
    if (ssr > 1) { return 1; }
    return ssr;
}

inline void setNoInfiltrationParameters(int i)
{
    cvs[i].ifF_mPdt = 0;
    cvs[i].ifRatef_mPsec = 0;
    cvs[i].ssr = 0;
    cvs[i].soilWaterC_m = 0;
    cvs[i].soilWaterC_tm1_m = 0;
    cvs[i].rfEff_dt_m = cvs[i].rfApp_dt_m;
}

inline void setWaterAreaInfiltrationPars(int i)
{
    cvs[i].ifF_mPdt = 0;
    cvs[i].ifRatef_mPsec = 0;
    cvs[i].ssr = 1;
    cvs[i].rfEff_dt_m = cvs[i].rfApp_dt_m;
}

