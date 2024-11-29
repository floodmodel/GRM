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
		if (cvs[i].canopyR > 0 && cvs[i].intcpAcc_m < intcpPossibleCurDay) { // 이때는 차단 발생
			double eff_rf_canopy_mPdt = 0.0;// canopy를 통과하는 강우가 없다.
			cvs[i].intcpAcc_m += cvs[i].rfApp_mPdt;
			if (cvs[i].intcpAcc_m > intcpPossibleCurDay) {
				eff_rf_canopy_mPdt = cvs[i].intcpAcc_m - intcpPossibleCurDay; // 이 차이만큼 canopy를 통과하는 강우가 있다.
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
	if (cvs[i].pet_mPdt < 0) { // 위에서 계산된 잠재증발산이 0보다 작으면, 0으로 처리
		cvs[i].pet_mPdt = 0.0;
	}
	// 여기서 토지피복별로 증발산을 손실처리 한다.
	double aet_canopy = 0.0;
	double aet_canopy_residual = 0.0;
	double aet_soil = 0.0;
	double aet_water = 0.0;
	if (cvs[i].intcpAcc_m == 0) {// 차단된 강수가 없으면
		aet_canopy = 0.0;
		aet_canopy_residual = cvs[i].pet_mPdt * cvs[i].etCoef; // 2022.11.30. 모든 잠재증발산은 증산으로 간주하고, 토양에서의 손실로 처리
	}
	else {//차단된 강수가 있으면. 여기서 증발산에 의해 차단량 감소, canopy 에서 최대 차단 가능량이 있기때문에.. 여기서 반영해줘야 함.
		aet_canopy = cvs[i].pet_mPdt * cvs[i].etCoef;
		if (cvs[i].intcpAcc_m < aet_canopy) {  //차단량이 모두 증발되면, 증발산량을 반영할 필요 있음.
			aet_canopy_residual = aet_canopy - cvs[i].intcpAcc_m; // 이부분은 토양 수분에서 손실로 처리.
			aet_canopy = cvs[i].intcpAcc_m;
			cvs[i].intcpAcc_m = 0.0;
		}
		else {
			cvs[i].intcpAcc_m = cvs[i].intcpAcc_m - aet_canopy;
			aet_canopy_residual = 0.0;
		}
	}

	if (cvs[i].lcCode == landCoverCode::WATR
		|| cvs[i].lcCode == landCoverCode::WTLD) {// 여기서는 수면과 수상식물에서 증발산		
		// aet_water는 canopy 영역을 제외하지 않고, 셀 전체에서 증발산
		// aet_canopy는 canopy 영역에서만 발생
		aet_water = cvs[i].pet_mPdt; // aet_water는 잠재증발산량 그대로 적용
		aet_soil = 0.0;
	}
	else {
		// 여기서는 토양과 식물에서 증발산
		// aet_soil은 canopy 영역을 제외하지 않고, 셀 전체에서 증발산
		// aet_canopy는 canopy 영역에서만 발생
		aet_soil = cvs[i].pet_mPdt * cvs[i].etCoef;
		aet_water = 0.0;
	}

	// 캐노피에서의 증발산과 그 외 부분(물, 토양)에서의 증발산 
	cvs[i].aet_mPdt = (aet_canopy + aet_canopy_residual) * cvs[i].canopyR
		+ aet_water * (1 - cvs[i].canopyR) + aet_soil * (1 - cvs[i].canopyR);
	cvs[i].aet_LS_mPdt = aet_canopy_residual* cvs[i].canopyR 
		+ aet_water * (1 - cvs[i].canopyR)	+ aet_soil * (1 - cvs[i].canopyR); // 2.22.11.30. 물, 지표면, 토양에서의 손실 계산에 이용. aet_canopy는 식생에서만 발생

	// canopy에서의 증발산에 의한 물수지는 위에서 계산됨
	// 여기서는 증발산에 의한 지표면, 수면에서의 손실 계산 필요. 2022.11.30. cvs[i].aet_LS_mPdt 이용
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
		if (cvs[i].hOF > cvs[i].aet_LS_mPdt) { // 이경우는 지표면 수면에서 손실 처리
			cvs[i].hOF = cvs[i].hOF - cvs[i].aet_LS_mPdt;
			if (cvs[i].hOF < WETDRY_CRITERIA && cvs[i].hOF>0.0) {
				cvs[i].hOF = WETDRY_CRITERIA;
			}
		}
		else {
			aet_residual = cvs[i].aet_LS_mPdt - cvs[i].hOF; // 지면의 수분이 모두 증발하고 남은 증발 가능량
			cvs[i].hOF = 0;// 모두 증발 되었으므로 0.
		}
	}
	else {
		aet_residual = cvs[i].aet_LS_mPdt;
	}
	if (cvs[i].hOF == 0) {
		if (cvs[i].soilWaterC_tm1_m > aet_residual) { // 여기서는 토양에서 손실처리
			cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m - aet_residual;
			if (cvs[i].soilWaterC_tm1_m < 0) {
				cvs[i].soilWaterC_tm1_m = 0.0;
			}
		}
		else {
			cvs[i].soilWaterC_tm1_m = 0.0;
		}
	}
}

void cal_h_CHbyAET(int i) {
	double aet_residual = 0.0;
	if (cvs[i].stream.hCH > 0) {
		if (cvs[i].stream.hCH > cvs[i].aet_LS_mPdt) { // 이경우는 지표면 수면에서 손실 처리
			cvs[i].stream.hCH = cvs[i].stream.hCH - cvs[i].aet_LS_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA && cvs[i].stream.hCH>0.0) {
				cvs[i].stream.hCH = WETDRY_CRITERIA;
			}
		}
		else {
			aet_residual = cvs[i].aet_LS_mPdt - cvs[i].stream.hCH;// 지면의 수분이 모두 증발하고 남은 증발 가능량
			cvs[i].stream.hCH = 0; // 모두 증발 되었으므로 0.
		}
	}
	else {
		aet_residual = cvs[i].aet_LS_mPdt; 
	}
	if (cvs[i].stream.hCH == 0) {
		if (cvs[i].soilWaterC_tm1_m > aet_residual) { // 여기서는 토양에서 손실처리
			cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m - aet_residual;
		}
		else {
			cvs[i].soilWaterC_tm1_m = 0.0;
		}
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
	double sdTerm = cvs[i].daytimeLength_hrs / 12.0;   // 입력자료 단위 hrs / day 가 그대로 방정식에 이용된다.
	double es = 0.6108 * exp(17.27 * tave / (tave + 237.3)); // es : kPa
	double svd = 216.7 * es / (tave + 273.3);  //svd : gr/m^3==> 이 단위 그대로 적용되게 한다.
	// 0.0055*25.4 = 0.1397 : inch -> mm로 변환하기 위해서 25.4를 곱해준다.
	double pet_mmPday = 0.1397 * sdTerm * sdTerm * svd;
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0;// 총 일 증발량을 dt 시간으로 나누어서 사용한다.
}

void calPET_Hargreaves(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double t_diff = cvs[i].tempMaxPday - cvs[i].tempMinPday;
	if (t_diff < 0) {
		t_diff = 0.0;
	}
	// 2024.11.25. 기상청 일사량 자료 단위 MJ/m^2을 그대로 적용하는 것으로 수정
	//double lv = 2.45; // 증발잠열 FAO 권장값. MJ/kg
	// 0.000939 = 0.0023 / 2.45
	double pet_mmPday = 0.000939 * cvs[i].solarRad_MJperM2 * pow(t_diff, 0.5) * (tave + 17.8);  // 입력자료 단위 MJ/m^2을 그대로 적용
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // 총 일 증발량을 초 단위로 나누어서 사용한다.
}


void calPET_PriestleyTaylor(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double lv = 2.45; // 증발잠열 FAO 권장값. MJ/kg
    //double airP = 101.3 * pow((293 - 0.0065 * 셀고도) / 293, 5.26); //기압 kPa/degreeC ==> 약 1기압 = 101.3kPa == 1013mb
	double gamma = 0.0674; // 0.000665 * 101.3;//건습계 상수 ==> 1기압에서 약 0.0674 kPa/degreeC
	double coeffTave = 17.27 * tave / (tave + 237.3);
	double coeffSVD = tave + 237.3;
	// 2503.06 = 4098 * 0.6108
	double svpGrad = 2503.06 * exp(coeffTave) / coeffSVD / coeffSVD;  // kPa/degreeC
	double pet_mmPday = 1.28 * svpGrad / (svpGrad + gamma) * cvs[i].solarRad_MJperM2 / lv;
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // mmPday => mPdt   // 총 일 증발량을 초단위로 나누어서 사용한다.
}

void calSnowMelt(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	if (tave < cvs[i].smeltTSR) {// 이경우 snow pack 증가
		cvs[i].spackAcc_m += cvs[i].rfApp_mPdt;
		cvs[i].rfApp_mPdt = 0; // 강수가 모두 snow pack로 변환
	}

	cvs[i].smelt_mPdt = 0.0;
	if (cvs[i].spackAcc_m > 0) {
		double sm_mmPday = 0.0;
		double cSM = cvs[i].smeltCoef; //융설계수 / 소유역 매개변수
		double sCov = cvs[i].snowCovR; // Snow coverage ratio / 소유역 매개변수
		double tMelt = cvs[i].smeltingT;// 융설온도 / 소유역 매개변수
		double tAve = (cvs[i].spackTemp + cvs[i].tempMaxPday) / 2.0; // 대상 일에서의 snow pack 온도, 일 최고기온

		if (tAve > tMelt) {
			//if (cvs[i].smMethod == SnowMeltMethod::Anderson) { // 현재는 Anderson 만 적용
			sm_mmPday = cSM * sCov * (tAve - tMelt);
			//}
		}
		else {// tAve 가 융설온도보다 낮은 경우===> 융설 없다.
			sm_mmPday = 0;
		}
		cvs[i].smelt_mPdt = ts.dtsec * sm_mmPday / 86400000.0; // mmPday => mPdt

		if (cvs[i].spackAcc_m > 0 && cvs[i].smelt_mPdt > 0) {
			if (cvs[i].spackAcc_m > cvs[i].smelt_mPdt) {
				cvs[i].spackAcc_m = cvs[i].spackAcc_m - cvs[i].smelt_mPdt;
			}
			else {// 이경우는 snow pack 이 없어진다.
				cvs[i].smelt_mPdt = cvs[i].spackAcc_m; // 최대 융설량은 snow pack 깊이 만큼이다.
				cvs[i].spackAcc_m = 0.0;
			}
		}

		// 융설은 강우로 더하지 않고.. 지표면 질량으로 더해준다..
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


