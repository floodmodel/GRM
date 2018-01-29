Imports System.Xml

''' <summary>
''' 매개변수 값의 범위 설정
''' </summary>
''' <remarks></remarks>
''' 
Public Class cParametersRange
    'Implements ioProjectFile

    ''' <summary>
    ''' GRM staticDB에서 매개변수를 검색하고, 모형에서 활용하기 위한 key 텍스트
    ''' </summary>
    ''' <remarks></remarks>
    Public Enum Name
        BankSideSlope
        CalibrationCoeffLandCoverRoughnessC
        CalibrationCoeffSoilDepth
        CalibrationCoeffSoilHydraulicC
        CalibrationCoeffSoilPorosity
        CalibrationCoeffSoilWettingFrontSH
        ChannelRoughnessCoeff
        ChannelWidth
        ChannelWidthEquationParameterC
        ChannelWidthEquationParameterD
        ChannelWidthEquationParameterE
        ComputationalTimeStep
        Discharge
        EfficitivePorosity
        FlowDepth
        HydraulicConductivity
        ImperviousRatio
        LandCoverRoughnessCoeff
        Porosity
        RainfallDuration
        ReservoirStorage
        ReservoirStorageRatio
        SimulationDuration
        Slope
        SoilDepth
        SoilSaturation
        StreamOrder
        TimeInterval
        WettingFrontSuctionHead
    End Enum


    Private Shared mMinValue As New SortedList(Of String, Decimal)
    Private Shared mMaxValue As New SortedList(Of String, Decimal)
    Private Shared mIncludeMinValue As New SortedList(Of String, Boolean)
    Private Shared mIncludeMaxValue As New SortedList(Of String, Boolean)
    Private Shared mdtParRange As GRMStaticDB.ParametersRangeDataTable


    Public Sub GetValues(fpnStaticXmlDB As String)
        Dim db As New GRMStaticDB
        db.ReadXml(fpnStaticXmlDB)
        mdtParRange = db.ParametersRange
    End Sub
    Public Sub SaveDB(grmStaticDB As GRMStaticDB)
        Console.WriteLine("여기서 뭔가 해야함")
        'mdtParRange.WriteXml(prjDBxmlFpn)
        'Throw New NotImplementedException()
    End Sub

    Public ReadOnly Property Min(ByVal ParName As Name) As Decimal
        Get
            Dim rows As DataRow() = mdtParRange.Select(String.Format("ParName = '{0}'", ParName.ToString))
            Return CDec(rows(0)("MinValue"))
        End Get
    End Property


    Public ReadOnly Property Max(ByVal ParName As Name) As Decimal
        Get
            Dim rows As DataRow() =
                    mdtParRange.Select(String.Format("ParName = '{0}'", ParName.ToString))
            Return CDec(rows(0)("MaxValue"))
        End Get
    End Property

    Public ReadOnly Property IncludeMin(ByVal ParName As Name) As Boolean
        Get
            Dim rows As DataRow() =
                    mdtParRange.Select(String.Format("ParName = '{0}'", ParName.ToString))
            Return CBool(rows(0)("IncludeMinValue"))
        End Get
    End Property

    Public ReadOnly Property IncludeMax(ByVal ParName As Name) As Boolean
        Get
            Dim rows As DataRow() =
                    mdtParRange.Select(String.Format("ParName = '{0}'", ParName.ToString))
            Return CBool(rows(0)("IncludeMaxValue"))
        End Get
    End Property


    Public Function GetValidator(ByVal tb As TextBox,
                                 ByVal RangeName As Name, ByVal UserMessage As String) As cTextBoxNumberValidator
        Return New cTextBoxNumberValidator(tb,
                        New cNumberBound(UserMessage, Min(RangeName), Max(RangeName), IncludeMin(RangeName),
                                                     IncludeMax(RangeName)))
    End Function


End Class
