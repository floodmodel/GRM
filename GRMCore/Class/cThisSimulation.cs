using System;


namespace GRMCore
{
    public struct sThisSimulation
    {

        /// <summary>
        ///   True로 설정해서 시작
        ///   </summary>
        ///   <remarks></remarks>
        public static bool mGRMSetupIsNormal;

        /// <summary>
        ///   현재 모델링 대상 이벤트(강우)에 포함된 자료의 개수
        ///   </summary>
        ///   <remarks></remarks>
        public static int mRFDataCountInThisEvent;



        /// <summary>
        ///   기존의 강우 레이어 위치. RGD 파일 커낵션 갱신을 위해 필요
        ///   </summary>
        ///   <remarks></remarks>
        public static string mRFGridFilePathNow;

        /// <summary>
        ///   dt 시간동안의 유역 평균강우량[m]
        ///   </summary>
        ///   <remarks></remarks>
        public static double mRFMeanForDT_m;
        public static double mRFMeanForThisInputRFData;
        public static double mRFIntensitySumForAllCellsInCurrentRFData_mPs;

        /// <summary>
        ///   dt 시간동안의 유역 평균강우량의 누적값[m]
        ///   </summary>
        ///   <remarks></remarks>
        public static double mRFMeanForAllCell_sumForDTprintOut_m;

        /// <summary>
        ///   모델링 시간 확인용 변수. 현재 모델링의 시작시간[DATE]
        ///   </summary>
        ///   <remarks></remarks>
        public static DateTime mTimeThisSimulationStarted;
        public static bool mAnalyzerSet = false;

        public static bool IsParallel = false;
        public static int MaxDegreeOfParallelism = -1;
        public static bool IsFixedTimeStep = true;
        public static int dtsec;
        public static double vMaxInThisStep = float.MinValue;
        public static int dtMaxLimit_sec = 1800; // 30분
        public static int dtMinLimit_sec = 10;
        public static int dtsec_usedtoForwardToThisTime;
    }
}
