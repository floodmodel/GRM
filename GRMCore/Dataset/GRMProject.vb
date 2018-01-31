

Partial Public Class GRMProject
    Partial Public Class ProjectSettingsDataTable
        Private Sub ProjectSettingsDataTable_ColumnChanging(sender As Object, e As DataColumnChangeEventArgs) Handles Me.ColumnChanging
            If (e.Column.ColumnName = Me.ConstantRoughnessCoeffColumn.ColumnName) Then
                '사용자 코드를 여기에 추가하십시오.
            End If

        End Sub

    End Class
End Class

Namespace GRMProjectTableAdapters

    Partial Public Class GreenAmptParameter_TableAdapter
    End Class
End Namespace
