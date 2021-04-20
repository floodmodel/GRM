#include "stdafx.h"
#include<ATLComTime.h>

#include "grm.h"
#include "realTime.h"

extern projectfilePathInfo ppi;
extern projectFile prj;
extern fs::path fpnLog;
extern cvAtt* cvs;
extern wpinfo wpis;
extern grmOutFiles ofs;
extern flowControlCellAndData fccds;
extern thisSimulation ts;
extern thisSimulationRT tsrt;

extern cvAtt* cvsb;

void writeRealTimeSimResults(int nowTmin, double cinterp)
{
    COleDateTime timeNow;
    double tsFromStarting_sec;
    string tStrToPrint;
    timeNow = COleDateTime::GetCurrentTime();
    COleDateTimeSpan tsTotalSim = timeNow - ts.time_thisSimStarted;
    tsFromStarting_sec = tsTotalSim.GetTotalSeconds();
    tStrToPrint = timeElaspedToDateTimeFormat(prj.simStartTime,
        nowTmin * 60, timeUnitToShow::toM,
        dateTimeFormat::yyyymmddHHMMSS);
    string strWPName;
    string strFNP;
    string vToP = "";
    double Qobs_cms;
    string strSQL_Server = "";       // SQL DB 에도 추가적으로 기입
    // real time은 wp 별 출력만 한다. discharge.out은 출력하지 않는다.
    for (int i : wpis.wpCVidxes) {
        if (cinterp == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = dtos(cvs[i].QOF_m3Ps, 2);
            }
            else {
                vToP = dtos(cvs[i].stream.QCH_m3Ps, 2);
            }
        }
        else if (ts.isbak == 1) {
            if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                vToP = dtos(getinterpolatedVLinear(cvsb[i].QOF_m3Ps,
                    cvs[i].QOF_m3Ps, cinterp), 2);
            }
            else {
                vToP = dtos(getinterpolatedVLinear(cvsb[i].stream.QCH_m3Ps,
                    cvs[i].stream.QCH_m3Ps, cinterp), 2);
            }
        }
        // 여기서 관측자료 받는다.. 직접 받을 수 있는 경우를 대비해서, 자리만 만들어줌..
        Qobs_cms = 0;
        strWPName = wpis.wpNames[i];
        if (CONST_bWriteTextFileOutput_FOR_RealTimeSystem == true) {
            string strOutPutLine;
            // 출력 순서는 시간, 모의유량, 관측유량, 강우, 시간
            strOutPutLine = tStrToPrint
                + "\t" + vToP + "\t" + dtos(Qobs_cms, 2)
                + "\t" + dtos(wpis.rfUpWSAveForDtP_mm[i], 2)
                + "\t" + dtos(tsFromStarting_sec / 60.0, 2) + "\n";
            appendTextToTextFile(ofs.ofpnWPs[i], strOutPutLine);
        }

        // 사용하지 않으면 삭제 필요. 원이사님 검토 필요. 2020.04.29. 최. 
        Log_Performance_data(ppi.fn_withoutExt_prj, strWPName, tStrToPrint,
            tsFromStarting_sec / 60.0); // 성능 비교 분석용 정보 수집

        // 이부분 원이사님 확인 및 수정 필요. 2020. 03. 29. 최
        if (CONST_bUseDBMS_FOR_RealTimeSystem == true) {
            //SqlConnection oSQLCon = new SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
            //if (oSQLCon.State == ConnectionState.Closed) {
            //    oSQLCon.Open();
            //}
            //int intPos = strWPName.find("_", 0);
            //string strGaugeCode = strWPName.substr(intPos + 1);
            //string strSQL = "insert into [Q_CAL] (runid,WPName, [Time],[value], RFMean_mm, model) values("
            //    + to_string(tsrt.g_RunID) + ", '"
            //    + strWPName + "', '"
            //    + tStrToPrint + "', "
            //    + toStrWithPrecision(wpis.rfUpWSAveForDtP_mm[i], 2) + ", "
            //    + vToP + ", '" + tsrt.g_strModel + "')";
            ///*string strSQL = string.Format("insert into [Q_CAL] (runid,WPName, [Time],[value], RFMean_mm, model) values({0},'{1}','{2}',{3},{4},'{5}')",
            //    tsrt.g_RunID, strWPName, tStrToPrint, vToP,
            //    wpis.rfUpWSAveForDtP_mm[i], tsrt.g_strModel);*/
            //SqlCommand oSQLCMD = new SqlCommand(strSQL, oSQLCon);
            //int intRetVal = oSQLCMD.ExecuteNonQuery();
            //if (intRetVal != 1) {
            //    writeLog(fpnLog, strSQL + "\r\n" + ppi.fp_prj, 1, 1);
            //    throw new ApplicationException("Error : " + strSQL);
            //}
            //oSQLCon.Close();
        }
    }
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