#include "stdafx.h"
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

//double svpGradient[100];
extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern cvAtt* cvs;
extern cvpos* cvps;
extern double* cvele;// 각셀의 해발고도. DEM에서 읽은 값. 배열 인덱스가, cv 인덱스와 같다
extern domaininfo di;
extern thisSimulation ts;

extern weatherData* rfs;
extern weatherData* tempMax;
extern weatherData* tempMin; // 최대기온 파일에서 읽은 자료
extern weatherData* dayTimeLength; 
extern weatherData* solarRad; 
extern weatherData* dewPointTemp; // 이슬점 온도 파일에서 읽은 자료
extern weatherData* windSpeed; // 풍속 파일에서 읽은 자료
extern weatherData* userET; // 사용자 입력 잠재(혹은 실제) 증발산량
extern weatherData* snowpackTemp;

extern double sunDurRatioForAday[65][13];  // 배열 인덱스가 해당 "위도", "월" 을 의미한다. 바로 참조가능

extern map<int, double[12]> laiRatio; // <LCvalue, 월별 laiRatio 12개>
extern double blaneyCriddleKData[12];
extern wpSimData wpSimValue;


int setRainfallData()
{
	//rfs.clear();
	//rfs = readAndSetWeatherData(lower(prj.fpnRainfallData), prj.rfDataType,
	//	prj.rfinterval_min, "Precipitation");
	vector<weatherData> wdinfo;
	wdinfo = readAndSetWeatherData(lower(prj.fpnRainfallData), prj.rfDataType,
			prj.rfinterval_min, "Precipitation");

	if (wdinfo.size() == 0) {
		writeLogString(fpnLog, "ERROR : Reading precipitation data file was failed.\n", 1, 1);
		return -1;
	}
	int nwd = (int)wdinfo.size();
	rfs = new weatherData[nwd];
	std::copy(wdinfo.begin(), wdinfo.end(), rfs);

	if (prj.simType != simulationType::RealTime) {
		ts.dataNumTotal_rf = nwd;// (int)rfs.size();
	}
	else {
		ts.dataNumTotal_rf = 0;
	}
	return 1;
}

int setCVRF(int order)
{
	int dtrf_sec = prj.rfinterval_min * 60;
	int dtrf_min = prj.rfinterval_min;
	int dt_sec = ts.dtsec;
	string fpnRF = "";
	double cellSize = di.cellSize;
	ts.rfiSumAllCellsInCurRFData_mPs = 0;
	int returnv = -1;
	for (int idx : wpSimValue.wpCVidxes) {
		wpSimValue.prcpiReadSumUpWS_mPs[idx] = 0;
	}
	if (prj.rfDataType == weatherDataType::Raster_ASC
		|| prj.rfDataType == weatherDataType::Mean_DividedArea)
	{
		fpnRF = rfs[order - 1].FilePath + "/" + rfs[order - 1].FileName;
		ascRasterFile* rfasc;
		map<int, double> idNv;
		if (prj.rfDataType == weatherDataType::Raster_ASC) {
			rfasc = new ascRasterFile(fpnRF);
			if (compareASCwithDomain(fpnRF, "precipitation", rfasc->header, di.nCols, di.nRows, di.dx) == -1) {
				return -1;
			}
			idNv.clear();
		}
		else {
			idNv = rfs[order - 1].vForEachRegion;
			rfasc = NULL;
		}
		omp_set_num_threads(prj.mdp);
		map<int, double>* rfiReadSumUpWS_mPs_L = new map<int, double>[prj.mdp];
		for (int nthread = 0; nthread < prj.mdp; ++nthread) {
			for (int idx : wpSimValue.wpCVidxes) {
				rfiReadSumUpWS_mPs_L[nthread][idx] = 0;  //0으로 초기화
			}
		}
		double* rfiSumAllCellsInCurRFData_mPs_L = new double[prj.mdp](); //0으로 초기화

 //parallel================================================
#pragma omp parallel
		{
			int nth = omp_get_thread_num();
#pragma omp for
			for (int i = 0; i < di.cellNnotNull; ++i) {
				// 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
				// 강우는 상류에 내린 강우도 포함하는 것이 타당.
				// 상류 cv 개수에 이 조건 추가하려면 주석 해제.
				//if (cvs[i].toBeSimulated == -1) {
				//    continue;  }
				int wid = cvps[i].wsid;
				double iniL_PRCP_mm = prj.swps[wid].iniLossPRCP_mm;
				double inRF_mm = 0.0;
				if (prj.rfDataType == weatherDataType::Raster_ASC) {
					inRF_mm = rfasc->valuesFromTL[cvps[i].xCol][cvps[i].yRow];
				}
				else {
					inRF_mm = idNv[cvps[i].wsid]; 
				}

				double inRFi_mPs = 0.0;
				if (inRF_mm <= 0) {
					inRFi_mPs = 0;
				}
				else {
					inRFi_mPs = rfintensity_mPsec(inRF_mm, dtrf_sec);
				}
				cvs[i].rfiRead_mPsec = inRFi_mPs;
				cvs[i].rfAccRead_fromStart_mm = cvs[i].rfAccRead_fromStart_mm + inRF_mm; // 파일에서 읽은 강우량을 더한다.
				if (iniL_PRCP_mm > 0.0) { 
					if (cvs[i].rfAccRead_fromStart_mm < iniL_PRCP_mm) {
						inRFi_mPs = 0.0;
					}
					else {
						double diff_mm=0.0;
						diff_mm = cvs[i].rfAccRead_fromStart_mm - iniL_PRCP_mm; // 처음 커지는 부분에서 강우량 차이 받는다.
						if (diff_mm < 0) {
							diff_mm = 0.0;
						}
						inRFi_mPs = rfintensity_mPsec(diff_mm, dtrf_sec);
						prj.swps[wid].iniLossPRCP_mm = 0.0; // 누적 강수가 초기 손실량을 초과하면, 이제 초기 손실을 사용하지 않으므로, 0으로 설정
					}
				}
				cvs[i].rfiRead_After_iniLoss_mPsec = inRFi_mPs;

				for (int idx : cvs[i].downWPCVidx) {
					rfiReadSumUpWS_mPs_L[nth][idx] += cvs[i].rfiRead_mPsec;
				}
				rfiSumAllCellsInCurRFData_mPs_L[nth] += cvs[i].rfiRead_mPsec;
			}
		}
				// reduction
		for (int nth = 0; nth < prj.mdp; ++nth) {
			for (int idx : wpSimValue.wpCVidxes) {
				wpSimValue.prcpiReadSumUpWS_mPs[idx] += rfiReadSumUpWS_mPs_L[nth][idx];
			}
			ts.rfiSumAllCellsInCurRFData_mPs += rfiSumAllCellsInCurRFData_mPs_L[nth];
		}
 //parallel ===================================================


//// serial==================================================
//		map<int, double> rfiReadSumUpWS_mPs_serial;
//		double rfiSumAllCellsInCurRFData_mPs_serial=0.0; //0으로 초기화
//			for (int i = 0; i < di.cellNnotNull; ++i) {
//				// 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
//				// 강우는 상류에 내린 강우도 포함하는 것이 타당.
//				// 상류 cv 개수에 이 조건 추가하려면 주석 해제.
//				//if (cvs[i].toBeSimulated == -1) {
//				//    continue;  }
//				int wid = cvps[i].wsid;
//				double iniLoss_PRCP_mm = prj.swps[wid].iniLossPRCP_mm;
//				double inRF_mm = 0.0;
//				if (prj.rfDataType == weatherDataType::Raster_ASC) {
//					inRF_mm = rfasc->valuesFromTL[cvps[i].xCol][cvps[i].yRow];
//				}
//				else {
//					inRF_mm = idNv[cvps[i].wsid]; 
//				}
//
//				double inRFi_mPs = 0.0;
//				if (inRF_mm <= 0) {
//					inRFi_mPs = 0;
//				}
//				else {
//					inRFi_mPs = rfintensity_mPsec(inRF_mm, dtrf_sec);
//				}
//				cvs[i].rfiRead_mPsec = inRFi_mPs;
//				cvs[i].rfAccRead_fromStart_mm = cvs[i].rfAccRead_fromStart_mm + inRF_mm; // 파일에서 읽은 강우량을 더한다.
//				if (iniLoss_PRCP_mm > 0.0) { 
//					if (cvs[i].rfAccRead_fromStart_mm < iniLoss_PRCP_mm) {
//						inRFi_mPs = 0.0;
//					}
//					else {
//
//						if (wid == 203 || wid == 203600) {
//							int a = 1;
//						}
//
//						double diff_mm=0.0;
//						diff_mm = cvs[i].rfAccRead_fromStart_mm - iniLoss_PRCP_mm; // 처음 커지는 부분에서 강우량 차이 받는다.
//						if (diff_mm < 0) {
//							diff_mm = 0.0;
//						}
//						inRFi_mPs = rfintensity_mPsec(diff_mm, dtrf_sec);
//						prj.swps[wid].iniLossPRCP_mm = 0.0; // 누적 강수가 초기 손실량을 초과하면, 이제 초기 손실을 사용하지 않으므로, 0으로 설정
//					}
//				}
//				cvs[i].rfiRead_After_iniLoss_mPsec = inRFi_mPs;
//
//				for (int idx : cvs[i].downWPCVidx) {
//					rfiReadSumUpWS_mPs_serial[idx] += cvs[i].rfiRead_mPsec;
//				}
//				rfiSumAllCellsInCurRFData_mPs_serial += cvs[i].rfiRead_mPsec;
//			}
//			// reduction
//				for (int idx : wpSimValue.wpCVidxes) {
//					wpSimValue.prcpiReadSumUpWS_mPs[idx] = rfiReadSumUpWS_mPs_serial[idx];
//				}
//				ts.rfiSumAllCellsInCurRFData_mPs += rfiSumAllCellsInCurRFData_mPs_serial;
//// serial ==========================================

		if (prj.rfDataType == weatherDataType::Raster_ASC
			|| prj.rfDataType == weatherDataType::Mean_DividedArea) {
			if (rfasc != NULL) {
				delete rfasc;
			}
			if (rfiSumAllCellsInCurRFData_mPs_L != NULL) {
				delete rfiSumAllCellsInCurRFData_mPs_L;
			}
			if (rfiReadSumUpWS_mPs_L != NULL) {
				rfiReadSumUpWS_mPs_L->clear();
			}
		}
		returnv = 1;
	}
	else if (prj.rfDataType == weatherDataType::Mean)
	{
		fpnRF = rfs[order - 1].FilePath + "/" + rfs[order - 1].FileName;
		string value = rfs[order - 1].value;
		double inRF_mm = stod(value);
		double inRFi_mPs;
		if (inRF_mm < 0) {
			inRF_mm = 0;
			inRFi_mPs = 0;
		}
		else {
			inRFi_mPs = rfintensity_mPsec(inRF_mm, dtrf_sec);
		}

#pragma omp parallel for schedule(guided)
		for (int i = 0; i < di.cellNnotNull; ++i) {
			// 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
			// 상류 cv 개수에 이 조건 추가하려면 주석 해제.
			//if (cvs[i].toBeSimulated == -1) { continue; }
			int wid = cvps[i].wsid;
			double iniLoss_PRCP_mm = prj.swps[wid].iniLossPRCP_mm;
			cvs[i].rfiRead_mPsec = inRFi_mPs;
			cvs[i].rfAccRead_fromStart_mm = cvs[i].rfAccRead_fromStart_mm + inRF_mm;
			if (iniLoss_PRCP_mm > 0.0) {
				if (cvs[i].rfAccRead_fromStart_mm < iniLoss_PRCP_mm) {
					inRFi_mPs = 0.0;
				}
				else {
					double diff_mm = 0.0;
					diff_mm = cvs[i].rfAccRead_fromStart_mm - iniLoss_PRCP_mm; // 처음 커지는 부분에서 강우량 차이 받는다.
					if (diff_mm < 0) {
						diff_mm = 0.0;
					}
					inRFi_mPs = rfintensity_mPsec(diff_mm, dtrf_sec);
					prj.swps[wid].iniLossPRCP_mm = 0.0; // 누적 강수가 초기 손실량을 초과하면, 이제 초기 손실을 사용하지 않으므로, 0으로 설정
				}
			}
			cvs[i].rfiRead_After_iniLoss_mPsec = inRFi_mPs;
		}

		ts.rfiSumAllCellsInCurRFData_mPs = inRFi_mPs * di.cellNtobeSimulated;
		for (int idx : wpSimValue.wpCVidxes) {
			wpSimValue.prcpiReadSumUpWS_mPs[idx] = inRFi_mPs * wpSimValue.cvCountAllup[idx];
		}
		returnv = 1;
	}
	else {
		writeLogString(fpnLog, "ERROR : Precipitation data type is invalid.\n", 1, 1);
		return -1;
	}
	if (returnv == -1) {
		writeLogString(fpnLog, "ERROR : An error was occurred while reading Precipitation data.\n", 1, 1);
		writeLogString(fpnLog, "    Precipitation file : " + fpnRF + ".\n", 1, 1);
	}
	return returnv;
}

 void setCVRFintensityAndDTrf_Zero()
{
	 writeLogString(fpnLog, "WARNNING : The precipitation data ended before the simulation was over. Precipitation was set to 0.\n", 1, -1);
	 writeLogString(fpnLog, "WARNNING : If the time step of precipitation data is equal or smaller than printing time step, add more data. Or decrease simulation duration.\n", 1, -1);
	 ts.rfAveForDT_m = 0;
	 ts.rfAveSumAllCells_PdT_m = 0;
	 ts.rfAveSumAllCells_PdTave_m = 0;
     ts.rfiSumAllCellsInCurRFData_mPs = 0;
#pragma omp parallel for
     for (int i = 0; i < di.cellNnotNull; ++i)    {
		 cvs[i].rfiRead_mPsec = 0;
		 cvs[i].rfiRead_After_iniLoss_mPsec = 0;
        cvs[i].rfApp_mPdt = 0;
    }

    for(int wpcvid : wpSimValue.wpCVidxes)    {
        wpSimValue.prcpWPGridForPdT_mm[wpcvid] = 0;
        wpSimValue.prcpUpWSAveForDt_mm[wpcvid] = 0;
        wpSimValue.prcpiReadSumUpWS_mPs[wpcvid] = 0;
        wpSimValue.prcpUpWSAveForPT_mm[wpcvid] = 0;
    }
}

     
 inline double rfintensity_mPsec(double rf_mm, double dtrf_sec)
 {
     return rf_mm / 1000.0 / dtrf_sec;
 }

 void initRFvars()
 {
	 ts.rfAveForDT_m = 0;
	 ts.rfAveSumAllCells_PdT_m = 0;
	 ts.rfAveSumAllCells_PdTave_m = 0;
	 ts.rfiSumAllCellsInCurRFData_mPs = 0;
	 for (int wpcvid : wpSimValue.wpCVidxes) {
		 wpSimValue.prcpWPGridForPdT_mm[wpcvid] = 0;
		 wpSimValue.prcpUpWSAveForDt_mm[wpcvid] = 0;
		 wpSimValue.prcpiReadSumUpWS_mPs[wpcvid] = 0;
		 wpSimValue.prcpUpWSAveForPT_mm[wpcvid] = 0;
	 }
 }
 
 int setLAIRatio()
 {
	 if (prj.fpnLAI != "" && fs::exists(lower(prj.fpnLAI)) != true) {
		 writeLogString(fpnLog, "ERROR : LAI ratio file is invalid.\n", 1, 1);
		 return -1;
	 }
	 laiRatio.clear();
	 vector<string> Lines;
	 Lines = readTextFileToStringVector(prj.fpnLAI);
	 for (int n = 0; n < Lines.size(); n++) {
		 if (trim(Lines[n]) == "") { break; }
		 string value = Lines[n];
		 vector<double> LineParts = splitToDoubleVector(Lines[n], ',');//13개
		 int lcValue = int(LineParts[0]);// 첫번째는 lc raster value 이다. 
		 copy(LineParts.begin() + 1, LineParts.end(), laiRatio[lcValue]);//12개
		 double maxV = 0.0;
		 for (int m = 0; m < 12; m++) {
			 if (maxV < laiRatio[lcValue][m]) {
				 maxV = laiRatio[lcValue][m];
			 }
		 }
		 for (int m = 0; m < 12; m++) {
			 if (maxV > 0) {
				 laiRatio[lcValue][m] = laiRatio[lcValue][m] / maxV;
			 }
			 else {
				 laiRatio[lcValue][m] = 0.0;
			 }
		 }
	 }
	 return 1;
 }

 int setCVweatherData(int order, string dataString, weatherDataType wdType, 
	 weatherData* vectWd, whatWeatherData wwd)
 {
	 string fpnData = "";
	 int returnv = -1;
	 fpnData = vectWd[order - 1].FilePath + "/" + vectWd[order - 1].FileName;
	 switch (wdType)
	 {
	 case weatherDataType::Raster_ASC: {
		 ascRasterFile* wdAsc;
		 wdAsc = new ascRasterFile(fpnData);
		 if (compareASCwithDomain(fpnData, dataString, wdAsc->header, di.nCols, di.nRows, di.dx) == -1) {
			 return -1;
		 }
		 omp_set_num_threads(prj.mdp);
#pragma omp parallel for
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 double wdValue = wdAsc->valuesFromTL[cvps[i].xCol][cvps[i].yRow];
			 returnv = assignWDtoCV(i, wwd, wdValue);
		 }
		 if (wdAsc != NULL) {
			 delete wdAsc;
		 }
		 break;
	 }
	 case weatherDataType::Mean_DividedArea: {
		 map<int, double> idNv;
		 idNv = vectWd[order - 1].vForEachRegion;
		 omp_set_num_threads(prj.mdp);
#pragma omp parallel for
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 double wdValue = idNv[cvps[i].wsid];
			 returnv = assignWDtoCV(i, wwd, wdValue);
		 }
		 break;
	 }
	 case weatherDataType::Mean: {
		 double wdValue = stod(vectWd[order - 1].value);
#pragma omp parallel for schedule(guided)
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
			 // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
			 //if (cvs[i].toBeSimulated == -1) { continue; }
			 returnv = assignWDtoCV(i, wwd, wdValue);
		 }
		 break;
	 }
	 default: {
		 writeLogString(fpnLog, "ERROR : " + dataString + " data type is invalid.\n", 1, 1);
		 return -1;
	 }
	 }
	 if (returnv == -1) {
		 writeLogString(fpnLog, "ERROR : An error was occurred while reading +"+ dataString+" data.\n", 1, 1);
		 writeLogString(fpnLog, "    Data file : " + fpnData + ".\n", 1, 1);
	 }
	 return returnv;
 }

 int assignWDtoCV(int i, whatWeatherData wwd, double wdValue) {
	 switch (wwd)
	 {
	 case whatWeatherData::TempMax: {
		 cvs[i].tempMaxPday = wdValue;
		 break;
	 }
	 case whatWeatherData::TempMin: {
		 cvs[i].tempMinPday = wdValue;
		 break;
	 }
	 case whatWeatherData::DaytimeLength: {
		 cvs[i].daytimeLength_hrs = wdValue;
		 break;
	 }
	 case whatWeatherData::DewPointTemp: {
		 cvs[i].dewPointTemp = wdValue;
		 break;
	 }
	 case whatWeatherData::SolarRad: {
		 cvs[i].solarRad_MJpM2 = wdValue;
		 break;
	 }
	 case whatWeatherData::WindSpeed: {
		 cvs[i].windSpeed_mps = wdValue;
		 break;
	 }
	 case whatWeatherData::UserET: {
		 cvs[i].userET_mm = wdValue;
		 break;
	 }
	 case whatWeatherData::SnowpackTemp: {
		 cvs[i].spackTemp = wdValue;
		 break;
	 }
	 default: {
		 return -1;
	 }
	 }
	 return 1;
 }

  
 int setTemperatureMax()
 {
	 vector<weatherData> wdinfo;
	 wdinfo = readAndSetWeatherData(prj.fpnTempMaxData, prj.tempMaxDataType,
		 prj.tempMaxInterval_min, "Maximum temperature");
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading maximum temperature data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 tempMax = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), tempMax);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.tempMaxInterval_min;
		 int quotient = simDur_min / prj.tempMaxInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the maximum temperature data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }
 
 int setTemperatureMin()
 {
	 vector<weatherData> wdinfo;
	 wdinfo = readAndSetWeatherData(prj.fpnTempMinData, prj.tempMinDataType,
		  prj.tempMinInterval_min, "Minimum temperature");
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading minimum temperature data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 tempMin = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), tempMin);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.tempMinInterval_min;
		 int quotient = simDur_min / prj.tempMinInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the minimum temperature data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }
 
 int setDaytimeLength()
 {
	 vector<weatherData> wdinfo;
	 wdinfo = readAndSetWeatherData(prj.fpnDTLData, prj.DTLDataType,
		  prj.DTLDataInterval_min, "Daytime length");
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading daytime length data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 dayTimeLength = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), dayTimeLength);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.DTLDataInterval_min;
		 int quotient = simDur_min / prj.DTLDataInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the daytime length data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }
   
 int setSunDurationRatioData()// 내장하는 것으로 수정. 2025.01.24
 {
	 int xRow_Lat = 65;
	 int yCol_Month = 13;
	  double local_sunDurRatioForAday[65][13] = { // 배열 인덱스가 해당 "위도", "월" 을 의미한다. 바로 참조가능
		{0, 0.274, 0.274, 0.274, 0.274, 0.274, 0.274, 0.274, 0.274, 0.274, 0.274, 0.274, 0.274},
		{0, 0.273, 0.273, 0.274, 0.275, 0.275, 0.275, 0.275, 0.275, 0.274, 0.273, 0.273, 0.273},
		{0, 0.272, 0.273, 0.274, 0.275, 0.276, 0.276, 0.276, 0.275, 0.274, 0.273, 0.272, 0.272},
		{0, 0.271, 0.272, 0.274, 0.276, 0.277, 0.278, 0.278, 0.276, 0.274, 0.272, 0.271, 0.270},
		{0, 0.270, 0.271, 0.273, 0.276, 0.278, 0.279, 0.279, 0.276, 0.274, 0.272, 0.270, 0.269},
		{0, 0.268, 0.270, 0.273, 0.277, 0.279, 0.280, 0.280, 0.277, 0.275, 0.271, 0.269, 0.268},
		{0, 0.267, 0.270, 0.273, 0.277, 0.280, 0.282, 0.281, 0.278, 0.275, 0.271, 0.268, 0.266},
		{0, 0.266, 0.269, 0.273, 0.278, 0.281, 0.283, 0.282, 0.279, 0.275, 0.270, 0.267, 0.265},
		{0, 0.265, 0.268, 0.273, 0.278, 0.282, 0.284, 0.283, 0.279, 0.275, 0.270, 0.266, 0.264},
		{0, 0.264, 0.268, 0.273, 0.279, 0.283, 0.286, 0.284, 0.280, 0.275, 0.270, 0.265, 0.262},
		{0, 0.263, 0.267, 0.273, 0.279, 0.284, 0.287, 0.285, 0.281, 0.275, 0.269, 0.264, 0.261},
		{0, 0.261, 0.266, 0.272, 0.280, 0.285, 0.288, 0.287, 0.282, 0.275, 0.269, 0.263, 0.260},
		{0, 0.260, 0.265, 0.272, 0.280, 0.286, 0.290, 0.288, 0.283, 0.275, 0.268, 0.262, 0.258},
		{0, 0.259, 0.265, 0.272, 0.281, 0.287, 0.292, 0.289, 0.283, 0.276, 0.267, 0.260, 0.257},
		{0, 0.257, 0.264, 0.272, 0.281, 0.288, 0.293, 0.290, 0.284, 0.276, 0.267, 0.259, 0.256},
		{0, 0.256, 0.263, 0.272, 0.282, 0.290, 0.294, 0.292, 0.285, 0.276, 0.266, 0.258, 0.254},
		{0, 0.255, 0.263, 0.272, 0.282, 0.291, 0.295, 0.293, 0.285, 0.276, 0.265, 0.257, 0.253},
		{0, 0.254, 0.262, 0.271, 0.283, 0.292, 0.296, 0.294, 0.286, 0.276, 0.265, 0.256, 0.251},
		{0, 0.253, 0.261, 0.271, 0.283, 0.293, 0.298, 0.295, 0.287, 0.276, 0.265, 0.255, 0.250},
		{0, 0.251, 0.260, 0.271, 0.284, 0.294, 0.299, 0.297, 0.288, 0.276, 0.264, 0.254, 0.248},
		{0, 0.250, 0.259, 0.271, 0.284, 0.295, 0.301, 0.298, 0.289, 0.276, 0.264, 0.253, 0.247},
		{0, 0.249, 0.258, 0.271, 0.285, 0.296, 0.302, 0.299, 0.290, 0.277, 0.263, 0.252, 0.245},
		{0, 0.247, 0.258, 0.271, 0.285, 0.297, 0.304, 0.301, 0.291, 0.277, 0.262, 0.250, 0.244},
		{0, 0.246, 0.257, 0.271, 0.286, 0.297, 0.307, 0.302, 0.291, 0.277, 0.262, 0.249, 0.242},
		{0, 0.245, 0.256, 0.271, 0.287, 0.297, 0.310, 0.303, 0.292, 0.277, 0.261, 0.248, 0.241},
		{0, 0.243, 0.255, 0.270, 0.287, 0.300, 0.310, 0.305, 0.293, 0.277, 0.261, 0.247, 0.239},
		{0, 0.242, 0.254, 0.270, 0.288, 0.302, 0.310, 0.306, 0.294, 0.277, 0.260, 0.245, 0.238},
		{0, 0.240, 0.253, 0.270, 0.289, 0.304, 0.311, 0.308, 0.295, 0.277, 0.259, 0.244, 0.236},
		{0, 0.239, 0.253, 0.270, 0.289, 0.305, 0.313, 0.309, 0.296, 0.277, 0.259, 0.243, 0.235},
		{0, 0.237, 0.252, 0.270, 0.290, 0.306, 0.315, 0.310, 0.296, 0.278, 0.258, 0.241, 0.233},
		{0, 0.236, 0.251, 0.270, 0.290, 0.308, 0.316, 0.312, 0.297, 0.278, 0.258, 0.240, 0.231},
		{0, 0.234, 0.250, 0.270, 0.291, 0.309, 0.318, 0.314, 0.298, 0.278, 0.257, 0.239, 0.229},
		{0, 0.232, 0.249, 0.270, 0.292, 0.310, 0.320, 0.315, 0.299, 0.278, 0.256, 0.237, 0.227},
		{0, 0.231, 0.248, 0.270, 0.293, 0.310, 0.322, 0.319, 0.300, 0.278, 0.256, 0.236, 0.225},
		{0, 0.229, 0.247, 0.269, 0.293, 0.310, 0.324, 0.322, 0.301, 0.278, 0.255, 0.234, 0.224},
		{0, 0.227, 0.246, 0.269, 0.294, 0.313, 0.326, 0.322, 0.302, 0.279, 0.254, 0.233, 0.222},
		{0, 0.225, 0.245, 0.269, 0.295, 0.316, 0.327, 0.322, 0.304, 0.279, 0.253, 0.231, 0.220},
		{0, 0.223, 0.244, 0.269, 0.296, 0.318, 0.330, 0.324, 0.305, 0.279, 0.252, 0.229, 0.218},
		{0, 0.222, 0.243, 0.269, 0.296, 0.319, 0.332, 0.326, 0.305, 0.279, 0.252, 0.228, 0.215},
		{0, 0.220, 0.241, 0.269, 0.297, 0.321, 0.334, 0.328, 0.307, 0.279, 0.251, 0.226, 0.213},
		{0, 0.218, 0.240, 0.268, 0.298, 0.323, 0.336, 0.330, 0.308, 0.280, 0.250, 0.224, 0.211},
		{0, 0.215, 0.239, 0.268, 0.299, 0.325, 0.339, 0.332, 0.309, 0.280, 0.249, 0.223, 0.209},
		{0, 0.213, 0.238, 0.268, 0.300, 0.327, 0.341, 0.334, 0.310, 0.280, 0.248, 0.221, 0.206},
		{0, 0.211, 0.236, 0.268, 0.301, 0.329, 0.344, 0.336, 0.312, 0.280, 0.247, 0.219, 0.203},
		{0, 0.209, 0.235, 0.267, 0.302, 0.331, 0.346, 0.338, 0.313, 0.280, 0.246, 0.217, 0.201},
		{0, 0.207, 0.233, 0.267, 0.303, 0.333, 0.349, 0.341, 0.315, 0.281, 0.245, 0.215, 0.198},
		{0, 0.204, 0.232, 0.267, 0.304, 0.335, 0.351, 0.344, 0.316, 0.281, 0.245, 0.212, 0.195},
		{0, 0.202, 0.231, 0.267, 0.305, 0.337, 0.354, 0.346, 0.317, 0.281, 0.243, 0.210, 0.192},
		{0, 0.199, 0.229, 0.266, 0.306, 0.339, 0.357, 0.349, 0.319, 0.282, 0.242, 0.208, 0.189},
		{0, 0.196, 0.227, 0.266, 0.307, 0.342, 0.361, 0.352, 0.321, 0.282, 0.241, 0.205, 0.185},
		{0, 0.193, 0.226, 0.266, 0.308, 0.345, 0.364, 0.355, 0.322, 0.282, 0.240, 0.203, 0.182},
		{0, 0.190, 0.224, 0.265, 0.309, 0.347, 0.368, 0.358, 0.324, 0.282, 0.239, 0.200, 0.179},
		{0, 0.187, 0.222, 0.265, 0.311, 0.350, 0.371, 0.361, 0.326, 0.283, 0.237, 0.197, 0.175},
		{0, 0.183, 0.220, 0.265, 0.312, 0.353, 0.376, 0.365, 0.328, 0.283, 0.236, 0.194, 0.171},
		{0, 0.179, 0.218, 0.264, 0.313, 0.356, 0.380, 0.368, 0.330, 0.283, 0.235, 0.191, 0.166},
		{0, 0.175, 0.216, 0.264, 0.315, 0.360, 0.385, 0.372, 0.332, 0.284, 0.233, 0.188, 0.162},
		{0, 0.171, 0.214, 0.264, 0.316, 0.363, 0.389, 0.376, 0.334, 0.284, 0.232, 0.184, 0.157},
		{0, 0.167, 0.211, 0.263, 0.318, 0.368, 0.395, 0.381, 0.337, 0.284, 0.230, 0.180, 0.152},
		{0, 0.162, 0.209, 0.263, 0.320, 0.372, 0.400, 0.385, 0.339, 0.284, 0.228, 0.177, 0.146},
		{0, 0.157, 0.206, 0.262, 0.321, 0.376, 0.407, 0.391, 0.342, 0.285, 0.226, 0.172, 0.140},
		{0, 0.152, 0.203, 0.262, 0.323, 0.380, 0.414, 0.397, 0.345, 0.285, 0.224, 0.167, 0.134},
		{0, 0.145, 0.199, 0.261, 0.325, 0.385, 0.422, 0.404, 0.348, 0.285, 0.222, 0.162, 0.126},
		{0, 0.139, 0.196, 0.260, 0.327, 0.391, 0.431, 0.411, 0.351, 0.285, 0.219, 0.157, 0.118},
		{0, 0.131, 0.192, 0.259, 0.329, 0.397, 0.443, 0.419, 0.354, 0.285, 0.217, 0.150, 0.108},
		{0, 0.123, 0.188, 0.258, 0.331, 0.403, 0.454, 0.428, 0.357, 0.285, 0.214, 0.144, 0.097}
	 };
	  std::copy(&local_sunDurRatioForAday[0][0], 
		  &local_sunDurRatioForAday[0][0] + xRow_Lat * yCol_Month, &sunDurRatioForAday[0][0]);

	 return 1;
 }

 //북위/월	1	2	3	4	5	6	7	8	9	10	11	12
	// 0	0.274	0.274	0.274	0.274	0.274	0.274	0.274	0.274	0.274	0.274	0.274	0.274
	// 1	0.273	0.273	0.274	0.275	0.275	0.275	0.275	0.275	0.274	0.273	0.273	0.273
	// 2	0.272	0.273	0.274	0.275	0.276	0.276	0.276	0.275	0.274	0.273	0.272	0.272
	// 3	0.271	0.272	0.274	0.276	0.277	0.278	0.278	0.276	0.274	0.272	0.271	0.270
	// 4	0.270	0.271	0.273	0.276	0.278	0.279	0.279	0.276	0.274	0.272	0.270	0.269
	// 5	0.268	0.270	0.273	0.277	0.279	0.280	0.280	0.277	0.275	0.271	0.269	0.268
	// 6	0.267	0.270	0.273	0.277	0.280	0.282	0.281	0.278	0.275	0.271	0.268	0.266
	// 7	0.266	0.269	0.273	0.278	0.281	0.283	0.282	0.279	0.275	0.270	0.267	0.265
	// 8	0.265	0.268	0.273	0.278	0.282	0.284	0.283	0.279	0.275	0.270	0.266	0.264
	// 9	0.264	0.268	0.273	0.279	0.283	0.286	0.284	0.280	0.275	0.270	0.265	0.262
	// 10	0.263	0.267	0.273	0.279	0.284	0.287	0.285	0.281	0.275	0.269	0.264	0.261
	// 11	0.261	0.266	0.272	0.280	0.285	0.288	0.287	0.282	0.275	0.269	0.263	0.260
	// 12	0.260	0.265	0.272	0.280	0.286	0.290	0.288	0.283	0.275	0.268	0.262	0.258
	// 13	0.259	0.265	0.272	0.281	0.287	0.292	0.289	0.283	0.276	0.267	0.260	0.257
	// 14	0.257	0.264	0.272	0.281	0.288	0.293	0.290	0.284	0.276	0.267	0.259	0.256
	// 15	0.256	0.263	0.272	0.282	0.290	0.294	0.292	0.285	0.276	0.266	0.258	0.254
	// 16	0.255	0.263	0.272	0.282	0.291	0.295	0.293	0.285	0.276	0.265	0.257	0.253
	// 17	0.254	0.262	0.271	0.283	0.292	0.296	0.294	0.286	0.276	0.265	0.256	0.251
	// 18	0.253	0.261	0.271	0.283	0.293	0.298	0.295	0.287	0.276	0.265	0.255	0.250
	// 19	0.251	0.260	0.271	0.284	0.294	0.299	0.297	0.288	0.276	0.264	0.254	0.248
	// 20	0.250	0.259	0.271	0.284	0.295	0.301	0.298	0.289	0.276	0.264	0.253	0.247
	// 21	0.249	0.258	0.271	0.285	0.296	0.302	0.299	0.290	0.277	0.263	0.252	0.245
	// 22	0.247	0.258	0.271	0.285	0.297	0.304	0.301	0.291	0.277	0.262	0.250	0.244
	// 23	0.246	0.257	0.271	0.286	0.297	0.307	0.302	0.291	0.277	0.262	0.249	0.242
	// 24	0.245	0.256	0.271	0.287	0.297	0.310	0.303	0.292	0.277	0.261	0.248	0.241
	// 25	0.243	0.255	0.270	0.287	0.300	0.310	0.305	0.293	0.277	0.261	0.247	0.239
	// 26	0.242	0.254	0.270	0.288	0.302	0.310	0.306	0.294	0.277	0.260	0.245	0.238
	// 27	0.240	0.253	0.270	0.289	0.304	0.311	0.308	0.295	0.277	0.259	0.244	0.236
	// 28	0.239	0.253	0.270	0.289	0.305	0.313	0.309	0.296	0.277	0.259	0.243	0.235
	// 29	0.237	0.252	0.270	0.290	0.306	0.315	0.310	0.296	0.278	0.258	0.241	0.233
	// 30	0.236	0.251	0.270	0.290	0.308	0.316	0.312	0.297	0.278	0.258	0.240	0.231
	// 31	0.234	0.250	0.270	0.291	0.309	0.318	0.314	0.298	0.278	0.257	0.239	0.229
	// 32	0.232	0.249	0.270	0.292	0.310	0.320	0.315	0.299	0.278	0.256	0.237	0.227
	// 33	0.231	0.248	0.270	0.293	0.310	0.322	0.319	0.300	0.278	0.256	0.236	0.225
	// 34	0.229	0.247	0.269	0.293	0.310	0.324	0.322	0.301	0.278	0.255	0.234	0.224
	// 35	0.227	0.246	0.269	0.294	0.313	0.326	0.322	0.302	0.279	0.254	0.233	0.222
	// 36	0.225	0.245	0.269	0.295	0.316	0.327	0.322	0.304	0.279	0.253	0.231	0.220
	// 37	0.223	0.244	0.269	0.296	0.318	0.330	0.324	0.305	0.279	0.252	0.229	0.218
	// 38	0.222	0.243	0.269	0.296	0.319	0.332	0.326	0.305	0.279	0.252	0.228	0.215
	// 39	0.220	0.241	0.269	0.297	0.321	0.334	0.328	0.307	0.279	0.251	0.226	0.213
	// 40	0.218	0.240	0.268	0.298	0.323	0.336	0.330	0.308	0.280	0.250	0.224	0.211
	// 41	0.215	0.239	0.268	0.299	0.325	0.339	0.332	0.309	0.280	0.249	0.223	0.209
	// 42	0.213	0.238	0.268	0.300	0.327	0.341	0.334	0.310	0.280	0.248	0.221	0.206
	// 43	0.211	0.236	0.268	0.301	0.329	0.344	0.336	0.312	0.280	0.247	0.219	0.203
	// 44	0.209	0.235	0.267	0.302	0.331	0.346	0.338	0.313	0.280	0.246	0.217	0.201
	// 45	0.207	0.233	0.267	0.303	0.333	0.349	0.341	0.315	0.281	0.245	0.215	0.198
	// 46	0.204	0.232	0.267	0.304	0.335	0.351	0.344	0.316	0.281	0.245	0.212	0.195
	// 47	0.202	0.231	0.267	0.305	0.337	0.354	0.346	0.317	0.281	0.243	0.210	0.192
	// 48	0.199	0.229	0.266	0.306	0.339	0.357	0.349	0.319	0.282	0.242	0.208	0.189
	// 49	0.196	0.227	0.266	0.307	0.342	0.361	0.352	0.321	0.282	0.241	0.205	0.185
	// 50	0.193	0.226	0.266	0.308	0.345	0.364	0.355	0.322	0.282	0.240	0.203	0.182
	// 51	0.190	0.224	0.265	0.309	0.347	0.368	0.358	0.324	0.282	0.239	0.200	0.179
	// 52	0.187	0.222	0.265	0.311	0.350	0.371	0.361	0.326	0.283	0.237	0.197	0.175
	// 53	0.183	0.220	0.265	0.312	0.353	0.376	0.365	0.328	0.283	0.236	0.194	0.171
	// 54	0.179	0.218	0.264	0.313	0.356	0.380	0.368	0.330	0.283	0.235	0.191	0.166
	// 55	0.175	0.216	0.264	0.315	0.360	0.385	0.372	0.332	0.284	0.233	0.188	0.162
	// 56	0.171	0.214	0.264	0.316	0.363	0.389	0.376	0.334	0.284	0.232	0.184	0.157
	// 57	0.167	0.211	0.263	0.318	0.368	0.395	0.381	0.337	0.284	0.230	0.180	0.152
	// 58	0.162	0.209	0.263	0.320	0.372	0.400	0.385	0.339	0.284	0.228	0.177	0.146
	// 59	0.157	0.206	0.262	0.321	0.376	0.407	0.391	0.342	0.285	0.226	0.172	0.140
	// 60	0.152	0.203	0.262	0.323	0.380	0.414	0.397	0.345	0.285	0.224	0.167	0.134
	// 61	0.145	0.199	0.261	0.325	0.385	0.422	0.404	0.348	0.285	0.222	0.162	0.126
	// 62	0.139	0.196	0.260	0.327	0.391	0.431	0.411	0.351	0.285	0.219	0.157	0.118
	// 63	0.131	0.192	0.259	0.329	0.397	0.443	0.419	0.354	0.285	0.217	0.150	0.108
	// 64	0.123	0.188	0.258	0.331	0.403	0.454	0.428	0.357	0.285	0.214	0.144	0.097

 

 
 int setSolarRadiation()
 {
	 vector<weatherData> wdinfo;
	 wdinfo = readAndSetWeatherData(prj.fpnSolarRadData, prj.solarRadDataType,
		  prj.solarRadInterval_min, "Solar radiation");
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading solar radiation data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 solarRad = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), solarRad);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.solarRadInterval_min;
		 int quotient = simDur_min / prj.solarRadInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the solar radiation data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }

 int setDewPointTemp()
 {
	 vector<weatherData> wdinfo;
	 wdinfo = readAndSetWeatherData(prj.fpnDewPointTemp, prj.dewPointTempDataType,
		  prj.dewPointTempInterval_min, "Dew point temperature");
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading dew point temperature data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 dewPointTemp = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), dewPointTemp);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.dewPointTempInterval_min;
		 int quotient = simDur_min / prj.dewPointTempInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the dew point temperature data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }
 
 int setWindSpeed()
 {
	 vector<weatherData> wdinfo;
	 wdinfo = readAndSetWeatherData(prj.fpnWindSpeed, prj.windSpeedDataType,
		  prj.windSpeedInterval_min, "Wind speed");
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading wind speed data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 windSpeed = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), windSpeed);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.windSpeedInterval_min;
		 int quotient = simDur_min / prj.windSpeedInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the wind speed data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }

 int setUserET(int dataType) //dataType : PET (1), AET (2)
 {
	 vector<weatherData> wdinfo;
	 if (dataType == 1) {
		 wdinfo = readAndSetWeatherData(prj.fpnUserPET, prj.userPETDataType,
			 prj.userETInterval_min, "User defined potential evapotranspiration");
	 }
	 else if (dataType == 2)
	 {
		 wdinfo = readAndSetWeatherData(prj.fpnUserAET, prj.userAETDataType,
			 prj.userETInterval_min, "User defined actual evapotranspiration");
	 }
	 else {
		 writeLogString(fpnLog, "ERROR : Reading user defined evapotranspiration data file was failed. \n", 1, 1);
		 return -1;
	 }
	 
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading user defined potential evapotranspiration data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 userET = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), userET);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.userETInterval_min;
		 int quotient = simDur_min / prj.userETInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the user defined evapotranspiration data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }

 int setSnowPackTemp()
 {
	 vector<weatherData> wdinfo;
	 wdinfo = readAndSetWeatherData(prj.fpnSnowpackTempData, prj.snowpackTempDataType,
		 prj.snowpackTempInterval_min, "Snowpack temperature");
	 if (wdinfo.size() == 0) {
		 writeLogString(fpnLog, "ERROR : Reading snowpack temperature data file was failed. \n", 1, 1);
		 return -1;
	 }
	 int nwd = (int)wdinfo.size();
	 snowpackTemp = new weatherData[nwd];
	 std::copy(wdinfo.begin(), wdinfo.end(), snowpackTemp);

	 if (prj.simType != simulationType::RealTime) {
		 int simDur_min = prj.simDuration_hr * 60;
		 double remainder = simDur_min % prj.snowpackTempInterval_min;
		 int quotient = simDur_min / prj.snowpackTempInterval_min;
		 int necesaryWDn = quotient;
		 if (remainder > 0.0) {
			 necesaryWDn++;
		 }
		 if (necesaryWDn > nwd) {
			 writeLogString(fpnLog, "ERROR : The number of the user defined potential evapotranspiration data is less than the simulation duration.\n", 1, 1);
			 return -1;
		 }
	 }
	 return 1;
 }

 int setBlaneyCriddleK() {

	 if (prj.fpnBlaneyCriddleK != "" // Project open에서 검증했는데, 여기서 또한번 검증
		 && fs::exists(lower(prj.fpnBlaneyCriddleK)) != true) {
		 writeLogString(fpnLog, "ERROR : The Blaney-Criddle K coefficient data file is invalid.\n", 1, 1);
		 return -1;
	 }
	 memset(blaneyCriddleKData, 0, 12 * sizeof(blaneyCriddleKData[0])); // 12 개의 자료가 있으므로,,
	 vector<double> Lines;
	 Lines = readTextFileToDoubleVector(prj.fpnBlaneyCriddleK);
	 if (Lines.size() == 0) {
		 writeLogString(fpnLog, "ERROR : The Blaney-Criddle K coefficient data file has no value.\n", 1, 1);
		 return -1;
	 }
	 if (Lines.size() > 12) {
		 string err = "ERROR : The Blaney-Criddle K coefficient data file data file is too long.\n";
		 err = err + "  The Blaney-Criddle K coefficient data needs only 12 values of monthly data.\n";
		 writeLogString(fpnLog, err, 1, 1);
		 return -1;
	 }
	 copy(Lines.begin(), Lines.end(), blaneyCriddleKData);
	 return 1;
 }

 weatherDataType getWeatherDataTypeByDataFile(string fpn_wdata) {
	 ifstream rfFile(lower(fpn_wdata));
	 string aline;
	 getline(rfFile, aline);
	 aline = trim(aline);
	 if (aline == "") {
		 writeLogString(fpnLog, "ERROR : Weather data file [" + fpn_wdata + "] is empty.\n", 1, 1);
		 return weatherDataType::None;
	 }

	 aline = eraseCR(aline);  //\r 문자가 남는경우에 \r 을 삭제
	 if (aline.length()<254 && fs::exists(lower(aline)) == true) {//MP 수정 win에서는 259자, Linux에서는 255자까지 인식
		 return weatherDataType::Raster_ASC;
	 }
	 else {
		 vector<string> parts = splitToStringVector(aline, ',');
		 if (parts.size() > 1 && isNumeric(parts[0]) == true) {
			 return weatherDataType::Mean_DividedArea;
		 }
		 if (parts.size() == 1 && isNumeric(parts[0]) == true) {
			 return weatherDataType::Mean;
		 }
	 }
	 return weatherDataType::None;
 }
 
 vector<int> setWSIDsInWeatherDataFile(string aStringLine) {
	 vector<string> strings;
	 vector<int> wsids;
	 strings = splitToStringVector(aStringLine, ',');
	 int nv = strings.size();
	 for (int i = 0; i < nv; ++i) {
		 if (isNumericInt(strings[i]) == true) {
			 wsids.push_back(stoi(strings[i]));
		 }
		 else {
			 writeLogString(fpnLog, "ERROR : Watershed IDs in [" + prj.fpnRainfallData + "] are invalid.\n", 1, 1);
			 wsids.clear();
			 break;
		 }
	 }
	 return wsids;
 }
 
 vector<weatherData> readAndSetWeatherData(string fpn_in_wd, weatherDataType wdType,
	 int dataInterval_min, string dataString) {
	 vector<weatherData> nullWD;
	 if (fpn_in_wd != "" 
		 && fs::exists(lower(fpn_in_wd)) != true) {
		 writeLogString(fpnLog, "ERROR : "+dataString + " data file is invalid.\n", 1, 1);
		 return nullWD;
	 }
	 vector<weatherData> wdToReturn;
	 wdToReturn.clear();
	 fs::path fpn_in = fs::path(fpn_in_wd);
	 string fp_in = fpn_in.parent_path().string();
	 string fn_in = fpn_in.filename().string();
	 vector<string> Lines;
	 vector<int> wsids;
	 Lines = readTextFileToStringVector(fpn_in_wd);
	 if (wdType == weatherDataType::Mean_DividedArea) {
		 wsids = setWSIDsInWeatherDataFile(Lines[0]);
		 if (wsids.size() == 0) {
			 return nullWD;
		 }
	 }
	 int time_order = 0;
	 for (int n = 0; n < Lines.size(); n++) {
		 if (trim(Lines[n]) == "") { break; }
		 weatherData wd;
		 switch (wdType)
		 {
		 case weatherDataType::Raster_ASC: {
			 wd.Order = n + 1;
			 time_order = n;
			 fs::path fpn = fs::path(eraseCR(Lines[n]).c_str());
			 wd.value = fpn.filename().string();
			 wd.FileName = fpn.filename().string();
			 wd.FilePath = fpn.parent_path().string();
			 break;
		 }
		 case weatherDataType::Mean: {
			 wd.Order = n + 1;
			 time_order = n;
			 string value = Lines[n];
			 if (isNumeric(value) == true) {
				 wd.value = value;
			 }
			 else {
				 string err = "ERROR : Can not read " + lower(dataString) 
					 + " value.\nOrder = "
					 + to_string(n + 1) + "\n";
				 writeLogString(fpnLog, err, 1, 1);
				 return nullWD;
			 }
			 wd.FileName = fn_in;
			 wd.FilePath = fp_in;
			 break;
		 }
		 case weatherDataType::Mean_DividedArea: {
			 if (n > 0) {
				 wd.Order = n;
				 time_order = n-1;
				 vector<double> values = splitToDoubleVector(Lines[n], ',');
				 int nv = values.size();
				 if (wsids.size() != nv) {
					 string err = "ERROR :  The number of " + dataString 
						 + " data is not match to the number of watershed ids in the file below.\n"
						 + " File name : " + fpn_in_wd + +"\n"
						 + " Line number = " + to_string(n + 1) + "\n";
					 writeLogString(fpnLog, err, 1, 1);
					 return nullWD;
				 }
				 for (int i = 0; i < nv; ++i) {
					 wd.vForEachRegion[wsids[i]] = values[i]; // wsid 순서대로 값을 저장	
				 }
				 wd.FileName = fn_in;
				 wd.FilePath = fp_in;			 
			 }
			 break;
		 }
		 }
		 if (prj.isDateTimeFormat == 1) {
			 wd.DataTime = timeElaspedToDateTimeFormat2(prj.simStartTime,
				 dataInterval_min * 60 * time_order, timeUnitToShow::toM,
				 dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		 }
		 else {
			 wd.DataTime = to_string(dataInterval_min * time_order);
		 }
		 if (wd.Order > 0) {
			 wdToReturn.push_back(wd);
		 }
	 }
	 return wdToReturn;
 }
