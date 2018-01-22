Imports System.Data.SqlClient
Module mDBMS

    'Friend Const g_strDBMSCnn As String = "data source=TEST-GRM\SQLEXPRESS;Initial catalog=hydroradar;Integrated Security=true"
    Friend Const g_strDBMSCnn As String = "data source=REALGRM\SQLEXPRESS;Initial catalog=hydroradar;Integrated Security=true"  '2016.7.26 원 : 신규 azure 

    'If g_bUseDBMS_for_RealTimeSystem then call Add_Log_toDBMS()
    Sub Add_Log_toDBMS(strBasin As String, strItem As String)
        Try
            Dim strCon As String = g_strDBMSCnn
            Dim oSQLCon As New SqlConnection(strCon)
            oSQLCon.Open()
            Dim query As String = String.Format("insert into [GRM_ServerStatus](basin,[Memo]) values('{0}','{1}')", strBasin, strItem)
            Dim SqlCommand As New SqlCommand(query, oSQLCon)
            SqlCommand.ExecuteNonQuery()
            oSQLCon.Close()
        Catch ex As Exception
            Console.WriteLine(ex.ToString)
            'Console.ReadKey()
        End Try
    End Sub

    Sub ReadyOleDBConnection(oOleDbConnection As OleDbConnection)   '2015.10.21 원영진 추가
        oOleDbConnection.ResetState()
        Select Case oOleDbConnection.State
            Case ConnectionState.Open
                'ok
            Case ConnectionState.Closed, ConnectionState.Connecting
                Dim intTry As Integer
                Dim intTrayMAx As Integer = 3
                For intTry = 1 To intTrayMAx 'intTrayMAx회 시도. 오류시 1초 대기
                    Try
                        oOleDbConnection.Open() '2015.10.22 원 : .ldb 파일의 존재 여부와 exception은 직접적 관련은 없어 보임
                        Exit For
                    Catch ex2 As InvalidOperationException         '연결이 닫혀 있지 않습니다. 현재 연결은 열린 상태입니다.  ,, ms access 로 열고 있는 경우 등.
                        '무시.. lock 파일 해제 지연 등으로 가정하고 1초 지연
                        System.Threading.Thread.Sleep(1000)
                    Catch ex1 As OleDbException
                        '무시.. lock 파일 해제 지연 등으로 가정하고 1초 지연
                        System.Threading.Thread.Sleep(1000)
                    Catch ex As Exception
                    End Try
                    GC.Collect()
                Next intTry
                If intTry > intTrayMAx Then '모두 실패
                    Console.WriteLine(String.Format("Connection to {0} : Try to Open {2}/3 Times, All Failed", oOleDbConnection.DataSource, oOleDbConnection.State, intTrayMAx))
                Else
                    'Console.WriteLine(String.Format("Connection to {0} : Closed->{1}   , Try {2}/3   ", oOleDbConnection.DataSource, oOleDbConnection.State, intTry))
                End If
            Case Else
                Console.WriteLine(String.Format("Connection to {0} : {1} ", oOleDbConnection.DataSource, oOleDbConnection.State))
        End Select
    End Sub
End Module
