Imports GRM

Public Class cSetChannel
    'Implements ioProjectFile ', ioProjectDB

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

    Public Sub GetValues(ByVal prjDB As GRMProject) 'Implements ioProjectFile.GetValues
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

                'Dim cc As New ColorConverter
                'mCellColor = CType(cc.ConvertFromString(.UserChannelWidthCellColor), Color)
            End With
        End If
    End Sub

    Public ReadOnly Property IsSet() As Boolean 'Implements ioProjectFile.IsSet
        Get
            Return mRightBankSlope.HasValue
        End Get
    End Property

    Public Sub SetValues(ByVal prjDB As GRMProject) 'Implements 'ioProjectFile.SetValues
        If IsSet Then
            Dim row As GRMProject.ProjectSettingsRow = CType(prjDB.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
            With row
                .BankSideSlopeRight = mRightBankSlope.Value
                .BankSideSlopeLeft = mLeftBankSlope.Value
                '.UserChannelWidthCellColor = mCellColor.A.ToString & "," _
                '           & mCellColor.R.ToString & "," _
                '           & mCellColor.G.ToString & "," _
                '           & mCellColor.B.ToString
            End With
            mCrossSection.SetValues(prjDB)
        End If
    End Sub

    Public ReadOnly Property CellColor() As Color
        Get
            Return mCellColor
        End Get
    End Property

    '''' <summary>
    '''' 현재의 channel CV의 제방 계수. 계산 편의를 위해서 channel CV 별로 미리계산한 값
    '''' </summary>
    '''' <remarks></remarks>
    'Public ReadOnly Property BankCoeff() As Single
    '    Get
    '        Return (1 / mLeftBankSlope + 1 / mRightBankSlope).Value
    '    End Get
    'End Property


End Class
