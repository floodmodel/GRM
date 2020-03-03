#include <stdio.h>

#include"gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

cRealTime::cRealTime()
{
	CONST_Output_File_Target_DISK = '?';
    mDateTimeStartRT = clock();
}

void  cRealTime::InitializeGRMRT()
{
    string strTmp = readTextFileToString("C:\\Nakdong\\outputDrive.txt");
    if (toUpper(strTmp) != "C" &  toUpper(strTmp) != "D" & toUpper(strTmp) != "S")    {
        cout<<"Can not Read " + strTmp<<endl;
        return;
    }
    CONST_Output_File_Target_DISK = strTmp.substr(0, 1).c_str()[0];
    cRealTime mGRMRT;
    //mGRMRT.RTStatus += new RTStatusEventHandler(mGRMRT.cRealTime_RTStatus);
}

cRealTime::~cRealTime()
{

}


void cRealTime::SetupGRM(string fpn_prj, bool isPrediction) // fc 자료는 항상 db를 사용하는 것으로 수정, Optional FPNfcdata As String = "")
{
    ppi = getProjectFileInfo(fpn_prj);
    if (openPrjAndSetupModel(1) == -1)
    {
        //RTStatus("모형 설정 실패.");
        //if (CONST_bUseDBMS_FOR_RealTimeSystem)
        //    cRealTime_DBMS.Add_Log_toDBMS(System.IO.Path.GetFileName(FPNprj), "Fail in Model Setting"); //  '모형 설정 실패.  2018.11.20 문구 수정함
        return;
    }
    //mRTProject = prj;
    //mRTProject.simType = simulationType::RealTime;
    prj.simType= simulationType::RealTime;
    mIsPrediction = isPrediction;
    //if (mRTProject.SetupModelParametersAfterProjectFileWasOpened() == false)
    //{
    //    writeLog(fpnLog,"GRM setup was failed !!!", 1, 1);
    //    return;
    //}
    //if (cOutPutControl.CreateNewOutputFiles(cProject.Current, true) == false)
    //    cGRM.writelogAndConsole("Deleting single event output files were failed !!!", true, true);

    //RTStatus("모형 설정 완료.");
    //if (CONST_bUseDBMS_FOR_RealTimeSystem)
    //    cRealTime_DBMS.Add_Log_toDBMS(mRTProject.ProjectNameOnly, "Model Setting Completed."); //'모형 설정 완료. 2018.11.20 문구 수정
}


string IO_Path_ChangeDrive(char strV, string strPath)
{
    if (strPath.substr(1, 1) != ":") {
        return "-1";
    }
    return strV + strPath.substr(1);
}