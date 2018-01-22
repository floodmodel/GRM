Public Class cSetCSSingle
    Inherits cSetCrossSection


    Public Enum CSSingleChannelWidthType
        CWEquation
        CWGeneration
    End Enum

    Public mCSSingleWidthType As Nullable(Of CSSingleChannelWidthType)

    '2017.07.04. 최. 사용자 지정 하폭은 db가 아니고, GUI에서 channel width file로 저장하는 방향으로 수정. 
    'Public mdtUserChannelWidthInfo As GRMProject.UserChannelWidthDataTable
    ' Equation Type Vars
    Public mCWEc As Single
    Public mCWEd As Single
    Public mCWEe As Single
    ' Generation Type Vars
    Public mMaxChannelWidthSingleCS As Single
    'Public mMinChannelWidthSingleCS As Single

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
                mCWEc = .ChannelWidthEQc
                mCWEd = .ChannelWidthEQd
                mCWEe = .ChannelWidthEQe
            Else
                mCSSingleWidthType = CSSingleChannelWidthType.CWGeneration
                mMaxChannelWidthSingleCS = .ChannelWidthMostDownStream
                'mMinChannelWidthSingleCS = .ChannelWidthMinSingleCS
            End If
        End With

        '2017.07.04. 최. 사용자 지정 하폭은 db가 아니고, GUI에서 channel width file로 저장하는 방향으로 수정. 
        'If mCSSingleWidthType.HasValue Then
        '    mdtUserChannelWidthInfo = New GRMProject.UserChannelWidthDataTable
        '    mdtUserChannelWidthInfo = prjdb.UserChannelWidth
        'End If
    End Sub

    Public Overrides ReadOnly Property IsSet() As Boolean
        Get
            Return mCSSingleWidthType.HasValue
        End Get
    End Property

    Public Overrides Sub SetValues(ByVal prjDB As GRMProject)
        '2017.07.04. 최. 사용자 지정 하폭은 db가 아니고, GUI에서 channel width file로 저장하는 방향으로 수정. 
        '         여기서는 사용자 지정하폭 외에 하폭관련 매개변수만 받는다 
        'Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.UserChannelWidth.Rows(0), GRMProject.ProjectSettingsRow)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            .CrossSectionType = CSTypeEnum.CSSingle.ToString
            If mCSSingleWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation Then
                .SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation.ToString
                .ChannelWidthEQc = mCWEc
                .ChannelWidthEQd = mCWEd
                .ChannelWidthEQe = mCWEe
                .ChannelWidthMostDownStream = Nothing
            ElseIf mCSSingleWidthType = cSetCSSingle.CSSingleChannelWidthType.CWGeneration Then
                .SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWGeneration.ToString
                .ChannelWidthMostDownStream = mMaxChannelWidthSingleCS
                .ChannelWidthEQc = Nothing
                .ChannelWidthEQd = Nothing
                .ChannelWidthEQe = Nothing
                '.ChannelWidthMinSingleCS = mMinChannelWidthSingleCS
            End If

            .CompoundCSIniFlowDepth = Nothing
            .LowerRegionBaseWidth = Nothing
            .LowerRegionHeight = Nothing
            .UpperRegionBaseWidth = Nothing
            '.ChannelWidthMinCompoundCS = Nothing
            .CompoundCSChannelWidthLimit = Nothing
        End With

        '2017.07.04. 최. 사용자 지정 하폭은 db가 아니고, GUI에서 channel width file로 저장하는 방향으로 수정 
        'If IsSet Then
        '    If mdtUserChannelWidthInfo.Rows.Count > 0 Then
        '        mdtUserChannelWidthInfo.AcceptChanges()
        '        For Each r As DataRow In mdtUserChannelWidthInfo.Rows
        '            r.SetAdded()
        '        Next
        '    End If
        'End If
    End Sub

    'Public Overrides Sub OpenDB(ByVal prjDBxmlFpn As String)
    '    If mCSSingleWidthType.HasValue Then
    '        'Dim adpt As New GRMDynamicDBTableAdapters.UserChannelWidthTableAdapter
    '        'adpt.Connection = conn
    '        mdtUserChannelWidthInfo = New GRMDynamicDB.UserChannelWidthDataTable
    '        'adpt.Fill(mdtUserChannelWidthInfo)
    '        mdtUserChannelWidthInfo.ReadXml(prjDBxmlFpn)
    '    End If
    'End Sub

    'Public Overrides Sub SaveDB(prjDBxmlFpn As String)
    '    'Dim adpt As New GRMDynamicDBTableAdapters.UserChannelWidthTableAdapter
    '    'adpt.Connection = conn
    '    'adpt.DeleteAll()
    '    If IsSet Then
    '        If mdtUserChannelWidthInfo.Rows.Count > 0 Then
    '            mdtUserChannelWidthInfo.AcceptChanges()
    '            For Each row As DataRow In mdtUserChannelWidthInfo.Rows
    '                row.SetAdded()
    '            Next
    '            'adpt.Update(mdtUserChannelWidthInfo)
    '            mdtUserChannelWidthInfo.WriteXml(prjDBxmlFpn)
    '        End If
    '    End If
    'End Sub

    '2017.07.04. 최. 사용자 지정 하폭은 db가 아니고, GUI에서 channel width file로 저장하는 방향으로 수정 
    'Public ReadOnly Property UserEditChannelCellCount() As Integer
    '    Get
    '        Return mdtUserChannelWidthInfo.Rows.Count
    '    End Get
    'End Property
End Class
