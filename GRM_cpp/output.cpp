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

extern cvAtt* cvsb;
extern map<int, double> fcdAb;
extern wpinfo wpisb;;
extern string msgFileProcess;



void writeSimStep(int elapsedT_min)
{
    double nowStep;
    string curProgressRatio = "";
    double simDur_min = prj.simDuration_hr * 60.0;
    nowStep = elapsedT_min / simDur_min * 100.0;
    curProgressRatio = forString(nowStep, 0);
    cout<<"\rCurrent progress: " + curProgressRatio + "%... " + msgFileProcess;
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
    tsFromStarting_sec = forString(tsTotalSim.GetTotalSeconds(), 0);
    if (prj.isDateTimeFormat == 1) {
        tStrToPrint = timeElaspedToDateTimeFormat(prj.simStartTime,
            nowTmin * 60, false, dateTimeFormat::yyyy_mm_dd_HHcolMMcolSS);
    }
    else {
        tStrToPrint = forString(nowTmin / 60.0, 2);
    }
    // 유량 =================================================
    string lineToP;
    lineToP = tStrToPrint;
    for (int id : wpis.wpCVIDs) {
        int i = id - 1;
        if (cinterp == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = forString(cvs[i].QOF_m3Ps, 2);
            }
            else {
                vToP = forString(cvs[i].stream.QCH_m3Ps, 2);
            }
        }
        else if (ts.isbak == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = forString(getinterpolatedVLinear(cvsb[i].QOF_m3Ps,
                    cvs[i].QOF_m3Ps, cinterp), 2);
            }
            else {
                vToP = forString(getinterpolatedVLinear(cvsb[i].stream.QCH_m3Ps,
                    cvs[i].stream.QCH_m3Ps, cinterp), 2);
            }
        }
        else {
            vToP = "0";
        }
        lineToP = lineToP + "\t" + vToP;
        double sv = stod(vToP);
        wpis.totalFlow_cms[id] = wpis.totalFlow_cms[id] + sv;
        wpis.qprint_cms[id] = sv;
        if (wpis.maxFlow_cms[id] < sv) {
            wpis.maxFlow_cms[id] = sv;
            wpis.maxFlowTime[id] = tStrToPrint;
        }
        writeWPouput(tStrToPrint, i, cinterp);
    }
    lineToP = lineToP + "\t" + forString(aveRFSumForDTP_mm, 2)
        + "\t" + tsFromStarting_sec + "\n";
    appendTextToTextFile(ofs.ofpnDischarge, lineToP);

    // FCAppFlow, FCStorage===================================
    if (prj.simFlowControl == 1 && prj.fcs.size() > 0) {
        string fcflow;
        string fcStorage;
        fcflow = tStrToPrint;
        fcStorage = tStrToPrint;
        if (cinterp == 1) {
            for (int fcvid : fccds.cvidsFCcell) {
                fcflow = fcflow + "\t" 
                    + forString(fccds.fcDataAppliedNowT_m3Ps[fcvid], 2);
                fcStorage = fcStorage + "\t" 
                    + forString(cvs[fcvid - 1].storageCumulative_m3, 2);
            }
        }
        else if(ts.isbak==1){
            for (int fcvid : fccds.cvidsFCcell) {
                fcflow = fcflow + "\t"
                    + forString(getinterpolatedVLinear( fcdAb[fcvid],
                        fccds.fcDataAppliedNowT_m3Ps[fcvid], cinterp), 2);
                fcStorage = fcStorage + "\t"
                    + forString(getinterpolatedVLinear(cvsb[fcvid - 1].storageCumulative_m3,
                        cvs[fcvid - 1].storageCumulative_m3, cinterp), 2);
            }
        }        
        fcflow = fcflow + "\n";
        fcStorage = fcStorage + "\n";
        appendTextToTextFile(ofs.ofpnFCData, fcflow);
        appendTextToTextFile(ofs.ofpnFCStorage, fcStorage);
    }
    // ==================================
}


void writeWPouput(string nowTP, int i, double cinterp)
{    // watchpoint별 모든 자료 출력
    int cvid = i + 1;
    string oStr;
    oStr.append(nowTP + "\t");
    oStr.append(forString(wpis.qprint_cms[cvid], 2) + "\t");
    if (cinterp == 1) {
        oStr.append(forString(cvs[i].hUAQfromChannelBed_m, 4) + "\t");
        oStr.append(forString(cvs[i].soilWaterC_m, 4) + "\t");
        oStr.append(forString(cvs[i].ssr, 4) + "\t");
        oStr.append(forString(wpis.rfWPGridForDtP_mm[cvid], 2) + "\t");
        oStr.append(forString(wpis.rfUpWSAveForDtP_mm[cvid], 2) + "\t");
        oStr.append(forString(wpis.qFromFCData_cms[cvid], 2) + "\t");
        oStr.append(forString(cvs[i].storageCumulative_m3, 2) + "\n");
    }
    else if (ts.isbak == 1) {
        oStr.append(forString(getinterpolatedVLinear(cvsb[i].hUAQfromChannelBed_m,
            cvs[i].hUAQfromChannelBed_m, cinterp), 4) + "\t");
        oStr.append(forString(getinterpolatedVLinear(cvsb[i].soilWaterC_m,
            cvs[i].soilWaterC_m, cinterp), 4) + "\t");
        oStr.append(forString(getinterpolatedVLinear(cvsb[i].ssr,
            cvs[i].ssr, cinterp), 4) + "\t");
        oStr.append(forString(getinterpolatedVLinear(wpisb.rfWPGridForDtP_mm[cvid],
            wpis.rfWPGridForDtP_mm[cvid], cinterp), 2) + "\t");
        oStr.append(forString(getinterpolatedVLinear(wpisb.rfUpWSAveForDtP_mm[cvid],
            wpis.rfUpWSAveForDtP_mm[cvid], cinterp), 2) + "\t");
        oStr.append(forString(getinterpolatedVLinear(wpisb.qFromFCData_cms[cvid],
            wpis.qFromFCData_cms[cvid], cinterp), 2) + "\t");
        oStr.append(forString(getinterpolatedVLinear(cvsb[i].storageCumulative_m3,
            cvs[i].storageCumulative_m3, cinterp), 2) + "\n");
    }
    //string ofpn = wpis.fpnWpOut[cvid];
    //string ofpn = ofs.ofpnWPs[cvid];
    appendTextToTextFile(ofs.ofpnWPs[cvid], oStr);
}

void writeDischargeOnly(double cinterp, int writeWPfiles)
{
    string strFNPDischarge;
    string ptext;
    string vToP = "";
    for (int cvid : wpis.wpCVIDs) {
        int i = cvid - 1;
        if (cinterp == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = forString(cvs[i].QOF_m3Ps, 2);
            }
            else {
                vToP = forString(cvs[i].stream.QCH_m3Ps, 2);
            }
        }
        else if (ts.isbak == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = forString(getinterpolatedVLinear(cvsb[i].QOF_m3Ps,
                    cvs[i].QOF_m3Ps, cinterp), 2);
            }
            else {
                vToP = forString(getinterpolatedVLinear(cvsb[i].stream.QCH_m3Ps,
                    cvs[i].stream.QCH_m3Ps, cinterp), 2);
            }
        }
        else {
            vToP = "0";
        }
        wpis.qprint_cms[cvid] = stod(vToP);
        if (writeWPfiles == 1) {
            string ofpn = ofs.ofpnWPs[cvid];
            appendTextToTextFile(ofpn, vToP);
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
    for (wpLocationRC awp : prj.wps) {
        string wpName = replaceText(awp.wpName, ",", "_");
        string wpfpn = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "WP_" + wpName + ".out";
        int acvid = cvais[awp.wpColX][awp.wpRowY] + 1;
        ofs.ofpnWPs[acvid] = wpfpn;
    }	
    if (prj.simType == simulationType::RealTime){
        changeOutputFileDisk(cRealTime::CONST_Output_File_Target_DISK);
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

int changeOutputFileDisk(char targetDisk)
{
	int isnormal = -1;
	ofs.ofpnDischarge = IO_Path_ChangeDrive(targetDisk, ofs.ofpnDischarge);
	ofs.ofpnDepth = IO_Path_ChangeDrive(targetDisk, ofs.ofpnDepth);
	ofs.ofpnRFGrid = IO_Path_ChangeDrive(targetDisk, ofs.ofpnRFGrid);
	ofs.ofpnRFMean = IO_Path_ChangeDrive(targetDisk, ofs.ofpnRFMean);
	//ofs.OFNPSwsPars = IO_Path_ChangeDrive(targetDisk, ofs.OFNPSwsPars);
	ofs.ofpnFCData = IO_Path_ChangeDrive(targetDisk, ofs.ofpnFCData);
	ofs.ofpnFCStorage = IO_Path_ChangeDrive(targetDisk, ofs.ofpnFCStorage);
	ofs.ofpSSRDistribution = IO_Path_ChangeDrive(targetDisk, ofs.ofpSSRDistribution);
	ofs.ofpRFDistribution = IO_Path_ChangeDrive(targetDisk, ofs.ofpRFDistribution);
	ofs.ofpRFAccDistribution = IO_Path_ChangeDrive(targetDisk, ofs.ofpRFAccDistribution);
	ofs.ofpFlowDistribution = IO_Path_ChangeDrive(targetDisk, ofs.ofpFlowDistribution);
	isnormal = 1;
	return isnormal;
}


int deleteAllOutputFiles()
{
    vector<string> fpns;
    fpns.push_back(ofs.ofpnDischarge);
    fpns.push_back(ofs.ofpnDepth);
    fpns.push_back(ofs.ofpnRFGrid);
    fpns.push_back(ofs.ofpnRFMean);
    fpns.push_back(ofs.ofpnFCStorage);
    fpns.push_back(ofs.ofpSSRDistribution);
    fpns.push_back(ofs.ofpRFDistribution);
    fpns.push_back(ofs.ofpRFAccDistribution);
    fpns.push_back(ofs.ofpFlowDistribution);
    for (int id : wpis.wpCVIDs) {
        fpns.push_back(ofs.ofpnWPs[id]);
    }
    vector<string> fps;
    if (prj.printOption == GRMPrintType::All)    {
        if (prj.makeASCFile==1 || prj.makeIMGFile == 1)        {
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
        }
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
    if (prj.printOption == GRMPrintType::All) {
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
        tm ltm;
        time_t now = time(0);
        localtime_s(&ltm, &now);
        string nowT = timeToString(ltm,
            false, dateTimeFormat::yyyy_mm_dd_HHcolMMcolSS);
        version grmVersion = getCurrentFileVersion();
        string ver = "GRM v."+to_string(grmVersion.major)+"."
            +to_string(grmVersion.minor)
            +"."+to_string(grmVersion.build)
            +" Built in "+ grmVersion.LastWrittenTime;
        comHeader = "Project name : " + ppi.fn_prj + "      " + nowT 
            + "  by " + ver+ "\n";
        string time_wpNames;
        time_wpNames = CONST_OUTPUT_TABLE_TIME_FIELD_NAME;
        string heads;
        for (wpLocationRC awp : prj.wps) {// wp 별 출력파일 설정
            time_wpNames = time_wpNames + "\t" + "[" + awp.wpName + "]";
            string awpName = replaceText(awp.wpName, ",", "_");
            heads = comHeader
                + "Output data : All the results for watch point '"
                + awpName + "'" + "\n\n";
            heads = heads + CONST_OUTPUT_TABLE_TIME_FIELD_NAME + "\t"
                + "Discharge[cms]" + "\t"
                + "BaseFlowDepth[m]" + "\t"
                + "SoilWaterContent[m]" + "\t"
                + "SoilSatR" + "\t" + "RFGrid" + "\t"
                + "RFUpMean" + "\t" + "FCData" + "\t"
                + "FCResStor" + "\n";
            int acvid = cvais[awp.wpColX][awp.wpRowY] + 1;
            string nFPN = ofs.ofpnWPs[acvid];
            appendTextToTextFile(nFPN, heads);
        }

        // ----------------------------------------------------
        // 이건 유량
        heads = comHeader
            + "Output data : Discharge[CMS]\n\n"
            + time_wpNames + "\t" + "Rainfall_Mean" + "\t" + "FromStarting[sec]" + "\n";
        appendTextToTextFile(ofs.ofpnDischarge, heads);

        // ----------------------------------------------------
        //// 이건 수심
        //heads = comHeader
        //    + "Output data : Depth[m]\n\n"
        //    + time_wpNames + "\t" + "Rainfall_Mean" + "\t" + "FromStarting[sec]" + "\n";
        //appendTextToTextFile(ofs.ofpnDepth, heads);

        // ----------------------------------------------------
        // 여기는 flow control 모듈 관련
        if (prj.simFlowControl == 1 && prj.fcs.size()>0)
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
            fcNameApp = "FC name :" ;
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
            for (int n : fccds.cvidsFCcell) {
                flowControlinfo afc = prj.fcs[n];
                fcDataField = fcDataField + "\t" + afc.fcName;
                fcNameApp = fcNameApp + "\t" + afc.fcName;
                fcTypeApp = fcTypeApp + "\t" + ENUM_TO_STR(afc.fcType);

                if (afc.fcType == flowControlType::ReservoirOutflow
                    || afc.fcType == flowControlType::Inlet
                    || afc.fcType== flowControlType::SinkFlow
                    || afc.fcType== flowControlType::SourceFlow) {
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
                if (afc.fcType == flowControlType::ReservoirOperation) {
                    if (afc.roType != reservoirOperationType::None) {
                        roType = roType + "\t" + ENUM_TO_STR(afc.roType);
                    }
                    else {
                        writeLog(fpnLog, "Reservoir operation type is invalid.\n", 1, 1);
                        return -1;
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
                    else {
                        writeLog(fpnLog, "Maximum reservoir storage or storage ratio is invalid.\n", 1, 1);
                        return -1;
                    }
                    if (afc.roType == reservoirOperationType::ConstantQ && afc.roConstQ_cms < 0) {
                        roConstQ = roConstQ + "\t" + to_string(afc.roConstQ_cms);
                        if (afc.roConstQDuration_hr > 0) {
                            roConstQduration = roConstQduration + "\t" + to_string(afc.roConstQDuration_hr);
                        }
                        else {
                            writeLog(fpnLog, "Constant reservoir outflow duration is invalid.\n", 1, 1);
                            return -1;
                        }
                    }

                    else {
                        writeLog(fpnLog, "Constant reservoir outflow is invalid.\n", 1, 1);
                        return -1;
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
                +  "Output data : Storage data[m^3]\n\n"
                + fcNameApp + "\n"
                + fcTypeApp + "\n"
                + roiniStorage + "\n"
                + romaxStorage + "\n"
                + romaxStorageRatio + "\n"
                + romaxStorageApp + "\n"
                + roType + "\n"
                +roConstQ + "\n"
                + roConstQduration + "\n"
                + fcDataField + "\n";
            appendTextToTextFile(ofs.ofpnFCStorage, heads);
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
    for (int wpcvid : wpis.wpCVIDs) {
        string fpn = ofs.ofpnWPs[wpcvid];
        if (fs::exists(fpn)) {
            std::remove(fpn.c_str());
        }
    }
    if (fs::exists(ppi.fpn_prj)) {
        std::remove(ppi.fpn_prj.c_str());
    }
    return 1;
}

inline double  getinterpolatedVLinear(double firstV, double nextV, double cinterp)
{
    double X;
    X = (nextV - firstV) * cinterp + firstV;
    return X;
}
