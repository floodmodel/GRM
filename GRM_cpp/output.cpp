#include <io.h>
#include <string>
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
extern wpinfo wpis;
extern flowControlCellAndData fccds;
extern thisSimulation ts;
extern thisSimulationRT tsrt;

extern cvAtt* cvsb;
extern map<int, double> fcdAb;
extern wpinfo wpisb;;
extern string msgFileProcess;



void writeSimStep(int elapsedT_min)
{
    double nowStep;
    double simDur_min = prj.simDuration_hr * 60.0;
    //nowStep = elapsedT_min / (double)ts.simDuration_min * 100.0;
    nowStep = elapsedT_min / simDur_min * 100.0;
	if (nowStep > 100) { nowStep = 100; }
	if (msgFileProcess == "") {
		printf("\rCurrent progress: %.0f%%... ", nowStep);
	}
	else {
		printf("\rCurrent progress: %.0f%%... %s", nowStep, msgFileProcess.c_str());
	}

}


void writeSingleEvent(int nowTmin, double cinterp)
{
    COleDateTime timeNow;
    string tsFromStarting_sec;
    string tStrToPrint;
    double aveRFSumForDTP_mm;
    string vToP = "";
    aveRFSumForDTP_mm = ts.rfAveSumAllCells_dtP_m * 1000.0;
    timeNow = COleDateTime::GetCurrentTime();
    COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
    tsFromStarting_sec = dtos(tsTotalSim.GetTotalSeconds(), 0);
    if (prj.isDateTimeFormat == 1) {
        tStrToPrint = timeElaspedToDateTimeFormat2(prj.simStartTime,
            nowTmin * 60,  timeUnitToShow::toM, 
            dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
    }
    else {
        tStrToPrint = dtos(nowTmin / 60.0, 2);
    }
    // 유량 =================================================
    string lineToP;
	string rfUpMean;
	string rfGrid;
    lineToP = tStrToPrint;
	rfUpMean= tStrToPrint;
	rfGrid = tStrToPrint;
    for (int i : wpis.wpCVidxes) {
        if (cinterp == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = dtos(cvs[i].QOF_m3Ps, 2);
            }
            else {
                vToP = dtos(cvs[i].stream.QCH_m3Ps, 2);
            }
        }
        else if (ts.isbak == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = dtos(getinterpolatedVLinear(cvsb[i].QOF_m3Ps,
                    cvs[i].QOF_m3Ps, cinterp), 2);
            }
            else {
                vToP = dtos(getinterpolatedVLinear(cvsb[i].stream.QCH_m3Ps,
                    cvs[i].stream.QCH_m3Ps, cinterp), 2);
            }
        }
        else {
            vToP = "0";
        }
        lineToP +=  "\t" + vToP;
        double sv = stod(vToP);
        wpis.totalFlow_cms[i] = wpis.totalFlow_cms[i] + sv;
        wpis.qprint_cms[i] = sv;
        if (wpis.maxFlow_cms[i] < sv) {
            wpis.maxFlow_cms[i] = sv;
            wpis.maxFlowTime[i] = tStrToPrint;
        }
		if (prj.printOption == GRMPrintType::All) {
			writeWPoutput(tStrToPrint, i, cinterp);
			rfGrid.append("\t" + dtos(wpis.rfWPGridForDtP_mm[i], 2));
			rfUpMean.append("\t"+dtos(wpis.rfUpWSAveForDtP_mm[i], 2) );
		}
    }
    lineToP += "\t" + dtos(aveRFSumForDTP_mm, 2)
        + "\t" + tsFromStarting_sec + "\n";
    appendTextToTextFile(ofs.ofpnDischarge, lineToP);
	if (prj.printOption == GRMPrintType::All) {
		rfGrid.append("\n");
		rfUpMean.append(+"\n");
		appendTextToTextFile(ofs.ofpnRFMean, rfUpMean);
		appendTextToTextFile(ofs.ofpnRFGrid, rfGrid);
	}

    // FCAppFlow, FCStorage===================================
	if (prj.printOption == GRMPrintType::All) {
		if (prj.simFlowControl == 1 && prj.fcs.size() > 0) {
			writeFCoutput(tStrToPrint, cinterp);
			//string fcflow;
			//string fcStorage;
			//fcflow = tStrToPrint;
			//fcStorage = tStrToPrint;
			//if (cinterp == 1) {
			//	for (int idx : fccds.cvidxsFCcell) {
			//		fcflow += "\t"
			//			+ dtos(fccds.fcDataAppliedNowT_m3Ps[idx], 2);
			//		fcStorage += "\t"
			//			+ dtos(cvs[idx].storageCumulative_m3, 2);
			//	}
			//}
			//else if (ts.isbak == 1) {
			//	for (int idx : fccds.cvidxsFCcell) {
			//		fcflow += "\t"
			//			+ dtos(getinterpolatedVLinear(fcdAb[idx],
			//				fccds.fcDataAppliedNowT_m3Ps[idx], cinterp), 2);
			//		fcStorage += "\t"
			//			+ dtos(getinterpolatedVLinear(cvsb[idx].storageCumulative_m3,
			//				cvs[idx].storageCumulative_m3, cinterp), 2);
			//	}
			//}
			//fcflow += "\n";
			//fcStorage += "\n";
			//appendTextToTextFile(ofs.ofpnFCData, fcflow);
			//appendTextToTextFile(ofs.ofpnFCStorage, fcStorage);
		}
	}
    // ==================================
}

void writeFCoutput(string tStrToPrint, double cinterp)
{
	string fcflow;
	string fcStorage;
	fcflow = tStrToPrint;
	fcStorage = tStrToPrint;
	if (cinterp == 1) {
		for (int idx : fccds.cvidxsFCcell) {
			if (fcflow == "") {
				fcflow = dtos(fccds.fcDataAppliedNowT_m3Ps[idx], 2);
			}
			else {
				fcflow += "\t"
					+ dtos(fccds.fcDataAppliedNowT_m3Ps[idx], 2);
			}
			if (fcStorage == "") {
				fcStorage = dtos(cvs[idx].storageCumulative_m3, 2);
			}
			else {
				fcStorage += "\t"
					+ dtos(cvs[idx].storageCumulative_m3, 2);
			}
		}
	}
	else if (ts.isbak == 1) {
		for (int idx : fccds.cvidxsFCcell) {
			if (fcflow == "") {
				fcflow = dtos(getinterpolatedVLinear(fcdAb[idx],
						fccds.fcDataAppliedNowT_m3Ps[idx], cinterp), 2);
			}
			else {
				fcflow += "\t"
					+ dtos(getinterpolatedVLinear(fcdAb[idx],
						fccds.fcDataAppliedNowT_m3Ps[idx], cinterp), 2);
			}
			if (fcStorage == "") {
				fcStorage = dtos(getinterpolatedVLinear(cvsb[idx].storageCumulative_m3,
						cvs[idx].storageCumulative_m3, cinterp), 2);
			}
			else {
				fcStorage += "\t"
					+ dtos(getinterpolatedVLinear(cvsb[idx].storageCumulative_m3,
						cvs[idx].storageCumulative_m3, cinterp), 2);
			}
		}
	}
	fcflow += "\n";
	fcStorage += "\n";
	appendTextToTextFile(ofs.ofpnFCData, fcflow);
	appendTextToTextFile(ofs.ofpnFCStorage, fcStorage);
}


void writeWPoutput(string nowTP, int i, double cinterp)
{    // watchpoint별 모든 자료 출력
    //int i = i + 1;
    string oStr;
    oStr.append(nowTP + "\t");
    oStr.append(dtos(wpis.qprint_cms[i], 2) + "\t");
    if (cinterp == 1) {
        oStr.append(dtos(cvs[i].hUAQfromChannelBed_m, 4) + "\t");
        oStr.append(dtos(cvs[i].soilWaterC_m, 4) + "\t");
        oStr.append(dtos(cvs[i].ssr, 4) + "\t");
        oStr.append(dtos(wpis.rfWPGridForDtP_mm[i], 2) + "\t");
        oStr.append(dtos(wpis.rfUpWSAveForDtP_mm[i], 2) + "\t");
        oStr.append(dtos(wpis.qFromFCData_cms[i], 2) + "\t");
        oStr.append(dtos(cvs[i].storageCumulative_m3, 2) + "\n");
    }
    else if (ts.isbak == 1) {
        oStr.append(dtos(getinterpolatedVLinear(cvsb[i].hUAQfromChannelBed_m,
            cvs[i].hUAQfromChannelBed_m, cinterp), 4) + "\t");
        oStr.append(dtos(getinterpolatedVLinear(cvsb[i].soilWaterC_m,
            cvs[i].soilWaterC_m, cinterp), 4) + "\t");
        oStr.append(dtos(getinterpolatedVLinear(cvsb[i].ssr,
            cvs[i].ssr, cinterp), 4) + "\t");
        oStr.append(dtos(getinterpolatedVLinear(wpisb.rfWPGridForDtP_mm[i],
            wpis.rfWPGridForDtP_mm[i], cinterp), 2) + "\t");
        oStr.append(dtos(getinterpolatedVLinear(wpisb.rfUpWSAveForDtP_mm[i],
            wpis.rfUpWSAveForDtP_mm[i], cinterp), 2) + "\t");
        oStr.append(dtos(getinterpolatedVLinear(wpisb.qFromFCData_cms[i],
            wpis.qFromFCData_cms[i], cinterp), 2) + "\t");
        oStr.append(dtos(getinterpolatedVLinear(cvsb[i].storageCumulative_m3,
            cvs[i].storageCumulative_m3, cinterp), 2) + "\n");
    }
    appendTextToTextFile(ofs.ofpnWPs[i], oStr);
}

void writeDischargeOnly(double cinterp, int writeWPfiles, int writeFCfiles)
{
    string strFNPDischarge;
    string ptext;
    string vToP = "";
    for (int idx : wpis.wpCVidxes) {
        if (cinterp == 1) {
            if (cvs[idx].flowType == cellFlowType::OverlandFlow) {
                vToP = dtos(cvs[idx].QOF_m3Ps, 2);
            }
            else {
                vToP = dtos(cvs[idx].stream.QCH_m3Ps, 2);
            }
        }
        else if (ts.isbak == 1) {
            if (cvs[idx].flowType == cellFlowType::OverlandFlow) {
                vToP = dtos(getinterpolatedVLinear(cvsb[idx].QOF_m3Ps,
                    cvs[idx].QOF_m3Ps, cinterp), 2);
            }
            else {
                vToP = dtos(getinterpolatedVLinear(cvsb[idx].stream.QCH_m3Ps,
                    cvs[idx].stream.QCH_m3Ps, cinterp), 2);
            }
        }
        else {
            vToP = "0";
        }
        wpis.qprint_cms[idx] = stod(vToP);
        if (writeWPfiles == 1) {
            string ofpn = ofs.ofpnWPs[idx];
			appendTextToTextFile(ofpn, vToP + "\n");
        }
        if (trim(ptext) == "") {
            ptext.append(vToP);
        }
        else {
            ptext.append("\t" + vToP);
        }
    }
    ptext.append("\n");
    appendTextToTextFile(ofs.ofpnDischarge, ptext);
	// FCAppFlow, FCStorage===================================
	if (writeFCfiles==1 &&
		prj.simFlowControl == 1 && prj.fcs.size() > 0) {
		writeFCoutput("", cinterp);
	}
}


int initOutputFiles()
{
	ofs.ofpnDischarge= ppi.fp_prj +"\\"+ ppi.fn_withoutExt_prj+CONST_TAG_DISCHARGE;
	ofs.ofpnDepth = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_DEPTH;
	ofs.ofpnRFGrid = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RFGRID;
	ofs.ofpnRFMean = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RFMEAN;
	//ofs.OFNPSwsPars = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_SWSPARSTEXTFILE;
	ofs.ofpnFCData = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FCDATA_APP;
	ofs.ofpnFCStorage = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FCSTORAGE;
	ofs.ofpSSRDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj+"_" + CONST_DIST_SSR_DIRECTORY_TAG;
	ofs.ofpRFDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "_" + CONST_DIST_RF_DIRECTORY_TAG;
	ofs.ofpRFAccDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "_" + CONST_DIST_RFACC_DIRECTORY_TAG;
	ofs.ofpFlowDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "_" + CONST_DIST_FLOW_DIRECTORY_TAG;

    string ensbModel = "";
    if (tsrt.g_strModel != "") {
        ensbModel = "_m" + tsrt.g_strModel;
    }
    for (wpLocationRC awp : prj.wps) {
        string wpName = replaceText(awp.wpName, ",", "_");
        string wpfpn = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "WP_" + wpName
            + ensbModel + ".out";
        int adix = cvais[awp.wpColX][awp.wpRowY];
        ofs.ofpnWPs[adix] = wpfpn;
    }	
    if (prj.simType == simulationType::RealTime){
        changeOutputFileDisk(tsrt.Output_File_Target_DISK);
    }
    if (deleteAllOutputFiles() == -1) {
        string outstr = "An error was occured while deleting previous ouput files or folders. Try starting the model again. \n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (makeNewOutputFiles() == -1) {
        string outstr = "An error was occured while making ouput files or folders. Try starting the model again. \n";
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
    fpns.push_back(ofs.ofpnDepth);
    fpns.push_back(ofs.ofpnRFGrid);
    fpns.push_back(ofs.ofpnRFMean);
    fpns.push_back(ofs.ofpnFCStorage);
    fpns.push_back(ofs.ofpnFCData);
    fpns.push_back(ofs.ofpSSRDistribution);
    fpns.push_back(ofs.ofpRFDistribution);
    fpns.push_back(ofs.ofpRFAccDistribution);
    fpns.push_back(ofs.ofpFlowDistribution);
    for (int id : wpis.wpCVidxes) {
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
        if (confirmDeleteFiles(fpns) == -1) {
            cout << "failed. \n";
            return -1;
        }
        if (fps.size() > 0) {
            for (int n = 0; n < fps.size(); ++n) {
                fs::remove_all(fps[n]); // 비어있지 않는 것도 삭제 되는지 확인 필요
            }
        }
        cout << "completed. \n";
    }
    return 1;
}


int makeNewOutputFiles()
{
	if (prj.printOption == GRMPrintType::All
		|| prj.printOption == GRMPrintType::DischargeFile) {
		if (prj.makeASCFile == 1 || prj.makeIMGFile == 1) {
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

		// 해더
		string outPutLine;
		string comHeader;
		//tm ltm;
		//time_t now = time(0);
		//localtime_s(&ltm, &now);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		string nowT = timeToString(tnow,
			false, dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
		version grmVersion = getCurrentFileVersion();
		string ver = "GRM v." + to_string(grmVersion.major) + "."
			+ to_string(grmVersion.minor)
			+ "." + to_string(grmVersion.build)
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
		for (wpLocationRC awp : prj.wps) {// wp 별 출력파일 설정
			time_wpNames = time_wpNames + "\t" + "[" + awp.wpName + "]";
			if (prj.printOption == GRMPrintType::All) {
				string awpName = replaceText(awp.wpName, ",", "_");
				heads = comHeader
					+ "Output data : All the results for watch point(s) '"
					+ awpName + "'" + "\n\n";
				if (prj.simType == simulationType::RealTime) {
					heads = heads + CONST_OUTPUT_TABLE_TIME_FIELD_NAME + "\t"
						+ "Flow_sim[cms]" + "\t"
						+ "Flow_obs[m]" + "\t"
						+ "RFUpMean" + "\t"
						+ "FromStarting[min]" + "\n";
				}
				else {
					heads = heads + CONST_OUTPUT_TABLE_TIME_FIELD_NAME + "\t"
						+ "Discharge[cms]" + "\t"
						+ "BaseFlowDepth[m]" + "\t"
						+ "SoilWaterContent[m]" + "\t"
						+ "SoilSatR" + "\t" + "RFGrid" + "\t"
						+ "RFUpMean" + "\t" + "FCData" + "\t"
						+ "FCResStor" + "\n";
				}

				int aidx = cvais[awp.wpColX][awp.wpRowY];
				string nFPN = ofs.ofpnWPs[aidx];
				appendTextToTextFile(nFPN, heads);
			}
		}

		if (prj.simType != simulationType::RealTime) {
			// ----------------------------------------------------
			// 이건 유량
			if (prj.printOption == GRMPrintType::All ||
				prj.printOption == GRMPrintType::DischargeFile) {
				heads = comHeader
					+ "Output data : Discharge[CMS]\n\n"
					+ time_wpNames + "\t" + "Rainfall_Mean" + "\t" + "FromStarting[sec]" + "\n";
				appendTextToTextFile(ofs.ofpnDischarge, heads);
			}

			// ----------------------------------------------------
			//// 이건 수심
			//heads = comHeader
			//    + "Output data : Depth[m]\n\n"
			//    + time_wpNames + "\t" + "Rainfall_Mean" + "\t" + "FromStarting[sec]" + "\n";
			//appendTextToTextFile(ofs.ofpnDepth, heads);

			// 이건 강우
			if (prj.printOption == GRMPrintType::All) {
				heads = comHeader
					+ "Output data : Grid rainfall for each watchpoint [mm]\n\n"
					+ time_wpNames +"\n";
				appendTextToTextFile(ofs.ofpnRFGrid, heads);
				heads = comHeader
					+ "Output data : Average rainfall for each watchpoint catchment [mm]\n\n"
					+ time_wpNames + "\n";
				appendTextToTextFile(ofs.ofpnRFMean, heads);
			}

			// ----------------------------------------------------
			// 여기는 flow control 모듈 관련
			if (prj.printOption == GRMPrintType::All) {
				if (prj.simFlowControl == 1 && prj.fcs.size() > 0)
				{
					string fcNameApp;
					string fcTypeApp;
					string sourceDT;
					string resOperation;
					string roiniStorage;
					string romaxStorage;
					string romaxStorageRatio;
					string romaxStorageApp;
					string roType;
					string roConstQ;
					string roConstQduration;
					fcNameApp = "FC name :";
					fcTypeApp = "FC type :";
					sourceDT = "Source data interval[min] :";
					resOperation = "Res. operation :";
					roiniStorage = "Ini. storage :";
					romaxStorage = "Max. storage :";
					romaxStorageRatio = "Max. storage ratio :";
					romaxStorageApp = "Max. storageMax applied :";
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
							if (afc.maxStorage_m3 > 0 && afc.maxStorageR > 0) {
								resOperation = resOperation + "\t" + "True";
							}
							else {
								resOperation = resOperation + "\t" + "False";
							}
						}
						/*if (afc.fcType == flowControlType::ReservoirOperation) {*/
						if (afc.fcType != flowControlType::Inlet) {
							if (afc.fcType == flowControlType::ReservoirOperation) {
								if (afc.roType != reservoirOperationType::None) {
									roType = roType + "\t" + ENUM_TO_STR(afc.roType);
								}
								else {
									writeLog(fpnLog, "Reservoir operation type is invalid.\n", 1, 1);
									return -1;
								}
							}

							if (afc.iniStorage_m3 >= 0) {
								roiniStorage = roiniStorage + "\t" + to_string(afc.iniStorage_m3);
							}
							else {
								writeLog(fpnLog, "Initial reservoir storage is invalid.\n", 1, 1);
								return -1;
							}
							if (afc.maxStorage_m3 > 0 || afc.maxStorageR > 0) {
								romaxStorage = romaxStorage + "\t" + to_string(afc.maxStorage_m3);
								if (afc.maxStorageR > 0) {
									romaxStorageRatio = romaxStorageRatio + "\t" + to_string(afc.maxStorageR);
									double storApp = afc.maxStorage_m3 * afc.maxStorageR;
									romaxStorageApp = romaxStorageApp + "\t" + to_string(storApp);
								}
							}
							else if (afc.fcType == flowControlType::ReservoirOperation) {
								writeLog(fpnLog, "Maximum reservoir storage or storage ratio is invalid.\n", 1, 1);
								return -1;
							}							
							if (afc.roType == reservoirOperationType::ConstantQ) {
								if (afc.roConstQ_cms < 0) {
									writeLog(fpnLog, "Constant reservoir outflow is invalid.\n", 1, 1);
									return -1;
								}
								else {
									roConstQ = roConstQ + "\t" + to_string(afc.roConstQ_cms);
									if (afc.roConstQDuration_hr > 0) {
										roConstQduration = roConstQduration + "\t" + to_string(afc.roConstQDuration_hr);
									}
									else {
										writeLog(fpnLog, "Constant reservoir outflow duration is invalid.\n", 1, 1);
										return -1;
									}
								}
							}
						}
					}
					// FCApp - flow control data
					heads = comHeader
						+ "Output data : Flow control data input[CMS]\n\n"
						+ fcNameApp + "\n"
						+ fcTypeApp + "\n"
						+ sourceDT + "\n"
						+ resOperation + "\n"
						+ fcDataField + "\n";
					appendTextToTextFile(ofs.ofpnFCData, heads);
					// reservoir operation
					heads = comHeader
						+ "Output data : Storage data[m^3]\n\n"
						+ fcNameApp + "\n"
						+ fcTypeApp + "\n"
						+ roiniStorage + "\n"
						+ romaxStorage + "\n"
						+ romaxStorageRatio + "\n"
						+ romaxStorageApp + "\n"
						+ roType + "\n"
						+ roConstQ + "\n"
						+ roConstQduration + "\n"
						+ fcDataField + "\n";
					appendTextToTextFile(ofs.ofpnFCStorage, heads);
				}
			}
		}
	}
	return 1;
}

int deleteAllFilesExceptDischarge()
{
    if (fs::exists(ofs.ofpnDepth)) {
        std::remove(ofs.ofpnDepth.c_str());
    }
    //if (fs::exists(ofs.ofpnDischarge)) {
    //    std::remove(ofs.ofpnDischarge.c_str());
    //}
    if (fs::exists(ofs.ofpnFCData)) {
        std::remove(ofs.ofpnFCData.c_str());
    }
    if (fs::exists(ofs.ofpnFCStorage)) {
        std::remove(ofs.ofpnFCStorage.c_str());
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
    for (int wpcvid : wpis.wpCVidxes) {
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
