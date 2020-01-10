using System;

namespace GRMCore
{
    public class cRealTime_Common
    {
        internal static string g_performance_log_GUID;         // 성능 측정 기록 용 
        internal static DateTime g_dtStart_from_MonitorEXE; // 성능 측정 기록 용 
        internal static string g_strModel;  //MODEL 구분용 2019.4.12   .LENS는 m00~m12
        internal static string g_strTimeTagBase_UCT;       //l030_v070_m00_h004.2016100300.gb2_1_clip.asc 의 경우 2016100300
        internal static string g_strTimeTagBase_KST;        // 상기의 경우 2016100300+9 즉 2016100309임.

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
