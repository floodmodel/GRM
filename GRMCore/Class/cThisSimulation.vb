Public Class cThisSimulation

    ' 삭제 대상
    'Public Shared tmp_24H_RunTime As String
    'Public Shared tmp_48H_RunTime As String
    'Public Shared tmp_InfoFile As String = "D:\GRM_ex\Parallel_evaluation\PTestTime.txt"



#Region "유출해석 시작전 설정되어야 하는 변수"
    ''' <summary>
    ''' True로 설정해서 시작
    ''' </summary>
    ''' <remarks></remarks>
    Public Shared mGRMSetupIsNormal As Boolean

    ''' <summary>
    ''' 현재 모델링 대상 이벤트(강우)에 포함된 자료의 개수
    ''' </summary>
    ''' <remarks></remarks>
    Public Shared mRFDataCountInThisEvent As Integer

#End Region


    ''' <summary>
    ''' 기존의 강우 레이어 위치. RGD 파일 커낵션 갱신을 위해 필요
    ''' </summary>
    ''' <remarks></remarks>
    Public Shared mRFGridFilePathNow As String

    ''' <summary>
    ''' dt 시간동안의 유역 평균강우량[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public Shared mRFMeanForDT_m As Double

    Public Shared mRFMeanForThisInputRFData As Double

    'Public Shared mRFSumForAllCellsForDT_m As Double

    Public Shared mRFIntensitySumForAllCellsInCurrentRFData_mPs As Double

    ''' <summary>
    ''' dt 시간동안의 유역 평균강우량의 누적값[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public Shared mRFMeanForAllCell_sumForDTprintOut_m As Double

    '''' <summary>
    '''' 모델링 시간 확인용 변수. 지금 모델링 스텝에서의 시작시간[DATE]
    '''' </summary>
    '''' <remarks></remarks>
    'Public Shared mTimeThisStepStarted As Date

    ''' <summary>
    ''' 모델링 시간 확인용 변수. 현재 모델링의 시작시간[DATE]
    ''' </summary>
    ''' <remarks></remarks>
    Public Shared mTimeThisSimulationStarted As Date

    '''' <summary>
    '''' 유출해석 결과를 출력한 행의 개수. 즉, 결과 출력 회수
    '''' </summary>
    '''' <remarks></remarks>
    'Public Shared mPrintOutResultRowCount As Integer

    Public Shared mAnalyzerSet As Boolean = False

    Public Shared IsParallel As Boolean = False
    Public Shared MaxDegreeOfParallelism As Integer = -1
    Public Shared IsFixedTimeStep As Boolean = True
    Public Shared dtsec As Integer
    Public Shared vMaxInThisStep As Single = Single.MinValue
    Public Shared dtMaxLimit_sec As Integer = 1800
    Public Shared dtMinLimit_sec As Integer = 10

    Public Shared dtsec_usedtoForwardToThisTime As Integer


End Class
