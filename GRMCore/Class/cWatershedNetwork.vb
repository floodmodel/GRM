Public Class cWatershedNetwork
    Private mWSidList As List(Of Integer)
    Private mWSIDsNearbyUp As New SortedList(Of Integer, List(Of Integer))
    Private mWSIDsNearbyDown As New SortedList(Of Integer, List(Of Integer))
    Private mWSIDsAllUps As New SortedList(Of Integer, List(Of Integer))
    Private mWSIDsAllDowns As New SortedList(Of Integer, List(Of Integer))
    Private mMostDownstreamWSIDs As List(Of Integer)

    Private mWSoutletCVids As New Dictionary(Of Integer, Integer)

    ''' <summary>
    ''' 모든 유역에 대해서 상하류 셀 리스트를 0으로 초기화
    ''' </summary>
    ''' <param name="WSList"></param>
    ''' <remarks></remarks>
    Public Sub New(ByVal WSList As List(Of Integer))
        mWSidList = WSList
        mWSIDsNearbyUp.Clear()
        mWSIDsNearbyDown.Clear()
        mWSoutletCVids.Clear()
        For Each i As Integer In mWSidList
            mWSIDsNearbyUp.Add(i, New List(Of Integer))
            mWSIDsNearbyDown.Add(i, New List(Of Integer))
            mWSoutletCVids.Add(i, -1)
        Next
        mMostDownstreamWSIDs = New List(Of Integer)
    End Sub

    Public Sub UpdateAllDownsAndUpsNetwork()
        mWSIDsAllUps.Clear()
        mWSIDsAllDowns.Clear()
        For Each i As Integer In mWSidList
            mWSIDsAllUps.Add(i, New List(Of Integer))
            mWSIDsAllDowns.Add(i, New List(Of Integer))
            For Each id As Integer In mWSIDsNearbyUp(i)
                mWSIDsAllUps(i).Add(id)
            Next
            For Each id As Integer In mWSIDsNearbyDown(i)
                mWSIDsAllDowns(i).Add(id)
            Next
        Next

        For Each nowID As Integer In mWSidList
            Dim upIDs As New List(Of Integer)
            Dim downIDs As New List(Of Integer)
            upIDs = mWSIDsAllUps(nowID)
            downIDs = mWSIDsAllDowns(nowID)
            For Each upID As Integer In upIDs
                For Each downID As Integer In downIDs
                    If Not mWSIDsAllUps(downID).Contains(upID) Then
                        mWSIDsAllUps(downID).Add(upID)
                    End If
                    If Not mWSIDsAllDowns(upID).Contains(downID) Then
                        mWSIDsAllDowns(upID).Add(downID)
                    End If
                Next
            Next
        Next
        For Each nowID As Integer In mWSidList
            If mWSIDsNearbyDown(nowID).Count = 0 Then
                mMostDownstreamWSIDs.Add(nowID)
                'mMostDownstreamWSIDs = nowID
                'Exit For
            End If
        Next
    End Sub

    Public Sub SetWSoutletCVID(ByVal wsid As Integer, ByVal cvid As Integer)
        mWSoutletCVids(wsid) = cvid
    End Sub

    Public Sub AddWSIDup(ByVal NowWSID As Integer, ByVal WSIDup As Integer)
        mWSIDsNearbyUp(NowWSID).Add(WSIDup)
    End Sub

    Public Sub AddWSIDdown(ByVal NowWSID As Integer, ByVal WSIDdown As Integer)
        mWSIDsNearbyDown(NowWSID).Add(WSIDdown)
    End Sub

    Public Sub ClearUpstreamWSIDs()
        mWSIDsNearbyUp.Clear()
    End Sub

    Public Sub ClearDownstreamWSIDs()
        mWSIDsNearbyDown.Clear()
    End Sub

    Public ReadOnly Property WSoutletCVID(ByVal wsid As Integer) As Integer
        Get
            Return mWSoutletCVids(wsid)
        End Get
    End Property

    Public ReadOnly Property WSIDsAllDowns(ByVal NowWSID As Integer) As List(Of Integer)
        Get
            mWSIDsAllDowns(NowWSID).Sort()
            Return mWSIDsAllDowns(NowWSID)
        End Get
    End Property

    Public ReadOnly Property WSIDsAllUps(ByVal NowWSID As Integer) As List(Of Integer)
        Get
            mWSIDsAllUps(NowWSID).Sort()
            Return mWSIDsAllUps(NowWSID)
        End Get
    End Property

    Public ReadOnly Property MostDownstreamWSIDs() As List(Of Integer)
        Get
            Return mMostDownstreamWSIDs
        End Get
    End Property

    Public ReadOnly Property WSIDsNearbyDown(ByVal NowWSID As Integer) As Integer
        Get
            If mWSIDsNearbyDown(NowWSID).Count > 0 Then
                Return mWSIDsNearbyDown(NowWSID).Item(0)
            Else
                Return -1
            End If
        End Get
    End Property

    Public ReadOnly Property WSIDsNearbyUp(ByVal NowWSID As Integer) As List(Of Integer)
        Get
            mWSIDsNearbyUp(NowWSID).Sort()
            Return mWSIDsNearbyUp(NowWSID)
        End Get
    End Property

    Public ReadOnly Property WSIDsAll As List(Of Integer)
        Get
            Return mWSidList
        End Get
    End Property

End Class
