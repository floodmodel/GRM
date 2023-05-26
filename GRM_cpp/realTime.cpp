// 이 문서의 코드는 최가 초안 작성, 실시간 시스템 구축시 보완. 특히 dbms 부분. 2020.04.23. 최
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
    
    if (rtStartDataTime != "") {// argument로 받은 시작시간이 있으면,  그 값을 이용
        rtef.rtstartDataTime = rtStartDataTime;
    }    
    ppi = getProjectFileInfo(rtef.fpnPrj);// 여기서 gmp 파일경로 설정
    // 여기서 thisprocess rt 변수 설정 시작. ====================
    tsrt.g_performance_log_GUID = strGUID;
    tsrt.g_dtStart_from_MonitorEXE = startCommandTime;    
    tsrt.g_strModel = nameLensModel; //LENS 앙상블 고려..
    tsrt.Output_File_Target_DISK = targetDisk.substr(0, 1).c_str()[0];

    // 2020.04.23. 최
    // 여기서 argument로 들어오는 시간 포맷은 yyyymmddHHMMSS 을 가정한다.
    // RTStartDateTime 의 포맷이 2020-03-24 15:30 일경우에는 
    //      timeElaspedToDateTimeFormat2()를 사용한다.
    // 다른 포맷으로 사용하려면 dateTimeFormat:: 에서 바꿔 준다.
    int tsec = -4 * 60 * 60;
    tsrt.g_strTimeTagBase_KST = timeElaspedToDateTimeFormat(rtef.rtstartDataTime,
        tsec, timeUnitToShow::toH, dateTimeFormat::yyyymmddHHMMSS);
    tsec = (-4 - 9) * 60 * 60;
    tsrt.g_strTimeTagBase_UCT = timeElaspedToDateTimeFormat(rtef.rtstartDataTime,
        tsec, timeUnitToShow::toH, dateTimeFormat::yyyymmddHHMMSS);

    tsrt.simDurationrRT_h = 24 * 365; // 충분히 365일 동안 모의하는 것으로 설정
    // thisprocess rt 변수 설정 끝.====================
}

grmRealTime::~grmRealTime()
{
}

void grmRealTime::setUpAndStartGRMRT()
{
    grmRealTime::setupGRMforRT();
    grmRealTime::setupDBMSforRT();

    // rtef (ref 파일)을 이용해서 GRM pars 와 rt pars를 업데이트 한다. =================
    // gmp 파일과 ref 파일이 다른 경우, ref 파일 값으로 매개변수 설정    
    prj.simType = simulationType::RealTime;
    //prj.rfDataType = rainfallDataType::TextFileASCgrid;
    prj.isDateTimeFormat = true;
    prj.dtPrint_min = rtef.outputInterval_min;
    prj.simStartTime = rtef.rtstartDataTime;
    prj.rfinterval_min = rtef.rfinterval_min;

    rfs.clear();// 사용할 강우자료 초기화

    ts.simDuration_min = tsrt.simDurationrRT_h * 60;
    ts.enforceFCautoROM = tsrt.enforceAutoROM;
    ts.dataNumTotal_rf = 0;

    picWidth = CONST_PIC_WIDTH;
    picHeight = CONST_PIC_HEIGHT;

    grmRealTime::runGRMRT();
}

void grmRealTime::setupGRMforRT() // fc 자료는 항상 db를 사용하는 것으로 수정, Optional FPNfcdata As String = "")
{

    if (openPrjAndSetupModel(1) == -1) {
        writeLog(fpnLog, "모형 설정 실패.\n", 1, 1);
        if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
            //여기 원이사님 확인 필요. 2020.04.23. 최
            add_Log_toDBMS(ppi.fn_prj, "Fail in Model Setting");
        }
        return;
    }
    writeLog(fpnLog, "모형 설정 완료.\n", 1, 1);
    if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
        //여기 원이사님 확인 필요. 2020.04.23. 최
        add_Log_toDBMS(ppi.fn_prj, "Model Setting Completed.");
    }
}

void grmRealTime::setupDBMSforRT()
{
    if (true) {
        // 이부분 주석 없애고, 원이사님 검토 필요. 2020.04.23. 최
        //System.Data.SqlClient.SqlConnection oSQLCon = new System.Data.SqlClient.SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
        //if (oSQLCon.State == ConnectionState.Closed) {
        //    oSQLCon.Open();
        //}
        //// monitor에서 id 를 넘겨줄 필요 없다.
        //string strGMPALL = readTextFileToString(ppi.fpn_prj);
        //SqlCommand oSQLCMD = new SqlCommand("INSERT INTO runmeta (runmeta, [who], gmp,run_starttime) VALUES (@runmeta, @who, @gmp,@run_starttime)", oSQLCon);
        //oSQLCMD.Parameters.AddWithValue("@runmeta", "누가 언제 어디서 어떻게 run 했는지..");
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
        //    cRealTime_DBMS.g_RunID = intID; // monitor 에서 받은거 보다. DB 우선 
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
        if (false) {//'2018.8 부터 이제 과거 분석 기록은 보존됨..그래서 삭제 code는 미수행.
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

    // 시작 시간에서의 자료 확인 하지 말자.. 2020.04.29. 최
    //if (_access(fpn_rfASC.c_str(), 0) != 0) {
    //    writeLog(fpnLog, "실시간 유출해석 시작 시간에서의 강우자료가 없습니다.\n", 1, 1);
    //    writeLog(fpnLog, "모의 시작 강우자료 : "+fpn_rfASC+"\n", 1, 1);
    //    writeLog(fpnLog, "강우자료와 유출해석 시작 시간을 확인하시길 바랍니다.\n", 1, 1);
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
    //                "] 지점에서 유출해석 시작 시간에서의 flow control 자료가 없습니다.\n", 1, 1);
    //            writeLog(fpnLog,
    //                "유출해석 시작시간과 댐방류량, inlet 자료 등 flow control 자료를 확인하시길 바랍니다.\n", 1, 1);
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
    // 이부분 원이사님 검토 필요.. 2020.04.25. 최 ===========================
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
    // 안사용하면 삭제 필요. 2020.04.25. 최
    ////경천DAM 처리.영주댐 처리 부분 인데... 너무 지엽적인 예외 조치 라서... 배제하는시도를 2018.10.12 원,안 하고 있슴
    //if (false) {
    //    string strSpcealDams = "'경천댐','영주댐'";       // 2018.8.29 원 : 여기서 n 개 기입... 이건 추후 DB 등으로 이동되어야 함..  2018.10/11 원 : 영주댐 추가
    //    string strSQL2 = string.Format("Select  w.name, 999 as cvid ,[Time] as datetime ,[QValue] AS VALUE From QStream_OBS_ht d , WatchPoint w  Where  d.GName in({1}) and  TIME ='{0}' and d.Gname=w.Gname ", TargetDateTime, strSpcealDams);
    //    System.Data.DataTable odt2 = new System.Data.DataTable();
    //    SqlDataAdapter oSqlDataAdapter2 = new SqlDataAdapter(strSQL2, cRealTime_DBMS.g_strDBMSCnn);
    //    oSqlDataAdapter2.SelectCommand.CommandTimeout = 60;
    //    oSqlDataAdapter2.Fill(odt2);

    //    if (odt2.Rows.Count != 2) {  // Stop   '2018.10.11 까지는 stop 이었슴
    //        writeLog(fpnLog, strSpcealDams + " 의 data가 2건이 아님!", 1, 1);
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
{// 모든 cvidx 에 대해서 자료를 읽는다..
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
        // cvs 파일은 fc_name, DataTime, Value 순서이다.
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
        // 출력 포맷이 필요한가? 필요하면 여기서.. 2020.04.29. 최
        //stringstream ss_fcname;
        //ss_fcname << right << setw(13) << fcname;
        //stringstream ss_idx;
        //ss_idx << left << setw(6) << idx;
        //stringstream ss_value;
        //ss_value << left << setw(8) << value;
        //msg = "  FC Data 입력완료 ("
        //    + ss_fcname.str() + " [cvidx="+to_string(idx)+"], "
        //    + t_yyyymmddHHMM            
        //    + ", Value=" + ss_value.str() + ").\n";
        msg = "  FC data 입력완료 ("
            + fcname + " [cvidx=" + to_string(idx) + "], "
            + t_yyyymmddHHMM
            + ", Value=" + dtos(value,2) + ").\n";
    }
    else {
        tsrt.newFcDataAddedRT[idx] = -1;
        msg = "    FC data 수신 대기 중 (" + fcname + " [cvidx=" + to_string(idx) + "], "
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
        writeLog(fpnLog, "  Rainfall data ("+arf.value + ") 분석완료. " 
            + tFromStart + "분 경과 \n", 1, 1);
        tsrt.newRFAddedRT = -1;
    }
    //string fpn_map = "";
    switch (prj.rfDataType)
    {
    //case weatherDataType::ASCraster_mmPhr:
    //    writeLog(fpnLog, "[TextFileASCgrid_mmPhr] rainfall data  type is not supported yet.\n", 1, 1);
    //    return;
    case weatherDataType::Mean: {
        // map는 지정된 폴더(RTRFfolderName)의 yyyymm.txt 파일에서 실시간 강우자료를 받는다.
        // RTRFfolderName\\yyyymm.txt
        // 데이터는 csv 형식이고, DataTime(yyyymmddHHMM), Value 순서이다.
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
            // cvs 파일은 DataTime, Value 순서이다.
            if (sv_aline[0] == t_yyyymmddHHMM) {
                ts.dataNumTotal_rf += 1;
                int rfOrder = ts.dataNumTotal_rf;
                weatherData nrf;
                nrf.Order = rfOrder;
                nrf.DataTime = t_yyyymmddHHMM;
                nrf.value = sv_aline[1]; // 
                nrf.FileName = fpn_map;
                rfs.push_back(nrf);
                writeLog(fpnLog, "  Rainfall data 입력완료 ("+ nrf.value +
                    " for "+ t_yyyymmddHHMM+" from "+ fpn_map + ").\n", 1, 1);
                tsrt.newRFAddedRT = 1;
                return;
            }
        }
        break;
    }
    case weatherDataType::Raster_ASC: {
        string rfFileName;    // 2018년 8.8 현재 산출 naming
        string ascFPN;        // 2018년 8.8 현재 산출 naming
        if (tsrt.g_strModel == "") {
            // 강우파일의 시간 스트링 앞뒤의 문자는 ref 파일에서 받는다.
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

        //LENS 인 경우 yyyymm 폴더 구분 하지 않음
            if (tsrt.g_strModel == "") {
                nrf.FilePath = rtef.fpRTRFfolder + "\\"
                    + getYYYYMMfromYYYYMMddHHmm(t_yyyymmddHHMM);
            }
            else {
                nrf.FilePath = rtef.fpRTRFfolder;
            }

            nrf.FileName = rfFileName;
            rfs.push_back(nrf);
            writeLog(fpnLog, "  Rainfall data 입력완료 ("+rfFileName + ").\n", 1, 1);
            tsrt.newRFAddedRT = 1;
            return;
        }
        break;
    }
    }
    cout << "    Rainfall data 수신 대기 중 (" + t_yyyymmddHHMM + ")... \n";
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
    // 가정.. ref의 시작 시간을.. l030_v070_m00_h004.2016100400.gb2_1_clip.asc,,  에 맞추고... h000만 조정...
    // 즉 KST로 시작 시간 지정은. 201610040900 + 4  즉 2016100413이 됨
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