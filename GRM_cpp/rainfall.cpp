#include "stdafx.h"
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern cvAtt* cvs;
extern cvpos* cvps;
extern domaininfo di;
extern thisSimulation ts;

extern vector<rainfallData> rfs;
extern wpinfo wpis;

int setRainfallData()
{
    if (prj.fpnRainfallData != "" && _access(prj.fpnRainfallData.c_str(), 0) != 0) {
        writeLog(fpnLog, "Rainfall data file is invalid.\n", 1, 1);
        return -1;
    }
    rfs.clear();
    fs::path fpn_rf_in = fs::path(prj.fpnRainfallData);
    string fp_rf_in = fpn_rf_in.parent_path().string();
    string fn_rf_in = fpn_rf_in.filename().string();
    vector<string> Lines;
    Lines = readTextFileToStringVector(prj.fpnRainfallData);
for (int n = 0; n < Lines.size(); n++) {
	if (trim(Lines[n]) == "") { break; }
	rainfallData r;
	r.Order = n + 1;
	switch (prj.rfDataType)
	{
	case weatherDataType::TextFileASCgrid: {
		fs::path fpn_rf = fs::path(Lines[n].c_str());
		r.Rainfall = fpn_rf.filename().string();
		r.FileName = fpn_rf.filename().string();
		r.FilePath = fpn_rf.parent_path().string();
		break;
	}
	case weatherDataType::TextFileASCgrid_mmPhr: {
		fs::path fpn_rf = fs::path(Lines[n].c_str());
		r.Rainfall = fpn_rf.filename().string();
		r.FileName = fpn_rf.filename().string();
		r.FilePath = fpn_rf.parent_path().string();
		break;
	}
	case weatherDataType::TextFileMEAN:
		string value = Lines[n];
		if (isNumeric(value) == true) {
			r.Rainfall = value;
		}
		else {
			string err = "Error: Can not read rainfall value.\nOrder = "
				+ to_string(n + 1) + "\n";
			writeLog(fpnLog, err, 1, 1);
			r.Rainfall = "0";
			return -1;
		}
		r.FileName = fn_rf_in;
		r.FilePath = fp_rf_in;
		break;
	}
	if (prj.isDateTimeFormat == 1) {
		r.DataTime = timeElaspedToDateTimeFormat2(prj.simStartTime,
			prj.rfinterval_min * 60 * n, timeUnitToShow::toM,
			dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
	}
	else {
		r.DataTime = to_string(prj.rfinterval_min * n);
	}
	rfs.push_back(r);
}
return 1;
}

int setCVRF(int order)
{
	int dtrf_sec = prj.rfinterval_min * 60;
	int dtrf_min = prj.rfinterval_min;
	string fpnRF = "";
	double cellSize = di.cellSize;
	ts.rfiSumAllCellsInCurRFData_mPs = 0;
	int returnv = -1;
	for (int idx : wpis.wpCVidxes) {
		wpis.rfiReadSumUpWS_mPs[idx] = 0;
	}
	if (prj.rfDataType == weatherDataType::TextFileASCgrid
		|| prj.rfDataType == weatherDataType::TextFileASCgrid_mmPhr) {
		fpnRF = rfs[order - 1].FilePath + "\\" + rfs[order - 1].FileName;

		ascRasterFile rfasc = ascRasterFile(fpnRF);
		omp_set_num_threads(prj.mdp);
		map<int, double>* rfiReadSumUpWS_mPs_L = new map<int, double>[prj.mdp];
		double* rfiSumAllCellsInCurRFData_mPs_L = new double[prj.mdp];
#pragma omp parallel
		{
			int nth = omp_get_thread_num();
			for (int idx : wpis.wpCVidxes) {
				rfiReadSumUpWS_mPs_L[nth][idx] = 0;
			}
			rfiSumAllCellsInCurRFData_mPs_L[nth] = 0;
#pragma omp for
			for (int i = 0; i < di.cellNnotNull; ++i) {
				// 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
				// 강우는 상류에 내린 강우도 포함하는 것이 타당.
				// 상류 cv 개수에 이 조건 추가하려면 주석 해제.
				//if (cvs[i].toBeSimulated == -1) {
				//    continue;  }
				double inRF_mm = rfasc.valuesFromTL[cvps[i].xCol][cvps[i].yRow];
				if (prj.rfDataType == weatherDataType::TextFileASCgrid_mmPhr) {
					inRF_mm = inRF_mm / (60.0 / dtrf_min);
				}
				if (inRF_mm <= 0) {
					cvs[i].rfiRead_mPsec = 0;
				}
				else {
					cvs[i].rfiRead_mPsec = rfintensity_mPsec(inRF_mm, dtrf_sec);
				}
				for (int idx : cvs[i].downWPCVidx) {
					rfiReadSumUpWS_mPs_L[nth][idx] += cvs[i].rfiRead_mPsec;
				}
				rfiSumAllCellsInCurRFData_mPs_L[nth] += cvs[i].rfiRead_mPsec;
			}
		}

		// reduction
		for (int nth = 0; nth < prj.mdp; ++nth) {
			for (int idx : wpis.wpCVidxes) {
				wpis.rfiReadSumUpWS_mPs[idx] += rfiReadSumUpWS_mPs_L[nth][idx];
			}
			ts.rfiSumAllCellsInCurRFData_mPs += rfiSumAllCellsInCurRFData_mPs_L[nth];
		}

  //      ascRasterFile rfasc = ascRasterFile(fpnRF);
  //      //#pragma omp parallel for// schedule(guided)
		////omp_set_num_threads(ps.mdp);
  //      for (int i = 0; i < di.cellNnotNull; ++i) {
  //          // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
		//	// 강우는 상류에 내린 강우도 포함하는 것이 타당.
  //          // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
  //          //if (cvs[i].toBeSimulated == -1) {
  //          //    continue;  }
  //          double inRF_mm = rfasc.valuesFromTL[cvps[i].xCol][cvps[i].yRow];
  //          if (prj.rfDataType == rainfallDataType::TextFileASCgrid_mmPhr) {
  //              inRF_mm = inRF_mm / (60.0 / dtrf_min);
  //          }
  //          if (inRF_mm <= 0) {
  //              cvs[i].rfiRead_mPsec = 0;
  //          }
  //          else {
  //              cvs[i].rfiRead_mPsec = rfintensity_mPsec(inRF_mm, dtrf_sec);
  //          }

  //          for (int idx : cvs[i].downWPCVidx) {
  //              wpis.rfiReadSumUpWS_mPs[idx] = wpis.rfiReadSumUpWS_mPs[idx]
  //                  + cvs[i].rfiRead_mPsec;
  //          }
  //          ts.rfiSumAllCellsInCurRFData_mPs =
  //              ts.rfiSumAllCellsInCurRFData_mPs
  //              + cvs[i].rfiRead_mPsec;
  //      }
		returnv = 1;
    }
    else if (prj.rfDataType == weatherDataType::TextFileMEAN) {
		fpnRF = rfs[order - 1].FilePath + "\\" + rfs[order - 1].FileName;
        string value = rfs[order - 1].Rainfall;
        double inRF_mm = stod(value);
        double inRF_mPs;
        if (inRF_mm < 0) {
            inRF_mm = 0;
            inRF_mPs = 0;
        }
        else {
            inRF_mPs = rfintensity_mPsec(inRF_mm, dtrf_sec);
        }
#pragma omp parallel for schedule(guided)
        for (int i = 0; i < di.cellNnotNull; ++i) {
            // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
            // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
            //if (cvs[i].toBeSimulated == -1) { continue; }
            cvs[i].rfiRead_mPsec = inRF_mPs;
        }
        ts.rfiSumAllCellsInCurRFData_mPs = inRF_mPs * di.cellNtobeSimulated;
        for (int idx : wpis.wpCVidxes) {
            wpis.rfiReadSumUpWS_mPs[idx] = inRF_mPs * wpis.cvCountAllup[idx];
        }
		returnv = 1;
    }
    else {
        writeLog(fpnLog, "Error: Rainfall data type is invalid.\n", 1, 1);
        return -1;
    }
	if (returnv == -1) {
		writeLog(fpnLog, "An error was occurred while reading rainfall data.\n", 1, 1);
		writeLog(fpnLog, "Rainfall file : " + fpnRF + ".\n", 1, 1);
	}
    return returnv;
}

 void setRFintensityAndDTrf_Zero()
{
     ts.rfAveForDT_m = 0;
	 ts.rfAveSumAllCells_dtP_m = 0;
     ts.rfiSumAllCellsInCurRFData_mPs = 0;
#pragma omp parallel for
     for (int i = 0; i < di.cellNnotNull; ++i)    {
        cvs[i].rfiRead_mPsec = 0;
        cvs[i].rfApp_dt_m = 0;
    }

    for(int wpcvid : wpis.wpCVidxes)    {
        wpis.rfWPGridForDtP_mm[wpcvid] = 0;
        wpis.rfUpWSAveForDt_mm[wpcvid] = 0;
        wpis.rfiReadSumUpWS_mPs[wpcvid] = 0;
        wpis.rfUpWSAveForDtP_mm[wpcvid] = 0;
    }
}

 void calWPCumulRFduringDTP(int dtsec)
 {
     ts.rfAveForDT_m = ts.rfiSumAllCellsInCurRFData_mPs * dtsec
         / di.cellNtobeSimulated;
     ts.rfAveSumAllCells_dtP_m = ts.rfAveSumAllCells_dtP_m
         + ts.rfAveForDT_m;
     for (int idx : wpis.wpCVidxes) {
		 wpis.rfUpWSAveForDt_mm[idx] = wpis.rfiReadSumUpWS_mPs[idx]
			 * dtsec * 1000 / (double)wpis.cvCountAllup[idx]; //(double)(cvs[idx].fac + 1);
         wpis.rfUpWSAveForDtP_mm[idx] = wpis.rfUpWSAveForDtP_mm[idx]
             + wpis.rfUpWSAveForDt_mm[idx];
         wpis.rfWPGridForDtP_mm[idx] = wpis.rfWPGridForDtP_mm[idx]
             + cvs[idx].rfiRead_mPsec * 1000 * dtsec;
     }
 }
     
 inline double rfintensity_mPsec(double rf_mm, double dtrf_sec)
 {
     return rf_mm / 1000.0 / dtrf_sec;
 }

 void initRFvars()
 {
	 ts.rfAveForDT_m = 0;
	 ts.rfAveSumAllCells_dtP_m = 0;
	 ts.rfiSumAllCellsInCurRFData_mPs = 0;
	 for (int wpcvid : wpis.wpCVidxes) {
		 wpis.rfWPGridForDtP_mm[wpcvid] = 0;
		 wpis.rfUpWSAveForDt_mm[wpcvid] = 0;
		 wpis.rfiReadSumUpWS_mPs[wpcvid] = 0;
		 wpis.rfUpWSAveForDtP_mm[wpcvid] = 0;
	 }
 }



