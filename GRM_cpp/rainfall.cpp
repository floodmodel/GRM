#include <filesystem>
#include <string>
#include <io.h>

#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern vector<rainfallData> rfs;

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
            r.DataTime = timeElaspedToString_yyyymmddHHMM(prj.simulStartingTime, prj.rfinterval_min * 60 * n);
        }
        else {
            r.DataTime = to_string(prj.rfinterval_min * n);
        }
        rfs.push_back(r);
    }
    return 1;
}