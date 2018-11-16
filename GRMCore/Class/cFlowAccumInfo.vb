''' <summary>
''' 흐름누적수별 인접상류셀 개수별 CVID 목록
''' </summary>
''' <remarks></remarks>
Public Class cFlowAccumInfo
    Private mDic As New Dictionary(Of Integer, List(Of Integer))

    ''' <summary>
    ''' 현재 흐름누적수에 해당하는 CV 목록에 자신의 CVAN을 추가함
    ''' </summary>
    ''' <param name="accum"></param>
    ''' <param name="cvan"></param>
    ''' <remarks></remarks>
    Public Sub Add(ByVal accum As Integer, ByVal cvan As Integer)
        Dim key As Integer = accum
        If Not mDic.ContainsKey(key) Then mDic.Add(key, New List(Of Integer))
        mDic(key).Add(cvan)
    End Sub



    ''' <summary>
    ''' 현재의 흐름누적수 값을 가지는 cv의 배열 번호들을 반환함
    ''' </summary>
    ''' <param name="accum"></param>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Default Public ReadOnly Property CellList(ByVal accum As Integer) As Array
        Get
            Dim key As Integer = accum
            If mDic.ContainsKey(key) Then
                Return mDic(key).ToArray
            Else
                Return Nothing
            End If
        End Get
    End Property

    Public Sub Clear()
        mDic.Clear()
    End Sub


    '''' <summary>
    '''' 현재 흐름누적수와 상류의 셀 개수에 해당하는 목록에 자신의 CVID를 추가함
    '''' </summary>
    '''' <param name="accum"></param>
    '''' <param name="cvid"></param>
    '''' <remarks></remarks>
    'Public Sub Add(ByVal accum As Integer, ByVal upSideCellCount As Integer, ByVal cvid As Integer)
    '    Dim key As String = MakeKey(accum)
    '    If Not mDic.ContainsKey(key) Then mDic.Add(key, New List(Of Integer))
    '    mDic(key).Add(cvid)
    'End Sub

    '''' <summary>
    '''' 현재의 흐름누적수와 상류의 셀 개수에 해당하는 CVID 목록을 반환함
    '''' </summary>
    '''' <param name="accum"></param>
    '''' <param name="upSideCellCount"></param>
    '''' <value></value>
    '''' <returns></returns>
    '''' <remarks></remarks>
    'Default Public ReadOnly Property CellList(ByVal accum As Integer, ByVal upSideCellCount As Integer) As List(Of Integer)
    '    Get
    '        Dim key As String = MakeKey(accum, upSideCellCount)
    '        If mDic.ContainsKey(key) Then
    '            Return mDic(key)
    '        Else
    '            Return Nothing
    '        End If
    '    End Get
    'End Property

    '''' <summary>
    '''' 흐름누적수와 상류셀 개수를 이용해서 Dictionary 키를 만듬
    '''' </summary>
    '''' <param name="accum"></param>
    '''' <param name="upSideCellCount"></param>
    '''' <returns></returns>
    '''' <remarks></remarks>
    'Public Function MakeKey(ByVal accum As Integer, ByVal upSideCellCount As Integer) As String
    '    Return String.Format("{0}_{1}", accum, upSideCellCount)
    'End Function


End Class
