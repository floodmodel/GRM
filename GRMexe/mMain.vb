Imports System.IO
Imports System.Threading
Imports GRMCore


''' <summary>
''' 이건 console 실행 코드
''' </summary>
''' <remarks></remarks>
Module mMain
    Private WithEvents mSimulator As cSimulator
    Private mMessage As String = ""
    Private mSimDurationHour As Integer
    Private mGrmAnalyzer As cGRMAnalyzer
    Private mbCreateDistributionFiles As Boolean = False

    Sub main()
        Try
            Dim prjFPN As String = ""
            Select Case My.Application.CommandLineArgs.Count
                Case 1
                    Dim arg0 As String = Trim(My.Application.CommandLineArgs(0).ToString)
                    Select Case arg0
                        Case ""
                            Console.WriteLine("GRM project file was not selected!!!")
                            waitUserKey()
                            Exit Select
                        Case "/?"
                            Call GRMconsoleHelp()
                            Exit Select
                        Case Else
                            If File.Exists(arg0) Then
                                prjFPN = arg0
                                StartSingleRun(prjFPN, False)
                                Exit Select
                            Else
                                Console.WriteLine("Current project file is inavlid!!!")
                                waitUserKey()
                                Exit Select
                            End If
                    End Select
                Case 2
                    Dim arg0 As String = Trim(My.Application.CommandLineArgs(0).ToString)
                    Dim arg1 As String = Trim(My.Application.CommandLineArgs(1).ToString)
                    If LCase(arg0) = "/f" OrElse LCase(arg0) = "/fd" Then
                        If Directory.Exists(arg1) Then
                            Dim gmpFiles() As String = GetGMPFileList(arg1)
                            If gmpFiles.Length = 0 Then
                                Console.WriteLine("There is no GRM project file in this directory.")
                                waitUserKey()
                                Exit Select
                            End If
                            Dim fpnBAT As String = ""
                            Dim bDelete As Boolean = False
                            Select Case LCase(arg0)
                                Case "/f"
                                    bDelete = False
                                Case "/fd"
                                    bDelete = True
                            End Select
                            For n As Integer = 0 To gmpFiles.Length - 1
                                mMessage = String.Format("Total progress: {0}/{1} ({2}%). ",
                                                                 (n + 1), gmpFiles.Length, Format(((n + 1) / gmpFiles.Length * 100), "#0.00"))
                                StartSingleRun(gmpFiles(n), bDelete)
                                If ((n + 1) Mod 100) = 0 Then Console.Clear()
                            Next
                            Exit Select
                        Else
                            Console.WriteLine("Project folder is invalid!!")
                            waitUserKey()
                            Exit Select
                        End If
                    ElseIf File.Exists(arg0) = True Then
                        StartSingleRun(arg0, False)
                        Exit Select
                    End If
                Case Else
                    Console.WriteLine("Invalid command. Use /? argument for help!!!")
                    waitUserKey()
                    Exit Select
            End Select
        Catch ex As Exception
            waitUserKey()
        End Try
    End Sub

    Private Sub waitUserKey()
        Console.Write("Press any key to continue..")
        Console.ReadKey()
    End Sub


    Private Sub StartSingleRun(ByVal currentPrjFPN As String, Optional bDeleteFilesExceptQ As Boolean = False)

        Dim wpNames As New List(Of String)
        If Path.GetDirectoryName(currentPrjFPN) = "" Then
            currentPrjFPN = Path.Combine(My.Application.Info.DirectoryPath, currentPrjFPN)
        End If
        Try
            cProject.OpenProject(currentPrjFPN, False)
            cProject.ValidateProjectFile(cProject.Current)
            mSimDurationHour = CInt(cProject.Current.GeneralSimulEnv.mSimDurationHOUR)
            If cProject.Current.SetupModelParametersAfterProjectFileWasOpened() = False Then
                cGRM.writelogAndConsole("GRM setup was failed !!!", True, True)
                Exit Sub
            End If
            ''여기서 셀 정보를 미리 알 수 있다.
            'Dim wsinfo As New cGetWatershedInfo(currentPrjFPN)
            ''혹은 아래의 방법
            'Dim WSFPN As String = cProject.Current.Watershed.mFPN_watershed
            'Dim SlopeFPN As String = cProject.Current.Watershed.mFPN_slope
            'Dim FdirFPN As String = cProject.Current.Watershed.mFPN_fdir
            'Dim FacFPN As String = cProject.Current.Watershed.mFPN_fac
            'Dim streamFPN As String = cProject.Current.Watershed.mFPN_stream
            'Dim lcFPN As String = cProject.Current.Landcover.mGridLandCoverFPN
            'Dim stFPN As String = cProject.Current.GreenAmpt.mGridSoilTextureFPN
            'Dim sdFPN As String = cProject.Current.SoilDepth.mGridSoilDepthFPN
            'Dim wsinfo As New cGetWatershedInfo(WSFPN, SlopeFPN, FdirFPN, FacFPN, streamFPN, lcFPN, stFPN, sdFPN,,)
            'wsinfo.SetOneSWSParametersAndUpdateAllSWSUsingNetwork(1, 1, 0.0001, 0.0001, 30, 0.045, 0, 1, 1, 1, 1, 1, True, 30)
            'Dim x As Integer = wsinfo.mostDownStreamCellArrayXColPosition
            'Dim y As Integer = wsinfo.mostDownStreamCellArrayYRowPosition
            'Dim isIn As Boolean = wsinfo.IsInWatershedArea(x, y)
            'Dim array() As String = wsinfo.allCellsInUpstreamArea(x, y)
            'Dim fd As String = wsinfo.flowDirection(x, y)
            'Dim lcv As Integer = wsinfo.landCoverValue(x, y)
            'Dim wsCount As Integer = wsinfo.WScount
            'Dim intStreamValue As Integer = wsinfo.streamValue(x, y)

            If cProject.Current.GeneralSimulEnv.mbCreateASCFile = True OrElse
                cProject.Current.GeneralSimulEnv.mbCreateImageFile = True Then
                mbCreateDistributionFiles = True
                mGrmAnalyzer = New cGRMAnalyzer(cProject.Current)
            End If
            cGRM.writelogAndConsole(currentPrjFPN + " -> Model setup completed.", cGRM.bwriteLog, True)
            For Each row As GRMProject.WatchPointsRow In cProject.Current.WatchPoint.mdtWatchPointInfo
                wpNames.Add(row.Name)
            Next
            mSimulator = New cSimulator
            mSimulator.SimulateSingleEvent(cProject.Current)
            '삭제 대상
            'IO.File.AppendAllText(cThisSimulation.tmp_InfoFile, String.Format("{0}", currentPrjFPN + vbTab))
            'IO.File.AppendAllText(cThisSimulation.tmp_InfoFile, String.Format("{0}{1}{2}{3}{4}{5}", cThisSimulation.MaxDegreeOfParallelism, vbTab, cThisSimulation.tmp_24H_RunTime, vbTab, cThisSimulation.tmp_48H_RunTime, vbCrLf))
            ''''''''''''''''''''''''''''
            'cProject.Current.SaveProject()
        Catch ex As Exception
            Console.WriteLine(ex.ToString)
            cGRM.writelogAndConsole(String.Format("Starting GRM project ({0}) is failed !!!", currentPrjFPN), True, True)
            Throw ex
        End Try
        Try
            If bDeleteFilesExceptQ = True Then
                Dim prjPathOnly As String = Path.GetDirectoryName(currentPrjFPN)
                Dim prjNameOnlyWithoutExtsion As String = Path.GetFileNameWithoutExtension(currentPrjFPN)
                Dim fNameTmp As String = ""
                File.Delete(currentPrjFPN)
                fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "Depth.out")
                File.Delete(fNameTmp)
                fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "RFGrid.out")
                File.Delete(fNameTmp)
                fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "RFUpMean.out")
                File.Delete(fNameTmp)
                For Each wpname As String In wpNames
                    fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "WP_" + wpname + ".out")
                    File.Delete(fNameTmp)
                Next
                File.Delete(currentPrjFPN)
            End If
            cProject.Current.Dispose()
            GC.Collect()
        Catch ex As Exception
            Console.WriteLine(String.Format("ERROR : [{0}] project could not be deleted!!!", currentPrjFPN))
        End Try
    End Sub

    Private Function GetGMPFileList(path As String) As String()
        Dim fileList() As String
        fileList = Directory.GetFiles(path, "*.gmp")
        Dim bN As Boolean = True
        For Each fs As String In fileList
            Dim n As Integer
            If Integer.TryParse(IO.Path.GetFileNameWithoutExtension(fs).Split(CChar("_")).First, n) = False Then
                bN = False
                Exit For
            End If
        Next
        If bN = True Then '모든 파일명의 머리글을 숫자로 변환할 수 있으면..  숫자 순으로 정렬
            fileList = fileList.OrderBy(Function(fn) Int32.Parse(IO.Path.GetFileNameWithoutExtension(fn).Split(CChar("_")).First)).ToArray
        End If
        Return fileList
    End Function

    Private Sub mSimulator_SimulationStep(sender As cSimulator, elapsedMinutes As Integer) Handles mSimulator.SimulationStep
        Dim nowStep As Single
        Dim currentProgress As String = ""
        nowStep = CSng((elapsedMinutes / (mSimDurationHour * 60)) * 100)
        currentProgress = Format(nowStep, "#0")
        Console.Write(vbCr + "Current progress: " + currentProgress + "%. " + mMessage)
    End Sub

    Private Sub mSimulator_SimulationComplete(sender As cSimulator) Handles mSimulator.SimulationComplete
        Console.WriteLine("Simulation completed!!")
    End Sub

    Private Sub mSimulator_SimulationRaiseError(sender As cSimulator, simulError As cSimulator.SimulationErrors, erroData As Object) Handles mSimulator.SimulationRaiseError
        Console.WriteLine("Simulation error!!!")
        Console.WriteLine(simulError.ToString)
        Console.WriteLine(erroData.ToString)
    End Sub

    Private Sub mSimulator_CallAnalyzer(sender As cSimulator, project As cProject,
                                        nowTtoPrint_MIN As Integer,
                                        createImgFile As Boolean,
                                        createAscFile As Boolean) Handles mSimulator.CallAnalyzer
        If mbCreateDistributionFiles = True Then
            mGrmAnalyzer.CreateDistributionFiles(nowTtoPrint_MIN, mGrmAnalyzer.ImgWidth, mGrmAnalyzer.ImgHeight)
        End If
    End Sub

    Private Sub GRMconsoleHelp()
        Console.WriteLine()
        Console.WriteLine("Usage : GRM [Current project file full path and name to simulate]")
        Console.WriteLine()
        Console.WriteLine("**사용법")
        Console.WriteLine("1. GRM 모형의 project 파일(.gmp)과 입력자료(지형공간자료, 강우, flow control 시계열 자료)를 준비한다.")
        Console.WriteLine("2. 모델링 대상 프로젝트 이름을 스위치(argument)로 넣고 실행한다.")
        Console.WriteLine("4. Console에서 grm.exe [argument] 로 시행한다.")
        Console.WriteLine("5. ** 주의사항 : 장기간 모의할 경우, 컴퓨터 업데이트로 인해, 종료될 수 있으니, 네트워크 차단, 자동업데이트 하지 않음으로 설정한다.")
        Console.WriteLine("6. [argument]")
        Console.WriteLine("- /?")
        Console.WriteLine("       도움말")
        Console.WriteLine("- 프로젝트 파일경로와 이름")
        Console.WriteLine("        grm을 파일단위로 실행시킨다.")
        Console.WriteLine("        이때 full path, name을 넣어야 하지만, ")
        Console.WriteLine("        대상 프로젝트 파일이 grm 실행파일과 동일한 폴더에 있을 경우에는, path는 입력하지 않아도 된다.")
        Console.WriteLine("        또한 대상 프로젝트 이름과 경로에 공백이 포함될 경우 큰따옴표로 묶어서 입력한다.")
        Console.WriteLine("         ** 예문(grm.exe가 d:\GRMrun에 있을 경우)")
        Console.WriteLine("             - grm.exe와 다른 폴더에 대상 파일들이 있을 경우")
        Console.WriteLine("               d:\GRMrun>grm D:\GRMTest\TestProject\test.gmp")
        Console.WriteLine("             - grm.exe와 같은 폴더에 대상 파일들이 있을 경우")
        Console.WriteLine("               d:\GRMrun>grm test.gmp")
        Console.WriteLine("- /f 폴더경로")
        Console.WriteLine("         grm을 폴더 단위로 실행시킨다.")
        Console.WriteLine("          ** 예문 : grmmp /f d:\GRMrun\TestProject")
        Console.WriteLine("- /fd 폴더경로")
        Console.WriteLine("         grm을 폴더 단위로 실행시킨다.")
        Console.WriteLine("         유량 모의결과인 *discharge.out을 제외한 파일을 지운다(*.gmp, *Depth.out, 등등...)")
        Console.WriteLine("          ** 예문 : grmmp /fd d:\GRMrun\TestProject")
    End Sub





End Module

