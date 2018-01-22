Imports GRMCore

Public Class cSetTSData
    'Implements ioProjectFile



    'Public mdtTSSummary As GRMProject.TimeSeriesDataSummaryDataTable

    'Public ReadOnly Property IsSet As Boolean Implements ioProjectFile.IsSet
    '    Get
    '        Throw New NotImplementedException()
    '    End Get
    'End Property

    Public Structure TimeSeriesInfoInTSDB
        Public TSID As Long
        Public StationName As String
        Public HydroCode As String
        Public MissingCount As Integer
    End Structure


    'Public Sub SetValues(prjDB As GRMProject) Implements ioProjectFile.SetValues
    '    mdtTSSummary.AcceptChanges()
    '    For Each row As DataRow In mdtTSSummary.Rows
    '        row.SetAdded()
    '    Next
    '    Throw New NotImplementedException()
    'End Sub

    'Public Sub GetValues(prjDB As GRMProject) Implements ioProjectFile.GetValues
    '    mdtTSSummary = New GRMProject.TimeSeriesDataSummaryDataTable
    '    mdtTSSummary = prjDB.TimeSeriesDataSummary
    '    Throw New NotImplementedException()
    'End Sub
End Class
