Imports System.Runtime.Serialization.Formatters.Binary

Public Class cSetWatchPoint
    Implements ICloneable
    Public Shared ReadOnly DEFAULT_WATCH_POINT_COLOR As Color = Color.Fuchsia
    Public mdtWatchPointInfo As GRMProject.WatchPointsDataTable
    Public mCellColor As Color
    Private mWatchPointCVidList As List(Of Integer)

    ''' <summary>
    ''' 현재 watch point 상류에 대해 원시자료에서 읽은 강우량.[m/s] 
    ''' 향후, 차단 고려시, 차단된 강우량은 제외한 땅에 떨어진 강우량
    ''' </summary>
    ''' <remarks></remarks>
    Public mRFReadIntensitySumUpWs_mPs As SortedList(Of Integer, Double)

    ''' <summary>
    ''' 현재 watch point 상류에 대해 dt(계산시간간격)
    ''' 시간동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
    ''' </summary>
    ''' <remarks></remarks>
    Public mRFUpWsMeanForDt_mm As SortedList(Of Integer, Double)

    ''' <summary>
    ''' 현재 watch point 상류의 출력시간간격
    ''' 동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
    ''' </summary>
    ''' <remarks></remarks>
    Public mRFUpWsMeanForDtPrintout_mm As SortedList(Of Integer, Double)

    ''' <summary>
    ''' 현재 watch point 상류의 평균강우량의 누적값[mm]
    ''' </summary>
    ''' <remarks></remarks>
    Public mRFUpWsMeanTotal_mm As SortedList(Of Integer, Double)

    ''' <summary>
    ''' Watchpoint 격자에 대한 출력시간간격
    ''' 동안의 누적강우량. 원시자료를 이용해서 계산된값.[mm]
    ''' </summary>
    ''' <remarks></remarks>
    Public mRFWPGridForDtPrintout_mm As SortedList(Of Integer, Double)

    ''' <summary>
    ''' Watchpoint 격자에 대한 누적강우량[mm]
    ''' </summary>
    ''' <remarks></remarks>
    Public mRFWPGridTotal_mm As SortedList(Of Integer, Double)

    ''' <summary>
    ''' Watchpoint 격자에 대한 전체유량[cms]
    ''' </summary>
    ''' <remarks></remarks>
    Public mTotalFlow_cms As SortedList(Of Integer, Double)


    ''' <summary>
    ''' Watchpoint 격자에 총유출고[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public mTotalDepth_m As SortedList(Of Integer, Double)

    ''' <summary>
    ''' Watchpoint 격자에 대한 최대유량[cms]
    ''' </summary>
    ''' <remarks></remarks>
    Public mMaxFlow_cms As SortedList(Of Integer, Double)

    ''' <summary>
    ''' Watchpoint 격자에 대한 최고수심[m]
    ''' </summary>
    ''' <remarks></remarks>
    Public mMaxDepth_m As SortedList(Of Integer, Double)

    ''' <summary>
    ''' Watchpoint 격자에 대한 최대유량 시간. 첨두시간.
    ''' </summary>
    ''' <remarks></remarks>
    Public mMaxFlowTime As SortedList(Of Integer, String)

    ''' <summary>
    ''' Watchpoint 격자에 대한 최고수심 시간. 첨두시간
    ''' </summary>
    ''' <remarks></remarks>
    Public mMaxDepthTime As SortedList(Of Integer, String)

    ''' <summary>
    ''' 해당 wp에서 Flow control에 의해서 계산되는 유량
    ''' </summary>
    ''' <remarks></remarks>
    Public mQfromFCDataCMS As SortedList(Of Integer, Double)

    ''' <summary>
    ''' Watch point 별 모의결과 출력을 위한 파일 이름 저장
    ''' </summary>
    ''' <remarks></remarks>
    Public mFpnWpOut As SortedList(Of Integer, String)


    Sub New()
        mRFReadIntensitySumUpWs_mPs = New SortedList(Of Integer, Double)
        mRFUpWsMeanForDt_mm = New SortedList(Of Integer, Double)
        mRFUpWsMeanForDtPrintout_mm = New SortedList(Of Integer, Double)
        mRFUpWsMeanTotal_mm = New SortedList(Of Integer, Double)
        mRFWPGridForDtPrintout_mm = New SortedList(Of Integer, Double)
        mRFWPGridTotal_mm = New SortedList(Of Integer, Double)
        mTotalFlow_cms = New SortedList(Of Integer, Double)
        mTotalDepth_m = New SortedList(Of Integer, Double)
        mMaxFlow_cms = New SortedList(Of Integer, Double)
        mMaxDepth_m = New SortedList(Of Integer, Double)
        mMaxFlowTime = New SortedList(Of Integer, String)
        mMaxDepthTime = New SortedList(Of Integer, String)
        mQfromFCDataCMS = New SortedList(Of Integer, Double)
        mFpnWpOut = New SortedList(Of Integer, String)
        mWatchPointCVidList = New List(Of Integer)
    End Sub

    ''' <summary>
    ''' Watchpoint 별 강우량 값 입력을 위한 리스트 변수의 key(CVid)를 업데이트 하고
    ''' Watchpoint CVid 리스트를 설정한다.
    ''' </summary>
    ''' <remarks></remarks>
    Public Function UpdatesWatchPointCVIDs(prj As cProject) As Boolean

        mRFReadIntensitySumUpWs_mPs.Clear()
        mRFUpWsMeanForDt_mm.Clear()
        mRFUpWsMeanForDtPrintout_mm.Clear()
        mRFUpWsMeanTotal_mm.Clear()
        mRFWPGridForDtPrintout_mm.Clear()
        mRFWPGridTotal_mm.Clear()
        mTotalFlow_cms.Clear()
        mTotalDepth_m.Clear()
        mMaxFlow_cms.Clear()
        mMaxDepth_m.Clear()
        mMaxFlowTime.Clear()
        mMaxDepthTime.Clear()
        mQfromFCDataCMS.Clear()
        mFpnWpOut.Clear()
        mWatchPointCVidList.Clear()
        For Each row As GRMProject.WatchPointsRow In mdtWatchPointInfo
            row.CVID = prj.WSCell(row.ColX, row.RowY).CVID
            Dim cvid As Integer = row.CVID
            If mWatchPointCVidList.Contains(cvid) Then
                cGRM.writelogAndConsole("ERROR : Two or more watch points were set at the same cell !!!", True, True)
                Return False
            Else
                mWatchPointCVidList.Add(cvid)
            End If
            mRFReadIntensitySumUpWs_mPs.Add(cvid, Nothing)
            mRFUpWsMeanForDt_mm.Add(cvid, Nothing)
            mRFUpWsMeanForDtPrintout_mm.Add(cvid, Nothing)
            mRFUpWsMeanTotal_mm.Add(cvid, Nothing)
            mRFWPGridForDtPrintout_mm.Add(cvid, Nothing)
            mRFWPGridTotal_mm.Add(cvid, Nothing)
            mTotalFlow_cms.Add(cvid, Nothing)
            mTotalDepth_m.Add(cvid, Nothing)
            mMaxFlow_cms.Add(cvid, Nothing)
            mMaxDepth_m.Add(cvid, Nothing)
            mMaxFlowTime.Add(cvid, Nothing)
            mMaxDepthTime.Add(cvid, Nothing)
            mQfromFCDataCMS.Add(cvid, Nothing)
            mFpnWpOut.Add(cvid, Nothing)
        Next
        Return True
    End Function

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return mdtWatchPointInfo IsNot Nothing
        End Get
    End Property

    Public Sub GetValues(ByVal prjdb As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        Dim cc As New ColorConverter
        If prjdb.WatchPoints.Rows.Count > 0 Then
            mdtWatchPointInfo = New GRMProject.WatchPointsDataTable
            mdtWatchPointInfo = prjdb.WatchPoints
        End If
    End Sub

    Public Sub SetValues(ByVal prjds As GRMProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prjds.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        If IsSet Then
            mdtWatchPointInfo.AcceptChanges()
            If mdtWatchPointInfo.Rows.Count > 0 Then
                For Each r As GRMProject.WatchPointsRow In mdtWatchPointInfo.Rows
                    r.SetAdded()
                Next
            End If
        End If
    End Sub


    Public ReadOnly Property wpName(cvid As Integer) As String
        Get
            For Each row As GRMProject.WatchPointsRow In mdtWatchPointInfo.Rows
                If row.CVID = cvid Then
                    Return Replace(row.Name, ",", "_")
                End If
            Next
            Return "-9999"
        End Get
    End Property


    Public ReadOnly Property WPCount() As Integer
        Get
            If mdtWatchPointInfo Is Nothing Then
                Return 0
            Else
                Return mdtWatchPointInfo.Rows.Count
            End If
        End Get
    End Property

    Public ReadOnly Property CellColor() As Color
        Get
            Return mCellColor
        End Get
    End Property

    Public ReadOnly Property WPCVidList() As List(Of Integer)
        Get
            Return mWatchPointCVidList
        End Get
    End Property

    Public Function Clone() As Object Implements ICloneable.Clone
        Dim cln As New cSetWatchPoint
        If Me.mRFUpWsMeanForDtPrintout_mm IsNot Nothing Then
            For Each k As Integer In mWatchPointCVidList
                If Me.mRFReadIntensitySumUpWs_mPs IsNot Nothing AndAlso
                    mRFReadIntensitySumUpWs_mPs.Keys.Contains(k) Then _
                cln.mRFReadIntensitySumUpWs_mPs.Add(k, mRFReadIntensitySumUpWs_mPs(k))
                If Me.mRFUpWsMeanForDt_mm IsNot Nothing AndAlso
                    mRFUpWsMeanForDt_mm.Keys.Contains(k) Then _
                cln.mRFUpWsMeanForDt_mm.Add(k, mRFUpWsMeanForDt_mm(k))
                If Me.mRFUpWsMeanForDtPrintout_mm IsNot Nothing AndAlso
                    mRFUpWsMeanForDtPrintout_mm.Keys.Contains(k) Then _
                cln.mRFUpWsMeanForDtPrintout_mm.Add(k, mRFUpWsMeanForDtPrintout_mm(k))
                If Me.mRFUpWsMeanTotal_mm IsNot Nothing AndAlso
                    mRFUpWsMeanTotal_mm.Keys.Contains(k) Then _
                cln.mRFUpWsMeanTotal_mm.Add(k, mRFUpWsMeanTotal_mm(k))
                If Me.mRFWPGridForDtPrintout_mm IsNot Nothing AndAlso
                    mRFWPGridForDtPrintout_mm.Keys.Contains(k) Then _
                cln.mRFWPGridForDtPrintout_mm.Add(k, mRFWPGridForDtPrintout_mm(k))
                If Me.mRFWPGridTotal_mm IsNot Nothing AndAlso
                    mRFWPGridTotal_mm.Keys.Contains(k) Then _
                cln.mRFWPGridTotal_mm.Add(k, mRFWPGridTotal_mm(k))
                If Me.mTotalFlow_cms IsNot Nothing AndAlso
                    mTotalFlow_cms.Keys.Contains(k) Then _
                cln.mTotalFlow_cms.Add(k, mTotalFlow_cms(k))
                If Me.mTotalDepth_m IsNot Nothing AndAlso
                    mTotalDepth_m.Keys.Contains(k) Then _
                cln.mTotalDepth_m.Add(k, mTotalDepth_m(k))
                If Me.mMaxFlow_cms IsNot Nothing AndAlso
                    mMaxFlow_cms.Keys.Contains(k) Then _
                cln.mMaxFlow_cms.Add(k, mMaxFlow_cms(k))
                If Me.mMaxDepth_m IsNot Nothing AndAlso
                    mMaxDepth_m.Keys.Contains(k) Then _
                cln.mMaxDepth_m.Add(k, mMaxDepth_m(k))
                If Me.mMaxFlowTime IsNot Nothing AndAlso
                    mMaxFlowTime.Keys.Contains(k) Then _
                cln.mMaxFlowTime.Add(k, mMaxFlowTime(k))
                If Me.mMaxDepthTime IsNot Nothing AndAlso
                    mMaxDepthTime.Keys.Contains(k) Then _
                cln.mMaxDepthTime.Add(k, mMaxDepthTime(k))
                If Me.mQfromFCDataCMS IsNot Nothing AndAlso
                    mQfromFCDataCMS.Keys.Contains(k) Then _
                cln.mQfromFCDataCMS.Add(k, mQfromFCDataCMS(k))
                If Me.mFpnWpOut IsNot Nothing AndAlso
                    mFpnWpOut.Keys.Contains(k) Then _
                cln.mFpnWpOut.Add(k, mFpnWpOut(k))
                cln.mWatchPointCVidList.Add(k)
            Next
        End If
        Return cln
    End Function
End Class
