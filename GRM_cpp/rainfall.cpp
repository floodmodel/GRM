#include <filesystem>
#include <string>
#include <io.h>
#include <omp.h>

#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern cvAtt* cvs;
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
        case rainfallDataType::TextFileASCgrid: {
            fs::path fpn_rf = fs::path(Lines[n].c_str());
            r.Rainfall = fpn_rf.filename().string();
            r.FileName = fpn_rf.filename().string();
            r.FilePath = fpn_rf.parent_path().string();
            break;
        }
        case rainfallDataType::TextFileASCgrid_mmPhr: {
            fs::path fpn_rf = fs::path(Lines[n].c_str());
            r.Rainfall = fpn_rf.filename().string();
            r.FileName = fpn_rf.filename().string();
            r.FilePath = fpn_rf.parent_path().string();
            break;
        }
        case rainfallDataType::TextFileMAP:
            string value = Lines[n];
            if (isNumeric(value) == true) {
                r.Rainfall = value;
            }
            else {
                string err= "Error: Can not read rainfall value.\nOrder = "
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
            r.DataTime = timeElaspedToDateTimeFormat(prj.simStartTime, 
                prj.rfinterval_min * 60 * n, false, dateTimeFormat::yyyy_mm_dd_HHcolMMcolSS);
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
    double dtrf_sec = prj.rfinterval_min * 60.0;
    double dtrf_min = prj.rfinterval_min;
    string fpnRF = rfs[order - 1].FilePath + "\\" + rfs[order - 1].FileName;
    double cellSize = di.cellSize;
    ts.rfiSumForAllCellsInCurrentRFData_mPs = 0;
    for (int wpCVID : wpis.wpCVIDs) {
        wpis.rfiReadSumUpWS_mPs[wpCVID] = 0;
    }
    if (prj.rfDataType == rainfallDataType::TextFileASCgrid
        || prj.rfDataType == rainfallDataType::TextFileASCgrid_mmPhr) {
        ascRasterFile rfasc = ascRasterFile(fpnRF);
        //        omp_set_num_threads(prj.maxDegreeOfParallelism);
        //#pragma omp parallel for schedule(guided)
        for (int i = 0; i < di.cellNnotNull; ++i) {
            // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
            // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
            //if (cvs[i].toBeSimulated == -1) {
            //    continue;
            //}
            double inRF_mm = rfasc.valuesFromTL[cvs[i].idx_xr][cvs[i].idx_yc];
            if (prj.rfDataType == rainfallDataType::TextFileASCgrid_mmPhr) {
                inRF_mm = inRF_mm / (60.0 / dtrf_min);
            }
            cvs[i].rfiRead_mPsec = rfintensity_mPsec(inRF_mm, dtrf_sec);

            for (int wpcvid : cvs[i].downWPCVIDs) {
                wpis.rfiReadSumUpWS_mPs[wpcvid] = wpis.rfiReadSumUpWS_mPs[wpcvid]
                    + cvs[i].rfiRead_mPsec;
            }
            ts.rfiSumForAllCellsInCurrentRFData_mPs =
                ts.rfiSumForAllCellsInCurrentRFData_mPs
                + cvs[i].rfiRead_mPsec;
        }
    }
    else if (prj.rfDataType == rainfallDataType::TextFileMAP) {
        string value = rfs[order - 1].Rainfall;
        double inRF_mm = stod(value);
        if (inRF_mm < 0) { inRF_mm = 0; }
        double inRF_mPs = rfintensity_mPsec(inRF_mm, dtrf_sec);
        for (int i = 0; i < di.cellNnotNull; ++i) {
            // 유역의 전체 강우량은 inlet 등으로 toBeSimulated == -1 여도 계산에 포함한다.
            // 상류 cv 개수에 이 조건 추가하려면 주석 해제.
            //if (cvs[i].toBeSimulated == -1) { continue; }
            cvs[i].rfiRead_mPsec = inRF_mPs;
        }        
        ts.rfiSumForAllCellsInCurrentRFData_mPs = inRF_mPs * di.cellNtobeSimulated;
        for (int wpcvid : wpis.wpCVIDs) {
            wpis.rfiReadSumUpWS_mPs[wpcvid] = inRF_mm * wpis.cvCountAllup[wpcvid];
        }
    }
    else {
        writeLog(fpnLog, "Error: Rainfall data type is invalid.\n", 1, 1);
        return -1;
    }
    return 1;
}

 void setRFintensityAndDTrf_Zero()
{
     ts.rfAveForDT_m = 0;
     ts.rfiSumForAllCellsInCurrentRFData_mPs = 0;
     for (int i = 0; i < di.cellNnotNull; ++i)    {
        cvs[i].rfiRead_mPsec = 0;
        cvs[i].rfApp_dt_m = 0;
    }
    for(int wpcvid : wpis.wpCVIDs)    {
        wpis.rfWPGridForDtPrint_mm[wpcvid] = 0;
        wpis.rfUpWSAveForDt_mm[wpcvid] = 0;
        wpis.rfiReadSumUpWS_mPs[wpcvid] = 0;
        wpis.rfUpWSAveForDtPrint_mm[wpcvid] = 0;
    }
}

 void calCumulativeRFDuringDTPrintOut(int dtsec)
 {
     ts.rfAveForDT_m = ts.rfiSumForAllCellsInCurrentRFData_mPs * dtsec
         / di.cellNtobeSimulated;
     ts.rfAveForAllCell_sumDTprint_m = ts.rfAveForAllCell_sumDTprint_m
         + ts.rfAveForDT_m;
     for (int wpcvid : wpis.wpCVIDs) {
         wpis.rfUpWSAveForDt_mm[wpcvid] = wpis.rfiReadSumUpWS_mPs[wpcvid]
             * dtsec * 1000 / (double)(cvs[wpcvid - 1].fac + 1);
         wpis.rfUpWSAveForDtPrint_mm[wpcvid] = wpis.rfUpWSAveForDtPrint_mm[wpcvid]
             + wpis.rfUpWSAveForDt_mm[wpcvid];
         wpis.rfWPGridForDtPrint_mm[wpcvid] = wpis.rfWPGridForDtPrint_mm[wpcvid]
             + cvs[wpcvid - 1].rfiRead_mPsec * 1000 * dtsec;
     }
     if (prj.makeASCFile == 1 || prj.makeIMGFile == 1) {
         if (prj.makeRfDistFile == 1 || prj.makeRFaccDistFile == 1) {
             for (int i = 0; i < di.cellNtobeSimulated; ++i) {
                 cvs[i].rf_dtPrint_m = cvs[i].rf_dtPrint_m
                     + cvs[i].rfiRead_mPsec * dtsec;
                 cvs[i].rfAcc_fromStart_m = cvs[i].rfAcc_fromStart_m
                     + cvs[i].rfiRead_mPsec * dtsec;
             }
         }
     }
 }
     


