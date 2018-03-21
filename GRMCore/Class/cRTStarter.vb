Imports System.Text

Public Class cRTStarter
    Event RTStatus(ByVal strMSG As String)
    Private WithEvents GRMRT As cRealTime
    Private RTProject As GRMCore.cProject
    Private Const CONST_PIC_WIDTH As Single = 1024
    Private Const CONST_PIC_HEIGHT As Single = 768
    Private mFPN_RTEnv As String
    Private mProjectFPN As String
    Private mFP_RFLayerFP As String
    'Private mFPN_FCData As String '현재 유역에서 읽을 fc data 파일. db 매개로 수정하였으므로 필요없음.
    Private mSimulationStartingTime As String
    Private mRainfallDataType As GRMCore.cRainfall.RainfallDataType
    Private mbDWS_EXIST As Boolean
    Private mbIsFC As Boolean
    Private mCWCellColXToConnectDW As Integer
    Private mCWCellRowYToConnectDW As Integer
    Private mDWCellColXToConnectCW As Integer
    Private mDWCellRowYToConnectCW As Integer
    'Private mFPN_DWSS_FCDATA As String  '하류 유역의 fc data로 사용할 파일. db 매개로 수정하였으므로 필요없음.
    Private mRFInterval_MIN As Integer
    Private mOutPutInterval_MIN As Integer

    ''' <summary>
    ''' 외부 코드에서 ref 파일만 가지고 RT 실행시킬때 이거 호출하면 됨
    ''' </summary>
    ''' <param name="fpn_REF"></param>
    ''' <param name="dtStart">런타임 확인을 위해 사용되는 모델링 시작 시간</param>
    ''' <param name="RTStartDateTime">GUI 등으로 부터 모의기간 시작시간 설정된 것을 받을때 사용</param>
    ''' <remarks></remarks>
    Public Sub New(fpn_REF As String, strGUID As String, dtStart As DateTime, Optional RTStartDateTime As String = "")
        mFPN_RTEnv = fpn_REF
        UpdateRTVariablesUsingEnvFile(mFPN_RTEnv, RTStartDateTime) '여기서 파일로 설정
        g_performance_log_GUID = strGUID
        g_dtStart_from_MonitorEXE = dtStart
    End Sub

    Private Function UpdateRTVariablesUsingEnvFile(rtEnvFPN As String, Optional RTStartDateTime As String = "") As Boolean
        Dim dtEnv As New GRMProject.RTenvDataTable
        dtEnv.ReadXml(rtEnvFPN)
        Dim r As GRMProject.RTenvRow = CType(dtEnv.Rows(0), GRMProject.RTenvRow)
        mProjectFPN = r.ProjectFPN
        mFP_RFLayerFP = r.RTRFfolderName
        If r.IsFC = True Then
            mbIsFC = True
            'If Not r.IsRTFCdataFPNNull AndAlso r.RTFCdataFPN <> "" Then
            '    mFPN_FCData = r.RTFCdataFPN
            'Else
            '    mFPN_FCData = ""
            'End If
        Else
            'mFPN_FCData = ""
            mbIsFC = False
        End If
        mbDWS_EXIST = r.IsDWSExist
        If mbDWS_EXIST = True Then
            mCWCellColXToConnectDW = r.CWCellColXToConnectDW
            mCWCellRowYToConnectDW = r.CWCellRowYToConnectDW
            mDWCellColXToConnectCW = r.DWCellColXToConnectCW
            mDWCellRowYToConnectCW = r.DWCellRowYToConnectCW
            'If cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem = False Then
            '    mFPN_DWSS_FCDATA = r.FPNDWSSFCdata
            'End If
        End If
        mRFInterval_MIN = r.RFInterval_min
        mOutPutInterval_MIN = r.OutputInterval_min
        If RTStartDateTime = "" Then
            mSimulationStartingTime = r.RTstartingTime
        Else
            mSimulationStartingTime = RTStartDateTime
        End If
    End Function


    Public Sub SetUpAndStartGRMRT()
        'If mbIsFC = True Then
        '    If IO.File.Exists(mFPN_FCData) Then IO.File.Delete(mFPN_FCData)
        '    Dim strH As String = "CVID,DataTime,Value" & vbCrLf
        '    IO.File.AppendAllText(mFPN_FCData, strH, Encoding.Default)
        'End If
        Call SetupRT()
        UpateGRMGeneralParsByUserSettiing()
        GRMRT.RunGRMRT()
    End Sub


    Public Sub SetupRT()
        GRMRT = Nothing
        RTProject = Nothing
        cRealTime.InitializeGRMRT()
        GRMRT = cRealTime.Current
        GRMRT.SetupGRM(mProjectFPN)
        RTProject = cProject.Current
    End Sub


    ''' <summary>
    ''' 여기서 사용자 설정 환경 매개변수로 업데이트
    ''' </summary>
    ''' <remarks></remarks>
    Private Sub UpateGRMGeneralParsByUserSettiing()
        With RTProject
            .Rainfall.mRainfallinterval = mRFInterval_MIN
            .GeneralSimulEnv.mIsDateTimeFormat = True
            .mSimulationType = cGRM.SimulationType.RealTime
            If .SubWSPar.userPars(.WSNetwork.MostDownstreamWSID).iniFlow = 0 Then
                '.GeneralSimulEnv.mbSimulateBFlow = False 'todo. 이거 확인 필요. 2018.03.13. 최
            End If
        End With

        With GRMRT
            .mDtPrintOutRT_min = mOutPutInterval_MIN
            .mRFStartDateTimeRT = mSimulationStartingTime
            .mRainfallDataTypeRT = cRainfall.RainfallDataType.TextFileASCgrid
            .mRfFilePathRT = mFP_RFLayerFP
            .mPicWidth = CONST_PIC_WIDTH
            .mPicHeight = CONST_PIC_HEIGHT
            .mbIsDWSS = mbDWS_EXIST
            If .mbIsDWSS = True Then
                .mCWCellColX_ToConnectDW = mCWCellColXToConnectDW
                .mCWCellRowY_ToConnectDW = mCWCellRowYToConnectDW
                .mDWCellColX_ToConnectCW = mDWCellColXToConnectCW
                .mDWCellRowY_ToConnectCW = mDWCellRowYToConnectCW
                'If GRMCore.cRealTime.CONST_bUseDBMS_FOR_RealTimeSystem = False Then
                '    .mFPNDWssFCData = mFPN_DWSS_FCDATA
                'End If
            End If
        End With
    End Sub

    ''' <summary>
    ''' 유역 매개변수 업데이트는 여기서 수행..
    ''' </summary>
    ''' <param name="wsid"></param>
    ''' <param name="iniSat"></param>
    ''' <param name="minSlopeLandSurface"></param>
    ''' <param name="minSlopeChannel"></param>
    ''' <param name="minChannelBaseWidth"></param>
    ''' <param name="roughnessChannel"></param>
    ''' <param name="dryStreamOrder"></param>
    ''' <param name="ccLCRoughness"></param>
    ''' <param name="ccSoilDepth"></param>
    ''' <param name="ccPorosity"></param>
    ''' <param name="ccWFSuctionHead"></param>
    ''' <param name="ccSoilHydraulicCond"></param>
    ''' <param name="applyIniFlow"></param>
    ''' <param name="iniFlow"></param>
    ''' <returns></returns>
    Public Function UpdateWSPars(ByVal wsid As Integer, iniSat As Single,
                                                minSlopeLandSurface As Single,
                                                 UnsKType As String,
                                                 coefUnsK As Single,
                                                minSlopeChannel As Single,
                                                minChannelBaseWidth As Single,
                                                roughnessChannel As Single,
                                                dryStreamOrder As Integer,
                                                ccLCRoughness As Single,
                                                ccSoilDepth As Single,
                                                ccPorosity As Single,
                                                ccWFSuctionHead As Single,
                                                ccSoilHydraulicCond As Single,
                                                Optional iniFlow As Single = 0) As Boolean
        With RTProject.SubWSPar.userPars(wsid)
            .iniSaturation = iniSat
            .minSlopeOF = minSlopeLandSurface
            .UKType = UnsKType
            .coefUK = coefUnsK
            .minSlopeChBed = minSlopeChannel
            .minChBaseWidth = minChannelBaseWidth
            .chRoughness = roughnessChannel
            .dryStreamOrder = dryStreamOrder
            .ccLCRoughness = ccLCRoughness
            .ccSoilDepth = ccSoilDepth
            .ccPorosity = ccPorosity
            .ccWFSuctionHead = ccWFSuctionHead
            .ccHydraulicK = ccSoilHydraulicCond
            .iniFlow = iniFlow
            .isUserSet = True
        End With
        cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(RTProject)
    End Function

    ''' <summary>
    ''' GRM 프로젝트 파일 저장
    ''' </summary>
    ''' <remarks></remarks>
    Public Sub SaveParsToProjectFile()
        RTProject.SaveProject()
    End Sub

    ''' <summary>
    ''' 실시간 모델링 종료
    ''' </summary>
    ''' <remarks></remarks>
    Public Sub StopRTsimulation()
        GRMRT.StopGRM()
    End Sub

    Private Sub GRMRT_RTStatus(strMSG As String) Handles GRMRT.RTStatus
        RaiseEvent RTStatus(strMSG)
    End Sub

    Public ReadOnly Property PrjName As String
        Get
            Return IO.Path.GetFileNameWithoutExtension(mFPN_RTEnv)
        End Get
    End Property
End Class
