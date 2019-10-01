Public Class fTextBox

    Private Sub txtTextBox_KeyPress(sender As Object, e As KeyPressEventArgs) Handles txtTextBox.KeyPress
        If e.KeyChar = Convert.ToChar(1) Then
            DirectCast(sender, TextBox).SelectAll()
            e.Handled = True
        End If
    End Sub
End Class