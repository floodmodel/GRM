Module mCommon

    Friend g_performance_log_GUID As String         '2017.6.1 원 : 성능 측정 기록 용도. 
    Friend g_dtStart_from_MonitorEXE As DateTime '2017.6.2 원 : 성능 측정 기록 용도. 

    Function IO_Path_ChangeDrive(strV As Char, strPath As String) As String
        If strPath.Substring(1, 1) <> ":" Then Stop '오류 유발 상황이므로 중단 시킴. 2017.6.1 원 
        Return strV + strPath.Substring(1)
    End Function
End Module
