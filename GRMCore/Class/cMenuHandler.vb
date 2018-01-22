Public Class cMenuHandler

    Private Sub New()

    End Sub

    Public Shared Sub OnNewProject()

        If cMap.mwAppMain Is Nothing Then
            MsgBox("Workspace is not exist!!!     ", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
            Exit Sub
        End If

        ' 기존 프로젝트의 저장
        If cProject.Current IsNot Nothing AndAlso cProject.Current.mbThisProjectIsSaved = False Then
            Dim mbr As MsgBoxResult = MsgBox( _
                "The current project is not saved. Do you want to save?", _
                MsgBoxStyle.YesNoCancel, cGRM.BuildInfo.ProductName)
            Select Case mbr
                Case MsgBoxResult.Yes
                    cProject.Current.SaveProject()
                Case MsgBoxResult.Cancel
                    Exit Sub
            End Select
        End If

        Dim dlgSaveFileProject As New SaveFileDialog
        With dlgSaveFileProject
            .Title = "Make new project file"
            .Filter = "GMP files (*.gmp)|*.gmp"
            .FileName = ""
        End With

        If dlgSaveFileProject.ShowDialog() = DialogResult.OK Then
            Try
                cProject.NewProject(dlgSaveFileProject.FileName, cMap.mwAppMain)
                cGRM.Start()
                InitMenuOnNewProject()
            Catch ex As Exception
                MsgBox("Current map is empty or not saved.  ", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                Exit Sub
            End Try
        End If
    End Sub

    Private Shared Sub InitMenuOnNewProject()
        cGRM.mEnableSave = True
        cGRM.mEnableSaveAs = True
        cGRM.mEnableProjectInfo = True
        cGRM.mEnableSetWatershed = True
        cGRM.mEnableSetLCST = False
        cGRM.mEnableRainfall = False
        cGRM.mEnableSetNRunGRM = False
        cGRM.mEnableGRMME = False
    End Sub

    Public Shared Sub OnOpenProject()
        Try
            ' 기존 프로젝트의 저장
            If cProject.Current IsNot Nothing AndAlso cProject.Current.mbThisProjectIsSaved = False Then
                Dim mbr As MsgBoxResult = MsgBox( _
                    "The current project is not saved. Do you want to save?", _
                    MsgBoxStyle.YesNoCancel, cGRM.BuildInfo.ProductName)
                Select Case mbr
                    Case MsgBoxResult.Yes
                        cProject.Current.SaveProject()
                    Case MsgBoxResult.Cancel
                        Exit Sub
                End Select
            End If

            Dim dlg As New OpenFileDialog
            With dlg
                .Title = "Open project"
                .Filter = "GRM Project files (*.gmp)|*.gmp|GRM Project xml files (*.xml)|*.xml|All files (*.*)|*.*"
                .FileName = ""
                .CheckFileExists = True
                .CheckPathExists = True
                .Multiselect = False
                If .ShowDialog() <> DialogResult.OK Then Exit Sub
            End With
            If cProject.OpenProject(dlg.FileName, False) = True Then
                cGRM.Start()
                UpdateMenuStatus()
                MsgBox(cProject.Current.ProjectPathName & " is opened.  ", MsgBoxStyle.Information, cGRM.BuildInfo.ProductName)
            End If
        Catch ex As Exception

        End Try
    End Sub

    Public Shared Sub OnSaveAsProject()
        Try
            If cProject.Current Is Nothing Then
                MsgBox("The current project is not exist.", , cGRM.BuildInfo.ProductName)
                Exit Sub
            Else
                Dim dlg As New SaveFileDialog
                With dlg
                    .Title = "Save as project"
                    .Filter = "GMP files (*.gmp)|*.gmp"
                    .FileName = ""
                    If .ShowDialog() = DialogResult.OK Then
                        cProject.SaveAsProject(dlg.FileName, cMap.mwAppMain)
                    End If
                End With
            End If
        Catch ex As Exception

        End Try
    End Sub

    Public Shared Sub UpdateMenuStatus()
        cGRM.mEnableSave = cProject.Current IsNot Nothing
        cGRM.mEnableSaveAs = cProject.Current IsNot Nothing
        cGRM.mEnableProjectInfo = cProject.Current IsNot Nothing
        cGRM.mEnableSetWatershed = cProject.Current IsNot Nothing
        cGRM.mEnableSetLCST = cProject.Current IsNot Nothing AndAlso cProject.Current.Watershed.IsSet
        cGRM.mEnableRainfall = cProject.Current IsNot Nothing AndAlso cProject.Current.Landcover.IsSet
        cGRM.mEnableSetNRunGRM = cProject.Current IsNot Nothing AndAlso cProject.Current.Rainfall.IsSet
        If cGRM.mEnableSetNRunGRM = True Then
            cGRM.mEnableGRMME = True
        Else
            cGRM.mEnableGRMME = False
        End If


    End Sub

End Class
