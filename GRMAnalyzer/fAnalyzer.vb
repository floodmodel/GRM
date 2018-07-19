Imports System.Windows.Forms.DataVisualization.Charting
Imports System.Math
Imports System.IO
Imports System.Drawing
Imports System.Drawing.Imaging
Imports System.Threading

Public Class fAnalyzer
    Private mPrjFPN As String
    Private mproject As cProject
    Private mStartNewSimulation As Boolean = True
    Private WithEvents mSimulator As cSimulator
    Private mbCreateDistributionFiles As Boolean = False
    Private mRasterOutput As cRasterOutput
    Private mCurrentOrder As Integer
    Private mObsDataCount As Integer
    Private mbShowSimData As Boolean = False
    Private mbShowObsData As Boolean = True
    Private mSimDuration_HR As Integer
    Public mImgFPN_dist_SSR As List(Of String)
    Public mImgFPN_dist_RF As List(Of String)
    Public mImgFPN_dist_RFAcc As List(Of String)
    Public mImgFPN_dist_Flow As List(Of String)
    Public mMaxPrintoutCount As Integer = 0
    Public mImg As New cImg(cImg.RendererType.Risk)
    Public mChart As cChart
    Private mdtData As DataTable

    Private mEventType As Nullable(Of TrackingEvent)

    Public Enum TrackingEvent
        TrackingBarChanged
        DataGridViewChanged
    End Enum

    Private mAninmationRepeat As Boolean = False

    Private Sub fAnalyzer_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        'Me.tbGMPfpn.Text = "D:\Github\TestSet_GRM\AnalyzerTest\SampleProject.gmp"
        'Me.tbFPNObsData.Text = "D:\Github\TestSet_GRM\AnalyzerTest\Qobs - 복사본.txt"
        cThisSimulation.mAnalyzerSet = False
        Me.tbChart.Width = 470
    End Sub

    Private Sub btStartGRMorApplySettings_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btStartGRMorApplySettings.Click
        Try
            If InitializeAndCheckErr() = False Then
                Exit Sub
            End If
            setupProject()
            SetupChart()
            SetDistributedPictureFilesAndRenderer(Me)
            If mStartNewSimulation = True Then
                Me.btStopSimulation.Enabled = True
                Me.btStartGRMorApplySettings.Enabled = False
                Me.btClose.Enabled = False
                startSingleEvent()
            End If
        Catch ex As Exception
            MsgBox("err", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
        End Try

    End Sub


    Private Function InitializeAndCheckErr() As Boolean
        mPrjFPN = Me.tbGMPfpn.Text.Trim
        If File.Exists(mPrjFPN) = False Then
            MsgBox("GMP  file is invalid!", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
            Return False
        End If
        If Trim(Me.tbFPNObsData.Text) = "" Then
            mbShowObsData = False
        ElseIf File.Exists(Trim(Me.tbFPNObsData.Text)) = False Then
            MsgBox("Observed file is invalid!", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
            Return False
        Else
            mbShowObsData = True
        End If

        If Me.chkLoadSimData.Checked = False OrElse Trim(Me.tbFPNSimData.Text) = "" Then
            mbShowSimData = False
            mStartNewSimulation = True
        Else
            If Me.chkLoadSimData.Checked = True AndAlso File.Exists(Trim(Me.tbFPNSimData.Text)) = True Then
                mbShowSimData = True
                mStartNewSimulation = False
            Else
                MsgBox("Simulated file is invalid!", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                mbShowSimData = False
                Return False
            End If
            mStartNewSimulation = False
        End If
        Return True
    End Function

    Private Sub setupProject()
        Try
            cProject.OpenProject(mPrjFPN, False)
            mproject = cProject.Current
            cProject.ValidateProjectFile(mproject)
            If mproject.SetupModelParametersAfterProjectFileWasOpened() = False Then
                cGRM.writelogAndConsole("GRM setup was failed !!!", True, False)
                MsgBox("GRM setup was failed !!!", MsgBoxStyle.Critical, cGRM.BuildInfo.ProductName)
                Exit Sub
            End If

            If mproject.GeneralSimulEnv.mbCreateASCFile = True OrElse
                mproject.GeneralSimulEnv.mbCreateImageFile = True Then
                mbCreateDistributionFiles = True
                mRasterOutput = New cRasterOutput(mproject)
            End If
            mproject.GeneralSimulEnv.mbRunAanlyzer = True
            cGRM.writelogAndConsole(mPrjFPN + " -> Model setup completed.", cGRM.bwriteLog, False)
            CreatDataTableAndSetDataGridView(mproject.WatchPoint.mdtWatchPointInfo)
            mSimDuration_HR = CInt(mproject.GeneralSimulEnv.mSimDurationHOUR)
        Catch ex As Exception
            Throw ex
        End Try
    End Sub

    Private Sub SetupChart()
        Dim FPNobs As String = Trim(Me.tbFPNObsData.Text)
        Dim FPNsim As String = Trim(Me.tbFPNSimData.Text)
        Dim Qobss As New Dictionary(Of Integer, List(Of Single))
        Dim LegendsObs As New List(Of String)
        Dim LegendsSim As New List(Of String)
        Dim xLabels As Dictionary(Of Integer, String)
        Dim Qsims As Dictionary(Of Integer, List(Of Single))
        Dim RFsims As Dictionary(Of Integer, Single)
        Try

            If mbShowObsData = True Then
                LegendsObs = cChart.ReadObservedDataAndGetLegends(FPNobs)
                Qobss = cChart.ReadObservedDataAndSetDictionary(FPNobs)
            End If
            mObsDataCount = Qobss.Count
            mMaxPrintoutCount = CInt(mSimDuration_HR * 60 / mproject.GeneralSimulEnv.mPrintOutTimeStepMIN) + 1
            If ObsDataCount > mMaxPrintoutCount Then
                mMaxPrintoutCount = ObsDataCount
            End If

            If mbShowSimData = True Then
                Qsims = New Dictionary(Of Integer, List(Of Single))
                RFsims = New Dictionary(Of Integer, Single)
                Dim strLines() As String = System.IO.File.ReadAllLines(FPNsim, System.Text.Encoding.Default)
                xLabels = New Dictionary(Of Integer, String)
                '3번행이 열 이름
                Dim Titles() As String = strLines(3).Split(New String() {vbTab, ","}, StringSplitOptions.RemoveEmptyEntries)
                Dim colindexTime As Integer = 0
                Dim IndexQsim As New List(Of Integer)
                Dim colindexRf As Integer
                For n As Integer = 1 To Titles.Length - 1 '0번은 시간 열이므로.. 1부터 카운트..
                    If Titles(n) = "Rainfall_Mean" Then
                        colindexRf = n
                        Exit For
                    End If
                    If Titles(n) <> "Data_Time" Then
                        IndexQsim.Add(n)
                        Dim leg As String = Replace(Titles(n), "[", "")
                        leg = Replace(leg, "]", "")
                        LegendsSim.Add(leg)
                    End If
                Next

                If IndexQsim.Count <> mproject.WatchPoint.mdtWatchPointInfo.Rows.Count Then
                    MsgBox(" Watchpoint of current project is different from simulated file. ", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                    Exit Sub
                End If
                If strLines.Length < 5 Then
                    MsgBox(" There is no simulated data in the output file. ", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                    Exit Sub
                End If
                ''여기서 키를 만들고
                For n As Integer = 4 To strLines.Length - 1 '4번 부터 값이 있음.
                    Dim LineParts() As String = strLines(n).Split(New String() {vbTab, ","}, StringSplitOptions.RemoveEmptyEntries)
                    Dim i As Integer = n - 4
                    xLabels.Add(i, LineParts(colindexTime))
                    Dim qsim As New List(Of Single)
                    For qn As Integer = 0 To IndexQsim.Count - 1
                        qsim.Add(CSng(LineParts(IndexQsim(qn))))
                    Next
                    Qsims.Add(i, qsim)
                    RFsims.Add(i, CSng(LineParts(colindexRf)))
                Next
                mChart = New cChart(pbChartMain, pbChartRF, Qobss, LegendsObs, Qsims, LegendsSim, RFsims, xLabels, mMaxPrintoutCount)

                Dim dt As New GRMCore.GRMProject.WatchPointsDataTable
                For rn As Integer = 0 To LegendsSim.Count - 1
                    Dim r As GRMProject.WatchPointsRow = dt.NewWatchPointsRow
                    r.Name = LegendsSim(rn)
                    dt.Rows.Add(r)
                Next
                UpdateDgvWithSimDataFile(IndexQsim.Count, Qsims, xLabels, RFsims)
            Else
                Qsims = Nothing
                RFsims = Nothing
                If mproject.GeneralSimulEnv.mIsDateTimeFormat = True Then
                    xLabels = New Dictionary(Of Integer, String)
                    xLabels = cChart.GenerateDictionaryKeyWithDataCount(mMaxPrintoutCount, xLabels) ' 20160725 관측자료가 모의기간보다 짧을 경우를 위해 수정
                    Dim labelList As New List(Of String)
                    labelList = cComTools.GetTimeListToPrintout(mproject.GeneralSimulEnv.mSimStartDateTime,
                                                   CInt(mproject.GeneralSimulEnv.mPrintOutTimeStepMIN),
                                                   mMaxPrintoutCount) ' 20160725 관측자료가 모의기간보다 짧을 경우를 위해 수정
                    For n As Integer = 0 To labelList.Count - 1
                        xLabels(n) = labelList(n)
                    Next
                Else
                    xLabels = Nothing
                End If
                If mproject.WatchPoint.WPCount > 0 Then
                    For Each row As GRMProject.WatchPointsRow In mproject.WatchPoint.mdtWatchPointInfo.Rows
                        LegendsSim.Add(row.Name)
                    Next
                End If
                If mproject.GeneralSimulEnv.mIsDateTimeFormat = True Then
                    mChart = New cChart(pbChartMain, pbChartRF, Qobss, LegendsObs, LegendsSim, xLabels, mMaxPrintoutCount)
                Else
                    mChart = New cChart(pbChartMain, pbChartRF, Qobss, LegendsObs, LegendsSim, mMaxPrintoutCount)
                End If
            End If
            Call SetTrackBar(mMaxPrintoutCount)
            'Call SetTrackBar(mChart.MaxCountOfData)
            Me.pbChartMain.Controls.Clear()
            Me.pbChartRF.Controls.Clear()
            mChart.DrawChart()
            cThisSimulation.mAnalyzerSet = True
        Catch ex As Exception
            MsgBox("err", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
        End Try
    End Sub


    Private Sub startSingleEvent()
        Try
            mSimulator = New cSimulator
            Dim ts As New ThreadStart(AddressOf StartGRMSimulationThread)
            Dim th As New Thread(ts)
            th.Start()
        Catch ex As Exception
            Throw ex
        End Try
    End Sub

    Private Sub StartGRMSimulationThread()
        If cOutPutControl.CreateNewOutputFiles(mproject, False) = False Then
            MsgBox("Making output file template was failed..", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
            Exit Sub
        End If
        clearPictureBoxes(True)
        mSimulator.SimulateSingleEvent(mproject)
    End Sub

    Private Sub clearPictureBoxes(all As Boolean, Optional pb As PictureBox = Nothing)
        If all = True Then
            If Me.pbSSRimg.Image IsNot Nothing Then
                pbSSRimg.Image.Dispose()
                pbSSRimg.Image = Nothing
            End If
            If Me.pbFLOWimg.Image IsNot Nothing Then
                pbFLOWimg.Image.Dispose()
                pbFLOWimg.Image = Nothing
            End If
            If Me.pbRFimg.Image IsNot Nothing Then
                pbRFimg.Image.Dispose()
                pbRFimg.Image = Nothing
            End If
            If Me.pbRFACCimg.Image IsNot Nothing Then
                pbRFACCimg.Image.Dispose()
                pbRFACCimg.Image = Nothing
            End If
        Else
            If pb IsNot Nothing AndAlso pb.Image IsNot Nothing Then
                pb.Image.Dispose()
                pb.Image = Nothing
            End If
        End If

    End Sub


    Private Delegate Sub RasterOutputDelegate(ByVal nowTtoPrint_MIN As Integer, imgWidth As Integer, imgHeight As Integer, usingOtherThread As Boolean)
    Private Delegate Sub DrawChartDelegate(ByVal nowT_Min As Integer, interCoef As Single)

    Private Sub Simulator_MakeRasterOutput(sender As cSimulator,
                                        nowTtoPrint_MIN As Integer) Handles mSimulator.MakeRasterOutput
        If mbCreateDistributionFiles = True Then
            mRasterOutput.MakeDistributionFiles(nowTtoPrint_MIN, mRasterOutput.ImgWidth, mRasterOutput.ImgHeight, False)
            Dim strNowTimeToPrintOut As String = cComTools.GetTimeToPrintOut(mproject.GeneralSimulEnv.mIsDateTimeFormat,
                                                                             mproject.GeneralSimulEnv.mSimStartDateTime, nowTtoPrint_MIN)
            strNowTimeToPrintOut = cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut)
            If mproject.GeneralSimulEnv.mbShowSoilSaturation = True Then
                Dim mIMGfpn As String = Path.Combine(mproject.OFPSSRDistribution, cGRM.CONST_DIST_SSR_FILE_HEAD + strNowTimeToPrintOut + ".png")
                mImgFPN_dist_SSR.Add(mIMGfpn)
                'DrawPictureBoxUsingNewImgFile(Me.pbSSRimg, mIMGfpn)
                DrawPictureBoxUsingBitmap(Me.pbSSRimg, mRasterOutput.mImgSSR, mIMGfpn)
            End If
            If mproject.GeneralSimulEnv.mbShowRFdistribution = True Then
                Dim mIMGfpn As String = Path.Combine(mproject.OFPRFDistribution, cGRM.CONST_DIST_RF_FILE_HEAD + strNowTimeToPrintOut + ".png")
                mImgFPN_dist_RF.Add(mIMGfpn)
                'DrawPictureBoxUsingNewImgFile(Me.pbRFimg, mIMGfpn)
                DrawPictureBoxUsingBitmap(Me.pbRFimg, mRasterOutput.mImgRF, mIMGfpn)
            End If
            If mproject.GeneralSimulEnv.mbShowRFaccDistribution = True Then
                Dim mIMGfpn As String = Path.Combine(mproject.OFPRFAccDistribution, cGRM.CONST_DIST_RFACC_FILE_HEAD + strNowTimeToPrintOut + ".png")
                mImgFPN_dist_RFAcc.Add(mIMGfpn)
                'DrawPictureBoxUsingNewImgFile(Me.pbRFACCimg, mIMGfpn)
                DrawPictureBoxUsingBitmap(Me.pbRFACCimg, mRasterOutput.mImgRFacc, mIMGfpn)
            End If
            If mproject.GeneralSimulEnv.mbShowFlowDistribution = True Then
                Dim mIMGfpn As String = Path.Combine(mproject.OFPFlowDistribution, cGRM.CONST_DIST_FLOW_FILE_HEAD + strNowTimeToPrintOut + ".png")
                mImgFPN_dist_Flow.Add(mIMGfpn)
                'DrawPictureBoxUsingNewImgFile(Me.pbFLOWimg, mIMGfpn)
                DrawPictureBoxUsingBitmap(Me.pbFLOWimg, mRasterOutput.mImgFlow, mIMGfpn)
            End If
        End If
        System.Windows.Forms.Application.DoEvents()

    End Sub


    Private Sub mSimulator_SendQToAnalyzer(sender As cSimulator, nowTtoPrint_MIN As Integer, interCoef As Single) Handles mSimulator.SendQToAnalyzer
        DrawChart(nowTtoPrint_MIN, interCoef)
    End Sub

    Private Sub DrawChart(nowTtoPrint_MIN As Integer, interCoef As Single)
        If Me.pbChartMain.InvokeRequired = True Then
            Dim d As New DrawChartDelegate(AddressOf AddDataAndUpdateChart)
            Me.pbChartMain.Invoke(d, nowTtoPrint_MIN, interCoef)
        Else
            AddDataAndUpdateChart(nowTtoPrint_MIN, interCoef)
        End If
    End Sub



    Public Sub AddDataAndUpdateChart(ByVal nowT_Min As Integer, interCoef As Single)
        Dim Qs As New List(Of Single)
        Dim qsim As Single = 0
        Dim qSimforEachWP As New Dictionary(Of Integer, Single)
        For Each cvid As Integer In cProject.Current.WatchPoint.WPCVidList
            Dim cvan As Integer = cvid - 1
            qsim = mproject.CV(cvan).Qprint_cms
            qSimforEachWP.Add(cvid, qsim)
            Qs.Add(qsim)
        Next
        mChart.AddQsimAndRFpointAndUpdateChart(CSng(cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m) * 1000, Qs)
        Dim timeToPrintOut As String
        If mproject.GeneralSimulEnv.mIsDateTimeFormat = True Then
            timeToPrintOut = cComTools.GetTimeToPrintOut(mproject.GeneralSimulEnv.mIsDateTimeFormat,
                                                         mproject.GeneralSimulEnv.mSimStartDateTime, nowT_Min)
        Else
            timeToPrintOut = CStr(nowT_Min)
        End If
        Call AddDataAndUpdateDataGridView(qSimforEachWP, timeToPrintOut)
    End Sub


    Private Sub pb_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) _
                Handles pb000.Click, pb005.Click, pb010.Click, pb015.Click,
                        pb020.Click, pb025.Click, pb030.Click, pb035.Click,
                        pb040.Click, pb045.Click, pb050.Click, pb055.Click,
                        pb060.Click, pb065.Click, pb070.Click, pb075.Click,
                        pb080.Click, pb085.Click, pb090.Click, pb095.Click,
                        pb100.Click
        Dim pb As PictureBox = CType(sender, PictureBox)
        Dim cd As New ColorDialog
        cd.Color = pb.BackColor
        If cd.ShowDialog(Me) = Windows.Forms.DialogResult.OK Then
            pb.BackColor = cd.Color
            Dim fpnBMP As String = CStr(Me.pbSSRimg.Image.Tag)
            Dim fpnASC As String = Path.Combine(IO.Path.GetDirectoryName(fpnBMP), Path.GetFileNameWithoutExtension(fpnBMP) + ".asc")
            If File.Exists(fpnASC) = True Then
                mImg.MakeImgFileUsingASCfileFromTL(fpnASC, fpnBMP, cImg.RendererRange.RendererFrom0to1,
                                                       Me.pbSSRimg.Width, Me.pbSSRimg.Height,
                                                   mImg.DefaultNullColor)
            End If
        End If
    End Sub

    Private Sub pbRF_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) _
            Handles pbRF000.Click, pbRF000to2.Click, pbRF002.Click, pbRF005.Click, pbRF010.Click,
                    pbRF015.Click, pbRF020.Click, pbRF025.Click, pbRF030.Click,
                    pbRF035.Click, pbRF040.Click, pbRF045.Click, pbRF050.Click,
                    pbRF060.Click, pbRF070.Click, pbRF080.Click, pbRF090.Click,
                    pbRF100.Click, pbRF120.Click, pbRF140.Click, pbRF160.Click,
                    pbRF200.Click
        Dim pb As PictureBox = CType(sender, PictureBox)
        Dim cd As New ColorDialog
        cd.Color = pb.BackColor
        If cd.ShowDialog(Me) = Windows.Forms.DialogResult.OK Then
            pb.BackColor = cd.Color
            Dim fpnBMP As String = CStr(Me.pbRFimg.Image.Tag)
            Dim fpnASC As String = Path.Combine(IO.Path.GetDirectoryName(fpnBMP), Path.GetFileNameWithoutExtension(fpnBMP) + ".asc")
            If File.Exists(fpnASC) = True Then
                mImg.MakeImgFileUsingASCfileFromTL(fpnASC, fpnBMP, cImg.RendererRange.RendererFrom0to100,
                                                       Me.pbRFimg.Width, Me.pbRFimg.Height,
                                                   mImg.DefaultNullColor)
            End If
        End If
    End Sub

    Private Sub pbRFacc_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) _
        Handles pbRFAcc000to5.Click, pbRFAcc005.Click, pbRFAcc010.Click, pbRFAcc015.Click,
                pbRFAcc020.Click, pbRFAcc025.Click, pbRFAcc030.Click, pbRFAcc040.Click,
                pbRFAcc050.Click, pbRFAcc060.Click, pbRFAcc070.Click, pbRFAcc080.Click,
                pbRFAcc100.Click, pbRFAcc120.Click, pbRFAcc140.Click, pbRFAcc160.Click,
                pbRFAcc200.Click, pbRFAcc250.Click, pbRFAcc300.Click, pbRFAcc400.Click,
                pbRFAcc500.Click
        Dim pb As PictureBox = CType(sender, PictureBox)
        Dim cd As New ColorDialog
        cd.Color = pb.BackColor
        If cd.ShowDialog(Me) = Windows.Forms.DialogResult.OK Then
            pb.BackColor = cd.Color
            Dim fpnBMP As String = CStr(Me.pbRFACCimg.Image.Tag)
            Dim fpnASC As String = Path.Combine(IO.Path.GetDirectoryName(fpnBMP), Path.GetFileNameWithoutExtension(fpnBMP) + ".asc")
            If File.Exists(fpnASC) = True Then
                mImg.MakeImgFileUsingASCfileFromTL(fpnASC, fpnBMP, cImg.RendererRange.RendererFrom0to500,
                                                       Me.pbRFACCimg.Width, Me.pbRFACCimg.Height,
                                                   mImg.DefaultNullColor)
            End If
        End If
    End Sub


    Private Sub fAnalyzer_FormClosed(ByVal sender As Object, ByVal e As System.Windows.Forms.FormClosedEventArgs) Handles Me.FormClosed


    End Sub

    Private Sub fAnalyzer_FormClosing(ByVal sender As Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles Me.FormClosing
        'ClearPictureBoxAndDrawNewImgwithImgFile(Me.pbSSRimg)
        'pbSSRimg = Nothing
        clearPictureBoxes(True)
        cThisSimulation.mAnalyzerSet = False
    End Sub


    ''' <summary>
    ''' 폼에 있는 picturebox를 지우고 새 이미지로 그린다.
    ''' </summary>
    ''' <param name="pb"></param>
    ''' <param name="imgFpn"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Shared Function DrawPictureBoxUsingNewImgFile(ByVal pb As PictureBox, imgFpn As String) As Boolean
        'If pb.Image IsNot Nothing Then
        '    pb.Image.Dispose()
        '    pb.Image = Nothing
        'End If
        If imgFpn <> "" Then
            If IO.File.Exists(imgFpn) Then
                Dim imgOrg As Bitmap = cImg.GetBMPImageByFileStream(imgFpn)
                Dim divideByH, divideByW As Double
                divideByW = imgOrg.Width / pb.Width
                divideByH = imgOrg.Height / pb.Height
                If divideByW > 1 Or divideByH > 1 Then
                    pb.Image = AutosizeImage(imgOrg, pb)
                Else
                    pb.Image = imgOrg
                End If
                pb.Image.Tag = imgFpn
            End If
        End If
        Return True
    End Function

    Public Shared Function DrawPictureBoxUsingBitmap(ByVal pb As PictureBox, inBM As Bitmap, Optional imgFpn As String = "") As Boolean
        If inBM IsNot Nothing Then
            Dim imgOrg As Bitmap = inBM
            Dim divideByH, divideByW As Double
            divideByW = imgOrg.Width / pb.Width
            divideByH = imgOrg.Height / pb.Height
            If divideByW > 1 Or divideByH > 1 Then
                pb.Image = AutosizeImage(imgOrg, pb)
            Else
                pb.Image = imgOrg
            End If
            pb.Image.Tag = imgFpn
        End If
        Return True
    End Function


    'Public Shared Function AutosizeImage(ByVal ImagePN As string, ByVal picBox As PictureBox, Optional ByVal pSizeMode As PictureBoxSizeMode = PictureBoxSizeMode.CenterImage) As Image
    Public Shared Function AutosizeImage(ByVal InBM As Bitmap, ByVal picBox As PictureBox, Optional ByVal pSizeMode As PictureBoxSizeMode = PictureBoxSizeMode.CenterImage) As Image
        Try
            picBox.Image = Nothing
            picBox.SizeMode = pSizeMode
            If InBM IsNot Nothing Then
                'If System.IO.File.Exists(ImagePN) Then
                Dim imgOrg As Bitmap = InBM
                Dim imgShow As Bitmap
                Dim g As Graphics
                Dim divideBy, divideByH, divideByW As Double
                'imgOrg = cImg.GetBMPImageByFileStream(ImagePN)
                divideByW = imgOrg.Width / picBox.Width
                divideByH = imgOrg.Height / picBox.Height
                '===============================================
                ' 이건 pb 보다 큰 이미지를 채우는 것,, 작은 것은 작게 보임
                If divideByW > 1 Or divideByH > 1 Then
                    If divideByW > divideByH Then
                        divideBy = divideByW
                    Else
                        divideBy = divideByH
                    End If
                    imgShow = New Bitmap(CInt(CDbl(imgOrg.Width) / divideBy), CInt(CDbl(imgOrg.Height) / divideBy))
                    imgShow.SetResolution(imgOrg.HorizontalResolution, imgOrg.VerticalResolution)
                    g = Graphics.FromImage(imgShow)
                    g.InterpolationMode = Drawing2D.InterpolationMode.HighQualityBicubic
                    g.DrawImage(imgOrg, New Rectangle(0, 0, CInt(CDbl(imgOrg.Width) / divideBy), CInt(CDbl(imgOrg.Height) / divideBy)), 0, 0, imgOrg.Width, imgOrg.Height, GraphicsUnit.Pixel)
                    g.Dispose()
                Else
                    imgShow = New Bitmap(imgOrg.Width, imgOrg.Height)
                    imgShow.SetResolution(imgOrg.HorizontalResolution, imgOrg.VerticalResolution)
                    g = Graphics.FromImage(imgShow)
                    g.InterpolationMode = Drawing2D.InterpolationMode.HighQualityBicubic
                    g.DrawImage(imgOrg, New Rectangle(0, 0, imgOrg.Width, imgOrg.Height), 0, 0, imgOrg.Width, imgOrg.Height, GraphicsUnit.Pixel)
                    g.Dispose()
                End If
                '===============================================
                imgOrg.Dispose()
                Return imgShow
            Else
                Return Nothing
            End If
        Catch ex As Exception
            MsgBox(ex.ToString)
            Return Nothing
        End Try
    End Function


    Public Sub SetDistributedPictureFilesAndRenderer(inForm As fAnalyzer)
        Dim riskImg As New cImg(cImg.RendererType.Risk)
        Dim depthImg As New cImg(cImg.RendererType.WaterDepth)

        'Dim bc As New Color
        'bc = Color.FromArgb(255, 255, 255, 224)
        'Me.pbSSRimg.BackColor = bc
        If mproject.GeneralSimulEnv.mbShowSoilSaturation = True Then
            Call UpdateSoilSaturationImgFileList(mproject.ProjectPathName, Imaging.ImageFormat.Png)
            Call SetSSRendererWithInitialColor(inForm, riskImg)
            'Me.pbSSRimg.BackColor = tmpImg.DefaultNullColor
        End If
        'Me.pbRFimg.BackColor = bc
        If mproject.GeneralSimulEnv.mbShowRFdistribution = True Then
            Call UpdateRFdistImgFileList(mproject.ProjectPathName, Imaging.ImageFormat.Png)
            Call SetRFRendererWithInitialColor(inForm, depthImg)
            'Me.pbRFimg.BackColor = tmpImg.DefaultNullColor
        End If
        'Me.pbRFACCimg.BackColor = bc
        If mproject.GeneralSimulEnv.mbShowRFaccDistribution = True Then
            Call UpdateRFAccDistImgFileList(mproject.ProjectPathName, Imaging.ImageFormat.Png)
            Call SetRFAccRendererWithInitialColor(inForm, depthImg)
            'Me.pbRFACCimg.BackColor = tmpImg.DefaultNullColor
        End If
        'Me.pbFLOWimg.BackColor = bc
        If mproject.GeneralSimulEnv.mbShowFlowDistribution = True Then
            Call UpdateFlowDistImgFileList(mproject.ProjectPathName, Imaging.ImageFormat.Png)
            Call SetFlowRendererWithInitialColor(inForm, depthImg)
            'Me.pbFLOWimg.BackColor = tmpImg.DefaultNullColor
        End If

    End Sub


    Public Shared Function SetSSRendererWithInitialColor(ByVal inputForm As fAnalyzer, ByVal inImg As cImg) As Boolean
        Try
            With inputForm
                .pb000.BackColor = inImg.iniColor(1)
                .pb005.BackColor = inImg.iniColor(2)
                .pb010.BackColor = inImg.iniColor(3)
                .pb015.BackColor = inImg.iniColor(4)
                .pb020.BackColor = inImg.iniColor(5)
                .pb025.BackColor = inImg.iniColor(6)
                .pb030.BackColor = inImg.iniColor(7)
                .pb035.BackColor = inImg.iniColor(8)
                .pb040.BackColor = inImg.iniColor(9)
                .pb045.BackColor = inImg.iniColor(10)
                .pb050.BackColor = inImg.iniColor(11)
                .pb055.BackColor = inImg.iniColor(12)
                .pb060.BackColor = inImg.iniColor(13)
                .pb065.BackColor = inImg.iniColor(14)
                .pb070.BackColor = inImg.iniColor(15)
                .pb075.BackColor = inImg.iniColor(16)
                .pb080.BackColor = inImg.iniColor(17)
                .pb085.BackColor = inImg.iniColor(18)
                .pb090.BackColor = inImg.iniColor(19)
                .pb095.BackColor = inImg.iniColor(20)
                .pb100.BackColor = inImg.iniColor(21)
            End With
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function


    Public Shared Function SetFlowRendererWithInitialColor(ByVal inputForm As fAnalyzer, ByVal inImg As cImg) As Boolean
        Try
            With inputForm
                .pbFlow_0_10.BackColor = inImg.iniColor(0)
                .pbFlow_10_50.BackColor = inImg.iniColor(1)
                .pbFlow_50_100.BackColor = inImg.iniColor(2)
                .pbFlow_100_200.BackColor = inImg.iniColor(3)
                .pbFlow_200_300.BackColor = inImg.iniColor(4)
                .pbFlow_300_400.BackColor = inImg.iniColor(5)
                .pbFlow_400_600.BackColor = inImg.iniColor(6)
                .pbFlow_600_800.BackColor = inImg.iniColor(7)
                .pbFlow_800_1000.BackColor = inImg.iniColor(8)
                .pbFlow_1000_1500.BackColor = inImg.iniColor(9)
                .pbFlow_1500_2000.BackColor = inImg.iniColor(10)
                .pbFlow_2000_2500.BackColor = inImg.iniColor(11)
                .pbFlow_2500_3000.BackColor = inImg.iniColor(12)
                .pbFlow_3000_4000.BackColor = inImg.iniColor(13)
                .pbFlow_4000_5000.BackColor = inImg.iniColor(14)
                .pbFlow_5000_6000.BackColor = inImg.iniColor(15)
                .pbFlow_6000_8000.BackColor = inImg.iniColor(16)
                .pbFlow_8000_10000.BackColor = inImg.iniColor(17)
                .pbFlow_10000_15000.BackColor = inImg.iniColor(18)
                .pbFlow_15000_20000.BackColor = inImg.iniColor(19)
                .pbFlow_20000_30000.BackColor = inImg.iniColor(20)
                .pbFlow_30000.BackColor = inImg.iniColor(21)
            End With
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Public Shared Function SetRFRendererWithInitialColor(ByVal inputForm As fAnalyzer, ByVal inImg As cImg) As Boolean
        Try
            With inputForm
                .pbRF000.BackColor = inImg.iniColor(0)
                .pbRF000to2.BackColor = inImg.iniColor(1)
                .pbRF002.BackColor = inImg.iniColor(2)
                .pbRF005.BackColor = inImg.iniColor(3)
                .pbRF010.BackColor = inImg.iniColor(4)
                .pbRF015.BackColor = inImg.iniColor(5)
                .pbRF020.BackColor = inImg.iniColor(6)
                .pbRF025.BackColor = inImg.iniColor(7)
                .pbRF030.BackColor = inImg.iniColor(8)
                .pbRF035.BackColor = inImg.iniColor(9)
                .pbRF040.BackColor = inImg.iniColor(10)
                .pbRF045.BackColor = inImg.iniColor(11)
                .pbRF050.BackColor = inImg.iniColor(12)
                .pbRF060.BackColor = inImg.iniColor(13)
                .pbRF070.BackColor = inImg.iniColor(14)
                .pbRF080.BackColor = inImg.iniColor(15)
                .pbRF090.BackColor = inImg.iniColor(16)
                .pbRF100.BackColor = inImg.iniColor(17)
                .pbRF120.BackColor = inImg.iniColor(18)
                .pbRF140.BackColor = inImg.iniColor(19)
                .pbRF160.BackColor = inImg.iniColor(20)
                .pbRF200.BackColor = inImg.iniColor(21)
            End With
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Public Shared Function SetRFAccRendererWithInitialColor(ByVal inputForm As fAnalyzer, ByVal inImg As cImg) As Boolean
        Try
            With inputForm
                .pbRFAcc000.BackColor = inImg.iniColor(0)
                .pbRFAcc000to5.BackColor = inImg.iniColor(1)
                .pbRFAcc005.BackColor = inImg.iniColor(2)
                .pbRFAcc010.BackColor = inImg.iniColor(3)
                .pbRFAcc015.BackColor = inImg.iniColor(4)
                .pbRFAcc020.BackColor = inImg.iniColor(5)
                .pbRFAcc025.BackColor = inImg.iniColor(6)
                .pbRFAcc030.BackColor = inImg.iniColor(7)
                .pbRFAcc040.BackColor = inImg.iniColor(8)
                .pbRFAcc050.BackColor = inImg.iniColor(9)
                .pbRFAcc060.BackColor = inImg.iniColor(10)
                .pbRFAcc070.BackColor = inImg.iniColor(11)
                .pbRFAcc080.BackColor = inImg.iniColor(12)
                .pbRFAcc100.BackColor = inImg.iniColor(13)
                .pbRFAcc120.BackColor = inImg.iniColor(14)
                .pbRFAcc140.BackColor = inImg.iniColor(15)
                .pbRFAcc160.BackColor = inImg.iniColor(16)
                .pbRFAcc200.BackColor = inImg.iniColor(17)
                .pbRFAcc250.BackColor = inImg.iniColor(18)
                .pbRFAcc300.BackColor = inImg.iniColor(19)
                .pbRFAcc400.BackColor = inImg.iniColor(20)
                .pbRFAcc500.BackColor = inImg.iniColor(21)
            End With
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Private Sub UpdateDgvWithSimDataFile(ByVal wpcount As Integer, ByVal Qsims As Dictionary(Of Integer, List(Of Single)),
                                        ByVal xLabels As Dictionary(Of Integer, String),
                                        ByVal RFsims As Dictionary(Of Integer, Single))
        mdtData.Clear()
        mdtData.BeginLoadData()
        For n As Integer = 0 To Qsims.Count - 1
            Dim row As Data.DataRow = mdtData.NewRow
            row.Item(0) = xLabels(n)
            For m As Integer = 0 To wpcount - 1
                row.Item(m + 1) = Format((Qsims(n)(m)), "#0.#0")
            Next
            row.Item(wpcount + 1) = Format(RFsims(n), "#0.#0")
            mdtData.Rows.Add(row)
        Next
        mdtData.EndLoadData()
    End Sub

    Public Sub AddDataAndUpdateDataGridView(qSimforEachWP As Dictionary(Of Integer, Single), timeToPrintOut As String)
        mEventType = Nothing
        Dim row As Data.DataRow = mdtData.NewRow
        row.Item(0) = timeToPrintOut
        For Each r As GRMProject.WatchPointsRow In mproject.WatchPoint.mdtWatchPointInfo.Rows
            Dim v As Single = qSimforEachWP(r.CVID)
            'If project.CV(r.CVID - 1).FlowType = cGRM.CellFlowType.OverlandFlow Then
            '    v = project.CV(r.CVID - 1).QCVof_i_j_m3Ps '배열번호는 cvid-1
            'Else
            '    v = project.CV(r.CVID - 1).mStreamAttr.QCVch_i_j_m3Ps
            'End If
            row.Item(r.Name) = Format(v, "#0.#0")
        Next

        row.Item(mdtData.Columns.Count - 1) = Format(cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m * 1000, "#0.#0")
        mdtData.Rows.Add(row)
        Me.dgvResults.CurrentCell = Me.dgvResults.Rows(Me.dgvResults.RowCount - 1).Cells(0)
        Me.dgvResults.CurrentRow.Selected = False '여기서는 선택하지 않게..
        System.Windows.Forms.Application.DoEvents()
    End Sub


    Private Function UpdateSoilSaturationImgFileList(ByVal FPNproject As String, imgFormat As Imaging.ImageFormat) As Boolean
        Try
            mImgFPN_dist_SSR = New List(Of String)
            Dim dirName_base As String = IO.Path.GetDirectoryName(FPNproject)
            Dim dirName_save As String = IO.Path.GetFileNameWithoutExtension(FPNproject) + "_" + cGRM.CONST_DIST_SSR_DIRECTORY_TAG
            Dim fpn As String = Path.Combine(dirName_base, dirName_save)
            Dim files As String() = Nothing
            If IO.Directory.Exists(fpn) Then
                If imgFormat Is Imaging.ImageFormat.Bmp Then
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                ElseIf imgFormat Is Imaging.ImageFormat.Png Then
                    files = IO.Directory.GetFiles(fpn, "*.png")
                Else
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                End If
            End If
            If files IsNot Nothing Then
                For n As Integer = 0 To files.Length - 1
                    mImgFPN_dist_SSR.Add(files(n))
                Next
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Private Function UpdateRFdistImgFileList(ByVal FPNproject As String, imgFormat As Imaging.ImageFormat) As Boolean
        Try
            mImgFPN_dist_RF = New List(Of String)
            Dim dirName As String = IO.Path.GetDirectoryName(FPNproject)
            Dim folderName As String = IO.Path.GetFileNameWithoutExtension(FPNproject) + "_" + cGRM.CONST_DIST_RF_DIRECTORY_TAG
            Dim fpn As String = Path.Combine(dirName, folderName)
            Dim files As String() = Nothing
            If IO.Directory.Exists(fpn) Then
                If imgFormat Is Imaging.ImageFormat.Bmp Then
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                ElseIf imgFormat Is Imaging.ImageFormat.Png Then
                    files = IO.Directory.GetFiles(fpn, "*.png")
                Else
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                End If
            End If
            If files IsNot Nothing Then
                For n As Integer = 0 To files.Length - 1
                    mImgFPN_dist_RF.Add(files(n))
                Next
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Private Function UpdateRFAccDistImgFileList(ByVal FPNproject As String, imgFormat As Imaging.ImageFormat) As Boolean
        Try
            mImgFPN_dist_RFAcc = New List(Of String)
            Dim dirName As String = IO.Path.GetDirectoryName(FPNproject)
            Dim folderName As String = IO.Path.GetFileNameWithoutExtension(FPNproject) + "_" + cGRM.CONST_DIST_RFACC_DIRECTORY_TAG
            Dim fpn As String = Path.Combine(dirName, folderName)
            Dim files As String() = Nothing
            If IO.Directory.Exists(fpn) Then
                If imgFormat Is Imaging.ImageFormat.Bmp Then
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                ElseIf imgFormat Is Imaging.ImageFormat.Png Then
                    files = IO.Directory.GetFiles(fpn, "*.png")
                Else
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                End If
            End If
            If files IsNot Nothing Then
                For n As Integer = 0 To files.Length - 1
                    mImgFPN_dist_RFAcc.Add(files(n))
                Next
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function


    Private Function UpdateFlowDistImgFileList(ByVal FPNproject As String, imgFormat As Imaging.ImageFormat) As Boolean
        Try
            mImgFPN_dist_Flow = New List(Of String)
            Dim dirName As String = IO.Path.GetDirectoryName(FPNproject)
            Dim folderName As String = IO.Path.GetFileNameWithoutExtension(FPNproject) + "_" + cGRM.CONST_DIST_FLOW_DIRECTORY_TAG
            Dim fpn As String = Path.Combine(dirName, folderName)
            Dim files As String() = Nothing
            If IO.Directory.Exists(fpn) Then
                If imgFormat Is Imaging.ImageFormat.Bmp Then
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                ElseIf imgFormat Is Imaging.ImageFormat.Png Then
                    files = IO.Directory.GetFiles(fpn, "*.png")
                Else
                    files = IO.Directory.GetFiles(fpn, "*.bmp")
                End If
            End If
            If files IsNot Nothing Then
                For n As Integer = 0 To files.Length - 1
                    mImgFPN_dist_Flow.Add(files(n))
                Next
            End If
            Return True
        Catch ex As Exception
            Return False
        End Try
    End Function

    Private Sub SetTrackBar(ByVal Max As Integer)
        With Me.tbChart
            .Maximum = Max
            .Minimum = 0
        End With
        Call SetBaseTarckingBar()
    End Sub


    Private Function updateDistributionPictureBoxWhileTrackBarChanged() As Boolean
        mCurrentOrder = Me.tbChart.Value
        If mproject.GeneralSimulEnv.mbShowSoilSaturation = True Then
            If mImgFPN_dist_SSR IsNot Nothing AndAlso mImgFPN_dist_SSR.Count > 0 Then
                If mCurrentOrder > 0 AndAlso mCurrentOrder - 1 < mImgFPN_dist_SSR.Count Then
                    If mImgFPN_dist_SSR(mCurrentOrder - 1) IsNot Nothing AndAlso
                       IO.File.Exists(mImgFPN_dist_SSR(mCurrentOrder - 1)) Then
                        DrawPictureBoxUsingNewImgFile(Me.pbSSRimg, mImgFPN_dist_SSR(mCurrentOrder - 1))
                    End If
                Else
                    clearPictureBoxes(False, Me.pbSSRimg)
                    Me.pbSSRimg.Refresh()
                End If
            Else
                clearPictureBoxes(False, Me.pbSSRimg)
                Me.pbSSRimg.Refresh()
            End If
        End If

        If mproject.GeneralSimulEnv.mbShowRFdistribution = True Then
            If mImgFPN_dist_RF IsNot Nothing AndAlso mImgFPN_dist_RF.Count > 0 Then
                If mCurrentOrder > 0 AndAlso mCurrentOrder - 1 < mImgFPN_dist_RF.Count Then
                    If mImgFPN_dist_RF(mCurrentOrder - 1) IsNot Nothing AndAlso
                       IO.File.Exists(mImgFPN_dist_RF(mCurrentOrder - 1)) Then
                        DrawPictureBoxUsingNewImgFile(Me.pbRFimg, mImgFPN_dist_RF(mCurrentOrder - 1))
                    End If
                Else
                    clearPictureBoxes(False, Me.pbRFimg)
                    Me.pbRFimg.Refresh()
                End If
            Else
                clearPictureBoxes(False, Me.pbRFimg)
                Me.pbRFimg.Refresh()
            End If
        End If

        If mproject.GeneralSimulEnv.mbShowRFaccDistribution = True Then
            If mImgFPN_dist_RFAcc IsNot Nothing AndAlso mImgFPN_dist_RFAcc.Count > 0 Then
                If mCurrentOrder > 0 AndAlso mCurrentOrder - 1 < mImgFPN_dist_RFAcc.Count Then
                    If mImgFPN_dist_RFAcc(mCurrentOrder - 1) IsNot Nothing AndAlso
                       IO.File.Exists(mImgFPN_dist_RFAcc(mCurrentOrder - 1)) Then
                        DrawPictureBoxUsingNewImgFile(Me.pbRFACCimg, mImgFPN_dist_RFAcc(mCurrentOrder - 1))
                    End If
                Else
                    clearPictureBoxes(False, Me.pbRFACCimg)
                    Me.pbRFACCimg.Refresh()
                End If
            Else
                clearPictureBoxes(False, Me.pbRFACCimg)
                Me.pbRFACCimg.Refresh()
            End If
        End If

        If mproject.GeneralSimulEnv.mbShowFlowDistribution = True Then
            If mImgFPN_dist_Flow IsNot Nothing AndAlso mImgFPN_dist_Flow.Count > 0 Then
                If mCurrentOrder > 0 AndAlso mCurrentOrder - 1 < mImgFPN_dist_Flow.Count Then
                    If mImgFPN_dist_Flow(mCurrentOrder - 1) IsNot Nothing AndAlso
                       IO.File.Exists(mImgFPN_dist_Flow(mCurrentOrder - 1)) Then
                        DrawPictureBoxUsingNewImgFile(Me.pbFLOWimg, mImgFPN_dist_Flow(mCurrentOrder - 1))
                    End If
                Else
                    clearPictureBoxes(False, Me.pbFLOWimg)
                    Me.pbFLOWimg.Refresh()
                End If
            Else
                clearPictureBoxes(False, Me.pbFLOWimg)
                Me.pbFLOWimg.Refresh()
            End If
        End If
        Return True
    End Function


    Private Sub RemoveTrackingLine()
        If Controls.Item("TrackLine") IsNot Nothing Then
            Controls.Remove(Controls.Item("TrackLine"))
        End If
    End Sub

    Private Sub DrawAndMoveTrackingLine()
        Try
            Call RemoveTrackingLine()
            Dim p As New Panel
            p.Name = "TrackLine"
            p.Enabled = False
            p.Width = 2
            p.Height = 267
            Dim r As Single = CSng(Me.tbChart.Value / Me.tbChart.Maximum)
            p.Location = New Point(CInt(r * 442 + 474), 40) '446+474
            p.BackColor = Color.Red
            Controls.Add(p)
            p.BringToFront()
        Catch ex As Exception
            'If ex.Message = "창 핸들을 만드는 동안 오류가 발생했습니다." Then
            '    tbChart.Value = GetTrackBarValueByMousePoisition()
            '    Me.Refresh()
            '    Exit Sub
            'End If
        End Try
    End Sub

    Private Sub tbChart_MouseDown(ByVal sender As Object, ByVal e As System.Windows.Forms.MouseEventArgs) Handles tbChart.MouseDown
        mEventType = TrackingEvent.TrackingBarChanged
    End Sub

    Private Sub tbChart_MouseUp(ByVal sender As Object, ByVal e As System.Windows.Forms.MouseEventArgs) Handles tbChart.MouseUp

    End Sub

    Private Function GetTrackBarValueByMousePoisition(ByVal e As System.Windows.Forms.MouseEventArgs) As Integer
        Dim tbvalue As Integer

        If e.X < 10 Then
            tbvalue = CInt(tbChart.Maximum * (e.X - 9) / 460)
        Else
            tbvalue = CInt(tbChart.Maximum * (e.X) / 460)
        End If

        If Abs(tbvalue - tbChart.Value) > tbChart.LargeChange Then
        Else
            tbvalue = tbChart.Value
        End If

        If tbvalue > tbChart.Maximum Then tbvalue = tbChart.Maximum
        If tbvalue < 0 Then tbvalue = 0
        mEventType = TrackingEvent.TrackingBarChanged
        Return tbvalue
    End Function




    Private Sub tbChart_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles tbChart.KeyDown
        If e.KeyValue = Keys.Right OrElse e.KeyValue = Keys.Left _
          OrElse e.KeyValue = Keys.Up OrElse e.KeyValue = Keys.Down Then
            mEventType = TrackingEvent.TrackingBarChanged
        End If
    End Sub


    Private Sub tbChart_ValueChanged(ByVal sender As Object, ByVal e As System.EventArgs) Handles tbChart.ValueChanged
        Call DrawAndMoveTrackingLine()
        If mEventType = TrackingEvent.TrackingBarChanged Then Call UpdateDgvWithTrackingBarValue()
        Me.gbHydrograph.Refresh()
        Call updateDistributionPictureBoxWhileTrackBarChanged()
    End Sub


    Private Sub UpdateDgvWithTrackingBarValue()
        If tbChart.Value > 0 AndAlso Me.dgvResults.Rows.Count >= tbChart.Value Then
            Me.dgvResults.Rows(Me.tbChart.Value - 1).Selected = True
            Me.dgvResults.CurrentCell = Me.dgvResults.SelectedRows(0).Cells(0)
        Else
            If Me.dgvResults.SelectedRows.Count > 0 Then
                Me.dgvResults.SelectedRows(0).Selected = False
            End If
        End If
    End Sub

    Private Sub CreatDataTableAndSetDataGridView(ByVal dtWatchPoint As GRMProject.WatchPointsDataTable)
        If dtWatchPoint IsNot Nothing Then
            If dtWatchPoint.Rows.Count > 0 Then
                mdtData = New DataTable
                mdtData.Columns.Add("Time") '이건 라벨. 시간
                For Each row As GRMProject.WatchPointsRow In dtWatchPoint.Rows
                    mdtData.Columns.Add(row.Name)
                Next
                mdtData.Columns.Add("Rainfall") '이건 강우
                dgvResults.ColumnHeadersDefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter
                Me.dgvResults.DataSource = mdtData
                dgvResults.AutoResizeColumns()
            End If
        End If
    End Sub


    Public Sub UpdateWatchpointINfo(ByVal inputWPdataTable As GRMProject.WatchPointsDataTable)
        mproject.WatchPoint.mdtWatchPointInfo = inputWPdataTable
        Call CreatDataTableAndSetDataGridView(mproject.WatchPoint.mdtWatchPointInfo)
    End Sub



    Private Sub btTBStop_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btTBStop.Click
        Me.tmAni.Stop()
    End Sub

    Private Sub btLoadSimData_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btLoadSimData.Click
        Dim fod As New OpenFileDialog
        If fod.ShowDialog = Windows.Forms.DialogResult.OK Then
            Me.tbFPNSimData.Text = fod.FileName
        End If
    End Sub

    Private Sub btOpenGMP_Click(sender As Object, e As EventArgs) Handles btOpenGMP.Click
        Dim fod As New OpenFileDialog
        If fod.ShowDialog = Windows.Forms.DialogResult.OK Then
            Me.tbGMPfpn.Text = fod.FileName
        End If
    End Sub

    Private Sub btLoadObsQ_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btLoadObsData.Click
        Dim fod As New OpenFileDialog
        If fod.ShowDialog = Windows.Forms.DialogResult.OK Then
            Me.tbFPNObsData.Text = fod.FileName
        End If
    End Sub



    Private Sub dgvResults_KeyDown(ByVal sender As Object, ByVal e As System.Windows.Forms.KeyEventArgs) Handles dgvResults.KeyDown
        If e.KeyValue = Keys.Up OrElse e.KeyValue = Keys.Down Then
            mEventType = TrackingEvent.DataGridViewChanged
        End If
    End Sub

    Private Sub dgvResults_MouseClick(ByVal sender As Object, ByVal e As System.Windows.Forms.MouseEventArgs) Handles dgvResults.MouseClick
        If dgvResults.CurrentRow IsNot Nothing Then
            Dim v As Integer = dgvResults.CurrentRow.Index + 1
            If v >= tbChart.Minimum AndAlso v <= tbChart.Maximum Then
                tbChart.Value = dgvResults.CurrentRow.Index + 1
            End If
        End If
    End Sub

    Private Sub dgvResults_RowHeaderMouseClick(ByVal sender As Object, ByVal e As System.Windows.Forms.DataGridViewCellMouseEventArgs) Handles dgvResults.RowHeaderMouseClick
        If dgvResults.CurrentRow IsNot Nothing Then
            Dim v As Integer = dgvResults.CurrentRow.Index + 1
            If v >= tbChart.Minimum AndAlso v <= tbChart.Maximum Then
                tbChart.Value = dgvResults.CurrentRow.Index + 1
            End If
        End If
    End Sub

    Private Sub chkLoadSimData_CheckedChanged(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles chkLoadSimData.CheckedChanged
        Me.tbFPNSimData.Enabled = Me.chkLoadSimData.Checked
        Me.btLoadSimData.Enabled = Me.chkLoadSimData.Checked
        If Me.chkLoadSimData.Checked Then
            Me.btStartGRMorApplySettings.Text = "Apply settings"
        Else
            Me.btStartGRMorApplySettings.Text = "Start simulation"
        End If
    End Sub

    Private Sub dgvResults_SelectionChanged(ByVal sender As Object, ByVal e As System.EventArgs) Handles dgvResults.SelectionChanged
        If mEventType = TrackingEvent.DataGridViewChanged Then
            tbChart.Value = dgvResults.CurrentRow.Index + 1
        End If
    End Sub

    Private Sub btInitializeRenderer_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) _
                Handles btInitializeSSRenderer.Click, btInitializeRFrenderer.Click, btInitializeRFAccRenderer.Click, btInitializeFlowRenderer.Click
        Dim fpnASC As String
        Dim fpnBMP As String
        Dim pb As PictureBox = Nothing
        Dim IniColor As New cImg(cImg.RendererType.Risk)
        Dim rangeType As cImg.RendererRange = Nothing
        If sender Is Me.btInitializeSSRenderer Then
            SetSSRendererWithInitialColor(Me, IniColor)
            pb = Me.pbSSRimg
            rangeType = cImg.RendererRange.RendererFrom0to1
        End If
        If sender Is Me.btInitializeRFrenderer Then
            SetRFRendererWithInitialColor(Me, IniColor)
            pb = Me.pbRFimg
            rangeType = cImg.RendererRange.RendererFrom0to50
        End If
        If sender Is Me.btInitializeRFAccRenderer Then
            SetRFAccRendererWithInitialColor(Me, IniColor)
            pb = Me.pbRFACCimg
            rangeType = cImg.RendererRange.RendererFrom0to500
        End If

        If sender Is Me.btInitializeFlowRenderer Then
            SetFlowRendererWithInitialColor(Me, IniColor)
            pb = Me.pbFLOWimg
            rangeType = cImg.RendererRange.RendererFrom0to10000
        End If

        If pb.Image IsNot Nothing Then
            fpnBMP = CStr(pb.Image.Tag)
            fpnASC = Path.Combine(IO.Path.GetDirectoryName(fpnBMP), Path.GetFileNameWithoutExtension(fpnBMP) + ".asc")
            If File.Exists(fpnASC) = True Then
                clearPictureBoxes(False, pb)
                pb.Refresh()
                mImg.MakeImgFileUsingASCfileFromTL(fpnASC, fpnBMP, rangeType,
                                                       pb.Width, pb.Height,
                                                       mImg.DefaultNullColor)
            End If
        End If
    End Sub


    Private Sub tbChart_Scroll(ByVal sender As Object, ByVal e As System.EventArgs) Handles tbChart.Scroll

    End Sub

    Public ReadOnly Property ObsDataCount() As Integer
        Get
            Return mObsDataCount
        End Get
    End Property


    Private Sub btTBstart_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btTBstart.Click
        mEventType = TrackingEvent.TrackingBarChanged
        mAninmationRepeat = False
        If Me.tbChart.Value = Me.tbChart.Maximum Then
            Call SetBaseTarckingBar()
        End If
        StartAnimation()
    End Sub


    Private Sub btTBrepeat_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btTBrepeat.Click
        Call SetBaseTarckingBar()
        mEventType = TrackingEvent.TrackingBarChanged
        mAninmationRepeat = True
        StartAnimation()
    End Sub

    Private Sub StartAnimation()
        Me.tmAni.Interval = 300
        Me.tmAni.Start()
    End Sub

    Private Sub tmAni_Tick(ByVal sender As Object, ByVal e As System.EventArgs) Handles tmAni.Tick
        Me.tbChart.Value = Me.tbChart.Value + 1
        If  mAninmationRepeat = False AndAlso Me.tbChart.Value = mChart.MaxCountOfSimulatedData Then
            Me.tmAni.Stop()
            Exit Sub
        End If

        If Me.tbChart.Value = tbChart.Maximum Then
            Me.tmAni.Stop()
            If mAninmationRepeat = True Then
                tbChart.Value = tbChart.Minimum
                Me.tmAni.Interval = 300
                Me.tmAni.Start()
            End If
        End If
    End Sub

    Private Sub btTBInitialize_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles btTBInitialize.Click
        Call SetBaseTarckingBar()
        mEventType = Nothing
        mAninmationRepeat = False
        Me.tmAni.Stop()
    End Sub

    Private Sub SetBaseTarckingBar()
        tbChart.Value = tbChart.Minimum
        Call RemoveTrackingLine()
        Call updateDistributionPictureBoxWhileTrackBarChanged()
    End Sub

    Private Sub btSample_Click(sender As Object, e As EventArgs) Handles btSample.Click
        Dim ofrmTextBox As New fTextBox
        Dim strOut As String

        strOut = "GaugeName" + vbCrLf
        strOut = strOut + "Value" + vbCrLf
        strOut = strOut + "Value" + vbCrLf
        strOut = strOut + "   .   " + vbCrLf
        strOut = strOut + "   .   " + vbCrLf + vbCrLf

        strOut = strOut + "Make observed data file as TEXT file type(using notepad, etc.)" + vbCrLf
        strOut = strOut + "GaugeName : observatory name. Variant data type" + vbCrLf
        strOut = strOut + "Value : observed data value. Integer or float data type." + vbCrLf
        strOut = strOut + "*******  Do not use comma(,) in value string."

        ofrmTextBox.txtTextBox.Text = strOut
        ofrmTextBox.txtTextBox.ReadOnly = True
        ofrmTextBox.txtTextBox.Font = System.Drawing.SystemFonts.DefaultFont
        ofrmTextBox.txtTextBox.Select(Len(strOut), 0)
        ofrmTextBox.Show()
    End Sub

    Private Sub btStopSimulation_Click(sender As Object, e As EventArgs) Handles btStopSimulation.Click
        Dim msgYesOrNo As Microsoft.VisualBasic.MsgBoxResult
        msgYesOrNo = MsgBox("Do you want to stop running GRM?      " + vbCrLf _
                                  , MsgBoxStyle.Exclamation Or MsgBoxStyle.YesNo Or MsgBoxStyle.SystemModal)
        If msgYesOrNo = MsgBoxResult.Yes Then
            mSimulator.StopSimulation()
            Me.btStartGRMorApplySettings.Enabled = True
            btClose.Enabled = True
        End If
    End Sub

    Private Sub btClose_Click(sender As Object, e As EventArgs) Handles btClose.Click
        Me.Close()
    End Sub

    Private Delegate Sub CompleteSimulationActionDelegate()

    Private Sub mSimulator_SimulationComplete(sender As cSimulator) Handles mSimulator.SimulationComplete
        If Me.InvokeRequired Then
            Dim d As New CompleteSimulationActionDelegate(AddressOf SimulationCompletedAction)
            Me.Invoke(d)
        Else
            SimulationCompletedAction()
        End If
    End Sub

    Private Sub SimulationCompletedAction()
        MsgBox("Simulation was completed!!   ", MsgBoxStyle.ApplicationModal, cGRM.BuildInfo.ProductName)
        Me.btStartGRMorApplySettings.Enabled = True
        btClose.Enabled = True
        btStopSimulation.Enabled = False
    End Sub


    Private Delegate Sub StopSimulationActionDelegate()
    Private Sub mSimulator_SimulationStop(sender As cSimulator) Handles mSimulator.SimulationStop
        If Me.InvokeRequired Then
            Dim d As New StopSimulationActionDelegate(AddressOf StopSimulationAction)
            Me.Invoke(d)
        Else
            StopSimulationAction()
        End If
    End Sub

    Private Sub StopSimulationAction()
        btStopSimulation.Enabled = False
    End Sub

End Class
