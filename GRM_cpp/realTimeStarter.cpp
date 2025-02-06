// �� ������ �ڵ�� �ְ� �ʾ� �ۼ�, �츣�޽ý����� ����. Ư�� dbms �κ�. 2020.04.23. ��
#include <stdio.h>
#include <io.h>
#include <string>
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern thisSimulation ts;
extern thisSimulationRT tsrt;

realTimeEnvFile rtef;

grmRealTime::grmRealTime(string fpn_REF, string strGUID,
    string dateTimeStart, string RTStartDateTime = "",
    string strModel = "")
{
    mFPN_RTEnv = fpn_REF;
    if (openRtEnvFile(mFPN_RTEnv) == -1) {
        cout << "Real time env. file [" << mFPN_RTEnv << "] is invalid." << endl;
        return;
    }
    if (RTStartDateTime != "") {// argument�� ���� ���۽ð��� ������,  �� ���� �̿�
        rtef.rtstartingTime = RTStartDateTime;
    }    
    ppi = getProjectFileInfo(rtef.fpnPrj);// ���⼭ gmp ���ϰ�� ����
    //GRMRT = realTime();
    // ���⼭ thisprocess rt ���� ���� ����. ====================
    tsrt.g_performance_log_GUID = strGUID;
    tsrt.g_dtStart_from_MonitorEXE = dateTimeStart;    
    tsrt.g_strModel = strModel; //LENS �ӻ�� ���..
    
    // tsrt.g_strTimeTagBase_KST = DateTime.ParseExact(RTStartDateTime, 
    //     "yyyyMMddHHmm", null).AddHours(-4).ToString("yyyyMMddHH");
    // tsrt.g_strTimeTagBase_UCT = DateTime.ParseExact(RTStartDateTime, 
    //      "yyyyMMddHHmm", null).AddHours(-9 - 4).ToString("yyyyMMddHH");

    // 2020.04.23. ��
    // ���⼭ argument�� ������ �ð� ������ yyyymmddHHMMSS �� �����Ѵ�.
    // RTStartDateTime �� ������ 2020-03-24 15:30 �ϰ�쿡�� 
    //      timeElaspedToDateTimeFormat2()�� ����Ѵ�.
    // �������� ����Ϸ��� dateTimeFormat:: ���� �ٲ� �ش�.
    int tsec = -4 * 60 * 60;
    tsrt.g_strTimeTagBase_KST = timeElaspedToDateTimeFormat(RTStartDateTime,
        tsec, timeUnitToShow::toHour, dateTimeFormat::yyyymmddHHMMSS);
    tsec = (-4 - 9) * 60 * 60;
    tsrt.g_strTimeTagBase_UCT = timeElaspedToDateTimeFormat(RTStartDateTime,
        tsec, timeUnitToShow::toHour, dateTimeFormat::yyyymmddHHMMSS);
    // thisprocess rt ���� ���� ��.====================
}

grmRealTime::~grmRealTime()
{
}

void grmRealTime::setUpAndStartGRMRT()
{
    grmRealTime::setupGRMandDBMSforRT();

    // rtef (ref ����)�� �̿��ؼ� GRM pars �� rt pars�� ������Ʈ �Ѵ�. =================
    // gmp ���ϰ� ref ������ �ٸ� ���, ref ���� ������ �Ű����� ����    
    prj.simType = simulationType::RealTime;
    prj.rfDataType = rainfallDataType::TextFileASCgrid;
    prj.isDateTimeFormat = true;
    prj.dtPrint_min = rtef.outputInterval_min;
    prj.simStartTime = rtef.rtstartingTime;
    prj.rfinterval_min = rtef.rfinterval_min;

    picWidth = CONST_PIC_WIDTH;
    picHeight = CONST_PIC_HEIGHT;

    runGRMRT();
}

void grmRealTime::setupGRMandDBMSforRT()
{
    setupGRMforRT();
    if (true) {
        // �̺κ� �ּ� ���ְ�, ���̻�� ���� �ʿ�. 2020.04.23. ��
        //System.Data.SqlClient.SqlConnection oSQLCon = new System.Data.SqlClient.SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
        //if (oSQLCon.State == ConnectionState.Closed) {
        //    oSQLCon.Open();
        //}
        //// monitor���� id �� �Ѱ��� �ʿ� ����.
        //string strGMPALL = readTextFileToString(ppi.fpn_prj);
        //SqlCommand oSQLCMD = new SqlCommand("INSERT INTO runmeta (runmeta, [who], gmp,run_starttime) VALUES (@runmeta, @who, @gmp,@run_starttime)", oSQLCon);
        //oSQLCMD.Parameters.AddWithValue("@runmeta", "���� ���� ��� ��� run �ߴ���..");
        //oSQLCMD.Parameters.AddWithValue("@who", "user1");
        //oSQLCMD.Parameters.AddWithValue("@gmp", strGMPALL);
        //oSQLCMD.Parameters.AddWithValue("@run_starttime", mSimulationStartingTime);

        //int intRetVal = oSQLCMD.ExecuteNonQuery();
        //if (intRetVal == 1) {
        //    DataTable dt1 = new DataTable();
        //    string strSQL1 = "select top 1 runid from runmeta order by runid desc";
        //    SqlDataAdapter oSqlDataAdapter = new SqlDataAdapter(strSQL1, cRealTime_DBMS.g_strDBMSCnn);
        //    oSqlDataAdapter.Fill(dt1);
        //    long lngID = (long)dt1.Rows[0][0];
        //    int intID = Convert.ToInt32(lngID); //int intID = dt1.Rows[0].Field<int>(0);    // by ice. 2018.11.21

        //    if (cRealTime_Common.g_performance_log_GUID == intID.ToString()) {
        //    }
        //    else {
        //        Console.WriteLine("Warning : If g_performance_log_GUID <> intID.ToString() Then");
        //    }
        //    cRealTime_DBMS.g_RunID = intID; // monitor ���� ������ ����. DB �켱 
        //    oSqlDataAdapter.Dispose();
        //}
        //else {
        //    Console.WriteLine("runmeta logging error");
        //}
        //oSQLCon.Close();
    }
}

void grmRealTime::setupGRMforRT() // fc �ڷ�� �׻� db�� ����ϴ� ������ ����, Optional FPNfcdata As String = "")
{
    string strTmp = readTextFileToString("C:\\Nakdong\\outputDrive.txt");
    if (upper(strTmp) != "C" & upper(strTmp) != "D" & upper(strTmp) != "S") {
        cout << "Can not Read " + strTmp << endl;
        return;
    }
    grmRealTime::CONST_Output_File_Target_DISK = strTmp.substr(0, 1).c_str()[0];

    if (openPrjAndSetupModel(1) == -1) {
        writeLog(fpnLog, "���� ���� ����.", 1, 1);
        if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
            //���� ���̻�� Ȯ�� �ʿ�. 2020.04.23. ��
            add_Log_toDBMS(ppi.fn_prj, "Fail in Model Setting");
        }
        return;
    }
    writeLog(fpnLog, "���� ���� �Ϸ�.", 1, 1);
    if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
        //���� ���̻�� Ȯ�� �ʿ�. 2020.04.23. ��
        add_Log_toDBMS(ppi.fn_prj, "Model Setting Completed.");
    }
}


void grmRealTime::runGRMRT()
{
    ts.setupGRMisNormal = 1;
    //if ( prj.makeASCorIMGfile == 1)    {
    //    mRTProject.mImgFPN_dist_Flow = new List<string>();
    //    mRTProject.mImgFPN_dist_RF = new List<string>();
    //    mRTProject.mImgFPN_dist_RFAcc = new List<string>();
    //    mRTProject.mImgFPN_dist_SSR = new List<string>();
    //    mbCreateDistributionFiles = true;
    //    mRasterFileOutput = new cRasterOutput(mRTProject);
    //}

    mSimDurationrRT_Hour = 24 * 100; // ����� 100�� ���� �����ϴ� ������ ����
    mbSimulationRTisOngoing = true;
    //mlstRFdataRT = new List<cRainfall.RainfallData>();
    mRFLayerCountToApply_RT = 0;
    if (CONST_bUseDBMS_FOR_RealTimeSystem) {
        if (false) {//'2018.8 ���� ���� ���� �м� ����� ������..�׷��� ���� code�� �̼���.
            clear_DBMS_Table_Qwatershed(ppi.fn_prj);
            writeLog(fpnLog, "DBMS [Q_CAL] Table Cleared", 1, 1);
            add_Log_toDBMS(ppi.fn_prj, "DBMS [Q_CAL] Table Cleared");
        }
    }

    writeLog(fpnLog, "RealTime runoff simulation was started..", 1, 1);
    if (CONST_bUseDBMS_FOR_RealTimeSystem) {
        add_Log_toDBMS(ppi.fn_prj, "RealTime Rainall Runoff Start..");
    }

    //mSimul = new cSimulator();
    //mSimul.MakeRasterOutput += new cSimulator.MakeRasterOutputEventHandler(makeRasterOutput);
    //if (CreateNewOutputFilesRT() == false)
    //    return;
    
    string fpn_rfASC;
    if (tsrt.g_strModel == "") {
        fpn_rfASC = rtef.rtRFfolderName + "/" 
            + getYYYYMMfromYYYYMMddHHmm(rtef. )  
            + ppi.fn_withoutExt_prj + CONST_TAG_DISCHARGE;
        //fpn_rfASC = mRfFilePathRT + @"\" + GetYearAndMonthFromyyyyMMddHHmm(mRFStartDateTimeRT) 
        // + @"\" + mRFStartDateTimeRT + ".asc";
    }
    else
    {
        //mRFStartDateTimeRT  ���� ����
        string strDIFF = (DateTime.ParseExact(mRFStartDateTimeRT, "yyyyMMddHHmm", null) - DateTime.ParseExact(cRealTime_Common.g_strTimeTagBase_KST, "yyyyMMddHH", null)).TotalHours.ToString("000");
        // ����.. ref�� ���� �ð���.. l030_v070_m00_h004.2016100400.gb2_1_clip.asc,,  �� ���߰�... h000�� ����...
        // �� KST�� ���� �ð� ������. 201610040900 + 4  �� 2016100413�� ��

        string strFileLEns = string.Format("l030_v070_{0}_h{1}.{2}.gb2_1_clip.asc", cRealTime_Common.g_strModel, strDIFF, cRealTime_Common.g_strTimeTagBase_UCT);
        fpn_rfASC = mRfFilePathRT + @"\" + strFileLEns ;
    }

    if (System.IO.File.Exists(fpn_rfASC) == false)
    {
        RTStatus("�����ؼ� ���� �ð������� �����ڷᰡ �����ϴ�.");
        RTStatus("�����ڷ�� �����ؼ� ���� �ð��� Ȯ���Ͻñ� �ٶ��ϴ�.");
    }

    if (mRTProject.fcGrid.IsSet == true)
    {
        mdicFCDataCountForEachCV = new Dictionary<int, int>();
        mdicFCNameForEachCV = new Dictionary<int, string>();
        mdicFCDataOrder = new Dictionary<int, int>();
        mdicBNewFCdataAddedRT = new Dictionary<int, bool>();
        foreach(int id in mRTProject.fcGrid.FCGridCVidList)
        {
            mdicFCDataCountForEachCV.Add(id, 0);
            mdicBNewFCdataAddedRT.Add(id, true);
            mdicFCDataOrder.Add(id, 0);
        }
        // If mGRMRT.mFPNFcData.Trim <> "" AndAlso IO.File.Exists(mGRMRT.mFPNFcData) = False Then
        // System.Console.WriteLine(String.Format("Realtime flow control data file is not valid. {0} {1}", vbCrLf, mGRMRT.mFPNFcData))
        // Exit Sub
        // End If
        mRTProject.fcGrid.mdtFCFlowData = new DataTable();
        ReadDBorCSVandMakeFCdataTableForRealTime_v2018(mRFStartDateTimeRT);
        if (mRTProject.fcGrid.mdtFCFlowData.Rows.Count < 1)
        {
            RTStatus("�����ؼ� ���� �ð������� flow control �ڷᰡ �����ϴ�.");
            RTStatus("�����ؼ� ���۽ð��� ������, inlet �ڷ� �� flow control �ڷḦ Ȯ���Ͻñ� �ٶ��ϴ�.");
        }
    }
    mSimul.SimulateRT(mRTProject, this, mIsPrediction);
}




int openRtEnvFile(string fpnref)
{
    vector<string> refFile;
    refFile = readTextFileToStringVector(ppi.fpn_prj);
    int LineCount = refFile.size();
    for (int i = 0; i < LineCount; ++i) {
        string aline = refFile[i];
        string vString = "";
        realTimeEnvFileFieldName fn;
        if (aline.find(fn.ProjectFPN) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.ProjectFPN);
            rtef.fpnPrj = "";
            if (vString != "" && _access(vString.c_str(), 0) == 0) {
                rtef.fpnPrj = vString;
            }
            else {
                writeLog(fpnLog, "GRM project file [" + vString + "] is invalid.\n", 1, 1);
                return -1;
            }
            continue;
        }
        if (aline.find(fn.RTRFfolderName) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.RTRFfolderName);
            rtef.rtRFfolderName = "";
            if (vString != "" && _access(vString.c_str(), 0) == 0) {
                rtef.rtRFfolderName = vString;
            }
            else {
                writeLog(fpnLog, "Real time rainfall data folder ["
                    + vString + "] is invalid.\n", 1, 1);
                return -1;
            }
            continue;
        }
        if (aline.find(fn.IsFC) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.IsFC);
            rtef.isFC = -1;
            if (lower(vString) == "true") {
                rtef.isFC = 1;
            }
            continue;
        }
        if (aline.find(fn.IsDWSExist) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.IsDWSExist);
            rtef.isDWSExist = -1;
            if (lower(vString) == "true") {
                rtef.isDWSExist = 1;
            }
            continue;
        }
        if (aline.find(fn.CWCellColXToConnectDW) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.CWCellColXToConnectDW);
            rtef.cwCellColXToConnectDW = -1;
            if (vString != "") {
                rtef.cwCellColXToConnectDW = stoi(vString);
            }
            continue;
        }
        if (aline.find(fn.CWCellRowYToConnectDW) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.CWCellRowYToConnectDW);
            rtef.cwCellRowYToConnectDW = -1;
            if (vString != "") {
                rtef.cwCellRowYToConnectDW = stoi(vString);
            }
            continue;
        }
        if (aline.find(fn.DWCellColXToConnectCW) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.DWCellColXToConnectCW);
            rtef.dwCellColXToConnectCW = -1;
            if (vString != "") {
                rtef.dwCellColXToConnectCW = stoi(vString);
            }
            continue;
        }
        if (aline.find(fn.DWCellRowYToConnectCW) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.DWCellRowYToConnectCW);
            rtef.dwCellRowYToConnectCW = -1;
            if (vString != "") {
                rtef.dwCellRowYToConnectCW = stoi(vString);
            }
            continue;
        }
        if (aline.find(fn.RFInterval_min) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.RFInterval_min);
            if (vString != "") {
                rtef.rfinterval_min = stoi(vString);
            }
            continue;
        }
        if (aline.find(fn.OutputInterval_min) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.OutputInterval_min);
            if (vString != "") {
                rtef.outputInterval_min = stoi(vString);
            }
            continue;
        }
        if (aline.find(fn.RTstartingTime) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.RTstartingTime);
            if (vString != "") {
                rtef.rtstartingTime = trim(vString);
            }
            continue;
        }
    }
}