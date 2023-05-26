// �� ������ �ڵ�� �ְ� �ʾ� �ۼ�, �ǽð� �ý��� ����� ����. Ư�� dbms �κ�. 2020.04.23. ��
#include "stdafx.h"
#include<ATLComTime.h>
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern flowControlCellAndData fccds;
extern vector<weatherData> rfs;

extern thisSimulation ts;
extern thisSimulationRT tsrt;

realTimeEnvFile rtef;

grmRealTime::grmRealTime(string fpn_REF, string strGUID,
    string startCommandTime, string rtStartDataTime,
    string nameLensModel)
{
    mFPN_RTEnv = fpn_REF;
    if (openRtEnvFile(mFPN_RTEnv) == -1) {
        cout << "Real time env. file [" << mFPN_RTEnv << "] is invalid." << endl;
        return;
    }
    string targetDisk = readTextFileToString("C:\\Nakdong\\outputDrive.txt");
    string tdU = upper(targetDisk.substr(0, 1));
    if (tdU != "C" & tdU != "D" & tdU != "S") {
        cout << "Can not Read " + targetDisk << endl;
        return;
    }
    
    if (rtStartDataTime != "") {// argument�� ���� ���۽ð��� ������,  �� ���� �̿�
        rtef.rtstartDataTime = rtStartDataTime;
    }    
    ppi = getProjectFileInfo(rtef.fpnPrj);// ���⼭ gmp ���ϰ�� ����
    // ���⼭ thisprocess rt ���� ���� ����. ====================
    tsrt.g_performance_log_GUID = strGUID;
    tsrt.g_dtStart_from_MonitorEXE = startCommandTime;    
    tsrt.g_strModel = nameLensModel; //LENS �ӻ�� ���..
    tsrt.Output_File_Target_DISK = targetDisk.substr(0, 1).c_str()[0];

    // 2020.04.23. ��
    // ���⼭ argument�� ������ �ð� ������ yyyymmddHHMMSS �� �����Ѵ�.
    // RTStartDateTime �� ������ 2020-03-24 15:30 �ϰ�쿡�� 
    //      timeElaspedToDateTimeFormat2()�� ����Ѵ�.
    // �ٸ� �������� ����Ϸ��� dateTimeFormat:: ���� �ٲ� �ش�.
    int tsec = -4 * 60 * 60;
    tsrt.g_strTimeTagBase_KST = timeElaspedToDateTimeFormat(rtef.rtstartDataTime,
        tsec, timeUnitToShow::toH, dateTimeFormat::yyyymmddHHMMSS);
    tsec = (-4 - 9) * 60 * 60;
    tsrt.g_strTimeTagBase_UCT = timeElaspedToDateTimeFormat(rtef.rtstartDataTime,
        tsec, timeUnitToShow::toH, dateTimeFormat::yyyymmddHHMMSS);

    tsrt.simDurationrRT_h = 24 * 365; // ����� 365�� ���� �����ϴ� ������ ����
    // thisprocess rt ���� ���� ��.====================
}

grmRealTime::~grmRealTime()
{
}

void grmRealTime::setUpAndStartGRMRT()
{
    grmRealTime::setupGRMforRT();
    grmRealTime::setupDBMSforRT();

    // rtef (ref ����)�� �̿��ؼ� GRM pars �� rt pars�� ������Ʈ �Ѵ�. =================
    // gmp ���ϰ� ref ������ �ٸ� ���, ref ���� ������ �Ű����� ����    
    prj.simType = simulationType::RealTime;
    //prj.rfDataType = rainfallDataType::TextFileASCgrid;
    prj.isDateTimeFormat = true;
    prj.dtPrint_min = rtef.outputInterval_min;
    prj.simStartTime = rtef.rtstartDataTime;
    prj.rfinterval_min = rtef.rfinterval_min;

    rfs.clear();// ����� �����ڷ� �ʱ�ȭ

    ts.simDuration_min = tsrt.simDurationrRT_h * 60;
    ts.enforceFCautoROM = tsrt.enforceAutoROM;
    ts.dataNumTotal_rf = 0;

    picWidth = CONST_PIC_WIDTH;
    picHeight = CONST_PIC_HEIGHT;

    grmRealTime::runGRMRT();
}

void grmRealTime::setupGRMforRT() // fc �ڷ�� �׻� db�� ����ϴ� ������ ����, Optional FPNfcdata As String = "")
{

    if (openPrjAndSetupModel(1) == -1) {
        writeLog(fpnLog, "���� ���� ����.\n", 1, 1);
        if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
            //���� ���̻�� Ȯ�� �ʿ�. 2020.04.23. ��
            add_Log_toDBMS(ppi.fn_prj, "Fail in Model Setting");
        }
        return;
    }
    writeLog(fpnLog, "���� ���� �Ϸ�.\n", 1, 1);
    if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
        //���� ���̻�� Ȯ�� �ʿ�. 2020.04.23. ��
        add_Log_toDBMS(ppi.fn_prj, "Model Setting Completed.");
    }
}

void grmRealTime::setupDBMSforRT()
{
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


void grmRealTime::runGRMRT()
{
    mbSimulationRTisOngoing = true;
    if (CONST_bUseDBMS_FOR_RealTimeSystem) {
        if (false) {//'2018.8 ���� ���� ���� �м� ����� ������..�׷��� ���� code�� �̼���.
            clear_DBMS_Table_Qwatershed(ppi.fn_prj);
            writeLog(fpnLog, "DBMS [Q_CAL] Table Cleared\n", 1, 1);
            add_Log_toDBMS(ppi.fn_prj, "DBMS [Q_CAL] Table Cleared");
        }
    }
    writeLog(fpnLog, "RealTime runoff simulation was started.\n", 1, 1);
    if (CONST_bUseDBMS_FOR_RealTimeSystem) {
        add_Log_toDBMS(ppi.fn_prj, "RealTime Rainall Runoff Start..");
    }

    string fpn_rfASC;
    if (tsrt.g_strModel == "") {
        fpn_rfASC = rtef.fpRTRFfolder + "\\"
            + getYYYYMMfromYYYYMMddHHmm(rtef.rtstartDataTime)
            + "\\" + rtef.rtstartDataTime + ".asc";
    }
    else {
        fpn_rfASC= getLENSrfFPNusingTimeString(rtef.rtstartDataTime);
    }
    fccds.inputFlowData_m3Ps.clear();

    // ���� �ð������� �ڷ� Ȯ�� ���� ����.. 2020.04.29. ��
    //if (_access(fpn_rfASC.c_str(), 0) != 0) {
    //    writeLog(fpnLog, "�ǽð� �����ؼ� ���� �ð������� �����ڷᰡ �����ϴ�.\n", 1, 1);
    //    writeLog(fpnLog, "���� ���� �����ڷ� : "+fpn_rfASC+"\n", 1, 1);
    //    writeLog(fpnLog, "�����ڷ�� �����ؼ� ���� �ð��� Ȯ���Ͻñ� �ٶ��ϴ�.\n", 1, 1);
    //}
    //if (prj.applyFC == 1) {
    //    for (int idx : fccds.cvidxsFCcell) {
    //        tsrt.newFcDataAddedRT.clear();
    //        mbFCDataOrder[idx] = 0;
    //    }
    //    if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
    //        if (readDBandFillFCdataForRealTime(rtef.rtstartDataTime) == -1) {
    //            return;
    //        }
    //    }
    //    else {
    //        if (readCSVandFillFCdataForRealTime(rtef.fpnRTFCdata,
    //            rtef.rtstartDataTime) == -1) {
    //            return;
    //        }
    //    }
    //    for (int idx : fccds.cvidxsFCcell) {
    //        if (fccds.flowData_m3Ps[idx].size() < 1) {
    //            writeLog(fpnLog, "[" + prj.fcs[idx].fcName +
    //                "] �������� �����ؼ� ���� �ð������� flow control �ڷᰡ �����ϴ�.\n", 1, 1);
    //            writeLog(fpnLog,
    //                "�����ؼ� ���۽ð��� ������, inlet �ڷ� �� flow control �ڷḦ Ȯ���Ͻñ� �ٶ��ϴ�.\n", 1, 1);
    //            return;
    //        }
    //    }
    //}
    writeLog(fpnLog, "Real time simulation was started. Predicion = "
        + to_string(tsrt.enforceAutoROM)+"\n", 1, 1);
    if (startSimulationRT() == -1) {
        writeLog(fpnLog, "Real time simulation error.\n", 1, 1);
    }
    return;    
}


int readDBandFillFCdataForRealTime(string targetDateTime)
{
    // �̺κ� ���̻�� ���� �ʿ�.. 2020.04.25. �� ===========================
    //string strSQL;
    //strSQL = "Select  w.name, 999 as cvid ,[Time] as datetime ,[QValue] AS VALUE  From [QDam_OBS] d , WatchPoint w Where d.Gname=w.Gname and w.fc=1 And TIME ="
    //    + targetDateTime;
    //System.Data.DataTable odt = new System.Data.DataTable();
    //SqlDataAdapter oSqlDataAdapter = new SqlDataAdapter(strSQL, cRealTime_DBMS.g_strDBMSCnn);
    //oSqlDataAdapter.SelectCommand.CommandTimeout = 60;
    //oSqlDataAdapter.Fill(odt);
    //timeSeries ts;
    //foreach(DataRow oDR in odt.Rows) {
    //    int cvidx = getCVidxByFcName(oDR.Field<string>("NAME"));
    //    ts.dataTime = odt.Field("datetime");
    //    ts.value = stod(odt.Field("QValue"));
    //    fccds.flowData_m3Ps[cvidx].push_back(ts);
    //    tsrt.newFcDataAddedRT[cvidx] = 1;
    //}
    //===========================================


    //===========================================
    // �Ȼ���ϸ� ���� �ʿ�. 2020.04.25. ��
    ////��õDAM ó��.���ִ� ó�� �κ� �ε�... �ʹ� �������� ���� ��ġ ��... �����ϴ½õ��� 2018.10.12 ��,�� �ϰ� �ֽ�
    //if (false) {
    //    string strSpcealDams = "'��õ��','���ִ�'";       // 2018.8.29 �� : ���⼭ n �� ����... �̰� ���� DB ������ �̵��Ǿ�� ��..  2018.10/11 �� : ���ִ� �߰�
    //    string strSQL2 = string.Format("Select  w.name, 999 as cvid ,[Time] as datetime ,[QValue] AS VALUE From QStream_OBS_ht d , WatchPoint w  Where  d.GName in({1}) and  TIME ='{0}' and d.Gname=w.Gname ", TargetDateTime, strSpcealDams);
    //    System.Data.DataTable odt2 = new System.Data.DataTable();
    //    SqlDataAdapter oSqlDataAdapter2 = new SqlDataAdapter(strSQL2, cRealTime_DBMS.g_strDBMSCnn);
    //    oSqlDataAdapter2.SelectCommand.CommandTimeout = 60;
    //    oSqlDataAdapter2.Fill(odt2);

    //    if (odt2.Rows.Count != 2) {  // Stop   '2018.10.11 ������ stop �̾���
    //        writeLog(fpnLog, strSpcealDams + " �� data�� 2���� �ƴ�!", 1, 1);
    //    }
    //    foreach(DataRow oDR2 in odt2.Rows) {
    //        DataRow oDR_Target2 = mRTProject.fcGrid.mdtFCGridInfo.Select(string.Format("Name='{0}'", oDR2["NAME"].ToString())).FirstOrDefault();
    //        string strCVID2 = oDR_Target2["CVID"].ToString();
    //        oDR2["CVID"] = strCVID2;
    //    }
    //    dt.Merge(odt2);
    //}
    //cProject.Current.fcGrid.mdtFCFlowData = dt;
    //===========================================
    return 1;
}

int readCSVandFillFCdataForRealTime(string fpnFCcvs, string targetDateTime)
{// ��� cvidx �� ���ؼ� �ڷḦ �д´�..
    int intL = 0;
    if (fpnFCcvs != "" && _access(fpnFCcvs.c_str(), 0) != 0) {
        string outstr = "Flow control data file [" + fpnFCcvs
            + "] is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    vector<string> sv;
    sv = readTextFileToStringVector(fpnFCcvs);
    for (int i = 0; i < sv.size(); ++i) {
        vector<string> sv_aline = splitToStringVector(sv[i], ',');
        // cvs ������ fc_name, DataTime, Value �����̴�.
        if (sv_aline[1] == targetDateTime) {
            timeSeries ts;          
            int cvidx = getCVidxByFcName(sv_aline[0]);;
            ts.dataTime = sv_aline[1];
            ts.value = stod(sv_aline[2]);
            fccds.inputFlowData_m3Ps[cvidx].push_back(ts);
            tsrt.newFcDataAddedRT[cvidx] = 1;
        }
    }
	return 1;
}

void updateFcDataStatusForEachFCcellGRMRT(string t_yyyymmddHHMM, int idx)
{
    string fcname = prj.fcs[idx].fcName;
    string msg = "";
    double value = 0;;
    vector<timeSeries> QsFC = fccds.inputFlowData_m3Ps[idx];
    int vsize = QsFC.size();
    int added = -1;
    for (timeSeries ats : QsFC) {
        if (ats.dataTime == t_yyyymmddHHMM) {
            value = ats.value;
            added = 1;
            break;
        }
    }
    if (added == 1) {
        tsrt.newFcDataAddedRT[idx] = 1;
        // ��� ������ �ʿ��Ѱ�? �ʿ��ϸ� ���⼭.. 2020.04.29. ��
        //stringstream ss_fcname;
        //ss_fcname << right << setw(13) << fcname;
        //stringstream ss_idx;
        //ss_idx << left << setw(6) << idx;
        //stringstream ss_value;
        //ss_value << left << setw(8) << value;
        //msg = "  FC Data �Է¿Ϸ� ("
        //    + ss_fcname.str() + " [cvidx="+to_string(idx)+"], "
        //    + t_yyyymmddHHMM            
        //    + ", Value=" + ss_value.str() + ").\n";
        msg = "  FC data �Է¿Ϸ� ("
            + fcname + " [cvidx=" + to_string(idx) + "], "
            + t_yyyymmddHHMM
            + ", Value=" + dtos(value,2) + ").\n";
    }
    else {
        tsrt.newFcDataAddedRT[idx] = -1;
        msg = "    FC data ���� ��� �� (" + fcname + " [cvidx=" + to_string(idx) + "], "
            + t_yyyymmddHHMM + ", Q_cms)...\n";
    }
    cout << msg;
}


void updateRFdataGRMRT(string t_yyyymmddHHMM)
{
    COleDateTime timeNow;
    timeNow = COleDateTime::GetCurrentTime();
    COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
    string tFromStart;
    tFromStart = dtos(tsTotalSim.GetTotalMinutes(), 2);
    if (tsrt.newRFAddedRT == 1 && ts.dataNumTotal_rf > 0) {
        weatherData arf;
        arf = rfs[ts.dataNumTotal_rf-1];
        writeLog(fpnLog, "  Rainfall data ("+arf.value + ") �м��Ϸ�. " 
            + tFromStart + "�� ��� \n", 1, 1);
        tsrt.newRFAddedRT = -1;
    }
    //string fpn_map = "";
    switch (prj.rfDataType)
    {
    //case weatherDataType::ASCraster_mmPhr:
    //    writeLog(fpnLog, "[TextFileASCgrid_mmPhr] rainfall data  type is not supported yet.\n", 1, 1);
    //    return;
    case weatherDataType::Mean: {
        // map�� ������ ����(RTRFfolderName)�� yyyymm.txt ���Ͽ��� �ǽð� �����ڷḦ �޴´�.
        // RTRFfolderName\\yyyymm.txt
        // �����ʹ� csv �����̰�, DataTime(yyyymmddHHMM), Value �����̴�.
        string fpn_map;
        string rfFileName;
        rfFileName = rtef.headText_BeforeTString_RFN
            + getYYYYMMfromYYYYMMddHHmm(t_yyyymmddHHMM)
            + rtef.tailText_AfterTString_RFN_withExt;
        fpn_map = rtef.fpRTRFfolder + "\\"+ rfFileName;
        if (_access(fpn_map.c_str(), 0) != 0) {
            break;
        }
        vector<string> sv;
        sv = readTextFileToStringVector(fpn_map);
        for (int i = 0; i < sv.size(); ++i) {
            vector<string> sv_aline = splitToStringVector(sv[i], ',');
            // cvs ������ DataTime, Value �����̴�.
            if (sv_aline[0] == t_yyyymmddHHMM) {
                ts.dataNumTotal_rf += 1;
                int rfOrder = ts.dataNumTotal_rf;
                weatherData nrf;
                nrf.Order = rfOrder;
                nrf.DataTime = t_yyyymmddHHMM;
                nrf.value = sv_aline[1]; // 
                nrf.FileName = fpn_map;
                rfs.push_back(nrf);
                writeLog(fpnLog, "  Rainfall data �Է¿Ϸ� ("+ nrf.value +
                    " for "+ t_yyyymmddHHMM+" from "+ fpn_map + ").\n", 1, 1);
                tsrt.newRFAddedRT = 1;
                return;
            }
        }
        break;
    }
    case weatherDataType::Raster_ASC: {
        string rfFileName;    // 2018�� 8.8 ���� ���� naming
        string ascFPN;        // 2018�� 8.8 ���� ���� naming
        if (tsrt.g_strModel == "") {
            // ���������� �ð� ��Ʈ�� �յ��� ���ڴ� ref ���Ͽ��� �޴´�.
            rfFileName =  rtef.headText_BeforeTString_RFN
                + t_yyyymmddHHMM + rtef.tailText_AfterTString_RFN_withExt;
            ascFPN = rtef.fpRTRFfolder + "\\"
                + getYYYYMMfromYYYYMMddHHmm(t_yyyymmddHHMM)
                + "\\" + rfFileName;
        }
        else {
            ascFPN = getLENSrfFPNusingTimeString(t_yyyymmddHHMM);
            fs::path fpn_arf = fs::path(ascFPN.c_str());
            rfFileName = fpn_arf.filename().string();
        }

        if (_access(ascFPN.c_str(), 0) == 0) {
            ts.dataNumTotal_rf += 1;
            int rfOrder = ts.dataNumTotal_rf;
            weatherData nrf;
            nrf.Order = rfOrder;
            nrf.DataTime = t_yyyymmddHHMM;
            nrf.value = rfFileName; // 

        //LENS �� ��� yyyymm ���� ���� ���� ����
            if (tsrt.g_strModel == "") {
                nrf.FilePath = rtef.fpRTRFfolder + "\\"
                    + getYYYYMMfromYYYYMMddHHmm(t_yyyymmddHHMM);
            }
            else {
                nrf.FilePath = rtef.fpRTRFfolder;
            }

            nrf.FileName = rfFileName;
            rfs.push_back(nrf);
            writeLog(fpnLog, "  Rainfall data �Է¿Ϸ� ("+rfFileName + ").\n", 1, 1);
            tsrt.newRFAddedRT = 1;
            return;
        }
        break;
    }
    }
    cout << "    Rainfall data ���� ��� �� (" + t_yyyymmddHHMM + ")... \n";
}

string getLENSrfFPNusingTimeString(string t_yyyymmddHHMM)
{
    tm tmTarget = stringToDateTime(t_yyyymmddHHMM);
    tm tmB = stringToDateTime(tsrt.g_strTimeTagBase_KST);
    COleDateTime oleTstart_KST;
    COleDateTime oleTBase_KST;
    oleTstart_KST.SetDateTime(tmTarget.tm_year, tmTarget.tm_mon, tmTarget.tm_mday,
        tmTarget.tm_hour, tmTarget.tm_min, 0);
    oleTBase_KST.SetDateTime(tmB.tm_year, tmB.tm_mon, tmB.tm_mday,
        tmB.tm_hour, tmB.tm_min, 0);
    COleDateTimeSpan tspan = oleTstart_KST - oleTBase_KST;
    stringstream ss;
    ss << setw(3) << std::setprecision(0) << tspan.GetTotalHours();
    string strDIFF = ss.str();
    if (strDIFF == "073") { 
        writeLog(fpnLog, "LENS : completed",1,1); 
        exit(0); 
    }
    // ����.. ref�� ���� �ð���.. l030_v070_m00_h004.2016100400.gb2_1_clip.asc,,  �� ���߰�... h000�� ����...
    // �� KST�� ���� �ð� ������. 201610040900 + 4  �� 2016100413�� ��
    string strFileLEns = "l030_v070_" + tsrt.g_strModel +
        "_h" + strDIFF + "." + tsrt.g_strTimeTagBase_UCT + ".gb2_1_clip.asc";
    string fpn_rfASC = rtef.fpRTRFfolder + "\\" + strFileLEns;
    return fpn_rfASC;
}




int openRtEnvFile(string fpnref)
{
    vector<string> refFile;
    refFile = readTextFileToStringVector(fpnref);
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
        if (aline.find(fn.RTPRCPfolderName) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.RTPRCPfolderName);
            rtef.fpRTRFfolder = "";
            if (vString != "" && _access(vString.c_str(), 0) == 0) {
                rtef.fpRTRFfolder = vString;
            }
            else {
                writeLog(fpnLog, "Real time rainfall data folder ["
                    + vString + "] is invalid.\n", 1, 1);
                return -1;
            }
            continue;
        }
        if (aline.find(fn.PRCPfileText_BeforeTString) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.PRCPfileText_BeforeTString);
            rtef.headText_BeforeTString_RFN = trim(vString);
            continue;
        }
        if (aline.find(fn.PRCPfileText_AfterTStringWithExt) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.PRCPfileText_AfterTStringWithExt);
            rtef.tailText_AfterTString_RFN_withExt = trim(vString);
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
        if (aline.find(fn.RTFCdataFPN) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.RTFCdataFPN);
            rtef.fpnRTFCdata = "";
            if (vString != "" && _access(vString.c_str(), 0) == 0) {
                rtef.fpnRTFCdata = vString;
            }
            else if(rtef.isFC==1){
                writeLog(fpnLog, "Real time flow control data file ["
                    + vString + "] is invalid.\n", 1, 1);
                return -1;
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
        if (aline.find(fn.PRCPInterval_min) != string::npos) {
            vString = getValueStringFromXmlLine(aline, fn.PRCPInterval_min);
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
                rtef.rtstartDataTime = trim(vString);
            }
            continue;
        }
    }
	return 1;
}