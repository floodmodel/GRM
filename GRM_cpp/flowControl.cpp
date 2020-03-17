
#include <io.h>
#include "grm.h"
using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern int** cvais;
extern flowControlCellAndData fccds;

int updateFCCellinfoAndData()
{
    fccds.cvidsinlet.clear();
    fccds.cvidsFCcell.clear();
    fccds.fcDataAppliedNowT.clear();
    fccds.flowData.clear();
    map<int, flowControlinfo>::iterator iter;
    map<int, flowControlinfo> fcs_tmp;
    fcs_tmp = prj.fcs;
    prj.fcs.clear();
    for (iter = fcs_tmp.begin(); iter != fcs_tmp.end(); ++iter) {
    //for (flowControlinfo afc : prj.fcs) {
        flowControlinfo afc = iter->second;
        int aidx = cvais[afc.fcColX][afc.fcRowY];
        int acvid = aidx + 1;
        prj.fcs[acvid] = afc;
        fccds.cvidsFCcell.push_back(acvid);
        if (afc.fcType == flowControlType::Inlet) {
            fccds.cvidsinlet.push_back(acvid);
        }
        if (afc.fcType != flowControlType::ReservoirOperation) {
            if (afc.fpnFCData != "" && _access(afc.fpnFCData.c_str(), 0) != 0) {
                string outstr = "Flow control data file (" + afc.fpnFCData 
                    + ") is invalid.\n";
                writeLog(fpnLog, outstr, 1, 1);
                return -1;
            }
            vector<double> vs;
            vs = readTextFileToDoubleVector(afc.fpnFCData);
            for (int i = 0; i < vs.size(); ++i) {
                timeSeries ts;
                if (prj.isDateTimeFormat == 1) {
                    ts.dataTime = timeElaspedToDateTimeFormat(prj.simStartTime,
                        afc.fcDT_min * 60 * i, -1
                        , dateTimeFormat::yyyy_mm_dd_HHcolMMcolSS);
                }
                else {
                    ts.dataTime = afc.fcDT_min * i;
                }
                ts.value = vs[i];
                fccds.flowData[acvid].push_back(ts);
            }
        }
    }
    if (fccds.cvidsinlet.size() > 0) {
        prj.isinletExist = 1;
    }
    else { prj.isinletExist = -1; }
    return 1;
}

//flowControlinfo getFCinfoByCVID()
//{
//
//}