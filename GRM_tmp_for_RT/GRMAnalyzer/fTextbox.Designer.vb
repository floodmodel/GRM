<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class fTextBox
    Inherits System.Windows.Forms.Form

    'Form은 Dispose를 재정의하여 구성 요소 목록을 정리합니다.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Windows Form 디자이너에 필요합니다.
    Private components As System.ComponentModel.IContainer

    '참고: 다음 프로시저는 Windows Form 디자이너에 필요합니다.
    '수정하려면 Windows Form 디자이너를 사용하십시오.  
    '코드 편집기에서는 수정하지 마세요.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.txtTextBox = New System.Windows.Forms.TextBox()
        Me.SuspendLayout()
        '
        'txtTextBox
        '
        Me.txtTextBox.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.txtTextBox.Location = New System.Drawing.Point(12, 12)
        Me.txtTextBox.Multiline = True
        Me.txtTextBox.Name = "txtTextBox"
        Me.txtTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both
        Me.txtTextBox.Size = New System.Drawing.Size(708, 196)
        Me.txtTextBox.TabIndex = 1
        '
        'fTextbox
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(7.0!, 12.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(732, 220)
        Me.Controls.Add(Me.txtTextBox)
        Me.Name = "fTextbox"
        Me.Text = "fTextbox"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

    Friend WithEvents txtTextBox As TextBox
End Class
