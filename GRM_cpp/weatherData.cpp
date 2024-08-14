#include "stdafx.h"
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

double svpGradient[100];
extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern cvAtt* cvs;
extern cvpos* cvps;
extern domaininfo di;
extern thisSimulation ts;

extern vector<weatherData> rfs;
extern vector<weatherData> tempMax;
extern vector<weatherData> tempMin;
extern vector<weatherData> dayTimeLength;
extern vector<weatherData> snowpackTemp;

extern double sunDurRatio[12];
extern vector<weatherData> solarRad;
extern map<int, double[12]> laiRatio; // <LCvalue, 월별 laiRatio 12개>
extern double blaneyCriddleKData[12];
extern wpSimData wpSimValue;

int setRainfallData()
{
	rfs.clear();
	rfs = readAndSetWeatherData(prj.fpnRainfallData, prj.rfDataType,
		prj.rfinterval_min, "Precipitation");
	if (rfs.size() == 0) {
		writeLog(fpnLog, "ERROR : Reading precipitation data file was failed\n", 1, 1);
		return -1;
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
		fpnRF = rfs[order - 1].FilePath + "\\" + rfs[order - 1].FileName;
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
		fpnRF = rfs[order - 1].FilePath + "\\" + rfs[order - 1].FileName;
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
		writeLog(fpnLog, "ERROR : Rainfall data type is invalid.\n", 1, 1);
		return -1;
	}
	if (returnv == -1) {
		writeLog(fpnLog, "ERROR : An error was occurred while reading rainfall data.\n", 1, 1);
		writeLog(fpnLog, "  Rainfall file : " + fpnRF + ".\n", 1, 1);
	}
	return returnv;
}

 void setCVRFintensityAndDTrf_Zero()
{
	 writeLog(fpnLog, "WARNNING : The precipitation data ended before the simulation was over. Precipitation was set to 0.\n", 1, -1);
	 writeLog(fpnLog, "WARNNING : If the time step of precipitation data is equal or smaller than printing time step, add more data. Or decrease simulation duration.\n", 1, -1);
	 ts.rfAveForDT_m = 0;
	 ts.rfAveSumAllCells_PT_m = 0;
	 ts.rfAveSumAllCells_PTave_m = 0;
     ts.rfiSumAllCellsInCurRFData_mPs = 0;
#pragma omp parallel for
     for (int i = 0; i < di.cellNnotNull; ++i)    {
		 cvs[i].rfiRead_mPsec = 0;
		 cvs[i].rfiRead_After_iniLoss_mPsec = 0;
        cvs[i].rfApp_mPdt = 0;
    }

    for(int wpcvid : wpSimValue.wpCVidxes)    {
        wpSimValue.prcpWPGridForPT_mm[wpcvid] = 0;
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
	 ts.rfAveSumAllCells_PT_m = 0;
	 ts.rfAveSumAllCells_PTave_m = 0;
	 ts.rfiSumAllCellsInCurRFData_mPs = 0;
	 for (int wpcvid : wpSimValue.wpCVidxes) {
		 wpSimValue.prcpWPGridForPT_mm[wpcvid] = 0;
		 wpSimValue.prcpUpWSAveForDt_mm[wpcvid] = 0;
		 wpSimValue.prcpiReadSumUpWS_mPs[wpcvid] = 0;
		 wpSimValue.prcpUpWSAveForPT_mm[wpcvid] = 0;
	 }
 }
 
 int setLAIRatio()
 {
	 if (prj.fpnLAI != "" && _access(prj.fpnLAI.c_str(), 0) != 0) {
		 writeLog(fpnLog, "ERROR : LAI ratio file is invalid.\n", 1, 1);
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
  

 int setCVTempMax(int order)
 {
	 string fpnTmax = "";
	 int returnv = -1;
	 if (prj.tempMaxDataType == weatherDataType::Raster_ASC
		 || prj.tempMaxDataType == weatherDataType::Mean_DividedArea) {
		 fpnTmax = tempMax[order - 1].FilePath + "\\" + tempMax[order - 1].FileName;
		 ascRasterFile* tMaxAsc;
		 map<int, double> idNv;
		 if (prj.tempMaxDataType == weatherDataType::Raster_ASC) {
			 tMaxAsc = new ascRasterFile(fpnTmax);
			 if (compareASCwithDomain(fpnTmax, "max. temperature", tMaxAsc->header, di.nCols, di.nRows, di.dx) == -1) {
				 return -1;
			 }
			 idNv.clear();
		 }
		 else {
			 idNv = tempMax[order - 1].vForEachRegion;
			 tMaxAsc = NULL;
		 }
		 omp_set_num_threads(prj.mdp);
#pragma omp parallel for
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 if (prj.tempMaxDataType == weatherDataType::Raster_ASC) {
				 cvs[i].tempMaxPday = tMaxAsc->valuesFromTL[cvps[i].xCol][cvps[i].yRow];
			 }
			 else {
				 cvs[i].tempMaxPday = idNv[cvps[i].wsid];
			 }
		 }
		 if (prj.tempMaxDataType == weatherDataType::Raster_ASC
			 && tMaxAsc != NULL) {
			 delete tMaxAsc;
		 }
		 returnv = 1;
	 }
	 else if (prj.tempMaxDataType == weatherDataType::Mean) {
		 fpnTmax = tempMax[order - 1].FilePath + "\\" + tempMax[order - 1].FileName;
		 string value = tempMax[order - 1].value;
		 double inTmax = stod(value);
#pragma omp parallel for schedule(guided)
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
			 // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
			 //if (cvs[i].toBeSimulated == -1) { continue; }
			 cvs[i].tempMaxPday = inTmax;
		 }
		 returnv = 1;
	 }
	 else {
		 writeLog(fpnLog, "ERROR : Max temperature data type is invalid.\n", 1, 1);
		 return -1;
	 }
	 if (returnv == -1) {
		 writeLog(fpnLog, "ERROR : An error was occurred while reading max temperature data.\n", 1, 1);
		 writeLog(fpnLog, " Max temperature data file : " + fpnTmax + ".\n", 1, 1);
	 }
	 return returnv;
 }

 void setCVTempMaxZero() {
	 writeLog(fpnLog, "WARNNING : The max temperature data ended before the simulation was over. Max temperature data was set as 0.\n", 1, -1);
	 writeLog(fpnLog, "WARNNING : If the time step of max temperature data is equal or smaller than printing time step, add more data. Or decrease simulation duration.\n", 1, -1);
#pragma omp parallel for schedule(guided)
	 for (int i = 0; i < di.cellNnotNull; ++i) {
		 cvs[i].tempMaxPday = 0.0;
	 }
 }


 int setCVTempMin(int order)
 {
	 string fpnTmin = "";
	 int returnv = -1;
	 if (prj.tempMinDataType == weatherDataType::Raster_ASC
		 || prj.tempMinDataType == weatherDataType::Mean_DividedArea) {
		 fpnTmin = tempMin[order - 1].FilePath + "\\" + tempMin[order - 1].FileName;
		 ascRasterFile* tMinAsc;
		 map<int, double> idNv;
		 if (prj.tempMinDataType == weatherDataType::Raster_ASC) {
			 tMinAsc = new ascRasterFile(fpnTmin);
			 if (compareASCwithDomain(fpnTmin, "min. temperature", tMinAsc->header, di.nCols, di.nRows, di.dx) == -1) {
				 return -1;
			 }
			 idNv.clear();
		 }
		 else {
			 idNv = tempMin[order - 1].vForEachRegion;
			 tMinAsc = NULL;
		 }
		 omp_set_num_threads(prj.mdp);
#pragma omp parallel for
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 if (prj.tempMinDataType == weatherDataType::Raster_ASC) {
				 cvs[i].tempMinPday = tMinAsc->valuesFromTL[cvps[i].xCol][cvps[i].yRow];
			 }
			 else {
				 cvs[i].tempMinPday = idNv[cvps[i].wsid];
			 }
		 }
		 if (prj.tempMinDataType == weatherDataType::Raster_ASC
			 && tMinAsc != NULL) {
			 delete tMinAsc;
		 }
		 returnv = 1;
	 }
	 else if (prj.tempMinDataType == weatherDataType::Mean) {
		 fpnTmin = tempMin[order - 1].FilePath + "\\" + tempMin[order - 1].FileName;
		 string value = tempMin[order - 1].value;
		 double inTmin = stod(value);
#pragma omp parallel for schedule(guided)
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
			 // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
			 //if (cvs[i].toBeSimulated == -1) { continue; }
			 cvs[i].tempMinPday = inTmin;
		 }
		 returnv = 1;
	 }
	 else {
		 writeLog(fpnLog, "ERROR : Min temperature data type is invalid.\n", 1, 1);
		 return -1;
	 }

	 // 여기서 온도 검증 ===================
	 int tempNormal = 1;
#pragma omp parallel for schedule(guided)
	 for (int i = 0; i < di.cellNnotNull; ++i) {
		 if (cvs[i].tempMinPday > cvs[i].tempMaxPday) {
			 tempNormal = -1;
		 }
	 }
	 if (tempNormal == -1) {
		 writeLog(fpnLog, "ERROR : The max temperature is lower than min temperature. Please check max and min temperatrue data.\n", 1, 1);
		 returnv = -1;
	 }
	 //====================

	 if (returnv == -1) {
		 writeLog(fpnLog, "ERROR : An error was occurred while reading min temperature data.\n", 1, 1);
		 writeLog(fpnLog, "  Min temperature data file : " + fpnTmin 
			 + ", data order : "+ to_string(order)+".\n", 1, 1);
	 }
	 return returnv;
 }


 void setCVTempMinZero() {
	 writeLog(fpnLog, "WARNNING : The min temperature data ended before the simulation was over. Min temperature data was set as 0.\n", 1, -1);
	 writeLog(fpnLog, "WARNNING : If the time step of min temperature data is equal or smaller than printing time step, add more data. Or decrease simulation duration.\n", 1, -1);
#pragma omp parallel for schedule(guided)
	 for (int i = 0; i < di.cellNnotNull; ++i) {
		 cvs[i].tempMinPday = 0.0;
	 }
 }

 int setCVSolarRad(int order)
 {
	 string fpnData = "";
	 int returnv = -1;
	 if (prj.solarRadDataType == weatherDataType::Raster_ASC
		 || prj.solarRadDataType == weatherDataType::Mean_DividedArea) {
		 fpnData = solarRad[order - 1].FilePath + "\\" + solarRad[order - 1].FileName;
		 ascRasterFile* SRAsc;
		 map<int, double> idNv;
		 if (prj.solarRadDataType == weatherDataType::Raster_ASC) {
			 SRAsc = new ascRasterFile(fpnData);
			 if (compareASCwithDomain(fpnData, "solar radiation", SRAsc->header, di.nCols, di.nRows, di.dx) == -1) {
				 return -1;
			 }
			 idNv.clear();
		 }
		 else {
			 idNv = solarRad[order - 1].vForEachRegion;
			 SRAsc = NULL;
		 }
		 omp_set_num_threads(prj.mdp);
#pragma omp parallel for
		 for (int i = 0; i < di.cellNnotNull; ++i) {			 
			 if (prj.solarRadDataType == weatherDataType::Raster_ASC) {
				 cvs[i].solarRad_mm = SRAsc->valuesFromTL[cvps[i].xCol][cvps[i].yRow]; // 입력자료 단위 mm/day 가 그대로 방정식에 이용된다.
			 }
			 else {
				 cvs[i].solarRad_mm = idNv[cvps[i].wsid];  // 입력자료 단위 mm/day 가 그대로 방정식에 이용된다.
			 }
		 }
		 if (prj.solarRadDataType == weatherDataType::Raster_ASC
			 && SRAsc != NULL) {
			 delete SRAsc;
		 }
		 returnv = 1;
	 }
	 else if (prj.solarRadDataType == weatherDataType::Mean) {
		 fpnData = solarRad[order - 1].FilePath + "\\" + solarRad[order - 1].FileName;
		 string value = solarRad[order - 1].value;
		 double inSR = stod(value);
#pragma omp parallel for schedule(guided)
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
			 // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
			 //if (cvs[i].toBeSimulated == -1) { continue; }
			 cvs[i].solarRad_mm = inSR;
		 }
		 returnv = 1;
	 }
	 else {
		 writeLog(fpnLog, "ERROR : Solar radiation data type is invalid.\n", 1, 1);
		 return -1;
	 }
	 if (returnv == -1) {
		 writeLog(fpnLog, "ERROR : An error was occurred while reading solar radiation data.\n", 1, 1);
		 writeLog(fpnLog, "  Solar radiation data file : " + fpnData + ".\n", 1, 1);
	 }
	 return returnv;
 }

 void setCVSolarRZero() {
	 writeLog(fpnLog, "WARNNING : The solar radiation data ended before the simulation was over. Solar radiation data was set as 0.\n", 1, -1);
	 writeLog(fpnLog, "WARNNING : If the time step of solar radiation data is equal or smaller than printing time step, add more data. Or decrease simulation duration.\n", 1, -1);
#pragma omp parallel for schedule(guided)
	 for (int i = 0; i < di.cellNnotNull; ++i) {
		 cvs[i].solarRad_mm = 0.0;
	 }
 }

 int setCVDayTimeLength(int order)
 {
	 string fpnData = "";
	 int returnv = -1;
	 if (prj.daytimeLengthDataType == weatherDataType::Raster_ASC
		 || prj.daytimeLengthDataType == weatherDataType::Mean_DividedArea) {
		 fpnData = dayTimeLength[order - 1].FilePath + "\\" + dayTimeLength[order - 1].FileName;
		 ascRasterFile* SDAsc;
		 map<int, double> idNv;
		 if (prj.tempMaxDataType == weatherDataType::Raster_ASC) {
			 SDAsc = new ascRasterFile(fpnData);
			 if (compareASCwithDomain(fpnData, "daytime length", SDAsc->header, di.nCols, di.nRows, di.dx) == -1) {
				 return -1;
			 }
			 idNv.clear();
		 }
		 else {
			 idNv = dayTimeLength[order - 1].vForEachRegion;
			 SDAsc = NULL;
		 }
		 omp_set_num_threads(prj.mdp);
#pragma omp parallel for
		 for (int i = 0; i < di.cellNnotNull; ++i) {			
			 if (prj.daytimeLengthDataType == weatherDataType::Raster_ASC) {
				 cvs[i].daytimeLength_hrs = SDAsc->valuesFromTL[cvps[i].xCol][cvps[i].yRow];  // 입력자료 단위 mm/day 가 그대로 방정식에 이용된다.
			 }
			 else {
				 cvs[i].daytimeLength_hrs = idNv[cvps[i].wsid];  // 입력자료 단위 mm/day 가 그대로 방정식에 이용된다.
			 }
		 }
		 if (prj.daytimeLengthDataType == weatherDataType::Raster_ASC
			 && SDAsc != NULL) {
			 delete SDAsc;
		 }
		 returnv = 1;
	 }
	 else if (prj.daytimeLengthDataType == weatherDataType::Mean) {
		 fpnData = dayTimeLength[order - 1].FilePath + "\\" + dayTimeLength[order - 1].FileName;
		 string value = dayTimeLength[order - 1].value;
		 double inSD = stod(value);
#pragma omp parallel for schedule(guided)
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
			 // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
			 //if (cvs[i].toBeSimulated == -1) { continue; }
			 cvs[i].daytimeLength_hrs = inSD;
		 }
		 returnv = 1;
	 }
	 else {
		 writeLog(fpnLog, "ERROR : Dunration of sunshine data type is invalid.\n", 1, 1);
		 return -1;
	 }
	 if (returnv == -1) {
		 writeLog(fpnLog, "ERROR : An error was occurred while reading dunration of sunshine data.\n", 1, 1);
		 writeLog(fpnLog, "  Dunration of sunshine data file : " + fpnData + ".\n", 1, 1);
	 }
	 return returnv;
 }

 void setCVDayTimeLengthZero() {
	 writeLog(fpnLog, "WARNNING : The daytime length data ended before the simulation was over. Daytime length data was set as 0.\n", 1, -1);
	 writeLog(fpnLog, "WARNNING : If the time step of daytime length data is equal or smaller than printing time step, add more data. Or decrease simulation duration.\n", 1, -1);
#pragma omp parallel for schedule(guided)
	 for (int i = 0; i < di.cellNnotNull; ++i) {
		 cvs[i].daytimeLength_hrs = 0.0;
	 }
 }

 int setCVSnowpackTemp(int order)
 {
	 string fpnSD = "";
	 int returnv = -1;
	 if (prj.snowpackTempDataType == weatherDataType::Raster_ASC
		 || prj.snowpackTempDataType == weatherDataType::Mean_DividedArea) {
		 fpnSD = snowpackTemp[order - 1].FilePath + "\\" + snowpackTemp[order - 1].FileName;
		 ascRasterFile* sptASC;
		 map<int, double> idNv;
		 if (prj.snowpackTempDataType == weatherDataType::Raster_ASC) {
			 sptASC = new ascRasterFile(fpnSD);
			 if (compareASCwithDomain(fpnSD, "snowpack temperature", sptASC->header, di.nCols, di.nRows, di.dx) == -1) {
				 return -1;
			 }
			 idNv.clear();
		 }
		 else {
			 idNv = snowpackTemp[order - 1].vForEachRegion;
			 sptASC = NULL;
		 }
		 omp_set_num_threads(prj.mdp);
#pragma omp parallel for
		 for (int i = 0; i < di.cellNnotNull; ++i) {			
			 if (prj.snowpackTempDataType == weatherDataType::Raster_ASC) {
				 cvs[i].spackTemp = sptASC->valuesFromTL[cvps[i].xCol][cvps[i].yRow];
			 }
			 else {
				 cvs[i].spackTemp = idNv[cvps[i].wsid];
			 }
		 }
		 if (prj.snowpackTempDataType == weatherDataType::Raster_ASC
			 && sptASC != NULL) {
			 delete sptASC;
		 }
		 returnv = 1;
	 }
	 else if (prj.snowpackTempDataType == weatherDataType::Mean) {
		 fpnSD = snowpackTemp[order - 1].FilePath + "\\" + snowpackTemp[order - 1].FileName;
		 string value = snowpackTemp[order - 1].value;
		 double inSPT = stod(value);
#pragma omp parallel for schedule(guided)
		 for (int i = 0; i < di.cellNnotNull; ++i) {
			 // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
			 // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
			 //if (cvs[i].toBeSimulated == -1) { continue; }
			 cvs[i].spackTemp = inSPT;
		 }
		 returnv = 1;
	 }
	 else {
		 writeLog(fpnLog, "ERROR : Snow pack temperature data type is invalid.\n", 1, 1);
		 return -1;
	 }
	 if (returnv == -1) {
		 writeLog(fpnLog, "ERROR : An error was occurred while reading snow pack temperature data.\n", 1, 1);
		 writeLog(fpnLog, " Snow pack temperature data file : " + fpnSD + ".\n", 1, 1);
	 }
	 return returnv;
 }

 void setCVSnowpackTempZero() {
	 writeLog(fpnLog, "WARNNING : The snow pack temperature data ended before the simulation was over. Snow pack temperature data was set as 0.\n", 1, -1);
	 writeLog(fpnLog, "WARNNING : If the time step ofsnow pack temperature data is equal or smaller than printing time step, add more data. Or decrease simulation duration.\n", 1, -1);
#pragma omp parallel for schedule(guided)
	 for (int i = 0; i < di.cellNnotNull; ++i) {
		 cvs[i].spackTemp = 0.0;
	 }
 }

  
 int setTemperatureMax()
 {
	 tempMax.clear();
	 tempMax = readAndSetWeatherData(prj.fpnTempMaxData, prj.tempMaxDataType,
		 prj.tempMaxInterval_min, "Maximum temperature");
	 if (tempMax.size() == 0) {
		 writeLog(fpnLog, "ERROR : Reading maximum temperature data file was failed\n", 1, 1);
		 return -1;
	 }
	 return 1;
 }
 
 int setTemperatureMin()
 {
	 tempMin.clear();
	 tempMin = readAndSetWeatherData(prj.fpnTempMinData, prj.tempMinDataType,
		 prj.tempMinInterval_min, "Minimum temperature");
	 if (tempMin.size() == 0) {
		 writeLog(fpnLog, "ERROR : Reading minimum temperature data file was failed\n", 1, 1);
		 return -1;
	 }
	 return 1;
 }
 
 int setDaytimeLength()
 {
	 dayTimeLength.clear();
	 dayTimeLength = readAndSetWeatherData(prj.fpnDaytimeLengthData, prj.daytimeLengthDataType,
		 prj.daytimeLengthDataInterval_min, "Daytime length");
	 if (dayTimeLength.size() == 0) {
		 writeLog(fpnLog, "ERROR : Reading daytime length data file was failed\n", 1, 1);
		 return -1;
	 }
	 return 1;
 }
   
 int setDaytimeHoursRatio()
 {
	 if (prj.fpnDaytimeLengthRatioData != "" && _access(prj.fpnDaytimeLengthRatioData.c_str(), 0) != 0) {
		 writeLog(fpnLog, "The ratio of daytime hours data file is invalid.\n", 1, 1);
		 return -1;
	 }
	 memset(sunDurRatio, 0, 12 * sizeof(sunDurRatio[0])); // 12 개의 자료가 있으므로,,
	 vector<double> Lines;
	 Lines = readTextFileToDoubleVector(prj.fpnDaytimeLengthRatioData);
	 if (Lines.size() == 0) { 
		 string err = "ERROR : The ratio of  daytime hours data file has no value.\n";		 
		 writeLog(fpnLog, err, 1, 1);
		 return -1; 
	 }
	 if (Lines.size() > 12) {
		 string err = "ERROR : The ratio of  daytime hours data file is too long.\n";
		 err = err + "The ratio of  daytime hours data needs only 12 values of monthly data.\n";
		 writeLog(fpnLog, err, 1, 1);
		 return -1;
	 }
	 copy(Lines.begin(), Lines.end(), sunDurRatio);
	 return 1;
 }
 
 int setSolarRadiation()
 {
	 solarRad.clear();
	 solarRad = readAndSetWeatherData(prj.fpnSolarRadData, prj.solarRadDataType,
		 prj.solarRadInterval_min, "Solar radiation");
	 if (solarRad.size() == 0) {
		 writeLog(fpnLog, "ERROR : Reading solar radiation data file was failed\n", 1, 1);
		 return -1;
	 }
	 return 1;
 }
 
 int setSnowPackTemp()
 {
	 snowpackTemp.clear();
	 snowpackTemp = readAndSetWeatherData(prj.fpnSnowpackTempData, prj.snowpackTempDataType,
		 prj.snowpackTempInterval_min, "Snowpack temperature");
	 if (snowpackTemp.size() == 0) {
		 writeLog(fpnLog, "ERROR : Reading snowpack temperature data file was failed\n", 1, 1);
		 return -1;
	 }
	 return 1;
 }

 int setBlaneyCriddleK() {

	 if (prj.fpnBlaneyCriddleK != "" && _access(prj.fpnBlaneyCriddleK.c_str(), 0) != 0) {
		 writeLog(fpnLog, "ERROR : The Blaney-Criddle K coefficient data file is invalid.\n", 1, 1);
		 return -1;
	 }
	 memset(blaneyCriddleKData, 0, 12 * sizeof(blaneyCriddleKData[0])); // 12 개의 자료가 있으므로,,
	 vector<double> Lines;
	 Lines = readTextFileToDoubleVector(prj.fpnBlaneyCriddleK);
	 if (Lines.size() == 0) {
		 string err = "ERROR : The Blaney-Criddle K coefficient data file has no value.\n";
		 writeLog(fpnLog, err, 1, 1);
		 return -1;
	 }
	 if (Lines.size() > 12) {
		 string err = "ERROR : The Blaney-Criddle K coefficient data file data file is too long.\n";
		 err = err + "  The Blaney-Criddle K coefficient data needs only 12 values of monthly data.\n";
		 writeLog(fpnLog, err, 1, 1);
		 return -1;
	 }
	 copy(Lines.begin(), Lines.end(), blaneyCriddleKData);
	 return 1;
 }

 // 기온별 포화증기압의 기울기를 미리 저장해 둔다
 // 기울기는 포화증기압 표에서 직접 계산
 void setSVPGradient() {
	 svpGradient[0] = 0.347; svpGradient[1] = 0.347; svpGradient[2] = 0.368; svpGradient[3] = 0.391;
	 svpGradient[4] = 0.416; svpGradient[5] = 0.444; svpGradient[6] = 0.468; svpGradient[7] = 0.500;
	 svpGradient[8] = 0.532; svpGradient[9] = 0.564; svpGradient[10] = 0.600; svpGradient[11] = 0.635;
	 svpGradient[12] = 0.674; svpGradient[13] = 0.713; svpGradient[14] = 0.756; svpGradient[15] = 0.801;
	 svpGradient[16] = 0.846; svpGradient[17] = 0.896; svpGradient[18] = 0.947; svpGradient[19] = 1.000;
	 svpGradient[20] = 1.058; svpGradient[21] = 1.115; svpGradient[22] = 1.177; svpGradient[23] = 1.241;
	 svpGradient[24] = 1.309; svpGradient[25] = 1.379; svpGradient[26] = 1.453; svpGradient[27] = 1.530;
	 svpGradient[28] = 1.610; svpGradient[29] = 1.694; svpGradient[30] = 1.781; svpGradient[31] = 1.871;
	 svpGradient[32] = 1.968; svpGradient[33] = 2.066; svpGradient[34] = 2.169; svpGradient[35] = 2.277;
	 svpGradient[36] = 2.388; svpGradient[37] = 2.504; svpGradient[38] = 2.625; svpGradient[39] = 2.750;
	 svpGradient[40] = 2.882; svpGradient[41] = 3.016; svpGradient[42] = 3.160; svpGradient[43] = 3.300;
	 svpGradient[44] = 3.460; svpGradient[45] = 3.620; svpGradient[46] = 3.770; svpGradient[47] = 3.950;
	 svpGradient[48] = 4.110; svpGradient[49] = 4.310; svpGradient[50] = 4.490; svpGradient[51] = 4.690;
	 svpGradient[52] = 4.890; svpGradient[53] = 5.110; svpGradient[54] = 5.310; svpGradient[55] = 5.530;
	 svpGradient[56] = 5.760; svpGradient[57] = 6.020; svpGradient[58] = 6.210; svpGradient[59] = 6.570;
	 svpGradient[60] = 6.780; svpGradient[61] = 7.050; svpGradient[62] = 7.340; svpGradient[63] = 7.610;
	 svpGradient[64] = 7.930; svpGradient[65] = 8.230; svpGradient[66] = 8.550; svpGradient[67] = 8.870;
	 svpGradient[68] = 9.210; svpGradient[69] = 9.560; svpGradient[70] = 9.970; svpGradient[71] = 10.200;
	 svpGradient[72] = 10.700; svpGradient[73] = 11.100; svpGradient[74] = 11.500; svpGradient[75] = 11.900;
	 svpGradient[76] = 12.300; svpGradient[77] = 12.700; svpGradient[78] = 13.200; svpGradient[79] = 13.700;
	 svpGradient[80] = 14.100; svpGradient[81] = 14.600; svpGradient[82] = 15.200; svpGradient[83] = 15.700;
	 svpGradient[84] = 16.200; svpGradient[85] = 16.800; svpGradient[86] = 17.300; svpGradient[87] = 17.800;
	 svpGradient[88] = 18.400; svpGradient[89] = 19.000; svpGradient[90] = 19.660; svpGradient[91] = 20.290;
	 svpGradient[92] = 20.940; svpGradient[93] = 21.610; svpGradient[94] = 22.300; svpGradient[95] = 23.000;
	 svpGradient[96] = 23.720; svpGradient[97] = 24.450; svpGradient[98] = 25.200; svpGradient[99] = 25.970;
 }

 weatherDataType getWeatherDataTypeByDataFile(string fpn_wdata) {
	 ifstream rfFile(fpn_wdata);
	 string aline;
	 getline(rfFile, aline);
	 aline = trim(aline);
	 if (aline == "") {
		 writeLog(fpnLog, "ERROR : Weather data file [" + fpn_wdata + "] is empty.\n", 1, 1);
		 return weatherDataType::None;
	 }
	 if (_access(aline.c_str(), 0) == 0) {
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
			 writeLog(fpnLog, "ERROR : Watershed IDs in [" + prj.fpnRainfallData + "] are invalid.\n", 1, 1);
			 wsids.clear();
			 break;
		 }
	 }
	 return wsids;
 }
 
 vector<weatherData> readAndSetWeatherData(string fpn_in_wd, weatherDataType wdType,
	 int dataInterval_min, string dataString) {
	 vector<weatherData> nullWD;
	 if (fpn_in_wd != "" && _access(fpn_in_wd.c_str(), 0) != 0) {
		 writeLog(fpnLog, "ERROR : "+dataString + " data file is invalid.\n", 1, 1);
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
			 fs::path fpn = fs::path(Lines[n].c_str());
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
				 writeLog(fpnLog, err, 1, 1);
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
					 writeLog(fpnLog, err, 1, 1);
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
