#include <io.h>
#include <string>

#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern grmOutFiles ofs;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern int** cvais;
extern wpinfo wpis;
extern flowControlCellAndData fccds;

extern thisProcess tp;

int initOutputFiles()
{
	ofs.ofpnDischarge= ppi.fp_prj +"\\"+ ppi.fn_withoutExt_prj+CONST_TAG_DISCHARGE;
	ofs.ofpnDepth = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_DEPTH;
	ofs.ofpnRFGrid = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RFGRID;
	ofs.ofpnRFMean = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RFMEAN;
	//ofs.OFNPSwsPars = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_SWSPARSTEXTFILE;
	//ofs.OFNPFCData = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FCAPP;
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
        string outstr = "An error was occured while deleting previous ouput files or folders. Try starting the Model again. \n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (makeNewOutputFiles() == -1) {
        string outstr = "An error was occured while making ouput files or folders. Try starting the Model again. \n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
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
	//ofs.OFNPFCData = IO_Path_ChangeDrive(targetDisk, ofs.OFNPFCData);
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
            tp.setupGRMisNormal = -1;
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
    return -1;
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
            +" Built in "+ grmVersion.LastWrittenTime+".\n";
        comHeader = "Project name : " + ppi.fn_prj + " " + nowT 
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

            fcNameApp = "FCName:";
            fcTypeApp = "FCType:";
            sourceDT = "SourceDT[min]:";
            resOperation = "ResOperation:";
            roiniStorage = "StorageINI:";
            romaxStorage = "StorageMax:";
            romaxStorageRatio = "StorageMaxRatio:";
            romaxStorageApp = "StorageMaxApp:";
            roType = "ROType:";
            roConstQ = "ConstQ:";
            roConstQduration = "ConstQDuration:";
            string fcDataField = CONST_OUTPUT_TABLE_TIME_FIELD_NAME;
                for(int n :fccds.cvidsFCcell){
                    flowControlinfo afc = prj.fcs[n];
                    fcDataField = fcDataField + "\t" + afc.fcName;
                    fcNameApp = fcNameApp + "\t" + afc.fcName;
                    fcTypeApp = fcTypeApp + "\t" + ENUM_TO_STR(afc.fcType);
                if (row.ControlType == cFlowControl.FlowControlType.ReservoirOutflow.ToString() | row.ControlType == cFlowControl.FlowControlType.Inlet.ToString())
                {
                    strSourceDT = strSourceDT + "\t" + System.Convert.ToString(row.DT);
                    strResOperation = strResOperation + "\t" + "FALSE";
                }
                else
                {
                    strSourceDT = strSourceDT + "\t" + "NONE";
                    if (!row.IsMaxStorageNull() && !row.IsMaxStorageRNull())
                    {
                        double v;
                        if (double.TryParse(row.MaxStorage, out v) == false || double.TryParse(row.MaxStorageR, out v) == false ||
                            System.Convert.ToDouble(row.MaxStorage) * System.Convert.ToDouble(row.MaxStorageR) <= 0)

                        {
                            strResOperation = strResOperation + "\t" + "FALSE";
                        }
                        else
                        {
                            strResOperation = strResOperation + "\t" + "TRUE";
                        }
                    }
                }
                if (!row.IsIniStorageNull() && row.IniStorage != "")
                {
                    strROiniStorage = strROiniStorage + "\t" + row.IniStorage;
                }
                if (!row.IsMaxStorageNull() && row.MaxStorage != "")
                {
                    strROmaxStorage = strROmaxStorage + "\t" + row.MaxStorage;
                }
                if (!row.IsMaxStorageRNull() && row.MaxStorageR != "")
                {
                    strROmaxStorageRatio = strROmaxStorageRatio + "\t" + row.MaxStorageR;
                }
                if (!row.IsMaxStorageRNull() && row.MaxStorageR != "")
                {
                    strROmaxStorageApp = strROmaxStorageApp + "\t" +
                        System.Convert.ToString(System.Convert.ToDouble(row.MaxStorage) * System.Convert.ToDouble(row.MaxStorageR));
                }
                if (!row.IsROTypeNull() && row.ROType != "")
                {
                    strROType = strROType + "\t" + row.ROType;
                }
                if (!row.IsROConstQNull() && row.ROConstQ != "")
                {
                    strROConstQ = strROConstQ + "\t" + row.ROConstQ;
                }
                if (!row.IsROConstQDurationNull() && row.ROConstQDuration != "")
                {
                    strROConstQduration = strROConstQduration + "\t" + row.ROConstQDuration;
                }
            }

            // FCApp - flow control data
            System.IO.File.AppendAllText(strFNPFCData, strOutputCommonHeader, Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCData, "Output data : Flow control data input[CMS]" + "\r\n" + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCData, strNameFCApp + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCData, strTypeFCApp + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCData, strSourceDT + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCData, strResOperation + "\r\n" + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCData, strFCDataField + "\r\n", Encoding.Default);

            // reservoir operation
            System.IO.File.AppendAllText(strFNPFCStorage, strOutputCommonHeader, Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, "Output data : Storage data[m^3]" + "\r\n" + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strNameFCApp + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strTypeFCApp + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strROiniStorage + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strROmaxStorage + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strROmaxStorageRatio + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strROmaxStorageApp + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strROType + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strROConstQ + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strROConstQduration + "\r\n" + "\r\n", Encoding.Default);
            System.IO.File.AppendAllText(strFNPFCStorage, strFCDataField + "\r\n", Encoding.Default);
        }

    }
    return 1;
}
