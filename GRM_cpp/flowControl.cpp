
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
    switch (cvs[i].fcType) {//������ sinkflow or source flow �� ���Ǿ�����..
    case flowControlType::SinkFlow: {
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
            - fccds.fcDataAppliedNowT_m3Ps[id] * dtsec;
        break;
    }
    case flowControlType::SourceFlow: {
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
            + fccds.fcDataAppliedNowT_m3Ps[id] * dtsec;
        break;
    }
    }
    switch (prj.fcs[id].roType) {
    case reservoirOperationType::AutoROM: {
        calReservoirAutoROM(i, prj.fcs[id].maxStorage_m3 * prj.fcs[id].maxStorageR);
        break;
    }
    case reservoirOperationType::RigidROM: {
        calReservoirRigidROM(i, prj.fcs[id].maxStorage_m3 * prj.fcs[id].maxStorageR, 
            prj.fcs[id].roConstQ_cms);
        break;
    }
    case reservoirOperationType::ConstantQ: {
        int inOutflowDuration = false;
        if (nowTmin <= prj.fcs[id].roConstQDuration_hr * 60) {
            inOutflowDuration = true;
        }
        calReservoirConstantQ(i, prj.fcs[id].roConstQ_cms,
            prj.fcs[id].maxStorage_m3 * prj.fcs[id].maxStorageR,
            inOutflowDuration);
        break;
    }
    case reservoirOperationType::SDEqation: {
        break;
    }
    }
}

void calReservoirConstantQ(int i, double roQ_CONST_CMS,
    double maxStorageApp, int bOutflowDuration)
{
    double dy_m = di.cellSize;

    int id = i + 1;
    double Qout_cms;
    double Qinput_m3 = cvs[i].storageAddedForDTbyRF_m3
        + cvs[i].QsumCVw_dt_m3;
    cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3 - prj.fcs[id].roConstQ_cms * ts.dtsec;
    if (bOutflowDuration == true)    {
        if (cvs[i].storageCumulative_m3 <= 0)        {
            // �̰��� ���� �������� ���� ���
            // �־��� ���ⷮ���� �������� ���ϰ�, �װͺ��� ���� ������ ����ȴٴ� �ǹ�
            if (Qinput_m3 < roQ_CONST_CMS * ts.dtsec) {
                // dt �ð����� �����Ǵ� ��� ���� ����Ǵ� ������ �Ѵ�.
                Qout_cms = Qinput_m3 / ts.dtsec;
                cvs[i].storageCumulative_m3 = 0;
            }
            else {//�̰��� ������ ���
                Qout_cms = roQ_CONST_CMS * ts.dtsec;
                if (cvs[i].fcType == flowControlType::SinkFlow
                    && cvs[i].fcType == flowControlType::SourceFlow) {
                    // Qinput_m3�� sink, source���� �������µ��� storage�� 0�̹Ƿ�, ���⼭�� 0
                    cvs[i].storageCumulative_m3 = 0; 
                }
                else {
                    cvs[i].storageCumulative_m3 = Qinput_m3
                        - roQ_CONST_CMS * ts.dtsec;
                }
            }

            //double Storage_tM1 = (roQ_CONST_CMS * ts.dtsec) + cvs[i].storageCumulative_m3;
            //if (Storage_tM1 < 0)
            //{
            //    Qout_cms = 0;
            //}
            //else
            //{
            //    Qout_cms = Storage_tM1 / (double)sThisSimulation.dtsec;
            //}
            //cvs[i].StorageCumulative_m3 = 0;
        }
        else        {

            Qout_cms = roQ_CONST_CMS;
            cvs[i].StorageCumulative_m3 = cvs[i].StorageCumulative_m3 - roQ_CONST_CMS;
            if (cvs[i].StorageCumulative_m3 >= maxStorageApp)
                // Constant dischrage������ ��� ���� ��Ų��. 
                // ������������ �ִ������� ���� ���ų� ũ��, ���̻� �������� �ʰ�, �ִ��������� �����Ѵ�.
            {
                cvs[i].StorageCumulative_m3 = maxStorageApp;
            }
        }
    }
    else {
        Qout_cms = 0;
        if (cvs[i].fcType != flowControlType::SinkFlow
            && cvs[i].fcType != flowControlType::SourceFlow) {
            cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3 + Qinput_m3;
        }
    }
    calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
}


void calReservoirAutoROM(int i, double maxStorageApp)
{
    double Qout_cms;
    double dy_m = di.cellSize;
    double Qinput_m3;
    if (cvs[i].storageCumulative_m3 >= maxStorageApp) {
        if (cvs[i].fcType == flowControlType::SinkFlow
            || cvs[i].fcType == flowControlType::SourceFlow)        {
            // �̰�쿡�� �����װ� ��� ���Է��� storage�� ������ �ִ�.
            Qout_cms = (cvs[i].storageCumulative_m3
                - maxStorageApp) / ts.dtsec; // ���̸�ŭ ��� ����
            if (Qout_cms < 0) { Qout_cms = 0; }
        }
        else {
            Qinput_m3 = cvs[i].storageAddedForDTbyRF_m3
                + cvs[i].QsumCVw_dt_m3; //�����װ� ��� ���Է�
            Qout_cms = Qinput_m3 / ts.dtsec;  // �̶��� ���� Ư���� ������� ��� ���� ��� ���� �����.
        }        
        cvs[i].storageCumulative_m3 = maxStorageApp; // ������������ �ִ������� ���� ���ų� ũ��, ���̻� �������� �ʰ�, �ִ��������� �����Ѵ�.
    }
    else {
        Qout_cms = 0;
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3 
            + Qinput_m3;
    }
    calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
}


void calReservoirRigidROM(int i, double maxStorageApp,
    double roQ_CONST_CMS) {
    double dy_m = di.cellSize;
    double Qout_cms;
    double Qinput_m3;
    double Qinput_m3 = cvs[i].storageAddedForDTbyRF_m3
        + cvs[i].QsumCVw_dt_m3;
    if (cvs[i].storageCumulative_m3 < roQ_CONST_CMS * ts.dtsec
        || maxStorageApp < roQ_CONST_CMS * ts.dtsec) {
        // ���� �������� dtsec ���ⷮ ���� ���� ���
        // �̰��� �־��� ���ⷮ���� �������� ���ϰ�, �װͺ��� ���� ������ ����ȴٴ� �ǹ�
        // �� dt �ð����� ������ ��� ���� ����Ǵ� �������� ���� ������������ ���ⷮ�� ����ؾ� �Ѵ�.
        Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
        cvs[i].storageCumulative_m3 = 0;
    }
    else if (cvs[i].storageCumulative_m3 >= maxStorageApp) {
        if (cvs[i].fcType == flowControlType::SinkFlow
            || cvs[i].fcType == flowControlType::SourceFlow) {
            // �̰�쿡�� �����װ� ��� ���Է��� storage�� ������ �ִ�.
            Qout_cms = (cvs[i].storageCumulative_m3
                - maxStorageApp) / ts.dtsec; // ���̸�ŭ ��� ����
            if (Qout_cms < 0) { Qout_cms = 0; }
        }
        else {
            // �̰��� ���ԵǴ� ��� ���� ����
            Qout_cms = Qinput_m3 / ts.dtsec;
        }
        cvs[i].storageCumulative_m3 = maxStorageApp; // ������������ �ִ������� ���� ���ų� ũ��, ���̻� �������� �ʰ�, �ִ��������� �����Ѵ�.
    }
    else {
        Qout_cms = roQ_CONST_CMS;
        if (cvs[i].fcType == flowControlType::SinkFlow
            || cvs[i].fcType == flowControlType::SourceFlow) {
            // sink flow or source flow������ storageCumulative_m3 �� �̹� ������ �ֹǷ�,
            // rigid�� ����Ǵ� �縸 ���ش�.
            cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                - Qout_cms;
        }
        else { // �̰��� ���Է��� ���� �� rigid ���ⷮ�� ����.
            cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                + Qinput_m3 - Qout_cms;
        }
    }
    calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
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