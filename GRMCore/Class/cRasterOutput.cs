using System;
using System.Threading.Tasks;
using System.Threading;
using System.Drawing;
using gentle;
using System.IO;
namespace GRMCore
{
    public struct ImgFileInfo
    {
        public string PFN;
        public int width;
        public int height;
    }

    public class cRasterOutput
    {
        private cProject mProject;
        private int mColCount;
        private int mRowCount;
        private string mASCHeaderStringAll;
        private int mascNodataValue = -9999;
        private double[,] mArraySSR;
        // Private mArraySSRasOneD As Double()

        private double[,] mArrayRF;
        private double[,] mArrayRFAcc;
        private double[,] mArrayQ;
        private string mASCfpnSSRD;
        private string mASCfpnRFD;
        private string mASCfpnRFaccD;
        private string mASCfpnFlowD;
        private ImgFileInfo mImgInfoSSR;
        private ImgFileInfo mImgInfoRF;
        private ImgFileInfo mImgInfoRFAcc;
        private ImgFileInfo mImgInfoFlow;
        private int mImgWidth;
        private int mImgHeight;

        public Bitmap mImgSSR;
        public Bitmap mImgRF;
        public Bitmap mImgRFacc;
        public Bitmap mImgFlow;

        private bool mbMakeArySSR = false;
        private bool mbMakeAryRF = false;
        private bool mbMakeAryRFAcc = false;
        private bool mbMakeAryQ = false;

        private bool mbMakeImgFile = false;
        private bool mbMakeASCFile = false;
        private bool mbMakeValueAry = false;

        private bool mbUseOtherThread = false;


        public cRasterOutput(cProject project)
        {
            mProject = project;
            mImgHeight = 370; // 433
            mImgWidth = 370; // 583
            mColCount = mProject.watershed.colCount;
            mRowCount = mProject.watershed.rowCount;
            mASCHeaderStringAll = cTextFile.MakeHeaderString(mColCount, mRowCount, mProject.watershed.mxllcorner, mProject.watershed.myllcorner, mProject.watershed.mCellSize, mascNodataValue.ToString());
            mbMakeImgFile = mProject.generalSimulEnv.mbCreateImageFile;
            mbMakeASCFile = mProject.generalSimulEnv.mbCreateASCFile;
            mbMakeValueAry = false;
            if (mProject.generalSimulEnv.mbShowSoilSaturation == true && (mbMakeImgFile == true || mbMakeASCFile == true))
            {
                mbMakeArySSR = true;
                mbMakeValueAry = true;
            }
            if (mProject.generalSimulEnv.mbShowRFdistribution == true && (mbMakeImgFile == true || mbMakeASCFile == true))
            {
                mbMakeAryRF = true;
                mbMakeValueAry = true;
            }
            if (mProject.generalSimulEnv.mbShowRFaccDistribution == true && (mbMakeImgFile == true || mbMakeASCFile == true))
            {
                mbMakeAryRFAcc = true;
                mbMakeValueAry = true;
            }
            if (mProject.generalSimulEnv.mbShowFlowDistribution == true && (mbMakeImgFile == true || mbMakeASCFile == true))
            {
                mbMakeAryQ = true;
                mbMakeValueAry = true;
            }
        }

        public void MakeDistributionFiles(int nowT_MIN, int imgWidth, int imgHeight, bool usingNewThreadMakingImgFile)
        {
            try
            {
                mImgHeight = imgHeight;
                mImgWidth = imgWidth;
                mbUseOtherThread = usingNewThreadMakingImgFile;
                string strNowTimeToPrintOut = "";
                strNowTimeToPrintOut = cComTools.GetTimeToPrintOut(mProject.generalSimulEnv.mIsDateTimeFormat, mProject.generalSimulEnv.mSimStartDateTime, nowT_MIN);
                strNowTimeToPrintOut = cComTools.GetTimeStringFromDateTimeFormat(strNowTimeToPrintOut);
                if (mbMakeValueAry == true)
                {
                    if (mbMakeArySSR == true)
                    {
                        mArraySSR = new double[mColCount - 1 + 1, mRowCount - 1 + 1];
                    }
                    if (mbMakeAryRF == true)
                    {
                        mArrayRF = new double[mColCount - 1 + 1, mRowCount - 1 + 1];
                    }
                    if (mbMakeAryRFAcc == true)
                    {
                        mArrayRFAcc = new double[mColCount - 1 + 1, mRowCount - 1 + 1];
                    }
                    if (mbMakeAryQ == true)
                    { mArrayQ = new double[mColCount - 1 + 1, mRowCount - 1 + 1]; }
                    GetStringArrayUsingCVAttribute(mProject.WSCells, true);
                }

                if (mbMakeArySSR == true)
                {
                    if (Directory.Exists(mProject.OFPSSRDistribution) == false)
                    { return; }
                    if (mbMakeImgFile == true)
                    {
                        mImgInfoSSR.PFN = Path.Combine(mProject.OFPSSRDistribution, cGRM.CONST_DIST_SSR_FILE_HEAD + strNowTimeToPrintOut + ".png");
                        mImgInfoSSR.width = imgWidth;
                        mImgInfoSSR.height = imgHeight;
                        StartMakeImgSSRD();
                    }
                    if (mbMakeASCFile == true)
                    {
                        mASCfpnSSRD = Path.Combine(mProject.OFPSSRDistribution, cGRM.CONST_DIST_SSR_FILE_HEAD + strNowTimeToPrintOut + ".asc");
                        StartMakeASCTextFileSSRD();
                    }
                }

                if (mbMakeAryRF == true)
                {
                    if (Directory.Exists(mProject.OFPRFDistribution) == false)
                    { return; }
                    if (mbMakeImgFile == true)
                    {
                        mImgInfoRF.PFN = Path.Combine(mProject.OFPRFDistribution, cGRM.CONST_DIST_RF_FILE_HEAD + strNowTimeToPrintOut + ".png");
                        mImgInfoRF.width = imgWidth;
                        mImgInfoRF.height = imgHeight;
                        StartMakeImgRFD();
                    }
                    if (mbMakeASCFile == true)
                    {
                        mASCfpnRFD = Path.Combine(mProject.OFPRFDistribution, cGRM.CONST_DIST_RF_FILE_HEAD + strNowTimeToPrintOut + ".asc");
                        StartMakeASCTextFileRFD();
                    }
                }

                if (mbMakeAryRFAcc == true)
                {
                    if (Directory.Exists(mProject.OFPRFAccDistribution) == false)
                    { return; }
                    if (mbMakeImgFile == true)
                    {
                        mImgInfoRFAcc.PFN = Path.Combine(mProject.OFPRFAccDistribution, cGRM.CONST_DIST_RFACC_FILE_HEAD + strNowTimeToPrintOut + ".png");
                        mImgInfoRFAcc.width = imgWidth;
                        mImgInfoRFAcc.height = imgHeight;
                        StartMakeImgRFAccD();
                    }
                    if (mbMakeASCFile == true)
                    {
                        mASCfpnRFaccD = Path.Combine(mProject.OFPRFAccDistribution, cGRM.CONST_DIST_RFACC_FILE_HEAD + strNowTimeToPrintOut + ".asc");
                        StartMakeASCTextFileRFaccD();
                    }
                }

                if (mbMakeAryQ == true)
                {
                    if (Directory.Exists(mProject.OFPFlowDistribution) == false)
                    { return; }
                    if (mbMakeImgFile == true)
                    {
                        mImgInfoFlow.PFN = Path.Combine(mProject.OFPFlowDistribution, cGRM.CONST_DIST_FLOW_FILE_HEAD + strNowTimeToPrintOut + ".png");
                        mImgInfoFlow.width = imgWidth;
                        mImgInfoFlow.height = imgHeight;
                        StartMakeImgFlowD();
                    }
                    if (mbMakeASCFile == true)
                    {
                        mASCfpnFlowD = Path.Combine(mProject.OFPFlowDistribution, cGRM.CONST_DIST_FLOW_FILE_HEAD + strNowTimeToPrintOut + ".asc");
                        StartMakeASCTextFileFlowD();
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString() + "   " + cGRM.BuildInfo.ProductName);
            }
        }

        private bool GetStringArrayUsingCVAttribute(cCVAttribute[,] inCells, bool isparallel)
        {
            string sformat = "#0.##";
            if (isparallel == false)
            {
                for (int nr = 0; nr < inCells.GetLength(1); nr++)
                {
                    for (int nc = 0; nc < inCells.GetLength(0); nc++)
                    {
                        if (inCells[nc, nr] != null)
                        {
                            if (inCells[nc, nr].toBeSimulated == 1)
                            {
                                if (mbMakeArySSR == true)
                                { mArraySSR[nc, nr] = inCells[nc, nr].soilSaturationRatio; }
                                if (mbMakeAryRF == true)
                                {
                                    mArrayRF[nc, nr] = (inCells[nc, nr].RF_dtPrintOut_meter * 1000);
                                }
                                if (mbMakeAryRFAcc == true)
                                {
                                    mArrayRFAcc[nc, nr] = (inCells[nc, nr].RFAcc_FromStartToNow_meter * 1000);
                                }
                                if (mbMakeAryQ == true)
                                {
                                    double v;
                                    if (inCells[nc, nr].FlowType == cGRM.CellFlowType.OverlandFlow)
                                    {
                                        v = inCells[nc, nr].QCVof_i_j_m3Ps;
                                    }
                                    else
                                    { v = inCells[nc, nr].mStreamAttr.QCVch_i_j_m3Ps; }
                                    mArrayQ[nc, nr] = v;
                                }
                            }
                        }
                        else
                        {
                            if (mbMakeArySSR == true)
                            {
                                mArraySSR[nc, nr] = -9999;
                            }
                            if (mbMakeAryRF == true)
                            {
                                mArrayRF[nc, nr] = -9999;
                            }
                            if (mbMakeAryRFAcc == true)
                            {
                                mArrayRFAcc[nc, nr] = -9999;
                            }
                            if (mbMakeAryQ == true)
                            { mArrayQ[nc, nr] = -9999; }
                        }
                    }
                }
            }
            else
            {
                ParallelOptions options = new ParallelOptions();
                if (sThisSimulation.IsParallel == true)
                {
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                }
                else
                {
                    options.MaxDegreeOfParallelism = -1;
                }
                Parallel.For(0, inCells.GetLength(1), options, delegate (int nr)
                {
                    for (int nc = 0; nc < inCells.GetLength(0); nc++)
                    {
                        if (inCells[nc, nr] != null)
                        {
                            if (inCells[nc, nr].toBeSimulated == 1)
                            {
                                if (mbMakeArySSR == true)
                                {
                                    mArraySSR[nc, nr] = inCells[nc, nr].soilSaturationRatio;
                                }
                                if (mbMakeAryRF == true)
                                {
                                    mArrayRF[nc, nr] = (inCells[nc, nr].RF_dtPrintOut_meter * 1000);
                                }
                                if (mbMakeAryRFAcc == true)
                                {
                                    mArrayRFAcc[nc, nr] = (inCells[nc, nr].RFAcc_FromStartToNow_meter * 1000);
                                }
                                if (mbMakeAryQ == true)
                                {
                                    double v;
                                    if (inCells[nc, nr].FlowType == cGRM.CellFlowType.OverlandFlow)
                                    {
                                        v = inCells[nc, nr].QCVof_i_j_m3Ps;
                                    }
                                    else
                                    {
                                        v = inCells[nc, nr].mStreamAttr.QCVch_i_j_m3Ps;
                                    }
                                    mArrayQ[nc, nr] = v;
                                }
                            }
                        }
                        else
                        {
                            if (mbMakeArySSR == true)
                            {
                                mArraySSR[nc, nr] = -9999;
                            }
                            if (mbMakeAryRF == true)
                            {
                                mArrayRF[nc, nr] = -9999;
                            }
                            if (mbMakeAryRFAcc == true)
                            {
                                mArrayRFAcc[nc, nr] = -9999;
                            }
                            if (mbMakeAryQ == true)
                            { mArrayQ[nc, nr] = -9999; }
                        }
                    }
                });
            }

            return true;
        }

        private void StartMakeImgSSRD()
        {
            if (mbUseOtherThread == true)
            {
                ThreadStart ts = new ThreadStart(MakeImgSSRDInner);
                Thread th = new Thread(ts);
                th.Start();
            }
            else
            {
                MakeImgSSRDInner();
            }
        }

        private void MakeImgSSRDInner()
        {
            gentle.cImg imgMaker = new gentle.cImg(cImg.RendererType.Risk);
            mImgSSR = null;
            // mImgSSR = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL(mImgInfoSSR.PFN, mArraySSR,
            // mImgInfoSSR.width, mImgInfoSSR.height, cImg.RendererRange.RendererFrom0to1)
            mImgSSR = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel(mImgInfoSSR.PFN, mArraySSR, mImgInfoSSR.width, mImgInfoSSR.height, cImg.RendererIntervalType.Differentinterval, cImg.RendererRange.RendererFrom0to1);
        }

        private void StartMakeImgRFD()
        {
            if (mbUseOtherThread == true)
            {
                ThreadStart ts = new ThreadStart(MakeImgRFDInner);
                Thread th = new Thread(ts);
                th.Start();
            }
            else
            {
                MakeImgRFDInner();
            }
        }

        private void MakeImgRFDInner()
        {
            gentle.cImg imgMaker = new gentle.cImg(cImg.RendererType.WaterDepth);
            mImgRF = null;
            // mImgRF = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL(mImgInfoRF.PFN, mArrayRF,
            // mImgInfoRF.width, mImgInfoRF.height, cImg.RendererRange.RendererFrom0to50)
            mImgRF = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel(mImgInfoRF.PFN, mArrayRF, mImgInfoRF.width, mImgInfoRF.height, cImg.RendererIntervalType.Differentinterval, cImg.RendererRange.RendererFrom0to50);
        }


        private void StartMakeImgRFAccD()
        {
            if (mbUseOtherThread == true)
            {
                ThreadStart ts = new ThreadStart(MakeImgRFAccDInner);
                Thread th = new Thread(ts);
                th.Start();
            }
            else
            {
                MakeImgRFAccDInner();
            }
        }

        private void MakeImgRFAccDInner()
        {
            gentle.cImg imgMaker = new gentle.cImg(cImg.RendererType.WaterDepth);
            mImgRFacc = null;
            // mImgRFacc = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL(mImgInfoRFAcc.PFN, mArrayRFAcc,
            // mImgInfoRFAcc.width, mImgInfoRFAcc.height, cImg.RendererRange.RendererFrom0to500)
            mImgRFacc = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel(mImgInfoRFAcc.PFN, mArrayRFAcc, mImgInfoRFAcc.width, mImgInfoRFAcc.height, cImg.RendererIntervalType.Differentinterval, cImg.RendererRange.RendererFrom0to500);
        }

        private void StartMakeImgFlowD()
        {
            if (mbUseOtherThread == true)
            {
                ThreadStart ts = new ThreadStart(MakeImgFlowDInner);
                Thread th = new Thread(ts);
                th.Start();
            }
            else
            {
                MakeImgFlowDInner();
            }
        }

        private void MakeImgFlowDInner()
        {
            gentle.cImg imgMaker = new gentle.cImg(cImg.RendererType.WaterDepth);
            mImgFlow = null;
            // mImgFlow = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL(mImgInfoFlow.PFN, mArrayQ,
            // mImgInfoFlow.width, mImgInfoFlow.height, cImg.RendererRange.RendererFrom0to10000)
            mImgFlow = imgMaker.MakeImgFileAndGetImgUsingArrayFromTL_InParallel(mImgInfoFlow.PFN, mArrayQ, mImgInfoFlow.width, mImgInfoFlow.height, cImg.RendererIntervalType.Differentinterval, cImg.RendererRange.RendererFrom0to10000);
        }

        private void StartMakeASCTextFileSSRD()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerSSRD);
            Thread th = new Thread(ts);
            th.Start();
        }

        private void MakeASCTextFileInnerSSRD()
        {
            cTextFile.MakeASCTextFile(mASCfpnSSRD, mASCHeaderStringAll, mArraySSR, 2, mascNodataValue);
        }

        private void StartMakeASCTextFileRFD()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerRFD);
            Thread th = new Thread(ts);
            th.Start();
        }

        private void MakeASCTextFileInnerRFD()
        {
            cTextFile.MakeASCTextFile(mASCfpnRFD, mASCHeaderStringAll, mArrayRF, 2, mascNodataValue);
        }

        private void StartMakeASCTextFileRFaccD()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerRFaccD);
            Thread th = new Thread(ts);
            th.Start();
        }

        private void MakeASCTextFileInnerRFaccD()
        {
            cTextFile.MakeASCTextFile(mASCfpnRFaccD, mASCHeaderStringAll, mArrayRFAcc, 2, mascNodataValue);
        }

        private void StartMakeASCTextFileFlowD()
        {
            ThreadStart ts = new ThreadStart(MakeASCTextFileInnerFlowD);
            Thread th = new Thread(ts);
            th.Start();
        }

        private void MakeASCTextFileInnerFlowD()
        {
            cTextFile.MakeASCTextFile(mASCfpnFlowD, mASCHeaderStringAll, mArrayQ, 2, mascNodataValue);
        }

        public int ImgWidth
        {
            get
            {
                return mImgWidth;
            }
            set
            {
                mImgWidth = value;
            }
        }

        public int ImgHeight
        {
            get
            {
                return mImgHeight;
            }
            set
            {
                mImgHeight = value;
            }
        }
    }

}
