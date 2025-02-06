#pragma once
#include <stdio.h>

#include "grm.h"

using namespace std;

const bool CONST_bUseDBMS_FOR_RealTimeSystem =false;
const bool CONST_bWriteTextFileOutput_FOR_RealTimeSystem = true;
const string g_strDBMSCnn = 
"data source=REALGRM\\SQLEXPRESS;Initial catalog=RealTimeGRM;Integrated Security=true";  // 2018.8.

//const bool isPrediction = false;//2019.10.01. ��. prediction ����


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
    int cwCellColXToConnectDW = -1; // �� ���� ���ؼ��� wp�� �����Ǿ, ���ǰ���� ����ؾ� �Ѵ�.
    int cwCellRowYToConnectDW = -1; // �� ���� ���ؼ��� wp�� �����Ǿ, ���ǰ���� ����ؾ� �Ѵ�.
    int dwCellColXToConnectCW = -1; // �Ϸ� ������ �ִ� �� �������� fc�� �����Ǿ, ����� ���� ����� ���� �� �־�� �Ѵ�.
    int dwCellRowYToConnectCW = -1; // �Ϸ� ������ �ִ� �� �������� fc�� �����Ǿ, ����� ���� ����� ���� �� �־�� �Ѵ�.
    int rfinterval_min = 0;
    int outputInterval_min = 0;
    string rtstartDataTime = "";   // yyyymmddHHMM, 201209160000
    string headText_BeforeTString_RFN = "";
    string tailText_AfterTString_RFN_withExt = "";// ����Ȯ���� ���Ե� ���ڿ�
} realTimeEnvFile;

typedef struct _thisSimulationRT // real time ����
{
    //COleDateTime g_RT_tStart_from_MonitorEXE;
    string g_performance_log_GUID="";         // ���� ���� ��� �� 
    string g_dtStart_from_MonitorEXE=""; // ���� ���� ��� �� 
    string g_strModel="";  //MODEL ���п� 2019.4.12   .LENS�� m00~m12
    string g_strTimeTagBase_UCT="";       //l030_v070_m00_h004.2016100300.gb2_1_clip.asc �� ��� 2016100300
    string g_strTimeTagBase_KST="";        // ����� ��� 2016100300+9 �� 2016100309��.
    int g_RunID=-1; // 2018.8.6 �ӽ� �߰�
    char Output_File_Target_DISK='?'; // png ���� ���� ����� c ,.d  ��� ��������. ���� 
    
    int simDurationrRT_h = 0;
    int  enforceAutoROM = -1;//2019.10.01. ��. prediction ����
    int newRFAddedRT = -1; //1: true, -1 : false
    map<int, int> newFcDataAddedRT;   // idx �� ����, //1: true, -1 : false
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
   map <int, int> mbFCDataOrder; // idx �� ����
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
    int cvidx); // added=1 �̸� ���ο� ������ �Էµ� ��
void updateRFdataGRMRT(string t_yyyymmddHHMM);
void writeRealTimeSimResults(string tStrToPrint, double cinterp,
	double tsFromStarting_sec);

// ����� realTime_DBMS.cpp �� �ִ� �Լ���
void add_Log_toDBMS(string strBasin, string strItem);
void clear_DBMS_Table_Qwatershed(string strName);
void readyOleDBConnection();// OleDbConnection oOleDbConnection);
