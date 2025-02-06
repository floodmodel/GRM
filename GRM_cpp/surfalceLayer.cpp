#include "stdafx.h"
#include "grm.h"

extern projectFile prj;
extern fs::path fpnLog;

extern cvAtt* cvs;
extern domaininfo di;
extern cvpos* cvps;
extern double* cvele;// ������ �ع߰�. DEM���� ���� ��. �迭 �ε�����, cv �ε����� ���� �Ѵ�.
extern map<int, double[12]> laiRatio;
extern double sunDurRatioForAday[65][13];
extern double blaneyCriddleKData[12];
//extern double svpGradient[100];

extern thisSimulation ts;

void calinterception(int i) {
	if (cvs[i].intcpMethod == InterceptionMethod::LAIRatio) {
		double intcpPossibleCurDay = cvs[i].intcpMaxWaterCanopy_m * laiRatio[cvs[i].lcCellValue][ts.tCurMonth - 1];
		if (cvs[i].canopyR > 0 && cvs[i].intcpAcc_m < intcpPossibleCurDay) { // �̶��� ���� �߻�
			double eff_rf_canopy_mPdt = 0.0;// canopy�� ����ϴ� ���찡 ����.
			cvs[i].intcpAcc_m += cvs[i].rfApp_mPdt;
			if (cvs[i].intcpAcc_m > intcpPossibleCurDay) {
				eff_rf_canopy_mPdt = cvs[i].intcpAcc_m - intcpPossibleCurDay; // �� ���̸�ŭ canopy�� ����ϴ� ���찡 �ִ�.
				cvs[i].intcpAcc_m = intcpPossibleCurDay;
			}
			cvs[i].rfApp_mPdt = cvs[i].rfApp_mPdt * (1 - cvs[i].canopyR)
				+ eff_rf_canopy_mPdt * cvs[i].canopyR;
		}
	}
}

void calET(int i) {
	switch (cvs[i].petMethod) {
	case PETmethod::BlaneyCriddle:
		calPET_BlaneyCriddle(i);
		break;
	case PETmethod::Hamon:
		calPET_Hamon(i);
		break;
	case PETmethod::Hargreaves:
		calPET_Hargreaves(i);
		break;
	case PETmethod::PriestleyTaylor:
		calPET_PriestleyTaylor(i);
		break;
	case PETmethod::FPM:
		calPET_FPM(i);
		break;
	case PETmethod::UserET:
		calPET_userET(i, prj.userETInterval_min, ts.wdUsed_userET);
		break;
	default:
		writeLogString(fpnLog, "ERROR : PET method is invalid (WSID =" + to_string(cvps[i].wsid) + ").\n", 1, 1);
		return;
	}
	if (cvs[i].pet_mPdt < 0) { // ������ ���� �������߻��� 0���� ������, 0���� ó��
		cvs[i].pet_mPdt = 0.0;
	}

	// ���⼭ �����Ǻ����� ���߻��� �ս�ó�� �Ѵ�. ũ�� ��, ���, canopy�� ����
// ���������� aet = pet * et_coeff ���� ���� ũ��(pet�� ����� ��), �׿ܿ����� aet = pet * et_coeff �� ����.
// �����Ǻ����� �����ؼ� ����ϴ� ������. ��� ��ȭ�� ��ȭ ��꿡 ����� �κ��� �����ϰ�, ���� ���� ���� Ȱ�� ���ɼ��� �����
	double aet_canopy = 0.0;
	// canopy���� ���߻� �Ǿ�� �ϴµ�, ���ܷ��� �����ؼ� ���߻��� ���� �κ�����, 
	// �� �κ��� canopy �α� ��翡�� �ö�� ������ ����Ǵ°����� ����.
	double aet_canopy_transp = 0.0;
	double aet_soil = 0.0;
	double aet_water = 0.0;
	int isWater = -1;
	double nonCanopyR = 1.0 - cvs[i].canopyR;
	if (cvs[i].lcCode == landCoverCode::WATR
		|| cvs[i].lcCode == landCoverCode::WTLD) {
		if (cvs[i].flowType == cellFlowType::OverlandFlow
			&& cvs[i].hOF > 0.0) { // �������� ������ �ִ� ���
			isWater = 1;
		}
		else	if ((cvs[i].flowType == cellFlowType::ChannelFlow
			|| cvs[i].flowType == cellFlowType::ChannelNOverlandFlow)
			&& cvs[i].stream.hCH > 0.0) { // ��õ�帧 ������ ������ �ִ� ���
			isWater = 1;
		}
	}

	if (ts.wdUsed_userET != 2) {// �������߻귮�� �����ϴ� ��찡 �ƴϸ�,
		if (cvs[i].intcpAcc_m == 0) {// ���ܵ� ������ ������
			aet_canopy = 0.0;
			aet_canopy_transp = cvs[i].pet_mPdt * cvs[i].etCoef; // 2022.11.30. ��� �������߻��� �������� �����ϰ�, ��翡���� �սǷ� ó��
		}
		else {//���ܵ� ������ ������. ���⼭ ���߻꿡 ���� ���ܷ� ����, canopy ���� �ִ� ���� ���ɷ��� �ֱ⶧����.. ���⼭ �ݿ������ ��.
			aet_canopy = cvs[i].pet_mPdt * cvs[i].etCoef;
			if (cvs[i].intcpAcc_m < aet_canopy) {  //���ܷ��� ��� ���ߵǸ�, ���߻귮�� �ݿ��� �ʿ� ����.
				aet_canopy_transp = aet_canopy - cvs[i].intcpAcc_m; // �̺κ��� ��� ���п��� �սǷ� ó��.
				aet_canopy = cvs[i].intcpAcc_m;
				cvs[i].intcpAcc_m = 0.0;
			}
			else {
				cvs[i].intcpAcc_m = cvs[i].intcpAcc_m - aet_canopy;
				aet_canopy_transp = 0.0;
			}
		}
		// aet_water�� canopy ������ �������� �ʰ�, �� ��ü���� ���߻�
		// aet_canopy�� canopy ���������� �߻�
		if (isWater == 1) {
			aet_water = cvs[i].pet_mPdt; // ���⼭�� ����� ����Ĺ����� ���߻� // aet_water�� �������߻귮 �״�� ����		
			aet_soil = 0.0;
		}
		else {
			// ���⼭�� ���� �Ĺ����� ���߻�
			aet_soil = cvs[i].pet_mPdt * cvs[i].etCoef;
			if (cvs[i].ssr == 0.0) { // �������� ������, ���߻굵 ����.
				aet_soil = 0.0;   // ������ ���ҿ� ������ ������ ���� �������߻� ������  cal_h_SWC_OFbyAET(i), cal_h_CHbyAET(i) ���⼭ �Ѵ�.
			}
			aet_water = 0.0;
		}

		// ĳ���ǿ����� ���߻�� �� �� �κ�(��, ���)������ ���߻� 
		cvs[i].aet_mPdt = (aet_canopy + aet_canopy_transp) * cvs[i].canopyR
			+ aet_water * nonCanopyR + aet_soil * nonCanopyR;
		cvs[i].aet_LS_mPdt = aet_canopy_transp * cvs[i].canopyR
			+ aet_water * nonCanopyR + aet_soil * nonCanopyR; // 2.22.11.30. ��, ��ǥ��, ��翡���� �ս� ��꿡 �̿�. aet_canopy�� �Ļ������� �߻�
	}
	else if (ts.wdUsed_userET == 2) { // ���� ���߻��� �̿��ϴ� ��쿡�� calPET_userET()���� ������ aet_mPdt ���� �״�� �̿�
		if (cvs[i].intcpAcc_m == 0) {// ���ܵ� ������ ������
			aet_canopy = 0.0;
			aet_canopy_transp = cvs[i].aet_mPdt; // 2022.11.30. ��� �������߻��� �������� �����ϰ�, ��翡���� �սǷ� ó��
		}
		else {//���ܵ� ������ ������. ���⼭ ���߻꿡 ���� ���ܷ� ����, canopy ���� �ִ� ���� ���ɷ��� �ֱ⶧����.. ���⼭ �ݿ������ ��.
			if (cvs[i].intcpAcc_m < cvs[i].aet_mPdt) {  
				aet_canopy_transp = cvs[i].aet_mPdt - cvs[i].intcpAcc_m; // �̺κ��� ��� ���п��� �սǷ� ó��.
				aet_canopy = cvs[i].intcpAcc_m; // ���ܷ��� ��� ���ߵ�
				cvs[i].intcpAcc_m = 0.0;
			}
			else { // ���ܵǾ� �ִ� ���� �� ũ��
				cvs[i].intcpAcc_m = cvs[i].intcpAcc_m - cvs[i].aet_mPdt;
				aet_canopy_transp = 0.0; // ��� ����, ������ ����.
			}
		}
		// aet_water�� canopy ������ �������� �ʰ�, �� ��ü���� ���߻�
		// aet_canopy�� canopy ���������� �߻�
		if (isWater == 1) {
			aet_water = cvs[i].aet_mPdt; // ���⼭�� ����� ����Ĺ����� ���߻� // aet_water�� �������߻귮 �״�� ����		
			aet_soil = 0.0;
		}
		else {
			// ���⼭�� ���� �Ĺ����� ���߻�
			aet_soil = cvs[i].aet_mPdt;
			if (cvs[i].ssr == 0.0) { // �������� ������, ���߻굵 ����.
				aet_soil = 0.0; // ������ ���ҿ� ������ ������ ���� �������߻� ������  cal_h_SWC_OFbyAET(i), cal_h_CHbyAET(i) ���⼭ �Ѵ�.
			}
			aet_water = 0.0;
		}
		cvs[i].aet_LS_mPdt = aet_canopy_transp * cvs[i].canopyR
			+ aet_water * nonCanopyR + aet_soil * nonCanopyR; // 2.22.11.30. ��, ��ǥ��, ��翡���� �ս� ��꿡 �̿�. aet_canopy�� �Ļ������� �߻�

	}
	// canopy������ ���߻꿡 ���� �������� ������ ����
	// ���⼭�� ���߻꿡 ���� ��ǥ��, ���鿡���� �ս� ��� �ʿ�. 2022.11.30. cvs[i].aet_LS_mPdt �̿�
	switch (cvs[i].flowType) {
	case cellFlowType::OverlandFlow: {
		cal_h_SWC_OFbyAET(i);
		break;
	}
	case cellFlowType::ChannelFlow: {
		cal_h_CHbyAET(i);
		break;
	}
	case cellFlowType::ChannelNOverlandFlow: { // �� ���� ��õ�� ������ �����п� �������̹Ƿ�, ��õ �߽����� �����Ѵ�.
		cal_h_SWC_OFbyAET(i);
		cal_h_CHbyAET(i);
		break;
	}
	}
}

void cal_h_SWC_OFbyAET(int i) {
	double aet_residual = 0.0;
	if (cvs[i].hOF > 0) {
		if (cvs[i].hOF > cvs[i].aet_LS_mPdt) { // �̰��� ��ǥ�� ���鿡�� �ս� ó��
			cvs[i].hOF = cvs[i].hOF - cvs[i].aet_LS_mPdt;
			if (cvs[i].hOF < WETDRY_CRITERIA && cvs[i].hOF>0.0) {
				cvs[i].hOF = WETDRY_CRITERIA;
			}
		}
		else {
			aet_residual = cvs[i].aet_LS_mPdt - cvs[i].hOF; // ������ ������ ��� �����ϰ� ���� ���� ���ɷ�
			cvs[i].hOF = 0;// ��� ���� �Ǿ����Ƿ� 0.
		}
	}
	else {
		aet_residual = cvs[i].aet_LS_mPdt;
	}

	// cellFlowType::ChannelNOverlandFlow �� cellFlowType::ChannelFlow ������
	// ��õ CV�� ���ؼ��� ������ ��ȭ �� �׿� ���� aet ������� ����Ѵ�.
	if (cvs[i].hOF == 0 && cvs[i].flowType == cellFlowType::OverlandFlow ) {
		if (cvs[i].soilWaterC_tm1_m > aet_residual) { // ���⼭�� ��翡�� �ս�ó��
			cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m - aet_residual;
		}
		else {
			if (ts.wdUsed_userET == 2) {// �̰��� ���� ���߻��� �������� �ʴ´�.
				cvs[i].soilWaterC_tm1_m = 0.0;
			}
			else {
				double aet_difficit = aet_residual - cvs[i].soilWaterC_tm1_m; // �������� �����ؼ� �̸�ŭ�� ���ߵ��� �ʴ´�.
				cvs[i].soilWaterC_tm1_m = 0.0;
				cvs[i].aet_mPdt = cvs[i].aet_mPdt - aet_difficit; // ���� ���߻��� �����Ѵ�.
				cvs[i].aet_LS_mPdt = cvs[i].aet_LS_mPdt - aet_difficit; // ���� ���߻��� �����Ѵ�.
			}

		}
	}
}

void cal_h_CHbyAET(int i) {
	double aet_residual = 0.0;
	if (cvs[i].stream.hCH > 0) {
		if (cvs[i].stream.hCH > cvs[i].aet_LS_mPdt) { // �̰��� ��õ ���鿡�� �ս� ó��
			cvs[i].stream.hCH = cvs[i].stream.hCH - cvs[i].aet_LS_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA && cvs[i].stream.hCH>0.0) {
				cvs[i].stream.hCH = WETDRY_CRITERIA;
			}
		}
		else {
			aet_residual = cvs[i].aet_LS_mPdt - cvs[i].stream.hCH;// ��õ�� ������ ��� �����ϰ� ���� ���� ���ɷ�
			cvs[i].stream.hCH = 0; // ��� ���� �Ǿ����Ƿ� 0.
		}
	}
	else {
		aet_residual = cvs[i].aet_LS_mPdt; 
	}

	if (cvs[i].stream.hCH == 0) {
		if (cvs[i].soilWaterC_tm1_m > aet_residual) { // ���⼭�� ��翡�� �ս�ó��
			cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m - aet_residual;
		}
		else {
			if (ts.wdUsed_userET == 2) {// �̰��� ���� ���߻��� �������� �ʴ´�.
				cvs[i].soilWaterC_tm1_m = 0.0;
			}
			else {
				double aet_difficit = aet_residual - cvs[i].soilWaterC_tm1_m; // �������� �����ؼ� �̸�ŭ�� ���ߵ��� �ʴ´�.
				cvs[i].soilWaterC_tm1_m = 0.0;
				cvs[i].aet_mPdt = cvs[i].aet_mPdt - aet_difficit; // ���� ���߻��� �����Ѵ�.
				cvs[i].aet_LS_mPdt = cvs[i].aet_LS_mPdt - aet_difficit; // ���� ���߻��� �����Ѵ�.
			}

		}
	}
}

void calPET_BlaneyCriddle(int i)
{
	int month_apply = ts.tCurMonth;
	if (cvps[i].intLat_degree < 0) { // ���ݱ��� �ش��ϴ� �������� ��ȯ
		month_apply += 6;
		if (month_apply > 12) {
			month_apply = month_apply - 12;
		}
	}
	double k = blaneyCriddleKData[month_apply - 1];
	int lat_abs = abs(cvps[i].intLat_degree);	
	if (lat_abs > 64) { lat_abs = 64; } // 64�� �̻󿡼��� 64���� ���� ����Ѵ�.
	double p = sunDurRatioForAday[lat_abs][month_apply]; //[x][0]�� 0���� dummy ���� �ԷµǾ� �����Ƿ�, �ش���� �״�� ����Ѵ�.
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double pet_mmPday = k * p * (0.46 * tave + 8.13);
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0;
}

void calPET_Hamon(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double sdTerm = cvs[i].daytimeLength_hrs / 12.0;   // �Է��ڷ� ���� hrs / day �� �״�� �����Ŀ� �̿�ȴ�.
	double es = 0.6108 * exp(17.27 * tave / (tave + 237.3)); // es : kPa
	double svd = 216.7 * es / (tave + 273.3);  //svd : gr/m^3==> �� ���� �״�� ����ǰ� �Ѵ�.
	// 0.0055*25.4 = 0.1397 : inch -> mm�� ��ȯ�ϱ� ���ؼ� 25.4�� �����ش�.
	double pet_mmPday = 0.1397 * sdTerm * sdTerm * svd;
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0;// 1/86400000 = 1/1000/24/60/60.	�� �� ���߷��� dt �ð����� ����� ����Ѵ�.
}

void calPET_Hargreaves(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double t_diff = cvs[i].tempMaxPday - cvs[i].tempMinPday;
	if (t_diff < 0.0) {
		t_diff = 0.0;
	}
	// 2024.11.25. ���û �ϻ緮 �ڷ� ���� MJ/m^2�� �״�� �����ϴ� ������ ����
	//double lv = 2.45; // �����῭ FAO ���尪. MJ/kg
	// 0.000939 = 0.0023 / 2.45
	//double pet_mmPday = 0.000939 * cvs[i].solarRad_MJpM2 * pow(t_diff, 0.5) * (tave + 17.8);  // �Է��ڷ� ���� MJ/m^2�� �״�� ����
	//double Ra = getRa(cvs[i].lat_rad);
	double Ra = getRa(cvps[i].lat_rad);
	double pet_mmPday = 0.000939 * Ra * pow(t_diff, 0.5) * (tave + 17.8);  // �Է��ڷ� ���� MJ/m^2�� �״�� ����
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // 1/86400000 = 1/1000/24/60/60.  �� �� ���߷��� �� ������ ����� ����Ѵ�.
}


void calPET_PriestleyTaylor(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;	
	double gamma = getPsychroConst(i);
	double parTMP = tave + 237.3;
	double es = 0.6108 * exp(17.27 * tave / parTMP); // es : kPa
	double svpGrad = 4098 * es / parTMP / parTMP;  // kPa/degreeC  // 2503.06 = 4098 * 0.6108
	//double pet_mmPday = 1.28 * svpGrad / (svpGrad + gamma) * cvs[i].solarRad_MJpM2 / lv;
	double ea = getActualVP_Ea(i);
	double Rn = getRn(i, ea); // net radiation
	double pet_mmPday = 0.52245 * Rn * svpGrad / (svpGrad + gamma); // 0.52245 = 1.28 / 2.45,   lv = 2.45; // �����῭ FAO ���尪. MJ/kg
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // mmPday => mPdt   1/86400000 = 1/1000/24/60/60. // �� �� ���߷��� �ʴ����� ����� ����Ѵ�.
}


void calPET_FPM(int i) { // FAO Penman-Monteith
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double gamma = getPsychroConst(i);
	double parTMP = tave + 237.3;
	double es = 0.6108 * exp(17.27 * tave / parTMP); // es : kPa	
	double svpGrad = 4098 *es/ parTMP / parTMP;  // kPa/degreeC  // 2503.06 = 4098 * 0.6108	
	double ea = getActualVP_Ea(i);
	//double term1 = 0.408 * svpGrad * cvs[i].solarRad_MJpM2;
	double Rn = getRn(i, ea); // net radiation
	double term1 = 0.408 * svpGrad * Rn;
	double term2 = gamma * 900 * cvs[i].windSpeed_mps * (es - ea) / (tave + 273);
	double term3 = svpGrad + gamma * (1 + 0.34 * cvs[i].windSpeed_mps);
	double pet_mmPday = (term1 + term2) / term3;		
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // mmPday => mPdt   1/86400000 = 1/1000/24/60/60. // �� �� ���߷��� �ʴ����� ����� ����Ѵ�.
}

void calPET_userET(int i, int dataInterval_min, int whatUserData) {
	double userET_mPs = cvs[i].userET_mm / dataInterval_min / 60000.0; // 60000 = 60 sec * 1000 mm // mPsec�� ��ȯ
	if (whatUserData == 1) { // PET �̸�
		cvs[i].pet_mPdt = ts.dtsec * userET_mPs; // mPdt �� ��ȯ
	}
	else if (whatUserData == 2) { // AET �̸�
		cvs[i].aet_mPdt = ts.dtsec * userET_mPs; // mPdt �� ��ȯ
		cvs[i].pet_mPdt = 0;
	}
}

// psychrometric constant. �ǽ��� ���
double getPsychroConst(int i) {
	double par1 = (293 - 0.0065 * cvele[i]) / 293;
	double airP = 101.3 * pow(par1, 5.26);  //��� kPa/degreeC ==> �� 1��� = 101.3kPa == 1013mb
	double gamma = 0.000665 * airP;//�ǽ��� ��� 
	return gamma;
}

// psychrometric constant
double getActualVP_Ea(int i) {
	double parEa = 17.27 * cvs[i].dewPointTemp / (cvs[i].dewPointTemp + 237.3);
	double ea = 0.6108 * exp(parEa); // ea : kPa	
	return ea;
}

//extraterrestrial radiation MJ/m^2/day
double getRa(double lat_rad) {
	double coef_dOy = 0.017214 * ts.tDayOfYear; // 0.017214 = 2*3.141593 / 365
	double dr = 1 + 0.33 * cos(coef_dOy);
	double sunSlope_rad = 0.409 * sin(coef_dOy - 1.39); // delta
	double ws_rad = acos((-1 * tan(lat_rad)) * tan(sunSlope_rad)); // sunlight angle when sunset
	// 1440 = 24*60,         37.58603 = 1440*0.0820 / 3.141593 ,       Gsc = 0.0820
	double term1 = 37.58603 * dr;
	double term2 = ws_rad * sin(lat_rad) * sin(sunSlope_rad) + cos(lat_rad) * cos(sunSlope_rad) * sin(ws_rad);
	double Ra = term1 * term2;
	return Ra;
}

// net radiation MJ/m^2/day
double getRn(int i, double ea) {
	double RnS = 0.77 * cvs[i].solarRad_MJpM2; // 0.77 = 1 - 0.23
	//double parEa = 17.27 * cvs[i].dewPointTemp / (cvs[i].dewPointTemp + 237.3);
	//double ea = 0.6108 * exp(parEa); // es : kPa	
	double Rso = (0.75 + 0.00002 * cvele[i]) * getRa(cvps[i].lat_rad);
	double tmaxK = cvs[i].tempMaxPday + +273.16;
	double tminK = cvs[i].tempMinPday + +273.16;
	double term1 = (pow(tmaxK, 4) + pow(tminK, 4)) / 2;
	double term2 = 0.34 - 0.14 * sqrt(ea);
	double term3 = 1.35 * cvs[i].solarRad_MJpM2 / Rso - 0.35;
	double RnL = 0.0000000049030 * term1 * term2 * term3; // Stefan-Boltzmann constant : 4.903 * 10^-9
	double Rn = RnS - RnL;
	return Rn;
}


void calSnowMelt(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	if (tave < cvs[i].smeltTSR) {// �̰�� snow pack ����
		cvs[i].spackAcc_m += cvs[i].rfApp_mPdt;
		cvs[i].rfApp_mPdt = 0; // ������ ��� snow pack�� ��ȯ
	}

	cvs[i].smelt_mPdt = 0.0;
	if (cvs[i].spackAcc_m > 0) {
		double sm_mmPday = 0.0;
		double cSM = cvs[i].smeltCoef; //������� / ������ �Ű�����
		double sCov = cvs[i].snowCovR; // Snow coverage ratio / ������ �Ű�����
		double tMelt = cvs[i].smeltingT;// �����µ� / ������ �Ű�����
		double tAve = (cvs[i].spackTemp + cvs[i].tempMaxPday) / 2.0; // ��� �Ͽ����� snow pack �µ�, �� �ְ���

		if (tAve > tMelt) {
			//if (cvs[i].smMethod == SnowMeltMethod::Anderson) { // ����� Anderson �� ����
			sm_mmPday = cSM * sCov * (tAve - tMelt);
			//}
		}
		else {// tAve �� �����µ����� ���� ���===> ���� ����.
			sm_mmPday = 0;
		}
		cvs[i].smelt_mPdt = ts.dtsec * sm_mmPday / 86400000.0; // mmPday => mPdt

		if (cvs[i].spackAcc_m > 0 && cvs[i].smelt_mPdt > 0) {
			if (cvs[i].spackAcc_m > cvs[i].smelt_mPdt) {
				cvs[i].spackAcc_m = cvs[i].spackAcc_m - cvs[i].smelt_mPdt;
			}
			else {// �̰��� snow pack �� ��������.
				cvs[i].smelt_mPdt = cvs[i].spackAcc_m; // �ִ� �������� snow pack ���� ��ŭ�̴�.
				cvs[i].spackAcc_m = 0.0;
			}
		}

		// ������ ����� ������ �ʰ�.. ��ǥ�� �������� �����ش�..
		switch (cvs[i].flowType) {
		case cellFlowType::OverlandFlow: {
			cvs[i].hOF = cvs[i].hOF + cvs[i].smelt_mPdt;
			if (cvs[i].hOF < WETDRY_CRITERIA ) { cvs[i].hOF = 0.0; }
			break;
		}
		case cellFlowType::ChannelFlow: {
			cvs[i].stream.hCH = cvs[i].stream.hCH + cvs[i].smelt_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA) { cvs[i].stream.hCH = 0.0; }
			break;
		}
		case cellFlowType::ChannelNOverlandFlow: {
			cvs[i].hOF = cvs[i].hOF + cvs[i].smelt_mPdt;
			if (cvs[i].hOF < WETDRY_CRITERIA) { cvs[i].hOF = 0.0; }
			cvs[i].stream.hCH = cvs[i].stream.hCH + cvs[i].smelt_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA) { cvs[i].stream.hCH = 0.0; }
			break;
		}
		}
	}
}


