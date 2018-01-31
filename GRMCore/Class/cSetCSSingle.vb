Public Class cSetCSSingle
    Inherits cSetCrossSection
    Public Enum CSSingleChannelWidthType
        CWEquation
        CWGeneration
    End Enum

    Public mCSSingleWidthType As Nullable(Of CSSingleChannelWidthType)
    Public mCWEc As Single
    Public mCWEd As Single
    Public mCWEe As Single
    Public mMaxChannelWidthSingleCS As Single

    Public Overrides ReadOnly Property CSType() As cSetCrossSection.CSTypeEnum
        Get
            Return CSTypeEnum.CSSingle
        End Get
    End Property

    Public Overrides Sub GetValues(ByVal prjdb As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            If .SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation.ToString Then
                mCSSingleWidthType = CSSingleChannelWidthType.CWEquation
                mCWEc = CSng(.ChannelWidthEQc)
                mCWEd = CSng(.ChannelWidthEQd)
                mCWEe = CSng(.ChannelWidthEQe)
            Else
                mCSSingleWidthType = CSSingleChannelWidthType.CWGeneration
                mMaxChannelWidthSingleCS = CSng(.ChannelWidthMostDownStream)
            End If
        End With
    End Sub

    Public Overrides ReadOnly Property IsSet() As Boolean
        Get
            Return mCSSingleWidthType.HasValue
        End Get
    End Property

    Public Overrides Sub SetValues(ByVal prjDB As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            .CrossSectionType = CSTypeEnum.CSSingle.ToString
            If mCSSingleWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation Then
                .SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation.ToString
                .ChannelWidthEQc = CStr(mCWEc)
                .ChannelWidthEQd = CStr(mCWEd)
                .ChannelWidthEQe = CStr(mCWEe)
                .ChannelWidthMostDownStream = Nothing
            ElseIf mCSSingleWidthType = cSetCSSingle.CSSingleChannelWidthType.CWGeneration Then
                .SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWGeneration.ToString
                .ChannelWidthMostDownStream = CStr(mMaxChannelWidthSingleCS)
                .ChannelWidthEQc = Nothing
                .ChannelWidthEQd = Nothing
                .ChannelWidthEQe = Nothing
            End If
            .CompoundCSIniFlowDepth = Nothing
            .LowerRegionBaseWidth = Nothing
            .LowerRegionHeight = Nothing
            .UpperRegionBaseWidth = Nothing
            .CompoundCSChannelWidthLimit = Nothing
        End With
    End Sub

End Class
