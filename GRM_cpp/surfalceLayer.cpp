#include "stdafx.h"
#include "grm.h"

extern projectFile prj;
extern fs::path fpnLog;

extern cvAtt* cvs;
extern domaininfo di;
extern cvpos* cvps;
extern double* cvele;// 각셀의 해발고도. DEM에서 읽은 값. 배열 인덱스가, cv 인덱스와 같게 한다.
extern map<int, double[12]> laiRatio;
extern double sunDurRatioForAday[65][13];
extern double blaneyCriddleKData[12];
//extern double svpGradient[100];

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
	if (cvs[i].pet_mPdt < 0) { // 위에서 계산된 잠재증발산이 0보다 작으면, 0으로 처리
		cvs[i].pet_mPdt = 0.0;
	}

	// 여기서 토지피복별로 증발산을 손실처리 한다. 크게 물, 토양, canopy로 구분
// 수역에서는 aet = pet * et_coeff 보다 값이 크고(pet에 가까운 값), 그외에서는 aet = pet * et_coeff 와 같다.
// 토지피복별로 세분해서 계산하는 이유는. 토양 포화도 변화 계산에 적용될 부분을 구분하고, 향후 세부 성분 활용 가능성을 고려함
	double aet_canopy = 0.0;
	// canopy에서 증발산 되어야 하는데, 차단량이 부족해서 증발산이 못된 부분으로, 
	// 이 부분은 canopy 인근 토양에서 올라온 수분이 증산되는것으로 본다.
	double aet_canopy_transp = 0.0;
	double aet_soil = 0.0;
	double aet_water = 0.0;
	int isWater = -1;
	double nonCanopyR = 1.0 - cvs[i].canopyR;
	if (cvs[i].lcCode == landCoverCode::WATR
		|| cvs[i].lcCode == landCoverCode::WTLD) {
		if (cvs[i].flowType == cellFlowType::OverlandFlow
			&& cvs[i].hOF > 0.0) { // 수역에서 수심이 있는 경우
			isWater = 1;
		}
		else	if ((cvs[i].flowType == cellFlowType::ChannelFlow
			|| cvs[i].flowType == cellFlowType::ChannelNOverlandFlow)
			&& cvs[i].stream.hCH > 0.0) { // 하천흐름 셀에서 수심이 있는 경우
			isWater = 1;
		}
	}

	if (ts.wdUsed_userET != 2) {// 실제증발산량을 적용하는 경우가 아니면,
		if (cvs[i].intcpAcc_m == 0) {// 차단된 강수가 없으면
			aet_canopy = 0.0;
			aet_canopy_transp = cvs[i].pet_mPdt * cvs[i].etCoef; // 2022.11.30. 모든 잠재증발산은 증산으로 간주하고, 토양에서의 손실로 처리
		}
		else {//차단된 강수가 있으면. 여기서 증발산에 의해 차단량 감소, canopy 에서 최대 차단 가능량이 있기때문에.. 여기서 반영해줘야 함.
			aet_canopy = cvs[i].pet_mPdt * cvs[i].etCoef;
			if (cvs[i].intcpAcc_m < aet_canopy) {  //차단량이 모두 증발되면, 증발산량을 반영할 필요 있음.
				aet_canopy_transp = aet_canopy - cvs[i].intcpAcc_m; // 이부분은 토양 수분에서 손실로 처리.
				aet_canopy = cvs[i].intcpAcc_m;
				cvs[i].intcpAcc_m = 0.0;
			}
			else {
				cvs[i].intcpAcc_m = cvs[i].intcpAcc_m - aet_canopy;
				aet_canopy_transp = 0.0;
			}
		}
		// aet_water는 canopy 영역을 제외하지 않고, 셀 전체에서 증발산
		// aet_canopy는 canopy 영역에서만 발생
		if (isWater == 1) {
			aet_water = cvs[i].pet_mPdt; // 여기서는 수면과 수상식물에서 증발산 // aet_water는 잠재증발산량 그대로 적용		
			aet_soil = 0.0;
		}
		else {
			// 여기서는 토양과 식물에서 증발산
			aet_soil = cvs[i].pet_mPdt * cvs[i].etCoef;
			if (cvs[i].ssr == 0.0) { // 토양수분이 없으면, 증발산도 없다.
				aet_soil = 0.0;   // 토양수분 감소와 토양수분 정도에 따른 실제증발산 수정은  cal_h_SWC_OFbyAET(i), cal_h_CHbyAET(i) 여기서 한다.
			}
			aet_water = 0.0;
		}

		// 캐노피에서의 증발산과 그 외 부분(물, 토양)에서의 증발산 
		cvs[i].aet_mPdt = (aet_canopy + aet_canopy_transp) * cvs[i].canopyR
			+ aet_water * nonCanopyR + aet_soil * nonCanopyR;
		cvs[i].aet_LS_mPdt = aet_canopy_transp * cvs[i].canopyR
			+ aet_water * nonCanopyR + aet_soil * nonCanopyR; // 2.22.11.30. 물, 지표면, 토양에서의 손실 계산에 이용. aet_canopy는 식생에서만 발생
	}
	else if (ts.wdUsed_userET == 2) { // 실제 증발산을 이용하는 경우에는 calPET_userET()에서 설정된 aet_mPdt 값을 그대로 이용
		if (cvs[i].intcpAcc_m == 0) {// 차단된 강수가 없으면
			aet_canopy = 0.0;
			aet_canopy_transp = cvs[i].aet_mPdt; // 2022.11.30. 모든 실재증발산은 증산으로 간주하고, 토양에서의 손실로 처리
		}
		else {//차단된 강수가 있으면. 여기서 증발산에 의해 차단량 감소, canopy 에서 최대 차단 가능량이 있기때문에.. 여기서 반영해줘야 함.
			if (cvs[i].intcpAcc_m < cvs[i].aet_mPdt) {  
				aet_canopy_transp = cvs[i].aet_mPdt - cvs[i].intcpAcc_m; // 이부분은 토양 수분에서 손실로 처리.
				aet_canopy = cvs[i].intcpAcc_m; // 차단량은 모두 증발됨
				cvs[i].intcpAcc_m = 0.0;
			}
			else { // 차단되어 있던 양이 더 크면
				cvs[i].intcpAcc_m = cvs[i].intcpAcc_m - cvs[i].aet_mPdt;
				aet_canopy_transp = 0.0; // 모두 증발, 증산은 없음.
			}
		}
		// aet_water는 canopy 영역을 제외하지 않고, 셀 전체에서 증발산
		// aet_canopy는 canopy 영역에서만 발생
		if (isWater == 1) {
			aet_water = cvs[i].aet_mPdt; // 여기서는 수면과 수상식물에서 증발산 // aet_water는 실재증발산량 그대로 적용		
			aet_soil = 0.0;
		}
		else {
			// 여기서는 토양과 식물에서 증발산
			aet_soil = cvs[i].aet_mPdt;
			if (cvs[i].ssr == 0.0) { // 토양수분이 없으면, 증발산도 없다.
				aet_soil = 0.0; // 토양수분 감소와 토양수분 정도에 따른 실제증발산 수정은  cal_h_SWC_OFbyAET(i), cal_h_CHbyAET(i) 여기서 한다.
			}
			aet_water = 0.0;
		}
		cvs[i].aet_LS_mPdt = aet_canopy_transp * cvs[i].canopyR
			+ aet_water * nonCanopyR + aet_soil * nonCanopyR; // 2.22.11.30. 물, 지표면, 토양에서의 손실 계산에 이용. aet_canopy는 식생에서만 발생

	}
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
	case cellFlowType::ChannelNOverlandFlow: { // 이 경우는 하천의 수위가 토양수분에 지배적이므로, 하천 중심으로 모의한다.
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

	// cellFlowType::ChannelNOverlandFlow 와 cellFlowType::ChannelFlow 에서는
	// 하천 CV에 대해서만 토양수분 변화 및 그에 따른 aet 재산정을 계산한다.
	if (cvs[i].hOF == 0 && cvs[i].flowType == cellFlowType::OverlandFlow ) {
		if (cvs[i].soilWaterC_tm1_m > aet_residual) { // 여기서는 토양에서 손실처리
			cvs[i].soilWaterC_tm1_m = cvs[i].soilWaterC_tm1_m - aet_residual;
		}
		else {
			if (ts.wdUsed_userET == 2) {// 이경우는 실제 증발산을 수정하지 않는다.
				cvs[i].soilWaterC_tm1_m = 0.0;
			}
			else {
				double aet_difficit = aet_residual - cvs[i].soilWaterC_tm1_m; // 토양수분이 부족해서 이만큼은 증발되지 않는다.
				cvs[i].soilWaterC_tm1_m = 0.0;
				cvs[i].aet_mPdt = cvs[i].aet_mPdt - aet_difficit; // 실제 증발산을 수정한다.
				cvs[i].aet_LS_mPdt = cvs[i].aet_LS_mPdt - aet_difficit; // 실제 증발산을 수정한다.
			}

		}
	}
}

void cal_h_CHbyAET(int i) {
	double aet_residual = 0.0;
	if (cvs[i].stream.hCH > 0) {
		if (cvs[i].stream.hCH > cvs[i].aet_LS_mPdt) { // 이경우는 하천 수면에서 손실 처리
			cvs[i].stream.hCH = cvs[i].stream.hCH - cvs[i].aet_LS_mPdt;
			if (cvs[i].stream.hCH < WETDRY_CRITERIA && cvs[i].stream.hCH>0.0) {
				cvs[i].stream.hCH = WETDRY_CRITERIA;
			}
		}
		else {
			aet_residual = cvs[i].aet_LS_mPdt - cvs[i].stream.hCH;// 하천의 수위가 모두 증발하고 남은 증발 가능량
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
			if (ts.wdUsed_userET == 2) {// 이경우는 실제 증발산을 수정하지 않는다.
				cvs[i].soilWaterC_tm1_m = 0.0;
			}
			else {
				double aet_difficit = aet_residual - cvs[i].soilWaterC_tm1_m; // 토양수분이 부족해서 이만큼은 증발되지 않는다.
				cvs[i].soilWaterC_tm1_m = 0.0;
				cvs[i].aet_mPdt = cvs[i].aet_mPdt - aet_difficit; // 실제 증발산을 수정한다.
				cvs[i].aet_LS_mPdt = cvs[i].aet_LS_mPdt - aet_difficit; // 실제 증발산을 수정한다.
			}

		}
	}
}

void calPET_BlaneyCriddle(int i)
{
	int month_apply = ts.tCurMonth;
	if (cvps[i].intLat_degree < 0) { // 남반구에 해당하는 참조월로 변환
		month_apply += 6;
		if (month_apply > 12) {
			month_apply = month_apply - 12;
		}
	}
	double k = blaneyCriddleKData[month_apply - 1];
	int lat_abs = abs(cvps[i].intLat_degree);	
	if (lat_abs > 64) { lat_abs = 64; } // 64도 이상에서는 64도의 값을 사용한다.
	double p = sunDurRatioForAday[lat_abs][month_apply]; //[x][0]에 0월로 dummy 값이 입력되어 있으므로, 해당월을 그대로 사용한다.
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
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0;// 1/86400000 = 1/1000/24/60/60.	총 일 증발량을 dt 시간으로 나누어서 사용한다.
}

void calPET_Hargreaves(int i) {
	double tave = (cvs[i].tempMaxPday + cvs[i].tempMinPday) / 2.0;
	double t_diff = cvs[i].tempMaxPday - cvs[i].tempMinPday;
	if (t_diff < 0.0) {
		t_diff = 0.0;
	}
	// 2024.11.25. 기상청 일사량 자료 단위 MJ/m^2을 그대로 적용하는 것으로 수정
	//double lv = 2.45; // 증발잠열 FAO 권장값. MJ/kg
	// 0.000939 = 0.0023 / 2.45
	//double pet_mmPday = 0.000939 * cvs[i].solarRad_MJpM2 * pow(t_diff, 0.5) * (tave + 17.8);  // 입력자료 단위 MJ/m^2을 그대로 적용
	//double Ra = getRa(cvs[i].lat_rad);
	double Ra = getRa(cvps[i].lat_rad);
	double pet_mmPday = 0.000939 * Ra * pow(t_diff, 0.5) * (tave + 17.8);  // 입력자료 단위 MJ/m^2을 그대로 적용
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // 1/86400000 = 1/1000/24/60/60.  총 일 증발량을 초 단위로 나누어서 사용한다.
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
	double pet_mmPday = 0.52245 * Rn * svpGrad / (svpGrad + gamma); // 0.52245 = 1.28 / 2.45,   lv = 2.45; // 증발잠열 FAO 권장값. MJ/kg
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // mmPday => mPdt   1/86400000 = 1/1000/24/60/60. // 총 일 증발량을 초단위로 나누어서 사용한다.
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
	cvs[i].pet_mPdt = ts.dtsec * pet_mmPday / 86400000.0; // mmPday => mPdt   1/86400000 = 1/1000/24/60/60. // 총 일 증발량을 초단위로 나누어서 사용한다.
}

void calPET_userET(int i, int dataInterval_min, int whatUserData) {
	double userET_mPs = cvs[i].userET_mm / dataInterval_min / 60000.0; // 60000 = 60 sec * 1000 mm // mPsec로 변환
	if (whatUserData == 1) { // PET 이면
		cvs[i].pet_mPdt = ts.dtsec * userET_mPs; // mPdt 로 변환
	}
	else if (whatUserData == 2) { // AET 이면
		cvs[i].aet_mPdt = ts.dtsec * userET_mPs; // mPdt 로 변환
		cvs[i].pet_mPdt = 0;
	}
}

// psychrometric constant. 건습계 상수
double getPsychroConst(int i) {
	double par1 = (293 - 0.0065 * cvele[i]) / 293;
	double airP = 101.3 * pow(par1, 5.26);  //기압 kPa/degreeC ==> 약 1기압 = 101.3kPa == 1013mb
	double gamma = 0.000665 * airP;//건습계 상수 
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


