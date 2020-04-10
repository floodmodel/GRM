#pragma once
#include <stdio.h>

#include "grm.h"

using namespace std;

const bool CONST_bUseDBMS_FOR_RealTimeSystem = true;

typedef struct _realtimeCommon
{
	std::string g_performance_log_GUID;         // 성능 측정 기록 용 
	clock_t g_dtStart_from_MonitorEXE; // 성능 측정 기록 용 
	string g_strModel;  //MODEL 구분용 2019.4.12   .LENS는 m00~m12
	string g_strTimeTagBase_UCT;       //l030_v070_m00_h004.2016100300.gb2_1_clip.asc 의 경우 2016100300
	string g_strTimeTagBase_KST;        // 상기의 경우 2016100300+9 즉 2016100309임.
} realtimeCommon;


class cRealTime
{
private:
	//private cSimulator mSimul = new cSimulator();
	//private DataTable m_odt_flowcontrolinfo;
	//private bool mIsPrediction = false;//2019.10.01. 최. prediction 관련
    static cRealTime mGRMRT;
public:
    static char CONST_Output_File_Target_DISK;// = '?'; // png 등의 모의 결과를 c ,.d  어디에 기입할지. 구분 
    //event RTStatusEventHandler RTStatus;

    //public delegate void RTStatusEventHandler(string strMSG);
    //projectFile  mRTProject;
    rainfallDataType mRainfallDataTypeRT;
    string mRfFilePathRT="";
    int mDtPrintOutRT_min=-1;
    int mSimDurationrRT_Hour=-1;
    string mRFStartDateTimeRT="";
    bool mbNewRFAddedRT=false;
    bool mIsPrediction = false;//2019.10.01. 최. prediction 관련
    //public Dictionary<int, bool> mdicBNewFCdataAddedRT;   // idx 로 구분
    vector<rainfallData> mlstRFdataRT;
    /// <summary>
    /// Get data count by cv idx
    /// </summary>
    /// <remarks></remarks>
    //public Dictionary<int, int> mdicFCDataCountForEachCV;
    //public Dictionary<int, string> mdicFCNameForEachCV;
    //public Dictionary<int, int> mdicFCDataOrder;
    int mRFLayerCountToApply_RT=-1;
    clock_t mDateTimeStartRT=clock();
    bool mbSimulationRTisOngoing=false;

    double mPicWidth= 0.0;
    double mPicHeight = 0.0;
    bool mbCreateDistributionFiles = false;
    //cRasterOutput mRasterFileOutput;

    bool mbIsDWSS=false;
    int mCWCellColX_ToConnectDW=-1;
    int mCWCellRowY_ToConnectDW = -1;
    int mDWCellColX_ToConnectCW = -1;
    int mDWCellRowY_ToConnectCW = -1;

	cRealTime();
    static void InitializeGRMRT();
    void SetupGRM(string fpn_prj, bool isPrediction);

	~cRealTime();
};


int changeOutputFileDisk(char targetDisk);
string IO_Path_ChangeDrive(char strV, string strPath);

void writeDBRealTime(int nowTmin, double cinterp);

