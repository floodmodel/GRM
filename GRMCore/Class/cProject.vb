Imports System.Text
Imports System.IO

Public Class cProject
    Implements IDisposable

#Region "프로젝트 일반, 공통 정보"
    Private Shared mProject As cProject
    Public mSimulationType As cGRM.SimulationType
#End Region

#Region "입력된 레이어를 이용해서 설정되는 정보"
    Private mWatershed As cSetWatershed
    Private mLandcover As cSetLandcover
    Private mGreenAmpt As cSetGreenAmpt
    Private mSoilDepth As cSetSoilDepth
    Private mRainfall As cRainfall

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
    Private mOFNPDischarge As String
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
        mGeneralSimulEnv = New cSetGeneralSimulEnvironment
        mChannel = New cSetChannel
        mWatchPoint = New cSetWatchPoint
        mFCGrid = New cFlowControl
        mSubWSpar = New cSetSubWatershedParameter
        mTSSummary = New cSetTSData
    End Sub

    Protected disposed As Boolean = False
    Protected Overridable Overloads Sub Dispose(ByVal disposing As Boolean)
        If Not disposed Then
            If disposing Then
                mProject = Nothing
                MyBase.Finalize()
                GC.Collect()
            End If
        End If
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
            ReadLayerWSandSetBasicInfo(row.WatershedFile, cThisSimulation.IsParallel)
            ReadLayerSlope(row.SlopeFile, cThisSimulation.IsParallel)
            ReadLayerFdir(row.FlowDirectionFile, cThisSimulation.IsParallel)
            ReadLayerFAcc(row.FlowAccumFile, cThisSimulation.IsParallel)
            Dim FPNstream As String = ""
            Dim FPNchannelWidth As String = ""
            Dim FPNiniSSR As String = ""
            Dim FPNiniChannelFlow As String = ""
            Dim FPNlc As String = ""
            Dim FPNst As String = ""
            Dim FPNsd As String = ""
            If row.IsStreamFileNull = False AndAlso File.Exists(row.StreamFile) = True Then
                FPNstream = row.StreamFile
                If ReadLayerStream(row.StreamFile, cThisSimulation.IsParallel) = False Then
                    cGRM.writelogAndConsole(String.Format("Some errors were occurred while reading stream file.. {0}",
                                    row.StreamFile.ToString), True, True)
                End If
            End If
            If row.IsChannelWidthFileNull = False AndAlso File.Exists(row.ChannelWidthFile) = True Then
                FPNchannelWidth = row.ChannelWidthFile
                If ReadLayerChannelWidth(row.ChannelWidthFile, cThisSimulation.IsParallel) = False Then
                    Console.WriteLine(String.Format("Some errors were occurred while reading channel width file.. {0}",
                                                        row.ChannelWidthFile.ToString), True, True)
                End If
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

            If Landcover.mLandCoverDataType.Equals(cGRM.FileOrConst.File) AndAlso row.IsLandCoverFileNull = False AndAlso File.Exists(row.LandCoverFile) Then
                FPNlc = row.LandCoverFile
                If ReadLayerLandCover(row.LandCoverFile, cThisSimulation.IsParallel) = False Then Return False
            Else
                If SetLandCoverAttUsingConstant(cThisSimulation.IsParallel) = False Then Return False
            End If

            If GreenAmpt.mSoilTextureDataType.Equals(cGRM.FileOrConst.File) AndAlso row.IsSoilTextureFileNull = False AndAlso File.Exists(row.SoilTextureFile) Then
                FPNst = row.SoilTextureFile
                If ReadLayerSoilTexture(row.SoilTextureFile, cThisSimulation.IsParallel) = False Then Return False
            Else
                If SetSoilTextureAttUsingConstant(cThisSimulation.IsParallel) = False Then Return False
            End If

            If SoilDepth.mSoilDepthDataType.Equals(cGRM.FileOrConst.File) AndAlso row.IsSoilDepthFileNull = False AndAlso File.Exists(row.SoilDepthFile) Then
                FPNsd = row.SlopeFile
                If ReadLayerSoilDepth(row.SoilDepthFile, cThisSimulation.IsParallel) = False Then Return False
            Else
                If SetSoilDepthAttUsingConstant(cThisSimulation.IsParallel) = False Then Return False
            End If
            SetGridNetworkFlowInformation()
            InitControlVolumeAttribute()
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
        mWatershed.mFacMostUpChannelCell = CVCount
        mCVANsForEachFA.Clear()
        mWatershed.mFacMax = 0
        mWatershed.mFacMin = Integer.MaxValue
        For cvan As Integer = 0 To CVCount - 1
            With mCVs(cvan)
                .FCType = cFlowControl.FlowControlType.NONE
                .toBeSimulated = True
                .DownStreamWPCVids = New List(Of Integer)
                Dim deltaXw As Single
                If .FAc > 0 Then
                    deltaXw = .deltaXwSum / .NeighborCVidFlowIntoMe.Count
                Else
                    deltaXw = .DeltaXDownHalf_m
                End If
                .CVDeltaX_m = .DeltaXDownHalf_m * 2
                'FA별 cvid 저장
                mCVANsForEachFA.Add(.FAc, cvan)

                If .FAc > mWatershed.mFacMax Then
                    mWatershed.mFacMax = .FAc
                    mMostDownCellArrayNumber = cvan
                End If

                If .FAc < mWatershed.mFacMin Then
                    mWatershed.mFacMin = .FAc
                End If

                '하도 매개변수 받고
                If .FlowType = cGRM.CellFlowType.ChannelFlow Then
                    If .FAc < mWatershed.mFacMostUpChannelCell Then
                        mWatershed.mFacMostUpChannelCell = .FAc
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
        DeltaXDiagonalHalf_m = CSng(Watershed.mCellSize * Math.Sqrt(2)) / 2
        DeltaXPerpendicularHalf_m = CSng(Watershed.mCellSize / 2)
        For row As Integer = 0 To Watershed.mRowCount - 1
            For col As Integer = 0 To Watershed.mColCount - 1
                If mWSCells(col, row) Is Nothing Then Continue For
                Dim cell As cCVAttribute = mWSCells(col, row)
                If cell.NeighborCVidFlowIntoMe Is Nothing Then cell.NeighborCVidFlowIntoMe = New List(Of Integer)
                Dim targetCell As cCVAttribute = Nothing
                Dim deltaXe As Single
                Dim targetC As Integer, targetR As Integer
                With cell
                    '좌상단이 0,0 이다... 즉, 북쪽이면, row-1, 동쪽이면 col +1
                    Select Case .FDir
                        Case cGRM.GRMFlowDirectionD8.NE
                            targetC = col + 1
                            targetR = row - 1
                            deltaXe = DeltaXDiagonalHalf_m
                        Case cGRM.GRMFlowDirectionD8.E
                            targetC = col + 1
                            targetR = row
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case cGRM.GRMFlowDirectionD8.SE
                            targetC = col + 1
                            targetR = row + 1
                            deltaXe = DeltaXDiagonalHalf_m
                        Case cGRM.GRMFlowDirectionD8.S
                            targetC = col
                            targetR = row + 1
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case cGRM.GRMFlowDirectionD8.SW
                            targetC = col - 1
                            targetR = row + 1
                            deltaXe = DeltaXDiagonalHalf_m
                        Case cGRM.GRMFlowDirectionD8.W
                            targetC = col - 1
                            targetR = row
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case cGRM.GRMFlowDirectionD8.NW
                            targetC = col - 1
                            targetR = row - 1
                            deltaXe = DeltaXDiagonalHalf_m
                        Case cGRM.GRMFlowDirectionD8.N
                            targetC = col
                            targetR = row - 1
                            deltaXe = DeltaXPerpendicularHalf_m
                        Case Else
                            Throw New InvalidOperationException
                    End Select

                    If IsInBound(targetC, targetR) Then
                        targetCell = mWSCells(targetC, targetR)
                        If targetCell Is Nothing Then
                            mWSNetwork.SetWSoutletCVID(.WSID, .CVID)
                        Else
                            If targetCell.NeighborCVidFlowIntoMe Is Nothing Then targetCell.NeighborCVidFlowIntoMe = New List(Of Integer)
                            targetCell.NeighborCVidFlowIntoMe.Add(.CVID) 'source의 cellid 기록
                            targetCell.deltaXwSum = targetCell.deltaXwSum + deltaXe
                            .DownCellidToFlow = targetCell.CVID  '흘러갈 방향의 cellid 기록함
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
                        .DeltaXDownHalf_m = deltaXe
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
                    Dim intNowCVArrayNum As Integer = CInt(baseCVID) - 1
                    Dim countUpCells As Integer = mCVs(intNowCVArrayNum).NeighborCVidFlowIntoMe.Count
                    If countUpCells > 0 Then
                        bAllisEnded = False
                        For Each CVidFlowIntoMe As Integer In mCVs(intNowCVArrayNum).NeighborCVidFlowIntoMe
                            Dim upArrayNum As Integer = CVidFlowIntoMe - 1
                            Dim colX As Integer = mCVs(upArrayNum).XCol
                            Dim rowY As Integer = mCVs(upArrayNum).YRow
                            currentUpstreamCells.Add(upArrayNum + 1) 'CVid == Cellid == CVArrayNumber+1
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
        mWatershed.mRowCount = gridWS.Header.numberRows
        mWatershed.mColCount = gridWS.Header.numberCols
        mWatershed.mCellSize = CInt(gridWS.Header.cellsize)
        mWatershed.mxllcorner = gridWS.Header.xllcorner
        mWatershed.myllcorner = gridWS.Header.yllcorner
        ' dim ary(n) 하면, vb.net에서는 0~n까지 n+1개의 배열요소 생성. c#에서는 0~(n-1) 까지 n 개의 요소 생성
        mWSCells = New cCVAttribute(mWatershed.mColCount - 1, mWatershed.mRowCount - 1) {}
        mCVs = New List(Of cCVAttribute)
        Dim cvid As Integer = 0
        If isparallel = True Then
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                               Dim valuesInaLine() As String = gridWS.ValuesInOneRowFromTopLeft(ry)
                                                               For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                   Dim wsid As Integer = CInt(valuesInaLine(cx))
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
                    Dim wsid As Integer = CInt(valuesInaLine(cx))
                    If wsid > 0 Then '유역 내부
                        Dim cv As New cCVAttribute
                        With cv
                            .WSID = wsid
                            .CVID = cvid + 1 ' mCVs.Count + 1.  CVid를 CV 리스트(mCVs)의 인덱스 번호 +1 의 값으로 입력 
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
                                                                       mWSCells(cx, ry).FDir = cHydroCom.GetFlowDirection(CInt(valuesInaLine(cx)), Watershed.mFDType)
                                                                   End If
                                                               Next
                                                           End Sub)
        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                Dim valuesInaLine() As String = gridFdir.ValuesInOneRowFromTopLeft(ry)
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        mWSCells(cx, ry).FDir = cHydroCom.GetFlowDirection(CInt(valuesInaLine(cx)), Watershed.mFDType)
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
                                                                       mWSCells(cx, ry).FAc = CInt(valuesInaLine(cx))
                                                                   End If
                                                               Next
                                                           End Sub)

        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                Dim valuesInaLine() As String = gridFac.ValuesInOneRowFromTopLeft(ry)
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        mWSCells(cx, ry).FAc = CInt(valuesInaLine(cx))
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
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridStream.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           If value > 0 Then
                                                                               mWSCells(cx, ry).FlowType = cGRM.CellFlowType.ChannelFlow
                                                                               With mWSCells(cx, ry)
                                                                                   .mStreamAttr = New cCVStreamAttribute
                                                                                   .mStreamAttr.ChStrOrder = value
                                                                                   .mStreamAttr.chBedSlope = .SlopeOF
                                                                               End With
                                                                           End If
                                                                       End If
                                                                   Next
                                                               End Sub)

            Else
                For ry As Integer = 0 To Watershed.mRowCount - 1
                    Dim valuesInaLine() As String = gridStream.ValuesInOneRowFromTopLeft(ry)
                    For cx As Integer = 0 To Watershed.mColCount - 1
                        If mWSCells(cx, ry) IsNot Nothing Then
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            If value > 0 Then
                                mWSCells(cx, ry).FlowType = cGRM.CellFlowType.ChannelFlow
                                With mWSCells(cx, ry)
                                    .mStreamAttr = New cCVStreamAttribute
                                    .mStreamAttr.ChStrOrder = value
                                    .mStreamAttr.chBedSlope = .SlopeOF
                                End With
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
                                                                           Dim cell As cCVAttribute = mWSCells(cx, ry)
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
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
                            Dim cell As cCVAttribute = mWSCells(cx, ry)
                            Dim value As Integer = CInt(valuesInaLine(cx))
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
                                                                       cell.LandCoverValue = 0     ' 상수 의미
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
                        cell.LandCoverValue = 0    ' 상수 의미
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
            If isParallel = True Then
                Dim options As ParallelOptions = New ParallelOptions()
                options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                Parallel.For(0, mWatershed.mRowCount, options, Sub(ry As Integer)
                                                                   Dim valuesInaLine() As String = gridSTexture.ValuesInOneRowFromTopLeft(ry)
                                                                   For cx As Integer = 0 To mWatershed.mColCount - 1
                                                                       If mWSCells(cx, ry) IsNot Nothing Then
                                                                           Dim cell As cCVAttribute = mWSCells(cx, ry)
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
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
                                                                           If value > 0 Then
                                                                               cell.SoilTextureValue = value
                                                                               cell.PorosityEtaOri = vatP(value)
                                                                               cell.EffectivePorosityThetaEori = vatEP(value)
                                                                               cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH(value) / 100  ' cm -> m
                                                                               cell.HydraulicConductKori_mPsec = vatHC(value) / 100 / 3600    ' cm/hr -> m/s
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
                            If value > 0 Then
                                cell.SoilTextureValue = value
                                cell.PorosityEtaOri = vatP(value)
                                cell.EffectivePorosityThetaEori = vatEP(value)
                                cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH(value) / 100  ' cm -> m
                                cell.HydraulicConductKori_mPsec = vatHC(value) / 100 / 3600    ' cm/hr -> m/s
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
                                                                       cell.WettingFrontSuctionHeadPsiOri_m = GreenAmpt.mConstWFS.Value / 100  ' cm -> m
                                                                       cell.HydraulicConductKori_mPsec = GreenAmpt.mConstHydraulicCond.Value / 100 / 3600    ' cm/hr -> m/s
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
                        cell.WettingFrontSuctionHeadPsiOri_m = GreenAmpt.mConstWFS.Value / 100  ' cm -> m
                        cell.HydraulicConductKori_mPsec = GreenAmpt.mConstHydraulicCond.Value / 100 / 3600    ' cm/hr -> m/s
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
                                                                           Dim value As Integer = CInt(valuesInaLine(cx))
                                                                           If value > 0 Then
                                                                               mWSCells(cx, ry).SoilDepthTypeValue = CInt(value)
                                                                               mWSCells(cx, ry).SoilDepthOri_m = vatSD(value) / 100     ' cm ->  m
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
                            Dim value As Integer = CInt(valuesInaLine(cx))
                            If value > 0 Then
                                mWSCells(cx, ry).SoilDepthTypeValue = CInt(value)
                                mWSCells(cx, ry).SoilDepthOri_m = vatSD(value) / 100     ' cm ->  m
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
                                                                       mWSCells(cx, ry).SoilDepthOri_m = SoilDepth.mConstSoilDepth.Value / 100     ' cm ->  m
                                                                   End If
                                                               Next
                                                           End Sub)
        Else
            For ry As Integer = 0 To Watershed.mRowCount - 1
                For cx As Integer = 0 To Watershed.mColCount - 1
                    If mWSCells(cx, ry) IsNot Nothing Then
                        mWSCells(cx, ry).SoilDepthTypeValue = Integer.MinValue  ' 상수를 의미
                        mWSCells(cx, ry).SoilDepthOri_m = SoilDepth.mConstSoilDepth.Value / 100     ' cm ->  m
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
                    .toBeSimulated = True
                    If .FlowType = cGRM.CellFlowType.ChannelNOverlandFlow Then .FlowType = cGRM.CellFlowType.ChannelFlow
                    '지표면 경사
                    If .Slope < mSubWSpar.userPars(wsid).minSlopeOF Then
                        .SlopeOF = mSubWSpar.userPars(wsid).minSlopeOF
                    Else
                        .SlopeOF = .Slope
                    End If
                    .RoughnessCoeffOF = .RoughnessCoeffOFori * mSubWSpar.userPars(wsid).ccLCRoughness

                    ' 토양
                    .powCUnsaturatedK = mSubWSpar.userPars(wsid).expUnsaturatedK
                    .porosityEta = .PorosityEtaOri * mSubWSpar.userPars(wsid).ccPorosity
                    If .porosityEta >= 1 Then .porosityEta = 0.99
                    If .porosityEta <= 0 Then .porosityEta = 0.01
                    .effectivePorosityThetaE = .EffectivePorosityThetaEori * mSubWSpar.userPars(wsid).ccPorosity   '유효 공극율의 보정은 공극률 보정계수를 함께 사용한다.
                    If .effectivePorosityThetaE >= 1 Then .effectivePorosityThetaE = 0.99
                    If .effectivePorosityThetaE <= 0 Then .effectivePorosityThetaE = 0.01
                    .wettingFrontSuctionHeadPsi_m = .WettingFrontSuctionHeadPsiOri_m * mSubWSpar.userPars(wsid).ccWFSuctionHead
                    .hydraulicConductK_mPsec = .HydraulicConductKori_mPsec * mSubWSpar.userPars(wsid).ccHydraulicK
                    .soilDepth_m = .SoilDepthOri_m * mSubWSpar.userPars(wsid).ccSoilDepth

                    If Watershed.mFPN_initialSoilSaturationRatio = "" OrElse File.Exists(Watershed.mFPN_initialSoilSaturationRatio) = False Then
                        .InitialSaturation = mSubWSpar.userPars(wsid).iniSaturation
                    Else
                        ' 이경우에는 레이어 설정에서 값이 입력되어 있다.
                    End If
                    .SoilDepthEffectiveAsWaterDepth_m = .soilDepth_m * .effectivePorosityThetaE
                    If .FlowType = cGRM.CellFlowType.ChannelFlow Then
                        .SoilDepthEffectiveAsWaterDepth_m = CSng(.SoilDepthEffectiveAsWaterDepth_m * 0.5) '하도 셀에서는 50% 정도가 침투 가능 영역 
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

                    '하천
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
                        '최소 하폭
                        If .mStreamAttr.ChBaseWidth < mSubWSpar.userPars(wsid).minChBaseWidth Then _
                            .mStreamAttr.ChBaseWidth = mSubWSpar.userPars(wsid).minChBaseWidth
                        If .mStreamAttr.ChBaseWidth < Watershed.mCellSize Then
                            .FlowType = cGRM.CellFlowType.ChannelNOverlandFlow
                        Else
                            .soilSaturationRatio = 1
                        End If

                    End If
                End With
            Next intC
        Next intR

        'Flow control
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

        ' Inlet
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
                    Dim cvAN As Integer = cvidBase - 1
                    If CV(cvAN).NeighborCVidFlowIntoMe.Count > 0 Then
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
            Dim nowAN As Integer = nowCVid - 1
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

            cThisSimulation.dtsec = CInt(row.ComputationalTimeStep) * 60
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
        ' bmp 등  생성 경로 disk 조정
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
                .GRMSimulationType = mProject.mSimulationType.ToString
                .SimulStartingTime = mProject.GeneralSimulEnv.mSimStartDateTime
                .ComputationalTimeStep = CStr(cThisSimulation.dtsec / 60)
            End With
            mProject.mGeneralSimulEnv.SetValues(mProject.PrjFile)
            mProject.mSubWSpar.SetValues(mProject.PrjFile)
            mProject.mWatershed.SetValues(mProject.PrjFile)
            mProject.mLandcover.SetValues(mProject.PrjFile)
            mProject.mGreenAmpt.SetValues(mProject.PrjFile)
            mProject.mSoilDepth.SetValues(mProject.PrjFile)
            mProject.mWatchPoint.SetValues(mProject.PrjFile)
            mProject.mChannel.SetValues(mProject.PrjFile)

            mProject.PrjFile.WriteXml(mProject.ProjectPathName)

            If mProject.mSimulationType = cGRM.SimulationType.SingleEvent Then
                Console.WriteLine(cProject.Current.ProjectPathName & " Is saved.  " + cGRM.BuildInfo.ProductName)
            End If
        Else
            Console.WriteLine(String.Format("Current project path {0} Is Not exist.   ", Path.GetDirectoryName(mProject.ProjectPath)))
            Exit Sub
        End If
    End Sub

    Public Shared Sub SaveAsProject(ByVal prjPathName As String)
        CopyAndUpdateFiles(mProject, prjPathName)
        SetProjectVariables(mProject, prjPathName)
        cProject.Current.SaveProject()
    End Sub


    Private Shared Sub CopyAndUpdateFiles(ByVal SourceProject As cProject, ByVal TargetPathName As String)
        With SourceProject
            Dim TargetProjectPathNameOnly As String = Path.Combine(Path.GetDirectoryName(TargetPathName), Path.GetFileNameWithoutExtension(TargetPathName))
            Dim Header As String = String.Format("Project name : {0} {1} {2}",
                                                  TargetPathName,
                                                  vbTab, Format(Now, "yyyy/MM/dd HH:mm"))
            '유량 파일
            If IO.File.Exists(.OFNPDischarge) = True Then
                FileCopy(.OFNPDischarge, TargetProjectPathNameOnly & cGRM.CONST_TAG_DISCHARGE)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_DISCHARGE, TargetProjectPathNameOnly & cGRM.CONST_TAG_DISCHARGE, 1, Header)
            End If

            '수심파일
            If IO.File.Exists(.OFNPDepth) = True Then
                FileCopy(.OFNPDepth, TargetProjectPathNameOnly & cGRM.CONST_TAG_DEPTH)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_DEPTH, TargetProjectPathNameOnly & cGRM.CONST_TAG_DEPTH, 1, Header)
            End If

            '격자 강우량파일
            If IO.File.Exists(.OFNPRFGrid) = True Then
                FileCopy(.OFNPRFGrid, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFGRID)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_RFGRID, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFGRID, 1, Header)
            End If

            '평균 강우량 파일
            If IO.File.Exists(.OFNPRFMean) = True Then
                FileCopy(.OFNPRFMean, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFMEAN)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_RFMEAN, TargetProjectPathNameOnly & cGRM.CONST_TAG_RFMEAN, 1, Header)
            End If

            'flow control 적용 정보 파일
            If IO.File.Exists(.OFNPFCData) = True Then
                FileCopy(.OFNPFCData, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCAPP)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_FCAPP, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCAPP, 1, Header)
            End If

            '저수지 파일
            If IO.File.Exists(.OFNPFCStorage) = True Then
                FileCopy(.OFNPFCStorage, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCSTORAGE)
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly & cGRM.CONST_TAG_FCSTORAGE, TargetProjectPathNameOnly & cGRM.CONST_TAG_FCSTORAGE, 1, Header)
            End If

            'WP별 모의결과, MREQ 
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
        End With
    End Sub


    Public Shared Sub SetProjectVariables(ByVal TargetProject As cProject, ByVal TargetPrjPathName As String)
        With TargetProject
            .ProjectNameWithExtension = Path.GetFileName(TargetPrjPathName)
            .ProjectNameOnly = Path.GetFileNameWithoutExtension(TargetPrjPathName)
            .ProjectPathName = TargetPrjPathName
            .ProjectPath = Path.GetDirectoryName(TargetPrjPathName)
            .geoDataPrjFPN = ""
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
        End With

        If TargetProject.PrjFile.ProjectSettings.Rows.Count > 0 Then
            Dim dtSettings As GRMProject.ProjectSettingsDataTable = TargetProject.PrjFile.ProjectSettings
            Dim settings As GRMProject.ProjectSettingsRow = CType(dtSettings.Rows(0), GRMProject.ProjectSettingsRow)
            With settings
                .ProjectFile = TargetPrjPathName
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

        If r.IsGRMSimulationTypeNull OrElse r.GRMSimulationType = "" Then
            Console.WriteLine(String.Format("GRMSimulationType is invalid!!"))
            Return False
        End If

        If Not r.IsWatershedFileNull AndAlso r.WatershedFile <> "" Then
            If File.Exists(r.WatershedFile) = False Then
                Console.WriteLine(String.Format("Watershed file is not exist!! {0} {1}", vbCrLf, r.WatershedFile))
                Return False
            End If
        Else
            Console.WriteLine(String.Format("Watershed file is not exist!! {0} {1}", vbCrLf, r.WatershedFile))
            Return False
        End If

        If Not r.IsSlopeFileNull AndAlso r.SlopeFile <> "" Then
            If File.Exists(r.SlopeFile) = False Then
                Console.WriteLine(String.Format("Slope file is not exist!! {0} {1}", vbCrLf, r.SlopeFile))
                Return False
            End If
        Else
            Console.WriteLine(String.Format("Slope file is not exist!! {0} {1}", vbCrLf, r.SlopeFile))
            Return False
        End If

        If Not r.IsFlowDirectionFileNull AndAlso r.FlowDirectionFile <> "" Then
            If File.Exists(r.FlowDirectionFile) = False Then
                Console.WriteLine(String.Format("Flow direction file is not exist!! {0} {1}", vbCrLf, r.FlowDirectionFile))
                Return False
            End If
        Else
            Console.WriteLine(String.Format("Flow direction file is not exist!! {0} {1}", vbCrLf, r.FlowDirectionFile))
            Return False
        End If

        If Not r.IsFlowAccumFileNull AndAlso r.FlowAccumFile <> "" Then
            If File.Exists(r.FlowAccumFile) = False Then
                Console.WriteLine(String.Format("Flow accumulation file is not exist!! {0} {1}", vbCrLf, r.FlowAccumFile))
                Return False
            End If
        Else
            Console.WriteLine(String.Format("Flow accumulation file is not exist!! {0} {1}", vbCrLf, r.FlowAccumFile))
            Return False
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

        If r.IsLandCoverDataTypeNull OrElse r.LandCoverDataType = "" Then
            Console.WriteLine(String.Format("LandCoverDataType is invalid!!"))
            Return False
        End If

        If Not r.IsLandCoverFileNull AndAlso r.LandCoverFile <> "" Then
            If File.Exists(r.LandCoverFile) = False Then
                Console.WriteLine(String.Format("Land cover file is not exist!! {0} {1}", vbCrLf, r.LandCoverFile))
                Return False
            Else
                If Not r.IsLandCoverVATFileNull AndAlso r.LandCoverVATFile <> "" Then
                    If File.Exists(r.LandCoverVATFile) = False Then
                        Console.WriteLine(String.Format("Land cover VAT file is not exist!! {0} {1}", vbCrLf, r.LandCoverVATFile))
                        Return False
                    End If
                Else
                    Console.WriteLine(String.Format("Land cover VAT file is invalid!!"))
                    Return False
                End If
            End If
        End If

        If Not r.IsConstantRoughnessCoeffNull AndAlso r.ConstantRoughnessCoeff <> "" Then
            Dim v As Single
            If Single.TryParse(r.ConstantRoughnessCoeff, v) = False Then
                Console.WriteLine(String.Format("ConstantRoughnessCoeff is invalid!! {0} {1}", vbCrLf, r.ConstantRoughnessCoeff))
                Return False
            End If
        End If

        If Not r.IsConstantImperviousRatioNull AndAlso r.ConstantImperviousRatio <> "" Then
            Dim v As Single
            If Single.TryParse(r.ConstantImperviousRatio, v) = False Then
                Console.WriteLine(String.Format("ConstantImperviousRatio is invalid!! {0} {1}", vbCrLf, r.ConstantImperviousRatio))
                Return False
            End If
        End If

        If r.IsSoilTextureDataTypeNull OrElse r.SoilTextureDataType = "" Then
            Console.WriteLine(String.Format("SoilTextureDataType is invalid!!"))
            Return False
        End If

        If Not r.IsSoilTextureFileNull AndAlso r.SoilTextureFile <> "" Then
            If File.Exists(r.SoilTextureFile) = False Then
                Console.WriteLine(String.Format("Soil texture file is not exist!! {0} {1}", vbCrLf, r.SoilTextureFile))
                Return False
            Else
                If Not r.IsSoilTextureVATFileNull AndAlso r.SoilTextureVATFile <> "" Then
                    If File.Exists(r.SoilTextureVATFile) = False Then
                        Console.WriteLine(String.Format("Soil texture VAT file is not exist!! {0} {1}", vbCrLf, r.SoilTextureVATFile))
                        Return False
                    End If
                Else
                    Console.WriteLine(String.Format("Soil texture VAT file is invalid!!"))
                    Return False
                End If
            End If
        End If

        If Not r.IsConstantSoilPorosityNull AndAlso r.ConstantSoilPorosity <> "" Then
            Dim v As Single
            If Single.TryParse(r.ConstantSoilPorosity, v) = False Then
                Console.WriteLine(String.Format("ConstantSoilPorosity is invalid!! {0} {1}", vbCrLf, r.ConstantSoilPorosity))
                Return False
            End If
        End If

        If Not r.IsConstantSoilEffPorosityNull AndAlso r.ConstantSoilEffPorosity <> "" Then
            Dim v As Single
            If Single.TryParse(r.ConstantSoilEffPorosity, v) = False Then
                Console.WriteLine(String.Format("ConstantSoilEffPorosity is invalid!! {0} {1}", vbCrLf, r.ConstantSoilEffPorosity))
                Return False
            End If
        End If

        If Not r.IsConstantSoilWettingFrontSuctionHeadNull AndAlso r.ConstantSoilWettingFrontSuctionHead <> "" Then
            Dim v As Single
            If Single.TryParse(r.ConstantSoilWettingFrontSuctionHead, v) = False Then
                Console.WriteLine(String.Format("ConstantSoilWettingFrontSuctionHead is invalid!! {0} {1}", vbCrLf, r.ConstantSoilWettingFrontSuctionHead))
                Return False
            End If
        End If

        If Not r.IsConstantSoilHydraulicConductivityNull AndAlso r.ConstantSoilHydraulicConductivity <> "" Then
            Dim v As Single
            If Single.TryParse(r.ConstantSoilHydraulicConductivity, v) = False Then
                Console.WriteLine(String.Format("ConstantSoilHydraulicConductivity is invalid!! {0} {1}", vbCrLf, r.ConstantSoilHydraulicConductivity))
                Return False
            End If
        End If

        If r.IsSoilDepthDataTypeNull OrElse r.SoilDepthDataType = "" Then
            Console.WriteLine(String.Format("SoilDepthDataType is invalid!!"))
            Return False
        End If

        If Not r.IsSoilDepthFileNull AndAlso r.SoilTextureFile <> "" Then
            If File.Exists(r.SoilDepthFile) = False Then
                Console.WriteLine(String.Format("Soil depth file is not exist!! {0} {1}", vbCrLf, r.SoilTextureFile))
                Return False
            Else
                If Not r.IsSoilDepthVATFileNull AndAlso r.SoilDepthVATFile <> "" Then
                    If File.Exists(r.SoilDepthVATFile) = False Then
                        Console.WriteLine(String.Format("Soil depth VAT file is not exist!! {0} {1}", vbCrLf, r.SoilDepthVATFile))
                        Return False
                    End If
                Else
                    Console.WriteLine(String.Format("Soil depth VAT file is invalid!!"))
                    Return False
                End If
            End If
        End If

        If Not r.IsConstantSoilDepthNull AndAlso r.ConstantSoilDepth <> "" Then
            Dim v As Single
            If Single.TryParse(r.ConstantSoilDepth, v) = False Then
                Console.WriteLine(String.Format("ConstantSoilDepth is invalid!! {0} {1}", vbCrLf, r.ConstantSoilDepth))
                Return False
            End If
        End If

        If Not r.IsInitialSoilSaturationRatioFileNull AndAlso r.InitialSoilSaturationRatioFile <> "" Then
            If File.Exists(r.InitialSoilSaturationRatioFile) = False Then
                Console.WriteLine(String.Format("InitialSoilSaturationRatioFile is not exist!! {0} {1}", vbCrLf, r.InitialSoilSaturationRatioFile))
                Return False
            End If
        End If

        If Not r.IsInitialChannelFlowFileNull AndAlso r.InitialChannelFlowFile <> "" Then
            If File.Exists(r.InitialChannelFlowFile) = False Then
                Console.WriteLine(String.Format("InitialChannelFlowFile is not exist!! {0} {1}", vbCrLf, r.InitialChannelFlowFile))
                Return False
            End If
        End If

        If r.IsRainfallDataTypeNull OrElse r.RainfallDataType = "" Then
            Console.WriteLine(String.Format("RainfallDataType is invalid!!"))
            Return False
        End If

        If Not r.IsRainfallIntervalNull AndAlso r.RainfallInterval <> "" Then
            Dim v As Integer
            If Integer.TryParse(r.RainfallInterval, v) = False Then
                Console.WriteLine(String.Format("Rainfall data interval is invalid!! {0} {1}", vbCrLf, r.RainfallInterval))
                Return False
            End If
        Else
            Return False
        End If

        If Not r.IsRainfallDataFileNull AndAlso r.RainfallDataFile <> "" Then
            If File.Exists(r.RainfallDataFile) = False Then
                Console.WriteLine(String.Format("Rainfall data file is not exist!! {0} {1}", vbCrLf, r.RainfallDataFile))
                Return False
            End If
        Else
            Return False
        End If

        If r.IsFlowDirectionTypeNull OrElse r.FlowDirectionType = "" Then
            Console.WriteLine(String.Format("FlowDirectionType is invalid!!"))
            Return False
        End If

        If Not r.IsGridCellSizeNull AndAlso r.GridCellSize <> "" Then
            Dim v As Integer
            If Integer.TryParse(r.GridCellSize, v) = False Then
                Console.WriteLine(String.Format("Grid cell size is invalid!! {0} {1}", vbCrLf, r.GridCellSize))
                Return False
            End If
        Else
            Return False
        End If

        If Not r.IsIsParallelNull AndAlso r.IsParallel = "" Then
            Console.WriteLine(String.Format("IsParallel is invalid!!"))
            Return False
        End If

        If Not r.IsMaxDegreeOfParallelismNull AndAlso r.MaxDegreeOfParallelism <> "" Then
            Dim v As Integer
            If Integer.TryParse(r.MaxDegreeOfParallelism, v) = False Then
                Console.WriteLine(String.Format("Grid cell size is invalid!! {0} {1}", vbCrLf, r.MaxDegreeOfParallelism))
                Return False
            End If
        End If

        If r.IsSimulStartingTimeNull OrElse r.SimulStartingTime = "" Then
            Console.WriteLine(String.Format("Simulation StartingTime is invalid!!"))
            Return False
        End If

        If Not r.IsSimulationDurationNull AndAlso r.SimulationDuration <> "" Then
            Dim v As Integer
            If Integer.TryParse(r.SimulationDuration, v) = False Then
                Console.WriteLine(String.Format("Simulation duration is invalid!! {0} {1}", vbCrLf, r.SimulationDuration))
                Return False
            End If
        Else
            Return False
        End If

        If Not r.IsComputationalTimeStepNull AndAlso r.ComputationalTimeStep <> "" Then
            Dim v As Integer
            If Integer.TryParse(r.ComputationalTimeStep, v) = False Then
                Console.WriteLine(String.Format("Computational time step is invalid!! {0} {1}", vbCrLf, r.ComputationalTimeStep))
                Return False
            End If
        Else
            Return False
        End If

        If Not r.IsIsFixedTimeStepNull AndAlso r.IsFixedTimeStep = "" Then
            Console.WriteLine(String.Format("IsFixedTimeStep is invalid!!"))
            Return False
        End If

        If Not r.IsOutputTimeStepNull AndAlso r.OutputTimeStep <> "" Then
            Dim v As Integer
            If Integer.TryParse(r.OutputTimeStep, v) = False Then
                Console.WriteLine(String.Format("Output time step is invalid!! {0} {1}", vbCrLf, r.OutputTimeStep))
                Return False
            End If
        Else
            Return False
        End If

        If Not r.IsCrossSectionTypeNull AndAlso r.CrossSectionType = "" Then
            Console.WriteLine(String.Format("CrossSectionType is invalid!!"))
            Return False
        End If

        If Not r.IsSingleCSChannelWidthTypeNull AndAlso r.SingleCSChannelWidthType = "" Then
            Console.WriteLine(String.Format("SingleCSChannelWidthType is invalid!!"))
            Return False
        End If

        If Not r.IsChannelWidthEQcNull AndAlso r.ChannelWidthEQc <> "" Then
            Dim v As Single
            If Single.TryParse(r.ChannelWidthEQc, v) = False Then
                Console.WriteLine(String.Format("ChannelWidthEQc is invalid!! {0} {1}", vbCrLf, r.ChannelWidthEQc))
                Return False
            End If
        End If

        If Not r.IsChannelWidthEQdNull AndAlso r.ChannelWidthEQd <> "" Then
            Dim v As Single
            If Single.TryParse(r.ChannelWidthEQd, v) = False Then
                Console.WriteLine(String.Format("ChannelWidthEQd is invalid!! {0} {1}", vbCrLf, r.ChannelWidthEQd))
                Return False
            End If
        End If

        If Not r.IsChannelWidthEQeNull AndAlso r.ChannelWidthEQe <> "" Then
            Dim v As Single
            If Single.TryParse(r.ChannelWidthEQe, v) = False Then
                Console.WriteLine(String.Format("ChannelWidthEQe is invalid!! {0} {1}", vbCrLf, r.ChannelWidthEQe))
                Return False
            End If
        End If

        If Not r.IsChannelWidthMostDownStreamNull AndAlso r.ChannelWidthMostDownStream <> "" Then
            Dim v As Single
            If Single.TryParse(r.ChannelWidthMostDownStream, v) = False Then
                Console.WriteLine(String.Format("ChannelWidthMostDownStream is invalid!! {0} {1}", vbCrLf, r.ChannelWidthMostDownStream))
                Return False
            End If
        End If

        If Not r.IsLowerRegionHeightNull AndAlso r.LowerRegionHeight <> "" Then
            Dim v As Single
            If Single.TryParse(r.LowerRegionHeight, v) = False Then
                Console.WriteLine(String.Format("LowerRegionHeight is invalid!! {0} {1}", vbCrLf, r.LowerRegionHeight))
                Return False
            End If
        End If

        If Not r.IsLowerRegionBaseWidthNull AndAlso r.LowerRegionBaseWidth <> "" Then
            Dim v As Single
            If Single.TryParse(r.LowerRegionBaseWidth, v) = False Then
                Console.WriteLine(String.Format("LowerRegionBaseWidth is invalid!! {0} {1}", vbCrLf, r.LowerRegionBaseWidth))
                Return False
            End If
        End If

        If Not r.IsUpperRegionBaseWidthNull AndAlso r.UpperRegionBaseWidth <> "" Then
            Dim v As Single
            If Single.TryParse(r.UpperRegionBaseWidth, v) = False Then
                Console.WriteLine(String.Format("UpperRegionBaseWidth is invalid!! {0} {1}", vbCrLf, r.UpperRegionBaseWidth))
                Return False
            End If
        End If

        If Not r.IsCompoundCSIniFlowDepthNull AndAlso r.CompoundCSIniFlowDepth <> "" Then
            Dim v As Single
            If Single.TryParse(r.CompoundCSIniFlowDepth, v) = False Then
                Console.WriteLine(String.Format("CompoundCSIniFlowDepth is invalid!! {0} {1}", vbCrLf, r.CompoundCSIniFlowDepth))
                Return False
            End If
        End If

        If Not r.IsCompoundCSChannelWidthLimitNull AndAlso r.CompoundCSChannelWidthLimit <> "" Then
            Dim v As Single
            If Single.TryParse(r.CompoundCSChannelWidthLimit, v) = False Then
                Console.WriteLine(String.Format("CompoundCSChannelWidthLimit is invalid!! {0} {1}", vbCrLf, r.CompoundCSChannelWidthLimit))
                Return False
            End If
        End If

        If Not r.IsBankSideSlopeRightNull AndAlso r.BankSideSlopeRight <> "" Then
            Dim v As Single
            If Single.TryParse(r.BankSideSlopeRight, v) = False Then
                Console.WriteLine(String.Format("BankSideSlopeRight is invalid!! {0} {1}", vbCrLf, r.BankSideSlopeRight))
                Return False
            End If
        End If

        If Not r.IsBankSideSlopeLeftNull AndAlso r.BankSideSlopeLeft <> "" Then
            Dim v As Single
            If Single.TryParse(r.BankSideSlopeLeft, v) = False Then
                Console.WriteLine(String.Format("BankSideSlopeLeft is invalid!! {0} {1}", vbCrLf, r.BankSideSlopeLeft))
                Return False
            End If
        End If

        If r.IsSimulateInfiltrationNull OrElse r.SimulateInfiltration = "" Then
            Console.WriteLine(String.Format("SimulateInfiltration option  is invalid!!"))
            Return False
        End If

        If r.IsSimulateSubsurfaceFlowNull OrElse r.SimulateSubsurfaceFlow = "" Then
            Console.WriteLine(String.Format("SimulateSubsurfaceFlow option  is invalid!!"))
            Return False
        End If

        If r.IsSimulateBaseFlowNull OrElse r.SimulateBaseFlow = "" Then
            Console.WriteLine(String.Format("SimulateBaseFlow option  is invalid!!"))
            Return False
        End If

        If r.IsSimulateFlowControlNull OrElse r.SimulateFlowControl = "" Then
            Console.WriteLine(String.Format("SimulateFlowControl option  is invalid!!"))
            Return False
        End If

        If r.IsMakeIMGFileNull OrElse r.MakeIMGFile = "" Then
            Console.WriteLine(String.Format("MakeIMGFile option  is invalid!!"))
            Return False
        End If

        If r.IsMakeASCFileNull OrElse r.MakeASCFile = "" Then
            Console.WriteLine(String.Format("MakeASCFile option  is invalid!!"))
            Return False
        End If

        If r.IsMakeSoilSaturationDistFileNull OrElse r.MakeSoilSaturationDistFile = "" Then
            Console.WriteLine(String.Format("MakeSoilSaturationDistFile option  is invalid!!"))
            Return False
        End If

        If r.IsMakeRfDistFileNull OrElse r.MakeRfDistFile = "" Then
            Console.WriteLine(String.Format("MakeRfDistFile option  is invalid!!"))
            Return False
        End If

        If r.IsMakeRFaccDistFileNull OrElse r.MakeRFaccDistFile = "" Then
            Console.WriteLine(String.Format("MakeRFaccDistFile option  is invalid!!"))
            Return False
        End If

        If r.IsMakeFlowDistFileNull OrElse r.MakeFlowDistFile = "" Then
            Console.WriteLine(String.Format("MakeFlowDistFile option  is invalid!!"))
            Return False
        End If

        If r.IsPrintOptionNull OrElse r.PrintOption = "" Then
            Console.WriteLine(String.Format("PrintOption is invalid!!"))
            Return False
        End If

        If Not r.IsWriteLogNull AndAlso r.WriteLog = "" Then
            Console.WriteLine(String.Format("WriteLog option  is invalid!!"))
            Return False
        End If


    End Function

End Class
