Public Class cSetWatershed
    Public mFPN_watershed As String
    Public mFPN_slope As String
    Public mFPN_fdir As String
    Public mFPN_fac As String
    Public mFPN_stream As String
    Public mFPN_channelWidth As String
    Public mFPN_initialChannelFlow As String
    Public mFPN_initialSoilSaturationRatio As String

    ''' <summary>
    ''' 격자의 크기[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public mCellSize As Integer = -1
    Public mFDType As cGRM.FlowDirectionType

    ''' <summary>
    ''' 격자 가로 셀 개수
    ''' </summary>
    ''' <remarks></remarks>
    Public mColCount As Integer = -1

    ''' <summary>
    ''' 격자 세로 셀 개수
    ''' </summary>
    ''' <remarks></remarks>
    Public mRowCount As Integer = -1
    Public mxllcorner As Double
    Public myllcorner As Double

    ''' <summary>
    ''' 대상 유역의 소유역 id 리스트
    ''' </summary>
    ''' <remarks></remarks>
    Public mWSIDList As List(Of Integer)

    ''' <summary>
    ''' 하도셀 중 최상류셀의 흐름누적수
    ''' </summary>
    ''' <remarks></remarks>
    Public mFacMostUpChannelCell As Integer

    ''' <summary>
    ''' 최대흐름 누적수
    ''' </summary>
    ''' <remarks></remarks>
    Public mFacMax As Integer
    Public mFacMin As Integer

    ''' <summary>
    ''' 소유역별 셀 개수
    ''' </summary>
    ''' <remarks></remarks>
    Public mCVidListForEachWS As Dictionary(Of Integer, List(Of Integer))

    Public Sub New()
        mWSIDList = New List(Of Integer)
        mCVidListForEachWS = New Dictionary(Of Integer, List(Of Integer))
    End Sub

    Public Sub SetValues(ByVal prjdb As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            .WatershedFile = mFPN_watershed
            .SlopeFile = mFPN_slope
            .FlowDirectionFile = mFPN_fdir
            .FlowAccumFile = mFPN_fac
            .StreamFile = mFPN_stream
            .ChannelWidthFile = mFPN_channelWidth
            .InitialSoilSaturationRatioFile = mFPN_initialSoilSaturationRatio
            .InitialChannelFlowFile = mFPN_initialChannelFlow
            '.GridCellSize = CStr(mCellSize)
            .FlowDirectionType = mFDType.ToString
        End With
    End Sub

    Public Sub GetValues(ByVal prjdb As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            If Not .IsWatershedFileNull Then
                mFPN_watershed = .WatershedFile
            End If
            If Not .IsSlopeFileNull Then
                mFPN_slope = .SlopeFile
            End If
            If Not .IsFlowDirectionFileNull Then
                mFPN_fdir = .FlowDirectionFile
            End If
            If Not .IsFlowAccumFileNull Then
                mFPN_fac = .FlowAccumFile
            End If
            If Not .IsStreamFileNull Then
                mFPN_stream = .StreamFile
            End If
            If .IsChannelWidthFileNull = False Then
                mFPN_channelWidth = .ChannelWidthFile
            End If
            If .IsInitialChannelFlowFileNull = False Then
                mFPN_initialChannelFlow = .InitialChannelFlowFile
            End If
            If .IsInitialSoilSaturationRatioFileNull = False Then
                mFPN_initialSoilSaturationRatio = .InitialSoilSaturationRatioFile
            End If
            'If .IsGridCellSizeNull Then
            '    mCellSize = -1
            'Else
            '    mCellSize = CInt(.GridCellSize)
            'End If
            If Not .IsFlowDirectionTypeNull Then
                Select Case .FlowDirectionType
                    Case cGRM.FlowDirectionType.StartsFromN.ToString
                        mFDType = cGRM.FlowDirectionType.StartsFromN
                    Case cGRM.FlowDirectionType.StartsFromNE.ToString
                        mFDType = cGRM.FlowDirectionType.StartsFromNE
                    Case cGRM.FlowDirectionType.StartsFromE.ToString
                        mFDType = cGRM.FlowDirectionType.StartsFromE
                    Case cGRM.FlowDirectionType.StartsFromE_TauDEM.ToString
                        mFDType = cGRM.FlowDirectionType.StartsFromE_TauDEM
                    Case Else
                        mFDType = cGRM.FlowDirectionType.StartsFromE_TauDEM
                End Select
            Else
                mFDType = cGRM.FlowDirectionType.StartsFromE_TauDEM
            End If
        End With
    End Sub

    Public ReadOnly Property HasStreamLayer() As Boolean
        Get
            Return Not String.IsNullOrEmpty(mFPN_stream)
        End Get
    End Property

    Public ReadOnly Property HasChannelWidthLayer() As Boolean
        Get
            Return Not String.IsNullOrEmpty(mFPN_channelWidth)
        End Get
    End Property

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return Not String.IsNullOrEmpty(mFPN_watershed)
        End Get
    End Property

    ''' <summary>
    ''' 분석대상 유역의 소유역 id 리스트
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property WSIDList() As List(Of Integer)
        Get
            mWSIDList.Sort()
            Return mWSIDList
        End Get
    End Property

    Public ReadOnly Property CellCountforEachWSID(ByVal wsid As Integer) As Integer
        Get
            Return mCVidListForEachWS(wsid).Count
        End Get
    End Property

End Class
