Imports System.Text

Public Class cRainfall
    Public Enum RainfallDataType
        TextFileMAP
        TextFileASCgrid
        TextFileASCgrid_mmPhr
        'GridFiles
        'GridFilesRadar_mmPhr
        'UniformConstant_mm
    End Enum

    Public Structure RainfallData
        Dim Order As Integer
        Dim DataTime As String
        Dim Rainfall As String 'map 에서는 강우량 값, asc에서는 파일 이름
        Dim FilePath As String
        Dim FileName As String
    End Structure

    Public mRainfallDataType As Nullable(Of RainfallDataType)

    ''' <summary>
    ''' 강우의 시간간격 [minutes]
    ''' </summary>
    ''' <remarks></remarks>
    Public mRainfallinterval As Nullable(Of Integer)
    Public mlstRainfallData As List(Of RainfallData)
    Private mRainfallDataFilePathName As String = ""

    ''' <summary>
    ''' Delta T시간 동안의 유역평균강우량
    ''' </summary>
    ''' <remarks></remarks>
    Public mRFMeanForDt_m As Double

    Public Sub GetValues(ByVal prj As cProject)
        Dim row As GRMProject.ProjectSettingsRow = CType(prj.PrjFile.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        With row
            If Not .IsRainfallDataTypeNull Then
                Select Case .RainfallDataType
                    'Case RainfallDataType.GridFiles.ToString
                    '    mRainfallDataType = RainfallDataType.GridFiles
                    'Case RainfallDataType.GridFilesRadar_mmPhr.ToString
                    '    mRainfallDataType = RainfallDataType.GridFilesRadar_mmPhr
                    Case RainfallDataType.TextFileMAP.ToString
                        mRainfallDataType = RainfallDataType.TextFileMAP
                    Case RainfallDataType.TextFileASCgrid.ToString
                        mRainfallDataType = RainfallDataType.TextFileASCgrid
                    Case RainfallDataType.TextFileASCgrid_mmPhr.ToString
                        mRainfallDataType = RainfallDataType.TextFileASCgrid_mmPhr
                End Select
                mRainfallinterval = CInt(.RainfallInterval)
            End If
            mRainfallDataFilePathName = row.RainfallDataFile
        End With
        If mRainfallDataType.HasValue Then
            mlstRainfallData = New List(Of RainfallData)
            Dim Lines() As String = System.IO.File.ReadAllLines(mRainfallDataFilePathName)
            For n As Integer = 0 To Lines.Length - 1
                If Lines(n).Trim = "" Then Exit For
                Dim r As New RainfallData
                r.Order = n + 1
                Select Case row.RainfallDataType
                    Case RainfallDataType.TextFileASCgrid.ToString,
                           RainfallDataType.TextFileASCgrid_mmPhr.ToString
                        r.Rainfall = Path.GetFileName(Lines(n).ToString)
                        r.FileName = Path.GetFileName(Lines(n).ToString)
                        r.FilePath = Path.GetDirectoryName(Lines(n).ToString)
                    Case RainfallDataType.TextFileMAP.ToString
                        r.Rainfall = Lines(n).ToString
                        r.FileName = Path.GetFileName(mRainfallDataFilePathName)
                        r.FilePath = Path.GetDirectoryName(mRainfallDataFilePathName)
                End Select
                If prj.GeneralSimulEnv.mIsDateTimeFormat = True Then
                    r.DataTime = cComTools.GetTimeToPrintOut(True, prj.GeneralSimulEnv.mSimStartDateTime, CInt(mRainfallinterval * n))
                Else
                    r.DataTime = CStr(mRainfallinterval * n)
                End If
                mlstRainfallData.Add(r)
            Next
        End If
    End Sub

    'Public Sub SetValues(ByVal prjdb As GRMProject)
    '    Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
    '    If mRainfallDataType.HasValue Then
    '        With row
    '            .RainfallDataType = mRainfallDataType.ToString
    '            .RainfallInterval = mRainfallinterval.Value
    '            If mRFStartDateTime IsNot Nothing Then
    '                .RainfallStartsFrom = mRFStartDateTime
    '                .RainfallEndsAt = mRainfallEndDateTime
    '            Else
    '                .RainfallStartsFrom = Nothing
    '                .RainfallEndsAt = Nothing
    '            End If
    '            .RainfallDuration = RFDuration
    '            .RainfallDataFile = RFDataFilePathName
    '        End With
    '        mdtRainfallinfo.AcceptChanges()
    '        For Each r As DataRow In mdtRainfallinfo.Rows
    '            r.SetAdded()
    '        Next
    '    End If
    'End Sub

    Public Shared Sub ReadRainfall(project As cProject, ByVal eRainfallDataType As cRainfall.RainfallDataType,
                                       ByVal lstRFData As List(Of RainfallData),
                                       ByVal RFinterval_MIN As Integer, ByVal nowRFOrder As Integer, isparallel As Boolean)
        Dim rfIntervalSEC As Integer = RFinterval_MIN * 60
        Dim rfRow As RainfallData = project.Rainfall.GetRFdataByOrder(lstRFData, nowRFOrder)
        Dim RFfpn As String = Path.Combine(rfRow.FilePath, rfRow.FileName)
        Dim cellSize As Single = project.Watershed.mCellSize
        cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = 0
        For Each wpCVID As Integer In project.WatchPoint.WPCVidList
            project.WatchPoint.mRFReadIntensitySumUpWs_mPs(wpCVID) = 0
        Next
        Try
            Select Case eRainfallDataType
                Case cRainfall.RainfallDataType.TextFileASCgrid, cRainfall.RainfallDataType.TextFileASCgrid_mmPhr
                    Dim ascReader As New cTextFileReaderASC(RFfpn)
                    Dim rowCount As Integer = project.Watershed.mRowCount
                    Dim colCount As Integer = project.Watershed.mColCount
                    If isparallel = True Then
                        Dim options As ParallelOptions = New ParallelOptions()
                        options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
                        Parallel.For(0, rowCount, options, Sub(ry As Integer)
                                                               Dim RFs As String() = ascReader.ValuesInOneRowFromTopLeft(ry)
                                                               For cx As Integer = 0 To colCount - 1
                                                                   If project.WSCell(cx, ry) Is Nothing OrElse project.WSCell(cx, ry).toBeSimulated = False Then Continue For
                                                                   Dim cvan As Integer = project.WSCell(cx, ry).CVID - 1
                                                                   Dim inRF_mm As Single = CSng(RFs(cx))
                                                                   If eRainfallDataType = cRainfall.RainfallDataType.TextFileASCgrid_mmPhr Then
                                                                       inRF_mm = inRF_mm / CSng(60 / RFinterval_MIN)
                                                                   End If
                                                                   Call CalRFintensity_mPsec(project.CV(cvan), inRF_mm, rfIntervalSEC)
                                                               Next
                                                           End Sub)
                        For ry As Integer = 0 To rowCount - 1
                            For cx As Integer = 0 To colCount - 1
                                If project.WSCell(cx, ry) IsNot Nothing AndAlso project.WSCell(cx, ry).toBeSimulated = True Then
                                    cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs =
                                              cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.WSCell(cx, ry).RFReadintensity_mPsec
                                    Dim cvan As Integer = project.WSCell(cx, ry).CVID - 1
                                    Call CalRFSumForWPUpWSWithRFGrid(cvan)
                                End If
                            Next
                        Next
                    Else
                        For ry As Integer = 0 To rowCount - 1
                            Dim RFs As String() = ascReader.ValuesInOneRowFromTopLeft(ry)
                            For cx As Integer = 0 To colCount - 1
                                If project.WSCell(cx, ry) Is Nothing OrElse project.WSCell(cx, ry).toBeSimulated = False Then Continue For
                                Dim cvan As Integer = project.WSCell(cx, ry).CVID - 1
                                Dim inRF_mm As Single = CSng(RFs(cx))
                                If eRainfallDataType = cRainfall.RainfallDataType.TextFileASCgrid_mmPhr Then
                                    inRF_mm = inRF_mm / CSng(60 / RFinterval_MIN)
                                End If
                                Call CalRFintensity_mPsec(project.CV(cvan), inRF_mm, rfIntervalSEC)
                                cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs =
                                          cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.WSCell(cx, ry).RFReadintensity_mPsec
                                Call CalRFSumForWPUpWSWithRFGrid(cvan)
                            Next
                        Next
                    End If
                Case cRainfall.RainfallDataType.TextFileMAP
                    Dim inRF_mm As Single
                    If IsNumeric(rfRow.Rainfall) Then
                        inRF_mm = CSng(rfRow.Rainfall)
                    Else
                        System.Console.WriteLine("Error: Can not read rainfall value!!" & vbCrLf & "Order = " & nowRFOrder)
                        Exit Sub
                    End If
                    If inRF_mm < 0 Then inRF_mm = 0
                    For cvan As Integer = 0 To project.CVCount - 1
                        Call CalRFintensity_mPsec(project.CV(cvan), inRF_mm, rfIntervalSEC)
                        cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs =
                                              cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.CV(cvan).RFReadintensity_mPsec
                    Next
                    Call CalRFSumForWPUpWSWithMAPValue(project.CV(0).RFReadintensity_mPsec) '모든 격자의 강우량 동일하므로.. 하나를 던저준다.
                Case Else
                    System.Console.WriteLine("Error: Rainfall data type is invalid.")
            End Select
        Catch ex As Exception
            System.Console.WriteLine("An error was occurred while reading rainfall data.")
            cThisSimulation.mGRMSetupIsNormal = False
            Exit Sub
        End Try
    End Sub

    ''' <summary>
    ''' 강우에서 차단만 고려된 강우강도와 dt 시간동안의 강우량 계산. 2008.02.28 아직 차단 모듈은 반영되지 않음. 
    ''' 침투량이 고려된 것은 유효 강우량으로 입려됨.
    ''' </summary>
    ''' <param name="cv"></param>
    ''' <param name="rf_mm"></param>
    ''' <param name="rfIntevalSEC"></param>
    ''' <remarks></remarks>
    Public Shared Sub CalRFintensity_mPsec(ByVal cv As cCVAttribute, ByVal rf_mm As Single,
                                                             ByVal rfIntevalSEC As Integer)
        With cv
            If rf_mm <= 0 Then
                .RFReadintensity_mPsec = 0
            Else
                .RFReadintensity_mPsec = rf_mm / 1000 / rfIntevalSEC
            End If
        End With
    End Sub

    Public Shared Sub CalRF_mPdt(ByVal cv As cCVAttribute,
                                                              ByVal dtsec As Integer, ByVal cellSize As Single)
        Dim rf_mPs As Single = cv.RFReadintensity_mPsec
        With cv
            If rf_mPs = 0 Then
                .RFApp_dt_meter = 0
            Else
                .RFApp_dt_meter = rf_mPs * dtsec * (cellSize / .CVDeltaX_m)
            End If
        End With
    End Sub


    Public Shared Sub CalRFSumForWPUpWSWithRFGrid(ByVal cvan As Integer)
        For Each wpCVid As Integer In cProject.Current.CV(cvan).DownStreamWPCVids
            With cProject.Current
                .WatchPoint.mRFReadIntensitySumUpWs_mPs(wpCVid) =
                    .WatchPoint.mRFReadIntensitySumUpWs_mPs(wpCVid) + .CV(cvan).RFReadintensity_mPsec
            End With
        Next
    End Sub

    Public Shared Sub CalRFSumForWPUpWSWithMAPValue(ByVal ConstRFintensity_mPs As Single)
        For Each wpCVid As Integer In cProject.Current.WatchPoint.WPCVidList
            cProject.Current.WatchPoint.mRFReadIntensitySumUpWs_mPs(wpCVid) =
                ConstRFintensity_mPs * (cProject.Current.CV(wpCVid - 1).FAc + (1 - cProject.Current.FacMin))
        Next
    End Sub

    ''' <summary>
    ''' 모든 검사체적의 강우량을 0으로 설정
    ''' </summary>
    ''' <remarks>강우 지속시간이 지난 시점 부터는 강우강도와 dt 동안의 강우량은 0으로 세팅한다.</remarks>
    Public Shared Sub SetRainfallintensity_mPsec_And_Rainfall_dt_meter_Zero(ByVal project As cProject)
        Dim intCVTotNumber As Integer = project.CVCount
        Dim intWPTotCount As Integer = project.WatchPoint.WPCount
        For cvan As Integer = 0 To intCVTotNumber - 1
            With project.CV(cvan)
                .RFReadintensity_mPsec = 0
                .RFApp_dt_meter = 0
            End With
        Next cvan
        cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = 0
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            With project.WatchPoint
                .mRFWPGridForDtPrintout_mm(wpcvid) = 0
                .mRFUpWsMeanForDt_mm(wpcvid) = 0
                .mRFReadIntensitySumUpWs_mPs(wpcvid) = 0
                .mRFUpWsMeanForDtPrintout_mm(wpcvid) = 0
            End With
        Next
    End Sub

    Public Shared Sub CalCumulativeRFDuringDTPrintOut(project As cProject, ByVal dtsec As Integer)
        cThisSimulation.mRFMeanForDT_m = (cThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs * dtsec) / cProject.Current.CVCount
        cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = cThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m + cThisSimulation.mRFMeanForDT_m
        For Each wpcvid As Integer In project.WatchPoint.WPCVidList
            Dim fac As Integer = project.CV(wpcvid - 1).FAc
            With project.WatchPoint
                .mRFUpWsMeanForDt_mm(wpcvid) = (.mRFReadIntensitySumUpWs_mPs(wpcvid) * dtsec * 1000) / (fac + 1)
                .mRFUpWsMeanForDtPrintout_mm(wpcvid) =
                                .mRFUpWsMeanForDtPrintout_mm(wpcvid) + .mRFUpWsMeanForDt_mm(wpcvid)
                .mRFWPGridForDtPrintout_mm(wpcvid) = .mRFWPGridForDtPrintout_mm(wpcvid) _
                                    + cProject.Current.CV(wpcvid - 1).RFReadintensity_mPsec * 1000 * dtsec
            End With
        Next
        If project.GeneralSimulEnv.mbCreateASCFile = True _
            OrElse project.GeneralSimulEnv.mbCreateImageFile = True Then
            If project.GeneralSimulEnv.mbShowRFdistribution = True _
            OrElse project.GeneralSimulEnv.mbShowRFaccDistribution = True Then
                With project
                    For cvan As Integer = 0 To project.CVCount - 1
                        .CV(cvan).RF_dtPrintOut_meter = .CV(cvan).RF_dtPrintOut_meter _
                                            + .CV(cvan).RFReadintensity_mPsec * dtsec
                        .CV(cvan).RFAcc_FromStartToNow_meter = .CV(cvan).RFAcc_FromStartToNow_meter _
                             + .CV(cvan).RFReadintensity_mPsec * dtsec
                    Next
                End With
            End If
        End If
    End Sub


    ''' <summary>
    ''' 강우의 지속기간[minute]
    ''' </summary>
    ''' <value></value>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public ReadOnly Property RFDuration() As Integer
        Get
            If IsSet Then
                Return mRainfallinterval.Value * mlstRainfallData.Count
            Else
                Throw New InvalidOperationException
            End If
        End Get
    End Property

    Public ReadOnly Property IsSet() As Boolean
        Get
            Return mRainfallDataType.HasValue
        End Get
    End Property


    Public ReadOnly Property RFDataPathFirst() As String
        Get
            If mlstRainfallData.Count > 0 Then
                Return mlstRainfallData(0).FilePath
            Else
                Return Nothing
            End If
        End Get
    End Property

    Public ReadOnly Property RFDataFilePathName As String
        Get
            Return mRainfallDataFilePathName
        End Get
    End Property

    Public Function GetRFdataByOrder(ByVal dtRFinfo As List(Of RainfallData), ByVal order As Integer) As RainfallData
        If dtRFinfo.Count = 0 Then
            System.Console.WriteLine("Error: Rainfall data is not exist.   " & vbCrLf & "Order = " & order.ToString)
            Return Nothing
        End If
        For n As Integer = 0 To dtRFinfo.Count
            If dtRFinfo(n).Order = order Then
                Return dtRFinfo(n)
            End If
        Next
        Return Nothing
    End Function

    Public ReadOnly Property RFIntervalSEC() As Integer
        Get
            Return CInt(mRainfallinterval * 60)
        End Get
    End Property
End Class
