Imports System.Text
Imports System.IO
'Imports System.Runtime.InteropServices
'Imports System.Runtime.Serialization.Formatters.Binary



Public Class cProject
    'Implements ICloneable
    Implements IDisposable

#Region "프로젝트 일반, 공통 정보"
    Private Shared mProject As cProject
    ' Project File
    'Private mOdbCnnDynamic As New OleDb.OleDbConnection

    '''' <summary>
    '''' Project Setting Objects
    '''' </summary>
    '''' <remarks></remarks>
    'Private mPrjDataSettings As ioProjectFile()

    'Public mProjectDBSettings As ioProjectDB()

    Public mSimulationType As cGRM.SimulationType
#End Region

#Region "입력된 레이어를 이용해서 설정되는 정보"
    Private mWatershed As cSetWatershed
    Private mLandcover As cSetLandcover
    Private mGreenAmpt As cSetGreenAmpt
    Private mSoilDepth As cSetSoilDepth
    Private mRainfall As cRainfall
    Private mEstimatedDist As cSetPDistribution

#End Region

#Region "SetupRunGRM UI에서 설정되는 정보"

    ''' <summary>
    ''' SetupGRM tab에서 설정되는 정보
    ''' </summary>
    ''' <remarks></remarks>
    Private mGeneralSimulEnv As cSetGeneralSimulEnvironment

    ''' <summary>
    ''' Channel tab에서 설정되는 정보
    ''' </summary>
    ''' <remarks></remarks>
    Private mChannel As cSetChannel

    ''' <summary>
    ''' Watchpoint tab에서 설정되는 정보
    ''' </summary>
    ''' <remarks></remarks>
    Private mWatchPoint As cSetWatchPoint

    ''' <summary>
    ''' Flow control tab에서 설정되는 정보
    ''' </summary>
    ''' <remarks></remarks>
    Private mFCGrid As cFlowControl

    ''' <summary>
    ''' Watershed parameter tab에서 설정되는 정보
    ''' </summary>
    ''' <remarks></remarks>
    Private mSubWSpar As cSetSubWatershedParameter

    Public ReadOnly Property MaxIniflowSetByUser() As Single
        Get
            Dim max As Single = 0
            For Each id As Integer In cProject.Current.Watershed.WSIDList
                If mProject.SubWSPar.userPars(id).isUserSet = True AndAlso
                    mProject.SubWSPar.userPars(id).iniFlow IsNot Nothing Then
                    If max < mProject.SubWSPar.userPars(id).iniFlow Then
                        max = CSng(mProject.SubWSPar.userPars(id).iniFlow)
                    End If
                End If
            Next
            Return max
        End Get
    End Property

#End Region

#Region "유역/격자 일반정보"

    Private mWSNetwork As cWatershedNetwork
    ''' <summary>
    ''' 최하류의 cv 배열 번호, cellid는 mMostDownCellArrayNumber+1
    ''' </summary>
    ''' <remarks></remarks>
    Public mMostDownCellArrayNumber As Integer
    Public mCVANsForEachFA As New cFlowAccumInfo

#End Region

#Region "관측자료용"
    Public gstObTSfromDB As cSetTSData.TimeSeriesInfoInTSDB
#End Region


#Region "프로젝트 파일등"
    Private mPrjFile As GRMProject
    'Private mProjectDBxmlFPN As String
    Private mProjectPathName As String
    Private mProjectPath As String
    Private mProjectNameWithExtension As String
    Private mProjectNameOnly As String
    Private mGeoDataPrjPathName As String

    Public Property PrjFile() As GRMProject
        Get
            Return mPrjFile
        End Get
        Set(value As GRMProject)
            mPrjFile = value
        End Set
    End Property

    'Public ReadOnly Property dtProjectSettings() As GRMProject.ProjectSettingsDataTable
    '    Get
    '        Return mPrjFile.ProjectSettings
    '    End Get
    'End Property

    'Public Property ProjectDBxmlFpn() As String
    '    Get
    '        Return mProjectDBxmlFPN
    '    End Get
    '    Set(value As String)
    '        mProjectDBxmlFPN = value
    '    End Set
    'End Property

    Public Property ProjectPathName() As String
        Get
            Return mProjectPathName
        End Get
        Set(value As String)
            mProjectPathName = value
        End Set
    End Property

    Public Property ProjectPath() As String
        Get
            Return mProjectPath
        End Get
        Set(value As String)
            mProjectPath = value
        End Set
    End Property

    Public Property ProjectNameWithExtension() As String
        Get
            Return mProjectNameWithExtension
        End Get
        Set(value As String)
            mProjectNameWithExtension = value
        End Set
    End Property

    Public Property ProjectNameOnly() As String
        Get
            Return mProjectNameOnly
        End Get
        Set(value As String)
            mProjectNameOnly = value
        End Set
    End Property

    Public Property geoDataPrjFPN As String
        Get
            Return mGeoDataPrjPathName
        End Get
        Set(value As String)
            mGeoDataPrjPathName = value
        End Set
    End Property


#End Region

#Region "모의결과 파일들"
    Private mOFNPDischarge As String 'Output File Name Path
    Private mOFNPDepth As String
    Private mOFNPRFGrid As String
    Private mOFNPRFMean As String
    Private mOFNPFCData As String
    Private mOFNPFCStorage As String
    Private mOFNPSwsPars As String
    Private mOFPSSRDistribution As String
    Private mOFPRFdist As String
    Private mOFPRFAccDist As String
    Private mOFPFlowDist As String
    Public mImgFPN_dist_SSR As List(Of String)
    Public mImgFPN_dist_RF As List(Of String)
    Public mImgFPN_dist_RFAcc As List(Of String)
    Public mImgFPN_dist_Flow As List(Of String)


    Public Property OFNPDischarge() As String
        Get
            Return mOFNPDischarge
        End Get
        Set(value As String)
            mOFNPDischarge = value
        End Set
    End Property

    Public Property OFNPDepth() As String
        Get
            Return mOFNPDepth
        End Get
        Set(value As String)
            mOFNPDepth = value
        End Set
    End Property

    Public Property OFNPRFGrid() As String
        Get
            Return mOFNPRFGrid
        End Get
        Set(value As String)
            mOFNPRFGrid = value
        End Set
    End Property

    Public Property OFNPRFMean() As String
        Get
            Return mOFNPRFMean
        End Get
        Set(value As String)
            mOFNPRFMean = value
        End Set
    End Property

    Public Property OFNPFCData() As String
        Get
            Return mOFNPFCData
        End Get
        Set(value As String)
            mOFNPFCData = value
        End Set
    End Property

    Public Property OFNPFCStorage() As String
        Get
            Return mOFNPFCStorage
        End Get
        Set(value As String)
            mOFNPFCStorage = value
        End Set
    End Property

    Public Property OFNPSwsPars() As String
        Get
            Return mOFNPSwsPars
        End Get
        Set(value As String)
            mOFNPSwsPars = value
        End Set
    End Property

    Public Property OFPSSRDistribution() As String
        Get
            Return mOFPSSRDistribution
        End Get
        Set(value As String)
            mOFPSSRDistribution = value
        End Set
    End Property

    Public Property OFPRFDistribution() As String
        Get
            Return mOFPRFdist
        End Get
        Set(value As String)
            mOFPRFdist = value
        End Set
    End Property

    Public Property OFPRFAccDistribution() As String
        Get
            Return mOFPRFAccDist
        End Get
        Set(value As String)
            mOFPRFAccDist = value
        End Set
    End Property

    Public Property OFPFlowDistribution() As String
        Get
            Return mOFPFlowDist
        End Get
        Set(value As String)
            mOFPFlowDist = value
        End Set
    End Property

    Public Property OFPRFdist() As String
        Get
            Return mOFPRFdist
        End Get
        Set(value As String)
            mOFPRFdist = value
        End Set
    End Property

    Public Property OFPRFAccDist() As String
        Get
            Return mOFPRFAccDist
        End Get
        Set(value As String)
            mOFPRFAccDist = value
        End Set
    End Property

    Public Property OFPFlowDist() As String
        Get
            Return mOFPFlowDist
        End Get
        Set(value As String)
            mOFPFlowDist = value
        End Set
    End Property

#End Region

    ''' <summary>
    ''' 검사체적 메모리 공간
    ''' </summary>
    ''' <remarks></remarks>
    Private mWSCells(,) As cCVAttribute
    Private mCVs As List(Of cCVAttribute)

    Private mTSSummary As cSetTSData

    Public Sub New()

        mWatershed = New cSetWatershed
        mLandcover = New cSetLandcover
        mGreenAmpt = New cSetGreenAmpt
        mSoilDepth = New cSetSoilDepth
        mRainfall = New cRainfall

        mEstimatedDist = New cSetPDistribution

        mGeneralSimulEnv = New cSetGeneralSimulEnvironment
        mChannel = New cSetChannel
        mWatchPoint = New cSetWatchPoint
        mFCGrid = New cFlowControl
        mSubWSpar = New cSetSubWatershedParameter
        mTSSummary = New cSetTSData
        'mPrjDataSettings = New ioProjectFile() {mGeneralSimulEnv, mSubWSpar,
        '      mWatershed, mLandcover, mGreenAmpt, mSoilDepth,
        '     mWatchPoint, mChannel, mEstimatedDist}

    End Sub

    Protected disposed As Boolean = False
    Protected Overridable Overloads Sub Dispose(ByVal disposing As Boolean)
        If Not disposed Then
            If disposing Then
                mProject = Nothing
                MyBase.Finalize()
                GC.Collect()
            End If
            ' Free any other managed objects here.
            ' Note that this is not thread safe.
        End If
        ' Free any unmanaged objects here.
        Me.disposed = True
    End Sub

    Public Overloads Sub Dispose() Implements IDisposable.Dispose
        Dispose(True)
        GC.SuppressFinalize(Me)
    End Sub

    Protected Overrides Sub Finalize()
        Dispose(False)
        MyBase.Finalize()
    End Sub

#Region "Shared Properties"
    Public Shared ReadOnly Property Current() As cProject
        Get
            Return mProject
        End Get
    End Property
#End Region



    Public ReadOnly Property Watershed() As cSetWatershed
        Get
            Return mWatershed
        End Get
    End Property

    ''' <summary>
    ''' 토지피복 정보
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks>
    ''' 외부에서 객체를 대체하지 못하도록 읽기전용으로 설정
    ''' </remarks>
    Public ReadOnly Property Landcover() As cSetLandcover
        Get
            Return mLandcover
        End Get
    End Property

    Public ReadOnly Property GreenAmpt() As cSetGreenAmpt
        Get
            Return mGreenAmpt
        End Get
    End Property

    Public ReadOnly Property SoilDepth() As cSetSoilDepth
        Get
            Return mSoilDepth
        End Get
    End Property

    Public ReadOnly Property Rainfall() As cRainfall
        Get
            Return mRainfall
        End Get
    End Property

    Public ReadOnly Property EstimatedDist() As cSetPDistribution
        Get
            Return mEstimatedDist
        End Get
    End Property

    ''' <summary>
    ''' 시물레이션 일반환경 설정
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property GeneralSimulEnv() As cSetGeneralSimulEnvironment
        Get
            Return mGeneralSimulEnv
        End Get
    End Property

    Public ReadOnly Property Channel() As cSetChannel
        Get
            Return mChannel
        End Get
    End Property

    Public ReadOnly Property WatchPoint() As cSetWatchPoint
        Get
            Return mWatchPoint
        End Get
    End Property

    Public ReadOnly Property SubWSPar() As cSetSubWatershedParameter
        Get
            Return mSubWSpar
        End Get
    End Property

    Public ReadOnly Property FCGrid() As cFlowControl
        Get
            Return mFCGrid
        End Get
    End Property

    Public ReadOnly Property TSSummary() As cSetTSData
        Get
            Return mTSSummary
        End Get
    End Property


    'Public ReadOnly Property MEvent() As cMultipleEvents
    '    Get
    '        Return mME
    '    End Get
    'End Property


    ''' <summary>
    ''' 분석을 위한 메모리격자 공간을 초기화
    ''' </summary>
    ''' <remarks>
    ''' 레이어 누락인 경우 FileNotFoundException 던짐
    ''' </remarks>
    Public Function SetBasicCVInfo() As Boolean
        Try
            If Not Watershed.IsSet OrElse Not Landcover.IsSet OrElse
                Not GreenAmpt.IsSet OrElse Not SoilDepth.IsSet Then
                Throw New InvalidOperationException
            End If
            If mProject.mSimulationType = cGRM.SimulationType.SingleEvent And Not Rainfall.IsSet Then
                Throw New InvalidOperationException
            End If
            Dim row As GRMProject.ProjectSettingsRow = CType(mPrjFile.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
            ReadLayerWSandSetBasicInfo(row.WatershedFile, cThisSimulation.IsParallel) ' 기본 격자정보와 Watershed 정보 읽기
            ReadLayerSlope(row.SlopeFile, cThisSimulation.IsParallel)
            ReadLayerFdir(row.FlowDirectionFile, cThisSimulation.IsParallel) ' Watershed Flow Dir 읽기
            ReadLayerFAcc(row.FlowAccumFile, cThisSimulation.IsParallel) ' Watershed Flow Acc 읽기
            Dim FPNstream As String = ""
            Dim FPNchannelWidth As String = ""
            Dim FPNiniSSR As String = ""
            Dim FPNiniChannelFlow As String = ""
            Dim FPNlc As String = ""
            Dim FPNst As String = ""
            Dim FPNsd As String = ""
            If row.IsStreamFileNull = False AndAlso File.Exists(row.StreamFile) = True Then
                FPNstream = row.StreamFile
                If ReadLayerStream(row.StreamFile, cThisSimulation.IsParallel) = False Then ' Watershed Stream 읽기
                    cGRM.writelogAndConsole(String.Format("Some errors were occurred while reading stream file.. {0}",
                                    row.StreamFile.ToString), True, True)
                End If
            End If
            If row.IsChannelWidthFileNull = False AndAlso File.Exists(row.ChannelWidthFile) = True Then
                FPNchannelWidth = row.ChannelWidthFile
                If ReadLayerChannelWidth(row.ChannelWidthFile, cThisSimulation.IsParallel) = False Then
                    Console.WriteLine(String.Format("Some errors were occurred while reading channel width file.. {0}",
                                                        row.ChannelWidthFile.ToString), True, True)
                End If ' 하폭 레이어 읽기
            End If
            If row.IsInitialSoilSaturationRatioFileNull = False AndAlso File.Exists(row.InitialSoilSaturationRatioFile) = True Then
                FPNiniSSR = row.InitialSoilSaturationRatioFile
                If ReadLayerInitialSoilSaturation(row.InitialSoilSaturationRatioFile, cThisSimulation.IsParallel) = False Then
                    cGRM.writelogAndConsole(String.Format("Some errors were occurred while reading initial soil saturation file.. {0}",
                                    row.InitialSoilSaturationRatioFile.ToString), True, True)
                End If
            End If
            If row.IsInitialChannelFlowFileNull = False AndAlso File.Exists(row.InitialChannelFlowFile) = True Then
                FPNiniChannelFlow = row.InitialChannelFlowFile
                If ReadLayerInitialChannelFlow(row.InitialChannelFlowFile, cThisSimulation.IsParallel) = False Then
                    cGRM.writelogAndConsole(String.Format("Some errors were occurred while reading initial channel flow file.. {0}",
                row.InitialChannelFlowFile.ToString), True, True)
                End If
            End If
            '2017.08.16. 토성, 토양심, 토지피복에서 속성 이름들 optional로 모두 수정. 단 grmcode는 필수항목으로 남겨둠.. 토지 피목별로 모의를 조금씩 다르게 할 수 있으므로..

            If Landcover.mLandCoverDataType.Equals(cGRM.FileOrConst.File) AndAlso row.IsLandCoverFileNull = False AndAlso File.Exists(row.LandCoverFile) Then
                FPNlc = row.LandCoverFile
                If ReadLayerLandCover(row.LandCoverFile, cThisSimulation.IsParallel) = False Then Return False  ' 토지피복 레이어 읽기
            Else
                If SetLandCoverAttUsingConstant(cThisSimulation.IsParallel) = False Then Return False
            End If

            If GreenAmpt.mSoilTextureDataType.Equals(cGRM.FileOrConst.File) AndAlso row.IsSoilTextureFileNull = False AndAlso File.Exists(row.SoilTextureFile) Then
                FPNst = row.SoilTextureFile
                If ReadLayerSoilTexture(row.SoilTextureFile, cThisSimulation.IsParallel) = False Then Return False ' 토성 레이어 읽기
            Else
                If SetSoilTextureAttUsingConstant(cThisSimulation.IsParallel) = False Then Return False
            End If

            If SoilDepth.mSoilDepthDataType.Equals(cGRM.FileOrConst.File) AndAlso row.IsSoilDepthFileNull = False AndAlso File.Exists(row.SoilDepthFile) Then
                FPNsd = row.SlopeFile
                If ReadLayerSoilDepth(row.SoilDepthFile, cThisSimulation.IsParallel) = False Then Return False ' 토양심 레이어 읽기
            Else
                If SetSoilDepthAttUsingConstant(cThisSimulation.IsParallel) = False Then Return False
            End If

            'ReadAllLayerAndSetInfo(row.WatershedFile,
            '                                row.SlopeFile, row.FlowDirectionFile, row.FlowAccumFile,
            '                                FPNstream, FPNchannelWidth, FPNiniSSR, FPNiniChannelFlow,
            '                                FPNlc, FPNst, FPNsd, cThisSimulation.IsParallel)
            SetGridNetworkFlowInformation() ' 인접셀의 흐름정보 설정
            InitControlVolumeAttribute() ' 초기화
            Return True
        Catch ex As Exception
            Throw ex
            Return False
        End Try

    End Function

    ''' <summary>
    ''' 계산되는 속성의 초기화
    ''' </summary>
    ''' <remarks></remarks>
    Public Sub InitControlVolumeAttribute()
        ' 최소값을 찾기 위해 최대값으로 초기화, 하도셀 중 최상류셀의 흐름누적수
        mWatershed.mFacMostUpChannelCell = CVCount
        mCVANsForEachFA.Clear()
        mWatershed.mFacMax = 0
        mWatershed.mFacMin = Integer.MaxValue
        For cvan As Integer = 0 To CVCount - 1
            With mCVs(cvan)
                .FCType = cFlowControl.FlowControlType.NONE
                .toBeSimulated = True
                '.bToEstimateParameters = False
                .DownStreamWPCVids = New List(Of Integer) '여기서 할당
                Dim deltaXw As Single
                If .FAc > 0 Then
                    deltaXw = .deltaXwSum / .NeighborCVidFlowIntoMe.Count
                Else
                    deltaXw = .DeltaXDownHalf_m
                End If
                .CVDeltaX_m = .DeltaXDownHalf_m * 2 '+ deltaXw '2015.03.12 상류 유입량이 w 끝으로 들어오는 것으로 계산..

                'FA별 상류셀개수별 array number를 저정함.
                'mCellFAInfo.Add(.FAc, .NeighborCVidFlowIntoMe.Count, cvan)

                'FA별 cvid를 저장
                mCVANsForEachFA.Add(.FAc, cvan)
                'If .NeighborCVidFlowIntoMe.Count > 4 Then
                '    cvan = cvan
                'End If

                '이건 최하류셀 정보 받는곳..
                If .FAc > mWatershed.mFacMax Then
                    mWatershed.mFacMax = .FAc
                    '최하류의 cellid를 저장함.   'CV의 배열번호는 cellid -1 이다.
                    mMostDownCellArrayNumber = cvan
                End If

                If .FAc < mWatershed.mFacMin Then
                    mWatershed.mFacMin = .FAc
                End If

                '여기서 하도 매개변수 받고
                If .FlowType = cGRM.CellFlowType.ChannelFlow Then
                    If .FAc < mWatershed.mFacMostUpChannelCell Then
                        mWatershed.mFacMostUpChannelCell = .FAc '여기서 하도셀 중 가장 작은 fa 값을 찾는다..
                    End If
                End If
            End With
        Next
    End Sub

    ''' <summary>
    ''' 자신으로 유입되는 인접셀의 개수와 cellidInWatershed를 입력함.
    ''' </summary>
    ''' <remarks></remarks>
    Public Sub SetGridNetworkFlowInformation()
        Dim DeltaXDiagonalHalf_m As Single
        Dim DeltaXPerpendicularHalf_m As Single
        '여기서 초기화
        DeltaXDiagonalHalf_m = CSng(Watershed.mCellSize * Math.Sqrt(2)) / 2
        DeltaXPerpendicularHalf_m = CSng(Watershed.mCellSize / 2)

        For row As Integer = 0 To Watershed.mRowCount - 1
            For col As Integer = 0 To Watershed.mColCount - 1
                If mWSCells(col, row) Is Nothing Then Continue For
                Dim cell As cCVAttribute = mWSCells(col, row)
                If cell.NeighborCVidFlowIntoMe Is Nothing Then cell.NeighborCVidFlowIntoMe = New List(Of Integer)
                Dim targetCell As cCVAttribute = Nothing
                'Dim deltaXSum As Single
                Dim deltaXe As Single
                Dim targetC As Integer, targetR As Integer
                With cell
                    '좌상단이 0,0 이다... 즉, 북쪽이면, row-1, 동쪽이면 col +1
                    Select Case .FDir
                        Case cGRM.GRMFlowDirectionD8.NE
                            targetC = col + 1
                            targetR = row - 1 'row + 1
                            deltaXe = DeltaXDiagonalHalf_m  '자신의 셀에서의 유출 길이(delta Eest x)를 입력한다.
                        Case cGRM.GRMFlowDirectionD8.E
                            targetC = col + 1
                            targetR = row
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case cGRM.GRMFlowDirectionD8.SE
                            targetC = col + 1
                            targetR = row + 1 'row - 1
                            deltaXe = DeltaXDiagonalHalf_m
                        Case cGRM.GRMFlowDirectionD8.S
                            targetC = col
                            targetR = row + 1 'row - 1
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case cGRM.GRMFlowDirectionD8.SW
                            targetC = col - 1
                            targetR = row + 1 'row - 1
                            deltaXe = DeltaXDiagonalHalf_m
                        Case cGRM.GRMFlowDirectionD8.W
                            targetC = col - 1
                            targetR = row
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case cGRM.GRMFlowDirectionD8.NW
                            targetC = col - 1
                            targetR = row - 1 'row + 1
                            deltaXe = DeltaXDiagonalHalf_m
                        Case cGRM.GRMFlowDirectionD8.N
                            targetC = col
                            targetR = row - 1 'row + 1
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case Else
                            Throw New InvalidOperationException
                    End Select
                    '좌하단을 0,0으로 설정하는 경우, 즉, 북쪽이면, row+1, 동쪽이면 col +1
                    'Select Case .FDir
                    '    Case cCVAttribute.GRMFlowDirection.NE
                    '        targetC = col + 1
                    '        targetR = row + 1
                    '        deltaXe = DeltaXDiagonalHalf_m  '자신의 셀에서의 유출 길이(delta Eest x)를 입력한다.
                    '    Case cCVAttribute.GRMFlowDirection.E
                    '        targetC = col + 1
                    '        targetR = row
                    '        deltaXe = DeltaXPerpendicularHalf_m
                    '    Case cCVAttribute.GRMFlowDirection.SE
                    '        targetC = col + 1
                    '        targetR = row - 1
                    '        deltaXe = DeltaXDiagonalHalf_m
                    '    Case cCVAttribute.GRMFlowDirection.S
                    '        targetC = col
                    '        targetR = row - 1
                    '        deltaXe = DeltaXPerpendicularHalf_m
                    '    Case cCVAttribute.GRMFlowDirection.SW
                    '        targetC = col - 1
                    '        targetR = row - 1
                    '        deltaXe = DeltaXDiagonalHalf_m
                    '    Case cCVAttribute.GRMFlowDirection.W
                    '        targetC = col - 1
                    '        targetR = row
                    '        deltaXe = DeltaXPerpendicularHalf_m
                    '    Case cCVAttribute.GRMFlowDirection.NW
                    '        targetC = col - 1
                    '        targetR = row + 1
                    '        deltaXe = DeltaXDiagonalHalf_m
                    '    Case cCVAttribute.GRMFlowDirection.N
                    '        targetC = col
                    '        targetR = row + 1
                    '        deltaXe = DeltaXPerpendicularHalf_m
                    '    Case Else
                    '        Throw New InvalidOperationException
                    'End Select

                    If IsInBound(targetC, targetR) Then
                        targetCell = mWSCells(targetC, targetR)
                        If targetCell Is Nothing Then '이경우는 최하류만 해당
                            mWSNetwork.SetWSoutletCVID(.WSID, .CVID)
                        Else
                            If targetCell.NeighborCVidFlowIntoMe Is Nothing Then targetCell.NeighborCVidFlowIntoMe = New List(Of Integer)
                            targetCell.NeighborCVidFlowIntoMe.Add(.CVID) '여기서 source의 cellid를 기록하고
                            targetCell.deltaXwSum = targetCell.deltaXwSum + deltaXe '하류셀에 셀사이즈의 반을 더하고(하류셀 입장에서는 delta West x)
                            .DownCellidToFlow = targetCell.CVID  '여기서 흘러갈 방향의 cellid를 기록함
                            If Not (.WSID = targetCell.WSID) Then
                                If mWSNetwork.WSIDsNearbyDown(.WSID) <> targetCell.WSID Then
                                    mWSNetwork.AddWSIDdown(.WSID, targetCell.WSID)
                                    mWSNetwork.SetWSoutletCVID(.WSID, .CVID)
                                End If

                                If Not mWSNetwork.WSIDsNearbyUp(targetCell.WSID).Contains(.WSID) Then
                                    mWSNetwork.AddWSIDup(targetCell.WSID, .WSID)
                                End If

                            End If
                        End If
                        .DeltaXDownHalf_m = deltaXe  '자신의 셀에서의 유출 길이(delta Eest x)를 입력한다.
                    End If

                End With
            Next col
        Next row
        mWSNetwork.UpdateAllDownsAndUpsNetwork()
    End Sub

    Public Function getAllUpstreamCells(startingBaseCVID As Integer) As List(Of Integer)
        Try
            Dim bAllisEnded As Boolean = False
            Dim cvidsOfAllUpstreamCells As New List(Of Integer)
            Dim baseCVids As List(Of Integer)
            Dim currentUpstreamCells As List(Of Integer)
            baseCVids = New List(Of Integer)
            cvidsOfAllUpstreamCells.Add(startingBaseCVID)
            baseCVids.Add(startingBaseCVID)
            Do Until bAllisEnded = True
                Dim intCountAL As Integer = baseCVids.Count
                currentUpstreamCells = New List(Of Integer)
                bAllisEnded = True
                For Each baseCVID As Integer In baseCVids
                    Dim intNowCVArrayNum As Integer = CInt(baseCVID) - 1 ' CInt(alBaseCVidSelected.Item(intNCV)) - 1
                    Dim countUpCells As Integer = mCVs(intNowCVArrayNum).NeighborCVidFlowIntoMe.Count
                    If countUpCells > 0 Then
                        '한번이라도 이 경우 있으면.. 상류 격자가 있다는 의미이므로.. bAllisEnded = False 으로 설정한다.
                        bAllisEnded = False
                        For Each CVidFlowIntoMe As Integer In mCVs(intNowCVArrayNum).NeighborCVidFlowIntoMe
                            Dim upArrayNum As Integer = CVidFlowIntoMe - 1
                            Dim colX As Integer = mCVs(upArrayNum).XCol
                            Dim rowY As Integer = mCVs(upArrayNum).YRow
                            currentUpstreamCells.Add(upArrayNum + 1) '.CVid == Cellid == CVArrayNumber+1
                            cvidsOfAllUpstreamCells.Add(upArrayNum + 1)
                        Next
                    End If
                Next
                baseCVids = New List(Of Integer)
                baseCVids = currentUpstreamCells
            Loop
            Return cvidsOfAllUpstreamCells
        Catch ex As Exception
            Return Nothing
        End Try
    End Function

    Private Function IsInBound(ByVal colIdx As Integer, ByVal rowIdy As Integer) As Boolean
        Return colIdx >= 0 AndAlso colIdx < Watershed.mColCount AndAlso rowIdy >= 0 AndAlso rowIdy < Watershed.mRowCount
    End Function


    'Public Function ReadAllLayerAndSetInfo(fpnWs As String, fpnSlope As String, fpnFdir As String,
    '                                       fpnFac As String, fpnStream As String, fpnChannelWidth As String,
    '                                       fpnIniSSR As String, fpnIniChannelFlow As String,
    '                                       fpnLc As String, fpnSt As String, fpnSd As String, isparallel As Boolean) As Boolean
    '    If File.Exists(fpnWs) = False Then
    '        Throw New FileNotFoundException(fpnWs)
    '        Return False
    '    End If
    '    If File.Exists(fpnSlope) = False Then
    '        Throw New FileNotFoundException(fpnWs)
    '        Return False
    '    End If
    '    If File.Exists(fpnFdir) = False Then
    '        Throw New FileNotFoundException(fpnWs)
    '        Return False
    '    End If
    '    If File.Exists(fpnFac) = False Then
    '        Throw New FileNotFoundException(fpnWs)
    '        Return False
    '    End If

    '    Dim gridWS As New cTextFileReaderASC(fpnWs)
    '    Dim gridSlope As New cTextFileReaderASC(fpnSlope)
    '    Dim gridFdir As New cTextFileReaderASC(fpnFdir)
    '    Dim gridFac As New cTextFileReaderASC(fpnFac)
    '    Dim gridStream As cTextFileReaderASC
    '    Dim gridCHWidth As cTextFileReaderASC
    '    Dim ascIniSSR As cTextFileReaderASC
    '    Dim ascIniChFlow As cTextFileReaderASC
    '    Dim gridLC As cTextFileReaderASC
    '    Dim gridSTexture As cTextFileReaderASC
    '    Dim gridSDepth As cTextFileReaderASC

    '    If fpnStream <> "" Then
    '        gridStream = New cTextFileReaderASC(fpnStream)
    '    End If

    '    'Dim gridCHWidth As New cTextFileReaderASC(fpnCHw)
    '    'Dim ascIniSSR As New cTextFileReaderASC(fpn)
    '    'Dim ascIniChFlow As New cTextFileReaderASC(fpn)
    '    'Dim gridLC As New cTextFileReaderASC(fpnLc)
    '    'Dim gridSTexture As New cTextFileReaderASC(fpnSt)
    '    'Dim gridSDepth As New cTextFileReaderASC(fpnSd)
    'End Function


    ''' <summary>
    ''' Watershed 레이어를 읽어서 유역 기본정보 설정
    ''' </summary>
    ''' <remarks>
    ''' 레이어가 없는 경우는 FileNotFoundException 던짐
    ''' </remarks>
    Public Function ReadLayerWSandSetBasicInfo(fpnWS As String, isparallel As Boolean) As Boolean
        If File.Exists(fpnWS) = False Then
            Throw New FileNotFoundException(fpnWS)
            Return False
        End If
        Dim gridWS As New cTextFileReaderASC(fpnWS)
        ' 그리드 해더정보 설정
        mWatershed.mRowCount = gridWS.Header.numberRows
        mWatershed.mColCount = gridWS.Header.numberCols
        mWatershed.mCellSize = CInt(gridWS.Header.cellsize)
        mWatershed.mxllcorner = gridWS.Header.xllcorner
        mWatershed.myllcorner = gridWS.Header.yllcorner
        ' dim ary(n) 하면, vb.net에서는 0~n까지 n+1개의 배열요소 생성
        'c#에서는 0~(n-1) 까지 n 개의 요소 생성
        mWSCells = New cCVAttribute(mWatershed.mColCount - 1, mWatershed.mRowCount - 1) {}
        mCVs = New List(Of cCVAttribute)
        ' 메모리 공간 생성
        Dim cvid As Integer = 0
        If isparallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               Dim valuesInaLine() As String = gridWS.ValuesInOneRowFromTopLeft(ry)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   Dim wsid As Integer = CInt(valuesInaLine(cx))
                                                                   'Dim wsid As Integer = CInt(gridWS.ValueFromTL(cx, ry))
                                                                   If wsid > 0 Then '유역 내부
                                                                       Dim cv As New cCVAttribute
                                                                       cv.WSID = wsid
                                                                       mWSCells(cx, ry) = cv
                                                                   End If
                                                               Next cx
                                                           End Sub)
            For ry As Integer = 0 To mWatershed.mRowCount - 1
                For cx As Integer = 0 To mWatershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        Dim wsid As Integer = mWSCells(cx, ry).WSID
                        With mWSCells(cx, ry)
                            cvid += 1
                            .CVID = cvid
                            .FlowType = cGRM.CellFlowType.OverlandFlow
                            .XCol = cx
                            .YRow = ry
                            If Not mWatershed.WSIDList.Contains(.WSID) Then
                                mWatershed.WSIDList.Add(.WSID)
                            End If
                        End With
                        If mWatershed.mCVidListForEachWS.ContainsKey(wsid) = False Then
                            mWatershed.mCVidListForEachWS.Add(wsid, New List(Of Integer))
                        End If
                        mWatershed.mCVidListForEachWS(wsid).Add(cvid)
                        CVs.Add(mWSCells(cx, ry))
                    End If
                Next
            Next
        Else
            For ry As Integer = 0 To mWatershed.mRowCount - 1
                Dim valuesInaLine() As String = gridWS.ValuesInOneRowFromTopLeft(ry)
                For cx As Integer = 0 To mWatershed.mColCount - 1
                    'Dim wsid As Integer = CInt(gridWS.ValueFromTL(cx, ry))
                    Dim wsid As Integer = CInt(valuesInaLine(cx))
                    If wsid > 0 Then '유역 내부
                        Dim cv As New cCVAttribute
                        With cv
                            .WSID = wsid
                            .CVID = cvid + 1 ' mCVs.Count + 1 'CVid를 CV 리스트(mCVs)의 인덱스 번호 +1 의 값으로 입력한다. 
                            .FlowType = cGRM.CellFlowType.OverlandFlow
                            .XCol = cx
                            .YRow = ry
                            If Not mWatershed.WSIDList.Contains(.WSID) Then
                                mWatershed.WSIDList.Add(.WSID)
                            End If
                        End With
                        If mWatershed.mCVidListForEachWS.ContainsKey(wsid) = False Then
                            mWatershed.mCVidListForEachWS.Add(wsid, New List(Of Integer))
                        End If
                        mWatershed.mCVidListForEachWS(wsid).Add(cv.CVID)
                        mWSCells(cx, ry) = cv
                        CVs.Add(cv)
                        cvid += 1
                    End If
                Next
            Next
        End If
        mWSNetwork = New cWatershedNetwork(mWatershed.WSIDList)
        mSubWSpar.SetSubWSkeys(mWatershed.WSIDList)
    End Function


    ''' <summary>
    ''' Slope 레이어 읽기
    ''' </summary>
    ''' <returns>오류가 있는 경우 False</returns>
    ''' <remarks></remarks>
    Public Function ReadLayerSlope(fpnSlope As String, isParallel As Boolean) As Boolean
        If File.Exists(fpnSlope) = False Then
            Throw New FileNotFoundException(fpnSlope)
            Return False
        End If
        Dim gridSlope As New cTextFileReaderASC(fpnSlope)
        Dim bNoError As Boolean = True
        If isParallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               Dim valuesInaLine() As String = gridSlope.ValuesInOneRowFromTopLeft(ry)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   If mWSCells(cx, ry) IsNot Nothing Then
                                                                       mWSCells(cx, ry).Slope = CSng(valuesInaLine(cx))
                                                                       'mWSCells(cx, ry).Slope = CSng(gridSlope.ValueFromTL(cx, ry))
                                                                       If mWSCells(cx, ry).Slope <= 0.0 Then
                                                                           mWSCells(cx, ry).Slope = 0.000001
                                                                           bNoError = False
                                                                       End If
                                                                   End If
                                                               Next
                                                           End Sub)

        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                Dim valuesInaLine() As String = gridSlope.ValuesInOneRowFromTopLeft(ry)
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        mWSCells(cx, ry).Slope = CSng(valuesInaLine(cx))
                        'mWSCells(cx, ry).Slope = CSng(gridSlope.ValueFromTL(cx, ry))
                        If mWSCells(cx, ry).Slope <= 0.0 Then
                            mWSCells(cx, ry).Slope = 0.000001
                            bNoError = False
                        End If
                    End If
                Next cx
            Next ry
        End If

        Return bNoError
    End Function

    ''' <summary>
    ''' Flow Dir 레이어 읽기
    ''' </summary>
    ''' <remarks></remarks>
    Public Function ReadLayerFdir(fpnFdir As String, isParallel As Boolean) As Boolean
        If File.Exists(fpnFdir) = False Then
            Throw New FileNotFoundException(fpnFdir)
            Return False
        End If
        Dim gridFdir As New cTextFileReaderASC(fpnFdir)
        If isParallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               Dim valuesInaLine() As String = gridFdir.ValuesInOneRowFromTopLeft(ry)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   If mWSCells(cx, ry) IsNot Nothing Then
                                                                       'If CInt(valuesInaLine(cx)) < 0 Then
                                                                       '    mWSCells(cx, ry) = Nothing
                                                                       'Else
                                                                       mWSCells(cx, ry).FDir = cHydroCom.GetFlowDirection(CInt(valuesInaLine(cx)), Watershed.mFDType)
                                                                       'mWSCells(cx, ry).FDir = cHydroCom.GetFlowDirection(CInt(gridFdir.ValueFromTL(cx, ry)), Watershed.mFDType)
                                                                       'End If
                                                                   End If
                                                               Next
                                                           End Sub)
        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                Dim valuesInaLine() As String = gridFdir.ValuesInOneRowFromTopLeft(ry)
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        'If CInt(valuesInaLine(cx)) < 0 Then
                        '    mWSCells(cx, ry) = Nothing
                        'Else
                        mWSCells(cx, ry).FDir = cHydroCom.GetFlowDirection(CInt(valuesInaLine(cx)), Watershed.mFDType)
                        'mWSCells(cx, ry).FDir = cHydroCom.GetFlowDirection(CInt(gridFdir.ValueFromTL(cx, ry)), Watershed.mFDType)
                        'End If
                    End If
                Next cx
            Next ry
        End If
        Return True
    End Function

    ''' <summary>
    ''' Flow Acc 레이어 읽기
    ''' </summary>
    ''' <remarks></remarks>
    Public Function ReadLayerFAcc(fpnFac As String, isParallel As Boolean) As Boolean
        If File.Exists(fpnFac) = False Then
            Throw New FileNotFoundException(fpnFac)
            Return False
        End If
        Dim gridFac As New cTextFileReaderASC(fpnFac)
        If isParallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               Dim valuesInaLine() As String = gridFac.ValuesInOneRowFromTopLeft(ry)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   If mWSCells(cx, ry) IsNot Nothing Then
                                                                       'If CInt(valuesInaLine(cx)) < 0 Then
                                                                       '    mWSCells(cx, ry) = Nothing
                                                                       'Else
                                                                       mWSCells(cx, ry).FAc = CInt(valuesInaLine(cx))
                                                                       'mWSCells(cx, ry).FAc = CInt(gridFac.ValueFromTL(cx, ry))
                                                                       'End If
                                                                   End If
                                                               Next
                                                           End Sub)

        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                Dim valuesInaLine() As String = gridFac.ValuesInOneRowFromTopLeft(ry)
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        'If cx = 59 AndAlso ry = 830 Then Stop
                        'If CInt(valuesInaLine(cx)) < 0 Then
                        '    mCVs.RemoveAt(mWSCells(cx, ry).CVID - 1)
                        '    mWSCells(cx, ry) = Nothing

                        'Else
                        mWSCells(cx, ry).FAc = CInt(valuesInaLine(cx))
                        'End If
                    End If
                Next cx
            Next ry
        End If
        Return True
    End Function

    ''' <summary>
    ''' Stream 레이어 읽기
    ''' </summary>
    ''' <remarks></remarks>
    Public Function ReadLayerStream(fpnStream As String, isParallel As Boolean) As Boolean
        Try
            Dim gridStream As New cTextFileReaderASC(fpnStream)
            'Dim count As Integer = 0
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridStream.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           'If value < 0 Then
                                                                           '    mWSCells(cx, ry) = Nothing
                                                                           'Else
                                                                           If value > 0 Then
                                                                               mWSCells(cx, ry).FlowType = cGRM.CellFlowType.ChannelFlow
                                                                               With mWSCells(cx, ry)
                                                                                   .mStreamAttr = New cCVStreamAttribute
                                                                                   .mStreamAttr.ChStrOrder = value
                                                                                   .mStreamAttr.chBedSlope = .SlopeOF
                                                                               End With
                                                                           End If
                                                                           'End If
                                                                       End If
                                                                   Next
                                                               End Sub)

            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridStream.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            'If value < 0 Then
                            '    mWSCells(cx, ry) = Nothing
                            'Else
                            If value > 0 Then
                                mWSCells(cx, ry).FlowType = cGRM.CellFlowType.ChannelFlow
                                With mWSCells(cx, ry)
                                    .mStreamAttr = New cCVStreamAttribute
                                    .mStreamAttr.ChStrOrder = value
                                    .mStreamAttr.chBedSlope = .SlopeOF
                                End With
                                'count += 1
                            End If
                            'End If
                        End If
                    Next cx
                Next ry
                'mWatershed.mCellCountInChannel = count
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    ''' <summary>
    ''' 하폭 레이어 읽기
    ''' </summary>
    ''' <remarks></remarks>
    Private Function ReadLayerChannelWidth(fpnCHw As String, isParallel As Boolean) As Boolean
        Try
            Dim gridCHWidth As New cTextFileReaderASC(fpnCHw)

            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridCHWidth.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim value As Single = CSng(valuesInaLine(cx))
                                                                           If value < 0 Then
                                                                               mWSCells(cx, ry).mStreamAttr.ChBaseWidthByLayer = 0
                                                                           Else
                                                                               mWSCells(cx, ry).mStreamAttr.ChBaseWidthByLayer = value
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridCHWidth.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim value As Single = CSng(valuesInaLine(cx))
                            If value < 0 Then
                                mWSCells(cx, ry).mStreamAttr.ChBaseWidthByLayer = 0
                            Else
                                mWSCells(cx, ry).mStreamAttr.ChBaseWidthByLayer = value
                            End If
                        End If
                    Next cx
                Next ry
            End If

            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function


    Public Function ReadLayerInitialSoilSaturation(ByVal fpn As String, isParallel As Boolean) As Boolean
        Try
            Dim ascIniSSR As New cTextFileReaderASC(fpn)
            Dim n As Integer = 0
            Dim sum As Single = 0

            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = ascIniSSR.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) Is Nothing Then
                                                                           Dim v As Single = CSng(valuesInaLine(cx))
                                                                           If v < 0 Then v = 0
                                                                           If v > 1 Then v = 1
                                                                           mWSCells(cx, ry).InitialSaturation = v
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = ascIniSSR.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) Is Nothing Then
                            Dim v As Single = CSng(valuesInaLine(cx))
                            'Dim v As Single = ascIniSSR.ValueFromTL(cx, ry)
                            If v < 0 Then v = 0
                            If v > 1 Then v = 1
                            mWSCells(cx, ry).InitialSaturation = v
                        End If
                    Next
                Next
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Public Function ReadLayerInitialChannelFlow(ByVal fpn As String, isParallel As Boolean) As Boolean
        Try
            Dim ascIniChFlow As New cTextFileReaderASC(fpn)
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = ascIniChFlow.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim value As Single = CSng(valuesInaLine(cx))
                                                                           If value > 0 Then
                                                                               mWSCells(cx, ry).mStreamAttr.initialQCVch_i_j_m3Ps = value
                                                                           Else
                                                                               mWSCells(cx, ry).mStreamAttr.initialQCVch_i_j_m3Ps = 0
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = ascIniChFlow.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim value As Single = CSng(valuesInaLine(cx))
                            'Dim value As Single = CSng(ascIniChFlow.ValueFromTL(cx, ry))
                            If value > 0 Then
                                mWSCells(cx, ry).mStreamAttr.initialQCVch_i_j_m3Ps = value
                            Else
                                mWSCells(cx, ry).mStreamAttr.initialQCVch_i_j_m3Ps = 0
                            End If
                        End If
                    Next
                Next
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Public Function ReadLandCoverFile(fpnLC As String, isParallel As Boolean) As Boolean
        Try
            Dim gridLC As New cTextFileReaderASC(fpnLC)

            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridLC.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim cell As cCVAttribute = mWSCells(cx, ry) '참조하는 시간 절약, 한번만 참조해서 사용한다.
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           'value = CInt(gridLC.ValueFromTL(cx, ry))
                                                                           If value > 0 Then
                                                                               cell.LandCoverValue = value
                                                                           Else
                                                                               Throw New KeyNotFoundException
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridLC.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim cell As cCVAttribute = mWSCells(cx, ry) '참조하는 시간 절약, 한번만 참조해서 사용한다.
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            'value = CInt(gridLC.ValueFromTL(cx, ry))
                            If value > 0 Then
                                cell.LandCoverValue = value
                            Else
                                Throw New KeyNotFoundException
                            End If
                        End If
                    Next cx
                Next ry
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    ''' <summary>
    ''' 토지피복레이어 값 읽기
    ''' </summary>
    ''' <remarks></remarks>
    Private Function ReadLayerLandCover(fpnLC As String, isParallel As Boolean) As Boolean
        If Not Landcover.IsSet Then Return False
        Dim vatRC As New SortedList(Of Integer, Single)
        Dim vatIR As New SortedList(Of Integer, Single)
        Dim vatLCcode As New SortedList(Of Integer, cSetLandcover.LandCoverCode)
        For Each row As GRMProject.LandCoverRow In Landcover.mdtLandCoverInfo
            vatRC.Add(CInt(row.GridValue), row.RoughnessCoefficient)
            vatIR.Add(CInt(row.GridValue), row.ImperviousRatio)
            Dim lcCode As cSetLandcover.LandCoverCode
            If Not row.IsGRMCodeNull Then
                lcCode = cSetLandcover.GetLandCoverCode(row.GRMCode)
            Else
                cGRM.writelogAndConsole(String.Format("Landcover attribute code was not set for {0}. ", row.GridValue), True, True)
                Return False
            End If
            vatLCcode.Add(CInt(row.GridValue), lcCode)
        Next
        Dim gridLC As New cTextFileReaderASC(fpnLC)
        'Dim value As Integer
        Try
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridLC.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim cell As cCVAttribute = mWSCells(cx, ry)
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           'value = CInt(gridLC.ValueFromTL(cx, ry))
                                                                           If value > 0 Then
                                                                               cell.LandCoverValue = value
                                                                               cell.RoughnessCoeffOFori = vatRC(value)
                                                                               cell.ImperviousRatio = vatIR(value)
                                                                               cell.LandCoverCode = vatLCcode(value)
                                                                           Else
                                                                               Throw New KeyNotFoundException
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridLC.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim cell As cCVAttribute = mWSCells(cx, ry)
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            'value = CInt(gridLC.ValueFromTL(cx, ry))
                            If value > 0 Then
                                cell.LandCoverValue = value
                                cell.RoughnessCoeffOFori = vatRC(value)
                                cell.ImperviousRatio = vatIR(value)
                                cell.LandCoverCode = vatLCcode(value)
                            Else
                                Throw New KeyNotFoundException
                            End If
                        End If
                    Next cx
                Next ry
            End If
            Return True
        Catch ex As KeyNotFoundException
            Throw New KeyNotFoundException(String.Format(
                                               "Landcover file {0} has an invalid value.", Landcover.mGridLandCoverFPN))
            Return False
        End Try
    End Function

    Public Function SetLandCoverAttUsingConstant(isParallel As Boolean) As Boolean
        If isParallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   If mWSCells(cx, ry) IsNot Nothing Then
                                                                       Dim cell As cCVAttribute = mWSCells(cx, ry)
                                                                       cell.LandCoverValue = 0     ' 상수를 의미
                                                                       cell.RoughnessCoeffOFori = Landcover.mConstRoughnessCoefficient.Value
                                                                       cell.ImperviousRatio = Landcover.mConstImperviousRatio.Value
                                                                       cell.LandCoverCode = cSetLandcover.LandCoverCode.CONST_VALUE
                                                                   End If
                                                               Next
                                                           End Sub)
        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        Dim cell As cCVAttribute = mWSCells(cx, ry)
                        cell.LandCoverValue = 0    ' 상수를 의미
                        cell.RoughnessCoeffOFori = Landcover.mConstRoughnessCoefficient.Value
                        cell.ImperviousRatio = Landcover.mConstImperviousRatio.Value
                        cell.LandCoverCode = cSetLandcover.LandCoverCode.CONST_VALUE
                    End If
                Next cx
            Next ry
        End If
        Return True
    End Function

    Public Function ReadSoilTextureFile(fpnST As String, isParallel As Boolean) As Boolean
        Try
            Dim gridSTexture As New cTextFileReaderASC(fpnST)
            'Dim value As Integer
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridSTexture.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim cell As cCVAttribute = mWSCells(cx, ry)
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           'Dim value As Integer = CInt(gridSTexture.ValueFromTL(cx, ry))
                                                                           If value > 0 Then
                                                                               cell.SoilTextureValue = value
                                                                           Else
                                                                               Throw New KeyNotFoundException
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridSTexture.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim cell As cCVAttribute = mWSCells(cx, ry)
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            'value = CInt(gridSTexture.ValueFromTL(cx, ry))
                            If value > 0 Then
                                cell.SoilTextureValue = value
                            Else
                                Throw New KeyNotFoundException
                            End If
                        End If
                    Next cx
                Next ry
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try

    End Function


    ''' <summary>
    ''' 토성레이어 값 읽기
    ''' </summary>
    ''' <remarks></remarks>
    Private Function ReadLayerSoilTexture(fpnST As String, isParallel As Boolean) As Boolean
        If Not GreenAmpt.IsSet Then Return False
        Dim gridSTexture As New cTextFileReaderASC(fpnST)
        Dim vatP As New SortedList(Of Integer, Single)
        Dim vatEP As New SortedList(Of Integer, Single)
        Dim vatWFSH As New SortedList(Of Integer, Single)
        Dim vatHC As New SortedList(Of Integer, Single)
        Dim vatSTcode As New SortedList(Of Integer, cSetGreenAmpt.SoilTextureCode)

        For Each row As GRMProject.GreenAmptParameterRow In GreenAmpt.mdtGreenAmptInfo
            vatP.Add(CInt(row.GridValue), row.Porosity)
            vatEP.Add(CInt(row.GridValue), row.EffectivePorosity)
            vatWFSH.Add(CInt(row.GridValue), row.WFSoilSuctionHead)
            vatHC.Add(CInt(row.GridValue), row.HydraulicConductivity)
            Dim stCode As cSetGreenAmpt.SoilTextureCode
            If Not row.IsGRMCodeNull Then
                stCode = cSetGreenAmpt.GetSoilTextureCode(row.GRMCode.ToString)
            Else
                cGRM.writelogAndConsole(String.Format("Soil texturer attribute code was not set for {0}. ", row.GridValue), True, True)
                Return False
            End If

            vatSTcode.Add(CInt(row.GridValue), stCode)
        Next

        'Dim value As Integer
        Try
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridSTexture.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim cell As cCVAttribute = mWSCells(cx, ry)
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           'Dim value As Integer = CInt(gridSTexture.ValueFromTL(cx, ry))
                                                                           If value > 0 Then
                                                                               cell.SoilTextureValue = value
                                                                               cell.PorosityEtaOri = vatP(value)
                                                                               cell.EffectivePorosityThetaEori = vatEP(value)
                                                                               cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH(value) / 100  ' 단위환산 cm -> m
                                                                               cell.HydraulicConductKori_mPsec = vatHC(value) / 100 / 3600    ' 단위환산  cm/hr -> m/s
                                                                               cell.SoilTextureCode = vatSTcode(value)
                                                                           Else
                                                                               Throw New KeyNotFoundException
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridSTexture.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim cell As cCVAttribute = mWSCells(cx, ry)
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            'value = CInt(gridSTexture.ValueFromTL(cx, ry))
                            If value > 0 Then
                                cell.SoilTextureValue = value
                                cell.PorosityEtaOri = vatP(value)
                                cell.EffectivePorosityThetaEori = vatEP(value)
                                cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH(value) / 100  ' 단위환산 cm -> m
                                cell.HydraulicConductKori_mPsec = vatHC(value) / 100 / 3600    ' 단위환산  cm/hr -> m/s
                                cell.SoilTextureCode = vatSTcode(value)
                            Else
                                Throw New KeyNotFoundException
                            End If
                        End If
                    Next cx
                Next ry
            End If
            Return True
        Catch ex As KeyNotFoundException
            Throw New KeyNotFoundException(String.Format(
                                           "Soil texture file {0} has an invalid value.", GreenAmpt.mGridSoilTextureFPN))
            Return False
        End Try
    End Function


    Public Function SetSoilTextureAttUsingConstant(isParallel As Boolean) As Boolean
        If isParallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   If mWSCells(cx, ry) IsNot Nothing Then
                                                                       Dim cell As cCVAttribute = mWSCells(cx, ry)
                                                                       cell.SoilTextureValue = 0  ' 상수를 의미
                                                                       cell.PorosityEtaOri = GreenAmpt.mConstPorosity.Value
                                                                       cell.EffectivePorosityThetaEori = GreenAmpt.mConstEffectivePorosity.Value
                                                                       cell.WettingFrontSuctionHeadPsiOri_m = GreenAmpt.mConstWFS.Value / 100  ' 단위환산 cm -> m
                                                                       cell.HydraulicConductKori_mPsec = GreenAmpt.mConstHydraulicCond.Value / 100 / 3600    ' 단위환산  cm/hr -> m/s
                                                                   End If
                                                               Next
                                                           End Sub)
        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        Dim cell As cCVAttribute = mWSCells(cx, ry)
                        cell.SoilTextureValue = 0  ' 상수를 의미
                        cell.PorosityEtaOri = GreenAmpt.mConstPorosity.Value
                        cell.EffectivePorosityThetaEori = GreenAmpt.mConstEffectivePorosity.Value
                        cell.WettingFrontSuctionHeadPsiOri_m = GreenAmpt.mConstWFS.Value / 100  ' 단위환산 cm -> m
                        cell.HydraulicConductKori_mPsec = GreenAmpt.mConstHydraulicCond.Value / 100 / 3600    ' 단위환산  cm/hr -> m/s
                    End If
                Next cx
            Next ry
        End If
        Return True
    End Function

    Public Function ReadSoilDepthFile(fpnSD As String, isParallel As Boolean) As Boolean

        Dim gridSDepth As New cTextFileReaderASC(fpnSD)
        Try
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridSDepth.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           'Dim value As Integer = CInt(gridSDepth.ValueFromTL(cx, ry))
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           If value > 0 Then
                                                                               mWSCells(cx, ry).SoilDepthTypeValue = CInt(value)
                                                                           Else
                                                                               Throw New KeyNotFoundException
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridSDepth.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            'value = CInt(gridSDepth.ValueFromTL(cx, ry))
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            If value > 0 Then
                                mWSCells(cx, ry).SoilDepthTypeValue = CInt(value)
                            Else
                                Throw New KeyNotFoundException
                            End If
                        End If
                    Next cx
                Next ry
            End If
            Return True
        Catch ex As KeyNotFoundException
            Return False
        End Try
    End Function

    ''' <summary>
    ''' 토양심 레이어 값 읽기
    ''' </summary>
    ''' <remarks></remarks>
    Private Function ReadLayerSoilDepth(fpnSD As String, isParallel As Boolean) As Boolean
        If Not SoilDepth.IsSet Then Return False
        Dim gridSDepth As New cTextFileReaderASC(fpnSD)
        Dim vatSD As New SortedList(Of Integer, Single)
        Dim vatSDcode As New SortedList(Of Integer, cSetSoilDepth.SoilDepthCode)
        For Each row As GRMProject.SoilDepthRow In SoilDepth.mdtSoilDepthInfo
            vatSD.Add(CInt(row.GridValue), row.SoilDepth)
            Dim stCode As cSetSoilDepth.SoilDepthCode
            If Not row.IsGRMCodeNull Then
                stCode = cSetSoilDepth.GetSoilDepthCode(row.GRMCode)
            Else
                cGRM.writelogAndConsole(String.Format("Soil depth attribute code was not set for {0}. ", row.GridValue), True, True)
                Return False
            End If

            vatSDcode.Add(CInt(row.GridValue), stCode)
        Next
        Try
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridSDepth.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           'Dim value As Integer = CInt(gridSDepth.ValueFromTL(cx, ry))
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           If value > 0 Then
                                                                               mWSCells(cx, ry).SoilDepthTypeValue = CInt(value)
                                                                               mWSCells(cx, ry).SoilDepthOri_m = vatSD(value) / 100     ' 단위환산 cm ->  m
                                                                               mWSCells(cx, ry).SoilDepthCode = vatSDcode(value)
                                                                           Else
                                                                               Throw New KeyNotFoundException
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)
            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridSDepth.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            'value = CInt(gridSDepth.ValueFromTL(cx, ry))
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            If value > 0 Then
                                mWSCells(cx, ry).SoilDepthTypeValue = CInt(value)
                                mWSCells(cx, ry).SoilDepthOri_m = vatSD(value) / 100     ' 단위환산 cm ->  m
                                mWSCells(cx, ry).SoilDepthCode = vatSDcode(value)
                            Else
                                Throw New KeyNotFoundException
                            End If
                        End If
                    Next cx
                Next ry
            End If
            Return True
        Catch ex As KeyNotFoundException
            Throw New KeyNotFoundException(String.Format(
                                               "Soil depth file {0} has an invalid value.", SoilDepth.mGridSoilDepthFPN))
            Return False
        End Try
    End Function


    Public Function SetSoilDepthAttUsingConstant(isParallel As Boolean) As Boolean
        If isParallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   If mWSCells(cx, ry) IsNot Nothing Then
                                                                       mWSCells(cx, ry).SoilDepthTypeValue = Integer.MinValue  ' 상수를 의미
                                                                       mWSCells(cx, ry).SoilDepthOri_m = SoilDepth.mConstSoilDepth.Value / 100     ' 단위환산 cm ->  m
                                                                   End If
                                                               Next
                                                           End Sub)
        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        mWSCells(cx, ry).SoilDepthTypeValue = Integer.MinValue  ' 상수를 의미
                        mWSCells(cx, ry).SoilDepthOri_m = SoilDepth.mConstSoilDepth.Value / 100     ' 단위환산 cm ->  m
                    End If
                Next cx
            Next ry
        End If
        Return True
    End Function


    Public Sub UpdateCVbyUserSettings()
        For intR As Integer = 0 To mWatershed.mRowCount - 1
            For intC As Integer = 0 To mWatershed.mColCount - 1
                If WSCell(intC, intR) Is Nothing Then Continue For
                Dim cell As cCVAttribute = WSCell(intC, intR)
                Dim wsid As Integer = cell.WSID
                With cell
                    .DownStreamWPCVids.Clear()
                    .toBeSimulated = True '우선 모두 모델링 하는 것으로 설정
                    '.bToEstimateParameters = False
                    If .FlowType = cGRM.CellFlowType.ChannelNOverlandFlow Then .FlowType = cGRM.CellFlowType.ChannelFlow '이건 다시 우선 channel로 설정
                    '-------------------------------------------------------------
                    '여기서 지표면 경사 설정
                    If .Slope < mSubWSpar.userPars(wsid).minSlopeOF Then
                        .SlopeOF = mSubWSpar.userPars(wsid).minSlopeOF  '최소 경사값 설정.. 
                    Else
                        .SlopeOF = .Slope
                    End If
                    .RoughnessCoeffOF = .RoughnessCoeffOFori * mSubWSpar.userPars(wsid).ccLCRoughness    '조도계수 보정은 여기서
                    '-------------------------------------------------------------

                    ' 토양에 대한 것----------------------------------------------
                    .powCUnsaturatedK = mSubWSpar.userPars(wsid).expUnsaturatedK
                    .porosityEta = .PorosityEtaOri * mSubWSpar.userPars(wsid).ccPorosity
                    If .porosityEta >= 1 Then .porosityEta = 0.99 '이조건 추가 2016.09.21
                    If .porosityEta <= 0 Then .porosityEta = 0.01 '이조건 추가 2016.09.21
                    .effectivePorosityThetaE = .EffectivePorosityThetaEori * mSubWSpar.userPars(wsid).ccPorosity   '유효 공극율의 보정은 공극률 보정계수를 함께 사용한다.
                    If .effectivePorosityThetaE >= 1 Then .effectivePorosityThetaE = 0.99 '이조건 추가 2016.09.21
                    If .effectivePorosityThetaE <= 0 Then .effectivePorosityThetaE = 0.01 '이조건 추가 2016.09.21
                    .wettingFrontSuctionHeadPsi_m = .WettingFrontSuctionHeadPsiOri_m * mSubWSpar.userPars(wsid).ccWFSuctionHead
                    .hydraulicConductK_mPsec = .HydraulicConductKori_mPsec * mSubWSpar.userPars(wsid).ccHydraulicK  '수리전도도 보정은 여기서
                    .soilDepth_m = .SoilDepthOri_m * mSubWSpar.userPars(wsid).ccSoilDepth  '토양심 보정은 여기서

                    If Watershed.mFPN_initialSoilSaturationRatio = "" OrElse File.Exists(Watershed.mFPN_initialSoilSaturationRatio) = False Then
                        .InitialSaturation = mSubWSpar.userPars(wsid).iniSaturation
                    Else
                        ' 이경우에는 레이어 설정에서 값이 입력되어 있다.
                    End If
                    'todo :이거 검토
                    '.SoilDepthEffectiveAsWaterDepth_m = .SoilDepth_m * (1 - .InitialSaturation) * .EffectivePorosityThetaE
                    '2014.1.8. 토양에 불투수 지역이 있을 경우, 침투가능 깊이도 불투수율 만큼 감소시킨다.
                    '2015.03.23 불투수율은 유효강우에서 반영하는 것으로 수정.. 
                    .SoilDepthEffectiveAsWaterDepth_m = .soilDepth_m * .effectivePorosityThetaE '* (1 - .ImperviousRatio)
                    'CellType.ChannelNOverlandCell 하도 부분은 침투고려하지 않은 강우를 유효강우로 하고, 
                    ' 지표면 부분에서는 침투고려한 유효강우를 적용한다.
                    ' 따라서 overland 부분에서 침투과정 제대로 모의하기 위해서 원래의 토양심 적용한다.
                    ' 2009.09.22 근데.. 그렇게 해보니까.. 수문곡선 상승부 초반에 느리게 올라간다.
                    ' 따라서 하도셀에서는 지하수위가 다른 지역보다 높거나 초기포화도가 좀더 큰 것으로 모의한다. 약 10%
                    If .FlowType = cGRM.CellFlowType.ChannelFlow Then
                        'todo : 이것의 적절성 검토. 2013.05.15
                        '.SoilDepthEffectiveAsWaterDepth_m = CSng(.SoilDepthEffectiveAsWaterDepth_m * 0.3) '다른 영역에 비해 하도 셀에서는 50% 정도가 침투 가능 영역 
                        .SoilDepthEffectiveAsWaterDepth_m = CSng(.SoilDepthEffectiveAsWaterDepth_m * 0.5) '다른 영역에 비해 하도 셀에서는 50% 정도가 침투 가능 영역 
                        '.SoilDepthEffectiveAsWaterDepth_m = CSng(.SoilDepthEffectiveAsWaterDepth_m * 0.6) '다른 영역에 비해 하도 셀에서는 100% 정도가 침투 가능 영역 
                        '.SoilDepthEffectiveAsWaterDepth_m = CSng(.SoilDepthEffectiveAsWaterDepth_m * 1) '다른 영역에 비해 하도 셀에서는 100% 정도가 침투 가능 영역 
                    End If

                    .CumulativeInfiltrationF_m = .SoilDepthEffectiveAsWaterDepth_m * .InitialSaturation
                    .CumulativeInfiltrationF_tM1_m = .CumulativeInfiltrationF_m
                    .soilSaturationRatio = .InitialSaturation
                    If .InitialSaturation = 1 OrElse
                       .LandCoverCode = cSetLandcover.LandCoverCode.WATR OrElse
                       .LandCoverCode = cSetLandcover.LandCoverCode.WTLD Then
                        .soilSaturationRatio = 1
                    End If

                    .SoilDepthToBedrock_m = cGRM.CONST_DEPTH_TO_BEDROCK '암반까지의 깊이를 20m로 가정, 산악지역에서는 5m
                    If .LandCoverCode = cSetLandcover.LandCoverCode.FRST Then
                        .SoilDepthToBedrock_m = cGRM.CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN
                    End If
                    '-------------------------------------------------------------

                    '-------------------------------------------------------------
                    '하천에 대한것
                    If Watershed.HasStreamLayer AndAlso .FlowType = cGRM.CellFlowType.ChannelFlow Then
                        .mStreamAttr.RoughnessCoeffCH = mSubWSpar.userPars(wsid).chRoughness
                        .mStreamAttr.chSideSlopeLeft = CSng(mChannel.mLeftBankSlope)
                        .mStreamAttr.chSideSlopeRight = CSng(mChannel.mRightBankSlope)
                        .mStreamAttr.mChBankCoeff = (1 / mChannel.mLeftBankSlope + 1 / mChannel.mRightBankSlope).Value
                        If .Slope < mSubWSpar.userPars(wsid).minSlopeChBed Then '하도의 최소 경사 설정
                            .mStreamAttr.chBedSlope = mSubWSpar.userPars(wsid).minSlopeChBed
                        Else
                            .mStreamAttr.chBedSlope = .Slope
                        End If

                        If Channel.mCrossSectionType = cSetCrossSection.CSTypeEnum.CSSingle Then
                            '셀크기보다 큰 하폭을 dy로 설정하지 않는다.. 즉, 하천에 떨어지는 강우는 dy를 적용해서 질량보존하고, 하도홍수추적은 실제 하폭(하도단면적 등)으로 한다.
                            Dim cs As New cSetCSSingle
                            cs = CType(Channel.mCrossSection, cSetCSSingle)
                            If cs.mCSSingleWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation Then
                                .mStreamAttr.ChBaseWidth = CSng(cs.mCWEc * ((.FAc + 1) * (Watershed.mCellSize * Watershed.mCellSize / 1000000)) ^ cs.mCWEd / .mStreamAttr.chBedSlope ^ cs.mCWEe)
                            Else
                                .mStreamAttr.ChBaseWidth = .FAc * cs.mMaxChannelWidthSingleCS / cProject.Current.FacMax
                            End If
                            If Not String.IsNullOrEmpty(Watershed.mFPN_channelWidth) AndAlso
                                 .mStreamAttr.ChBaseWidthByLayer > 0 Then
                                .mStreamAttr.ChBaseWidth = .mStreamAttr.ChBaseWidthByLayer
                            End If

                            '2017.07.04. 최. 사용자 지정 하폭은 db가 아니고, GUI에서 channel width file로 저장하는 방향으로 수정 
                            ''여기서는 사용자 편집 하폭 적용 
                            'If cs.UserEditChannelCellCount > 0 Then
                            '    Dim row As GRMProject.UserChannelWidthRow
                            '    Dim rows As DataRow() = cs.mdtUserChannelWidthInfo.Select("CVID = " & cell.CVID)
                            '    If rows.Length = 1 Then
                            '        row = CType(rows(0), GRMProject.UserChannelWidthRow)
                            '        .mStreamAttr.ChBaseWidth = row.ChannelWidth
                            '    End If
                            'End If

                            .mStreamAttr.chUpperRBaseWidth_m = Nothing
                            .mStreamAttr.chIsCompoundCS = False
                            .mStreamAttr.chLowerRArea_m2 = 0
                        Else
                            Dim cs As New cSetCSCompound
                            cs = CType(Channel.mCrossSection, cSetCSCompound)
                            .mStreamAttr.ChBaseWidth = .FAc * cs.mLowerRegionBaseWidth / FacMax

                            If .mStreamAttr.ChBaseWidth < cs.mCompoundCSCriteriaChannelWidth Then
                                .mStreamAttr.chIsCompoundCS = False
                                .mStreamAttr.chUpperRBaseWidth_m = 0
                                .mStreamAttr.chLowerRHeight = 0
                                .mStreamAttr.chLowerRArea_m2 = 0
                            Else
                                .mStreamAttr.chIsCompoundCS = True
                                .mStreamAttr.chUpperRBaseWidth_m = .FAc * cs.mUpperRegionBaseWidth / FacMax
                                .mStreamAttr.chLowerRHeight = .FAc * cs.mLowerRegionHeight / FacMax
                                Dim mFVMSolver As New cFVMSolver
                                .mStreamAttr.chLowerRArea_m2 = mFVMSolver.GetChannelCrossSectionAreaUsingDepth(.mStreamAttr.ChBaseWidth,
                                                                                             .mStreamAttr.mChBankCoeff,
                                                                                             .mStreamAttr.chLowerRHeight, False,
                                                                                             .mStreamAttr.chLowerRHeight,
                                                                                             .mStreamAttr.chLowerRArea_m2,
                                                                                            0)
                            End If
                        End If
                        '여기서 최소 하폭 결정
                        If .mStreamAttr.ChBaseWidth < mSubWSpar.userPars(wsid).minChBaseWidth Then _
                            .mStreamAttr.ChBaseWidth = mSubWSpar.userPars(wsid).minChBaseWidth
                        If .mStreamAttr.ChBaseWidth < Watershed.mCellSize Then
                            .FlowType = cGRM.CellFlowType.ChannelNOverlandFlow
                        Else
                            .soilSaturationRatio = 1
                        End If

                    End If
                    '-------------------------------------------------------------
                End With
            Next intC
        Next intR

        ''----------------------------초기 유량 파일을 이용하는 경우---------------------
        'If Not mEstimatedDist.mFPN_ChFlow = "" AndAlso File.Exists(mEstimatedDist.mFPN_ChFlow) Then
        '    cProject.Current.EstimatedDist.SetChFlowDwihtASC(cProject.Current, mEstimatedDist.mFPN_ChFlow)
        'End If
        ''------------------------------------------------------------------------------------------

        '---------------------------Flow control 설정----------------------------------
        If GeneralSimulEnv.mbSimulateFlowControl = True AndAlso FCGrid.FCCellCount > 0 Then
            For Each cvid As Integer In FCGrid.FCGridCVidList
                Dim rows() As DataRow = FCGrid.mdtFCGridInfo.Select("CVID = " & cvid)
                Dim row As GRMProject.FlowControlGridRow
                row = CType(rows(0), GRMProject.FlowControlGridRow)
                Select Case row.ControlType
                    Case cFlowControl.FlowControlType.Inlet.ToString
                        CV(cvid - 1).FCType = cFlowControl.FlowControlType.Inlet
                    Case cFlowControl.FlowControlType.ReservoirOperation.ToString
                        CV(cvid - 1).FCType = cFlowControl.FlowControlType.ReservoirOperation
                    Case cFlowControl.FlowControlType.ReservoirOutflow.ToString
                        CV(cvid - 1).FCType = cFlowControl.FlowControlType.ReservoirOutflow
                    Case cFlowControl.FlowControlType.SinkFlow.ToString
                        CV(cvid - 1).FCType = cFlowControl.FlowControlType.SinkFlow
                    Case cFlowControl.FlowControlType.SourceFlow.ToString
                        CV(cvid - 1).FCType = cFlowControl.FlowControlType.SourceFlow
                    Case Else
                        Throw New InvalidDataException
                End Select
            Next
        End If
        '-------------------------------------------------------------

        '---------------------------Inlet 이 있을 경우 상류를 검색해서 모델링 하지 않는 것으로 설정----------------------------------
        If mGeneralSimulEnv.mbSimulateFlowControl = True AndAlso FCGrid.InletExisted Then
            Dim bEnded As Boolean = False
            Dim lBaseCVid As List(Of Integer)
            Dim lNewCVid As List(Of Integer)
            lBaseCVid = New List(Of Integer)
            lBaseCVid = FCGrid.InletCVidList
            Do Until bEnded = True
                lNewCVid = New List(Of Integer)
                bEnded = True
                For Each cvidBase As Integer In lBaseCVid
                    Dim cvAN As Integer = cvidBase - 1 '.CVid == CVArrayNumber+1
                    If CV(cvAN).NeighborCVidFlowIntoMe.Count > 0 Then
                        '한번이라도 이 경우 있으면.. 상류 격자가 있다는 의미이므로.. bAllisEnded = False 으로 설정한다.
                        'inlet에서 inlet 자신의 격자는 besimulated = true 로 설정됨.
                        bEnded = False
                        For Each cvidFlowIntoMe As Integer In CV(cvAN).NeighborCVidFlowIntoMe
                            CV(cvidFlowIntoMe - 1).toBeSimulated = False
                            lNewCVid.Add(cvidFlowIntoMe)
                        Next
                    End If
                Next
                lBaseCVid = New List(Of Integer)
                lBaseCVid = lNewCVid
            Loop
        End If
        '-------------------------------------------------------------
    End Sub

    ''' <summary>
    ''' 업데이트된 하폭 정보를 이용해서 wp 격자의 celltype을 업데이트 하고,
    ''' WP를 기준으로 상류의 셀을 검색해서 모든 격자에 대해서 하류에 있는 wp 정보를 설정한다.
    ''' 각 cv에 대해서 맨 마지막에 추가된 wpcvid가 해당 cv와 가장 가까운 wp임.
    ''' </summary>
    ''' <remarks></remarks>
    Public Sub UpdateDownstreamWPforAllCVs()

        Dim lCVidBase As List(Of Integer)
        Dim lCVidNew As List(Of Integer)
        lCVidBase = New List(Of Integer)

        For Each row As GRMProject.WatchPointsRow In WatchPoint.mdtWatchPointInfo.Rows
            Dim nowCVid As Integer = row.CVID
            'CV의 배열 번호는 CVid -1과 같으므로
            Dim nowAN As Integer = nowCVid - 1
            '현재 셀의 flow 타입을 업데이트 하고...
            'row.CellType = CV(nowCVid - 1).FlowType.ToString
            '출발점은 현재의 wp 격자
            lCVidBase.Add(nowCVid)
            CV(nowAN).DownStreamWPCVids.Add(nowCVid)
            Dim bEnded As Boolean = False
            Do Until bEnded = True
                lCVidNew = New List(Of Integer)
                bEnded = True
                For Each cvidBase As Integer In lCVidBase
                    nowAN = cvidBase - 1
                    If CV(nowAN).NeighborCVidFlowIntoMe.Count > 0 Then
                        bEnded = False
                        For Each cvid As Integer In CV(nowAN).NeighborCVidFlowIntoMe
                            CV(cvid - 1).DownStreamWPCVids.Add(nowCVid)
                            lCVidNew.Add(cvid)
                        Next
                    End If
                Next
                lCVidBase = New List(Of Integer)
                lCVidBase = lCVidNew
            Loop
        Next
    End Sub

    ''' <summary>
    ''' 
    ''' </summary>
    ''' <param name="colx"></param>
    ''' <param name="rowy"></param>
    ''' <returns></returns>
    Public ReadOnly Property WSCell(ByVal colx As Integer, ByVal rowy As Integer) As cCVAttribute
        Get
            Return mWSCells(colx, rowy)
        End Get
    End Property

    Public ReadOnly Property WSCells As cCVAttribute(,)
        Get
            Return mWSCells
        End Get
    End Property

    Public ReadOnly Property CV(ByVal index As Integer) As cCVAttribute

        Get
            Return mCVs(index)
        End Get

    End Property

    Public ReadOnly Property CVs As List(Of cCVAttribute)
        Get
            Return mCVs
        End Get
    End Property

    ''' <summary>
    ''' 모델링 대상 영역의 검사체적 개수
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property CVCount() As Integer
        Get
            If mWSCells Is Nothing Then
                Return -1
            Else
                Return mCVs.Count
            End If
        End Get
    End Property

    Public ReadOnly Property FacMax() As Integer
        Get
            If mWatershed Is Nothing Then
                Return -1
            Else
                Return mWatershed.mFacMax
            End If
        End Get
    End Property


    Public ReadOnly Property FacMin() As Integer
        Get
            If mWatershed Is Nothing Then
                Return -1
            Else
                Return mWatershed.mFacMin
            End If
        End Get
    End Property

    ''' <summary>
    ''' 유역별 네트워크 정보
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property WSNetwork() As cWatershedNetwork
        Get
            Return mWSNetwork
        End Get
    End Property

    ''' <summary>
    ''' 출력 시간간격[min]
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property DTPrintOut() As Integer
        Get
            Return mGeneralSimulEnv.mPrintOutTimeStepMIN.Value
        End Get
    End Property


    ''' <summary>
    ''' Project file을 읽어서 mProject 변수에 설정
    ''' </summary>
    ''' <param name="prjFPN"></param>
    ''' <remarks></remarks>
    Public Shared Function OpenProject(ByVal prjFPN As String, forceRealTime As Boolean) As Boolean
        Try
            If String.IsNullOrEmpty(prjFPN) OrElse Not File.Exists(prjFPN) Then
                Throw New FileNotFoundException
                Return False
            End If
            mProject = New cProject
            mProject.mPrjFile = New GRMProject
            mProject.mPrjFile.ReadXml(prjFPN)

            Dim dtPrjSettings As GRMProject.ProjectSettingsDataTable = mProject.mPrjFile.ProjectSettings
            Dim row As GRMProject.ProjectSettingsRow = CType(dtPrjSettings.Rows(0), GRMProject.ProjectSettingsRow)
            With mProject
                .mProjectNameWithExtension = Path.GetFileName(prjFPN)
                .mProjectPathName = prjFPN
                .mProjectPath = Path.GetDirectoryName(prjFPN)
                .mProjectNameOnly = Path.GetFileNameWithoutExtension(prjFPN)
                If Not row.IsGRMSimulationTypeNull Then
                    If row.GRMSimulationType = cGRM.SimulationType.SingleEvent.ToString Then
                        .mSimulationType = cGRM.SimulationType.SingleEvent
                    ElseIf row.GRMSimulationType = cGRM.SimulationType.RealTime.ToString Then
                        .mSimulationType = cGRM.SimulationType.RealTime
                    End If
                End If
                If forceRealTime = True Then
                    .mSimulationType = cGRM.SimulationType.RealTime
                End If
                .mOFNPDischarge = Path.Combine(.mProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_DISCHARGE)
                .mOFNPDepth = Path.Combine(.mProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_DEPTH)
                .mOFNPRFGrid = Path.Combine(.mProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_RFGRID)
                .mOFNPRFMean = Path.Combine(.mProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_RFMEAN)
                .mOFNPFCData = Path.Combine(.mProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_FCAPP)
                .mOFNPFCStorage = Path.Combine(.mProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_FCSTORAGE)
                .mOFNPSwsPars = Path.Combine(.mProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_SWSPARSTEXTFILE)
                .mOFPSSRDistribution = Path.Combine(.mProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_SSR_DIRECTORY_TAG)
                .mOFPRFdist = Path.Combine(.mProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_RF_DIRECTORY_TAG)
                .mOFPRFAccDist = Path.Combine(.mProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_RFACC_DIRECTORY_TAG)
                .mOFPFlowDist = Path.Combine(.mProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_FLOW_DIRECTORY_TAG)
            End With
            If mProject.mSimulationType = cGRM.SimulationType.RealTime Then
                changeOutputFileDisk(cRealTime.CONST_Output_File_Target_DISK)
            End If
            'For Each settingObj As ioProjectFile In mProject.mPrjDataSettings
            '    settingObj.GetValues(mProject.mPrjFile)
            'Next
' 객체들 설정. 2017.08.16.
            mProject.mGeneralSimulEnv.GetValues(mProject.mPrjFile)
            mProject.mSubWSpar.GetValues(mProject.mPrjFile)
            mProject.mWatershed.GetValues(mProject.mPrjFile)
            mProject.mLandcover.GetValues(mProject.mPrjFile)
            mProject.mGreenAmpt.GetValues(mProject.mPrjFile)
            mProject.mSoilDepth.GetValues(mProject.mPrjFile)
            mProject.mWatchPoint.GetValues(mProject.mPrjFile)
            mProject.mChannel.GetValues(mProject.mPrjFile)
            'mProject.mEstimatedDist.GetValues(mProject.mPrjFile)

            If mProject.mSimulationType = cGRM.SimulationType.SingleEvent Then
                mProject.mRainfall.GetValues(mProject)
            End If

            If mProject.GeneralSimulEnv.mbSimulateFlowControl = True Then
                mProject.mFCGrid.GetValues(mProject)
            End If

            'If Not row.IsFlowAccumulationMaxNull Then
            '    cProject.Current.mWatershed.mFacMax = row.FlowAccumulationMax
            'Else
            '    cGRM.writelogAndConsole("Maximum value of flow accumulation Is Not set. The value will be reset automatically. ",
            '                             cGRM.bwriteLog, True)
            'End If

            cThisSimulation.dtsec = row.ComputationalTimeStep * 60
            If cThisSimulation.dtsec > CInt(mProject.GeneralSimulEnv.mPrintOutTimeStepMIN * 30) Then '출력시간간격의 반보다는 작게한다. 즉 한번 출력하기 전에 두번은 계산하게..
                cThisSimulation.dtsec = CInt(mProject.GeneralSimulEnv.mPrintOutTimeStepMIN * 30)
            End If
            cThisSimulation.IsParallel = False
            cThisSimulation.MaxDegreeOfParallelism = 0
            If row.IsIsParallelNull = False AndAlso row.IsParallel.ToLower = "true" Then
                cThisSimulation.IsParallel = True
            End If
            If cThisSimulation.IsParallel = True AndAlso row.IsMaxDegreeOfParallelismNull = False Then
                cThisSimulation.MaxDegreeOfParallelism = CInt(row.MaxDegreeOfParallelism)
                If cThisSimulation.MaxDegreeOfParallelism < -1 OrElse cThisSimulation.MaxDegreeOfParallelism = 0 Then
                    cGRM.writelogAndConsole("MaxDegreeOfParallelism value must be equal or greater than -1. And zero is not allowed.  Parallel option could not applied.",
                                         cGRM.bwriteLog, True)
                    cThisSimulation.IsParallel = False
                End If
            End If

            cThisSimulation.IsFixedTimeStep = True
            If row.IsIsFixedTimeStepNull = False AndAlso row.IsFixedTimeStep.ToLower = "false" Then
                cThisSimulation.IsFixedTimeStep = False
            End If
            mProject.Watershed.WSIDList.Clear()
            For Each id As Integer In mProject.SubWSPar.userPars.Keys
                mProject.Watershed.WSIDList.Add(id)
            Next

            cGRM.bwriteLog = False
            cGRM.fpnlog = prjFPN.Replace(".gmp", ".log")
            If row.IsWriteLogNull = False AndAlso LCase(row.WriteLog) = "true" Then
                cGRM.bwriteLog = True
                If File.Exists(cGRM.fpnlog) Then
                    File.Delete(cGRM.fpnlog)
                End If
            End If
            Return True
        Catch ex As Exception
            cGRM.writelogAndConsole("Open project failed.", True, True)
            Console.WriteLine(ex.ToString)
            Return False
        End Try
        Return True
    End Function

    Private Shared Function changeOutputFileDisk(targetDisk As Char) As Boolean
        '2016.7.10, 2016.8.17 원영진 임시 코드. bmp 등  생성 경로 d: ssd로 임시 조정. 성능 위해
        mProject.mOFPSSRDistribution = IO_Path_ChangeDrive(targetDisk, mProject.mOFPSSRDistribution)
        mProject.mOFPRFdist = IO_Path_ChangeDrive(targetDisk, mProject.mOFPRFdist)
        mProject.mOFPRFAccDist = IO_Path_ChangeDrive(targetDisk, mProject.mOFPRFAccDist)
        mProject.mOFPFlowDist = IO_Path_ChangeDrive(targetDisk, mProject.mOFPFlowDist)
        mProject.mOFNPDischarge = IO_Path_ChangeDrive(targetDisk, mProject.mOFNPDischarge)
        mProject.mOFNPDepth = IO_Path_ChangeDrive(targetDisk, mProject.mOFNPDepth)
        mProject.mOFNPRFGrid = IO_Path_ChangeDrive(targetDisk, mProject.mOFNPRFGrid)
        mProject.mOFNPRFMean = IO_Path_ChangeDrive(targetDisk, mProject.mOFNPRFMean)
        mProject.mOFNPFCData = IO_Path_ChangeDrive(targetDisk, mProject.mOFNPFCData)
        mProject.mOFNPFCStorage = IO_Path_ChangeDrive(targetDisk, mProject.mOFNPFCStorage)
        mProject.mOFNPSwsPars = IO_Path_ChangeDrive(targetDisk, mProject.mOFNPSwsPars)
    End Function

    Public Sub SaveProject()
        If Directory.Exists(Path.GetDirectoryName(mProject.ProjectPath)) Then
            Dim row As GRMProject.ProjectSettingsRow =
                        CType(mProject.PrjFile.ProjectSettings.Rows(0),
                              GRMProject.ProjectSettingsRow)
            With row
                '아래 5개 매개변수 cgeneralEnv로 옮기는 것으로 변경 필요
                .GRMSimulationType = mProject.mSimulationType.ToString
                .SimulStartingTime = mProject.GeneralSimulEnv.mSimStartDateTime
                '.SImulEndingTime = mProject.GeneralSimulEnv.EndingTimeToPrint
                .ComputationalTimeStep = CInt(cThisSimulation.dtsec / 60)
                'If mProject.CVCount <> -1 Then
                '    .WatershedCellCount = mProject.CVCount
                'End If
                '.FlowAccumulationMax = cProject.Current.FacMax
            End With
            'For Each settingDB As ioProjectFile In mProject.mPrjDataSettings
            '    settingDB.SetValues(mProject.PrjFile)
            'Next

            ''
            mProject.mGeneralSimulEnv.SetValues(mProject.PrjFile)
            mProject.mSubWSpar.SetValues(mProject.PrjFile)
            mProject.mWatershed.SetValues(mProject.PrjFile)
            mProject.mLandcover.SetValues(mProject.PrjFile)
            mProject.mGreenAmpt.SetValues(mProject.PrjFile)
            mProject.mSoilDepth.SetValues(mProject.PrjFile)
            mProject.mWatchPoint.SetValues(mProject.PrjFile)
            mProject.mChannel.SetValues(mProject.PrjFile)
            'mProject.mEstimatedDist.SetValues(mProject.PrjFile)


            mProject.PrjFile.WriteXml(mProject.ProjectPathName)

            '======================================
            '강우자료 파일은 프로젝트 xml 파일에 쓰지 않고, 별도의 텍스트 파일로 관리한다.
            'mProject.mRainfall.SetValues(mPrjFile)
            'cRainfall.SaveRainfallDataFile(mProject.mRainfall)
            '======================================

            '이건 gmp 파일에 일괄적으로 있으니.. 다시 기입하지 않는 것으로 수정. 2017.10.26. 최
            'If mProject.Watershed.WSIDList IsNot Nothing Then
            '    cOutPutControl.SaveSWSParsTextFile(mProject)
            'End If
            If mProject.mSimulationType = cGRM.SimulationType.SingleEvent Then
                Console.WriteLine(cProject.Current.ProjectPathName & " Is saved.  " + cGRM.BuildInfo.ProductName)
            End If
        Else
            Console.WriteLine(String.Format("Current project path {0} Is Not exist.   ", Path.GetDirectoryName(mProject.ProjectPath)))
            Exit Sub
        End If
    End Sub


    Public Shared Sub SaveAsProject(ByVal prjPathName As String)

        'If String.IsNullOrEmpty(prjPathName) OrElse mwProject Is Nothing OrElse
        '    String.IsNullOrEmpty(mwProject.Project.FileName) Then
        '    Throw New ArgumentException
        'End If

        CopyAndUpdateFiles(mProject, prjPathName)
        SetProjectVariables(mProject, prjPathName) ' 클래스 기본정보 수정
        cProject.Current.SaveProject() '프로젝트 파일은 복사만 해서는 해결안됨. 여기서 저장해야 내부내용도 바뀜
    End Sub


    Private Shared Sub CopyAndUpdateFiles(ByVal SourceProject As cProject, ByVal TargetPathName As String)
        With SourceProject
            Dim TargetProjectPathNameOnly As String = Path.Combine(Path.GetDirectoryName(TargetPathName), Path.GetFileNameWithoutExtension(TargetPathName))
            Dim Header As String = String.Format("Project name : {0} {1} {2}",
                                                  TargetPathName,
                                                  vbTab, Format(Now, "yyyy/MM/dd HH:mm"))
            '유량 파일 복사
            If IO.File.Exists(.OFNPDischarge) = True Then
                FileCopy(.OFNPDischarge, TargetProjectPathNameOnly & cGRM.CONST_TAG_DISCHARGE)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_DISCHARGE, TargetProjectPathNameOnly & cGRM.CONST_TAG_DISCHARGE, 1, Header)
            End If

            '수심파일 복사
            If IO.File.Exists(.OFNPDepth) = True Then
                FileCopy(.OFNPDepth, TargetProjectPathNameOnly & cGRM.CONST_TAG_DEPTH)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_DEPTH, TargetProjectPathNameOnly & cGRM.CONST_TAG_DEPTH, 1, Header)
            End If

            '격자 강우량파일 복사
            If IO.File.Exists(.OFNPRFGrid) = True Then
                FileCopy(.OFNPRFGrid, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFGRID)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_RFGRID, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFGRID, 1, Header)
            End If

            '평균 강우량 파일 복사
            If IO.File.Exists(.OFNPRFMean) = True Then
                FileCopy(.OFNPRFMean, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFMEAN)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_RFMEAN, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFMEAN, 1, Header)
            End If

            'flow control 적용 정보 파일 복사
            If IO.File.Exists(.OFNPFCData) = True Then
                FileCopy(.OFNPFCData, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCAPP)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_FCAPP, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCAPP, 1, Header)
            End If

            '저수지 파일 복사
            If IO.File.Exists(.OFNPFCStorage) = True Then
                FileCopy(.OFNPFCStorage, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCSTORAGE)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_FCSTORAGE, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCSTORAGE, 1, Header)
            End If

            'WP별 모의결과, MREQ 복사
            If .WatchPoint.WPCount > 0 Then
                For i As Integer = 0 To .WatchPoint.WPCount - 1
                    Dim strWPName As String = Replace(mProject.WatchPoint.mdtWatchPointInfo(i).Name, ",", "_")
                    Dim strWPFName As String = .ProjectPath & "\" & .ProjectNameOnly & "WP_" & strWPName & ".out"
                    Dim strMREFName As String = .ProjectPath & "\" & .ProjectNameOnly & "MREQ_" & strWPName & ".out"
                    Dim TargetWPFpn As String = TargetProjectPathNameOnly & "WP_" & strWPName & ".out"
                    Dim TargetMREFpn As String = TargetProjectPathNameOnly & "MREQ_" & strWPName & ".out"
                    If IO.File.Exists(strWPFName) = True Then
                        FileCopy(strWPFName, TargetWPFpn)
                        cTextFile.ReplaceALineInTextFile(TargetWPFpn, TargetWPFpn, 1, Header)
                    End If
                    If IO.File.Exists(strMREFName) = True Then
                        FileCopy(strMREFName, TargetMREFpn)
                        cTextFile.ReplaceALineInTextFile(TargetMREFpn, TargetMREFpn, 1, Header)
                    End If
                Next i
            End If

            '소유역 매개변수 파일은 복사하지 않고, SaveProject 과정에서 자동 저장되게..
            '프로젝트 mdb 복사
            'FileCopy(.ProjectDBxmlFpn, Path.Combine(Path.GetDirectoryName(TargetPathName), Path.GetFileNameWithoutExtension(TargetPathName) & ".mdb"))
        End With
    End Sub


    Public Shared Sub SetProjectVariables(ByVal TargetProject As cProject, ByVal TargetPrjPathName As String)
        With TargetProject
            .ProjectNameWithExtension = Path.GetFileName(TargetPrjPathName)
            .ProjectNameOnly = Path.GetFileNameWithoutExtension(TargetPrjPathName)
            .ProjectPathName = TargetPrjPathName
            .ProjectPath = Path.GetDirectoryName(TargetPrjPathName)
            '.mProjectGMWPathName = cNonGDK.GetRelativeFilePath(TargetPrjPathName, cGRM.gGMMap.CurrentApp.Workspace.Path)
            .geoDataPrjFPN = "" 'cMap.mwAppMain.Project.FileName.ToString  'cGRM.gGMMap.CurrentApp.Workspace.Path

            '.ProjectDBxmlFpn = Path.Combine(.ProjectPath, .ProjectNameOnly & ".xml")
            .OFNPDischarge = Path.Combine(.ProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_DISCHARGE)
            .OFNPDepth = Path.Combine(.ProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_DEPTH)
            .OFNPRFGrid = Path.Combine(.ProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_RFGRID)
            .OFNPRFMean = Path.Combine(.ProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_RFMEAN)
            .OFNPFCData = Path.Combine(.ProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_FCAPP)
            .OFNPFCStorage = Path.Combine(.ProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_FCSTORAGE)
            .OFNPSwsPars = Path.Combine(.ProjectPath, .ProjectNameOnly & cGRM.CONST_TAG_SWSPARSTEXTFILE)
            .OFPSSRDistribution = Path.Combine(.ProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_SSR_DIRECTORY_TAG)
            .OFPRFdist = Path.Combine(.ProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_RF_DIRECTORY_TAG)
            .OFPRFAccDist = Path.Combine(.ProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_RFACC_DIRECTORY_TAG)
            .OFPFlowDist = Path.Combine(.ProjectPath, .ProjectNameOnly + "_" + cGRM.CONST_DIST_FLOW_DIRECTORY_TAG)

            'If mProject.OdbCnnDynamic.State = ConnectionState.Open Then
            '    Current.OdbCnnDynamic.Close()
            'End If
            'If Current.OdbCnnDynamic.State = ConnectionState.Open Then '아직까지 열려있으면, 그냥 종료한다.
            '    MsgBox(String.Format("Previous project database(mdb) file  cannot be closed. Please try again"), MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
            '    Exit Sub
            'Else
            '    .OdbCnnDynamic.ConnectionString = "Provider=Microsoft.Jet.OLEDB.4.0;Persist Security Info=False;Data Source=" + .ProjectDBxmlFpn
            'End If
        End With

        If TargetProject.PrjFile.ProjectSettings.Rows.Count > 0 Then
            Dim dtSettings As GRMProject.ProjectSettingsDataTable = TargetProject.PrjFile.ProjectSettings
            Dim settings As GRMProject.ProjectSettingsRow = CType(dtSettings.Rows(0), GRMProject.ProjectSettingsRow)
            With settings
                .ProjectFile = TargetPrjPathName
                '.GeoDataPrjPathName = .GeoDataPrjPathName 'mMwGRMPrj.mProjectMWPrjPathName ' cFile.GetRelativeFilePath(TargetPrjPathName, TargetProject.ProjectMWPrjPathName)
                '.GRMStaticDB = cGRM.PathNameGRMStaticDB ' cFile.GetRelativeFilePath(TargetPrjPathName, cGRM.PathNameGRMStaticDB)
                '.GRMProjectDB = cFile.GetRelativeFilePath(TargetPrjPathName, TargetProject.ProjectNameWithExtension )
            End With
        End If
    End Sub

    Public Sub Close()

    End Sub

    Public Function SetupModelParametersAfterProjectFileWasOpened() As Boolean
        If cProject.Current.SetBasicCVInfo() = False Then Return False
        cProject.Current.WatchPoint.UpdatesWatchPointCVIDs(cProject.Current)
        If mProject.GeneralSimulEnv.mbSimulateFlowControl = True Then
            cProject.Current.FCGrid.UpdateFCGridInfoAndData(cProject.Current)
        End If
        cProject.Current.UpdateCVbyUserSettings()
        cProject.Current.UpdateDownstreamWPforAllCVs()
        cGRM.Start()
        cThisSimulation.mGRMSetupIsNormal = True
        If mProject.mSimulationType <> cGRM.SimulationType.RealTime Then
            cThisSimulation.mRFDataCountInThisEvent = mProject.Rainfall.mlstRainfallData.Count
        End If
        Dim deleteOnly As Boolean = False
        If cProject.Current.mSimulationType = cGRM.SimulationType.RealTime Then
            deleteOnly = True
        End If
        If cOutPutControl.CreateNewOutputFiles(mProject, deleteOnly) = False Then
            Console.WriteLine("Making output file template was failed..")
            Return False
        End If
        Return True
    End Function



    Public Shared Function ValidateProjectFile(prj As cProject) As Boolean
        Dim r As GRMProject.ProjectSettingsRow = CType(prj.PrjFile.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)

        If Not r.IsWatershedFileNull AndAlso r.WatershedFile <> "" Then
            If File.Exists(r.WatershedFile) = False Then
                Console.WriteLine(String.Format("Watershed file is not exist!! {0} {1}", vbCrLf, r.WatershedFile))
                Return False
            End If
        End If
        If Not r.IsSlopeFileNull AndAlso r.SlopeFile <> "" Then
            If File.Exists(r.SlopeFile) = False Then
                Console.WriteLine(String.Format("Slope file is not exist!! {0} {1}", vbCrLf, r.SlopeFile))
                Return False
            End If
        End If

        If Not r.IsFlowDirectionFileNull AndAlso r.FlowDirectionFile <> "" Then
            If File.Exists(r.FlowDirectionFile) = False Then
                Console.WriteLine(String.Format("Flow direction file is not exist!! {0} {1}", vbCrLf, r.FlowDirectionFile))
                Return False
            End If
        End If

        If Not r.IsFlowAccumFileNull AndAlso r.FlowAccumFile <> "" Then
            If File.Exists(r.FlowAccumFile) = False Then
                Console.WriteLine(String.Format("Flow accumulation file is not exist!! {0} {1}", vbCrLf, r.FlowAccumFile))
                Return False
            End If
        End If

        If Not r.IsStreamFileNull AndAlso r.StreamFile <> "" Then
            If File.Exists(r.StreamFile) = False Then
                Console.WriteLine(String.Format("Stream file is not exist!! {0} {1}", vbCrLf, r.StreamFile))
                Return False
            End If
        End If

        If Not r.IsChannelWidthFileNull AndAlso r.ChannelWidthFile <> "" Then
            If File.Exists(r.ChannelWidthFile) = False Then
                Console.WriteLine(String.Format("Channel width file is not exist!! {0} {1}", vbCrLf, r.ChannelWidthFile))
                Return False
            End If
        End If

        If Not r.IsLandCoverFileNull AndAlso r.LandCoverFile <> "" Then
            If File.Exists(r.LandCoverFile) = False Then
                Console.WriteLine(String.Format("Land cover file is not exist!! {0} {1}", vbCrLf, r.LandCoverFile))
                Return False
            End If
        End If

        If Not r.IsSoilTextureFileNull AndAlso r.SoilTextureFile <> "" Then
            If File.Exists(r.SoilTextureFile) = False Then
                Console.WriteLine(String.Format("Soil texture file is not exist!! {0} {1}", vbCrLf, r.SoilTextureFile))
                Return False
            End If
        End If

        If Not r.IsSoilDepthFileNull AndAlso r.SoilTextureFile <> "" Then
            If File.Exists(r.SoilDepthFile) = False Then
                Console.WriteLine(String.Format("Soil depth file is not exist!! {0} {1}", vbCrLf, r.SoilTextureFile))
                Return False
            End If
        End If

        If Not r.IsRainfallDataFileNull AndAlso r.RainfallDataFile <> "" Then
            If File.Exists(r.RainfallDataFile) = False Then
                Console.WriteLine(String.Format("Rainfall data file is not exist!! {0} {1}", vbCrLf, r.RainfallDataFile))
                Return False
            End If
        End If

        'If Not r.IsFlowControlDataFileNull AndAlso r.FlowControlDataFile <> "" Then
        '    If File.Exists(r.FlowControlDataFile) = False Then
        '        Console.WriteLine(String.Format("Flow constrol data file is not exist!! {0} {1}", vbCrLf, r.FlowControlDataFile))
        '        Return False
        '    End If
        'End If
    End Function


    'Public Function Clone() As Object Implements ICloneable.Clone
    '    Dim clo As New cProjectBAK
    '    clo.mWatchPoint = Me.WatchPoint
    '    clo.mFCGrid = Me.FCGrid
    '    clo.mCVs = Me.CVs
    '    Return clo
    'End Function


End Class
