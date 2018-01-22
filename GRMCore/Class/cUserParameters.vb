Public Class cUserParameters
    Public wsid As Integer
    Public iniSaturation As Single
    Public minSlopeOF As Single
    Public minSlopeChBed As Single
    Public minChBaseWidth As Single
    Public chRoughness As Single
    Public dryStreamOrder As Integer
    Public iniFlow As Nullable(Of Single)

    Public ccLCRoughness As Single
    Public ccPorosity As Single
    Public ccWFSuctionHead As Single
    Public ccHydraulicK As Single
    Public ccSoilDepth As Single
    Public expUnsaturatedK As Single

    Public isUserSet As Boolean
End Class
