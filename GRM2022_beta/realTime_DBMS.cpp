// �� ������ �ڵ�� �ǽð� �ý��� ����� ����, ��������. 2020.04.23. ��
#include "stdafx.h"
//#include <sqlext.h>

#include "gentle.h"
#include "realTime.h"

using namespace std;
extern fs::path fpnLog;

void add_Log_toDBMS(string strBasin, string strItem)
{// �� �Լ� �츣�޽ý� �۾� �ʿ��� . 2020.04.23. ��   
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
{// �� �Լ� �츣�޽ý� �۾� �ʿ���. 2020.04.23. ��
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
    //    for (intTry = 1; intTry <= intTrayMAx; intTry++) {// intTrayMAxȸ �õ�. ������ 1�� ���
    //        try {
    //            oOleDbConnection.Open();
    //            break;
    //        }
    //        catch (InvalidOperationException ex2) {
    //            // ����.. lock ���� ���� ���� ������ �����ϰ� 1�� ����
    //            Console.WriteLine(ex2.ToString());
    //            System.Threading.Thread.Sleep(1000);
    //        }
    //        catch (OleDbException ex1) {
    //            // ����.. lock ���� ���� ���� ������ �����ϰ� 1�� ����
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


// 2018.8.8 ���� ���ʹ� ����  run �� ���� . �׷��� �� �Լ��� �̻���.
void clear_DBMS_Table_Qwatershed(string strName)
{// �� �Լ� �츣�޽ý� �۾� �ʿ��� . 2020.04.23. ��   
    //System.Data.SqlClient.SqlConnection oSQLCon = new System.Data.SqlClient.SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
    //if (oSQLCon.State == ConnectionState.Closed)
    //    oSQLCon.Open();
    //string strSQL = string.Format("delete [Q_CAL] where runid={0}", cRealTime_Common.g_performance_log_GUID);   // '2018.8.8 ���� �ӽ� ����. �ѽ���
    //SqlCommand oSQLCMD = new SqlCommand(strSQL, oSQLCon);
    int intAffectedRecords=0;
    // intAffectedRecords = oSQLCMD.ExecuteNonQuery();
    string strMsg = "[Q_CAL] Table���� " + strName + " ���� " 
        + to_string(intAffectedRecords) + ":�� Data ������. �ʱ�ȭ �Ϸ�.";
    writeLog(fpnLog, strMsg, 1, 1);
    add_Log_toDBMS(strName, strMsg);
}