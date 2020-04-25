#include <io.h>
#include <string>
#include<ATLComTime.h>

#include "grm.h"
#include "realTime.h"

extern projectfilePathInfo ppi;
extern projectFile prj;
extern cvAtt* cvs;
extern wpinfo wpis;
extern flowControlCellAndData fccds;
extern thisSimulation ts;

extern cvAtt* cvsb;
extern map<int, double> fcdAb;
extern wpinfo wpisb;;

void writeDBRealTime(int nowTmin, double cinterp)
{
    COleDateTime timeNow;
    double tsFromStarting_sec;
    string tStrToPrint;
    timeNow = COleDateTime::GetCurrentTime();
    COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
    tsFromStarting_sec = tsTotalSim.GetTotalSeconds();
    tStrToPrint = timeElaspedToDateTimeFormat2(prj.simStartTime,
        nowTmin * 60, timeUnitToShow::toMinute, 
        dateTimeFormat::yyyy_mm_dd__HHcolMMcolSS);
    string strWPName;
    string strFNP;
    string vToP = "";
    double Qobs_cms;
    string strOutPutLine;
    string strSQL_Server = "";       // SQL DB 에도 추가적으로 기입
    for (int i : wpis.wpCVidxes) {
        if (cinterp == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = toStrWithPrecision(cvs[i].QOF_m3Ps, 2);
            }
            else {
                vToP = toStrWithPrecision(cvs[i].stream.QCH_m3Ps, 2);
            }
        }
        else if (ts.isbak == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = toStrWithPrecision(getinterpolatedVLinear(cvsb[i].QOF_m3Ps,
                    cvs[i].QOF_m3Ps, cinterp), 2);
            }
            else {
                vToP = toStrWithPrecision(getinterpolatedVLinear(cvsb[i].stream.QCH_m3Ps,
                    cvs[i].stream.QCH_m3Ps, cinterp), 2);
            }
        }
        // 여기서 관측자료 받는다.. 직접 받을 수 있는 경우를 대비해서, 자리만 만들어줌..
        Qobs_cms = 0;
        strWPName = wpis.wpNames[i];

        //// 실시간 모델링 텍스트 파일 쓰지 않는다. 2020.03.29. 최
        //strFNP = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "RealTime_" + strWPName + ".out";
        //strFNP = IO_Path_ChangeDrive(cRealTime::CONST_Output_File_Target_DISK, strFNP);
        //strOutPutLine = tStrToPrint
        //    + "\t" + forString(wpis.rfUpWSAveForDtPrint_mm[cvid], 2)
        //    + "\t" + forString(Qobs_cms, 2) + "\t" + vToP
        //    + "\t" + forString(tsFromStarting_sec / 60.0, 2) + "\n";
        //if (fs::exists(ppi.fp_prj) == false) {
        //    fs::create_directories(ppi.fp_prj);
        //}
        //appendTextToTextFile(strFNP, strOutPutLine);

        Log_Performance_data(ppi.fn_withoutExt_prj, strWPName, tStrToPrint,
            tsFromStarting_sec / 60.0); // 성능 비교 분석용 정보 수집

        //// 이부분 원이사님 확인 및 수정 필요. 2020. 03. 29. 최
        //if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
        //    SqlConnection oSQLCon = new SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
        //    if (oSQLCon.State == ConnectionState.Closed) {
        //        oSQLCon.Open();
        //    }
        //    int intPos = strWPName.find("_", 0);
        //    string strGaugeCode = strWPName.substr(intPos + 1);
        //    string strSQL = string.Format("insert into [Q_CAL] (runid,WPName, [Time],[value], RFMean_mm, model) values({0},'{1}','{2}',{3},{4},'{5}')",
        //        cRealTime_DBMS.g_RunID, strWPName, cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut), vToP, 
        //        project.watchPoint.RFUpWsMeanForDtPrintout_mm[row.CVID], cRealTime_Common.g_strModel);
        //    SqlCommand oSQLCMD = new SqlCommand(strSQL, oSQLCon);
        //    int intRetVal = oSQLCMD.ExecuteNonQuery();
        //    if (intRetVal != 1)
        //    {
        //        System.Console.WriteLine(strSQL + "\r\n" + project.ProjectPath);
        //        throw new ApplicationException("Error : " + strSQL);
        //    }
        //    oSQLCon.Close();
        //}

    }
}


void Log_Performance_data(string strBasin, string strTag, string strDataTime, double dblElapTime)
{
    /// 이부분 원이사님 확인, 수정 필요. 2020.03.29. 최
    //// 성능 비교 용도. 개별 프로세스. launcher .exe 에 집중. 머신 전체는 monitor에서 측정하도록함
    //SqlConnection oSQLCon = new SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
    //if (oSQLCon.State == ConnectionState.Closed)    {
    //    oSQLCon.Open();
    //}
    //string strSQL = string.Format("insert into run_perf ([basin],[RainfallDataCompleted],[ElapsedTime_Min],[net_process_PrivateMemorySize64],[run_meta_guid],[OutputDrive],[tag]) values('{0}','{1}',{2},{3},'{4}','{5}','{6}')",
    //    strBasin, strDataTime, dblElapTime, Process.GetCurrentProcess().PrivateMemorySize64 / (double)1024 / 1024 / 1024, cRealTime_DBMS.g_RunID, cRealTime.CONST_Output_File_Target_DISK, strTag);
    //System.Data.SqlClient.SqlCommand oSQLCMD = new System.Data.SqlClient.SqlCommand(strSQL, oSQLCon);
    //int intRetVal = oSQLCMD.ExecuteNonQuery();
    //if (intRetVal != 1)    {
    //    Console.WriteLine("perf logging error");
    //}
    //oSQLCon.Close();
}