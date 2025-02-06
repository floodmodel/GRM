#pragma once
#include <stdio.h>

#include "grm.h"

using namespace std;

const bool CONST_bUseDBMS_FOR_RealTimeSystem =false;
const bool CONST_bWriteTextFileOutput_FOR_RealTimeSystem = true;
const string g_strDBMSCnn = 
"data source=REALGRM\\SQLEXPRESS;Initial catalog=RealTimeGRM;Integrated Security=true";  // 2018.8.

//const bool isPrediction = false;//2019.10.01. 최. prediction 관련


typedef struct _realTimeEnvFileFieldName
{
    const string ProjectFPN = "ProjectFPN";
    const string RTPRCPfolderName = "RTPRCPfolderName";
    const string PRCPfileText_BeforeTString = "PRCPfileText_BeforeTString";
    const string PRCPfileText_AfterTStringWithExt = "PRCPfileText_AfterTStringWithExt"; 
    const string IsFC = "IsFC";
    const string RTFCdataFPN = "RTFCdataFPN";
    const string IsDWSExist = "IsDWSExist";
    const string CWCellColXToConnectDW = "CWCellColXToConnectDW";
    const string CWCellRowYToConnectDW = "CWCellRowYToConnectDW";
    const string DWCellColXToConnectCW = "DWCellColXToConnectCW";
    const string DWCellRowYToConnectCW = "DWCellRowYToConnectCW";
    const string PRCPInterval_min = "PRCPInterval_min";
    const string OutputInterval_min = "OutputInterval_min";
    const string RTstartingTime = "RTstartingTime";
} realTimeEnvFileFieldName;

typedef struct _realTimeEnvFile
{
    string	fpnPrj = "";
    string fpRTRFfolder = "";
    string fpnRTFCdata = "";
    int isFC = 0; // 1:true, -1:false
    int isDWSExist = 0;// 1:true, -1:false
    int cwCellColXToConnectDW = -1; // 이 셀에 대해서는 wp가 지정되어서, 모의결과를 출력해야 한다.
    int cwCellRowYToConnectDW = -1; // 이 셀에 대해서는 wp가 지정되어서, 모의결과를 출력해야 한다.
    int dwCellColXToConnectCW = -1; // 하류 유역에 있는 이 셀에서는 fc가 지정되어서, 상류의 모의 결과를 받을 수 있어야 한다.
    int dwCellRowYToConnectCW = -1; // 하류 유역에 있는 이 셀에서는 fc가 지정되어서, 상류의 모의 결과를 받을 수 있어야 한다.
    int rfinterval_min = 0;
    int outputInterval_min = 0;
    string rtstartDataTime = "";   // yyyymmddHHMM, 201209160000
    string headText_BeforeTString_RFN = "";
    string tailText_AfterTString_RFN_withExt = "";// 파일확장자 포함된 문자열
} realTimeEnvFile;

typedef struct _thisSimulationRT // real time 전용
{
    //COleDateTime g_RT_tStart_from_MonitorEXE;
    string g_performance_log_GUID="";         // 성능 측정 기록 용 
    string g_dtStart_from_MonitorEXE=""; // 성능 측정 기록 용 
    string g_strModel="";  //MODEL 구분용 2019.4.12   .LENS는 m00~m12
    string g_strTimeTagBase_UCT="";       //l030_v070_m00_h004.2016100300.gb2_1_clip.asc 의 경우 2016100300
    string g_strTimeTagBase_KST="";        // 상기의 경우 2016100300+9 즉 2016100309임.
    int g_RunID=-1; // 2018.8.6 임시 추가
    char Output_File_Target_DISK='?'; // png 등의 모의 결과를 c ,.d  어디에 기입할지. 구분 
    
    int simDurationrRT_h = 0;
    int  enforceAutoROM = -1;//2019.10.01. 최. prediction 관련
    int newRFAddedRT = -1; //1: true, -1 : false
    map<int, int> newFcDataAddedRT;   // idx 로 구분, //1: true, -1 : false
    //int rfDataCountToApply_RT = -1;

} thisSimulationRT;

class grmRealTime {
private:
    projectFile RTProject;
    const int CONST_PIC_WIDTH = 1024;
    const int CONST_PIC_HEIGHT = 768;
    string mFPN_RTEnv;

public:    
    vector<weatherData> rfsForRT;
   map <int, int> mbFCDataOrder; // idx 로 구분
   bool mbSimulationRTisOngoing = false;

    double picWidth = 0.0;
    double picHeight = 0.0;
    bool mbCreateDistributionFiles = false;
    //cRasterOutput mRasterFileOutput;

    grmRealTime(string fpn_REF, string strGUID="1",
        string startCommandTime="", string rtStartDataTime = "",
        string nameLensModel = "");
    ~grmRealTime();
    void setupGRMforRT();
    void setupDBMSforRT();
    void setUpAndStartGRMRT();
    void runGRMRT();
};

inline string IO_Path_ChangeDrive(char strV, string strPath)
{
    if (strPath.substr(1, 1) != ":") { return "-1"; }
    return strV + strPath.substr(1);
}

int changeOutputFileDisk(char targetDisk);
string getLENSrfFPNusingTimeString(string t_yyyymmddHHMM);
int  grmRTLauncher(int argc, char** args, int isPrediction);
int openRtEnvFile(string fpnref);
int readCSVandFillFCdataForRealTime(string fpnFCcvs, 
    string targetDateTime);
int readDBandFillFCdataForRealTime(string targetDateTime);

int startSimulationRT();

void updateFcDataStatusForEachFCcellGRMRT(string t_yyyymmddHHMM,
    int cvidx); // added=1 이면 새로운 데이터 입력된 것
void updateRFdataGRMRT(string t_yyyymmddHHMM);
void writeRealTimeSimResults(string tStrToPrint, double cinterp,
	double tsFromStarting_sec);

// 여기는 realTime_DBMS.cpp 에 있는 함수들
void add_Log_toDBMS(string strBasin, string strItem);
void clear_DBMS_Table_Qwatershed(string strName);
void readyOleDBConnection();// OleDbConnection oOleDbConnection);
