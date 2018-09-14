''' <summary>
''' 하천속성을 가지는 셀에만 부여되는 클래스
''' </summary>
''' <remarks></remarks>
Public Class cCVStreamAttribute

    ''' <summary>
    ''' 하도셀과 하도+지표면 흐름 속성을 가지는 격자에 부여되는 하천경사(m/m)
    ''' </summary>
    ''' <remarks></remarks>
    Public chBedSlope As Double
    ''' <summary>
    ''' 하천차수
    ''' </summary>
    ''' <remarks></remarks>
    Public ChStrOrder As Integer = 0
    ''' <summary>
    ''' 하천의 바닥폭[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public ChBaseWidth As Single = 0

    ''' <summary>
    ''' 하폭레이어에서 받은 하폭[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public ChBaseWidthByLayer As Single = 0

    ''' <summary>
    ''' 현재의 channel CV의 하도조도계수
    ''' </summary>
    ''' <remarks></remarks>
    Public RoughnessCoeffCH As Single

    ''' <summary>
    ''' t 시간에서의 유출해석 전의 현재 channel CV의 초기 흐름단면적[m^2]
    ''' </summary>
    ''' <remarks></remarks>
    Public CSAch_i_j_ori As Single

    ''' <summary>
    ''' t 시간에서의 유출해석 결과 현재 channel CV의 흐름단면적[m^2]
    ''' </summary>
    ''' <remarks></remarks>
    Public CSAch_i_j As Single

    Public CSAchAddedByOFinCHnOFcell As Single

    ''' <summary>
    ''' t 시간에서의 유출해석 전의 현재 channel CV의 초기 수심[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public hCVch_i_j_ori As Single

    ''' <summary>
    ''' t 시간에서의 유출해석 결과 현재 channel CV의 수심[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public hCVch_i_j As Single

    ''' <summary>
    ''' t 시간에서의 유출해석 결과 현재 channel CV의 유속[m/s]
    ''' </summary>
    ''' <remarks></remarks>
    Public uCVch_i_j As Single

    ''' <summary>
    ''' t 시간에서의 유출해석 결과 현재 channel CV의 유량[m^3/s]
    ''' </summary>
    ''' <remarks></remarks>
    Public QCVch_i_j_m3Ps As Single

    ''' <summary>
    ''' 하천셀에서의 초기 유량을 파일로 받을때 설정되는 값
    ''' </summary>
    Public initialQCVch_i_j_m3Ps As Single

    ''' <summary>
    ''' 현재의 channel CV의 좌측 제방 경사
    ''' </summary>
    ''' <remarks></remarks>
    Public chSideSlopeLeft As Single

    ''' <summary>
    ''' 현재의 channel CV의 우측 제방 경사
    ''' </summary>
    ''' <remarks></remarks>
    Public chSideSlopeRight As Single

    '''' <summary>
    '''' 현재의 channel CV의 제방 계수. 계산 편의를 위해서 channel CV 별로 미리계산한 값
    '''' </summary>
    '''' <remarks></remarks>
    Public mChBankCoeff As Single

#Region "복단면 관련"
    ''' <summary>
    '''현재 channel CV의 복단면 고수부지 바닥 폭[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public chUpperRBaseWidth_m As Single

    ''' <summary>
    ''' 현재 channel CV의 복단면 고수부지의 수심[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public chLowerRHeight As Single

    '''' <summary>
    '''' 현재 channel CV의 복단면의 전체 수심[m]
    '''' </summary>
    '''' <remarks></remarks>
    'Public chFlowDepthNow As Single

    '''' <summary>
    '''' 현재 channel CV의 복단면 저수부의 수심[m]
    '''' </summary>
    '''' <remarks></remarks>
    'Public chNormalFlowDepthNow As Single

    '''' <summary>
    '''' 현재 channel CV의 저수부 수면에서 복단면까지의 초기 높이 차
    '''' </summary>
    '''' <remarks></remarks>
    'Public chHeightFromLowerAreaWaterSurfaceToUpperArea_m As Single

    ''' <summary>
    ''' 현재의 channel CV가 복단면인지(true), 단단면(false)인지를 나타내는 변수
    ''' </summary>
    ''' <remarks></remarks>
    Public chIsCompoundCS As Boolean

    ''' <summary>
    ''' 복단면 channel 중 하층부의 면적[m^2]
    ''' </summary>
    ''' <remarks></remarks>
    Public chLowerRArea_m2 As Single
#End Region


 



End Class
