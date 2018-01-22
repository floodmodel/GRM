Imports System.Runtime.Serialization.Formatters.Binary
Public Class cProjectBAK
    Implements ICloneable
    Public mCVs As List(Of cCVAttribute)
    Public mWatchPoint As cSetWatchPoint
    Public mFCGrid As cFlowControl

    Public Sub New()
        mCVs = New List(Of cCVAttribute)
        mWatchPoint = New cSetWatchPoint
        mFCGrid = New cFlowControl
    End Sub

    Public Sub SetCloneUsingCurrentProject(project As cProject) 'As cProjectBAK
        mCVs = project.CVs
        mWatchPoint = project.WatchPoint
        mFCGrid = project.FCGrid
        Me.Clone()
    End Sub

    Public ReadOnly Property CVs As List(Of cCVAttribute)
        Get
            Return mCVs
        End Get
    End Property

    Public ReadOnly Property WatchPoint As cSetWatchPoint
        Get
            Return mWatchPoint
        End Get
    End Property

    Public ReadOnly Property FCGrid As cFlowControl
        Get
            Return mFCGrid
        End Get
    End Property

    Public ReadOnly Property CV(index As Integer) As cCVAttribute
        Get
            Return mCVs(index)
        End Get
    End Property

    Public Function Clone() As Object Implements ICloneable.Clone
        Dim clo As New cProjectBAK
        For Each cv As cCVAttribute In Me.mCVs
            clo.mCVs.Add(CType(cv.Clone, cCVAttribute))
        Next
        clo.mWatchPoint = CType(Me.WatchPoint.Clone, cSetWatchPoint)
        clo.mFCGrid = CType(Me.FCGrid.Clone, cFlowControl)
        mCVs = clo.mCVs
        mWatchPoint = clo.mWatchPoint
        mFCGrid = clo.mFCGrid
        Return clo
    End Function

End Class
