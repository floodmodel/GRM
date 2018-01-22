Imports GRMCore

Public Class cSetPDistribution
    'Implements ioProjectFile

    'Public Enum PType
    '    SSR
    '    ChannelFlow
    'End Enum
    'Public mFPN_SSR As String
    'Public mSSRdist As Dictionary(Of Integer, Single)
    ''Private mSSR_mean As Single = 0

    'Public mFPN_ChFlow As String
    ''Public mChFlowDist As Dictionary(Of Integer, Single)



    'Public Function GetEstimatedParametersDistribution(ByVal pType As PType, ByVal cvid As Integer) As Single
    '    Select Case pType
    '        Case PType.SSR
    '            Return mSSRdist(cvid)
    '        Case PType.ChannelFlow
    '            Return mChFlowDist(cvid)
    '        Case Else
    '            Return -9999
    '    End Select
    'End Function

    'Public Function SetSSRDwihtASC(ByVal project As cProject, ByVal fpn As String) As Boolean
    '    mSSRdist = New Dictionary(Of Integer, Single)

    '    Dim ascReader As New cTextFileReaderASC(fpn)
    '    Dim n As Integer = 0
    '    Dim sum As Single = 0
    '    With project
    '        For intR As Integer = 0 To .Watershed.mRowCount - 1
    '            Dim RFs As String() = ascReader.ValuesInOneRowFromTopLeft(intR)
    '            For intC As Integer = 0 To .Watershed.mColCount - 1
    '                If .WSCell(intC, intR) Is Nothing Then Continue For
    '                Dim v As Single = ascReader.ValueAtColumeXFormOneRowAsNotNegative(intC, RFs)
    '                Dim cvid As Integer = .WSCell(intC, intR).CVID
    '                If Not mSSRdist.Keys.Contains(cvid) Then
    '                    mSSRdist.Add(cvid, v)
    '                    n += 1
    '                    sum = sum + v
    '                End If
    '            Next
    '        Next
    '    End With
    '    mSSR_mean = sum / n
    'End Function

    'Public Function SetChFlowDwihtASC(ByVal project As cProject, ByVal fpn As String) As Boolean
    '    mChFlowDist = New Dictionary(Of Integer, Single)

    '    Dim ascReader As New cTextFileReaderASC(fpn)
    '    With project
    '        For intR As Integer = 0 To .Watershed.mRowCount - 1
    '            Dim RFs As String() = ascReader.ValuesInOneRowFromTopLeft(intR)
    '            For intC As Integer = 0 To .Watershed.mColCount - 1
    '                If .WSCell(intC, intR) Is Nothing Then Continue For
    '                Dim v As Single = ascReader.ValueAtColumeXFormOneRowAsNotNegative(intC, RFs)
    '                Dim cvid As Integer = .WSCell(intC, intR).CVID
    '                If Not mChFlowDist.Keys.Contains(cvid) Then mChFlowDist.Add(cvid, v)
    '            Next
    '        Next
    '    End With
    'End Function

    'Public Sub SetValues(prjds As GRMProject) 'Implements ioProjectFile.SetValues
    '    Dim row As GRMProject.ProjectSettingsRow = CType(prjds.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
    '    row.InitialSoilSaturationRatioFile = mFPN_SSR
    '    row.InitialChannelFlowFile = mFPN_ChFlow
    'End Sub

    'Public Sub GetValues(prjds As GRMProject) 'Implements ioProjectFile.GetValues
    '    Dim row As GRMProject.ProjectSettingsRow = CType(prjds.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
    '    If row.IsInitialSoilSaturationRatioFileNull = False Then
    '        mFPN_SSR = Trim(row.InitialSoilSaturationRatioFile.ToString)
    '    End If
    '    If row.IsInitialChannelFlowFileNull = False Then
    '        mFPN_ChFlow = Trim(row.InitialChannelFlowFile.ToString)
    '    End If
    'End Sub

    'Public ReadOnly Property SSRDidst() As Dictionary(Of Integer, Single)
    '    Get
    '        If mSSRdist IsNot Nothing Then
    '            Return mSSRdist
    '        Else
    '            Return Nothing
    '        End If
    '    End Get
    'End Property


    ''Public ReadOnly Property SSR_mean() As Single
    ''    Get
    ''        Return mSSR_mean
    ''    End Get
    ''End Property

    'Public ReadOnly Property IsSet As Boolean ' Implements ioProjectFile.IsSet
    '    Get
    '        Throw New NotImplementedException()
    '    End Get
    'End Property
End Class
