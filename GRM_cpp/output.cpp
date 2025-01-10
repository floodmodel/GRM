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
extern map<int, double> fcInflowSumPT_m3_Bak;// <idx, value> t-dt 시간에 적용된 flow control inflow 합
extern wpSimData wpSimValueB;;
extern string msgFileProcess;


void writeSimProgress(int elapsedT_min)
{
    double nowStep;
    double simDur_min = prj.simDuration_hr * 60.0;
    nowStep = elapsedT_min / simDur_min * 100.0;
	if (nowStep > 100) { nowStep = 100; }
	if (ts.showFileProgress  !=1) {
		printf("\rCurrent progress: %.2f%%... ", nowStep);
	}
	else {
		printf("\rCurrent progress: %.2f%%... %s", nowStep, msgFileProcess.c_str());
	}
}

void writeDischargeFile(string tStrToPrint, double cinterp)
{   // tStrToPrint.size() == 0 인경우는 유량만 출력하는 경우
	string vs = prj.vSeparator;
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
			lineToP += vs + dtos(vToP, 2);
		}
		else {
			lineToP = dtos(vToP, 2);
		}

		wpSimValue.q_cms_print[i] = vToP; // 이 값은 wp file 출력에 사용됨
	}
	if (tStrToPrint.size() > 0) { // 이경우는 유량 + 다른 정보도 출력
		double rfSumForPT_mm = getMeanRFValueToPrintForAllCells(cinterp);
		lineToP += vs + dtos(rfSumForPT_mm, 2) + "\n"; //+ vs + tsFromStarting_sec + "\n";
	}
	else {
		lineToP += "\n";
	}
	appendTextToTextFile(ofs.ofpnDischarge, lineToP);
}


void writeDischargeAveFile(string tStrToPrint, double cinterp)
{
	double vToP_ave = 0.0;
	string vs = prj.vSeparator;
	string lineToPave = tStrToPrint;
	for (int i : wpSimValue.wpCVidxes) {
		if (cinterp == 1.0) {
			vToP_ave = wpSimValue.Q_sumPTforAVE_m3[i] / prj.dtPrintAveValue_sec; // cms 합에서 총 pt 시간으로 나눈다.
		}
		else if (ts.isbak == 1) {
			vToP_ave = getinterpolatedVLinear(wpSimValueB.Q_sumPTforAVE_m3[i],
				wpSimValue.Q_sumPTforAVE_m3[i], cinterp) / prj.dtPrintAveValue_sec;
		}
		else {
			vToP_ave = 0.0;
		}
		if (lineToPave.size() > 0) {
			lineToPave += vs + dtos(vToP_ave, 2); 
		}
		else {
			lineToPave += dtos(vToP_ave, 2);
		}
		wpSimValue.Q_sumPTforAVE_m3_print[i] = vToP_ave; // 이 값은 wp file 출력에 사용됨
	}
	if (tStrToPrint.size() > 0) { // 이경우는 유량 + 다른 정보도 출력
		double rfSumForPT_mm = getMeanRFValueToPrintAveForAllCells(cinterp);
		lineToPave += vs + dtos(rfSumForPT_mm, 2) + "\n";
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

double getMeanRFValueToPrintAveForAllCells(double cinterp) {
	double rfSumForPT_mm = 0.0;
	if (cinterp == 1.0) {
		rfSumForPT_mm = ts.rfAveSumAllCells_PTave_m * 1000.0;
	}
	else {
		rfSumForPT_mm = getinterpolatedVLinear(ts.rfAveSumAllCells_PTave_m_bak,
			ts.rfAveSumAllCells_PTave_m, cinterp) * 1000.0;
	}
	return rfSumForPT_mm;
}

void writeWPoutputFile(string tStrToPrint, double cinterp)
{
	string vs = prj.vSeparator;
	for (int i : wpSimValue.wpCVidxes) {
		string oStr;
		if (tStrToPrint.size() > 0) {
			oStr.append(tStrToPrint + vs);
		}
		if (cinterp == 1.0) {
			oStr.append(dtos(wpSimValue.q_cms_print[i], 2) + vs); // 이 값은 writeDischargeFile에서 계산되어 있다. 
			oStr.append(dtos(cvs[i].hUAQfromChannelBed_m, 4) + vs);
			oStr.append(dtos(cvs[i].soilWaterC_m, 4) + vs);
			oStr.append(dtos(cvs[i].ssr, 4) + vs);
			oStr.append(dtos(wpSimValue.prcpWPGridForPT_mm[i], 2) + vs);
			oStr.append(dtos(wpSimValue.prcpUpWSAveForPT_mm[i], 2) + vs);

			oStr.append(dtos(wpSimValue.pet_sumPT_mm[i], 4) + vs); // mm로 표시
			oStr.append(dtos(wpSimValue.aet_sumPT_mm[i], 4) + vs); // mm로 표시

			oStr.append(dtos(cvs[i].intcpAcc_m * 1000.0, 4) + vs);	 // mm로 표시

			oStr.append(dtos(cvs[i].spackAcc_m * 100.0, 4) + vs);  // cm로 표시
			oStr.append(dtos(wpSimValue.snowM_sumPT_mm[i], 4) + vs); // mm로 표시

			////wp 별로 inflow, inflow_ave 출력하려면 아래의 것 활성화. 2022.10.17
			//oStr.append(dtos(cvs[i].QsumCVw_m3Ps, 2) + vs);
			//if (prj.printAveValue == 1 && 
			//	std::find(fccds.cvidxsFCcell.begin(), fccds.cvidxsFCcell.end(), i) != fccds.cvidxsFCcell.end()) {
			//			double ptSec = prj.dtPrint_min * 60.0;
			//	oStr.append(dtos(fccds.inflowSumPT_m3[i] / ptSec, 2) + vs);
			//}
			oStr.append(dtos(cvs[i].storageCumulative_m3 / 1000.0, 3) + "\n");
		}
		else if (ts.isbak == 1) {
			oStr.append(dtos(wpSimValue.q_cms_print[i], 2) + vs); // 이 값은 writeDischargeFile에서 계산되어 있다. 
			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].hUAQfromChannelBed_m,
				cvs[i].hUAQfromChannelBed_m, cinterp), 4) + vs);
			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].soilWaterC_m,
				cvs[i].soilWaterC_m, cinterp), 4) + vs);
			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].ssr,
				cvs[i].ssr, cinterp), 4) + vs);
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.prcpWPGridForPT_mm[i],
				wpSimValue.prcpWPGridForPT_mm[i], cinterp), 2) + vs);
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.prcpUpWSAveForPT_mm[i],
				wpSimValue.prcpUpWSAveForPT_mm[i], cinterp), 2) + vs);

			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.pet_sumPT_mm[i],
				cvs[i].pet_mPdt, cinterp), 4) + vs); // mm로 표시
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.aet_sumPT_mm[i],
				cvs[i].aet_mPdt, cinterp), 4) + vs); // mm로 표시

			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].intcpAcc_m,
				cvs[i].intcpAcc_m, cinterp) * 1000.0, 4) + vs); // mm로 표시

			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].spackAcc_m,
				cvs[i].spackAcc_m, cinterp) * 100.0, 4) + vs); // cm로 표시
			oStr.append(dtos(getinterpolatedVLinear(wpSimValueB.snowM_sumPT_mm[i],
				cvs[i].smelt_mPdt, cinterp), 4) + vs); // mm로 표시

			oStr.append(dtos(getinterpolatedVLinear(cvsb[i].storageCumulative_m3,
				cvs[i].storageCumulative_m3, cinterp) / 1000.0, 3) + "\n");
		}
		appendTextToTextFile(ofs.ofpnWPs[i], oStr);
	}
}


void writeRainfallOutputFile(string tStrToPrint, double cinterp) {
	string osRFUpMean;
	string osRFGrid;
	string vs = prj.vSeparator;
	osRFUpMean = tStrToPrint;
	osRFGrid = tStrToPrint;
	for (int i : wpSimValue.wpCVidxes) {
		if (cinterp == 1.0) {
			osRFGrid.append(vs+dtos(wpSimValue.prcpWPGridForPT_mm[i], 2) );
			osRFUpMean.append(vs + dtos(wpSimValue.prcpUpWSAveForPT_mm[i], 2));
		}
		else if (ts.isbak == 1) {
			osRFGrid.append(vs + dtos(getinterpolatedVLinear(wpSimValueB.prcpWPGridForPT_mm[i],
				wpSimValue.prcpWPGridForPT_mm[i], cinterp), 2));
			osRFUpMean.append(vs + dtos(getinterpolatedVLinear(wpSimValueB.prcpUpWSAveForPT_mm[i],
				wpSimValue.prcpUpWSAveForPT_mm[i], cinterp), 2));
		}
	}
	osRFGrid.append("\n");
	osRFUpMean.append(+"\n");
	appendTextToTextFile(ofs.ofpnPRCPMean, osRFUpMean);
	appendTextToTextFile(ofs.ofpnPRCPGrid, osRFGrid);
}


void writeFCOutputFile(string tStrToPrint, double cinterp)
{
	double ptSec = prj.dtPrint_min * 60.0;
	string vs = prj.vSeparator;
	string fc_inflow = "";
	string fc_storage = "";	
	fc_inflow = tStrToPrint;
	fc_storage = tStrToPrint;
	if (cinterp == 1.0) {
		for (int idx : fccds.cvidxsFCcell) {
			if (fc_storage.size() > 0) { fc_storage += vs; } // 앞에 뭔가 있으면, tab으로 구분
			if (cvs[idx].fcType1 == flowControlType::DetentionPond) { // 저류지에서는 별도의 변수
				fc_storage += dtos(cvs[idx].DP_storageCumulative_m3 / 1000.0, 3);
			}
			else {
				fc_storage += dtos(cvs[idx].storageCumulative_m3 / 1000.0, 3);
			}

			if (fc_inflow.size() > 0) { fc_inflow += vs; }
			if (cvs[idx].fcType1 == flowControlType::DetentionPond) { // 저류지에서는 별도의 변수
				fc_inflow += dtos(cvs[idx].DP_inflow_m3Ps, 2);
			}
			else {
				fc_inflow += dtos(cvs[idx].QsumCVw_m3Ps, 2);
			}			
		}
	}
	else if (ts.isbak == 1) {
		for (int idx : fccds.cvidxsFCcell) {
			if (fc_storage.size() > 0) { fc_storage += vs; } // 앞에 뭔가 있으면, tab으로 구분
			if (cvs[idx].fcType1 == flowControlType::DetentionPond) { // 저류지에서는 별도의 변수
				fc_storage += dtos(getinterpolatedVLinear(cvsb[idx].DP_storageCumulative_m3,
					cvs[idx].DP_storageCumulative_m3, cinterp) / 1000.0, 3);
			}
			else {
				fc_storage += dtos(getinterpolatedVLinear(cvsb[idx].storageCumulative_m3,
						cvs[idx].storageCumulative_m3, cinterp) / 1000.0, 3);
			}
			if (fc_inflow.size() > 0) { fc_inflow += vs; }
			if (cvs[idx].fcType1 == flowControlType::DetentionPond) { // 저류지에서는 별도의 변수
				fc_inflow += dtos(getinterpolatedVLinear(cvsb[idx].DP_inflow_m3Ps,
					cvs[idx].DP_inflow_m3Ps, cinterp), 2);
			}
			else {
				fc_inflow += dtos(getinterpolatedVLinear(cvsb[idx].QsumCVw_m3Ps,
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
	string fc_inflow_Ave = "";
	string vs = prj.vSeparator;
	fc_inflow_Ave = tStrToPrint;
	if (cinterp == 1.0) {
		for (int idx : fccds.cvidxsFCcell) {
				if (fc_inflow_Ave == "") {
					fc_inflow_Ave = dtos(fccds.inflowSumPT_m3[idx] / prj.dtPrintAveValue_sec, 2);
				}
				else {
					fc_inflow_Ave += vs
						+ dtos(fccds.inflowSumPT_m3[idx] / prj.dtPrintAveValue_sec, 2);
				}
		}
	}
	else if (ts.isbak == 1) {
		for (int idx : fccds.cvidxsFCcell) {
				if (fc_inflow_Ave == "") {
					fc_inflow_Ave = dtos(getinterpolatedVLinear(fcInflowSumPT_m3_Bak[idx],
						fccds.inflowSumPT_m3[idx], cinterp) / prj.dtPrintAveValue_sec, 2);
				}
				else {
					fc_inflow_Ave += vs
						+ dtos(getinterpolatedVLinear(fcInflowSumPT_m3_Bak[idx],
							fccds.inflowSumPT_m3[idx], cinterp) / prj.dtPrintAveValue_sec, 2);
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
	ofs.ofpnPRCPGrid = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_PRCP_GRID;
	ofs.ofpnPRCPMean = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_PRCP_MEAN;
	ofs.ofpnFCStorage = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FC_STORAGE;
	ofs.ofpnFCinflow = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FC_INFLOW;
	ofs.ofpnFCinflowPTAve = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FC_INFLOW_PTAVE;

	ofs.ofpSSRDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj+ CONST_DIST_SSR_DIRECTORY_TAG;
	ofs.ofpPRCPDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj +  CONST_DIST_RF_DIRECTORY_TAG;
	ofs.ofpPRCPAccDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_DIST_RFACC_DIRECTORY_TAG;
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
	fpns.push_back(ofs.ofpnPRCPGrid);
	fpns.push_back(ofs.ofpnPRCPMean);
	fpns.push_back(ofs.ofpnFCStorage);
	fpns.push_back(ofs.ofpnFCinflow);
	fpns.push_back(ofs.ofpnFCinflowPTAve);
	fpns.push_back(ofs.ofpSSRDistribution);
	fpns.push_back(ofs.ofpPRCPDistribution);
	fpns.push_back(ofs.ofpPRCPAccDistribution);
	fpns.push_back(ofs.ofpFlowDistribution);
	for (int id : wpSimValue.wpCVidxes) {
		fpns.push_back(ofs.ofpnWPs[id]);
	}
	vector<string> fps;
	if (prj.makeSoilSaturationDistFile == 1) {
		fps.push_back(ofs.ofpSSRDistribution);
	}
	if (prj.makeRfDistFile == 1) {
		fps.push_back(ofs.ofpPRCPDistribution);
	}
	if (prj.makeRFaccDistFile == 1) {
		fps.push_back(ofs.ofpPRCPAccDistribution);
	}
	if (prj.makeFlowDistFile == 1) {
		fps.push_back(ofs.ofpFlowDistribution);
	}

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
				fs::remove_all(fps[n]); // 비어있지 않는 것도 삭제 되는지 확인 필요
			}
		}
		cout << "completed. \n";
		writeLog(fpnLog, "Deleting previous output files... completed.\n", 1, -1);
	}
	return 1;
}

// GRMPrintType 중 All, DischargeFile, DischargeAndFcFile, AverageFile 은 파일에 해더를 기록한다.
int makeNewOutputFiles()
{
	string vs = prj.vSeparator;
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
		// wp 별 출력파일 설정
		if (prj.printOption == GRMPrintType::All
			|| prj.printOption == GRMPrintType::DischargeFile
			|| prj.printOption == GRMPrintType::DischargeAndFcFile
			|| prj.printOption == GRMPrintType::AverageFile) { // wp 별 모의 결과는 normal, realtime 모두 파일로 출력
			for (wpLocationRC awp : prj.wps) {
				// wp pet, et, intcp, sm 텍스트파일 출력은 all 옵션에서 wp별 파일에 출력하자..
				time_wpNames = time_wpNames + vs + "[" + awp.wpName + "]";

				if (prj.printOption == GRMPrintType::All) { // wp 별 출력은 ALL 에서만 한다.
					string awpName = replaceText(awp.wpName, ",", "_");
					heads = comHeader
						+ "Output data : All the results for watch point(s) ["
						+ awpName + "]" + "\n\n";
					if (prj.simType == simulationType::RealTime) {
						heads = heads + CONST_OUTPUT_TABLE_TIME_FIELD_NAME + vs
							+ "Flow_sim[m^3/s]" + vs;
						//if (prj.printAveValue == 1) {
						//	heads = heads + "Flow_sim_AVE[CMS]" + vs;
						//}
						heads = heads + "Flow_obs[m]" + vs
							+ "PRCP_UpMean[mm]" + "\n";
					}
					else {
						heads = heads + CONST_OUTPUT_TABLE_TIME_FIELD_NAME + vs
							+ "Discharge[m^3/s]" + vs;
						//if (prj.printAveValue == 1) { // WP 별 출력에서는 순간 cms 옆에 Ave cms 기록한다.
						//	heads = heads + "Discharge_AVE[CMS]" + vs;
						//}
						heads = heads + "BaseFlowDepth[m]" + vs
							+ "SoilWaterContent[m]" + vs
							+ "SoilSatR" + vs
							+ "PRCP_Grid[mm]" + vs + "PRCP_UpMean[mm]" + vs
							+ "PETGrid[mm]" + vs + "ETGrid[mm]" + vs
							+ "INTCPAccGrid[mm]" + vs
							+ "SnowPackAccGrid[cm]" + vs + "SnowMeltGrid[mm]" + vs
							+ "FCResStor[1,000m^3]" + "\n";
					}

					int aidx = cvais[awp.wpColX][awp.wpRowY];
					string nFPN = ofs.ofpnWPs[aidx];
					appendTextToTextFile(nFPN, heads);
				}
			}
		}

		if (prj.simType != simulationType::RealTime) { // real time이 아닌경우는 텍스트 파일로 출력, real time에서는 db로 입력
			if (prj.printOption == GRMPrintType::All) {
				// 이건 강우 파일
				heads = comHeader
					+ "Output data : Grid precipitation for each watchpoint [mm]\n\n"
					+ time_wpNames + "\n";
				appendTextToTextFile(ofs.ofpnPRCPGrid, heads);
				heads = comHeader
					+ "Output data : Average precipitation for each watchpoint catchment [mm]\n\n"
					+ time_wpNames + "\n";
				appendTextToTextFile(ofs.ofpnPRCPMean, heads);
			}

			if (prj.printOption == GRMPrintType::All
				|| prj.printOption == GRMPrintType::DischargeFile
				|| prj.printOption == GRMPrintType::DischargeAndFcFile) {
				// 이건 유량 파일
				heads = comHeader
					+ "Output data : Discharge [m^3/s]\n\n"
					+ time_wpNames + vs + CONST_OUTPUT_TABLE_MEAN_PRCP_FIELD_NAME + "\n";
				appendTextToTextFile(ofs.ofpnDischarge, heads);
			}

			if (prj.printOption == GRMPrintType::All
				|| prj.printOption == GRMPrintType::DischargeFile
				|| prj.printOption == GRMPrintType::DischargeAndFcFile
				|| prj.printOption == GRMPrintType::AverageFile) {
				if (prj.printAveValue == 1) {
					// 이건 ave 유량 파일
					heads = comHeader
						+ "Output data : Average discharge [m^3/s] for time interval "
						+ to_string(prj.dtPrintAveValue_min) + "[min].\n\n"						
						+ time_wpNames + vs + CONST_OUTPUT_TABLE_MEAN_PRCP_FIELD_NAME + "\n";
					appendTextToTextFile(ofs.ofpnDischargePTAve, heads);
				}
			}

			// ----------------------------------------------------
			// 여기는 flow control 관련
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
					flowControlinfo afc = prj.fcs[n][0];
					fcDataField = fcDataField + vs + afc.fcName;
					fcNameApp = fcNameApp + vs + afc.fcName;
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
					case flowControlType::DetentionPond:
						fct = ENUM_TO_STR(DetentionPond);
						break;
					case flowControlType::None:
						fct = ENUM_TO_STR(None);
						break;
					}
					fcTypeApp = fcTypeApp + vs + fct;
					if (afc.fcType == flowControlType::ReservoirOutflow
						|| afc.fcType == flowControlType::Inlet
						|| afc.fcType == flowControlType::SinkFlow
						|| afc.fcType == flowControlType::SourceFlow) {
						sourceDT = sourceDT + vs + to_string(afc.fcDT_min);
						resOperation = resOperation + vs + "False";
					}
					else {
						sourceDT = sourceDT + vs + "None";
						if (afc.maxStorage_m3 > 0) {
							resOperation = resOperation + vs + "True";
						}
						else {
							resOperation = resOperation + vs + "False";
						}
					}
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
					roType = roType + vs + roTypeStr;

					if (afc.iniStorage_m3 >= 0) {
						roiniStorage = roiniStorage + vs + dtos(afc.iniStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Initial reservoir storage is invalid.\n", 1, 1);
						return -1;
					}
					if (afc.maxStorage_m3 >= 0) {
						roMaxStorage = roMaxStorage + vs + dtos(afc.maxStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Maximum reservoir storage or storage ratio is invalid.\n", 1, 1);
						return -1;
					}
					if (afc.NormalHighStorage_m3 >= 0) {
						roNormalHighStorage = roNormalHighStorage + vs + dtos(afc.NormalHighStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Normal high water level storage is invalid.\n", 1, 1);
						return -1;
					}
					if (afc.RestrictedStorage_m3 >= 0) {
						roRestrictedStorage = roRestrictedStorage + vs + dtos(afc.RestrictedStorage_m3, 2);
					}
					else if (afc.fcType == flowControlType::ReservoirOperation) {
						writeLog(fpnLog, "ERROR : Restricted water level storage is invalid.\n", 1, 1);
						return -1;
					}

					string rp_s = afc.RestrictedPeriod_Start;
					if (rp_s == "") {
						rp_s = "None";
					}
					roRestrictedPeriod_Start = roRestrictedPeriod_Start + vs + rp_s;
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
					roRestrictedPeriod_End = roRestrictedPeriod_End + vs + rp_e;
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
							roConstQ = roConstQ + vs + dtos(afc.roConstQ_cms, 2);
							if (afc.roConstQDuration_hr > 0) {
								roConstQduration = roConstQduration + vs + to_string(afc.roConstQDuration_hr);
							}
							else {
								writeLog(fpnLog, "ERROR : Constant reservoir outflow duration is invalid.\n", 1, 1);
								return -1;
							}
						}
					}
					else {
						roConstQ = roConstQ + vs + "None";
						roConstQduration = roConstQduration + vs + "None";
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
							+ "Output data : Average reservoir inflow [m^3/s] for "
							+ to_string(prj.dtPrintAveValue_min) + "[min].\n\n"
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
			fs::create_directories(ofs.ofpPRCPDistribution);
		}
		if (prj.makeRFaccDistFile == 1) {
			fs::create_directories(ofs.ofpPRCPAccDistribution);
		}
		if (prj.makeFlowDistFile == 1) {
			fs::create_directories(ofs.ofpFlowDistribution);
		}
	}
	return 1;
}

// 프로젝트 파일과 로그파일도 지워진다. 주의 필요
int deleteAllFilesExceptDischarge()
{
    if (fs::exists(ofs.ofpnDepth)) {
        std::remove(ofs.ofpnDepth.c_str());
    }
    if (fs::exists(ofs.ofpnFCStorage)) {
        std::remove(ofs.ofpnFCStorage.c_str());
    }
	if (fs::exists(ofs.ofpnFCinflow)) {
		std::remove(ofs.ofpnFCinflow.c_str());
	}
	if (fs::exists(ofs.ofpnFCinflowPTAve)) {
		std::remove(ofs.ofpnFCinflowPTAve.c_str());
	}

    if (fs::exists(ofs.ofpnPRCPGrid)) {
        std::remove(ofs.ofpnPRCPGrid.c_str());
    }
    if (fs::exists(ofs.ofpnPRCPMean)) {
        std::remove(ofs.ofpnPRCPMean.c_str());
    }
    if (fs::exists(ofs.ofpFlowDistribution)) {
        fs::remove_all(ofs.ofpFlowDistribution);
    }
    if (fs::exists(ofs.ofpPRCPAccDistribution)) {
        fs::remove_all(ofs.ofpPRCPAccDistribution);
    }
    if (fs::exists(ofs.ofpPRCPDistribution)) {
        fs::remove_all(ofs.ofpPRCPDistribution);
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

