Imports System.Windows.Forms.DataVisualization.Charting
Imports System.IO

Public Class cChart

    Private mChartMain As System.Windows.Forms.DataVisualization.Charting.Chart
    Private mChartAreaMain As ChartArea
    Private mChartRF As Windows.Forms.DataVisualization.Charting.Chart
    Private mChartAreaRF As ChartArea
    Private mPbChartMain As PictureBox
    Private mPbChartRF As PictureBox

    Private mSRf As Series
    Private mSQobs As List(Of Series)
    Private mSQsim As List(Of Series)
    Private mLegendsObs As List(Of String)
    Private mLegendsSim As List(Of String)
    Private mLabelsAxisX As List(Of String)

    Private mAxisXLabel As Dictionary(Of Integer, String)
    Private mRFs As Dictionary(Of Integer, Single)
    Private mQobss As Dictionary(Of Integer, List(Of Single))
    Private mQsims As Dictionary(Of Integer, List(Of Single))

    Private mMaxValueInQsim As Single
    Private mMaxOutPrintCount As Integer

    Public mAddedValueCount As Integer = 0

    ''' <summary>
    ''' 강우, 관측유량, 계산유량을 챠팅하기 위한 생성자
    ''' 계산유량. 값이 채워져 있으면, 바로 챠팅 가능. 계산유량과 관측유량의 개수가 다르면.. x축 날자표시 안됨.
    ''' 그러나 값이 채워져 있지 않으면 AddPointAndUpdateChart 함수를 이용해서 값을 채워가면서 실시간으로 챠팅
    ''' 계산에 적용된 강우. 값이 채워져 있으면, 바로 챠팅 가능.
    ''' 그러나 값이 채워져 있지 않으면 AddPointAndUpdateChart 함수를 이용해서 값을 채워가면서 실시간으로 챠팅
    ''' </summary>
    ''' <param name="pbChartMain"> 유량 그래프를 위치시킬 픽쳐 박스</param>
    ''' <param name="pbChartRF"> 강우 그래프를 위치시킬 픽쳐 박스</param>
    ''' <param name="inQobs">관측유량. Dictionary 값이 모두 채워져 있어야 함</param>
    ''' <remarks></remarks>
    ''' 
    Public Sub New(ByVal pbChartMain As PictureBox, ByVal pbChartRF As PictureBox, _
                   ByVal inQobs As Dictionary(Of Integer, List(Of Single)), _
                   ByVal inLegendsObs As List(Of String), _
                   ByVal inLegendsSim As List(Of String), _
                   Optional ByVal maxPrintCount As Integer = 0)
        If inQobs Is Nothing OrElse inLegendsObs Is Nothing OrElse inQobs.Values.Count < 1 Then
            MsgBox("Observed data is not selected. ")
        End If

        If pbChartMain Is Nothing OrElse pbChartRF Is Nothing Then
            MsgBox("err")
            Exit Sub
        End If
        mMaxOutPrintCount = maxPrintCount
        Call initializeVariables(pbChartMain, pbChartRF, inQobs, inLegendsObs, inLegendsSim)
        setChartFormat()
    End Sub

    Public Sub New(ByVal pbChartMain As PictureBox, ByVal pbChartRF As PictureBox, _
               ByVal inQobs As Dictionary(Of Integer, List(Of Single)), _
               ByVal inLegendObs As List(Of String), _
               ByVal inLegendsSim As List(Of String), _
               ByVal inputXLabels As Dictionary(Of Integer, String), _
               Optional ByVal maxPrintCount As Integer = 0)

        If inQobs Is Nothing OrElse inLegendObs Is Nothing OrElse inQobs.Values.Count < 1 Then
            MsgBox("Observed data is not selected. ")
        End If

        If pbChartMain Is Nothing OrElse pbChartRF Is Nothing _
           OrElse inputXLabels Is Nothing Then
            MsgBox("err")
            Exit Sub
        End If
        mMaxOutPrintCount = maxPrintCount
        Call initializeVariables(pbChartMain, pbChartRF, inQobs, inLegendObs, inLegendsSim, , , inputXLabels)
        setChartFormat()
    End Sub


    Public Sub New(ByVal pbChartMain As PictureBox, ByVal pbChartRF As PictureBox, _
               ByVal inQobs As Dictionary(Of Integer, List(Of Single)), _
               ByVal inObsLegends As List(Of String), _
               ByVal inQsim As Dictionary(Of Integer, List(Of Single)), _
               ByVal inSimLegends As List(Of String), _
               ByVal inRF As Dictionary(Of Integer, Single), _
               Optional ByVal maxPrintCount As Integer = 0)

        If inQobs Is Nothing OrElse inObsLegends Is Nothing OrElse inQobs.Values.Count < 1 Then
            MsgBox("Observed data is not selected. ")
        End If

        If pbChartMain Is Nothing OrElse pbChartRF Is Nothing _
          OrElse inQsim Is Nothing OrElse inSimLegends Is Nothing _
          OrElse inRF Is Nothing Then
            MsgBox("err")
            Exit Sub
        End If

        If inQobs.Count <> inQsim.Count OrElse inQobs.Count <> inRF.Count Then
            MsgBox(" err")
            Exit Sub
        End If
        mMaxOutPrintCount = maxPrintCount
        Call initializeVariables(pbChartMain, pbChartRF, inQobs, inObsLegends, inSimLegends, , )

        mQsims = inQsim
        mRFs = inRF
        mLegendsSim = inSimLegends
        setChartFormat()
    End Sub

    Public Sub New(ByVal pbChartMain As PictureBox, ByVal pbChartRF As PictureBox, _
           ByVal inQobs As Dictionary(Of Integer, List(Of Single)), _
           ByVal inObsLegends As List(Of String), _
           ByVal inQsim As Dictionary(Of Integer, List(Of Single)), _
           ByVal inSimLegends As List(Of String), _
           ByVal inRF As Dictionary(Of Integer, Single), _
           ByVal inXLabels As Dictionary(Of Integer, String), _
           Optional ByVal maxPrintCount As Integer = 0)

        If inQobs Is Nothing OrElse inObsLegends Is Nothing Then
            MsgBox("Observed data is not selected. ")
        End If

        If pbChartMain Is Nothing OrElse pbChartRF Is Nothing _
          OrElse inQsim Is Nothing OrElse inSimLegends Is Nothing _
          OrElse inRF Is Nothing _
          OrElse inXLabels Is Nothing Then
            MsgBox("err")
            Exit Sub
        End If

        If inQobs.Values.Count < 1 Then
            MsgBox("err")
            Exit Sub
        End If

        If inQobs.Count <> inQsim.Count OrElse inQobs.Count <> inRF.Count Then
            inXLabels = Nothing
        End If
        mMaxOutPrintCount = maxPrintCount
        Call initializeVariables(pbChartMain, pbChartRF, inQobs, inObsLegends, inSimLegends, inQsim, inRF, inXLabels)

        'mQsims = inQsim
        'mRFs = inRF
        'mLegendsSim = inSimLegends
        setChartFormat()
    End Sub



    Private Sub initializeVariables(ByVal pbChartMain As PictureBox, ByVal pbChartRF As PictureBox, _
                                    ByVal inQobs As Dictionary(Of Integer, List(Of Single)), ByVal inObsLegends As List(Of String), ByVal inSimLegends As List(Of String), _
                                    Optional ByVal inQsims As Dictionary(Of Integer, List(Of Single)) = Nothing, _
                                    Optional ByVal inRF As Dictionary(Of Integer, Single) = Nothing, Optional ByVal inXLabels As Dictionary(Of Integer, String) = Nothing)

        mChartMain = New Windows.Forms.DataVisualization.Charting.Chart
        mChartAreaMain = New ChartArea
        mChartRF = New Windows.Forms.DataVisualization.Charting.Chart
        mChartAreaRF = New ChartArea

        mRFs = New Dictionary(Of Integer, Single)
        mQobss = New Dictionary(Of Integer, List(Of Single))
        mQsims = New Dictionary(Of Integer, List(Of Single))
        mAxisXLabel = New Dictionary(Of Integer, String)

        mSRf = New Series
        mSQobs = New List(Of Series)
        mSQsim = New List(Of Series)

        mLegendsObs = New List(Of String)
        mLegendsSim = New List(Of String)

        mPbChartMain = pbChartMain
        mPbChartRF = pbChartRF
        mQobss = inQobs
        If Not inQsims Is Nothing Then mQsims = inQsims
        mLegendsObs = inObsLegends
        mLegendsSim = inSimLegends


        If Not inSimLegends Is Nothing Then mLegendsSim = inSimLegends

        Dim maxCountX As Integer = mQobss.Count
        If mQsims IsNot Nothing AndAlso mQsims.Count > mQobss.Count Then
            maxCountX = mQsims.Count
        End If
        If maxCountX < mMaxOutPrintCount Then
            maxCountX = mMaxOutPrintCount
        End If
        If Not inRF Is Nothing Then
            For n As Integer = 0 To maxCountX - 1
                If n <= (inRF.Count - 1) Then
                    mRFs.Add(n, inRF(n))
                Else
                    mRFs.Add(n, 0)
                End If
            Next
        End If



        If Not inXLabels Is Nothing AndAlso inXLabels.Count > 1 Then
            mAxisXLabel = inXLabels
        Else
            'x 라벨이 설정되지 않을 경우, 키 값을 x 축 라벨로 한다.
            For n As Integer = 0 To mMaxOutPrintCount - 1 ' mQobss.Count - 1
                mAxisXLabel.Add(n, CStr(n))
            Next
        End If

        For n As Integer = 0 To SeriesCountInAClassOBS - 1
            mSQobs.Add(New Series)
        Next

        For n As Integer = 0 To SeriesCountInAClassSIM - 1
            mSQsim.Add(New Series)
        Next

        Call GenerateDictionaryKeys()
        mAddedValueCount = 0
    End Sub

    Private Sub GenerateDictionaryKeys()
        Dim addQsim As Boolean = False
        Dim addRFs As Boolean = False
        Dim addXlabel As Boolean = False
        Dim totalKeyNum As Integer
        If mQsims.Count = 0 Then addQsim = True
        If mRFs.Count = 0 Then addRFs = True
        If mAxisXLabel.Count = 0 Then addXlabel = True
        If mQobss Is Nothing OrElse mQobss.Count < 1 OrElse mMaxOutPrintCount > mQobss.Count Then
            totalKeyNum = mMaxOutPrintCount
        Else
            totalKeyNum = mQobss.Count '이거 키는 0부터 시작
        End If
        For k As Integer = 0 To totalKeyNum - 1
            If addQsim = True Then mQsims.Add(k, Nothing)
            If addRFs = True Then mRFs.Add(k, Nothing)
            If addXlabel = True Then mAxisXLabel.Add(k, Nothing)
        Next
    End Sub

    Public Sub UpdateLegendSim(ByVal inSimLegends As List(Of String))
        mLegendsSim = inSimLegends
    End Sub


    Public Shared Function GenerateDictionaryKeyWithObservedDataDictionary(ByVal ReferenceQobsDic As Dictionary(Of Integer, List(Of Single)), _
                                     ByVal inDicToSet As Dictionary(Of Integer, List(Of Single))) As Dictionary(Of Integer, List(Of Single))
        For Each n As Integer In ReferenceQobsDic.Keys
            inDicToSet.Add(n, Nothing)
        Next
        Return inDicToSet
    End Function


    Public Shared Function GenerateDictionaryKeyWithObservedDataDictionary(ByVal ReferenceQobsDic As Dictionary(Of Integer, List(Of Single)), _
                                 ByVal inDicToSet As Dictionary(Of Integer, String)) As Dictionary(Of Integer, String)
        For Each n As Integer In ReferenceQobsDic.Keys
            inDicToSet.Add(n, Nothing)
        Next
        Return inDicToSet
    End Function

    Public Shared Function GenerateDictionaryKeyWithDataCount(totalCount As Integer, _
                             ByVal inDicToSet As Dictionary(Of Integer, String)) As Dictionary(Of Integer, String)
        For n As Integer = 0 To totalCount - 1
            inDicToSet.Add(n, Nothing) 'key는 0번 부터
        Next
        Return inDicToSet
    End Function

    Public Function DrawChart() As Boolean
        Try
            'mSRf.Points.DataBindXY(mRFs.Keys, mRFs.Values)
            'mSQsim.Points.DataBindXY(mQsims.Keys, mQsims.Values)

            'If Not mRFs(mRFs.Keys(0)) = Nothing Then
            mSRf.Points.DataBindXY(mAxisXLabel.Values, mRFs.Values)
            'End If

            For n As Integer = 0 To SeriesCountInAClassOBS - 1
                Dim valuesObs As New List(Of Single)
                For m As Integer = 0 To mMaxOutPrintCount - 1 'mQobss.Count - 1 '키는 0부터 시작
                    If m < mQobss.Count Then
                        If mQobss(m) IsNot Nothing Then
                            valuesObs.Add(mQobss(m)(n))
                        Else
                            valuesObs.Add(vbNull)
                        End If
                    Else
                        valuesObs.Add(vbNull)
                    End If
                Next

                If valuesObs IsNot Nothing Then
                    If n = 0 Then
                        mSQobs(n).Points.DataBindXY(mAxisXLabel.Values, valuesObs)
                    Else
                        mSQobs(n).Points.DataBindY(valuesObs)
                    End If
                End If
            Next

            For n As Integer = 0 To SeriesCountInAClassSIM - 1
                Dim valuesSim As New List(Of Single)
                For m As Integer = 0 To mQsims.Count - 1 '키는 0부터 시작
                    If mQsims(m) IsNot Nothing Then valuesSim.Add(mQsims(m)(n))
                Next
                'For m As Integer = valuesSim.Count To mMaxOutPrintCount - 1 'mQsims.Count - 1 '키는 0부터 시작
                '    valuesSim.Add(Nothing)
                'Next
                'mSQsim(n).Points.DataBindXY(mAxisXLabel.Values, valuesSim)
                Dim axisXLabelSim As New Dictionary(Of Integer, String)
                For nl As Integer = 0 To valuesSim.Count - 1
                    axisXLabelSim.Add(nl, mAxisXLabel.Values(nl))
                Next
                mSQsim(n).Points.DataBindXY(axisXLabelSim.Values, valuesSim)

                If valuesSim IsNot Nothing Then
                    mSQsim(n).Points.DataBindY(valuesSim)
                End If
            Next

            With mChartMain
                .Location = New System.Drawing.Point(0, 0)
                .Size = New System.Drawing.Size(mPbChartMain.Width, mPbChartMain.Height)
                .ChartAreas.Add(mChartAreaMain)
                For n As Integer = 0 To SeriesCountInAClassOBS - 1
                    .Series.Add(mSQobs(n))
                Next
                For n As Integer = 0 To SeriesCountInAClassSIM - 1
                    .Series.Add(mSQsim(n))
                Next

                .AntiAliasing = AntiAliasingStyles.All
                .Legends.Add(New Legend("MainChartLegend"))
            End With

            With mChartMain.Legends("MainChartLegend")
                .BorderColor = Color.DarkGray
                .Docking = Docking.Right
                .LegendStyle = LegendStyle.Column
                .Alignment = StringAlignment.Near
                .InsideChartArea = mChartAreaMain.Name
            End With

            With mChartRF
                .Location = New System.Drawing.Point(0, 0)
                .Size = New System.Drawing.Size(mPbChartRF.Width, mPbChartRF.Height)
                .ChartAreas.Add(mChartAreaRF)
                .Series.Add(mSRf)
                .AntiAliasing = AntiAliasingStyles.All
            End With

            mPbChartMain.Controls.Add(mChartMain)
            mPbChartRF.Controls.Add(mChartRF)
        Catch ex As Exception
            MsgBox("An error was occurred while drawing charts", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
        End Try
        Return True
    End Function

    Public Function AddQsimAndRFpointAndUpdateChart(ByVal RfToAdd As Single, ByVal QsimToAdd As List(Of Single)) As Boolean
        mAddedValueCount += 1
        'Dim kindexToBeAdded As Integer = GetKeyIndexTobeAdded(mRFs)
        Dim kindexToBeAdded As Integer = mAddedValueCount - 1

        'If kindexToBeAdded > mQobss.Count - 1 Then
        If kindexToBeAdded > mMaxOutPrintCount - 1 Then
            Return False
            Exit Function
        End If
        Dim keyToBeAdded As Integer = mRFs.Keys(kindexToBeAdded)
        mRFs(keyToBeAdded) = RfToAdd
        mSRf.Points.DataBindXY(mAxisXLabel.Values, mRFs.Values)
        mChartRF.Series(mSRf.Name) = mSRf
        mQsims(keyToBeAdded) = QsimToAdd

        For n As Integer = 0 To SeriesCountInAClassSIM - 1
            Dim l As New List(Of Single)
            For m As Integer = 0 To mAddedValueCount - 1
                l.Add(mQsims(m)(n))
            Next
            mSQsim(n).Points.DataBindY(l)
            mChartMain.Series(mSQsim(n).Name) = mSQsim(n)
        Next
        mChartRF.Update()
        mChartMain.Update()
        Return True
    End Function

    Public Function ClearChartingSeriesRFandQsim() As Boolean
        Dim oriC As Integer = mChartMain.Series.Count
        Dim nowC As Integer = mChartMain.Series.Count
        Dim nowAn As Integer = 0
        For n As Integer = 0 To oriC - 1
            For m As Integer = 0 To mSQsim.Count - 1
                If mChartMain.Series(nowAn).Name = mSQsim(m).Name Then
                    mChartMain.Series.RemoveAt(nowAn)
                    nowC = nowC - 1
                    'nowAn = nowAn - 1

                End If
            Next
            If mChartMain.Series.Count <= nowAn Then Exit For
            nowAn = nowAn + 1
        Next

        mRFs = New Dictionary(Of Integer, Single)
        mSRf = Nothing
        mSRf = New Series
        Call SetRFSeriesStyle()
        mChartRF.Series(mSRf.Name) = mSRf
        mChartRF.Update()

        mQsims = New Dictionary(Of Integer, List(Of Single))
        mSQsim = New List(Of Series)
        For n As Integer = 0 To SeriesCountInAClassSIM - 1
            mSQsim.Add(New Series)
            'mSQsim(n) = Nothing
            'mSQsim(n) = New Series
        Next
        Call SetQSimulatedSeriesStyle()
        For n As Integer = 0 To SeriesCountInAClassSIM - 1
            mChartMain.Series(mSQsim(n).Name) = mSQsim(n)
        Next
        mChartMain.Update()
        GenerateDictionaryKeys()
        Return True
    End Function


    Private Function GetKeyIndexTobeAdded(ByVal SeriesToBeAdd As Dictionary(Of Integer, Single)) As Integer
        For n As Integer = 0 To SeriesToBeAdd.Keys.Count - 1
            If SeriesToBeAdd(SeriesToBeAdd.Keys(n)) = Nothing Then
                Return n
                Exit For
            End If
        Next
        Return -9999
    End Function


    Private Function setChartFormat() As Boolean
        With mChartAreaMain
            .Name = "MainChart"
            .AxisX.Title = "Time"
            .AxisX.IsStartedFromZero = True
            .AxisX.Minimum = 0
            .AxisX.Crossing = .AxisY.Minimum
            .AxisX.MajorGrid.LineDashStyle = ChartDashStyle.Dot
            .AxisX.IsLabelAutoFit = False
            .AxisX.LabelStyle.IsStaggered = False
            '.AxisX.IsLabelAutoFit = True
            '.AxisX.LabelStyle.Font = New Font("Arial", 10)


            .AxisY.Title = "Discharge (㎥/s)"
            .AxisY.TitleAlignment = StringAlignment.Center
            .AxisY.IsStartedFromZero = True
            .AxisY.IsReversed = False
            .AxisY.Crossing = 0
            .AxisY.Minimum = 0
            '.AxisY.Maximum = Double.NaN
            '.AxisY.IsLabelAutoFit = True
            '.AxisY.LabelStyle.Font = New Font("Arial", 10)
            .AxisY.MajorGrid.LineDashStyle = ChartDashStyle.Dot
            .AxisY.IsLabelAutoFit = False
            .AxisY.LabelStyle.IsStaggered = False

            '.AxisY2.IsStartedFromZero = True
            '.AxisY2.IsReversed = True
            '.AxisY2.Minimum = 0
            '.AxisY2.MajorGrid.Enabled = False
            '.AxisY2.MinorGrid.Enabled = False
            '.Area3DStyle.Enable3D = True

            .BorderDashStyle = ChartDashStyle.Solid
            .IsSameFontSizeForAllAxes = True

            .InnerPlotPosition.Auto = False
            .InnerPlotPosition.X = 11
            .InnerPlotPosition.Y = 0
            .InnerPlotPosition.Width = 84.5
            .InnerPlotPosition.Height = 80
        End With

        With mChartAreaRF
            .Name = "RainfallChart"
            .AxisX.IsStartedFromZero = True
            .AxisX.Minimum = 0
            .AxisX.Crossing = .AxisY.Minimum
            .AxisX.MajorGrid.LineDashStyle = ChartDashStyle.Dot
            .AxisX.LabelStyle.Enabled = False
            .AxisX.IsLabelAutoFit = False
            .AxisX.LabelStyle.IsStaggered = False
            '.AxisX.LabelStyle.ForeColor = Color.White
            '.AxisX.IsLabelAutoFit = False
            '.AxisX.LabelStyle.Font = New Font(.AxisX.LabelStyle.Font, 10)

            .AxisY.Title = "RF(mm)"
            .AxisY.TitleAlignment = StringAlignment.Center
            .AxisY.IsStartedFromZero = True
            .AxisY.IsReversed = False
            .AxisY.Crossing = 0
            .AxisY.Minimum = 0
            '.AxisY.Maximum = Double.NaN
            .AxisY.MajorGrid.LineDashStyle = ChartDashStyle.Dot
            .AxisY.IsLabelAutoFit = False
            .AxisY.LabelStyle.IsStaggered = False
            '.AxisY.IsLabelAutoFit = False
            '.AxisY.LabelStyle.Font = New Font(.AxisX.LabelStyle.Font, 10)

            .BorderDashStyle = ChartDashStyle.Solid
            .IsSameFontSizeForAllAxes = True

            .InnerPlotPosition.Auto = False
            .InnerPlotPosition.X = 11
            .InnerPlotPosition.Y = 5
            .InnerPlotPosition.Width = 84.5
            .InnerPlotPosition.Height = 85
        End With

        Call SetRFSeriesStyle()
        Call SetQObservedSeriesStyle()
        Call SetQSimulatedSeriesStyle()
        Return True
    End Function


    Private Sub SetRFSeriesStyle()
        With mSRf
            .Name = "RF"
            .ChartType = SeriesChartType.Column
            .ChartArea = mChartAreaRF.Name
            '.YAxisType = AxisType.Secondary
            .YAxisType = AxisType.Primary
        End With
    End Sub

    Private Sub SetQObservedSeriesStyle()
        Dim rand As New Random
        For n As Integer = 0 To SeriesCountInAClassOBS - 1
            'Randomize()
            With mSQobs(n)
                .Name = mLegendsObs(n) + "(Obs)"
                .ChartType = SeriesChartType.Point
                .MarkerStyle = MarkerStyle.Circle
                .MarkerColor = Color.White
                For m As Integer = 0 To 10000
                    .MarkerBorderColor = Color.FromArgb(rand.Next(0, 256), rand.Next(0, 256), rand.Next(0, 256))
                    If .MarkerBorderColor <> Color.FromArgb(255, 255, 255) Then Exit For
                Next

                If .MarkerBorderColor = Color.FromArgb(255, 255, 255) Then
                    MsgBox("Observed data color is white. Draw again. ", MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                    Exit For
                End If

                .ChartArea = mChartAreaMain.Name
                .YAxisType = AxisType.Primary
            End With
        Next
    End Sub

    Private Sub SetQSimulatedSeriesStyle()
        For n As Integer = 0 To SeriesCountInAClassSIM - 1
            With mSQsim(n)
                .Name = mLegendsSim(n) + "(Sim)"
                .ChartType = SeriesChartType.Line
                .MarkerStyle = MarkerStyle.None '.Circle
                .BorderWidth = 2
                .ChartArea = mChartAreaMain.Name
                .YAxisType = AxisType.Primary
            End With
        Next
    End Sub


    Public Shared Function ReadObservedDataAndSetDictionary(ByVal obsFPN As String) As Dictionary(Of Integer, List(Of Single))
        Dim strLines() As String = System.IO.File.ReadAllLines(obsFPN)
        Dim qObs As New Dictionary(Of Integer, List(Of Single))
        For n As Integer = 1 To strLines.Length - 1 '0번은 레전드 이므로.. 스킵
            Dim LineParts() As String = strLines(n).Split(New String() {" ", ","}, StringSplitOptions.RemoveEmptyEntries)
            If LineParts Is Nothing OrElse LineParts.Length < 1 Then Exit For
            Dim v As Single
            Dim vAline As New List(Of Single)
            'If LineParts IsNot Nothing  Then
            For Each strV As String In LineParts
                If Single.TryParse(strV, v) Then
                    vAline.Add(v)
                Else
                    MsgBox("Observed data is invalid.  " + vbCrLf + _
                    "Check the observed data file.    " + vbCrLf + _
                    obsFPN, MsgBoxStyle.Exclamation, cGRM.BuildInfo.ProductName)
                End If
            Next
            'End If
            qObs.Add(n - 1, vAline) 'key는 0번 부터 
        Next
        Return qObs
    End Function

    Public Shared Function ReadObservedDataAndGetLegends(ByVal obsFPN As String) As List(Of String)
        Dim reader As New StreamReader(obsFPN, System.Text.Encoding.Default)
        ' 첫 행은 전체 관측지점 이름, 전체 몇 개의 관측소에서 자료를 읽을 것인지 설정 
        Dim strLine As String = reader.ReadLine()
        Dim LineParts() As String = strLine.Split(New String() {" ", ","}, StringSplitOptions.RemoveEmptyEntries)
        Return LineParts.ToList()
    End Function

    Public ReadOnly Property MaxCountOfSimulatedData() As Integer
        Get
            If mQsims IsNot Nothing Then
                Return mQsims.Count
            Else
                If mQobss IsNot Nothing Then Return mQobss.Count
            End If
            Return 0
        End Get
    End Property

    Public ReadOnly Property MaxCountOfData() As Integer
        Get
            If mQobss IsNot Nothing Then
                If mQsims IsNot Nothing Then
                    If mQsims.Count > mQobss.Count Then
                        Return mQsims.Count
                    Else
                        Return mQobss.Count
                    End If
                Else
                    Return mQobss.Count
                End If
            End If
            Return 0
        End Get
    End Property



    Public ReadOnly Property SeriesCountInAClassOBS() As Integer
        Get
            Return mLegendsObs.Count
        End Get
    End Property
    Public ReadOnly Property SeriesCountInAClassSIM() As Integer
        Get
            Return mLegendsSim.Count
        End Get
    End Property

End Class


