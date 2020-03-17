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
extern thisProcess tp;

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
            r.Rainfall = Lines[n];
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
    tp.rfintensitySumForAllCellsInCurrentRFData_mPs = 0;
    for (int wpCVID : wpis.wpCVIDs) {
        wpis.rfReadIntensitySumUpWS_mPs[wpCVID] = 0;
    }
    if (prj.rfDataType == rainfallDataType::TextFileASCgrid
        || prj.rfDataType == rainfallDataType::TextFileASCgrid_mmPhr) {
        ascRasterFile rfasc = ascRasterFile(fpnRF);
        omp_set_num_threads(prj.maxDegreeOfParallelism);
#pragma omp parallel for schedule(guided)
        for (int i = 0; i < di.cellCountNotNull; ++i) {
            if (cvs[i].toBeSimulated == -1) {
                continue;
            }
            double inRF_mm = rfasc.valuesFromTL[cvs[i].idx_xr][cvs[i].idx_yc];
            if (prj.rfDataType == rainfallDataType::TextFileASCgrid_mmPhr) {
                inRF_mm = inRF_mm / (60.0 / dtrf_min);
            }
            cvs[i].rfintensityRead_mPsec = rfintensity_mPsec(inRF_mm, dtrf_sec);
        }
        for (int i = 0; i < di.cellCountNotNull; ++i) {
            if (project.WSCells[cx, ry] != null && project.WSCells[cx, ry].toBeSimulated == 1)
            {
                sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs =
                    sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.WSCells[cx, ry].RFReadintensity_mPsec;
                int cvan = project.WSCells[cx, ry].CVID - 1;
                CalRFSumForWPUpWSWithRFGrid(cvan);
            }
        }
    }
    else if (eRainfallDataType == cRainfall.RainfallDataType.TextFileMAP)
    {
        double inRF_mm;
        double v = 0;
        if (double.TryParse(rfRow.Rainfall, out v) == true)
        {
            inRF_mm = v;
        }
        else
        {
            System.Console.WriteLine("Error: Can not read rainfall value!!" + "\r\n" + "Order = " + nowRFOrder.ToString());
            return;
        }
        if (inRF_mm < 0) { inRF_mm = 0; }
        for (int cvan = 0; cvan < project.CVCount; cvan++)
        {
            CalRFintensity_mPsec(project.CVs[cvan], inRF_mm, rfIntervalSEC);
            sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.CVs[cvan].RFReadintensity_mPsec;
        }
        CalRFSumForWPUpWSWithMAPValue(project.CVs[0].RFReadintensity_mPsec); // 모든 격자의 강우량 동일하므로.. 하나를 던저준다.
    }
    else
    {
        System.Console.WriteLine("Error: Rainfall data type is invalid.");
    }
}


double rfintensity_mPsec(double rf_mm, double dtrf_sec)
{
    if (rf_mm <= 0) {
       return 0;
    }
    else {
        return rf_mm / 1000.0 / dtrf_sec;
    }
}