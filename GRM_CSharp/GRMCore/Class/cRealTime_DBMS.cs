using System;
using System.Data.SqlClient;
using System.Data.OleDb;
using System.Data;

namespace GRMCore
{
    public class cRealTime_DBMS
    {
        // Friend Const g_strDBMSCnn As String = "data source=REALGRM\SQLEXPRESS;Initial catalog=hydroradar;Integrated Security=true"  '2016.7.26. 신규 azure 
        internal const string g_strDBMSCnn = @"data source=REALGRM\SQLEXPRESS;Initial catalog=RealTimeGRM;Integrated Security=true";  // 2018.8.
        internal static int g_RunID; // 2018.8.6 임시 추가
                                     // 

        public static void Add_Log_toDBMS(string strBasin, string strItem)
        {
            try
            {
                string strCon = g_strDBMSCnn;
                SqlConnection oSQLCon = new SqlConnection(strCon);
                oSQLCon.Open();
                string query = string.Format("insert into [GRM_ServerStatus](basin,[Memo]) values('{0}','{1}')", strBasin, strItem);
                SqlCommand SqlCommand = new SqlCommand(query, oSQLCon);
                SqlCommand.ExecuteNonQuery();
                oSQLCon.Close();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        public static void ReadyOleDBConnection(OleDbConnection oOleDbConnection)
        {
            oOleDbConnection.ResetState();
            switch (oOleDbConnection.State)
            {
                case  ConnectionState.Open:
                    {
                        break;
                    }

                case  ConnectionState.Closed:
                case  ConnectionState.Connecting:
                    {
                        int intTry;
                        int intTrayMAx = 3;
                        for (intTry = 1; intTry <= intTrayMAx; intTry++) // intTrayMAx회 시도. 오류시 1초 대기
                        {
                            try
                            {
                                oOleDbConnection.Open();
                                break;
                            }
                            catch (InvalidOperationException ex2)
                            {
                                // 무시.. lock 파일 해제 지연 등으로 가정하고 1초 지연
                                Console.WriteLine(ex2.ToString());
                                System.Threading.Thread.Sleep(1000);
                            }
                            catch (OleDbException ex1)
                            {
                                // 무시.. lock 파일 해제 지연 등으로 가정하고 1초 지연
                                Console.WriteLine(ex1.ToString());
                                System.Threading.Thread.Sleep(1000);
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.ToString());
                            }
                            GC.Collect();
                        }
                        if (intTry > intTrayMAx)
                            Console.WriteLine(string.Format("Connection to {0} : Try to Open {2}/3 Times, All Failed", oOleDbConnection.DataSource, oOleDbConnection.State, intTrayMAx));
                        else
                        {
                        }

                        break;
                    }

                default:
                    {
                        Console.WriteLine(string.Format("Connection to {0} : {1} ", oOleDbConnection.DataSource, oOleDbConnection.State));
                        break;
                    }
            }
        }
    }
}
