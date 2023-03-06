#include "stdafx.h"
#include "grm.h"

extern fs::path fpnLog;

extern cvAtt* cvs;
extern domaininfo di;
extern cvpos* cvps;

extern map<int, double[12]> laiRatio;
extern double sunDurRatio[12];
extern double blaneyCriddleKData[12];
extern double svpGradient[100];

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
	default:
		writeLog(fpnLog, "ERROR : PET method is invalid (WSID ="+ to_string(cvps[i].wsid) +").\n", 1, 1);
		return;
	}
	if (cvs[i].pet_mPdt < 0) { // ������ ���� �������߻��� 0���� ������, 0���� ó��
		cvs[i].pet_mPdt = 0.0;
	}
	// ���⼭ �����Ǻ����� ���߻��� �ս�ó�� �Ѵ�.
	double aet_canopy = 0.0;
	double aet_canopy_residual = 0.0;
	double aet_soil = 0.0;
	double aet_water = 0.0;
	if (cvs[i].intcpAcc_m == 0) {// ���ܵ� ������ ������
		aet_canopy = 0.0;
		aet_canopy_residual = cvs[i].pet_mPdt * cvs[i].etCoef; // 2022.11.30. ��� �������߻��� �������� �����ϰ�, ��翡���� �սǷ� ó��
	}
	else {//���ܵ� ������ ������. ���⼭ ���߻꿡 ���� ���ܷ� ����, canopy ���� �ִ� ���� ���ɷ��� �ֱ⶧����.. ���⼭ �ݿ������ ��.
		aet_canopy = cvs[i].pet_mPdt * cvs[i].etCoef;
		if (cvs[i].intcpAcc_m < aet_canopy) {  //���ܷ��� ��� ���ߵǸ�, ���߻귮�� �ݿ��� �ʿ� ����.
			aet_canopy_residual = aet_canopy - cvs[i].intcpAcc_m; // �̺κ��� ��� ���п��� �սǷ� ó��.
			aet_canopy = cvs[i].intcpAcc_m;
			cvs[i].intcpAcc_m = 0.0;
		}
		else {
			cvs[i].intcpAcc_m = cvs[i].intcpAcc_m - aet_canopy;
			aet_canopy_residual = 0.0;
		}
	}

	if (cvs[i].lcCode == landCoverCode::WATR
		|| cvs[i].lcCode == landCoverCode::WTLD) {// ���⼭�� ����� ����Ĺ����� ���߻�		
		// aet_water�� canopy ������ �������� �ʰ�, �� ��ü���� ���߻�
		// aet_canopy�� canopy ���������� �߻�
		aet_water = cvs[i].pet_mPdt; // aet_water�� �������߻귮 �״�� ����
		aet_soil = 0.0;
	}
	else {
		// ���⼭�� ���� �Ĺ����� ���߻�
		// aet_soil�� canopy ������ �������� �ʰ�, �� ��ü���� ���߻�
		// aet_canopy�� canopy ���������� �߻�
		aet_soil = cvs[i].pet_mPdt * cvs[i].etCoef;
		aet_water = 0.0;
	}

	// ĳ���ǿ����� ���߻�� �� �� �κ�(��, ���)������ ���߻� 
	cvs[i].aet_mPdt = (aet_canopy + aet_canopy_residual) * cvs[i].canopyR
		+ aet_water * (1 - cvs[i].canopyR) + aet_soil * (1 - cvs[i].canopyR);
	cvs[i].aet_LS_mPdt = aet_canopy_residual* cvs[i].canopyR 
		+ aet_water * (1 - cvs[i].canopyR)	+ aet_soil * (1 - cvs[i].canopyR); // 2.22.11.30. ��, ��ǥ��, ��翡���� �ս� ��꿡 �̿�. aet_canopy�� �Ļ������� �߻�

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
	case cellFlowType::ChannelNOverlandFlow: {
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
			aet_residual = cvs[i].aet_LS_mPdt - cvs[i].hOF;
			cvs[i].hOF = 0;
		}
	}
	else {
		aet_residual = cvs[i].aet_LS_mPdt;
	}
	if (cvs[i].hOF == 0) {
if (cvs[i].soilWaterC_m > aet_residual) { // ���⼭�� ��翡�� �ս�ó��
	cvs[i].soilWaterC_m = cvs[i].soilWaterC_m - aet_residual;
	if (cvs[i].soilWaterC_m < 0) {
		cvs[i].soilWaterC_m = 0.0;
	}
}
else {
	cvs[i].soilWaterC_m = 0.0;
}
	}
}

void cal_h_CHbyAET(int i) {
	if (cvs[i].stream.hCH > 0) {
		if (cvs[i].stream.hCH > cvs[i].aet_LS_mPdt) { // �̰��� ��ǥ�� ���鿡�� �ս� ó��
			cvs[i].stream.hCH = cvs[i].stream.hCH - cvs[i].aet_LS_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA && cvs[i].stream.hCH>0.0) {
				cvs[i].stream.hCH = 0.0;
			}
		}
		else {
			cvs[i].aet_LS_mPdt = cvs[i].stream.hCH; // �̸�ŭ�� ���߻�ȴ�. 
			cvs[i].stream.hCH = 0; // ��� ���� �Ǿ����Ƿ� 0.
		}
	}
	else if (cvs[i].stream.hCH == 0) {// ��õ�� �ִ� �������� ����� �׻� ��ȭ�� ������ ó��. �� ��� ���߻� ������� ����.
		cvs[i].aet_LS_mPdt = 0;
	}
}

void calPET_BlaneyCriddle(int i)
{
	double k = blaneyCriddleKData[ts.tCurMonth - 1];
	double p = sunDurRatio[ts.tCurMonth - 1];
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double pet_mmPday = k * p * (0.46 * tave + 8.13);
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0;
}

void calPET_Hamon(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double sdTerm = cvs[i].sunDur_hrs / 12.0;   // �Է��ڷ� ���� hrs / day �� �״�� �����Ŀ� �̿�ȴ�.
	double es = 6.108 * exp(17.26939 * tave / (tave + 273.3));
	double svd = 216.7 * es / (tave + 273.3);
	double pet_mmPday = 0.55 * 25.4 * sdTerm * sdTerm * svd / 12.0;
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0;// �� �� ���߷��� dt �ð����� ����� ����Ѵ�.
}

void calPET_Hargreaves(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double t_diff = cvs[i].tempMaxPday - cvs[i].tempMinPday;
	if (t_diff < 0) {
		t_diff = 0.0;
	}
	double pet_mmPday = 0.0023 * cvs[i].solarRad_mm * pow(t_diff, 0.5) * (tave + 17.8);  // �Է��ڷ� ���� mm/day �� �״�� �����Ŀ� �̿�ȴ�.
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // �� �� ���߷��� dt �ð����� ����� ����Ѵ�.
}

void calPET_PriestleyTaylor(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double lv = -0.56 * tave + 597.3; // �����῭
	double rhow = 1.0; // ���� �е�
	double er = cvs[i].solarRad_mm / lv / rhow; //������  // �Է��ڷ� ���� mm/day �� �״�� �����Ŀ� �̿�ȴ�.
	double gamma = 0.0006 * tave + 0.655;//�ǽ��� ���
	int t = int(tave); //�µ��� ������ ��ȯ. �Ҽ��� ���ϴ� ����
	double svpGrad = 0.0;
	if (t >= -1 && t < 100) {
		svpGrad = svpGradient[t];//��ȭ����� ����
	}
	else {
		svpGrad = 0.347;//0�������� ��ȭ������� �̿�
	}
	double pet_cmPday = 1.28 * er * svpGrad / (svpGrad + gamma);
	cvs[i].pet_mPdt = ts.dtsec * pet_cmPday / 8640000.0; // cmPday => mPdt   // �� �� ���߷��� dt �ð����� ����� ����Ѵ�.
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
			break;
		}
		case cellFlowType::ChannelFlow: {
			cvs[i].stream.hCH = cvs[i].stream.hCH + cvs[i].smelt_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA && cvs[i].stream.hCH>0.0) {
				cvs[i].stream.hCH = WETDRY_CRITERIA;
			}
			break;
		}
		case cellFlowType::ChannelNOverlandFlow: {
			cvs[i].hOF = cvs[i].hOF + cvs[i].smelt_mPdt;
			if (cvs[i].hOF < WETDRY_CRITERIA && cvs[i].hOF>0.0) {
				cvs[i].hOF = WETDRY_CRITERIA;
			}
			cvs[i].stream.hCH = cvs[i].stream.hCH + cvs[i].smelt_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA && cvs[i].stream.hCH>0.0) {
				cvs[i].stream.hCH = WETDRY_CRITERIA;
			}
			break;
		}
		}
	}
}

