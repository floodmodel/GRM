Imports System.Threading


Public Class cGRMAnalyzer
    'Private mfAnalyzer As fAnalyzer
    Private mProject As cProject
    Private mColCount As Integer
    Private mRowCount As Integer
    Private mASCHeaderStringAll As String
    Private mArraySSR As Double(,)
    Private mArrayRF As Double(,)
    Private mArrayRFAcc As Double(,)
    Private mArrayQ As Double(,)
    'Private mDistributionRenderer As cDistribution.RendererType
    Private mIMGfpn As String
    Private mASCfpnSSRD As String
    Private mASCfpnRFD As String
    Private mASCfpnRFaccD As String
    Private mASCfpnFlowD As String
    Private mImgWidth As Integer
    Private mImgHeight As Integer

    Private mbMakeArySSR As Boolean = False
    Private mbMakeAryRF As Boolean = False
    Private mbMakeAryRFAcc As Boolean = False
    Private mbMakeAryQ As Boolean = False

    Private mbMakeImgFile As Boolean = False
    Private mbMakeASCFile As Boolean = False
    Private mbMakeValueAry As Boolean = False


    Sub New(ByVal project As cProject)
        'mfAnalyzer = dlgAnalyzer
        mProject = project
        mImgHeight = 433
        mImgWidth = 583
        mColCount = mProject.Watershed.mColCount
        mRowCount = mProject.Watershed.mRowCount
        mASCHeaderStringAll = cTextFile.MakeHeaderString(mColCount, mRowCount,
                                                 mProject.Watershed.mxllcorner, mProject.Watershed.myllcorner, mProject.Watershed.mCellSize, "-9999")
        mbMakeImgFile = mProject.GeneralSimulEnv.mbCreateImageFile
        mbMakeASCFile = mProject.GeneralSimulEnv.mbCreateASCFile
        mbMakeValueAry = False
        If mProject.GeneralSimulEnv.mbShowSoilSaturation = True AndAlso
           (mbMakeImgFile = True OrElse mbMakeASCFile = True) Then
            mbMakeArySSR = True
            mbMakeValueAry = True
        End If
        If mProject.GeneralSimulEnv.mbShowRFdistribution = True AndAlso
           (mbMakeImgFile = True OrElse mbMakeASCFile = True) Then
            mbMakeAryRF = True
            mbMakeValueAry = True
        End If
        If mProject.GeneralSimulEnv.mbShowRFaccDistribution = True AndAlso
           (mbMakeImgFile = True OrElse mbMakeASCFile = True) Then
            mbMakeAryRFAcc = True
            mbMakeValueAry = True
        End If
        If mProject.GeneralSimulEnv.mbShowFlowDistribution = True AndAlso
           (mbMakeImgFile = True OrElse mbMakeASCFile = True) Then
            mbMakeAryQ = True
            mbMakeValueAry = True
        End If
    End Sub

    Public Sub CreateDistributionFiles(ByVal nowT_MIN As Integer, picWidth As Single, picHeight As Single)
        Try
            Dim strNowTimeToPrintOut As String
            strNowTimeToPrintOut = cComTools.GetTimeToPrintOut(mProject.GeneralSimulEnv.mIsDateTimeFormat, mProject.GeneralSimulEnv.mSimStartDateTime, nowT_MIN)
            strNowTimeToPrintOut = cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut)

            If mbMakeValueAry = True Then
                If mbMakeArySSR = True Then mArraySSR = New Double(mColCount - 1, mRowCount - 1) {}
                If mbMakeAryRF = True Then mArrayRF = New Double(mColCount - 1, mRowCount - 1) {}
                If mbMakeAryRFAcc = True Then mArrayRFAcc = New Double(mColCount - 1, mRowCount - 1) {}
                If mbMakeAryQ = True Then mArrayQ = New Double(mColCount - 1, mRowCount - 1) {}
                GetStringArrayUsingCVAttribute(mProject.WSCells)
            End If

            If mbMakeArySSR = True Then
                If Directory.Exists(mProject.OFPSSRDistribution) = False Then Exit Sub
                'mStringArraySSR = GetStringArrayUsingCVAttribute(mProject.wsCells)
                If mbMakeImgFile = True Then
                    mIMGfpn = Path.Combine(mProject.OFPSSRDistribution, cGRM.CONST_DIST_SSR_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
                    imgMaker.MakeImgFileUsingArrayFromTL(mIMGfpn, mArraySSR,
                                                               picWidth, picHeight, cImg.RendererRange.RendererFrom0to1)
                End If
                If mbMakeASCFile = True Then
                    mASCfpnSSRD = Path.Combine(mProject.OFPSSRDistribution, cGRM.CONST_DIST_SSR_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                    StartCreateASCTextFileSSRD()
                    End If
                End If

                If mbMakeAryRF = True Then
                    If Directory.Exists(mProject.OFPRFDistribution) = False Then Exit Sub
                    'mStringArrayRF = GetStringArrayUsingCVAttribute(mProject.wsCells, cGRM.GRMOutputType.Rainfall)
                    If mbMakeImgFile = True Then
                        mIMGfpn = Path.Combine(mProject.OFPRFDistribution, cGRM.CONST_DIST_RF_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
                    imgMaker.MakeImgFileUsingArrayFromTL(mIMGfpn, mArrayRF,
                                                               picWidth, picHeight, cImg.RendererRange.RendererFrom0to500)
                End If
                    If mbMakeASCFile = True Then
                        mASCfpnRFD = Path.Combine(mProject.OFPRFDistribution, cGRM.CONST_DIST_RF_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                        StartCreateASCTextFileRFD()
                    End If
                End If

                If mbMakeAryRFAcc = True Then
                    If Directory.Exists(mProject.OFPRFAccDistribution) = False Then Exit Sub
                    'mStringArrayRFAcc = GetStringArrayUsingCVAttribute(mProject.wsCells, cGRM.GRMOutputType.RainfallCumulative)
                    If mbMakeImgFile = True Then
                        mIMGfpn = Path.Combine(mProject.OFPRFAccDistribution, cGRM.CONST_DIST_RFACC_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
                    imgMaker.MakeImgFileUsingArrayFromTL(mIMGfpn, mArrayRFAcc,
                                                               picWidth, picHeight, cImg.RendererRange.RendererFrom0to1000)
                End If
                    If mbMakeASCFile = True Then
                        mASCfpnRFaccD = Path.Combine(mProject.OFPRFAccDistribution, cGRM.CONST_DIST_RFACC_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                        StartCreateASCTextFileRFaccD()
                    End If
                End If

                If mbMakeAryQ = True Then
                    If Directory.Exists(mProject.OFPFlowDistribution) = False Then Exit Sub
                    'mStringArrayQ = GetStringArrayUsingCVAttribute(mProject.wsCells, cGRM.GRMOutputType.Discharge)
                    If mbMakeImgFile = True Then
                        mIMGfpn = Path.Combine(mProject.OFPFlowDistribution, cGRM.CONST_DIST_FLOW_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
                    imgMaker.MakeImgFileUsingArrayFromTL(mIMGfpn, mArrayQ,
                                                               picWidth, picHeight, cImg.RendererRange.RendererFrom0to30000)
                End If
                    If mbMakeASCFile = True Then
                        mASCfpnFlowD = Path.Combine(mProject.OFPFlowDistribution, cGRM.CONST_DIST_FLOW_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                        StartCreateASCTextFileFlowD()
                    End If
                End If
        Catch ex As Exception
            Console.WriteLine(ex.ToString + "   " + cGRM.BuildInfo.ProductName)
        End Try
    End Sub


    Private Function GetStringArrayUsingCVAttribute(inCells As cCVAttribute(,)) As Boolean
        Dim sformat As String = ""
        For nr As Integer = 0 To inCells.GetLength(1) - 1
            For nc As Integer = 0 To inCells.GetLength(0) - 1
                If inCells(nc, nr) IsNot Nothing Then
                    If inCells(nc, nr).toBeSimulated = True Then
                        If mbMakeArySSR = True Then
                            Dim v As Single = inCells(nc, nr).soilSaturationRatio
                            sformat = "#0.##"
                            mArraySSR(nc, nr) = CDbl(Format(v, sformat))
                        End If
                        If mbMakeAryRF = True Then
                            sformat = "#0.##"
                            Dim v As Single = (inCells(nc, nr).RF_dtPrintOut_meter * 1000)
                            mArrayRF(nc, nr) = CDbl(Format(v, sformat))
                        End If
                        If mbMakeAryRFAcc = True Then
                            sformat = "#0.##"
                            Dim v As Single = (inCells(nc, nr).RFAcc_FromStartToNow_meter * 1000)
                            mArrayRFAcc(nc, nr) = CDbl(Format(v, sformat))
                        End If
                        If mbMakeAryQ = True Then
                            sformat = "#0.##"
                            Dim v As Single
                            If inCells(nc, nr).FlowType = cGRM.CellFlowType.OverlandFlow Then
                                v = inCells(nc, nr).QCVof_i_j_m3Ps
                            Else
                                v = inCells(nc, nr).mStreamAttr.QCVch_i_j_m3Ps
                            End If
                            mArrayQ(nc, nr) = CDbl(Format(v, sformat))
                        End If
                    End If
                End If
            Next
        Next
        Return True

        'Select Case ValueType
        '    Case cGRM.GRMOutputType.SoilSaturation

        '        For nr As Integer = 0 To inCells.GetLength(1) - 1
        '            For nc As Integer = 0 To inCells.GetLength(0) - 1
        '                If inCells(nc, nr) IsNot Nothing Then
        '                    If inCells(nc, nr).bToBeSimulated = True OrElse inCells(nc, nr).bToEstimateParameters = True Then
        '                        Dim v As Single = inCells(nc, nr).soilSaturationRatio
        '                        strAry(nc, nr) = Format(CDbl(v), sformat)
        '                    End If
        '                End If
        '            Next
        '        Next
        '    Case cGRM.GRMOutputType.Rainfall

        '        For nr As Integer = 0 To inCells.GetLength(1) - 1
        '            For nc As Integer = 0 To inCells.GetLength(0) - 1
        '                If inCells(nc, nr) IsNot Nothing Then
        '                    If inCells(nc, nr).bToBeSimulated = True OrElse inCells(nc, nr).bToEstimateParameters = True Then

        '                    End If
        '                End If
        '            Next
        '        Next
        '    Case cGRM.GRMOutputType.RainfallCumulative

        '        For nr As Integer = 0 To inCells.GetLength(1) - 1
        '            For nc As Integer = 0 To inCells.GetLength(0) - 1
        '                If inCells(nc, nr) IsNot Nothing Then
        '                    If inCells(nc, nr).bToBeSimulated = True OrElse inCells(nc, nr).bToEstimateParameters = True Then

        '                    End If
        '                End If
        '            Next
        '        Next
        '    Case cGRM.GRMOutputType.Discharge

        '        For nr As Integer = 0 To inCells.GetLength(1) - 1
        '            For nc As Integer = 0 To inCells.GetLength(0) - 1
        '                If inCells(nc, nr) IsNot Nothing Then
        '                    If inCells(nc, nr).bToBeSimulated = True OrElse inCells(nc, nr).bToEstimateParameters = True Then

        '                    End If
        '                End If
        '            Next
        '        Next
        'End Select
        'Return strAry
    End Function

    Private Sub StartCreateASCTextFileSSRD()
        Dim ts As New ThreadStart(AddressOf CareateASCTextFileInnerSSRD)
        Dim th As New Thread(ts)
        th.Start()
        'Call CareateASCTextFileInnerSSRD()

    End Sub

    Private Sub CareateASCTextFileInnerSSRD()
        gentle.cTextFile.MakeASCTextFile(mASCfpnSSRD, mASCHeaderStringAll, "-9999", mArraySSR)
    End Sub

    Private Sub StartCreateASCTextFileRFD()
        Dim ts As New ThreadStart(AddressOf CareateASCTextFileInnerRFD)
        Dim th As New Thread(ts)
        th.Start()
        'Call CareateASCTextFileInnerRFD()
    End Sub

    Private Sub CareateASCTextFileInnerRFD()
        cTextFile.MakeASCTextFile(mASCfpnRFD, mASCHeaderStringAll, "-9999", mArrayRF)
    End Sub

    Private Sub StartCreateASCTextFileRFaccD()
        Dim ts As New ThreadStart(AddressOf CareateASCTextFileInnerRFaccD)
        Dim th As New Thread(ts)
        th.Start()
        'Call CareateASCTextFileInnerRFaccD()
    End Sub

    Private Sub CareateASCTextFileInnerRFaccD()
        cTextFile.MakeASCTextFile(mASCfpnRFaccD, mASCHeaderStringAll, "-9999", mArrayRFAcc)
    End Sub

    Private Sub StartCreateASCTextFileFlowD()
        Dim ts As New ThreadStart(AddressOf CareateASCTextFileInnerFlowD)
        Dim th As New Thread(ts)
        th.Start()
        'Call CareateASCTextFileInnerFlowD()

    End Sub

    Private Sub CareateASCTextFileInnerFlowD()
        cTextFile.MakeASCTextFile(mASCfpnFlowD, mASCHeaderStringAll, "-9999", mArrayQ)
    End Sub


    Public Property ImgWidth As Integer
        Get
            Return mImgWidth
        End Get
        Set(value As Integer)
            mImgWidth = value
        End Set
    End Property

    Public Property ImgHeight As Integer
        Get
            Return mImgHeight
        End Get
        Set(value As Integer)
            mImgHeight = value
        End Set
    End Property


End Class
