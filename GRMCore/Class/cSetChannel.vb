
Public Class cSetChannel
    Public Shared ReadOnly DEFAULT_USER_CHANNEL_WIDTH_COLOR As Color = Color.Cyan
    Public mCrossSection As cSetCrossSection = Nothing
    Public mRightBankSlope As Nullable(Of Single)
    Public mLeftBankSlope As Nullable(Of Single)
    Public mCrossSectionType As cSetCrossSection.CSTypeEnum
    Public mCellColor As Color

    Public Sub New()
        mRightBankSlope = Nothing
        mLeftBankSlope = Nothing
        mCrossSection = New cSetCSSingle
    End Sub

    Public Sub GetValues(ByVal prjDB As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        If Not row.IsCrossSectionTypeNull Then
            With row
                mRightBankSlope = .BankSideSlopeRight
                mLeftBankSlope = .BankSideSlopeLeft
                If .CrossSectionType.ToString = cSetCrossSection.CSTypeEnum.CSCompound.ToString Then
                    mCrossSection = New cSetCSCompound
                Else
                    mCrossSection = New cSetCSSingle
                End If
                mCrossSection.GetValues(prjDB)
            End With
        End If
    End Sub

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return mRightBankSlope.HasValue
        End Get
    End Property

    Public Sub SetValues(ByVal prjDB As GRMProject)
        If IsSet Then
            Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
            With row
                .BankSideSlopeRight = mRightBankSlope.Value
                .BankSideSlopeLeft = mLeftBankSlope.Value
            End With
            mCrossSection.SetValues(prjDB)
        End If
    End Sub

    Public ReadOnly Property CellColor() As Color
        Get
            Return mCellColor
        End Get
    End Property

End Class
