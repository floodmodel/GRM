// 이 문서의 코드는 실시간 시스템 구축시 보완, 유지관리. 2020.04.23. 최
#include "stdafx.h"
//#include <sqlext.h>

#include "gentle.h"
#include "realTime.h"

using namespace std;
extern fs::path fpnLog;

void add_Log_toDBMS(string strBasin, string strItem)
{// 이 함수 헤르메시스 작업 필요함 . 2020.04.23. 최   
    //try
    //{
    //    string strCon = g_strDBMSCnn;
    //    SqlConnection oSQLCon = new SqlConnection(strCon);
    //    oSQLCon.Open();
    //    string query = string.Format("insert into [GRM_ServerStatus](basin,[Memo]) values('{0}','{1}')", strBasin, strItem);
    //    SqlCommand SqlCommand = new SqlCommand(query, oSQLCon);
    //    SqlCommand.ExecuteNonQuery();
    //    oSQLCon.Close();
    //}
    //catch (int expn) {
    //    writeLog(fpnLog, to_string(expn), 1, 1);
    //}
}


void readyOleDBConnection()//OleDbConnection oOleDbConnection)
{// 이 함수 헤르메시스 작업 필요함. 2020.04.23. 최
    //oOleDbConnection.ResetState();
    //switch (oOleDbConnection.State)
    //{
    //case  ConnectionState.Open: {
    //    break;
    //}
    //case  ConnectionState.Closed:
    //case  ConnectionState.Connecting: {
    //    int intTry;
    //    int intTrayMAx = 3;
    //    for (intTry = 1; intTry <= intTrayMAx; intTry++) {// intTrayMAx회 시도. 오류시 1초 대기
    //        try {
    //            oOleDbConnection.Open();
    //            break;
    //        }
    //        catch (InvalidOperationException ex2) {
    //            // 무시.. lock 파일 해제 지연 등으로 가정하고 1초 지연
    //            Console.WriteLine(ex2.ToString());
    //            System.Threading.Thread.Sleep(1000);
    //        }
    //        catch (OleDbException ex1) {
    //            // 무시.. lock 파일 해제 지연 등으로 가정하고 1초 지연
    //            Console.WriteLine(ex1.ToString());
    //            System.Threading.Thread.Sleep(1000);
    //        }
    //        catch (Exception ex) {
    //            Console.WriteLine(ex.ToString());
    //        }
    //        GC.Collect();
    //    }
    //    if (intTry > intTrayMAx) {
    //        Console.WriteLine(string.Format("Connection to {0} : Try to Open {2}/3 Times, All Failed", oOleDbConnection.DataSource, oOleDbConnection.State, intTrayMAx));
    //    }
    //    else {
    //    }
    //    break;
    //}
    //default:
    //{
    //    Console.WriteLine(string.Format("Connection to {0} : {1} ", oOleDbConnection.DataSource, oOleDbConnection.State));
    //    break;
    //}
    //}
}


// 2018.8.8 이제 부터는 과거  run 도 보존 . 그래서 이 함수는 미사용됨.
void clear_DBMS_Table_Qwatershed(string strName)
{// 이 함수 헤르메시스 작업 필요함 . 2020.04.23. 최   
    //System.Data.SqlClient.SqlConnection oSQLCon = new System.Data.SqlClient.SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
    //if (oSQLCon.State == ConnectionState.Closed)
    //    oSQLCon.Open();
    //string strSQL = string.Format("delete [Q_CAL] where runid={0}", cRealTime_Common.g_performance_log_GUID);   // '2018.8.8 부터 임시 적용. 한시적
    //SqlCommand oSQLCMD = new SqlCommand(strSQL, oSQLCon);
    int intAffectedRecords=0;
    // intAffectedRecords = oSQLCMD.ExecuteNonQuery();
    string strMsg = "[Q_CAL] Table에서 " + strName + " 유역 " 
        + to_string(intAffectedRecords) + ":건 Data 삭제됨. 초기화 완료.";
    writeLog(fpnLog, strMsg, 1, 1);
    add_Log_toDBMS(strName, strMsg);
}