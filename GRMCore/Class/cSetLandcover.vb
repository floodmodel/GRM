Public Class cSetLandcover

    Public Enum LandCoverCode
        WATR
        URBN
        BARE
        WTLD
        GRSS
        FRST
        AGRL
        CONST_VALUE
        NULL
    End Enum

    Public mLandCoverDataType As Nullable(Of cGRM.FileOrConst)
    Public mGridLandCoverFPN As String
    Public mLandCoverVATFPN As String
    Public mdtLandCoverInfo As GRMProject.LandCoverDataTable
    Public mConstRoughnessCoefficient As Nullable(Of Single)
    Public mConstImperviousRatio As Nullable(Of Single)

    Public Sub New()

    End Sub

    Public Sub SetValues(ByVal prjDB As GRMProject)
        If mLandCoverDataType.HasValue Then
            Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
            With row
                .LandCoverDataType = mLandCoverDataType.Value.ToString
                Select Case mLandCoverDataType.Value
                    Case cGRM.FileOrConst.File
                        .LandCoverFile = mGridLandCoverFPN
                        .LandCoverVATFile = mLandCoverVATFPN
                        .SetConstantRoughnessCoeffNull()
                        .SetConstantImperviousRatioNull()
                    Case cGRM.FileOrConst.Constant
                        .SetLandCoverFileNull()
                        .SetLandCoverVATFileNull()
                        .ConstantRoughnessCoeff = CStr(mConstRoughnessCoefficient.Value)
                        .ConstantImperviousRatio = CStr(mConstImperviousRatio.Value)
                End Select
            End With
            If mLandCoverDataType.Equals(cGRM.FileOrConst.File) Then
                mdtLandCoverInfo.AcceptChanges()
                For Each r As DataRow In mdtLandCoverInfo.Rows
                    r.SetAdded()
                Next
            End If
        End If
    End Sub

    Public Sub GetValues(ByVal prjDB As GRMProject)
        mLandCoverDataType = Nothing
        mLandCoverVATFPN = Nothing
        mConstRoughnessCoefficient = Nothing
        mConstImperviousRatio = Nothing
        Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            If Not .IsLandCoverDataTypeNull Then
                Select Case .LandCoverDataType
                    Case cGRM.FileOrConst.File.ToString
                        mLandCoverDataType = cGRM.FileOrConst.File
                        mGridLandCoverFPN = .LandCoverFile
                        mLandCoverVATFPN = .LandCoverVATFile
                    Case cGRM.FileOrConst.Constant.ToString
                        mLandCoverDataType = cGRM.FileOrConst.Constant
                        mConstImperviousRatio = CSng(.ConstantImperviousRatio)
                        mConstRoughnessCoefficient = CSng(.ConstantRoughnessCoeff)
                    Case Else
                        Throw New InvalidOperationException
                End Select
            End If
        End With
        If mLandCoverDataType.Equals(cGRM.FileOrConst.File) Then
            mdtLandCoverInfo = New GRMProject.LandCoverDataTable
            mdtLandCoverInfo = prjDB.LandCover
        Else
            mdtLandCoverInfo = Nothing
        End If
    End Sub

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return mLandCoverDataType.HasValue
        End Get
    End Property

    Public Function GetLandCoverName(ByVal intGridValue As Integer) As String
        If mLandCoverDataType.Equals(cGRM.FileOrConst.File) Then
            Dim rows As DataRow() = mdtLandCoverInfo.Select("GridValue = " & intGridValue)
            Return CStr(rows(0)("GRMLandCoverE"))
        Else
            Return "[CONST]"
        End If
    End Function

    Public Shared Function GetLandCoverCode(ByVal inName As String) As LandCoverCode
        Select Case Trim(inName)
            Case LandCoverCode.AGRL.ToString : Return cSetLandcover.LandCoverCode.AGRL
            Case LandCoverCode.BARE.ToString : Return cSetLandcover.LandCoverCode.BARE
            Case LandCoverCode.FRST.ToString : Return LandCoverCode.FRST
            Case LandCoverCode.GRSS.ToString : Return LandCoverCode.GRSS
            Case LandCoverCode.URBN.ToString : Return LandCoverCode.URBN
            Case LandCoverCode.WATR.ToString : Return LandCoverCode.WATR
            Case LandCoverCode.WTLD.ToString : Return LandCoverCode.WTLD
            Case Else : Return Nothing
        End Select
    End Function

End Class
