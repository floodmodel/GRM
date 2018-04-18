Public Class cSetSoilDepth
    Public Enum SoilDepthCode
        D
        M
        S
        VD
        VS
        USER
        CONSTV
        NULL
    End Enum

    Public mSoilDepthDataType As Nullable(Of cGRM.FileOrConst)
    Public mGridSoilDepthFPN As String
    Public mSoilDepthVATFPN As String
    Public mdtSoilDepthInfo As GRMProject.SoilDepthDataTable
    Public mConstSoilDepth As Nullable(Of Single)

    Public Sub New()

    End Sub

    Public Sub SetValues(ByVal prjdb As GRMProject)
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
                        .ConstantSoilDepth = CStr(mConstSoilDepth.Value)
                End Select
            End With
        End If
        If mSoilDepthDataType.Equals(cGRM.FileOrConst.File) Then
            mdtSoilDepthInfo.AcceptChanges()
            For Each r As DataRow In mdtSoilDepthInfo.Rows
                r.SetAdded()
            Next
        End If

    End Sub

    Public Sub GetValues(ByVal prjdb As GRMProject)
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
                        mConstSoilDepth = CSng(.ConstantSoilDepth)
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

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return mSoilDepthDataType.HasValue
        End Get
    End Property

    Public Shared Function GetSoilDepthCode(ByVal inName As String) As SoilDepthCode
        Select Case Trim(inName)
            Case SoilDepthCode.D.ToString : Return SoilDepthCode.D
            Case SoilDepthCode.M.ToString : Return SoilDepthCode.M
            Case SoilDepthCode.S.ToString : Return SoilDepthCode.S
            Case SoilDepthCode.VD.ToString : Return SoilDepthCode.VD
            Case SoilDepthCode.VS.ToString : Return SoilDepthCode.VS
            Case SoilDepthCode.USER.ToString : Return SoilDepthCode.USER
            Case SoilDepthCode.CONSTV.ToString : Return SoilDepthCode.CONSTV
            Case SoilDepthCode.NULL.ToString : Return SoilDepthCode.NULL
            Case Else : Return Nothing
        End Select
    End Function
End Class
