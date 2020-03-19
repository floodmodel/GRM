
#include <io.h>
#include "grm.h"
using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern int** cvais;
extern cvAtt* cvs;
extern flowControlCellAndData fccds;
extern thisSimulation ts;

int updateFCCellinfoAndData()
{
    fccds.cvidsinlet.clear();
    fccds.cvidsFCcell.clear();
    fccds.fcDataAppliedNowT.clear();
    fccds.flowData.clear();
    fccds.curDorder.clear();
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
        fccds.curDorder[acvid] = 0;
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

void calFCReservoirOutFlow(double nowTmin, int i)
{
    int id = i + 1;//cvid=arrayindex+1;
    int dtfc = prj.fcs[id].fcDT_min;
    if (nowTmin > dtfc* fccds.curDorder[id]) {
        if (fccds.curDorder[id] < fccds.flowData[id].size()) {
            fccds.curDorder[id]++;
        }
        else {
            fccds.curDorder[id] = INT_MAX;
        }
    }
    else {
        //이 조건 넣으면, 마지막 자료가 끝까지 사용된다..
        // fccds.curDorder[id]= fccds.flowData[id].size() - 1; }
        return;
    }
    int fcOrder = fccds.curDorder[id];
    double v = fccds.flowData[id][fcOrder].value;
    if (v < 0) { v = 0; }
    cvs[i].stream.QCH_m3Ps = v;
    cvs[i].stream.csaCH = getChCSAbyQusingIteration(cvs[i], cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
    cvs[i].stream.hCH = getChannelDepthUsingArea(cvs[i].stream.chBaseWidth,
        cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
        cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
    cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
    fccds.fcDataAppliedNowT[id] = cvs[i].stream.QCH_m3Ps;
}