Imports System.Threading

Public Structure ImgFileInfo
    Public PFN As String
    Public width As Integer
    Public height As Integer
End Structure

Public Class cRasterOutput
    Private mProject As cProject
    Private mColCount As Integer
    Private mRowCount As Integer
    Private mASCHeaderStringAll As String
    Private mArraySSR As Double(,)
    Private mArrayRF As Double(,)
    Private mArrayRFAcc As Double(,)
    Private mArrayQ As Double(,)
    Private mASCfpnSSRD As String
    Private mASCfpnRFD As String
    Private mASCfpnRFaccD As String
    Private mASCfpnFlowD As String
    Private mImgInfoSSR As ImgFileInfo
    Private mImgInfoRF As ImgFileInfo
    Private mImgInfoRFAcc As ImgFileInfo
    Private mImgInfoFlow As ImgFileInfo
    Private mImgWidth As Integer
    Private mImgHeight As Integer

    Private mbMakeArySSR As Boolean = False
    Private mbMakeAryRF As Boolean = False
    Private mbMakeAryRFAcc As Boolean = False
    Private mbMakeAryQ As Boolean = False

    Private mbMakeImgFile As Boolean = False
    Private mbMakeASCFile As Boolean = False
    Private mbMakeValueAry As Boolean = False

    Private mbUseOtherThread As Boolean = False


    Sub New(ByVal project As cProject)
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

    Public Sub MakeDistributionFiles(ByVal nowT_MIN As Integer, imgWidth As Integer, imgHeight As Integer, usingNewThreadMakingImgFile As Boolean)
        Try
            mImgHeight = imgHeight
            mImgWidth = imgWidth
            mbUseOtherThread = usingNewThreadMakingImgFile
            Dim strNowTimeToPrintOut As String = ""
            strNowTimeToPrintOut = cComTools.GetTimeToPrintOut(mProject.GeneralSimulEnv.mIsDateTimeFormat, mProject.GeneralSimulEnv.mSimStartDateTime, nowT_MIN)
            strNowTimeToPrintOut = cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut)
            If mbMakeValueAry = True Then
                If mbMakeArySSR = True Then mArraySSR = New Double(mColCount - 1, mRowCount - 1) {}
                If mbMakeAryRF = True Then mArrayRF = New Double(mColCount - 1, mRowCount - 1) {}
                If mbMakeAryRFAcc = True Then mArrayRFAcc = New Double(mColCount - 1, mRowCount - 1) {}
                If mbMakeAryQ = True Then mArrayQ = New Double(mColCount - 1, mRowCount - 1) {}
                GetStringArrayUsingCVAttribute(mProject.WSCells, True)
            End If

            If mbMakeArySSR = True Then
                If Directory.Exists(mProject.OFPSSRDistribution) = False Then Exit Sub
                If mbMakeImgFile = True Then
                    mImgInfoSSR.PFN = Path.Combine(mProject.OFPSSRDistribution, cGRM.CONST_DIST_SSR_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    mImgInfoSSR.width = imgWidth
                    mImgInfoSSR.height = imgHeight
                    StartMakeImgSSRD()
                End If
                If mbMakeASCFile = True Then
                    mASCfpnSSRD = Path.Combine(mProject.OFPSSRDistribution, cGRM.CONST_DIST_SSR_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                    StartMakeASCTextFileSSRD()
                End If
            End If

            If mbMakeAryRF = True Then
                If Directory.Exists(mProject.OFPRFDistribution) = False Then Exit Sub
                If mbMakeImgFile = True Then
                    mImgInfoRF.PFN = Path.Combine(mProject.OFPRFDistribution, cGRM.CONST_DIST_RF_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    mImgInfoRF.width = imgWidth
                    mImgInfoRF.height = imgHeight
                    StartMakeImgRFD()
                End If
                If mbMakeASCFile = True Then
                    mASCfpnRFD = Path.Combine(mProject.OFPRFDistribution, cGRM.CONST_DIST_RF_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                    StartMakeASCTextFileRFD()
                End If
            End If

            If mbMakeAryRFAcc = True Then
                If Directory.Exists(mProject.OFPRFAccDistribution) = False Then Exit Sub
                If mbMakeImgFile = True Then
                    mImgInfoRFAcc.PFN = Path.Combine(mProject.OFPRFAccDistribution, cGRM.CONST_DIST_RFACC_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    mImgInfoRFAcc.width = imgWidth
                    mImgInfoRFAcc.height = imgHeight
                    StartMakeImgRFAccD()
                End If
                If mbMakeASCFile = True Then
                    mASCfpnRFaccD = Path.Combine(mProject.OFPRFAccDistribution, cGRM.CONST_DIST_RFACC_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                    StartMakeASCTextFileRFaccD()
                End If
            End If

            If mbMakeAryQ = True Then
                If Directory.Exists(mProject.OFPFlowDistribution) = False Then Exit Sub
                If mbMakeImgFile = True Then
                    mImgInfoFlow.PFN = Path.Combine(mProject.OFPFlowDistribution, cGRM.CONST_DIST_FLOW_FILE_HEAD + strNowTimeToPrintOut + ".png")
                    mImgInfoFlow.width = imgWidth
                    mImgInfoFlow.height = imgHeight
                    StartMakeImgFlowD()
                End If
                If mbMakeASCFile = True Then
                    mASCfpnFlowD = Path.Combine(mProject.OFPFlowDistribution, cGRM.CONST_DIST_FLOW_FILE_HEAD + strNowTimeToPrintOut + ".asc")
                    StartMakeASCTextFileFlowD()
                End If
            End If
        Catch ex As Exception
            Console.WriteLine(ex.ToString + "   " + cGRM.BuildInfo.ProductName)
        End Try
    End Sub


    Private Function GetStringArrayUsingCVAttribute(inCells As cCVAttribute(,), isparallel As Boolean) As Boolean
        Dim sformat As String = ""
        If isparallel = False Then
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
        Else
            Dim options As ParallelOptions = New ParallelOptions()
            options.MaxDegreeOfParallelism = cThisSimulation.MaxDegreeOfParallelism
            Parallel.For(0, inCells.GetLength(1), options, Sub(nr As Integer)
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
                                                           End Sub)
        End If

        Return True
    End Function

    Private Sub StartMakeImgSSRD()
        If mbUseOtherThread = True Then
            Dim ts As New ThreadStart(AddressOf MakeImgSSRDInner)
            Dim th As New Thread(ts)
            th.Start()
        Else
            MakeImgSSRDInner()
        End If
    End Sub

    Private Sub MakeImgSSRDInner()
        Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
        imgMaker.MakeImgFileUsingArrayFromTL(mImgInfoSSR.PFN, mArraySSR,
                                                   mImgInfoSSR.width, mImgInfoSSR.height, cImg.RendererRange.RendererFrom0to1)
    End Sub

    Private Sub StartMakeImgRFD()
        If mbUseOtherThread = True Then
            Dim ts As New ThreadStart(AddressOf MakeImgRFDInner)
            Dim th As New Thread(ts)
            th.Start()
        Else
            MakeImgRFDInner()
        End If
    End Sub

    Private Sub MakeImgRFDInner()
        Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
        imgMaker.MakeImgFileUsingArrayFromTL(mImgInfoRF.PFN, mArrayRF,
                                                   mImgInfoRF.width, mImgInfoRF.height, cImg.RendererRange.RendererFrom0to500)
    End Sub

    Private Sub StartMakeImgRFAccD()
        If mbUseOtherThread = True Then
            Dim ts As New ThreadStart(AddressOf MakeImgRFAccDInner)
            Dim th As New Thread(ts)
            th.Start()
        Else
            MakeImgRFAccDInner()
        End If
    End Sub

    Private Sub MakeImgRFAccDInner()
        Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
        imgMaker.MakeImgFileUsingArrayFromTL(mImgInfoRFAcc.PFN, mArrayRFAcc,
                                                   mImgInfoRFAcc.width, mImgInfoRFAcc.height, cImg.RendererRange.RendererFrom0to1000)
    End Sub

    Private Sub StartMakeImgFlowD()
        If mbUseOtherThread = True Then
            Dim ts As New ThreadStart(AddressOf MakeImgFlowDInner)
            Dim th As New Thread(ts)
            th.Start()
        Else
            MakeImgFlowDInner()
        End If
    End Sub

    Private Sub MakeImgFlowDInner()
        Dim imgMaker As New gentle.cImg(cImg.RendererType.Risk)
        imgMaker.MakeImgFileUsingArrayFromTL(mImgInfoFlow.PFN, mArrayQ,
                                                   mImgInfoFlow.width, mImgInfoFlow.height, cImg.RendererRange.RendererFrom0to30000)
    End Sub

    Private Sub StartMakeASCTextFileSSRD()
        Dim ts As New ThreadStart(AddressOf MakeASCTextFileInnerSSRD)
        Dim th As New Thread(ts)
        th.Start()
    End Sub

    Private Sub MakeASCTextFileInnerSSRD()
        gentle.cTextFile.MakeASCTextFile(mASCfpnSSRD, mASCHeaderStringAll, "-9999", mArraySSR)
    End Sub

    Private Sub StartMakeASCTextFileRFD()
        Dim ts As New ThreadStart(AddressOf MakeASCTextFileInnerRFD)
        Dim th As New Thread(ts)
        th.Start()
    End Sub

    Private Sub MakeASCTextFileInnerRFD()
        cTextFile.MakeASCTextFile(mASCfpnRFD, mASCHeaderStringAll, "-9999", mArrayRF)
    End Sub

    Private Sub StartMakeASCTextFileRFaccD()
        Dim ts As New ThreadStart(AddressOf MakeASCTextFileInnerRFaccD)
        Dim th As New Thread(ts)
        th.Start()
    End Sub

    Private Sub MakeASCTextFileInnerRFaccD()
        cTextFile.MakeASCTextFile(mASCfpnRFaccD, mASCHeaderStringAll, "-9999", mArrayRFAcc)
    End Sub

    Private Sub StartMakeASCTextFileFlowD()
        Dim ts As New ThreadStart(AddressOf MakeASCTextFileInnerFlowD)
        Dim th As New Thread(ts)
        th.Start()
    End Sub

    Private Sub MakeASCTextFileInnerFlowD()
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
