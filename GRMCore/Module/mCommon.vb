Module mCommon
    Friend g_performance_log_GUID As String         '성능 측정 기록 용 
    Friend g_dtStart_from_MonitorEXE As DateTime '성능 측정 기록 용 

    Function IO_Path_ChangeDrive(strV As Char, strPath As String) As String
        If strPath.Substring(1, 1) <> ":" Then Stop '오류 유발 상황이므로 중단
        Return strV + strPath.Substring(1)
    End Function
End Module
