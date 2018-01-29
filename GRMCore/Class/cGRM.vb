Public Class cGRM

    Public Enum GRMOutputType
        Discharge
        Rainfall
        RainfallCumulative
        SoilSaturation
    End Enum

    Public Enum GRMPrintType
        All
        DischargeFileQ
        AllQ
    End Enum

    Public Enum FileOrConst
        File = 0
        Constant = 1
    End Enum

    Public Enum CellFlowType As Byte
        OverlandFlow
        ChannelFlow
        ChannelNOverlandFlow
    End Enum

    Public Enum SimulationType
        SingleEvent
        SingleEventPE_SSR
        RealTime
        'MultiEvents
        'RealTimeLauncher
        'MultiProject
    End Enum

    Public Enum DataType
        DTByte
        DTShort
        DTInteger
        DTSingle
        DTDouble
    End Enum

    Public Enum GRMParametersAbbreviation
        ISSR 'Initial soil saturation ratio
        MSLS 'Minimum slope of land surface
        MSCB 'Minimum slope of channel bed
        MCW 'Minimum channel width
        CRC 'Channel roughness coefficient
        CLCRC 'Calibration coefficient of Land cover roughness coefficient
        CSD 'Calibration coefficient of soil depth
        CSP 'Calibration coefficient of soil porosity
        CSWS 'Calibration coefficient of soil wetting front suction head
        CSHC 'Calibration coefficient of soil hydraulic conductivity
    End Enum

    Public Enum OutputTableName
        OutputDischarge
        OutputDepth
        RainfallWPGrid
        RainfallWPUpMean
        FCDataWP
        FCResStorage
        WPTS
    End Enum

    Public Enum GRMFlowDirectionD8 As Byte
        NE
        E
        SE
        S
        SW
        W
        NW
        N
    End Enum

    Public Enum FlowDirectionType
        StartsFromNE
        StartsFromN
        StartsFromE
        StartsFromE_TauDEM
    End Enum

    Public Shared ReadOnly LONG_DATE_FORMAT As String = "yyyy/MM/dd HH:mm"
    Public Shared fpnlog As String = ""
    Public Shared bwriteLog As Boolean

#Region "상수"
    '결과파일 Tag
    Public Const CONST_TAG_DISCHARGE As String = "Discharge.out"
    Public Const CONST_TAG_DEPTH As String = "Depth.out"
    Public Const CONST_TAG_RFGRID As String = "RFGrid.out"
    Public Const CONST_TAG_RFMEAN As String = "RFUpMean.out"
    Public Const CONST_TAG_FCAPP As String = "FCData.out"
    Public Const CONST_TAG_FCSTORAGE As String = "FCStorage.out"
    Public Const CONST_TAG_SWSPARSTEXTFILE As String = "SWSPars.out"
    Public Const CONST_OUTPUT_TABLE_TIME_FIELD_NAME As String = "DataTime"
    Public Const CONST_OUTPUT_TABLE_MEAN_RAINFALL_FIELD_NAME As String = "Rainfall_Mean"
    Public Const CONST_DIST_SSR_DIRECTORY_TAG As String = "SSD"
    Public Const CONST_DIST_RF_DIRECTORY_TAG As String = "RFD"
    Public Const CONST_DIST_RFACC_DIRECTORY_TAG As String = "RFAccD"
    Public Const CONST_DIST_FLOW_DIRECTORY_TAG As String = "FlowD"
    Public Const CONST_DIST_SSR_FILE_HEAD As String = "ss_"
    Public Const CONST_DIST_RF_FILE_HEAD As String = "rf_"
    Public Const CONST_DIST_RFACC_FILE_HEAD As String = "rfc_"
    Public Const CONST_DIST_FLOW_FILE_HEAD As String = "flow_"
    Public Const FILE_NAME_GRMMP_EXE As String = "GRMMP.exe"

    Public Const CONST_MAX_CVW As Integer = 7 '인접상류셀의 최대 개수는 7개
    'Public Const CONST_TEMPORAL_WEIGHTING_FACTOR As Single = 1

    Public Const CONST_MAX_DT_LIMIT_SEC As Integer = 600
    Public Const CONST_MIN_DT_LIMIT_SEC As Integer = 60
    Public Const CONST_GRAVITY As Single = 9.81
    Public Const CONST_CFL_NUMBER As Single = 1.0
    Public Const CONST_EXPONENTIAL_NUMBER_UNSATURATED_K As Single = 6.4


    ''' <summary>
    ''' 최상류 셀의 쐐기 흐름 계산시 p의 수심에 곱해지는 계수
    ''' </summary>
    ''' <remarks></remarks>
    Public Const CONST_WEDGE_FLOW_COEFF As Single = 1
    '''' <summary>
    '''' 저수위 적용을 위한 기준 수심[m]
    '''' </summary>
    '''' <remarks></remarks>
    'Public Const CONST_LOWFLOW_CRITERIA_DEPTH As Single = 0 '0.001
    'todo : 이거 확인 필요

    'Public Const CONST_MINIMUM_MANNING_COEFF_MULTIPLIER As Single = 0.001

    Public Const CONST_WET_AND_DRY_CRITERIA As Single = 0.000001
    Public Const CONST_TOLERANCE As Single = 0.0001

    ''' <summary>
    ''' 암반까지의 깊이[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public Const CONST_DEPTH_TO_BEDROCK As Single = 20
    ''' <summary>
    ''' 산악지역에서의 암반까지의 깊이[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public Const CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN As Single = 10
    ''' <summary>
    ''' 비피압대수층까지의 깊이[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public Const CONST_DEPTH_TO_UNCONFINED_GROUNDWATERTABEL As Single = 10
    ''' <summary>
    ''' 암반에서 비피압대수층 상단까지의 깊이[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public Const CONST_UAQ_HEIGHT_FROM_BEDROCK As Single = 5
    '''' <summary>
    '''' 지표면 흐름의 초기수심[m]
    '''' </summary>
    '''' <remarks></remarks>
    'Public Const CONST_INITIAL_OVERLANDFLOW_DEPTH As Single = 0
#End Region

    'Private Shared mRange As cParametersRange '이건 GUI에서 필요한 것
    Private Shared mBuildInfo As FileVersionInfo

#Region "DB 관련 shared 변수"
    Private Shared mGrmStarted As Boolean
    Private Shared mStaticXmlFPN As String

#End Region

    Public Shared mProcessChart As Process
#Region "메뉴 활성화 상태. shared 변수"
    Public Shared mEnableSave As Boolean
    Public Shared mEnableSaveAs As Boolean
    Public Shared mEnableProjectInfo As Boolean
    Public Shared mEnableSetWatershed As Boolean
    Public Shared mEnableSetLCST As Boolean
    Public Shared mEnableRainfall As Boolean
    Public Shared mEnableSetNRunGRM As Boolean
    Public Shared mEnableGRMME As Boolean
#End Region

    Shared Sub New()
        Dim GRMCoreFPN As String = Path.Combine(My.Application.Info.DirectoryPath, "GRMCore.dll")
        'If IO.File.Exists(My.Application.Info.DirectoryPath & "\Plugins\GRM\" & "grm.exe") Then
        '    GRMexeFPN = Path.Combine(My.Application.Info.DirectoryPath, "Plugins\GRM\" & "grm.exe")
        'ElseIf IO.File.Exists(Path.Combine(My.Application.Info.DirectoryPath, "grm.exe")) Then
        '    GRMexeFPN = Path.Combine(My.Application.Info.DirectoryPath, "grm.exe")
        'ElseIf IO.File.Exists(Path.Combine(cFile.GetAbsolutePathOneUpper(My.Application.Info.DirectoryPath), "grm.exe")) Then
        '    Dim np As String = cFile.GetAbsolutePathOneUpper(My.Application.Info.DirectoryPath)
        '    GRMexeFPN = Path.Combine(np, "grm.exe")
        'Else
        '    Console.WriteLine(String.Format("GRM plugin 파일({0})이 {1} 폴더에 없습니다.", "grm.exe", My.Application.Info.DirectoryPath & "\Plugins\GRM"))
        'End If
        mBuildInfo = FileVersionInfo.GetVersionInfo(GRMCoreFPN)
        GRMCoreFPN = Path.GetDirectoryName(GRMCoreFPN)
        'mFpnGRMStaticXmlDB = Path.Combine(GRMCoreFPN, "GRMStaticDB.xml")
        'If File.Exists(mFpnGRMStaticXmlDB) = False Then
        '    Exit Sub
        'End If
        mStaticXmlFPN = Path.Combine(GRMCoreFPN, "GRMStatic.xml")
        'mRange = New cParametersRange
        'mRange.GetValues(mFpnGRMStaticXmlDB)
    End Sub

    Public Shared Sub Start()
        mGrmStarted = True
    End Sub

    Public Shared Function GetGRMDataTypeByName(inType As String) As DataType
        Select Case inType
            Case DataType.DTByte.ToString
                Return DataType.DTByte
            Case DataType.DTShort.ToString
                Return DataType.DTShort
            Case DataType.DTInteger.ToString
                Return DataType.DTInteger
            Case DataType.DTSingle.ToString
                Return DataType.DTSingle
            Case DataType.DTDouble.ToString
                Return DataType.DTDouble
            Case Else
                Return Nothing
        End Select
    End Function

    Public Shared Function GetFDAngleNumber(fd As GRMFlowDirectionD8) As Integer
        Select Case fd
            Case GRMFlowDirectionD8.NW
                Return 315
            Case GRMFlowDirectionD8.W
                Return 270
            Case GRMFlowDirectionD8.SW
                Return 225
            Case GRMFlowDirectionD8.S
                Return 180
            Case GRMFlowDirectionD8.SE
                Return 135
            Case GRMFlowDirectionD8.E
                Return 90
            Case GRMFlowDirectionD8.NE
                Return 45
            Case GRMFlowDirectionD8.N
                Return 0
            Case Else
                Return -1
        End Select
    End Function

    Public Shared Function AboutInfo_GRM() As String
        Try
            Dim strToSay As String
            strToSay = "Product name : " + cGRM.BuildInfo.ProductName + "       " + vbCrLf
            strToSay = strToSay + "Version : " + cGRM.BuildInfo.ProductVersion + "       " + vbCrLf
            strToSay = strToSay + "File name : " + cGRM.BuildInfo.FileName + "       " + vbCrLf
            strToSay = strToSay + "File description : " + cGRM.BuildInfo.FileDescription + "       " + vbCrLf
            strToSay = strToSay + "Company name : " + cGRM.BuildInfo.CompanyName + "       " + vbCrLf
            strToSay = strToSay + "Comments : " + cGRM.BuildInfo.Comments + "       " + vbCrLf
            strToSay = strToSay + "Homepage : http://www.hygis.net" + "       " + vbCrLf
            strToSay = strToSay + "Contact : @kict.re.kr" + "       " + vbCrLf
            AboutInfo_GRM = strToSay

        Catch ex As Exception
            AboutInfo_GRM = ""
            System.Console.WriteLine(ex.ToString)
        End Try
    End Function

    Public Shared Sub writelogAndConsole(logtxt As String, Optional bwriteLog As Boolean = False, Optional bwriteConsole As Boolean = False)

        If bwriteConsole = True Then
            Console.WriteLine(String.Format(logtxt))
        End If
        If bwriteLog = True Then
            'File.AppendAllText(fpnlog,
            '                String.Format("{0:yyyy-MM-dd HH:mm ss}", DateTime.Now) + " " + logtxt + vbCrLf)
            File.AppendAllText(fpnlog, logtxt + vbCrLf)
        End If
    End Sub

#Region "Shared Properties"

    Public Shared ReadOnly Property BuildInfo() As FileVersionInfo
        Get
            Return mBuildInfo
        End Get
    End Property

    Public Shared ReadOnly Property Started() As Boolean
        Get
            Return mGrmStarted
        End Get
    End Property

#End Region

End Class
