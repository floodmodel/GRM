
#include <io.h>
#include "grm.h"
using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern int** cvais;
extern flowControlCellAndData fccds;

int initFCCellinfoAndData()
{
    fccds.cvidsinlet.clear();
    fccds.cvidsFCcell.clear();
    fccds.fcDataAppliedNowT.clear();
    fccds.flowData.clear();
    for (flowControlinfo afc : prj.fcs) {
        int aid = cvais[afc.fcColX][afc.fcRowY];
        fccds.cvidsFCcell.push_back(aid);
        if (afc.fcType == flowControlType::Inlet) {
            fccds.cvidsinlet.push_back(aid);
        }
        if (afc.fcType != flowControlType::ReservoirOperation) {
            if (afc.fpnFCData != "" && _access(afc.fpnFCData.c_str(), 0) != 0) {
                string outstr = "Flow control data file (" + afc.fpnFCData + ") is invalid.\n";
                writeLog(fpnLog, outstr, 1, 1);
                return -1;
            }
            vector<double> vs;
            vs = readTextFileToDoubleVector(afc.fpnFCData);
            for (int i = 0; i < vs.size(); ++i) {
                timeSeries ts;
                if (prj.isDateTimeFormat == 1) {
                    ts.dataTime = timeElaspedToDateTimeFormat(prj.simulStartingTime, 
                        afc.fcDT_min * 60 * i,-1,dateTimeFormat::yyyy_mm_dd_HHcolMMcolSS);
                }
                else {
                    ts.dataTime = afc.fcDT_min * i;
                }
                ts.value = vs[i];
                fccds.flowData[aid].push_back(ts);
            }
        }
    }
    if (fccds.cvidsinlet.size() > 0) {
        prj.isinletApplied = 1;
    }
    else { prj.isinletApplied = -1; }
    return 1;
}