Public Class cSetSoilDepth
    'Implements ioProjectFile
    'Implements ioProjectDB

    Public Enum SoilDepthCode
        D
        MDMS
        S
        VD
        VS
        NULL
    End Enum

    Public mSoilDepthDataType As Nullable(Of cGRM.FileOrConst)
    'Public mGridNameSoilDepth As String
    Public mGridSoilDepthFPN As String
    Public mSoilDepthVATFPN As String
    Public mdtSoilDepthInfo As GRMProject.SoilDepthDataTable
    Public mConstSoilDepth As Nullable(Of Single)

    Public Sub New()

    End Sub

    Public Sub SetValues(ByVal prjdb As GRMProject) 'Implements ioProjectFile.SetValues
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        If mSoilDepthDataType.HasValue Then
            With row
                .SoilDepthDataType = mSoilDepthDataType.Value.ToString
                Select Case mSoilDepthDataType.Value
                    Case cGRM.FileOrConst.File
                        .SoilDepthFile = mGridSoilDepthFPN
                        .SoilDepthVATFile = mSoilDepthVATFPN
                        .SetConstantSoilDepthNull()
                    Case cGRM.FileOrConst.Constant
                        .SetSoilDepthFileNull()
                        .SetSoilDepthVATFileNull()
                        .ConstantSoilDepth = mConstSoilDepth.Value
                End Select
            End With
        End If
        If mSoilDepthDataType.Equals(cGRM.FileOrConst.File) Then
            mdtSoilDepthInfo.AcceptChanges()
            For Each r As DataRow In mdtSoilDepthInfo.Rows
                r.SetAdded()
            Next
            'mdtSoilDepthInfo.WriteXml(prjDBxmlFpn)
        End If

    End Sub

    Public Sub GetValues(ByVal prjdb As GRMProject) 'Implements ioProjectFile.GetValues
        '  일단 초기화
        mSoilDepthDataType = Nothing
        mSoilDepthVATFPN = Nothing
        mConstSoilDepth = Nothing
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            If Not .IsSoilDepthDataTypeNull Then
                Select Case .SoilDepthDataType
                    Case cGRM.FileOrConst.File.ToString
                        mSoilDepthDataType = cGRM.FileOrConst.File
                        mGridSoilDepthFPN = .SoilDepthFile
                        mSoilDepthVATFPN = .SoilDepthVATFile
                    Case cGRM.FileOrConst.Constant.ToString
                        mSoilDepthDataType = cGRM.FileOrConst.Constant
                        mConstSoilDepth = .ConstantSoilDepth
                    Case Else
                        Throw New InvalidOperationException
                End Select
            End If
        End With
        If mSoilDepthDataType.Equals(cGRM.FileOrConst.File) Then
            mdtSoilDepthInfo = New GRMProject.SoilDepthDataTable
            mdtSoilDepthInfo = prjdb.SoilDepth
        Else
            mdtSoilDepthInfo = Nothing
        End If

    End Sub

    'Public Sub Save(ByVal prjDBxmlFpn As String) Implements ioProjectDB.SaveDB
    '    'Dim adpt As New GRMDynamicDBTableAdapters.SoilDepthTableAdapter
    '    'adpt.Connection = conn
    '    'adpt.DeleteAll()
    '    If mSoilDepthDataType.Equals(LayerOrConst.Layer) Then
    '        mdtSoilDepthInfo.AcceptChanges()
    '        For Each row As DataRow In mdtSoilDepthInfo.Rows
    '            row.SetAdded()
    '        Next
    '        'adpt.Update(mdtSoilDepthInfo)
    '        mdtSoilDepthInfo.WriteXml(prjDBxmlFpn)
    '    End If
    'End Sub

    'Public Sub Open(ByVal prjDBxmlFpn As String) Implements ioProjectDB.OpenDB
    '    If mSoilDepthDataType.Equals(LayerOrConst.Layer) Then
    '        'Dim adpt As New GRMDynamicDBTableAdapters.SoilDepthTableAdapter
    '        'adpt.Connection = conn
    '        mdtSoilDepthInfo = New GRMDynamicDB.SoilDepthDataTable
    '        'adpt.Fill(mdtSoilDepthInfo)
    '        mdtSoilDepthInfo.ReadXml(prjDBxmlFpn)
    '    Else
    '        mdtSoilDepthInfo = Nothing
    '    End If
    'End Sub

    Public ReadOnly Property IsSet() As Boolean ' Implements ioProjectFile.IsSet
        Get
            Return mSoilDepthDataType.HasValue
        End Get
    End Property

    Public Shared Function GetSoilDepthCode(ByVal inName As String) As SoilDepthCode
        Select Case Trim(inName)
            Case SoilDepthCode.D.ToString : Return SoilDepthCode.D
            Case SoilDepthCode.MDMS.ToString : Return SoilDepthCode.MDMS
            Case SoilDepthCode.S.ToString : Return SoilDepthCode.S
            Case SoilDepthCode.VD.ToString : Return SoilDepthCode.VD
            Case SoilDepthCode.VS.ToString : Return SoilDepthCode.VS
            Case SoilDepthCode.NULL.ToString : Return SoilDepthCode.NULL
            Case Else : Return Nothing
        End Select
    End Function
End Class
