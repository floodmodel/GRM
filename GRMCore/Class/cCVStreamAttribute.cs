using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GRMCore
{
    /// <summary>
    /// 하천속성을 가지는 셀에만 부여되는 클래스
    ///</summary>
    ///<remarks></remarks>
    public class cCVStreamAttribute
    {
        /// <summary>
        ///하도셀과 하도+지표면 흐름 속성을 가지는 격자에 부여되는 하천경사(m/m)
        ///</summary>
        ///<remarks></remarks>
        public double chBedSlope;
        /// <summary>
        ///하천차수
        ///</summary>
        ///<remarks></remarks>
        public int ChStrOrder = 0;
        /// <summary>
        ///하천의 바닥폭[m]
        ///</summary>
        ///<remarks></remarks>
        public double ChBaseWidth = 0;

        /// <summary>
        ///하폭레이어에서 받은 하폭[m]
        ///</summary>
        ///<remarks></remarks>
        public double ChBaseWidthByLayer = 0;

        /// <summary>
        ///현재의 channel CV의 하도조도계수
        ///</summary>
        ///<remarks></remarks>
        public double RoughnessCoeffCH;

        /// <summary>
        ///t 시간에서의 유출해석 전의 현재 channel CV의 초기 흐름단면적[m^2]
        ///</summary>
        ///<remarks></remarks>
        public double CSAch_i_j_ori;

        /// <summary>
        ///t 시간에서의 유출해석 결과 현재 channel CV의 흐름단면적[m^2]
        ///</summary>
        ///<remarks></remarks>
        public double CSAch_i_j;

        public double CSAchAddedByOFinCHnOFcell;

        /// <summary>
        ///t 시간에서의 유출해석 전의 현재 channel CV의 초기 수심[m]
        ///</summary>
        ///<remarks></remarks>
        public double hCVch_i_j_ori;

        /// <summary>
        ///t 시간에서의 유출해석 결과 현재 channel CV의 수심[m]
        ///</summary>
        ///<remarks></remarks>
        public double hCVch_i_j;

        /// <summary>
        ///t 시간에서의 유출해석 결과 현재 channel CV의 유속[m/s]
        ///</summary>
        ///<remarks></remarks>
        public double uCVch_i_j;

        /// <summary>
        ///t 시간에서의 유출해석 결과 현재 channel CV의 유량[m^3/s]
        ///</summary>
        ///<remarks></remarks>
        public double QCVch_i_j_m3Ps;

        /// <summary>
        ///하천셀에서의 초기 유량을 파일로 받을때 설정되는 값
        ///</summary>
        public double initialQCVch_i_j_m3Ps;

        /// <summary>
        ///현재의 channel CV의 좌측 제방 경사
        ///</summary>
        ///<remarks></remarks>
        public double chSideSlopeLeft;

        /// <summary>
        ///현재의 channel CV의 우측 제방 경사
        ///</summary>
        ///<remarks></remarks>
        public double chSideSlopeRight;

        /// <summary>
        /// 현재의 channel CV의 제방 계수. 계산 편의를 위해서 channel CV 별로 미리계산한 값
        /// </summary>
        /// <remarks></remarks>
        public double mChBankCoeff;

        /// <summary>
        ///현재 channel CV의 복단면 고수부지 바닥 폭[m]
        ///</summary>
        ///<remarks></remarks>
        public double chUpperRBaseWidth_m;

        /// <summary>
        /// 현재 channel CV의 복단면 고수부지의 수심[m]
        ///</summary>
        ///<remarks></remarks>
        public double chLowerRHeight;


        /// <summary>
        ///현재의 channel CV가 복단면인지(true), 단단면(false)인지를 나타내는 변수
        ///</summary>
        ///<remarks></remarks>
        public bool chIsCompoundCS;

        /// <summary>
        /// 복단면 channel 중 하층부의 면적[m^2]
        /// </summary>
        /// <remarks></remarks>
        public double chLowerRArea_m2;
    }
}
