Imports System.Runtime.Serialization.Formatters.Binary
Public Class cCVAttribute
    Implements ICloneable

#Region "Watershed"
    ''' <summary>
    ''' 유역에 해당하는 셀에만 부여되는 일련번호
    ''' </summary>
    ''' <remarks></remarks>
    Public CVID As Integer

    ''' <summary>
    ''' 유역 ID
    ''' </summary>
    ''' <remarks></remarks>
    Public WSID As Integer
    ''' <summary>
    ''' 셀의 종류, 지표면흐름, 하도흐름, 지표면+하도
    ''' </summary>
    ''' <remarks></remarks>
    Public FlowType As cGRM.CellFlowType
    ''' <summary>
    ''' 지표면 해석에 적용되는 overland flow 셀의 경사(m/m)
    ''' </summary>
    ''' <remarks></remarks>
    Public SlopeOF As Double
    ''' <summary>
    ''' 셀의 경사(m/m)
    ''' </summary>
    ''' <remarks></remarks>
    Public Slope As Double

    ''' <summary>
    ''' 흐름방향
    ''' </summary>
    ''' <remarks></remarks>
    Public FDir As cGRM.GRMFlowDirectionD8

    ''' <summary>
    ''' 흐름누적수, 자신의 셀을 제외하고, 상류에 있는 격자 개수
    ''' </summary>
    ''' <remarks></remarks>
    Public FAc As Integer

    ''' <summary>
    ''' x방향 column 번호
    ''' </summary>
    ''' <remarks></remarks>
    Public XCol As Integer
    ''' <summary>
    ''' y방향 row 번호
    ''' </summary>
    ''' <remarks></remarks>
    Public YRow As Integer

    ''' <summary>
    ''' x방향 직각 좌료
    ''' </summary>
    ''' <remarks></remarks>
    Public MapX As Double
    ''' <summary>
    ''' y방향 직각 좌표
    ''' </summary>
    ''' <remarks></remarks>
    Public MapY As Double

    ''' <summary>
    ''' 격자 중심으로부터 하류방향 격자면까지의 거리
    ''' </summary>
    ''' <remarks></remarks>
    Public DeltaXDownHalf_m As Single
    ''' <summary>
    ''' 격자 중심으로부터 상류방향 격자면까지의 거리합
    ''' </summary>
    ''' <remarks></remarks>
    Public deltaXwSum As Single

#End Region

#Region "Flow network"
    ''' <summary>
    ''' 현재 셀로 흘러 들어오는 인접셀의 ID, 최대 7개
    ''' 없는 경우 Nothing
    ''' </summary>
    ''' <remarks></remarks>
    Public NeighborCVidFlowIntoMe As List(Of Integer)
    ''' <summary>
    ''' 흘러갈 직하류셀의 ID
    ''' </summary>
    ''' <remarks></remarks>
    Public DownCellidToFlow As Integer


    ''' <summary>
    ''' 인접상류셀 중 실제 유출이 발생하는 셀들의 개수
    ''' </summary>
    ''' <remarks></remarks>
    Public effCVCountFlowINTOCViW As Byte

    ''' <summary>
    ''' 모델링에 적용할 검사체적의 X방향 길이
    ''' </summary>
    ''' <remarks></remarks>
    Public CVDeltaX_m As Single
    ''' <summary>
    ''' 현재 CV 하류에 있는 watchpoint 들의 CVid 집합
    ''' </summary>
    ''' <remarks></remarks>
    Public DownStreamWPCVids As List(Of Integer)

    ''' <summary>
    ''' 현재의 CV가 모의할 셀인지 아닌지 표시
    ''' </summary>
    ''' <remarks></remarks>
    Public toBeSimulated As Boolean 'FlagTrueFalseNone

    ''' <summary>
    ''' 현재 CV가 Stream 일경우 즉, eCellType이 Channel 혹은 OverlandAndChannel일 경우 부여되는 속성
    ''' </summary>
    ''' <remarks></remarks>
    Public mStreamAttr As cCVStreamAttribute


    Public ReadOnly Property IsStream() As Boolean
        Get
            Return mStreamAttr IsNot Nothing
        End Get
    End Property


#End Region

#Region "Modelling results"
    ''' <summary>
    ''' t 시간에서 유출해석 시작 전 overland flow 검사체적의 수심
    ''' </summary>
    ''' <remarks></remarks>
    Public hCVof_i_j_ori As Single

    ''' <summary>
    ''' t 시간에서 유출해석 결과 overland flow 검사체적의 유속
    ''' </summary>
    ''' <remarks></remarks>
    Public uCVof_i_j As Single

    ''' <summary>
    ''' t 시간에서 유출해석 결과 overland flow 검사체적의 수심
    ''' </summary>
    ''' <remarks></remarks>
    Public hCVof_i_j As Single  '수심

    ''' <summary>
    ''' t 시간에서 유출해석 결과 overland flow의 흐름 단면적
    ''' </summary>
    ''' <remarks></remarks>
    Public CSAof_i_j As Single

    ''' <summary>
    ''' 단위폭당 overland flow 유량
    ''' </summary>
    ''' <remarks></remarks>
    Public qCVof_i_j As Single

    ''' <summary>
    ''' t 시간에서의 유출해석 결과 overland flow의 유량 [m^3/s]
    ''' </summary>
    ''' <remarks></remarks>
    Public QCVof_i_j_m3Ps As Single

    ''' <summary>
    ''' t 시간에서의 현재 셀에서 다음셀로 지표하에서 유출되는 유량 [m^3/s]
    ''' </summary>
    ''' <remarks></remarks>
    Public SSF_Q_m3Ps As Single

    ''' <summary>
    ''' 상류인접 CV에서 현재 CV로 유입되는 유량 단순합. 이건 CVi에서의 연속방정식, 고려하지 않은 단순 합.[m^3/dt]
    ''' </summary>
    ''' <remarks></remarks>
    Public QsumCVw_dt_m3 As Single
#End Region

#Region "강우관련"
    ''' <summary>
    ''' dt 시간 동안의 강우량
    ''' </summary>
    ''' <remarks></remarks>
    Public RFApp_dt_meter As Single
    ''' <summary>
    ''' 현재 강우입력자료에서 읽은 강우강도 m/s
    ''' </summary>
    ''' <remarks></remarks>
    Public RFReadintensity_mPsec As Single
    ''' <summary>
    ''' 이전 시간의 강우강도 m/s
    ''' </summary>
    ''' <remarks></remarks>
    Public RFReadintensity_tM1_mPsec As Single

    ''' <summary>
    ''' dt시간 동안의 유효강우량
    ''' </summary>
    ''' <remarks></remarks>
    Public EffRFCV_dt_meter As Single

    ''' <summary>
    ''' 출력 시간간격 동안의 누적 강우량[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public RF_dtPrintOut_meter As Single

    ''' <summary>
    ''' 출력 시간간격 동안의 누적 강우량[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public RFAcc_FromStartToNow_meter As Single
#End Region

#Region "침투관련"
    ''' <summary>
    ''' t 시간까지의 누적 침투량[m], 토양깊이가 아니고, 수심이다.
    ''' </summary>
    ''' <remarks></remarks>
    Public CumulativeInfiltrationF_m As Single

    ''' <summary>
    ''' t-1 시간까지의 누적 침투량[m]. 수심
    ''' </summary>
    ''' <remarks></remarks>
    Public CumulativeInfiltrationF_tM1_m As Single

    ''' <summary>
    ''' t 시간에서 계산된 침투률[m/s]
    ''' </summary>
    ''' <remarks></remarks>
    Public InfiltrationRatef_mPsec As Single
    ''' <summary>
    ''' t-1 시간에서 적용된 침투률[m/s]
    ''' </summary>
    ''' <remarks></remarks>
    Public InfiltrationRatef_tM1_mPsec As Single

    ''' <summary>
    ''' t 시간에서 계산된 dt 시간동안의 침투량[m/dt]
    ''' </summary>
    ''' <remarks></remarks>
    Public InfiltrationF_mPdt As Single

    Public bAfterSaturated As Boolean = False

    'Public powCUnsaturatedK As Single

#End Region

#Region "토양특성"

    Public SoilTextureCode As cSetGreenAmpt.SoilTextureCode

    ''' <summary>
    ''' 토성레이어의 값, VAT참조
    ''' </summary>
    ''' <remarks></remarks>
    Public SoilTextureValue As Integer

    ''' <summary>
    ''' 현재 CV 토양의 수리전도도[m/s] 모델링 적용값
    ''' </summary>
    ''' <remarks></remarks>
    Public hydraulicConductK_mPsec As Single

    ''' <summary>
    ''' 현재 CV 토양의 수리전도도[m/s] GRM default
    ''' </summary>
    ''' <remarks></remarks>
    Public HydraulicConductKori_mPsec As Single

    ''' <summary>
    ''' 현재 CV 토양의 유효공극률 모델링 적용값. 무차원. 0~1
    ''' </summary>
    ''' <remarks></remarks>
    Public effectivePorosityThetaE As Single

    ''' <summary>
    ''' 현재 CV 토양의 유효공극률 grm default. 무차원. 0~1
    ''' </summary>
    ''' <remarks></remarks>
    Public EffectivePorosityThetaEori As Single

    ''' <summary>
    ''' 현재 CV 토양의 공극률 모델링 적용값. 무차원. 0~1
    ''' </summary>
    ''' <remarks></remarks>
    Public porosityEta As Single

    ''' <summary>
    ''' 현재 CV 토양의 공극률 GRM default. 무차원. 0~1
    ''' </summary>
    ''' <remarks></remarks>
    Public PorosityEtaOri As Single

    ''' <summary>
    ''' 현재 CV 토양의 습윤전선흡인수두[m] 모델링 적용값
    ''' </summary>
    ''' <remarks></remarks>
    Public wettingFrontSuctionHeadPsi_m As Single

    ''' <summary>
    ''' 현재 CV 토양의 습윤전선흡인수두[m] grm default
    ''' </summary>
    ''' <remarks></remarks>
    Public WettingFrontSuctionHeadPsiOri_m As Single

    ''' <summary>
    ''' 토양습윤변화량
    ''' </summary>
    ''' <remarks></remarks>
    Public SoilMoistureChangeDeltaTheta As Single
#End Region

#Region "토양깊이"

    Public SoilDepthCode As cSetSoilDepth.SoilDepthCode

    ''' <summary>
    ''' 토심레이어의 값, VAT 참조 
    ''' </summary>
    ''' <remarks></remarks>
    Public SoilDepthTypeValue As Integer

    ''' <summary>
    ''' 현재 CV의 토양심 모델링 적용 값[m].
    ''' </summary>
    ''' <remarks></remarks>
    Public soilDepth_m As Single

    ''' <summary>
    ''' 현재 CV의 토양심 GRM default 값[m].
    ''' </summary>
    ''' <remarks></remarks>
    Public SoilDepthOri_m As Single

    ''' <summary>
    ''' 현재 CV의 유효토양심 값[m]. 토양심에서 유효공극률을 곱한 값
    ''' </summary>
    ''' <remarks></remarks>
    Public SoilDepthEffectiveAsWaterDepth_m As Single
#End Region

#Region "포화도 관련"

    ''' <summary>
    ''' 현재 CV 토양의 초기포화도. 무차원. 0~1
    ''' </summary>
    ''' <remarks></remarks>
    Public InitialSaturation As Single

    ''' <summary>
    ''' 현재 CV 토양의 유효포화도. 무차원. 0~1
    ''' </summary>
    ''' <remarks></remarks>
    Public EffectiveSaturationSe As Single '무차원 %/100

    ''' <summary>
    ''' 토양의 현재 포화도
    ''' </summary>
    ''' <remarks></remarks>
    Public soilSaturationRatio As Single

    '''' <summary>
    '''' 현재 토양이 포화되었는지 아닌지를 구분
    '''' </summary>
    '''' <remarks></remarks>
    'Public bSaturated As Boolean

    Public UKType As cGRM.UnSaturatedKType
    Public coefUK As Single 
#End Region

#Region "지하수 관련"
    ''' <summary>
    ''' 하도셀에서 비피압대수층의 수심(하도바닥에서의 높이)[m].
    ''' </summary>
    ''' <remarks></remarks>
    Public hUAQfromChannelBed_m As Single '

    ''' <summary>
    ''' 암반으로부터 비피압대수층의 상단부까지의 깊이[m]. 토양깊이.
    ''' </summary>
    ''' <remarks></remarks>
    Public hUAQfromBedrock_m As Single

    ''' <summary>
    ''' 현재 CV 토양의 암반까지의 깊이[m]. 지표면에서 암반까지의 깊이임.
    ''' </summary>
    ''' <remarks></remarks>
    Public SoilDepthToBedrock_m As Single

    ''' <summary>
    ''' 현재 CV의 기저유출량 [m^3/s]
    ''' </summary>
    ''' <remarks></remarks>
    Public baseflow_Q_m3Ps As Single
#End Region

#Region "토지피복"

    Public LandCoverCode As cSetLandcover.LandCoverCode

    ''' <summary>
    ''' 토지피복레이어의 값, VAT 참조
    ''' </summary>
    ''' <remarks></remarks>
    Public LandCoverValue As Integer

    ''' <summary>
    ''' 현재 CV 토지피복의 불투수율. 무차원, 0~1.
    ''' </summary>
    ''' <remarks></remarks>
    Public ImperviousRatio As Single

    ''' <summary>
    ''' 현재 CV 토지피복에서의 차단량 [m].
    ''' </summary>
    ''' <remarks></remarks>
    Public Interception_m As Single

    ''' <summary>
    ''' 현재 CV 토지피복의 모델링 적용 지표면 조도계수
    ''' </summary>
    ''' <remarks></remarks>
    Public RoughnessCoeffOF As Single

    ''' <summary>
    ''' 현재 CV 토지피복의 grm default 지표면 조도계수
    ''' </summary>
    ''' <remarks></remarks>
    Public RoughnessCoeffOFori As Single

#End Region

#Region "Flow control"
    ''' <summary>
    ''' 현재 CV에 부여된 Flow control 종류
    ''' </summary>
    ''' <remarks></remarks>
    Public FCType As cFlowControl.FlowControlType

    ''' <summary>
    ''' 현재 CV에서 flow control 모의시 누적 저류량[m^3]
    ''' </summary>
    ''' <remarks></remarks>
    Public StorageCumulative_m3 As Single

    ''' <summary>
    ''' 현재 CV에서 flow control 모의시 dt 시간동안의 강우에 의해서 추가되는 저류량[m^3/dt]
    ''' </summary>
    ''' <remarks></remarks>
    Public StorageAddedForDTfromRF_m3 As Single
#End Region

#Region "매개변수 추정"
    'Public bToEstimateParameters As Boolean
#End Region



    Public Function Clone() As Object Implements ICloneable.Clone
        Dim cln As New cCVAttribute
        If Me.mStreamAttr IsNot Nothing Then
            cln.mStreamAttr = New cCVStreamAttribute
            cln.mStreamAttr.QCVch_i_j_m3Ps = Me.mStreamAttr.QCVch_i_j_m3Ps
        Else
            cln.QCVof_i_j_m3Ps = Me.QCVof_i_j_m3Ps
        End If
        cln.hUAQfromChannelBed_m = Me.hUAQfromChannelBed_m
        cln.soilSaturationRatio = Me.soilSaturationRatio
        cln.CumulativeInfiltrationF_m = Me.CumulativeInfiltrationF_m
        cln.StorageCumulative_m3 = Me.StorageCumulative_m3
        Return cln

    End Function
End Class