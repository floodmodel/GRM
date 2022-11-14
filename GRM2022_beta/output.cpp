#include "stdafx.h"
#include<ATLComTime.h>

#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern grmOutFiles ofs;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern int** cvais;
extern cvAtt* cvs;
extern domaininfo di;
extern wpSimData wpSimValue;
extern flowControlCellAndData fccds;
extern thisSimulation ts;
extern thisSimulationRT tsrt;

extern cvAtt* cvsb;
extern map<int, double> fcDataAppliedBak;
extern map<int, double> fcInflowSumPT_m3_Bak;// <idx, value> t-dt �ð��� ����� flow control inflow ��
extern wpSimData wpSimValueB;;
extern string msgFileProcess;


void writeSimProgress(int elapsedT_min)
{
    double nowStep;
    double simDur_min = prj.simDuration_hr * 60.0;
    //nowStep = elapsedT_min / (double)ts.simDuration_min * 100.0;
    nowStep = elapsedT_min / simDur_min * 100.0;
	if (nowStep > 100) { nowStep = 100; }
	if (msgFileProcess == "") {
		printf("\rCurrent progress: %.2f%%... ", nowStep);
	}
	else {
		printf("\rCurrent progress: %.2f%%... %s", nowStep, msgFileProcess.c_str());
	}

}

void writeDischargeFile(string tStrToPrint, double cinterp)
{   // tStrToPrint.size() == 0 �ΰ��� ������ ����ϴ� ���
	double ptSec = prj.dtPrint_min * 60.0;
	double vToP = 0.0;
	string lineToP = tStrToPrint;
	for (int i : wpSimValue.wpCVidxes) {
		if (cinterp == 1.0) {
			if (cvs[i].flowType == cellFlowType::OverlandFlow) {
				vToP = cvs[i].QOF_m3Ps;
			}
			else {
				vToP = cvs[i].stream.QCH_m3Ps;
			}
		}
		else if (ts.isbak == 1) {
			if (cvs[i].flowType == cellFlowType::OverlandFlow) {
				vToP = getinterpolatedVLinear(cvsb[i].QOF_m3Ps,
					cvs[i].QOF_m3Ps, cinterp);
			}
			else {
				vToP = getinterpolatedVLinear(cvsb[i].stream.QCH_m3Ps,
					cvs[i].stream.QCH_m3Ps, cinterp);
			}
		}
		else {
			vToP = 0.0;
		}

		if (lineToP.size() > 0) {
			lineToP += "\t" + dtos(vToP, 2);
		}
		else {
			lineToP = dtos(vToP, 2);
		}

		wpSimValue.q_cms_print[i] = vToP; // �� ���� wp file ��¿� ����
		//if (wpSimValue.maxFlow_cms[i] < printV) {
		//	wpSimValue.maxFlow_cms[i] = printV;
		//	wpSimValue.maxFlowTime[i] = tStrToPrint;
		//}
		// =============================
	}
	if (tStrToPrint.size() > 0) { // �̰��� ���� + �ٸ� ������ ���
		double rfSumForPT_mm = getMeanRFValueToPrintForAllCells(cinterp);
		lineToP += "\t" + dtos(rfSumForPT_mm, 2) + "\n"; //+ "\t" + tsFromStarting_sec + "\n";
	}
	else {
		lineToP += "\n";
	}
	appendTextToTextFile(ofs.ofpnDischarge, lineToP);
}


void writeDischargeAveFile(string tStrToPrint, double cinterp)
{
	double ptSec = prj.dtPrint_min * 60.0;
	double vToP_ave = 0.0;
	string lineToPave = tStrToPrint;
	for (int i : wpSimValue.wpCVidxes) {
		if (cinterp == 1.0) {
			vToP_ave = wpSimValue.Q_sumPT_m3[i] / ptSec; // cms �տ��� �� pt �ð����� ������.
		}
		else if (ts.isbak == 1) {
			vToP_ave = getinterpolatedVLinear(wpSimValueB.Q_sumPT_m3[i],
				wpSimValue.Q_sumPT_m3[i], cinterp) / ptSec;
		}
		else {
			vToP_ave = 0.0;
		}
		if (lineToPave.size() > 0) {
			lineToPave += "\t" + dtos(vToP_ave, 2); 
		}
		else {
			lineToPave += dtos(vToP_ave, 2);
		}
		wpSimValue.Q_sumPT_m3_print[i] = vToP_ave; // �� ���� wp file ��¿� ����
		// =============================
	}
	if (tStrToPrint.size() > 0) { // �̰��� ���� + �ٸ� ������ ���
		double rfSumForPT_mm = getMeanRFValueToPrintForAllCells(cinterp);
		lineToPave += "\t" + dtos(rfSumForPT_mm, 2) + "\n";
	}
	else {
		lineToPave += "\n";
	}
	appendTextToTextFile(ofs.ofpnDischargePTAve, lineToPave);
}

double getMeanRFValueToPrintForAllCells(double cinterp) {
	double rfSumForPT_mm = 0.0;
	if (cinterp == 1.0) {
		rfSumForPT_mm = ts.rfAveSumAllCells_PT_m * 1000.0;
	}
	else {
		rfSumForPT_mm = getinterpolatedVLinear(ts.rfAveSumAllCells_PT_m_bak,
			ts.rfAveSumAllCells_PT_m, cinterp) * 1000.0;
	}
	return rfSumForPT_mm;
}

void writeWPoutputFile(string tStrToPrint, double cinterp)
{
	for (int i : wpSimValue.wpCVidxes) {
		string oStr;
		if (tStrToPrint.size() > 0) {
			oStr.append(tStrToPrint + "\t");
		}
		if (cinterp == 1.0) {
			oStr.append(dtos(wpSimValue.q_cms_print[i], 2) + "\t"); // �� ���� writeDischargeFile���� ���Ǿ� �ִ�. 
			if (prj.printAveValue == 1) {
				oStr.append(dtos(wpSimValue.Q_sumPT_m3_print[i], 2) + "\t"); // �� ���� writeDischargeFile���� ���Ǿ� �ִ�. 
			}
			oStr.append(dtos(cvs[i].hUAQfromChannelBed_m, 4) + "\t");
			oStr.append(dtos(cvs[i].soilWaterC_m, 4) + "\t");
			oStr.append(dtos(cvs[i].ssr, 4) + "\t");
			oStr.append(dtos(wpSimValue.rfWPGridForPT_mm[i], 2) + "\t");
			oStr.append(dtos(wpSimValue.rfUpWSAveForPT_mm[i], 2) + "\t");

			oStr.append(dtos(wpSimValue.pet_sumPT_mm[i], 4) + "\t"); // mm�� ǥ��
			oStr.append(dtos(wpSimValue.aet_sumPT_mm[i], 4) + "\t"); // mm�� ǥ��

			oStr.append(dtos(cvs[i].intcpAcc_m * 1000.0, 4) + "\t");	 // mm�� ǥ��

			oStr.append(dtos(cvs[i].spackAcc_m * 100.0, 4) + "\t");  // cm�� ǥ��
			oStr.append(dtos(wpSimValue.snowM_sumPT_mm[i], 4) + "\t"); // mm�� ǥ��

			////wp ���� inflow, inflow_ave ����Ϸ��� �Ʒ��� �� Ȱ��ȭ. 2022.10.17
			//oStr.append(dtos(cvs[i].QsumCVw_m3Ps, 2) + "\t");
			//if (prj.printAveValue == 1 && 
			//	std::find(fccds.cvidxsFCcell.begin(), fccds.cvidxsFCcell.end(), i) != fccds.cvidxsFCcell.end()) {
			//			double ptSec = prj.dtPrint_min * 60.0;
			//	oStr.append(dtos(fccds.inflowSumPT_m3[i] / ptSec, 2) + "\t");
			//}
			oStr.append(dtos(cvs[i].storageCumulative_m3 / 1000.0, 3) + "\n");
		}
		else if (ts.isbak == 1) {
			oStr.append(dtos(wpSimValue.q_cms_print[i], 2) + "\t"); // �� ���� writeDischargeFile���� ���Ǿ� �ִ�. 
			if (prj.printAveValue == 1) {
				oStr.append(dtos(wpSimValue.Q_sumPT_m3_print[i], 2) + "\t"); // �� ���� writeDischargeFile���� ���Ǿ� �ִ�. 
			}
			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].hUAQfromChannelBed_m,
				cvs[i].hUAQfromChannelBed_m, cinterp), 4) + "\t");
			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].soilWaterC_m,
				cvs[i].soilWaterC_m, cinterp), 4) + "\t");
			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].ssr,
				cvs[i].ssr, cinterp), 4) + "\t");
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.rfWPGridForPT_mm[i],
				wpSimValue.rfWPGridForPT_mm[i], cinterp), 2) + "\t");
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.rfUpWSAveForPT_mm[i],
				wpSimValue.rfUpWSAveForPT_mm[i], cinterp), 2) + "\t");

			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.pet_sumPT_mm[i],
				cvs[i].pet_mPdt, cinterp), 4) + "\t"); // mm�� ǥ��
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.aet_sumPT_mm[i],
				cvs[i].aet_mPdt, cinterp), 4) + "\t"); // mm�� ǥ��

			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].intcpAcc_m,
				cvs[i].intcpAcc_m, cinterp) * 1000.0, 4) + "\t"); // mm�� ǥ��

			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].spackAcc_m,
				cvs[i].spackAcc_m, cinterp) * 100.0, 4) + "\t"); // cm�� ǥ��
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.snowM_sumPT_mm[i],
				cvs[i].smelt_mPdt, cinterp), 4) + "\t"); // mm�� ǥ��

			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].storageCumulative_m3,
				cvs[i].storageCumulative_m3, cinterp) / 1000.0, 3) + "\n");
		}
		appendTextToTextFile(ofs.ofpnWPs[i], oStr);
	}
}




void writeRainfallOutputFile(string tStrToPrint, double cinterp) {
	string osRFUpMean;
	string osRFGrid;
	osRFUpMean = tStrToPrint + "\t";
	osRFGrid = tStrToPrint + "\t";
	for (int i : wpSimValue.wpCVidxes) {
		if (cinterp == 1.0) {
			osRFGrid.append(dtos(wpSimValue.rfWPGridForPT_mm[i], 2) + "\t");
			osRFUpMean.append(dtos(wpSimValue.rfUpWSAveForPT_mm[i], 2) + "\t");
		}
		else if (ts.isbak == 1) {
			osRFGrid.append(dtos(getinterpolatedVLinear(wpSimValueB.rfWPGridForPT_mm[i],
				wpSimValue.rfWPGridForPT_mm[i], cinterp), 2) + "\t");
			osRFUpMean.append(dtos(getinterpolatedVLinear(wpSimValueB.rfUpWSAveForPT_mm[i],
				wpSimValue.rfUpWSAveForPT_mm[i], cinterp), 2) + "\t");
		}
	}
	osRFGrid.append("\n");
	osRFUpMean.append(+"\n");
	appendTextToTextFile(ofs.ofpnRFMean, osRFUpMean);
	appendTextToTextFile(ofs.ofpnRFGrid, osRFGrid);
}



void writeFCOutputFile(string tStrToPrint, double cinterp)
{
	double ptSec = prj.dtPrint_min * 60.0;
	string fc_inflow = "";
	string fc_storage = "";	
	fc_inflow = tStrToPrint;
	fc_storage = tStrToPrint;
	if (cinterp == 1.0) {
		for (int idx : fccds.cvidxsFCcell) {
			if (fc_storage.size() > 0) {
				fc_storage += "\t" + dtos(cvs[idx].storageCumulative_m3 / 1000.0, 3);
			}
			else {
				fc_storage = dtos(cvs[idx].storageCumulative_m3 / 1000.0, 3);
			}
			if (fc_inflow.size() > 0) {
				fc_inflow += "\t" + dtos(cvs[idx].QsumCVw_m3Ps, 2);
			}
			else {
				fc_inflow = dtos(cvs[idx].QsumCVw_m3Ps, 2);
			}
		}
	}
	else if (ts.isbak == 1) {
		for (int idx : fccds.cvidxsFCcell) {
			if (fc_storage == "") {
				fc_storage = dtos(getinterpolatedVLinear(cvsb[idx].storageCumulative_m3,
						cvs[idx].storageCumulative_m3, cinterp) / 1000.0, 3);
			}
			else {
				fc_storage += "\t"
					+ dtos(getinterpolatedVLinear(cvsb[idx].storageCumulative_m3,
						cvs[idx].storageCumulative_m3, cinterp) / 1000.0, 3);
			}
			if (fc_inflow.size()> 0) {
				fc_inflow += "\t" + dtos(getinterpolatedVLinear(cvsb[idx].QsumCVw_m3Ps,
					cvs[idx].QsumCVw_m3Ps, cinterp), 2); 
			}
			else {
				fc_inflow = dtos(getinterpolatedVLinear(cvsb[idx].QsumCVw_m3Ps,
					cvs[idx].QsumCVw_m3Ps, cinterp), 2);
			}
		}
	}
	fc_inflow += "\n";
	fc_storage += "\n";
	appendTextToTextFile(ofs.ofpnFCinflow, fc_inflow);
	appendTextToTextFile(ofs.ofpnFCStorage, fc_storage);
}

void writeFCAveOutputFile(string tStrToPrint, double cinterp) {
	double ptSec = prj.dtPrint_min * 60.0;
	string fc_inflow_Ave = "";
	fc_inflow_Ave = tStrToPrint;
	if (cinterp == 1.0) {
		for (int idx : fccds.cvidxsFCcell) {
				if (fc_inflow_Ave == "") {
					fc_inflow_Ave = dtos(fccds.inflowSumPT_m3[idx] / ptSec, 2);
				}
				else {
					fc_inflow_Ave += "\t"
						+ dtos(fccds.inflowSumPT_m3[idx] / ptSec, 2);
				}
		}
	}
	else if (ts.isbak == 1) {
		for (int idx : fccds.cvidxsFCcell) {
				if (fc_inflow_Ave == "") {
					fc_inflow_Ave = dtos(getinterpolatedVLinear(fcInflowSumPT_m3_Bak[idx],
						fccds.inflowSumPT_m3[idx], cinterp) / ptSec, 2);
				}
				else {
					fc_inflow_Ave += "\t"
						+ dtos(getinterpolatedVLinear(fcInflowSumPT_m3_Bak[idx],
							fccds.inflowSumPT_m3[idx], cinterp) / ptSec, 2);
				}
		}
	}
	fc_inflow_Ave += "\n";
	appendTextToTextFile(ofs.ofpnFCinflowPTAve, fc_inflow_Ave);
}


int initOutputFiles()
{
	ofs.ofpnDischarge= ppi.fp_prj +"\\"+ ppi.fn_withoutExt_prj+CONST_TAG_DISCHARGE;
	ofs.ofpnDischargePTAve = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_DISCHARGE_PTAVE;
	ofs.ofpnDepth = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_DEPTH;
	ofs.ofpnRFGrid = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RF_GRID;
	ofs.ofpnRFMean = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RF_MEAN;
	//ofs.OFNPSwsPars = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_SWSPARSTEXTFILE;
	//ofs.ofpnFCData = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FC_DATA_APP;
	ofs.ofpnFCStorage = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FC_STORAGE;
	ofs.ofpnFCinflow = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FC_INFLOW;
	ofs.ofpnFCinflowPTAve = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FC_INFLOW_PTAVE;

	ofs.ofpSSRDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj+ CONST_DIST_SSR_DIRECTORY_TAG;
	ofs.ofpRFDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj +  CONST_DIST_RF_DIRECTORY_TAG;
	ofs.ofpRFAccDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_DIST_RFACC_DIRECTORY_TAG;
	ofs.ofpFlowDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj +  CONST_DIST_FLOW_DIRECTORY_TAG;

	ofs.ofpPETDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_DIST_PET_DIRECTORY_TAG;
	ofs.ofpETDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_DIST_ET_DIRECTORY_TAG;
	ofs.ofpINTCPDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_DIST_INTERCEPTION_DIRECTORY_TAG;
	ofs.ofpSnowMDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_DIST_SNOWMELT_DIRECTORY_TAG;


    string ensbModel = "";
    if (tsrt.g_strModel != "") {
        ensbModel = "_m" + tsrt.g_strModel;
    }
    for (wpLocationRC awp : prj.wps) {
        string wpName = replaceText(awp.wpName, ",", "_");
        string wpfpn = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_WP + wpName
            + ensbModel + ".out";
        int adix = cvais[awp.wpColX][awp.wpRowY];
        ofs.ofpnWPs[adix] = wpfpn;
    }	
    if (prj.simType == simulationType::RealTime){
        changeOutputFileDisk(tsrt.Output_File_Target_DISK);
    }
    if (deleteAllOutputFiles() == -1) {
        string outstr = "ERROR : An error was occured while deleting previous ouput files or folders. Try starting the model again. \n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (makeNewOutputFiles() == -1) {
        string outstr = "ERROR : An error was occured while making ouput files or folders. Try starting the model again. \n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.makeASCorIMGfile == 1) {
        initRasterOutput();
    }
	return 1;
}



int deleteAllOutputFiles()
{
    vector<string> fpns;
    fpns.push_back(ofs.ofpnDischarge);
	fpns.push_back(ofs.ofpnDischargePTAve);
    fpns.push_back(ofs.ofpnDepth);
    fpns.push_back(ofs.ofpnRFGrid);
    fpns.push_back(ofs.ofpnRFMean);
    fpns.push_back(ofs.ofpnFCStorage);
    //fpns.push_back(ofs.ofpnFCData);
	fpns.push_back(ofs.ofpnFCinflow);
	fpns.push_back(ofs.ofpnFCinflowPTAve);
    fpns.push_back(ofs.ofpSSRDistribution);
    fpns.push_back(ofs.ofpRFDistribution);
    fpns.push_back(ofs.ofpRFAccDistribution);
    fpns.push_back(ofs.ofpFlowDistribution);
    for (int id : wpSimValue.wpCVidxes) {
        fpns.push_back(ofs.ofpnWPs[id]);
    }
    vector<string> fps;
    //if (prj.printOption == GRMPrintType::All)    {
        //if (prj.makeASCFile==1 || prj.makeIMGFile == 1)        {
            if (prj.makeSoilSaturationDistFile== 1)            {
                fps.push_back( ofs.ofpSSRDistribution);
            }
            if (prj.makeRfDistFile==1)            {
                fps.push_back(ofs.ofpRFDistribution);
            }
            if (prj.makeRFaccDistFile == 1)            {
                fps.push_back(ofs.ofpRFAccDistribution);
            }
            if (prj.makeFlowDistFile == 1)            {
                fps.push_back(ofs.ofpFlowDistribution);
            }
        //}
    //}
    bool beenRun = false;
    for (string fpn : fpns) {
        if (fs::exists(fpn) == true) {
            beenRun = true;
            break;
        }
    }
    for (string fp : fps) {
        if (fs::exists(fp) == true) {
            beenRun = true;
            break;
        }
    }
    if (beenRun == true) {
        cout << "Deleting previous output files... ";
		writeLog(fpnLog, "Deleting previous output files... \n", 1, -1);
        if (confirmDeleteFiles(fpns) == -1) {
            cout << "failed. \n";
			writeLog(fpnLog, "ERROR : Deleting previous output files... failed.\n", 1, -1);
            return -1;
        }
        if (fps.size() > 0) {
            for (int n = 0; n < fps.size(); ++n) {
                fs::remove_all(fps[n]); // ������� �ʴ� �͵� ���� �Ǵ��� Ȯ�� �ʿ�
            }
        }
        cout << "completed. \n";
		writeLog(fpnLog, "Deleting previous output files... completed.\n", 1, -1);
    }
    return 1;
}

// GRMPrintType �� All, DischargeFile, DischargeAndFcFile, AverageFile �� ���Ͽ� �ش��� ����Ѵ�.
int makeNewOutputFiles()
{
	if (prj.printOption == GRMPrintType::All
		|| prj.printOption == GRMPrintType::DischargeFile
		|| prj.printOption == GRMPrintType::DischargeAndFcFile
		|| prj.printOption == GRMPrintType::AverageFile) {
		string outPutLine;
		string comHeader;
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nowT = timeToString(tnow,
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		version grmVersion = getCurrentFileVersion();
		string ver = "GRM v." + to_string(grmVersion.fmajor) + "."
			+ to_string(grmVersion.fminor)
			+ "." + to_string(grmVersion.fbuild)
			+ " Built in " + grmVersion.LastWrittenTime;

		if (prj.simType == simulationType::RealTime) {
			comHeader = "Project name : " + ppi.fn_prj + " [Real time simulation] " + nowT
				+ "  by " + ver + "\n";
		}
		else {
			comHeader = "Project name : " + ppi.fn_prj + "    " + nowT
				+ "  by " + ver + "\n";
		}
		string heads;
		string time_wpNames;
		time_wpNames = CONST_OUTPUT_TABLE_TIME_FIELD_NAME;
		// wp �� ������� ����
		if (prj.printOption == GRMPrintType::All
			|| prj.printOption == GRMPrintType::DischargeFile
			|| prj.printOption == GRMPrintType::DischargeAndFcFile
			|| prj.printOption == GRMPrintType::AverageFile) { // wp �� ���� ����� normal, realtime ��� ���Ϸ� ���
			for (wpLocationRC awp : prj.wps) {
				// wp pet, et, intcp, sm �ؽ�Ʈ���� ����� all �ɼǿ��� wp�� ���Ͽ� �������..
				time_wpNames = time_wpNames + "\t" + "[" + awp.wpName + "]";

				if (prj.printOption == GRMPrintType::All) { // wp �� ����� ALL ������ �Ѵ�.
					string awpName = replaceText(awp.wpName, ",", "_");
					heads = comHeader
						+ "Output data : All the results for watch point(s) ["
						+ awpName + "]" + "\n\n";
					if (prj.simType == simulationType::RealTime) {
						heads = heads + CONST_OUTPUT_TABLE_TIME_FIELD_NAME + "\t"
							+ "Flow_sim[CMS]" + "\t";
						if (prj.printAveValue == 1) {
							heads = heads + "Flow_sim_AVE[CMS]" + "\t";
						}
						heads = heads + "Flow_obs[m]" + "\t"
							+ "RFUpMean[mm]" + "\n";
					}
					else {
						heads = heads + CONST_OUTPUT_TABLE_TIME_FIELD_NAME + "\t"
							+ "Discharge[CMS]" + "\t";
						if (prj.printAveValue == 1) { // WP �� ��¿����� ���� cms ���� Ave cms ����Ѵ�.
							heads = heads + "Discharge_AVE[CMS]" + "\t";
						}
						heads = heads + "BaseFlowDepth[m]" + "\t"
							+ "SoilWaterContent[m]" + "\t"
							+ "SoilSatR" + "\t"
							+ "RFGrid[mm]" + "\t" + "RFUpMean[mm]" + "\t"
							+ "PETGrid[mm]" + "\t" + "ETGrid[mm]" + "\t"
							+ "INTCPAccGrid[mm]" + "\t"
							+ "SnowPackAccGrid[cm]" + "\t" + "SnowMeltGrid[mm]" + "\t"
							+ "FCResStor[1,000m^3]" + "\n";
					}

					int aidx = cvais[awp.wpColX][awp.wpRowY];
					string nFPN = ofs.ofpnWPs[aidx];
					appendTextToTextFile(nFPN, heads);
				}
			}
		}

		if (prj.simType != simulationType::RealTime) { // real time�� �ƴѰ��� �ؽ�Ʈ ���Ϸ� ���, real time������ db�� �Է�
			if (prj.printOption == GRMPrintType::All) {
				// �̰� ���� ����
				heads = comHeader
					+ "Output data : Grid rainfall for each watchpoint [mm]\n\n"
					+ time_wpNames + "\n";
				appendTextToTextFile(ofs.ofpnRFGrid, heads);
				heads = comHeader
					+ "Output data : Average rainfall for each watchpoint catchment [mm]\n\n"
					+ time_wpNames + "\n";
				appendTextToTextFile(ofs.ofpnRFMean, heads);
			}

			if (prj.printOption == GRMPrintType::All
				|| prj.printOption == GRMPrintType::DischargeFile
				|| prj.printOption == GRMPrintType::DischargeAndFcFile) {
				// �̰� ���� ����
				heads = comHeader
					+ "Output data : Discharge[CMS]\n\n"
					+ time_wpNames + "\t" + "Rainfall_Mean" + "\n";
				appendTextToTextFile(ofs.ofpnDischarge, heads);
			}

			if (prj.printOption == GRMPrintType::All
				|| prj.printOption == GRMPrintType::DischargeFile
				|| prj.printOption == GRMPrintType::DischargeAndFcFile
				|| prj.printOption == GRMPrintType::AverageFile) {
				if (prj.printAveValue == 1) {
					// �̰� ave ���� ����
					heads = comHeader
						+ "Output data : Average discharge [CMS] for printing period. \n\n"
						+ time_wpNames + "\t" + "Rainfall_Mean" + "\n";
					appendTextToTextFile(ofs.ofpnDischargePTAve, heads);
				}
			}

			// ----------------------------------------------------
			// ����� flow control ����
			if (prj.simFlowControl == 1 && prj.fcs.size() > 0) {
				string fcNameApp;
				string fcTypeApp;
				string sourceDT;
				string resOperation;
				string roiniStorage;
				string roMaxStorage;
				string roNormalHighStorage;
				string roRestrictedStorage;
				string roRestrictedPeriod_Start;
				string roRestrictedPeriod_End;
				string roType;
				string roConstQ;
				string roConstQduration;
				fcNameApp = "FC name :";
				fcTypeApp = "FC type :";
				sourceDT = "Source data interval[min] :";
				resOperation = "Res. operation :";
				roiniStorage = "Ini. storage :";
				roMaxStorage = "Max. storage :";
				roNormalHighStorage = "Normal high water level storage :";
				roRestrictedStorage = "Restricted water level storage :";
				roRestrictedPeriod_Start = "First time of restricted water level storage period :";
				roRestrictedPeriod_End = "Last time of restricted water level storage period :";
				roType = "RO type :";
				roConstQ = "Constant Q :";
				roConstQduration = "Constant Q duration :";
				string fcDataField = CONST_OUTPUT_TABLE_TIME_FIELD_NAME;
				for (int n : fccds.cvidxsFCcell) {
					flowControlinfo afc = prj.fcs[n];
					fcDataField = fcDataField + "\t" + afc.fcName;
					fcNameApp = fcNameApp + "\t" + afc.fcName;
					string fct = ENUM_TO_STR(None);
					switch (afc.fcType) {
					case flowControlType::ReservoirOperation:
						fct = ENUM_TO_STR(ReservoirOperation);
						break;
					case flowControlType::Inlet:
						fct = ENUM_TO_STR(Inlet);
						break;
					case flowControlType::ReservoirOutflow:
						fct = ENUM_TO_STR(ReservoirOutflow);
						break;
					case flowControlType::SinkFlow:
						fct = ENUM_TO_STR(SinkFlow);
						break;
					case flowControlType::SourceFlow:
						fct = ENUM_TO_STR(SourceFlow);
						break;
					case flowControlType::None:
						fct = ENUM_TO_STR(None);
						break;
					}
					fcTypeApp = fcTypeApp + "\t" + fct;
					if (afc.fcType == flowControlType::ReservoirOutflow
						|| afc.fcType == flowControlType::Inlet
						|| afc.fcType == flowControlType::SinkFlow
						|| afc.fcType == flowControlType::SourceFlow) {
						sourceDT = sourceDT + "\t" + to_string(afc.fcDT_min);
						resOperation = resOperation + "\t" + "False";
					}
					else {
						sourceDT = sourceDT + "\t" + "None";
						if (afc.maxStorage_m3 > 0) {
							resOperation = resOperation + "\t" + "True";
						}
						else {
							resOperation = resOperation + "\t" + "False";
						}
					}
					/*if (afc.fcType == flowControlType::ReservoirOperation) {*/
					//if (afc.fcType != flowControlType::Inlet) {
						//if (afc.fcType == flowControlType::ReservoirOperation) {
							//if (afc.roType != reservoirOperationType::None) {
					string roTypeStr = "";
					if (afc.roType == reservoirOperationType::AutoROM) {
						roTypeStr = ENUM_TO_STR(AutoROM);
					}
					else if (afc.roType == reservoirOperationType::RigidROM) {
						roTypeStr = ENUM_TO_STR(RigidROM);
					}
					else if (afc.roType == reservoirOperationType::ConstantQ) {
						roTypeStr = ENUM_TO_STR(ConstantQ);
					}
					else if (afc.roType == reservoirOperationType::SDEqation) {
						roTypeStr = ENUM_TO_STR(SDEqation);
					}
					else if (afc.roType == reservoirOperationType::None) {
						roTypeStr = ENUM_TO_STR(None);
					}
					else {
						writeLog(fpnLog, "ERROR : Reservoir operation type of ["
							+ afc.fcName + "] is invalid.\n", 1, 1);
						return -1;
					}
					roType = roType + "\t" + roTypeStr;

					if (afc.iniStorage_m3 >= 0) {
						roiniStorage = roiniStorage + "\t" + dtos(afc.iniStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Initial reservoir storage is invalid.\n", 1, 1);
						return -1;
					}
					if (afc.maxStorage_m3 >= 0) {
						roMaxStorage = roMaxStorage + "\t" + dtos(afc.maxStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Maximum reservoir storage or storage ratio is invalid.\n", 1, 1);
						return -1;
					}
					if (afc.NormalHighStorage_m3 >= 0) {
						roNormalHighStorage = roNormalHighStorage + "\t" + dtos(afc.NormalHighStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Normal high water level storage is invalid.\n", 1, 1);
						return -1;
					}
					if (afc.RestrictedStorage_m3 >= 0) {
						roRestrictedStorage = roRestrictedStorage + "\t" + dtos(afc.RestrictedStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Restricted water level storage is invalid.\n", 1, 1);
						return -1;
					}

					string rp_s = afc.RestrictedPeriod_Start;
					if (rp_s == "") {
						rp_s = "None";
					}
					roRestrictedPeriod_Start = roRestrictedPeriod_Start + "\t" + rp_s;
					if (afc.RestrictedPeriod_Start == ""
						&& afc.fcType == flowControlType::ReservoirOperation
						&& afc.RestrictedStorage_m3 > 0) {
						writeLog(fpnLog, "ERROR : Starting time of applying restricted water level storage is invalid.\n", 1, 1);
						return -1;
					}

					string rp_e = afc.RestrictedPeriod_End;
					if (rp_e == "") {
						rp_e = "None";
					}
					roRestrictedPeriod_End = roRestrictedPeriod_End + "\t" + rp_e;
					if (afc.RestrictedPeriod_End == ""
						&& afc.fcType == flowControlType::ReservoirOperation
						&& afc.RestrictedStorage_m3 > 0) {
						writeLog(fpnLog, "ERROR : Ending time of applying restricted water level storage is invalid.\n", 1, 1);
						return -1;
					}
					if (afc.roType == reservoirOperationType::ConstantQ) {
						if (afc.roConstQ_cms < 0) {
							writeLog(fpnLog, "ERROR : Constant reservoir outflow is invalid.\n", 1, 1);
							return -1;
						}
						else {
							roConstQ = roConstQ + "\t" + dtos(afc.roConstQ_cms, 2);
							if (afc.roConstQDuration_hr > 0) {
								roConstQduration = roConstQduration + "\t" + to_string(afc.roConstQDuration_hr);
							}
							else {
								writeLog(fpnLog, "ERROR : Constant reservoir outflow duration is invalid.\n", 1, 1);
								return -1;
							}
						}
					}
					else {
						roConstQ = roConstQ + "\t" + "None";
						roConstQduration = roConstQduration + "\t" + "None";
					}
				}

				if (prj.printOption == GRMPrintType::All
					|| prj.printOption == GRMPrintType::DischargeAndFcFile) {
					// reservoir storage
					heads = comHeader
						+ "Output data : Storage data[1,000 m^3]\n\n"
						+ fcNameApp + "\n"
						+ fcTypeApp + "\n"
						+ roiniStorage + "\n"
						+ roMaxStorage + "\n"
						+ roNormalHighStorage + "\n"
						+ roRestrictedStorage + "\n"
						+ roRestrictedPeriod_Start + "\n"
						+ roRestrictedPeriod_End + "\n"
						+ roType + "\n"
						+ roConstQ + "\n"
						+ roConstQduration + "\n\n"
						+ fcDataField + "\n";
					appendTextToTextFile(ofs.ofpnFCStorage, heads);

					// reservoir inflow
					heads = comHeader
						+ "Output data : Reservoir inflow[m^3/s]\n\n"
						+ fcDataField + "\n";
					appendTextToTextFile(ofs.ofpnFCinflow, heads);
				}

				if (prj.printOption == GRMPrintType::All
					|| prj.printOption == GRMPrintType::DischargeAndFcFile
					|| prj.printOption == GRMPrintType::AverageFile) {
					// reservoir average inflow 
					if (prj.printAveValue == 1) {
						heads = comHeader
							+ "Output data : Average reservoir inflow [m^3/s] for printing period.\n\n"
							+ fcDataField + "\n";
						appendTextToTextFile(ofs.ofpnFCinflowPTAve, heads);
					}

				}
			}
		}
	}
	if (prj.makeASCorIMGfile == 1) {
		if (prj.makeSoilSaturationDistFile == 1) {
			fs::create_directories(ofs.ofpSSRDistribution);
		}
		if (prj.makeRfDistFile == 1) {
			fs::create_directories(ofs.ofpRFDistribution);
		}
		if (prj.makeRFaccDistFile == 1) {
			fs::create_directories(ofs.ofpRFAccDistribution);
		}
		if (prj.makeFlowDistFile == 1) {
			fs::create_directories(ofs.ofpFlowDistribution);
		}
	}
	return 1;
}

// ������Ʈ ���ϰ� �α����ϵ� ��������. ���� �ʿ�
int deleteAllFilesExceptDischarge()
{
    if (fs::exists(ofs.ofpnDepth)) {
        std::remove(ofs.ofpnDepth.c_str());
    }
    //if (fs::exists(ofs.ofpnFCData)) {
    //    std::remove(ofs.ofpnFCData.c_str());
    //}
    if (fs::exists(ofs.ofpnFCStorage)) {
        std::remove(ofs.ofpnFCStorage.c_str());
    }
	if (fs::exists(ofs.ofpnFCinflow)) {
		std::remove(ofs.ofpnFCinflow.c_str());
	}
	if (fs::exists(ofs.ofpnFCinflowPTAve)) {
		std::remove(ofs.ofpnFCinflowPTAve.c_str());
	}

    if (fs::exists(ofs.ofpnRFGrid)) {
        std::remove(ofs.ofpnRFGrid.c_str());
    }
    if (fs::exists(ofs.ofpnRFMean)) {
        std::remove(ofs.ofpnRFMean.c_str());
    }
    if (fs::exists(ofs.ofpFlowDistribution)) {
        fs::remove_all(ofs.ofpFlowDistribution);
    }
    if (fs::exists(ofs.ofpRFAccDistribution)) {
        fs::remove_all(ofs.ofpRFAccDistribution);
    }
    if (fs::exists(ofs.ofpRFDistribution)) {
        fs::remove_all(ofs.ofpRFDistribution);
    }
    if (fs::exists(ofs.ofpSSRDistribution)) {
        fs::remove_all(ofs.ofpSSRDistribution);
    }
    for (int wpcvid : wpSimValue.wpCVidxes) {
        string fpn = ofs.ofpnWPs[wpcvid];
        if (fs::exists(fpn)) {
            std::remove(fpn.c_str());
        }
    }
    if (fs::exists(ppi.fpn_prj)) {
        std::remove(ppi.fpn_prj.c_str());
    }
	if (fs::exists(fpnLog)) {
		std::remove(fpnLog.string().c_str());
	}
    return 1;
}

inline double  getinterpolatedVLinear(double firstV, double nextV, double cinterp)
{
    double X;
    X = (nextV - firstV) * cinterp + firstV;
    return X;
}
