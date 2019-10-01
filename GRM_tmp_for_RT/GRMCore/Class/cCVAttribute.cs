using System;
using System.Collections.Generic;

namespace GRMCore
{
    public class cCVAttribute : ICloneable
    {
        
        /// <summary>
        ///유역에 해당하는 셀에만 부여되는 일련번호
        ///</summary>
        ///<remarks></remarks>
        public int CVID;

        /// <summary>
        ///유역 ID
        ///</summary>
        ///<remarks></remarks>
        public int WSID;
        /// <summary>
        ///셀의 종류, 지표면흐름, 하도흐름, 지표면+하도
        ///</summary>
        ///<remarks></remarks>
        public cGRM.CellFlowType FlowType;
        /// <summary>
        ///지표면 해석에 적용되는 overland flow 셀의 경사(m/m)
        ///</summary>
        ///<remarks></remarks>
        public double SlopeOF;
        /// <summary>
        ///셀의 경사(m/m)
        ///</summary>
        ///<remarks></remarks>
        public double Slope;

        /// <summary>
        ///흐름방향
        ///</summary>
        ///<remarks></remarks>
        public cGRM.GRMFlowDirectionD8 FDir;

        /// <summary>
        ///흐름누적수, 자신의 셀을 제외하고, 상류에 있는 격자 개수
        ///</summary>
        ///<remarks></remarks>
        public int FAc;

        /// <summary>
        ///격자 중심으로부터 하류방향 격자면까지의 거리
        ///</summary>
        ///<remarks></remarks>
        public double DeltaXDownHalf_m;
        /// <summary>
        ///격자 중심으로부터 상류방향 격자면까지의 거리합
        ///</summary>
        ///<remarks></remarks>
        public double deltaXwSum;


        /// <summary>
        ///현재 셀로 흘러 들어오는 인접셀의 ID, 최대 7개
        ///없는 경우 Nothing
        ///</summary>
        ///<remarks></remarks>
        public List<int> NeighborCVidFlowIntoMe;
        /// <summary>
        ///흘러갈 직하류셀의 ID
        ///</summary>
        ///<remarks></remarks>
        public int DownCellidToFlow;


        /// <summary>
        ///인접상류셀 중 실제 유출이 발생하는 셀들의 개수
        ///</summary>
        ///<remarks></remarks>
        public int effCVCountFlowINTOCViW;

        /// <summary>
        ///모델링에 적용할 검사체적의 X방향 길이
        ///</summary>
        ///<remarks></remarks>
        public double CVDeltaX_m;
        /// <summary>
        ///현재 CV 하류에 있는 watchpoint 들의 CVid 집합
        ///</summary>
        ///<remarks></remarks>
        public List<int> DownStreamWPCVids;

        /// <summary>
        ///현재의 CV가 모의할 셀인지 아닌지 표시
        ///</summary>
        ///<remarks></remarks>
        public int toBeSimulated; // -1 : false, 1 : true

        /// <summary>
        ///현재 CV가 Stream 일경우 즉, eCellType이 Channel 혹은 OverlandAndChannel일 경우 부여되는 속성
        ///</summary>
        ///<remarks></remarks>
        public cCVStreamAttribute mStreamAttr;


        public bool IsStream
        {
            get
            {
                return mStreamAttr != null;
            }
        }

        /// <summary>
        ///t 시간에서 유출해석 시작 전 overland flow 검사체적의 수심
        ///</summary>
        ///<remarks></remarks>
        public double hCVof_i_j_ori;

        /// <summary>
        ///t 시간에서 유출해석 결과 overland flow 검사체적의 유속
        ///</summary>
        ///<remarks></remarks>
        public double uCVof_i_j;

        /// <summary>
        ///t 시간에서 유출해석 결과 overland flow 검사체적의 수심
        ///</summary>
        ///<remarks></remarks>
        public double hCVof_i_j;  // 수심

        /// <summary>
        ///t 시간에서 유출해석 결과 overland flow의 흐름 단면적
        ///</summary>
        ///<remarks></remarks>
        public double CSAof_i_j;

        /// <summary>
        ///단위폭당 overland flow 유량
        ///</summary>
        ///<remarks></remarks>
        public double qCVof_i_j;

        /// <summary>
        ///t 시간에서의 유출해석 결과 overland flow의 유량 [m^3/s]
        ///</summary>
        ///<remarks></remarks>
        public double QCVof_i_j_m3Ps;

        /// <summary>
        ///t 시간에서의 현재 셀에서 다음셀로 지표하에서 유출되는 유량 [m^3/s]
        ///</summary>
        ///<remarks></remarks>
        public double SSF_Q_m3Ps;

        /// <summary>
        ///상류인접 CV에서 현재 CV로 유입되는 유량 단순합. 이건 CVi에서의 연속방정식, 고려하지 않은 단순 합.[m^3/dt]
        ///</summary>
        ///<remarks></remarks>
        public double QsumCVw_dt_m3;

        /// <summary>
        ///dt 시간 동안의 강우량
        ///</summary>
        ///<remarks></remarks>
        public double RFApp_dt_meter;
        /// <summary>
        ///현재 강우입력자료에서 읽은 강우강도 m/s
        ///</summary>
        ///<remarks></remarks>
        public double RFReadintensity_mPsec;
        /// <summary>
        ///이전 시간의 강우강도 m/s
        ///</summary>
        ///<remarks></remarks>
        public double RFReadintensity_tM1_mPsec;

        /// <summary>
        ///dt시간 동안의 유효강우량
        ///</summary>
        ///<remarks></remarks>
        public double EffRFCV_dt_meter;

        /// <summary>
        ///출력 시간간격 동안의 누적 강우량[m]
        ///</summary>
        ///<remarks></remarks>
        public double RF_dtPrintOut_meter;

        /// <summary>
        ///출력 시간간격 동안의 누적 강우량[m]
        ///</summary>
        ///<remarks></remarks>
        public double RFAcc_FromStartToNow_meter;

        /// <summary>
        ///토양수분함량. t 시간까지의 누적 침투량[m], 토양깊이가 아니고, 수심이다.
        ///</summary>
        ///<remarks></remarks>
        public double soilWaterContent_m;

        /// <summary>
        ///토양수분함량. t-1 시간까지의 누적 침투량[m]. 수심
        ///</summary>
        ///<remarks></remarks>
        public double soilWaterContent_tM1_m;

        /// <summary>
        ///t 시간에서 계산된 침투률[m/s]
        ///</summary>
        ///<remarks></remarks>
        public double InfiltrationRatef_mPsec;
        /// <summary>
        ///t-1 시간에서 적용된 침투률[m/s]
        ///</summary>
        ///<remarks></remarks>
        public double InfiltrationRatef_tM1_mPsec;

        /// <summary>
        ///t 시간에서 계산된 dt 시간동안의 침투량[m/dt]
        ///</summary>
        ///<remarks></remarks>
        public double InfiltrationF_mPdt;

        public bool bAfterSaturated = false;

        // Public powCUnsaturatedK As Single

        public cSetGreenAmpt.SoilTextureCode SoilTextureCode;

        /// <summary>
        ///토성레이어의 값, VAT참조
        ///</summary>
        ///<remarks></remarks>
        public int SoilTextureValue;

        /// <summary>
        ///현재 CV 토양의 수리전도도[m/s] 모델링 적용값
        ///</summary>
        ///<remarks></remarks>
        public double hydraulicConductK_mPsec;

        /// <summary>
        ///현재 CV 토양의 수리전도도[m/s] GRM default
        ///</summary>
        ///<remarks></remarks>
        public double HydraulicConductKori_mPsec;

        /// <summary>
        ///현재 CV 토양의 유효공극률 모델링 적용값. 무차원. 0~1
        ///</summary>
        ///<remarks></remarks>
        public double effectivePorosityThetaE;

        /// <summary>
        ///현재 CV 토양의 유효공극률 grm default. 무차원. 0~1
        ///</summary>
        ///<remarks></remarks>
        public double EffectivePorosityThetaEori;

        /// <summary>
        ///현재 CV 토양의 공극률 모델링 적용값. 무차원. 0~1
        ///</summary>
        ///<remarks></remarks>
        public double porosityEta;

        /// <summary>
        ///현재 CV 토양의 공극률 GRM default. 무차원. 0~1
        ///</summary>
        ///<remarks></remarks>
        public double PorosityEtaOri;

        /// <summary>
        ///현재 CV 토양의 습윤전선흡인수두[m] 모델링 적용값
        ///</summary>
        ///<remarks></remarks>
        public double wettingFrontSuctionHeadPsi_m;

        /// <summary>
        ///현재 CV 토양의 습윤전선흡인수두[m] grm default
        ///</summary>
        ///<remarks></remarks>
        public double WettingFrontSuctionHeadPsiOri_m;

        /// <summary>
        ///토양습윤변화량
        ///</summary>
        ///<remarks></remarks>
        public double SoilMoistureChangeDeltaTheta;


        public cSetSoilDepth.SoilDepthCode SoilDepthCode;

        /// <summary>
        ///토심레이어의 값, VAT 참조 
        ///</summary>
        ///<remarks></remarks>
        public int SoilDepthTypeValue;

        /// <summary>
        ///현재 CV의 토양심 모델링 적용 값[m].
        ///</summary>
        ///<remarks></remarks>
        public double soilDepth_m;

        /// <summary>
        ///현재 CV의 토양심 GRM default 값[m].
        ///</summary>
        ///<remarks></remarks>
        public double SoilDepthOri_m;

        /// <summary>
        ///현재 CV의 유효토양심 값[m]. 토양심에서 유효공극률을 곱한 값
        ///</summary>
        ///<remarks></remarks>
        public double SoilDepthEffectiveAsWaterDepth_m;


        /// <summary>
        ///현재 CV 토양의 초기포화도. 무차원. 0~1
        ///</summary>
        ///<remarks></remarks>
        public double InitialSaturation;

        /// <summary>
        ///현재 CV 토양의 유효포화도. 무차원. 0~1
        ///</summary>
        ///<remarks></remarks>
        public double EffectiveSaturationSe; // 무차원 %/100

        /// <summary>
        ///토양의 현재 포화도
        ///</summary>
        ///<remarks></remarks>
        public double soilSaturationRatio;

        // ''' <summary>
        // ''' 현재 토양이 포화되었는지 아닌지를 구분
        // ''' </summary>
        // ''' <remarks></remarks>
        // Public bSaturated As Boolean

        public cGRM.UnSaturatedKType UKType;
        public double coefUK;

        /// <summary>
        ///하도셀에서 비피압대수층의 수심(하도바닥에서의 높이)[m].
        ///</summary>
        ///<remarks></remarks>
        public double hUAQfromChannelBed_m; // 

        /// <summary>
        ///암반으로부터 비피압대수층의 상단부까지의 깊이[m]. 토양깊이.
        ///</summary>
        ///<remarks></remarks>
        public double hUAQfromBedrock_m;

        /// <summary>
        ///현재 CV 토양의 암반까지의 깊이[m]. 지표면에서 암반까지의 깊이임.
        ///</summary>
        ///<remarks></remarks>
        public double SoilDepthToBedrock_m;

        /// <summary>
        ///현재 CV의 기저유출량 [m^3/s]
        ///</summary>
        ///<remarks></remarks>
        public double baseflow_Q_m3Ps;


        public cSetLandcover.LandCoverCode LandCoverCode;

        /// <summary>
        ///토지피복레이어의 값, VAT 참조
        ///</summary>
        ///<remarks></remarks>
        public int LandCoverValue;

        /// <summary>
        ///현재 CV 토지피복의 불투수율. 무차원, 0~1.
        ///</summary>
        ///<remarks></remarks>
        public double ImperviousRatio;

        /// <summary>
        ///현재 CV 토지피복에서의 차단량 [m].
        ///</summary>
        ///<remarks></remarks>
        public double Interception_m;

        /// <summary>
        ///현재 CV 토지피복의 모델링 적용 지표면 조도계수
        ///</summary>
        ///<remarks></remarks>
        public double RoughnessCoeffOF;

        /// <summary>
        ///현재 CV 토지피복의 grm default 지표면 조도계수
        ///</summary>
        ///<remarks></remarks>
        public double RoughnessCoeffOFori;


        /// <summary>
        ///현재 CV에 부여된 Flow control 종류
        ///</summary>
        ///<remarks></remarks>
        public cFlowControl.FlowControlType FCType;

        /// <summary>
        ///현재 CV에서 flow control 모의시 누적 저류량[m^3]
        ///</summary>
        ///<remarks></remarks>
        public double StorageCumulative_m3;

        /// <summary>
        ///현재 CV에서 flow control 모의시 dt 시간동안의 강우에 의해서 추가되는 저류량[m^3/dt]
        ///</summary>
        ///<remarks></remarks>
        public double StorageAddedForDTfromRF_m3;


        public object Clone()
        {
            cCVAttribute cln = new cCVAttribute();
            if (mStreamAttr != null)
            {
                cln.mStreamAttr = new cCVStreamAttribute();
                cln.mStreamAttr.QCVch_i_j_m3Ps = mStreamAttr.QCVch_i_j_m3Ps;
            }
            else
            { cln.QCVof_i_j_m3Ps = QCVof_i_j_m3Ps; }
            cln.hUAQfromChannelBed_m = hUAQfromChannelBed_m;
            cln.soilSaturationRatio = soilSaturationRatio;
            cln.soilWaterContent_m = soilWaterContent_m;
            cln.StorageCumulative_m3 = StorageCumulative_m3;
            return cln;
        }
    }
}
