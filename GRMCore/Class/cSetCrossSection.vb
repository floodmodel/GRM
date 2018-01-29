
Public MustInherit Class cSetCrossSection

    Public Enum CSTypeEnum
        CSSingle
        CSCompound
    End Enum

    Public MustOverride ReadOnly Property CSType() As CSTypeEnum
    Public MustOverride Sub GetValues(ByVal prjds As GRMProject)
    Public MustOverride ReadOnly Property IsSet() As Boolean
    Public MustOverride Sub SetValues(ByVal prjds As GRMProject)

End Class
