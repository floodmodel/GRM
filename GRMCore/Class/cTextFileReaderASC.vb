Imports System.IO
Public Class cTextFileReaderASC
    Private mLines() As String
    Private mRowCountAll As Integer
    Private mDataStartLineInASCfile As Integer
    Private mDataValueOri As Single
    Private mHeaderNcols As Integer
    Private mHeaderNrows As Integer
    Private mHeaderXllcorner As Double
    Private mHeaderYllcorner As Double
    Private mHeaderCellsize As Single
    Private mHeaderNODATA_value As String
    Private mSeparator As String() = {" ", vbTab, ","}

    Public Sub New(ByVal FPN As String)
        'todo : 여기서 한줄만 읽으면 더 좋은거 아닌가?
        mLines = System.IO.File.ReadAllLines(FPN, System.Text.Encoding.Default)
        mRowCountAll = mLines.Length

        If mRowCountAll < GetDataStartLineInASCfile Then
            MsgBox(FPN + " has no data valuable. ", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
            Exit Sub
        End If
        Call GetHeaderInfo(GetDataStartLineInASCfile() - 1)

        mDataStartLineInASCfile = GetDataStartLineInASCfile()
    End Sub

    Private Sub GetHeaderInfo(ByVal HeaderLineCount As Integer)
        For n As Integer = 0 To HeaderLineCount - 1
            Dim line As String = mLines(n)
            Dim LineParts() As String = line.Split(mSeparator, StringSplitOptions.RemoveEmptyEntries)
            Select Case n
                Case 0
                    Dim rc As Integer
                    If Integer.TryParse(LineParts(1), rc) Then
                        mHeaderNcols = rc
                    Else
                        mHeaderNcols = -1
                    End If
                Case 1
                    Dim rc As Integer
                    If Integer.TryParse(LineParts(1), rc) Then
                        mHeaderNrows = rc
                    Else
                        mHeaderNrows = -1
                    End If
                Case 2
                    Dim rc As Double
                    If Double.TryParse(LineParts(1), rc) Then
                        mHeaderXllcorner = rc
                    Else
                        mHeaderXllcorner = -1
                    End If
                Case 3
                    Dim rc As Double
                    If Double.TryParse(LineParts(1), rc) Then
                        mHeaderYllcorner = rc
                    Else
                        mHeaderYllcorner = -1
                    End If
                Case 4
                    Dim rc As Integer
                    If Integer.TryParse(LineParts(1), rc) Then
                        mHeaderCellsize = rc
                    Else
                        mHeaderCellsize = -1
                    End If
                Case 5
                    If LineParts(1) = "" Then
                        mHeaderNODATA_value = "-9999"
                    Else
                        mHeaderNODATA_value = LineParts(1)
                    End If
            End Select
        Next
    End Sub

    Public ReadOnly Property RowCountAll() As Integer
        Get
            Return mRowCountAll
        End Get
    End Property

    Public ReadOnly Property HeaderNrows() As Integer
        Get
            Return mHeaderNrows
        End Get
    End Property

    Public ReadOnly Property HeaderNcols() As Integer
        Get
            Return mHeaderNcols
        End Get
    End Property

    Public ReadOnly Property HeaderXllcorner() As Double
        Get
            Return mHeaderXllcorner
        End Get
    End Property


    Public ReadOnly Property HeaderYllcorner() As Double
        Get
            Return mHeaderYllcorner
        End Get
    End Property

    Public ReadOnly Property HeaderCellsize() As Single
        Get
            Return mHeaderCellsize
        End Get
    End Property

    Public ReadOnly Property HeaderNODATA_value() As String
        Get
            If mHeaderNODATA_value Is Nothing Then
                Return "-9999"
            Else
                Return mHeaderNODATA_value
            End If

        End Get
    End Property

    Public ReadOnly Property ValueAtrowNcolPositionFromLowLeft(ByVal xcol As Integer, ByVal yrow As Integer) As Single
        Get
            Dim row As Integer = mRowCountAll - yrow - 1
            Dim LVals() As String = mLines(row).Split(mSeparator, StringSplitOptions.RemoveEmptyEntries)
            Dim result As Single
            If Single.TryParse(LVals(xcol), result) Then
                Return result
            Else
                Return -9999
            End If
        End Get
    End Property

    Public ReadOnly Property ValueAtrowNcolPositionFromLowLeftAsNotNegative(ByVal xcol As Integer, ByVal yrow As Integer) As Single
        Get
            mDataValueOri = ValueAtrowNcolPositionFromLowLeft(xcol, yrow)
            If mDataValueOri < 0 Then
                Return 0
            Else
                Return mDataValueOri
            End If
        End Get
    End Property


    Private Function GetDataStartLineInASCfile() As Integer
        For ln As Integer = 0 To mLines.Length - 1
            Dim aline As String = mLines(ln)
            Dim LineParts() As String = aline.Split(mSeparator, StringSplitOptions.RemoveEmptyEntries)
            Dim Val As Single
            If LineParts.Length > 0 AndAlso Single.TryParse(LineParts(0), Val) = True Then
                Return ln + 1
            End If
        Next
        Return -1
    End Function

    Public ReadOnly Property DataStartLineInASCfile() As Integer
        Get
            Return mDataStartLineInASCfile
        End Get
    End Property

    Public ReadOnly Property ValuesInOneRowFromLowLeft(ByVal yrow As Integer) As String()
        Get
            Dim row As Integer = mRowCountAll - yrow - 1
            Return mLines(row).Split(New String() {" "}, StringSplitOptions.RemoveEmptyEntries)
        End Get
    End Property


    Public ReadOnly Property ValuesInOneRowFromTopLeft(ByVal yrow As Integer) As String()
        Get
            Dim row As Integer = GetDataStartLineInASCfile - 1 + yrow - 1
            Return mLines(row).Split(mSeparator, StringSplitOptions.RemoveEmptyEntries)
        End Get
    End Property

    Public ReadOnly Property ValueAtColumeXFormOneRow(ByVal xcol As Integer, ByVal Values As String()) As Single
        Get
            Dim result As Single
            If Single.TryParse(Values(xcol), result) Then
                Return result
            Else
                Return -9999
            End If
        End Get
    End Property

    Public ReadOnly Property ValueAtColumeXFormOneRowAsNotNegative(ByVal xcol As Integer, ByVal Values As String()) As Single
        Get
            mDataValueOri = ValueAtColumeXFormOneRow(xcol, Values)
            If mDataValueOri < 0 Then
                Return 0
            Else
                Return mDataValueOri
            End If
        End Get
    End Property

    Public Shared Function CompareFiles(ByVal FileToReference As cTextFileReaderASC, ByVal FileToCompare As cTextFileReaderASC) As Boolean
        If FileToReference.HeaderNcols <> FileToCompare.HeaderNcols Then Return False
        If FileToReference.HeaderNrows <> FileToCompare.HeaderNrows Then Return False
        If FileToReference.HeaderCellsize <> FileToCompare.HeaderCellsize Then Return False
        Return True
    End Function

End Class