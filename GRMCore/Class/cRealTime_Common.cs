using System;

namespace GRMCore
{
    public class cRealTime_Common
    {
        internal static string g_performance_log_GUID;         // 성능 측정 기록 용 
        internal static DateTime g_dtStart_from_MonitorEXE; // 성능 측정 기록 용 

        public static string IO_Path_ChangeDrive(char strV, string strPath)
        {
            if (strPath.Substring(1, 1) != ":")
            {
                System.Diagnostics.Debugger.Break(); // 오류 유발 상황이므로 중단
            }
            return strV + strPath.Substring(1);
        }
    }
}
