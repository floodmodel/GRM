
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
    fccds.cvidsinlet.clear();
    fccds.cvidsFCcell.clear();
    fccds.fcDataAppliedNowT_m3Ps.clear();
    fccds.flowData_m3Ps.clear();
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
                fccds.flowData_m3Ps[acvid].push_back(ts);
            }
        }
    }
    if (fccds.cvidsinlet.size() > 0) {
        prj.isinletExist = 1;
    }
    else { prj.isinletExist = -1; }
    return 1;
}

void calFCReservoirOutFlow(int i, double nowTmin)
{// nowTmin�� �ּҰ��� dtsec/60�̴�.
    int id = i + 1;//cvid=arrayindex+1;
    int dtfc = prj.fcs[id].fcDT_min;
    if (nowTmin > dtfc* fccds.curDorder[id]) {
        if (fccds.curDorder[id] < fccds.flowData_m3Ps[id].size()) {
            fccds.curDorder[id]++;
        }
        else {
            fccds.curDorder[id] = INT_MAX;
            setNoFluxCVCH(i);
            fccds.fcDataAppliedNowT_m3Ps[id] = 0;
            return;
        }
    }
    else {
        // //�� �������� ����ϸ�, ������ �ڷᰡ ������ ���ȴ�..
        // fccds.curDorder[id]= fccds.flowData[id].size() - 1; }
        // �Ʒ� �������� ����ϸ�, �Էµ� �ڷ� ��ŭ�� �ݿ��ǰ�, �� ���Ĵ� 0
        setNoFluxCVCH(i);
        fccds.fcDataAppliedNowT_m3Ps[id] = 0;
        return;
    }
    int idx = fccds.curDorder[id] - 1;//vector index, �� �������� order�� �ּҰ��� 1
    double v = fccds.flowData_m3Ps[id][idx].value;
    if (v < 0) { v = 0; }
    cvs[i].stream.QCH_m3Ps = v;
    cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i], cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
    cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
        cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
        cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
    cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
    fccds.fcDataAppliedNowT_m3Ps[id] = cvs[i].stream.QCH_m3Ps;
    //�Ʒ�����  cvs[i].QsumCVw_dt_m3�� t-dt ������ ���� ����Ǿ� �ִ�.
    // �׸���, fcDataAppliedNowT_m3Ps �� ���� ������ �Ϸ��� ����Ǵ� ���̹Ƿ�, ���ش�.
    cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
        + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
        - fccds.fcDataAppliedNowT_m3Ps[id] * ts.dtsec;
}


void calSinkOrSourceFlow(int i, int nowTmin)
{// nowTmin�� �ּҰ��� dtsec/60�̴�.
    int id = i + 1;//cvid=arrayindex+1;
    int dtfc = prj.fcs[id].fcDT_min;
    double cellsize = di.cellSize;
    if (nowTmin > dtfc* fccds.curDorder[id]) {
        if (fccds.curDorder[id] < fccds.flowData_m3Ps[id].size()) {
            fccds.curDorder[id]++;
        }
        else {
            fccds.curDorder[id] = INT_MAX;
            fccds.fcDataAppliedNowT_m3Ps[id] = 0.0;
            return; //sink, source flow �������� �ʴ´�
        }
    }
    else {
        // //�� �������� ����ϸ�, ������ �ڷᰡ ������ ���ȴ�..
        // fccds.curDorder[id]= fccds.flowData[id].size() - 1; }
        // �Ʒ� �������� ����ϸ�, �Էµ� �ڷ� ��ŭ�� �ݿ��ǰ�, �� ���Ĵ� sin, source �� 0
        fccds.fcDataAppliedNowT_m3Ps[id] = 0.0;
        return; //sink, source flow �������� �ʴ´�
    }
    int idx = fccds.curDorder[id] - 1;//vector index, �� �������� order�� �ּҰ��� 1
    double v = fccds.flowData_m3Ps[id][idx].value;
    double QtoApp = fccds.flowData_m3Ps[id][idx].value;
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
    fccds.fcDataAppliedNowT_m3Ps[id] = QtoApp;
}

void calReservoirOperation(int i, int nowTmin)
{
    int dtsec = ts.dtsec;
    int id = i + 1;
    double QforDTbySinkOrSourceFlow = 0;
    double cellsize = di.cellSize;
    //���⼭�� ro �ϱ� ���� storage ���
    if (cvs[i].fcType == flowControlType::SinkFlow) {//������ sinkflow �� ���Ǿ�����..
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
            - fccds.fcDataAppliedNowT_m3Ps[id] * dtsec;
    }
    else if (cvs[i].fcType == flowControlType::SourceFlow) {//������ source flow �� ���Ǿ�����..
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
            + fccds.fcDataAppliedNowT_m3Ps[id] * dtsec;
    }
    else {//������ sinkflow or source flow �� �ƴϾ�����, �׳� ����� ���, Ⱦ���⸸ �߰��Ѵ�.
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3
            + cvs[i].QsumCVw_dt_m3;
    }
    double Qout_cms = 0.0;
    double dy_m = di.cellSize;
    double maxStorageApp = prj.fcs[id].maxStorage_m3 * prj.fcs[id].maxStorageR;
    switch (prj.fcs[id].roType) {
    case reservoirOperationType::AutoROM: {
        if (cvs[i].storageCumulative_m3 >= maxStorageApp) {
            Qout_cms = (cvs[i].storageCumulative_m3
                - maxStorageApp) / ts.dtsec; // ���̸�ŭ ��� ����
            cvs[i].storageCumulative_m3 = maxStorageApp; // �ִ������� ����
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
                - maxStorageApp) / ts.dtsec; // ���̸�ŭ ��� ����
            cvs[i].storageCumulative_m3 = maxStorageApp;
        }
        else  if (cvs[i].storageCumulative_m3 < prj.fcs[id].roConstQ_cms * ts.dtsec) {
            // ���� �������� dtsec ���ⷮ ���� ���� ���, ������ ��� ���� ����
            Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
            cvs[i].storageCumulative_m3 = 0;
        }
        else {
            Qout_cms = prj.fcs[id].roConstQ_cms;
            cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                - prj.fcs[id].roConstQ_cms;
            if (cvs[i].storageCumulative_m3 < 0) {
                cvs[i].storageCumulative_m3 = 0;
            }
        }
        if (Qout_cms < 0) { Qout_cms = 0; }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::ConstantQ: {
        //calReservoirConstantQ(i, prj.fcs[id].roConstQ_cms,
        //    prj.fcs[id].maxStorage_m3 * prj.fcs[id].maxStorageR,
        //    inOutflowDuration);
        if (nowTmin <= prj.fcs[id].roConstQDuration_hr * 60) {
            //���⼭�� �ִ� ���� ���ɷ��� ��� ���� ������ ���            
            if (cvs[i].storageCumulative_m3 <= prj.fcs[id].roConstQ_cms * ts.dtsec) {
                // �̰��� �������� ��� ����
                Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
                cvs[i].storageCumulative_m3 = 0;
            }
            else {
                Qout_cms = prj.fcs[id].roConstQ_cms * ts.dtsec;
                cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                    - prj.fcs[id].roConstQ_cms * ts.dtsec;
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