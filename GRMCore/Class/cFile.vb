Imports System.Text
Imports System.Threading

Public Class cFile

    Public Enum FilePattern
        TEXTFILE
        TIFFILE
        ASCFILE
        ALLFILE
        OUT_Discharge
    End Enum

    Dim mSourceFileList As String()
    Private mNotDupFileList As List(Of String)


    Public Shared Function GetFileList(folderPath As String, filePattern As String) As List(Of String)
        Dim lstFLselected As New List(Of String)
        Dim lstFLallinSourcePath As String() = IO.Directory.GetFiles(folderPath)

        Select Case filePattern
            Case cFile.FilePattern.TEXTFILE.ToString
                For Each rfFN As String In lstFLallinSourcePath
                    If LCase(IO.Path.GetExtension(rfFN)) = ".txt" OrElse _
                        LCase(IO.Path.GetExtension(rfFN)) = ".mrf" Then
                        lstFLselected.Add(IO.Path.GetFileName(rfFN))
                    End If
                Next
            Case cFile.FilePattern.TIFFILE.ToString
                For Each rfFN As String In lstFLallinSourcePath
                    If LCase(IO.Path.GetExtension(rfFN)) = ".tif" Then
                        lstFLselected.Add(IO.Path.GetFileName(rfFN))
                    End If
                Next
            Case cFile.FilePattern.ASCFILE.ToString
                For Each rfFN As String In lstFLallinSourcePath
                    If LCase(IO.Path.GetExtension(rfFN)) = ".asc" Then
                        lstFLselected.Add(IO.Path.GetFileName(rfFN))
                    End If
                Next
            Case cFile.FilePattern.OUT_Discharge.ToString
                For Each rfFN As String In lstFLallinSourcePath
                    If LCase(rfFN).Contains("discharge.out") = True Then
                        lstFLselected.Add(IO.Path.GetFileName(rfFN))
                    End If
                Next
            Case cFile.FilePattern.ALLFILE.ToString
                For Each rfFN As String In lstFLallinSourcePath
                    lstFLselected.Add(IO.Path.GetFileName(rfFN))
                Next
            Case Else
                For Each rfFN As String In lstFLallinSourcePath
                    If LCase(rfFN).Contains(filePattern) = True Then
                        lstFLselected.Add(IO.Path.GetFileName(rfFN))
                    End If
                Next
        End Select
        Return lstFLselected
    End Function

    Public Shared Function GetFileListDataTable(folderPath As String, filePattern As cFile.FilePattern) As DataTable
        Dim fList As List(Of String) = GetFileList(folderPath, filePattern.ToString)
        Dim dt As New DataTable()
        dt.Columns.Add("FileName")
        For n As Integer = 0 To fList.Count - 1
            Dim nr As DataRow = dt.NewRow
            nr.Item(0) = fList(n).ToString
            dt.Rows.Add(nr)
        Next
        Return dt
    End Function


    Public Shared Function GetAbsoluteFullPath(strBaseFile As String, strRelativeFile As String) As String
        Dim StrAbsoluteFullPath As String = ""
        Try
            Dim intPos1 As Integer = 0
            Dim intPos2 As Integer = 0
            If (Microsoft.VisualBasic.Strings.Mid(strRelativeFile, 2, 2) = ":\") Then
                StrAbsoluteFullPath = strRelativeFile
            Else
                StrAbsoluteFullPath = System.IO.Path.GetDirectoryName(strBaseFile) + "\" + strRelativeFile
                intPos1 = Strings.InStr(1, StrAbsoluteFullPath, "\..\")
            End If
            Do While (intPos1 > 0)
                intPos2 = Strings.InStrRev(StrAbsoluteFullPath, "\", intPos1 - 1)
                Dim strPart1 As String = ""
                Dim strPart2 As String = ""
                strPart1 = Strings.Mid(StrAbsoluteFullPath, 1, intPos2)
                strPart2 = Strings.Mid(StrAbsoluteFullPath, intPos1 + 4)
                StrAbsoluteFullPath = strPart1 + strPart2
                intPos1 = Strings.InStr(StrAbsoluteFullPath, "\..\")
            Loop
            'If String.Compare(Path.Combine(strBaseFile, strRelativeFile), StrAbsoluteFullPath) <> 0 Then
            'End If
            Return StrAbsoluteFullPath
        Catch ex As Exception
            Throw ex
        End Try
    End Function

    Public Shared Function GetRelativeFilePath(basePath As String, PathToChange As String) As String
        Dim outPathName As String = ""
        Try
            If (LCase(Left(basePath, 2)) = LCase(Left(PathToChange, 2))) Then
                If LCase(IO.Path.GetDirectoryName(basePath)) = LCase(IO.Path.GetDirectoryName(PathToChange)) Then
                    outPathName = IO.Path.GetFileName(PathToChange)
                Else
                    Dim s1 As String = ""
                    Dim s2 As String = ""
                    Dim nmax As Integer = basePath.Length
                    If nmax < PathToChange.Length Then nmax = PathToChange.Length
                    For n As Integer = 1 To nmax
                        s1 = Mid(basePath, 1, n)
                        s2 = Mid(PathToChange, 1, n)
                        If LCase(s1) <> LCase(s2) Then
                            s1 = Mid(basePath, 1, n - 1)
                            s2 = Mid(PathToChange, 1, n - 1)
                            Exit For
                        End If
                        s1 = ""
                        s2 = ""
                    Next n
                    Dim len As Integer = s1.Length
                    Dim p As String
                    For n As Integer = len To 1 Step -1
                        p = Mid(s1, n, 1)
                        If p = "\" Then
                            s1 = Mid(s1, 1, n - 1)
                            Exit For
                        End If
                    Next n
                    outPathName = Replace(PathToChange, s1, "..")
                End If
            Else
                outPathName = PathToChange
            End If
            Return outPathName.Trim()
        Catch ex As Exception
            Throw ex
        End Try
    End Function

    Public Shared Function GetAbsolutePathOneUpper(PathToChange As String) As String
        Dim outPathName As String = ""
        Try

            Dim len As Integer = PathToChange.Length
            Dim p As String
            For n As Integer = len To 1 Step -1
                p = Mid(PathToChange, n, 1)
                If p = "\" Then
                    PathToChange = Mid(PathToChange, 1, n - 1)
                    Exit For
                End If
            Next n
            Return PathToChange.Trim()
        Catch ex As Exception
            Throw ex
        End Try
    End Function


    Public Shared Function ConfirmDeleteFiles(ByVal FilePathNames As List(Of String)) As Boolean
        Dim bAlldeleted As Boolean = False
        Dim n As Integer = 0
        Do Until bAlldeleted = True
            n += 1
            For Each fpn As String In FilePathNames
                If File.Exists(fpn) = True Then
                    File.Delete(fpn)
                    On Error Resume Next
                End If
            Next
            
            bAlldeleted = True 'Assumption
            For Each fpn As String In FilePathNames
                If File.Exists(fpn) Then
                    bAlldeleted = False
                    Exit For
                End If
            Next
            If n > 100 Then Return False
        Loop
        Return True
    End Function

    Public Shared Function ConfirmDeleteFiles(ByVal FilePathNames As String) As Boolean
        Dim bAlldeleted As Boolean = False
        Dim n As Integer = 0
        Do Until bAlldeleted = True
            n += 1
            If File.Exists(FilePathNames) Then
                File.Delete(FilePathNames)
                On Error Resume Next
            End If
            If File.Exists(FilePathNames) = False Then
                bAlldeleted = True
            Else
                bAlldeleted = False
            End If
            If n > 100 Then Return False
        Loop
        Return True
    End Function

    Public Shared Function ConfirmDeleteDirectory(ByVal DirectoryPath As List(Of String)) As Boolean
        Dim bAlldeleted As Boolean = False
        Do Until bAlldeleted = True
            For Each dp As String In DirectoryPath
                If Directory.Exists(dp) Then
                    Directory.Delete(dp, True)
                    On Error Resume Next
                End If
            Next
            For Each dp As String In DirectoryPath
                If Directory.Exists(dp) = False Then
                    bAlldeleted = True
                Else
                    bAlldeleted = False
                    Return False
                    Exit For
                End If
            Next
        Loop
        Return True
    End Function


    Public Shared Function ConfirmCreateDirectory(ByVal DirectoryPath As List(Of String)) As Boolean
        Dim bAllCreated As Boolean = False
        Do Until bAllCreated = True
            For Each dpn As String In DirectoryPath
                Directory.CreateDirectory(dpn)
                On Error Resume Next
            Next

            For Each dpn As String In DirectoryPath
                If Directory.Exists(dpn) = True Then
                    bAllCreated = True
                Else
                    bAllCreated = False
                    Return False
                    Exit For
                End If
            Next
        Loop
        Return True
    End Function

    Public Shared Function IsFileLocked(filePath As String, checkingDuration_sec As Integer) As Boolean
        Dim isLocked As Boolean = True
        Dim iter_sec As Integer = 0
        checkingDuration_sec = checkingDuration_sec * 10
        While isLocked = True AndAlso ((iter_sec < checkingDuration_sec) OrElse (checkingDuration_sec = 0))
            Try
                '주석처리
                'Using File.Open(filePath, FileMode.Open)
                'End Using
                Return False
            Catch e As IOException
                '이건 애러의 종류를 확인하는거 같은데... 잘 이해 안됨.. 주석 처리. 2015.04.22
                'Dim errorCode As Integer = System.Runtime.InteropServices.Marshal.GetHRForException(e) And ((1 << 16) - 1)
                'If errorCode = 32 OrElse errorCode = 33 Then
                '    isLocked = True
                'Else
                '    isLocked = False
                'End If
                iter_sec += 1
                If checkingDuration_sec <> 0 Then
                    Dim mre As New System.Threading.ManualResetEvent(False)
                    mre.WaitOne(100)
                End If
            End Try
        End While
        Return isLocked
    End Function


    Public Function GetFilesAndCheckDuplication(ByVal TargetListBox As ListBox, ByVal bSelected As Boolean, ByVal NowDirPath As String, _
                                                       ByVal dtTarget As DataTable, ByVal ColNameFileName As String, ByVal ColNameFilePath As String) As String()
        If bSelected Then
            mSourceFileList = New String(TargetListBox.SelectedItems.Count - 1) {}
            For i As Integer = 0 To mSourceFileList.Length - 1
                mSourceFileList(i) = CStr(TargetListBox.SelectedItems(i))
            Next
        Else
            mSourceFileList = New String(TargetListBox.Items.Count - 1) {}
            For i As Integer = 0 To mSourceFileList.Length - 1
                mSourceFileList(i) = CStr(TargetListBox.Items(i))
            Next
        End If

        mNotDupFileList = New List(Of String)

        For Each fileName As String In mSourceFileList
            Dim fullPath As String = Path.Combine(NowDirPath, fileName).ToLower
            Dim bFound As Boolean = False
            For Each row As DataRow In dtTarget.Rows
                Dim aPath As String = Path.Combine(row(ColNameFilePath).ToString, row(ColNameFileName).ToString).ToLower
                If fullPath = aPath Then
                    bFound = True
                    Exit For
                End If
            Next
            If Not bFound Then mNotDupFileList.Add(fileName)
        Next

        Return mNotDupFileList.ToArray
    End Function

    Public Sub AddToDataTable(ByVal items As String(), ByVal SourceFilePath As String, ByVal dtTarget As DataTable, _
                                     ByVal NameCol1 As String, ByVal NameCol2 As String, ByVal NameCol3 As String)
        Array.Sort(items)
        Dim intFileCount As Integer
        Dim intRecordCountOri As Integer
        intFileCount = items.Length
        intRecordCountOri = dtTarget.Rows.Count
        Dim ofrmPrograssBar As New fGRMToolsProgressBar
        ofrmPrograssBar.GRMToolsPrograssBar.Maximum = intFileCount
        ofrmPrograssBar.GRMToolsPrograssBar.Style = ProgressBarStyle.Blocks
        ofrmPrograssBar.labGRMToolsPrograssBar.Text = "Add 0/" & CStr(items.Length) & "files..."
        ofrmPrograssBar.Text = "Add files"
        ofrmPrograssBar.Show()
        System.Windows.Forms.Application.DoEvents()
        dtTarget.BeginLoadData()
        For i As Integer = 0 To items.Length - 1
            Dim strFNameOnly As String = Path.GetFileName(items(i))
            Dim nr As Data.DataRow = dtTarget.NewRow
            nr.Item(NameCol1) = intRecordCountOri + i + 1
            nr.Item(NameCol2) = strFNameOnly
            nr.Item(NameCol3) = SourceFilePath
            dtTarget.Rows.Add(nr)
            ofrmPrograssBar.GRMToolsPrograssBar.Value = i + 1
            ofrmPrograssBar.labGRMToolsPrograssBar.Text = "Add " + CStr(i + 1) + "/" & CStr(intFileCount) & " files..."
            System.Windows.Forms.Application.DoEvents()
        Next i
        dtTarget.EndLoadData()
        ofrmPrograssBar.Close()
    End Sub

    ''' <summary>
    ''' 기존 파일 목록에서 중복되는 것이 있으면, 이를 제외하고 추가할 것인지 확인. 모든 파일이 중복이면 추가하지 않고 중단
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property DuplicationfileProcessMessage() As MsgBoxResult
        Get
            If mNotDupFileList.Count = 0 Then
                MsgBox("All of the selected files are already added.", MsgBoxStyle.Exclamation, _
                        cGRM.BuildInfo.ProductName)
                Return MsgBoxResult.Cancel
            ElseIf mNotDupFileList.Count <> mSourceFileList.Length Then
                Dim mbr As MsgBoxResult = MsgBox( _
                    "Some of the selected files are already added." & vbCrLf & _
                    "Do you want to add not duplicated files(Skips duplicated files)?", _
                    MsgBoxStyle.OkCancel, cGRM.BuildInfo.ProductName)
                Return mbr
            End If

        End Get
    End Property

    Public Shared Sub RefreshOrderInDataTable(ByVal dt As DataTable, ByVal OrderColumeName As String)
        For i As Integer = 0 To dt.Rows.Count - 1
            dt.Rows(i)(OrderColumeName) = i + 1
        Next
    End Sub
End Class
