#include "stdafx.h"
#include "grm.h"

extern projectFile prj;
extern cvAtt* cvs;
extern cvpos* cvps;
extern domaininfo di;

void calEffectiveRFbyInfiltration(int i, int dtrf_sec, int dtsec)
{
    if (prj.simInfiltration == -1) {
		setNoInfiltrationParameters(i);
		return;
	}
	//// 2024.02.08 �ּ�ó��. ���������� 1�� ��쿡��, ����� �帧���� �ٸ� ������ ���ԵǴ� ��� ���п� ���ؼ� ���� ���� ������, ��ȭ���� �޶��� �� �ִ�.
	//if (cvs[i].imperviousR == 1) {
	//	setNoInfiltrationParameters(i);
	//	return;
	//}

	bool beenPonding = false; // 2023.04.12. ������ �ִ� ���, ponding���� ����ϱ� ���ؼ� �߰�
	if (cvs[i].flowType == cellFlowType::ChannelFlow
		&& cvs[i].stream.hCH > PONDING_DEPTH_CRITERIA_M) {
			 beenPonding = true;
	}
	else if (cvs[i].hOF > PONDING_DEPTH_CRITERIA_M) {// 2023.04.12. channel and overlandflow �������� �Ʒ��� ������ �����Ѵ�. 
		beenPonding = true;

	}

    double CONSTGreenAmpt = 0.0;
    double residuMC_ThetaR = 0.0;

    cvs[i].ssr = soilSSRbyCumulF(cvs[i].soilWaterC_tm1_m,
        cvs[i].sdEffAsWaterDepth_m); // ������ �������������� ��ȭ���� �ϴ°��� �´�.
	if (cvs[i].ssr > SOIL_SATURATION_RATIO_CRITERIA)	{
		cvs[i].isAfterSaturated = 1;
		cvs[i].ifF_mPdt = 0;
		cvs[i].ifRatef_mPsec = 0;
		cvs[i].rfEff_dt_m = cvs[i].rfApp_mPdt;
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

        double infiltrationF_mPdt_max=0.0;
        bool isPonding = false;
		if (cvs[i].ifRatef_tm1_mPsec >= cvs[i].rfiRead_tm1_mPsec || beenPonding == false) {
			// ���� �ð������� ħ������ ���� �ð������� ���찭������ �Ǵٸ�, �׸��� ponding ���°� �ƴ϶��, ��� ����� ħ����
			infiltrationF_mPdt_max = cvs[i].rfApp_mPdt;// 0 �̻��� �����
		}
        else {
            // ���� �ð������� ħ������ ���� �ð������� ���찭������ ���ų� �۾Ҵٸ� Ȥ�� ponding ���¶��
            infiltrationF_mPdt_max = getinfiltrationForDtAfterPonding(i, 
                dtsec, CONSTGreenAmpt, cvs[i].hc_K_mPsec);// 0 �̻��� �����
            isPonding = true;
        }                       
        if (infiltrationF_mPdt_max < 0) {
            infiltrationF_mPdt_max = 0; // �̰� Ȯ�� �ʿ�
        }
        else {
            double dF = cvs[i].sdEffAsWaterDepth_m - cvs[i].soilWaterC_tm1_m;
            if (dF < 0) {
                infiltrationF_mPdt_max = 0;
            }
            else if (dF < infiltrationF_mPdt_max) {
                infiltrationF_mPdt_max = dF;
            }
        }
            cvs[i].ifF_mPdt = infiltrationF_mPdt_max;

        // ���� ħ�������� dt ���ȿ� �߰��� ħ������ ���Ѵ�.
        cvs[i].soilWaterC_m = cvs[i].soilWaterC_tm1_m + cvs[i].ifF_mPdt;
        // ��������� ���� ħ������ �̿��ؼ� �̿� ���� ���ټ� ħ������ ����Ѵ�.
        if (cvs[i].soilWaterC_m <= 0) {
            cvs[i].ifRatef_mPsec = 0;
            cvs[i].soilWaterC_m = 0;
        }
        else {
            cvs[i].ifRatef_mPsec = cvs[i].hc_K_mPsec 
                * (1 + CONSTGreenAmpt / cvs[i].soilWaterC_m);
        }

        // �̰�쿡�� ħ���� ������.. ����� ��� ���� ����.. ħ���� ��ǥ�� ���� ���¿��� �߻�
        if (cvs[i].isAfterSaturated == 1 
            && (isPonding == true || cvs[i].ssr > SOIL_SATURATION_RATIO_CRITERIA)) {
            cvs[i].rfEff_dt_m = cvs[i].rfApp_mPdt;
        }
        else {
            double effRF_dt_m = cvs[i].rfApp_mPdt - cvs[i].ifF_mPdt;
            if (effRF_dt_m > cvs[i].rfApp_mPdt) {
                effRF_dt_m = cvs[i].rfApp_mPdt;
            }
            if (effRF_dt_m < 0) {
                effRF_dt_m = 0;
            }
            cvs[i].rfEff_dt_m = effRF_dt_m;
            if (cvs[i].imperviousR < 1) {
                cvs[i].rfEff_dt_m = cvs[i].rfEff_dt_m * (1 - cvs[i].imperviousR) 
                    + cvs[i].rfApp_mPdt * cvs[i].imperviousR;
                if (cvs[i].rfEff_dt_m > cvs[i].rfApp_mPdt) {
                    cvs[i].rfEff_dt_m = cvs[i].rfApp_mPdt;
                }
            }
        }
    }
    // ��ȿ���췮�� ����� �������Ƿ�, ������� ���� ħ��, ���찭�� ���� tM1 ������ �����Ѵ�.    
    cvs[i].ssr = soilSSRbyCumulF(cvs[i].soilWaterC_m, 
        cvs[i].sdEffAsWaterDepth_m); // ������ �������������� ��ȭ���� �ϴ°��� �´�.
    if (cvs[i].ssr > SOIL_SATURATION_RATIO_CRITERIA) {
        cvs[i].isAfterSaturated = 1;
    }
    cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_m;
    cvs[i].ifRatef_tm1_mPsec = cvs[i].ifRatef_mPsec;
    cvs[i].rfiRead_tm1_mPsec = cvs[i].rfiRead_After_iniLoss_mPsec;
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
		conCriteria = (CI_n * TOLERANCE);
		if (err < conCriteria) {
			double dF = CI_nP1 - constCI_tm1;
			if (dF < 0.0) { dF = 0.0; }
			return dF;
		}
		CI_n = CI_nP1;
	}
	double dFr = CI_n - constCI_tm1;
	if (dFr < 0.0) { return 0.0; }
	else { return dFr; }
}


// ���� cv�� Return flow�� ������� ���ԵǴ� ssflwo�� ����ϰ�, 
// ���� cv�� ssf�� ���� ���� �����Է����� ����Ѵ�.
double calRFlowAndSSFlow(int i,
    int dtsec, double dy_m)
{
    double dx_m = cvs[i].cvdx_m;
    double SSFfromCVw_m;
    double SSFAddedToMeFromCVw_m;
    double RflowBySSFfromCVw_m2=0.0; // �̰� return flow
    SSFfromCVw_m = totalSSFfromCVwOFcell_m3Ps(i) * dtsec / (dy_m * dx_m); // ���Ե� �ѷ� ��� �� ���ɰ��
    double ssr = soilSSRbyCumulF(cvs[i].soilWaterC_tm1_m,
                     cvs[i].sdEffAsWaterDepth_m);
	// ����� ��ǥ�� ���⿡ ���� ���� ���� Rflow �⿩�� ���
	if (ssr > SOIL_SATURATION_RATIO_CRITERIA) { // �̰��� ������� ���Ե� SSF ��ü�� return flow
        RflowBySSFfromCVw_m2 = SSFfromCVw_m * dy_m;
        SSFAddedToMeFromCVw_m = 0;
    }
    else {
		RflowBySSFfromCVw_m2 = SSFfromCVw_m * ssr * dy_m; //����*��
		SSFAddedToMeFromCVw_m = SSFfromCVw_m * (1 - ssr);
		
		// 2023.04.13 ��ǥ�� �̼� �ӵ��� ������.. �Ʒ��� ���� ������..
		//double maxSSF_m = cvs[i].sdEffAsWaterDepth_m - cvs[i].soilWaterC_tm1_m; // ���� ���� ������
		//if (maxSSF_m < 0) maxSSF_m = 0;
		//if (SSFfromCVw_m < maxSSF_m) {// �̰�쿡�� ��� Ⱦ�������� �̼�
		//	SSFAddedToMeFromCVw_m = SSFfromCVw_m;
		//	RflowBySSFfromCVw_m2 = 0.0;
		//}
		//else {
		//	SSFAddedToMeFromCVw_m = maxSSF_m;
		//	RflowBySSFfromCVw_m2 = (SSFfromCVw_m - maxSSF_m) * dy_m; //����*��
		//}
    }
    // ������� ���Ե� ��ǥ�� ���� ���� ���ϰ�..
    cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m
        + SSFAddedToMeFromCVw_m;

    // ������ ħ�� ���̸� �̿��ؼ� return flow ����ϰ�..
    if (cvs[i].soilWaterC_tm1_m > cvs[i].sdEffAsWaterDepth_m) {
        // �Ϸ��� �̼۵� ��ǥ�� ���ⷮ�� ���� ���� ��ȭ���� ���̸� �ʰ��ϸ�, �ʰ����� returnflow
        RflowBySSFfromCVw_m2 = (cvs[i].soilWaterC_tm1_m
            - cvs[i].sdEffAsWaterDepth_m)
            * dy_m + RflowBySSFfromCVw_m2;
		cvs[i].soilWaterC_tm1_m = cvs[i].sdEffAsWaterDepth_m;
    }

    // ���� ���� ��ǥ�� ����. 
    cvs[i].QSSF_m3Ps = cvs[i].soilWaterC_tm1_m / cvs[i].porosity_Eta
        * cvs[i].hc_K_mPsec * sin(atan(cvs[i].slopeOF)) * dy_m;
    // �Ϸ��� ������ ������ ���� ����ħ����
    cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m
        - cvs[i].QSSF_m3Ps / (dy_m * cvs[i].cvdx_m) * dtsec;
	if (cvs[i].soilWaterC_tm1_m < 0) {
		cvs[i].soilWaterC_tm1_m = 0;
	}
    return RflowBySSFfromCVw_m2; 
}


double calBFlowAndGetCSAaddedByBFlow(int i, int dtsec, double cellSize_m)
{
	double deltaSoilDepthofUAQ_m = 0.0; // B��(���Ǿ� �����)�� ���� ��ȭ. A���� ��ȭ�� ���, B������ ħ���Ǹ鼭 �߻�, �������� ħ������
	double soilDepthPercolated_m = 0.0;
	double waterDepthPercolated_m;
	double csa = 0.0;
	int wsid = cvps[i].wsid;
	int calLayerA = 1;
	double ssr = soilSSRbyCumulF(cvs[i].soilWaterC_tm1_m,
		cvs[i].sdEffAsWaterDepth_m);
	if (ssr > 0.0) { // A ���� ��ȭ���� �ʾƵ� ħ���߻�
		soilDepthPercolated_m = Kunsaturated(cvs[i]) * dtsec * ssr; // ������ �̵��� �Ÿ�. 
	}
	else {
		soilDepthPercolated_m = 0;
		calLayerA = 0;
	}

// 2023.04.19. �ϵ�, ����, ���� ����� �׻� ħ��, Layer B���� Ⱦ���� �̼��� �߻��ϴ� ������ ����.
	if (cvs[i].sd_m <= soilDepthPercolated_m && calLayerA ==1 ) {
		soilDepthPercolated_m = cvs[i].sd_m;
		cvs[i].soilWaterC_tm1_m = 0; // A ���� �ִ� ��� ������ B ������ ħ���� ����̹Ƿ�..
		calLayerA = 0;
	}

	if (calLayerA == 1) {
		waterDepthPercolated_m = soilDepthPercolated_m * cvs[i].effPorosity_ThetaE;
		if (waterDepthPercolated_m < cvs[i].soilWaterC_tm1_m) {
			cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m - waterDepthPercolated_m;
		}
		else {// ������ �ִ� ��� ���� �Է� �̻��� ħ������ �ʴ´�. 
			cvs[i].soilWaterC_tm1_m = 0.0;
			waterDepthPercolated_m = cvs[i].soilWaterC_tm1_m;
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
		if (cvs[i].stream.hCH > 0.0) {
			double dHinUAQ_m = soilDepthPercolated_m
				* cvs[i].porosity_Eta;
			if (dHinUAQ_m > cvs[i].stream.hCH) { // ��õ ���ɺ��� �� ���� ���� ħ���Ǿ��� ���, ��õ ���ɸ�ŭ�� ħ���� ������ �Ѵ�.
				dHinUAQ_m = cvs[i].stream.hCH;
			}
			cvs[i].hUAQfromChannelBed_m = cvs[i].hUAQfromChannelBed_m
				+ dHinUAQ_m;
			if (cvs[i].hUAQfromChannelBed_m > 0.0) {
				// ���������� ���� ���ִ� �������� �߻��ϹǷ�.. ��ȭ���������� ����
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
			else { csa = 0.0; }
		}
		else { csa = 0.0; }
		cvs[i].hUAQfromChannelBed_m = cvs[i].hUAQfromChannelBed_m - csa
			/ cvs[i].stream.chBaseWidth / cvs[i].porosity_Eta;
		cvs[i].hUAQfromBedrock_m = cvs[i].hUAQfromBedrock_m
			- csa / cvs[i].stream.chBaseWidth
			/ cvs[i].porosity_Eta;
		if (cvs[i].hUAQfromChannelBed_m < 0.0) {
			cvs[i].hUAQfromChannelBed_m = 0.0;
		}
		if (cvs[i].hUAQfromBedrock_m < 0.0) {
			cvs[i].hUAQfromBedrock_m = 0.0;
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
            chCSA = SSFlow_cms / cvs[i].stream.uCH; // ���� �ð��� ���� ����
            // ������ ���� ���.. �߻� ���ɼ� �����Ƿ�.. ���� �ܸ��� ���� ū �ܸ������� ���Ե� ���.. 
            // ���� �ܸ������� ���ԵǴ� ������ ����..
            // ��, ��ǥ�� ������ �⿩ �ܸ����� ���� ��õ�ܸ������� Ŭ �� ����.
            if (chCSA > cvs[i].stream.csaCH)
                chCSA = cvs[i].stream.csaCH;
            return chCSA;
        }
    }
    // �̰�쿡�� �ϵ��� �⿩���� �ʴ� ������ ����
    // �� ���쿡 ���� �ϵ��� ���� �߻��� ���۵��� �ʾҴµ�(�ϵ��� �������� ���).. 
    // ��ǥ�� ���⿡ ���� �⿩�� ���� �� ����.
    return 0;
}


double Kunsaturated(cvAtt cv)
{
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
    if (cv.coefUK <= 0) {
        return Ks * 0.1;
    }
    if (ssr > SOIL_SATURATION_RATIO_CRITERIA) {
        return Ks;
    }
    else {
        switch (cv.ukType) {
        case unSaturatedKType::Linear: {
            return Ks * ssr * cv.coefUK;
        }
        case unSaturatedKType::Exponential: {
            return Ks * pow(ssr, cv.coefUK);
        }
        case unSaturatedKType::Constant: {
			// constant�� �����ϰ�, ����� 1�� �Է��ϸ�
			// (default �������) * (��������������)�� ���� �״�� ����ȴ�.
            return Ks * cv.coefUK;
        }
        default: {
            return Ks * ssr * cv.coefUK;
        }
        }
    }
    return Ks;
}

void calBFLateralMovement(int i,
    int facMin, double dY_m, double dtsec)
{
    double cumulBFq_m3Ps = 0;
    double dhUAQ_m;
    double dX_m = cvs[i].cvdx_m;
    for (int idx : cvs[i].neighborCVidxFlowintoMe) {
        if (cvs[idx].flowType == cellFlowType::OverlandFlow) {
            cumulBFq_m3Ps = cumulBFq_m3Ps 
                + cvs[idx].bfQ_m3Ps;// ���ΰ�簡 ���� ��ǥ�� ���� ���� ������ ����
        }
    }
    if (cvs[i].fac == facMin) {
		dX_m = dX_m / 2.0;
    }
    dhUAQ_m = cumulBFq_m3Ps * dtsec / (dY_m * dX_m)
        / cvs[i].effPorosity_ThetaE;  // �̰� �˻�ü���� �����ϰ� ���� �ִ� ������ȭ��
    cvs[i].hUAQfromBedrock_m = cvs[i].hUAQfromBedrock_m 
        + dhUAQ_m; // �̰� ���ϼ����� ��� ���� ��ȭ
    double toBed = cvs[i].sdToBedrock_m - cvs[i].sd_m;
    if (cvs[i].hUAQfromBedrock_m > toBed) {
        cvs[i].hUAQfromBedrock_m = toBed;
    }
    // �������� ��꿡���� ��ȭ���������� ����. ��ȭ�� �������� �߻�
    cvs[i].bfQ_m3Ps = cvs[i].hUAQfromBedrock_m 
        * cvs[i].hc_K_mPsec * sin(atan(cvs[i].slopeOF)) * dY_m;
    if (cvs[i].bfQ_m3Ps < 0) {
        cvs[i].bfQ_m3Ps = 0;
    }
}


double totalSSFfromCVwOFcell_m3Ps(int i)
{
    double SSF_m3Ps = 0.0;
    for(int idx: cvs[i].neighborCVidxFlowintoMe)    {
        if (cvs[idx].flowType == cellFlowType::OverlandFlow) {
            // ��, GRM���� Green-Ampt ������ �̿��ؼ� ħ���� ���� ����Ҷ�, 
            // ��ȭ�� ���̷� ����. ��, ���� ħ������ ��ŭ ���� �������� ������� �ʴ´�..
            // ����, �ϵ��� �⿩�ϴ� ��ǥ�� ���ⷮ ����Ҷ��� ����ħ�����̰� .saturatedSoildepth_m �� �ȴ�.
            SSF_m3Ps = SSF_m3Ps + cvs[idx].QSSF_m3Ps;
        }
    }
    return SSF_m3Ps;
}

inline double soilSSRbyCumulF(double cumulinfiltration,
    double effSoilDepth)
{
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
    cvs[i].rfEff_dt_m = cvs[i].rfApp_mPdt;
}

inline void setWaterAreaInfiltrationPars(int i)
{
    cvs[i].ifF_mPdt = 0;
    cvs[i].ifRatef_mPsec = 0;
    cvs[i].ssr = 1;
    cvs[i].rfEff_dt_m = cvs[i].rfApp_mPdt;
}

