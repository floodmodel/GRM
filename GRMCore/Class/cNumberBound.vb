Public Class cNumberBound
    Private mValueName As String
    Private mMin As Decimal
    Private mMax As Decimal
    Private mIncludeMinvalue As Boolean
    Private mIncludeMaxValue As Boolean

    Public Sub New(ByVal valueName As String, ByVal min As Decimal, ByVal max As Decimal, Optional ByVal IncludeMinvalue As Boolean = True, Optional ByVal IncludeMaxValue As Boolean = True)
        mValueName = valueName
        SetBound(min, max)
        mIncludeMinvalue = IncludeMinvalue
        mIncludeMaxValue = IncludeMaxValue
    End Sub

    Public Property ValueName() As String
        Get
            Return mValueName
        End Get
        Set(ByVal value As String)
            mValueName = value
        End Set
    End Property

    Public ReadOnly Property Min() As Decimal
        Get
            Return mMin
        End Get
    End Property

    Public ReadOnly Property Max() As Decimal
        Get
            Return mMax
        End Get
    End Property

    Public Sub SetBound(ByVal min As Decimal, ByVal max As Decimal)
        mMin = min
        mMax = max
        If mMin > mMax Then
            mMax = min
            mMin = max
        End If
    End Sub

    Public Property IncludeMinvalue() As Boolean
        Get
            Return mIncludeMinvalue
        End Get
        Set(ByVal value As Boolean)
            mIncludeMinvalue = value
        End Set
    End Property

    Public Property IncludeMaxValue() As Boolean
        Get
            Return mIncludeMaxValue
        End Get
        Set(ByVal value As Boolean)
            mIncludeMaxValue = value
        End Set
    End Property

    Public Function Validate(ByVal strValue As String) As Boolean
        Dim value As Decimal
        If Not Decimal.TryParse(strValue, value) Then
            Return False
        End If
        Return Validate(value)
    End Function

    Public Function Validate(ByVal value As Decimal) As Boolean
        If mIncludeMinvalue Then
            If value < mMin Then Return False
        Else
            If value <= mMin Then Return False
        End If

        If mIncludeMaxValue Then
            If value > mMax Then Return False
        Else
            If value >= mMax Then Return False
        End If

        Return True
    End Function

    Public ReadOnly Property ErrorMsg() As String
        Get
            Dim minSymbol As String
            Dim maxSymbol As String

            If IncludeMinvalue Then
                minSymbol = "≤"
            Else
                minSymbol = "<"
            End If

            If IncludeMaxValue Then
                maxSymbol = "≤"
            Else
                maxSymbol = "<"
            End If

            Return String.Format("[{0}] is invalid. {3}  {1}{4}{0}{5}{2}", _
                                 ValueName, Min, Max, _
                                 vbCrLf, minSymbol, maxSymbol)
        End Get
    End Property

End Class
