Public Class cTextBoxNumberValidator
    Private mBox As TextBox
    Private mBound As cNumberBound

    Public Sub New(ByVal box As TextBox, ByVal bound As cNumberBound)
        mBox = box
        mBound = bound
    End Sub

    Public Function Validate() As Boolean
        Dim value As Decimal
        If String.IsNullOrEmpty(mBox.Text.Trim) OrElse _
            Not Decimal.TryParse(mBox.Text.Trim, value) Then
            Return False
        End If

        Return mBound.Validate(value)
    End Function

    Public ReadOnly Property ErrorMsg() As String
        Get
            Return mBound.ErrorMsg
        End Get
    End Property

    Public ReadOnly Property TextBox() As TextBox
        Get
            Return mBox
        End Get
    End Property
    Public ReadOnly Property bound() As cNumberBound
        Get
            Return mBound
        End Get
    End Property

End Class
