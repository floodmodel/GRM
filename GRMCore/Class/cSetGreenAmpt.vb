Public Class cSetGreenAmpt

    Public Enum SoilTextureCode
        C
        CL
        L
        LS
        S
        SC
        SCL
        SiC
        SiCL
        SiL
        SL
        NULL
    End Enum

    Public mSoilTextureDataType As Nullable(Of cGRM.FileOrConst)
    Public mGridSoilTextureFPN As String
    Public mSoilTextureVATFPN As String
    Public mdtGreenAmptInfo As GRMProject.GreenAmptParameterDataTable
    Public mConstPorosity As Nullable(Of Single)
    Public mConstEffectivePorosity As Nullable(Of Single)
    Public mConstWFS As Nullable(Of Single)
    Public mConstHydraulicCond As Nullable(Of Single)

    Public Sub New()

    End Sub

    Public Sub SetValues(ByVal prjdb As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        If mSoilTextureDataType.HasValue Then
            With row
                .SoilTextureDataType = mSoilTextureDataType.Value.ToString
                If mSoilTextureDataType.Value = cGRM.FileOrConst.File Then
                    .SoilTextureFile = mGridSoilTextureFPN
                    .SoilTextureVATFile = mSoilTextureVATFPN
                    .SetConstantSoilPorosityNull()
                    .SetConstantSoilEffPorosityNull()
                    .SetConstantSoilWettingFrontSuctionHeadNull()
                    .SetConstantSoilHydraulicConductivityNull()
                Else
                    .SetSoilTextureFileNull()
                    .SetSoilTextureVATFileNull()
                    .ConstantSoilPorosity = mConstPorosity.Value
                    .ConstantSoilEffPorosity = mConstEffectivePorosity.Value
                    .ConstantSoilWettingFrontSuctionHead = mConstWFS.Value
                    .ConstantSoilHydraulicConductivity = mConstHydraulicCond.Value
                End If
            End With
        End If

        If mSoilTextureDataType.Equals(cGRM.FileOrConst.File) Then
            mdtGreenAmptInfo.AcceptChanges()
            For Each r As DataRow In mdtGreenAmptInfo.Rows
                r.SetAdded()
            Next
        End If

    End Sub

    Public Sub GetValues(ByVal prjdb As GRMProject)
        mSoilTextureDataType = Nothing
        mSoilTextureVATFPN = Nothing
        mConstPorosity = Nothing
        mConstEffectivePorosity = Nothing
        mConstWFS = Nothing
        mConstHydraulicCond = Nothing
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            If Not .IsSoilTextureDataTypeNull Then
                If .SoilTextureDataType = cGRM.FileOrConst.File.ToString Then
                    mSoilTextureDataType = cGRM.FileOrConst.File
                    mGridSoilTextureFPN = .SoilTextureFile
                    mSoilTextureVATFPN = .SoilTextureVATFile
                ElseIf .SoilTextureDataType = cGRM.FileOrConst.Constant.ToString Then
                    mSoilTextureDataType = cGRM.FileOrConst.Constant
                    mConstPorosity = .ConstantSoilPorosity
                    mConstEffectivePorosity = .ConstantSoilEffPorosity
                    mConstWFS = .ConstantSoilWettingFrontSuctionHead
                    mConstHydraulicCond = .ConstantSoilHydraulicConductivity
                Else
                    Throw New InvalidOperationException
                End If
            End If
        End With
        If mSoilTextureDataType.Equals(cGRM.FileOrConst.File) Then
            mdtGreenAmptInfo = New GRMProject.GreenAmptParameterDataTable
            mdtGreenAmptInfo = prjdb.GreenAmptParameter
        Else
            mdtGreenAmptInfo = Nothing
        End If
    End Sub

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return mSoilTextureDataType.HasValue
        End Get
    End Property

    Public Function GetSoilTextureName(ByVal intGridValue As Integer) As String
        If mSoilTextureDataType.Equals(cGRM.FileOrConst.File) Then
            Dim rows As DataRow() =
                mdtGreenAmptInfo.Select(String.Format("GridValue = {0}", intGridValue))
            Return CStr(rows(0)("GRMTextureE"))
        Else
            Return "[CONST]"
        End If
    End Function

    Public Shared Function GetSoilTextureCode(ByVal inName As String) As SoilTextureCode
        Select Case Trim(inName)
            Case SoilTextureCode.C.ToString : Return SoilTextureCode.C
            Case SoilTextureCode.CL.ToString : Return SoilTextureCode.CL
            Case SoilTextureCode.L.ToString : Return SoilTextureCode.L
            Case SoilTextureCode.LS.ToString : Return SoilTextureCode.LS
            Case SoilTextureCode.S.ToString : Return SoilTextureCode.S
            Case SoilTextureCode.SC.ToString : Return SoilTextureCode.SC
            Case SoilTextureCode.SCL.ToString : Return SoilTextureCode.SCL
            Case SoilTextureCode.SiC.ToString : Return SoilTextureCode.SiC
            Case SoilTextureCode.SiCL.ToString : Return SoilTextureCode.SiCL
            Case SoilTextureCode.SiL.ToString : Return SoilTextureCode.SiL
            Case SoilTextureCode.SL.ToString : Return SoilTextureCode.SL
            Case SoilTextureCode.NULL.ToString : Return SoilTextureCode.NULL
            Case Else : Return Nothing
        End Select
    End Function

End Class
