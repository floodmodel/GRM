#include "stdafx.h"
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
    fccds.inputFlowData_m3Ps.clear();
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
                string outstr = "ERROR : Flow control data file (" + afc.fpnFCData 
                    + ") is invalid.\n";
                writeLog(fpnLog, outstr, 1, 1);
                return -1;
            }
            vector<double> vs;
            vs = readTextFileToDoubleVector(afc.fpnFCData);
			if (vs.size() == 0) {
				string outstr = "WARNNING : Flow control data file (" + afc.fpnFCData
					+ ") has no value.\n";
				writeLog(fpnLog, outstr, 1, 1);
				if (ts.enforceFCautoROM == -1) {
					return -1;
				}
			}
            for (int i = 0; i < vs.size(); ++i) {
                timeSeries ts;
                if (prj.isDateTimeFormat == 1) {
                    ts.dataTime = timeElaspedToDateTimeFormat2(prj.simStartTime,
                        afc.fcDT_min * 60 * i,  timeUnitToShow::toM
                        , dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
                }
                else {
                    ts.dataTime = afc.fcDT_min * i;
                }
                ts.value = vs[i];
                fccds.inputFlowData_m3Ps[aidx].push_back(ts);
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
{// nowTmin�� �ּҰ��� dtsec/60�̴�.
    int dtfc = prj.fcs[i].fcDT_min;
	int calStreamFlow = 1;
	if (nowTmin >= dtfc * fccds.curDorder[i]) {
		if (fccds.curDorder[i] < fccds.inputFlowData_m3Ps[i].size()) {
			fccds.curDorder[i]++;
		}
		else {
			// �Ʒ� ���� �ּ�ó���ϸ�, ������ �ڷᰡ ������ ���ȴ�..
			if (ts.enforceFCautoROM == -1) {
				setNoFluxCVCH(i);
				fccds.fcDataAppliedNowT_m3Ps[i] = 0;
			}
			else if (ts.enforceFCautoROM == 1) {	
				double t_currentFC = dtfc * fccds.curDorder[i];
				convertFCtypeToAutoROM(dtos(t_currentFC, 2) +" min", i);
			}
			calStreamFlow = 0;
		}
	}
	if (calStreamFlow == 1) {
		int orderidx = fccds.curDorder[i] - 1;//vector index, �� ��������  fccds.curDorder[i]�� �ּҰ��� 1
		double v = fccds.inputFlowData_m3Ps[i][orderidx].value;
		if (v < 0) { v = 0; }
		cvs[i].stream.QCH_m3Ps = v;
		cvs[i].stream.csaCH = getChCSAusingQbyiteration(cvs[i], cvs[i].stream.csaCH, cvs[i].stream.QCH_m3Ps);
		cvs[i].stream.hCH = getChDepthUsingCSA(cvs[i].stream.chBaseWidth,
			cvs[i].stream.csaCH, cvs[i].stream.isCompoundCS, cvs[i].stream.chURBaseWidth_m,
			cvs[i].stream.chLRArea_m2, cvs[i].stream.chLRHeight, cvs[i].stream.bankCoeff);
		cvs[i].stream.uCH = cvs[i].stream.QCH_m3Ps / cvs[i].stream.csaCH;
		fccds.fcDataAppliedNowT_m3Ps[i] = cvs[i].stream.QCH_m3Ps;
	}

    //�Ʒ�����  cvs[i].QsumCVw_dt_m3�� t-dt ������ ���� ����Ǿ� �ִ�.
    // �׸���, fcDataAppliedNowT_m3Ps �� ���� ������ �Ϸ��� ����Ǵ� ���̹Ƿ�, ���ش�.
	if (cvs[i].fcType == flowControlType::ReservoirOutflow && cvs[i].fac>0) {
		// ���⿡�� w�� ������ ���Է� ���..
		if (cvs[i].flowType == cellFlowType::OverlandFlow) {
			getOverlandFlowDepthCVw(i);
		}
		else {
			getChCSAatCVW(i);
		}
		cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
			+ cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
			- fccds.fcDataAppliedNowT_m3Ps[i] * ts.dtsec;
		if (cvs[i].storageCumulative_m3 < 0) {
			cvs[i].storageCumulative_m3 = 0;
		}
	}
}


void calSinkOrSourceFlow(int i, double nowTmin)
{// nowTmin�� �ּҰ��� dtsec/60�̴�.
    int dtfc = prj.fcs[i].fcDT_min;
    double cellsize = di.cellSize;
	if (nowTmin > dtfc * fccds.curDorder[i]) {
		double t_currentFC = dtfc * fccds.curDorder[i];
		if (fccds.curDorder[i] < fccds.inputFlowData_m3Ps[i].size()) {
			fccds.curDorder[i]++;
		}
		else {
			// �Ʒ� ���� �ּ�ó���ϸ�, ������ �ڷᰡ ������ ���ȴ�..
			if (ts.enforceFCautoROM == -1) {
				setNoFluxCVCH(i);
				fccds.fcDataAppliedNowT_m3Ps[i] = 0.0;
			}
			else if (ts.enforceFCautoROM == 1) {
				convertFCtypeToAutoROM(dtos(t_currentFC, 2) + " min", i);
			}
			return; //sink, source flow �������� �ʴ´�
		}
	}
    int orderidx = fccds.curDorder[i]-1 ;//vector index, �� �������� fccds.curDorder[i] �� �ּҰ��� 1
    double QtoApp = fccds.inputFlowData_m3Ps[i][orderidx].value;
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
	int dtsec = ts.dtsec;
	if (cvs[i].fcType == flowControlType::SinkFlow) {
		cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
			+ cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
			- fccds.fcDataAppliedNowT_m3Ps[i] * dtsec;
	}
	else if (cvs[i].fcType == flowControlType::SourceFlow) {//������ source flow �� ���Ǿ�����..
		cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
			+ cvs[i].storageAddedForDTbyRF_m3 + cvs[i].QsumCVw_dt_m3
			+ fccds.fcDataAppliedNowT_m3Ps[i] * dtsec;
	}
	if (cvs[i].storageCumulative_m3 < 0) {
		cvs[i].storageCumulative_m3 = 0;
	}
}

void calReservoirOperation(int i, double nowTmin)
{
    int dtsec = ts.dtsec;
    //int i = i + 1;
	flowControlinfo afci;
	afci = prj.fcs[i];
    double QforDTbySinkOrSourceFlow = 0;
    double cellsize = di.cellSize;
   //������ sinkflow or source flow ������, calSinkOrSourceFlow(int i, double nowTmin) ���� cell storage �̸� ���ȴ�.
	//������ sinkflow or source flow �� �ƴϾ�����, ������¿����� cell storage �ѹ� ����Ѵ�.
	if (cvs[i].fcType != flowControlType::SinkFlow &&
	    cvs[i].fcType != flowControlType::SourceFlow){
        cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
            + cvs[i].storageAddedForDTbyRF_m3
            + cvs[i].QsumCVw_dt_m3;
    }
	if (cvs[i].storageCumulative_m3 < 0) {
		cvs[i].storageCumulative_m3 = 0;
	}
    double Qout_cms = 0.0;
    double dy_m = di.cellSize;
	double maxStorageApp=0;
	maxStorageApp = afci.NormalHighStorage_m3;
	if (prj.isDateTimeFormat == 1) { // RestrictedStorage_m3>0 �� ��쿡�� ����
		if (afci.RestrictedStorage_m3 > 0) {
			if (ts.tElapsed_DateTime_Month >= afci.restricedP_SM
				&& ts.tElapsed_DateTime_Day >= afci.restricedP_SD) {
				if (ts.tElapsed_DateTime_Month <= afci.restricedP_EM
					&& ts.tElapsed_DateTime_Day <= afci.restricedP_ED) {
					maxStorageApp = afci.RestrictedStorage_m3;
				}
			}
		}
	}
	else {
		if (afci.RestrictedStorage_m3 > 0) {
			if (afci.RestrictedPeriod_Start_min >= nowTmin
				&& afci.RestrictedPeriod_End_min <= nowTmin) {
				maxStorageApp = afci.RestrictedStorage_m3;
			}
		}
	}
    //double maxStorageApp = afci.maxStorage_m3 * afci.maxStorageR;
    switch (afci.roType) {
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
        if (cvs[i].storageCumulative_m3 >= maxStorageApp) {
            Qout_cms = (cvs[i].storageCumulative_m3
                - maxStorageApp) / ts.dtsec; // ���̸�ŭ ��� ����
            cvs[i].storageCumulative_m3 = maxStorageApp;
        }
        else  if (cvs[i].storageCumulative_m3 < afci.roConstQ_cms * ts.dtsec) {
            // ���� �������� dtsec ���ⷮ ���� ���� ���, ������ ��� ���� ����
            Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
            cvs[i].storageCumulative_m3 = 0;
        }
        else {
            Qout_cms = afci.roConstQ_cms;
            cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                - afci.roConstQ_cms;
            if (cvs[i].storageCumulative_m3 < 0) {
                cvs[i].storageCumulative_m3 = 0;
            }
        }
        if (Qout_cms < 0) { Qout_cms = 0; }
        calReservoirOutFlowInReservoirOperation(i, Qout_cms, dy_m);
        break;
    }
    case reservoirOperationType::ConstantQ: {
        if (nowTmin <= afci.roConstQDuration_hr * 60) {
            //���⼭�� �ִ� ���� ���ɷ��� ��� ���� ������ ���            
            if (cvs[i].storageCumulative_m3 <= afci.roConstQ_cms * ts.dtsec) {
                // �̰��� �������� ��� ����
                Qout_cms = cvs[i].storageCumulative_m3 / ts.dtsec;
                cvs[i].storageCumulative_m3 = 0;
            }
            else {
                Qout_cms = afci.roConstQ_cms * ts.dtsec;
                cvs[i].storageCumulative_m3 = cvs[i].storageCumulative_m3
                    - afci.roConstQ_cms * ts.dtsec;
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
                / pow(sqrt(cvs[i].slopeOF), 0.6);  // 0.5 = 3/5
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

void convertFCtypeToAutoROM(string strDate, int cvidx)
{
	reservoirOperationType rot_bak;
	rot_bak = prj.fcs[cvidx].roType;
	prj.fcs[cvidx].fcType = flowControlType::ReservoirOperation;
	prj.fcs[cvidx].roType = reservoirOperationType::AutoROM;
	cvs[cvidx].fcType = flowControlType::ReservoirOperation;
    string fcname = prj.fcs[cvidx].fcName;
	if (rot_bak != reservoirOperationType::AutoROM) {
		string msg = "  Reservoir operation type was converted to AutoROM (FC Name:"
			+ fcname + ", (ColX, RowY):(" + to_string(prj.fcs[cvidx].fcColX)
			+", "+ to_string(prj.fcs[cvidx].fcRowY) + "), Time:" + strDate
			+ ").\n";
		writeLog(fpnLog, msg, 1, 1);
	}
	
}
