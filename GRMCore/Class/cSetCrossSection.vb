Imports GRM

Public MustInherit Class cSetCrossSection
    'Implements ioProjectFile ', ioProjectDB

    Public Enum CSTypeEnum
        CSSingle
        CSCompound
    End Enum

    Public MustOverride ReadOnly Property CSType() As CSTypeEnum
    Public MustOverride Sub GetValues(ByVal prjds As GRMProject) 'Implements ioProjectFile.GetValues
    Public MustOverride ReadOnly Property IsSet() As Boolean 'Implements ioProjectFile.IsSet
    Public MustOverride Sub SetValues(ByVal prjds As GRMProject) 'Implements ioProjectFile.SetValues

End Class
