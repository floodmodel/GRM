Imports GRM

Public Class cSetCSCompound
    Inherits cSetCrossSection


    '''' <summary>
    '''' 저부수의 깊이[m]
    '''' </summary>
    '''' <remarks></remarks>
    'Public mIniFlowDepth As Single

    ''' <summary>
    ''' 저수부의 하폭[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public mLowerRegionBaseWidth As Single

    ''' <summary>
    ''' 하상으로 부터 고수부가 시작되는 부분까지의 높이[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public mLowerRegionHeight As Single

    ''' <summary>
    ''' 고수부의 하폭[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public mUpperRegionBaseWidth As Single
    'Public mMinChannelWidthCompoundCS As Single

    ''' <summary>
    ''' 이거 보다 큰 하폭에서만 복단면 적용한다.[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public mCompoundCSCriteriaChannelWidth As Single

    '''' <summary>
    '''' 저수부 수면에서 복단면까지의 초기 높이 차
    '''' </summary>
    '''' <remarks></remarks>
    'Public mHeightFromLowerAreaWaterSurfaceToUpperArea_m As Single


    Public Overrides ReadOnly Property CSType() As cSetCrossSection.CSTypeEnum
        Get
            Return CSTypeEnum.CSCompound
        End Get
    End Property

    Public Overrides Sub GetValues(prjDB As GRMProject)

        Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            'mIniFlowDepth = .CompoundCSIniFlowDepth
            mLowerRegionBaseWidth = .LowerRegionBaseWidth
            mLowerRegionHeight = .LowerRegionHeight
            mUpperRegionBaseWidth = .UpperRegionBaseWidth
            'mMinChannelWidthCompoundCS = .ChannelWidthMinCompoundCS
            mCompoundCSCriteriaChannelWidth = .CompoundCSChannelWidthLimit
        End With
        Throw New NotImplementedException()
    End Sub

    Public Overrides ReadOnly Property IsSet() As Boolean
        Get
            Return mLowerRegionBaseWidth <> Nothing
        End Get
    End Property

    '''' <summary>
    '''' 기존 수심에서 얼마만큼의 수심이 더해지면 고수부로 전환되는지를 결정하는 변수
    '''' </summary>
    '''' <value></value>
    '''' <returns></returns>
    '''' <remarks></remarks>
    'Public Overridable ReadOnly Property CompoundCSCriteriaDepth_m() As Single
    '    Get
    '        Return mLowerRegionHeight - mIniFlowDepth
    '    End Get
    'End Property


    Public Overrides Sub SetValues(prjDB As GRMProject)

        Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            .CrossSectionType = CSTypeEnum.CSCompound.ToString
            '.CompoundCSIniFlowDepth = mIniFlowDepth
            .LowerRegionBaseWidth = mLowerRegionBaseWidth
            .LowerRegionHeight = mLowerRegionHeight
            .UpperRegionBaseWidth = mUpperRegionBaseWidth
            '.ChannelWidthMinCompoundCS = mMinChannelWidthCompoundCS
            .CompoundCSChannelWidthLimit = mCompoundCSCriteriaChannelWidth

            .SingleCSChannelWidthType = Nothing
            .ChannelWidthEQc = Nothing
            .ChannelWidthEQd = Nothing
            .ChannelWidthEQe = Nothing
            .ChannelWidthMostDownStream = Nothing
            '.ChannelWidthMinSingleCS = Nothing
        End With
        Throw New NotImplementedException()

    End Sub


End Class
