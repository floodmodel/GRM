
#include <io.h>
#include "grm.h"
using namespace std;

extern fs::path fpnLog;
extern projectFile prj;
extern int** cvais;
extern cvAtt* cvs;
extern domaininfo di;

extern flowControlCellAndData fccds;
extern thisSimulation ts;

int updateFCCellinfoAndData()
{
    fccds.cvidxsinlet.clear();
    fccds.cvidxsFCcell.clear();
    fccds.fcDataAppliedNowT_m3Ps.clear();
    fccds.flowData_m3Ps.clear();
    fccds.curDorder.clear();
    map<int, flowControlinfo>::iterator iter;
    map<int, flowControlinfo> fcs_tmp;
    fcs_tmp = prj.fcs;
    prj.fcs.clear();
    for (iter = fcs_tmp.begin(); iter != fcs_tmp.end(); ++iter) {
        flowControlinfo afc = iter->second;
        int aidx = cvais[afc.fcColX][afc.fcRowY];
        prj.fcs[aidx] = afc;
        fccds.cvidxsFCcell.push_back(aidx);
        fccds.curDorder[aidx] = 0;
        if (afc.fcType == flowControlType::Inlet) {
            fccds.cvidxsinlet.push_back(aidx);
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
                    ts.dataTime = timeElaspedToDateTimeFormat2(prj.simStartTime,
                        afc.fcDT_min * 60 * i,  timeUnitToShow::toMinute
                        , dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
                }
                else {
                    ts.dataTime = afc.fcDT_min * i;
                }
                ts.value = vs[i];
                fccds.flowData_m3Ps[aidx].push_back(ts);
            }
        }
    }
    if (fccds.cvidxsinlet.size() > 0) {
        prj.isinletExist = 1;
    }
    else { prj.isinletExist = -1; }
    return 1;
}

void calFCReservoirOutFlow(int i, double nowTmin)
{// nowTmin의 최소값은 dtsec/60이다.
    int dtfc = prj.fcs[i].fcDT_min;
    if (nowTmin > dtfc* fccds.curDorder[i]) {
        if (fccds.curDorder[i] < fccds.flowData_m3Ps[i].size()) {
            fccds.curDorder[i]++;
        }
        else {
            // 아래 조건 주석처리하면, 마지막 자료가 끝까지 사용된다..
            setNoFluxCVCH(i);
            fccds.fcDataAppliedNowT_m3Ps[i] = 0;
            return;
        }
    }
    int orderidx = fccds.curDorder[i] -1 ;//vector index, 이 지점에서  fccds.curDorder[i]의 최소값은 1
    double v = fccds.flowData_m3Ps[i][orderidx].value;
    if (v < 0) { v = 0; }
    cvs[i].stream.QCH_m3Ps = v;
    cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i], cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
    cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
        cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
        cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
    cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
    fccds.fcDataAppliedNowT_m3Ps[i] = cvs[i].stream.QCH_m3Ps;
    //아래에서  cvs[i].QsumCVw_dt_m3는 t-dt 에서의 값이 저장되어 있다.
    // 그리고, fcDataAppliedNowT_m3Ps 는 현재 셀에서 하류로 방류되는 값이므로, 빼준다.
    cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
        + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
        - fccds.fcDataAppliedNowT_m3Ps[i] * ts.dtsec;
    if (cvs[i].storageCumulative_m3, 0) {
        cvs[i].storageCumulative_m3 = 0;
    }
}


void calSinkOrSourceFlow(int i, double nowTmin)
{// nowTmin의 최소값은 dtsec/60이다.
    int dtfc = prj.fcs[i].fcDT_min;
    double cellsize = di.cellSize;
    if (nowTmin > dtfc* fccds.curDorder[i]) {
        if (fccds.curDorder[i] < fccds.flowData_m3Ps[i].size()) {
            fccds.curDorder[i]++;
        }
        else {
            // 아래 조건 주석처리하면, 마지막 자료가 끝까지 사용된다..
            setNoFluxCVCH(i);
            fccds.fcDataAppliedNowT_m3Ps[i] = 0.0;
            return; //sink, source flow 모의하지 않는다
        }
    }
    int orderidx = fccds.curDorder[i]-1 ;//vector index, 이 지점에서 fccds.curDorder[i] 의 최소값은 1
    double v = fccds.flowData_m3Ps[i][orderidx].value;
    double QtoApp = fccds.flowData_m3Ps[i][orderidx].value;
    if (cvs[i].flowType == cellFlowType::OverlandFlow) {
        switch (cvs[i].fcType) {
        case flowControlType::SinkFlow: {
            cvs[i].QOF_m3Ps = cvs[i].QOF_m3Ps - QtoApp;
            break;
        }
        case flowControlType::SourceFlow: {
            cvs[i].QOF_m3Ps = cvs[i].QOF_m3Ps + QtoApp;
            break;
        }
        }
        if (cvs[i].QOF_m3Ps < 0) {
            cvs[i].QOF_m3Ps = 0;
        }
        cvs[i].hOF = pow(cvs[i].rcOF * cvs[i].QOF_m3Ps 
            / cellsize / pow(cvs[i].slopeOF, 0.5), 0.6);
        cvs[i].csaOF = cvs[i].hOF * cellsize;
        cvs[i].stream.uCH = cvs[i].QOF_m3Ps / cvs[i].csaOF;
    }
    else {
        switch (cvs[i].fcType) {
        case flowControlType::SinkFlow: {
            cvs[i].stream.QCH_m3Ps = cvs[i].stream.QCH_m3Ps - QtoApp;
            break;
        }
        case flowControlType::SourceFlow: {
            cvs[i].stream.QCH_m3Ps = cvs[i].stream.QCH_m3Ps + QtoApp;
            break;
        }
        }
        if (cvs[i].stream.QCH_m3Ps < 0) {
            cvs[i].stream.QCH_m3Ps = 0;
        }
        cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i], 
            cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
        cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
            cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS,
            cvs[i].stream.chURBaseWidth_m, cvs[i].stream.chLRArea_m2,
            cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
        cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
    }
    fccds.fcDataAppliedNowT_m3Ps[i] = QtoApp;
}

void calReservoirOperation(int i, double nowTmin)
{
    int dtsec = ts.dtsec;
    //int i = i + 1;
    double QforDTbySinkOrSourceFlow = 0;
    double cellsize = di.cellSize;
    //여기서는 ro 하기 전의 storage 계산
    if (cvs[i].fcType == flowControlType::SinkFlow) {//이전에 sinkflow 가 계산되었으면..
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
            - fccds.fcDataAppliedNowT_m3Ps[i] * dtsec;
    }
    else if (cvs[i].fcType == flowControlType::SourceFlow) {//이전에 source flow 가 계산되었으면..
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
            + fccds.fcDataAppliedNowT_m3Ps[i] * dtsec;
    }
    else {//이전에 sinkflow or source flow 가 아니었으면, 그냥 강우와 상류, 횡방향만 추가한다.
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3
            + cvs[i].QsumCVw_dt_m3;
    }
    double Qout_cms = 0.0;
    double dy_m = di.cellSize;
    double maxStorageApp = prj.fcs[i].maxStorage_m3 * prj.fcs[i].maxStorageR;
    switch (prj.fcs[i].roType) {
    case reservoirOperationType::AutoROM: {
        if (cvs[i].storageCumulative_m3 >= maxStorageApp) {
            Qout_cms = (cvs[i].storageCumulative_m3
                - maxStorageApp) / ts.dtsec; // 차이만큼 모두 유출
            cvs[i].storageCumulative_m3 = maxStorageApp; // 최대저류량 유지
        }
        if (Qout_cms < 0) { Qout_cms = 0; }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::RigidROM: {
        //calReservoirRigidROM(i, prj.fcs[id].maxStorage_m3 * prj.fcs[id].maxStorageR, 
        //    prj.fcs[id].roConstQ_cms);
        if (cvs[i].storageCumulative_m3 >= maxStorageApp) {
            Qout_cms = (cvs[i].storageCumulative_m3
                - maxStorageApp) / ts.dtsec; // 차이만큼 모두 유출
            cvs[i].storageCumulative_m3 = maxStorageApp;
        }
        else  if (cvs[i].storageCumulative_m3 < prj.fcs[i].roConstQ_cms * ts.dtsec) {
            // 현재 저류량이 dtsec 유출량 보다 작은 경우, 저류된 모든 양이 유출
            Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
            cvs[i].storageCumulative_m3 = 0;
        }
        else {
            Qout_cms = prj.fcs[i].roConstQ_cms;
            cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                - prj.fcs[i].roConstQ_cms;
            if (cvs[i].storageCumulative_m3 < 0) {
                cvs[i].storageCumulative_m3 = 0;
            }
        }
        if (Qout_cms < 0) { Qout_cms = 0; }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::ConstantQ: {
        if (nowTmin <= prj.fcs[i].roConstQDuration_hr * 60) {
            //여기서는 최대 저류 가능량에 상관 없이 일정량 방류            
            if (cvs[i].storageCumulative_m3 <= prj.fcs[i].roConstQ_cms * ts.dtsec) {
                // 이경우는 저류량이 모두 유출
                Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
                cvs[i].storageCumulative_m3 = 0;
            }
            else {
                Qout_cms = prj.fcs[i].roConstQ_cms * ts.dtsec;
                cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                    - prj.fcs[i].roConstQ_cms * ts.dtsec;
                if (cvs[i].storageCumulative_m3 < 0) {
                    cvs[i].storageCumulative_m3 = 0;
                }
            }
        }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::SDEqation: {
        break;
    }
    }
}

void calReservoirOutFlowInReservoirOperation(int i,
    double Qout_cms, double dy_m)
{
    if (Qout_cms > 0) {
        if (cvs[i].flowType == cellFlowType::OverlandFlow) {
            cvs[i].QOF_m3Ps = Qout_cms;
            cvs[i].hOF = cvs[i].rcOF * cvs[i].QOF_m3Ps / dy_m
                / pow(sqrt(cvs[i].slopeOF), 0.6);
            cvs[i].stream.QCH_m3Ps = 0;
            cvs[i].stream.uCH = 0;
            cvs[i].stream.csaCH = 0;
            cvs[i].stream.hCH = 0;
            cvs[i].stream.uCH = 0;
        }
        else {
            cvs[i].QOF_m3Ps = 0;
            cvs[i].hOF = 0;
            cvs[i].stream.QCH_m3Ps = Qout_cms;
            cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i],
                cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
            cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
            cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
                cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS,
                cvs[i].stream.chURBaseWidth_m, cvs[i].stream.chLRArea_m2,
                cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
        }
    }
    else {
        cvs[i].hOF = 0;
        cvs[i].csaOF = 0;
        cvs[i].QOF_m3Ps = 0;
        cvs[i].stream.uCH = 0;
        cvs[i].stream.csaCH = 0;
        cvs[i].stream.hCH = 0;
        cvs[i].stream.uCH = 0;
        cvs[i].stream.QCH_m3Ps = 0;
    }
}


int getCVidxByFcName(string fcName)
{
    for (int idx : fccds.cvidxsFCcell) {
        if (fcName == prj.fcs[idx].fcName) {
            return idx;
        }
    }
    return -1;
}