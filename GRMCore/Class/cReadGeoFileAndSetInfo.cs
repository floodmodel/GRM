using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.IO;
using gentle;

namespace GRMCore
{
    public class cReadGeoFileAndSetInfo
    {
        /// <summary>
        ///   Watershed 레이어를 읽어서 유역 기본정보 설정
        ///   </summary>
        ///   <remarks>
        ///   레이어가 없는 경우는 FileNotFoundException 던짐
        ///   </remarks>
        public static bool ReadLayerWSandSetBasicInfo(string fpnWS, cSetWatershed watershed, ref cCVAttribute[,] wsCells,
            ref cCVAttribute[] CVs, ref sDomain[] dmInfo, ref cWatershedNetwork WSNetwork, ref cSetSubWatershedParameter subWSPar)
        {
            if (File.Exists(fpnWS) == false)
            {
                throw new FileNotFoundException(fpnWS);
            }
            cAscRasterReader gridWS = new cAscRasterReader(fpnWS);
            watershed.rowCount = gridWS.Header.numberRows;
            watershed.colCount = gridWS.Header.numberCols;
            watershed.mCellSize = System.Convert.ToInt32(gridWS.Header.cellsize);
            watershed.mxllcorner = gridWS.Header.xllcorner;
            watershed.myllcorner = gridWS.Header.yllcorner;
            // dim ary(n) 하면, vb.net에서는 0~n까지 n+1개의 배열요소 생성. c#에서는 0~(n-1) 까지 n 개의 요소 생성
            //cCVAttribute[,] wsCells = new cCVAttribute[watershed.colCount, watershed.rowCount];
            wsCells = new cCVAttribute[watershed.colCount, watershed.rowCount];
            List<cCVAttribute> lstCV = new List<cCVAttribute>();
            List<sDomain> lstDM = new List<sDomain>();
            int cvid = 0;
            // cvid를 순차적으로 부여하기 위해서, 이 과정은 병렬로 하지 않는다..
            for (int ry = 0; ry < watershed.rowCount; ry++)
            {
                for (int cx = 0; cx < watershed.colCount; cx++)
                {
                    int wsid = System.Convert.ToInt32(gridWS.ValueFromTL(cx, ry));
                    if (wsid > 0)
                    {
                        cCVAttribute cv = new cCVAttribute();
                        sDomain dm = new sDomain();
                        cv.WSID = wsid;
                        cvid += 1;
                        cv.CVID = cvid; // mCVs.Count + 1.  CVid를 CV 리스트(mCVs)의 인덱스 번호 +1 의 값으로 입력 
                        cv.FlowType = cGRM.CellFlowType.OverlandFlow; // 우선 overland flow로 설정
                        dm.XCol = cx;
                        dm.YRow = ry;
                        if (!watershed.WSIDList.Contains(cv.WSID))
                        {
                            watershed.WSIDList.Add(cv.WSID);
                        }
                        if (watershed.mCVidListForEachWS.ContainsKey(wsid) == false)
                        {
                            watershed.mCVidListForEachWS.Add(wsid, new List<int>());
                        }
                        watershed.mCVidListForEachWS[wsid].Add(cv.CVID);
                        cv.toBeSimulated = 1;
                        //if (wsid != 3) { cv.toBeSimulated = -1; } //TODO:주석 2018.12.11
                        wsCells[cx, ry] = cv;
                        lstCV.Add(cv);
                        lstDM.Add(dm);
                    }
                }
            }
            WSNetwork = new cWatershedNetwork(watershed.WSIDList);
            subWSPar.SetSubWSkeys(watershed.WSIDList);
            //CVs = new cCVAttribute[lstCV.Count];
            CVs = lstCV.ToArray();
            dmInfo = lstDM.ToArray();
            return true;
        }


        /// <summary>
        ///   Slope 레이어 읽기
        ///   </summary>
        ///   <returns>오류가 있는 경우 False</returns>
        ///   <remarks></remarks>
        public static bool ReadLayerSlope(string fpnSlope, cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {
            if (File.Exists(fpnSlope) == false)
            {
                throw new FileNotFoundException(fpnSlope);
                //return false;
            }
            cAscRasterReader gridSlope = new cAscRasterReader(fpnSlope);
            bool bNoError = true;
            if (isParallel == true)
            {
                ParallelOptions options = new ParallelOptions();
                options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                Parallel.For(0, rowyCount, options, delegate (int ry)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            WSCells[cx, ry].Slope = gridSlope.ValueFromTL(cx, ry);
                            if (WSCells[cx, ry].Slope <= 0.0)
                            {
                                WSCells[cx, ry].Slope = 0.0001;
                                bNoError = false;
                            }
                        }
                    }
                });
            }
            else
                for (int ry = 0; ry < rowyCount; ry++)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            WSCells[cx, ry].Slope = gridSlope.ValueFromTL(cx, ry);
                            if (WSCells[cx, ry].Slope <= 0.0)
                            {
                                WSCells[cx, ry].Slope = 0.0001;
                                bNoError = false;
                            }
                        }
                    }
                }

            return bNoError;
        }

        /// <summary>
        ///   Flow Dir 레이어 읽기
        ///   </summary>
        ///   <remarks></remarks>
        public static bool ReadLayerFdir(string fpnFdir, cCVAttribute[,] WSCells, int colxCount, int rowyCount, cGRM.FlowDirectionType fdtype, bool isParallel)
        {
            if (File.Exists(fpnFdir) == false)
            {
                throw new FileNotFoundException(fpnFdir);
            }
            cAscRasterReader gridFdir = new cAscRasterReader(fpnFdir);
            if (isParallel == true)
            {
                ParallelOptions options = new ParallelOptions();
                options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                Parallel.For(0, rowyCount, options, delegate (int ry)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            WSCells[cx, ry].FDir = cHydroCom.GetFlowDirection(System.Convert.ToInt32(gridFdir.ValueFromTL(cx, ry)), fdtype);
                        }
                    }
                });
            }
            else
                for (int ry = 0; ry < rowyCount; ry++)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            WSCells[cx, ry].FDir = cHydroCom.GetFlowDirection(System.Convert.ToInt32(gridFdir.ValueFromTL(cx, ry)), fdtype);
                        }
                    }
                }
            return true;
        }

        /// <summary>
        ///   Flow Acc 레이어 읽기
        ///   </summary>
        ///   <remarks></remarks>
        public static bool ReadLayerFAcc(string fpnFac, cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {
            if (File.Exists(fpnFac) == false)
            {
                throw new FileNotFoundException(fpnFac);
                //return false;
            }
            cAscRasterReader gridFac = new cAscRasterReader(fpnFac);
            if (isParallel == true)
            {
                ParallelOptions options = new ParallelOptions();
                options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                Parallel.For(0, rowyCount, options, delegate (int ry)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            int v = System.Convert.ToInt32(gridFac.ValueFromTL(cx, ry));
                            if (v < 0)
                            {
                                WSCells[cx, ry].FAc = 0;
                            }
                            else
                            {
                                WSCells[cx, ry].FAc = v;
                            }
                        }
                    }
                });
            }
            else
                for (int ry = 0; ry < rowyCount; ry++)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            int v = System.Convert.ToInt32(gridFac.ValueFromTL(cx, ry));
                            if (v < 0)
                            {
                                WSCells[cx, ry].FAc = 0;
                            }
                            else
                            {
                                WSCells[cx, ry].FAc = v;
                            }
                        }
                    }
                }
            return true;
        }

        /// <summary>
        ///   Stream 레이어 읽기
        ///   </summary>
        ///   <remarks></remarks>
        public static bool ReadLayerStream(string fpnStream, cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {
            try
            {
                cAscRasterReader gridStream = new cAscRasterReader(fpnStream);
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                int value = System.Convert.ToInt32(gridStream.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    WSCells[cx, ry].FlowType = cGRM.CellFlowType.ChannelFlow;
                                    WSCells[cx, ry].mStreamAttr = new cCVStreamAttribute();
                                    WSCells[cx, ry].mStreamAttr.ChStrOrder = value;
                                    WSCells[cx, ry].mStreamAttr.chBedSlope = WSCells[cx, ry].SlopeOF;
                                }
                            }
                        }
                    });
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                int value = System.Convert.ToInt32(gridStream.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    WSCells[cx, ry].FlowType = cGRM.CellFlowType.ChannelFlow;
                                    WSCells[cx, ry].mStreamAttr = new cCVStreamAttribute();
                                    WSCells[cx, ry].mStreamAttr.ChStrOrder = value;
                                    WSCells[cx, ry].mStreamAttr.chBedSlope = WSCells[cx, ry].SlopeOF;
                                }
                            }
                        }
                    }
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return false;
            }
        }

        /// <summary>
        ///   하폭 레이어 읽기
        ///   </summary>
        ///   <remarks></remarks>
        public static bool ReadLayerChannelWidth(string fpnCHw, cCVAttribute[,] WSCells, int colxCount, int rowyCount,
            bool isParallel)
        {
            try
            {
                cAscRasterReader gridCHWidth = new cAscRasterReader(fpnCHw);

                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                double value = gridCHWidth.ValueFromTL(cx, ry);
                                if (value < 0)
                                {
                                    WSCells[cx, ry].mStreamAttr.ChBaseWidthByLayer = 0;
                                }
                                else
                                {
                                    WSCells[cx, ry].mStreamAttr.ChBaseWidthByLayer = value;
                                }
                            }
                        }
                    });
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                double value = gridCHWidth.ValueFromTL(cx, ry);
                                if (value > 0)
                                {
                                    WSCells[cx, ry].mStreamAttr.ChBaseWidthByLayer = value;
                                }
                                //else
                                //    WSCells[cx, ry].mStreamAttr.ChBaseWidthByLayer = Watershed.mCellSize / (double)10;
                            }
                        }
                    }

                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return false;
            }
        }


        public static bool ReadLayerInitialSoilSaturation(string fpn, cCVAttribute[,] WSCells, int colxCount, int rowyCount, 
            bool isParallel)
        {
            try
            {
                cAscRasterReader ascIniSSR = new cAscRasterReader(fpn);
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                double v = ascIniSSR.ValueFromTL(cx, ry);
                                if (v < 0) { v = 0; }
                                if (v > 1) { v = 1; }
                                WSCells[cx, ry].InitialSaturation = v;
                            }
                        }
                    });
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                double v = ascIniSSR.ValueFromTL(cx, ry);
                                if (v < 0) { v = 0; }
                                if (v > 1) { v = 1; }
                                WSCells[cx, ry].InitialSaturation = v;
                            }
                        }
                    }
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return false;
            }
        }

        public static bool ReadLayerInitialChannelFlow(string fpn, cCVAttribute[,] WSCells, int colxCount, int rowyCount, 
            bool isParallel)
        {
            try
            {
                cAscRasterReader ascIniChFlow = new cAscRasterReader(fpn);
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                double value = ascIniChFlow.ValueFromTL(cx, ry);
                                if (value > 0)
                                {
                                    WSCells[cx, ry].mStreamAttr.initialQCVch_i_j_m3Ps = value;
                                }
                                else
                                {
                                    WSCells[cx, ry].mStreamAttr.initialQCVch_i_j_m3Ps = 0;
                                }
                            }
                        }
                    });
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                double value = ascIniChFlow.ValueFromTL(cx, ry);
                                if (value > 0)
                                {
                                    WSCells[cx, ry].mStreamAttr.initialQCVch_i_j_m3Ps = value;
                                }
                                else
                                {
                                    WSCells[cx, ry].mStreamAttr.initialQCVch_i_j_m3Ps = 0;
                                }
                            }
                        }
                    }
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return false;
            }
        }

        public static bool ReadLandCoverFile(string fpnLC, cCVAttribute[,] WSCells, int colxCount, int rowyCount, 
            bool isParallel)
        {
            try
            {
                cAscRasterReader gridLC = new cAscRasterReader(fpnLC);
                bool isnormal = true;
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridLC.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    cell.LandCoverValue = value;
                                }
                                else
                                {
                                    Console.WriteLine(string.Format("Landcover file {0} has an invalid value.", fpnLC), true, true);
                                    isnormal = false;
                                }
                            }
                        }
                    });
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridLC.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    cell.LandCoverValue = value;
                                }
                                else
                                {
                                    isnormal = false;
                                    Console.WriteLine(string.Format("Landcover file {0} has an invalid value.", fpnLC), true, true);
                                }
                            }
                        }
                    }
                return isnormal;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                throw new KeyNotFoundException(string.Format("Landcover file {0} has an invalid value.", fpnLC));
            }
        }

        /// <summary>
        ///   토지피복레이어 값 읽기
        ///   </summary>
        ///   <remarks></remarks>
        public static bool ReadLandCoverFileAndSetVAT(string fpnLC, cSetLandcover Landcover, cCVAttribute[,] WSCells, int colxCount, int rowyCount, 
            bool isParallel)
        {
            SortedList<int, float> vatRC = new SortedList<int, float>();
            SortedList<int, float> vatIR = new SortedList<int, float>();
            SortedList<int, cSetLandcover.LandCoverCode> vatLCcode = new SortedList<int, cSetLandcover.LandCoverCode>();
            foreach (Dataset.GRMProject.LandCoverRow row in Landcover.mdtLandCoverInfo)
            {
                vatRC.Add(System.Convert.ToInt32(row.GridValue), row.RoughnessCoefficient);
                vatIR.Add(System.Convert.ToInt32(row.GridValue), row.ImperviousRatio);
                cSetLandcover.LandCoverCode lcCode;
                if (!row.IsGRMCodeNull())
                {
                    lcCode = cSetLandcover.GetLandCoverCode(row.GRMCode);
                }
                else
                {
                    cGRM.writelogAndConsole(string.Format("Landcover attribute code was not set for {0}. ", row.GridValue), true, true);
                    return false;
                }
                vatLCcode.Add(System.Convert.ToInt32(row.GridValue), lcCode);
            }
            cAscRasterReader gridLC = new cAscRasterReader(fpnLC);
            // Dim isnormal As Boolean = True
            int vBak = vatRC.Keys[0]; // 여기서 기본값.
            bool isnormal = true;
            try
            {
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridLC.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    if (vatLCcode.ContainsKey (value)==true )
                                    {
                                    vBak = value; // 여기서 최신 셀의 값
                                    cell.LandCoverValue = value;
                                    cell.RoughnessCoeffOFori = vatRC[value];
                                    cell.ImperviousRatio = vatIR[value];
                                    cell.LandCoverCode = vatLCcode[value];
                                    }
                                    else
                                    {
                                        cGRM.writelogAndConsole(string.Format("Landcover VAT file ({0}) has not land cover value {1}. Check the land cover file or land cover VAT file.", Landcover.mLandCoverVATFPN, value), false, true);
                                        isnormal = false;
                                    }
                                }
                                else
                                {
                                    cell.LandCoverValue = vBak;
                                    cell.RoughnessCoeffOFori = vatRC[vBak];
                                    cell.ImperviousRatio = vatIR[vBak];
                                    cell.LandCoverCode = vatLCcode[vBak];
                                    cGRM.writelogAndConsole(string.Format("Landcover file {0} has an invalid value {1} at ({2}, {3}). {4} was applied.", Landcover.mGridLandCoverFPN, value, cx, ry, vBak), false, true);
                                }
                            }
                        }
                    });
                    if (isnormal ==false) { return false; }
                }
                else
                {
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridLC.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    if (vatLCcode .ContainsKey (value)==true )
                                    {
                                        vBak = value; // 여기서 최신 셀의 값
                                        cell.LandCoverValue = value;
                                        cell.RoughnessCoeffOFori = vatRC[value];
                                        cell.ImperviousRatio = vatIR[value];
                                        cell.LandCoverCode = vatLCcode[value];
                                    }
                                    else
                                    {
                                        cGRM.writelogAndConsole(string.Format("Landcover VAT file ({0}) has not land cover value {1}. Check the land cover file or land cover VAT file.", Landcover.mLandCoverVATFPN, value), false, true);
                                        return false;
                                    }
                                }
                                else
                                {
                                    cell.LandCoverValue = vBak;
                                    cell.RoughnessCoeffOFori = vatRC[vBak];
                                    cell.ImperviousRatio = vatIR[vBak];
                                    cell.LandCoverCode = vatLCcode[vBak];
                                    cGRM.writelogAndConsole(string.Format("Landcover file {0} has an invalid value {1} at ({2}, {3}). {4} was applied.", Landcover.mGridLandCoverFPN, value, cx, ry, vBak), false, true);
                                }
                            }
                        }
                    }
                }
                return true;
            }
            catch (KeyNotFoundException ex)
            {
                System.Console.WriteLine(ex.ToString());
                throw new KeyNotFoundException(string.Format("Landcover file {0} has an invalid value.", Landcover.mGridLandCoverFPN));
            }
        }

        public static bool SetLandCoverAttUsingConstant(cSetLandcover Landcover,
            cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {
            if (isParallel == true)
            {
                ParallelOptions options = new ParallelOptions();
                options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            cCVAttribute cell = WSCells[cx, ry];
                            cell.LandCoverValue = 0;     // 상수 의미
                            cell.RoughnessCoeffOFori = Landcover.mConstRoughnessCoefficient.Value;
                            cell.ImperviousRatio = Landcover.mConstImperviousRatio.Value;
                            cell.LandCoverCode = cSetLandcover.LandCoverCode.CONSTV;
                        }
                    }
                });
            }
            else
                for (int ry = 0; ry < rowyCount; ry++)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            cCVAttribute cell = WSCells[cx, ry];
                            cell.LandCoverValue = 0;    // 상수 의미
                            cell.RoughnessCoeffOFori = Landcover.mConstRoughnessCoefficient.Value;
                            cell.ImperviousRatio = Landcover.mConstImperviousRatio.Value;
                            cell.LandCoverCode = cSetLandcover.LandCoverCode.CONSTV;
                        }
                    }
                }
            return true;
        }

        public static bool ReadSoilTextureFile(string fpnST,
            cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {
            try
            {
                cAscRasterReader gridSTexture = new cAscRasterReader(fpnST);
                bool isnormal = true;
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridSTexture.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    cell.SoilTextureValue = value;
                                }
                                else
                                {
                                    Console.WriteLine(string.Format("Soil texture file {0} has an invalid value.", fpnST), true, true);
                                    isnormal = false;
                                }
                            }
                        }
                    });
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridSTexture.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    cell.SoilTextureValue = value;
                                }
                                else
                                {
                                    isnormal = false;
                                    Console.WriteLine(string.Format("Soil texture file {0} has an invalid value.", fpnST), true, true);
                                }
                            }
                        }
                    }
                return isnormal;
            }
            catch (Exception ex)
            {
                System.Console.WriteLine(ex.ToString());
                throw new KeyNotFoundException(string.Format("Soil texture file {0} has an invalid value.", fpnST));
            }
        }

        /// <summary>
        ///   토성레이어 값 읽기
        ///   </summary>
        ///   <remarks></remarks>
        public static  bool ReadSoilTextureFileAndSetVAT(string fpnST, cSetGreenAmpt GreenAmpt,
            cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {            
            cAscRasterReader gridSTexture = new cAscRasterReader(fpnST);
            SortedList<int, double> vatP = new SortedList<int, double>();
            SortedList<int, double> vatEP = new SortedList<int, double>();
            SortedList<int, double> vatWFSH = new SortedList<int, double>();
            SortedList<int, double> vatHC = new SortedList<int, double>();
            SortedList<int, cSetGreenAmpt.SoilTextureCode> vatSTcode = new SortedList<int, cSetGreenAmpt.SoilTextureCode>();
            foreach (Dataset.GRMProject.GreenAmptParameterRow row in GreenAmpt.mdtGreenAmptInfo)
            {
                vatP.Add(System.Convert.ToInt32(row.GridValue), row.Porosity);
                vatEP.Add(System.Convert.ToInt32(row.GridValue), row.EffectivePorosity);
                vatWFSH.Add(System.Convert.ToInt32(row.GridValue), row.WFSoilSuctionHead);
                vatHC.Add(System.Convert.ToInt32(row.GridValue), row.HydraulicConductivity);
                cSetGreenAmpt.SoilTextureCode stCode;
                if (!row.IsGRMCodeNull())
                {
                    stCode = cSetGreenAmpt.GetSoilTextureCode(row.GRMCode.ToString());
                }
                else
                {
                    cGRM.writelogAndConsole(string.Format("Soil texture attribute code was not set for {0}. ", row.GridValue), true, true);
                    return false;
                }
                vatSTcode.Add(System.Convert.ToInt32(row.GridValue), stCode);
            }
            bool isnormal = true;
            int vBak = vatP.Keys[0]; // 여기서 기본값.
            try
            {
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridSTexture.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    if (vatSTcode .ContainsKey (value)==true)
                                    {
                                        vBak = value; // 여기서 최신 셀의 값
                                        cell.SoilTextureValue = value;
                                        cell.PorosityEtaOri = vatP[value];
                                        cell.EffectivePorosityThetaEori = vatEP[value];
                                        cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH[value] / 100.0;  // cm -> m
                                        cell.HydraulicConductKori_mPsec = vatHC[value] / 100.0 / 3600.0;    // cm/hr -> m/s
                                        cell.SoilTextureCode = vatSTcode[value];
                                    }
                                    else
                                    {
                                        cGRM.writelogAndConsole(string.Format("Soil texture VAT file ({0}) has not soil texture value {1}. Check the soil texture file or soil texture VAT file.", GreenAmpt.mSoilTextureVATFPN, value), false, true);
                                        isnormal = false;
                                    }
                                }
                                else
                                {
                                    cGRM.writelogAndConsole(string.Format("Soil texture file {0} has an invalid value {1} at ({2}, {3}). {4} was applied.", GreenAmpt.mGridSoilTextureFPN, value, cx, ry, vBak), false, true);
                                    cell.SoilTextureValue = vBak;
                                    cell.PorosityEtaOri = vatP[vBak];
                                    cell.EffectivePorosityThetaEori = vatEP[vBak];
                                    cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH[vBak] / 100.0;  // cm -> m
                                    cell.HydraulicConductKori_mPsec = vatHC[vBak] / 100.0 / 3600.0;    // cm/hr -> m/s
                                    cell.SoilTextureCode = vatSTcode[vBak];
                                }
                            }
                        }
                    });
                    if (isnormal == false) { return false; }
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                cCVAttribute cell = WSCells[cx, ry];
                                int value = System.Convert.ToInt32(gridSTexture.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    if (vatSTcode.ContainsKey(value) == true)
                                    {
                                        vBak = value; // 여기서 최신 셀의 값
                                        cell.SoilTextureValue = value;
                                        cell.PorosityEtaOri = vatP[value];
                                        cell.EffectivePorosityThetaEori = vatEP[value];
                                        cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH[value] / 100.0;  // cm -> m
                                        cell.HydraulicConductKori_mPsec = vatHC[value] / 100.0 / 3600.0;    // cm/hr -> m/s
                                        cell.SoilTextureCode = vatSTcode[value];
                                    }
                                    else
                                    {
                                        cGRM.writelogAndConsole(string.Format("Soil texture VAT file ({0}) has not soil texture value {1}. Check the soil texture file or soil texture VAT file.", GreenAmpt.mSoilTextureVATFPN, value), false, true);
                                        return false;
                                    }
                                }
                                else
                                {
                                    cGRM.writelogAndConsole(string.Format("Soil texture file {0} has an invalid value {1} at ({2}, {3}). {4} was applied.", GreenAmpt.mGridSoilTextureFPN, value, cx, ry, vBak), false, true);
                                    cell.SoilTextureValue = vBak;
                                    cell.PorosityEtaOri = vatP[vBak];
                                    cell.EffectivePorosityThetaEori = vatEP[vBak];
                                    cell.WettingFrontSuctionHeadPsiOri_m = vatWFSH[vBak] / 100.0;  // cm -> m
                                    cell.HydraulicConductKori_mPsec = vatHC[vBak] / 100.0 / 3600.0;    // cm/hr -> m/s
                                    cell.SoilTextureCode = vatSTcode[vBak];
                                }
                            }
                        }
                    }
                return true;
            }
            catch (KeyNotFoundException ex)
            {
                System.Console.WriteLine(ex.ToString());
                throw new KeyNotFoundException(string.Format("Soil texture file {0} has an invalid value.", GreenAmpt.mGridSoilTextureFPN));
            }
        }


        public static bool SetSoilTextureAttUsingConstant(cSetGreenAmpt GreenAmpt,
            cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {
            if (isParallel == true)
            {
                ParallelOptions options = new ParallelOptions();
                options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            cCVAttribute cell = WSCells[cx, ry];
                            cell.SoilTextureValue = 0;  // 상수를 의미
                            cell.PorosityEtaOri = GreenAmpt.mConstPorosity.Value;
                            cell.EffectivePorosityThetaEori = GreenAmpt.mConstEffectivePorosity.Value;
                            cell.WettingFrontSuctionHeadPsiOri_m = GreenAmpt.mConstWFS.Value / 100.0;  // cm -> m
                            cell.HydraulicConductKori_mPsec = GreenAmpt.mConstHydraulicCond.Value / 100.0 / 3600.0;    // cm/hr -> m/s
                            cell.SoilTextureCode = cSetGreenAmpt.SoilTextureCode.CONSTV;
                        }
                    }
                });
            }
            else
                for (int ry = 0; ry < rowyCount; ry++)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            cCVAttribute cell = WSCells[cx, ry];
                            cell.SoilTextureValue = 0;  // 상수를 의미
                            cell.PorosityEtaOri = GreenAmpt.mConstPorosity.Value;
                            cell.EffectivePorosityThetaEori = GreenAmpt.mConstEffectivePorosity.Value;
                            cell.WettingFrontSuctionHeadPsiOri_m = GreenAmpt.mConstWFS.Value / 100.0;  // cm -> m
                            cell.HydraulicConductKori_mPsec = GreenAmpt.mConstHydraulicCond.Value / 100.0 / 3600.0;    // cm/hr -> m/s
                            cell.SoilTextureCode = cSetGreenAmpt.SoilTextureCode.CONSTV;
                        }
                    }
                }
            return true;
        }

        public static bool ReadSoilDepthFile(string fpnSD,
            cCVAttribute[,] WSCells, int colxCount, int rowyCount, bool isParallel)
        {
            cAscRasterReader gridSDepth = new cAscRasterReader(fpnSD);
            bool isnormal = true;
            try
            {
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                int value = System.Convert.ToInt32(gridSDepth.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    WSCells[cx, ry].SoilDepthTypeValue = System.Convert.ToInt32(value);
                                }
                                else
                                {
                                    Console.WriteLine(string.Format("Soil depth file {0} has an invalid value.", fpnSD), true, true);
                                    isnormal = false;
                                }
                            }
                        }
                    });
                }
                else
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                int value = System.Convert.ToInt32(gridSDepth.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    WSCells[cx, ry].SoilDepthTypeValue = System.Convert.ToInt32(value);
                                }
                                else
                                {
                                    isnormal = false;
                                    throw new KeyNotFoundException();
                                }
                            }
                        }
                    }
                return isnormal;
            }
            catch (KeyNotFoundException ex)
            {
                System.Console.WriteLine(ex.ToString());
                throw new KeyNotFoundException(string.Format("Soil depth file {0} has an invalid value.", fpnSD));
            }
        }

        /// <summary>
        ///   토양심 레이어 값 읽기
        ///   </summary>
        ///   <remarks></remarks>
        public static bool ReadSoilDepthFileAndSetVAT(string fpnSD, cSetSoilDepth SoilDepth, cCVAttribute[,] WSCells, int colxCount, int rowyCount,
            bool isParallel)
        {

            cAscRasterReader gridSDepth = new cAscRasterReader(fpnSD);
            SortedList<int, float> vatSD = new SortedList<int, float>();
            SortedList<int, cSetSoilDepth.SoilDepthCode> vatSDcode = new SortedList<int, cSetSoilDepth.SoilDepthCode>();
            foreach (Dataset.GRMProject.SoilDepthRow row in SoilDepth.mdtSoilDepthInfo)
            {
                vatSD.Add(System.Convert.ToInt32(row.GridValue), row.SoilDepth);
                cSetSoilDepth.SoilDepthCode stCode;
                if (!row.IsGRMCodeNull())
                    stCode = cSetSoilDepth.GetSoilDepthCode(row.GRMCode);
                else
                {
                    cGRM.writelogAndConsole(string.Format("Soil depth attribute code was not set for {0}. ", row.GridValue), true, true);
                    return false;
                }
                vatSDcode.Add(System.Convert.ToInt32(row.GridValue), stCode);
            }
            bool isnormal = true;
            int vBak = vatSD.Keys[0]; // 여기서 기본값.
            try
            {
                if (isParallel == true)
                {
                    ParallelOptions options = new ParallelOptions();
                    options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                    Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                int value = System.Convert.ToInt32(gridSDepth.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    if (vatSDcode.ContainsKey(value) == true)
                                    {
                                        vBak = value; // 여기서 최신 셀의 값
                                        WSCells[cx, ry].SoilDepthTypeValue = System.Convert.ToInt32(value);
                                        WSCells[cx, ry].SoilDepthOri_m = vatSD[value] / (double)100;     // cm ->  m
                                        WSCells[cx, ry].SoilDepthCode = vatSDcode[value];
                                    }
                                    else
                                    {
                                        cGRM.writelogAndConsole(string.Format("Soil depth VAT file ({0}) has not soil depth class value {1}. Check the soil depth file or soil depth VAT file.", SoilDepth.mSoilDepthVATFPN, value), false, true);
                                        isnormal = false;
                                    }
                                }
                                else
                                {
                                    cGRM.writelogAndConsole(string.Format("Soil depth file {0} has an invalid value {1} at ({2}, {3}). {4} was applied.", SoilDepth.mGridSoilDepthFPN, value, cx, ry, vBak), false, true);
                                    WSCells[cx, ry].SoilDepthTypeValue = System.Convert.ToInt32(vBak);
                                    WSCells[cx, ry].SoilDepthOri_m = vatSD[vBak] / (double)100;     // cm ->  m
                                    WSCells[cx, ry].SoilDepthCode = vatSDcode[vBak];
                                }
                            }
                        }
                    });
                    if (isnormal == false) { return false; }
                }
                else
                {
                    for (int ry = 0; ry < rowyCount; ry++)
                    {
                        for (int cx = 0; cx < colxCount; cx++)
                        {
                            if (WSCells[cx, ry] != null)
                            {
                                int value = System.Convert.ToInt32(gridSDepth.ValueFromTL(cx, ry));
                                if (value > 0)
                                {
                                    if (vatSDcode.ContainsKey(value) == true)
                                    {
                                        vBak = value; // 여기서 최신 셀의 값
                                        WSCells[cx, ry].SoilDepthTypeValue = System.Convert.ToInt32(value);
                                        WSCells[cx, ry].SoilDepthOri_m = vatSD[value] / (double)100;     // cm ->  m
                                        WSCells[cx, ry].SoilDepthCode = vatSDcode[value];
                                    }
                                    else
                                    {
                                        cGRM.writelogAndConsole(string.Format("Soil depth VAT file ({0}) has not soil depth class value {1}. Check the soil depth file or soil depth VAT file.", SoilDepth.mSoilDepthVATFPN, value), false, true);
                                        return false;
                                    }
                                }
                                else
                                {
                                    cGRM.writelogAndConsole(string.Format("Soil depth file {0} has an invalid value{1} at ({2}, {3}). {4} was applied.", SoilDepth.mGridSoilDepthFPN, value, cx, ry, vBak), false, true);
                                    WSCells[cx, ry].SoilDepthTypeValue = System.Convert.ToInt32(vBak);
                                    WSCells[cx, ry].SoilDepthOri_m = vatSD[vBak] / (double)100;     // cm ->  m
                                    WSCells[cx, ry].SoilDepthCode = vatSDcode[vBak];
                                }
                            }
                        }
                    }
                }
                return true;
            }
            catch (KeyNotFoundException ex)
            {
                System.Console.WriteLine(ex.ToString());
                throw new KeyNotFoundException(string.Format("Soil depth file {0} has an invalid value.", SoilDepth.mGridSoilDepthFPN));
            }
        }


        public static bool SetSoilDepthAttUsingConstant(cSetSoilDepth SoilDepth, cCVAttribute[,] WSCells, int colxCount, int rowyCount, 
            bool isParallel)
        {
            if (isParallel == true)
            {
                ParallelOptions options = new ParallelOptions();
                options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                Parallel.For(0, rowyCount, options, (Action<int>)delegate (int ry)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            WSCells[cx, ry].SoilDepthTypeValue = int.MinValue;  // 상수를 의미
                            WSCells[cx, ry].SoilDepthOri_m = SoilDepth.mConstSoilDepth.Value / (double)100;     // cm ->  m
                            WSCells[cx, ry].SoilDepthCode = cSetSoilDepth.SoilDepthCode.CONSTV;
                        }
                    }
                });
            }
            else
                for (int ry = 0; ry < rowyCount; ry++)
                {
                    for (int cx = 0; cx < colxCount; cx++)
                    {
                        if (WSCells[cx, ry] != null)
                        {
                            WSCells[cx, ry].SoilDepthTypeValue = int.MinValue;  // 상수를 의미
                            WSCells[cx, ry].SoilDepthOri_m = SoilDepth.mConstSoilDepth.Value / (double)100;     // cm ->  m
                            WSCells[cx, ry].SoilDepthCode = cSetSoilDepth.SoilDepthCode.CONSTV;
                        }
                    }
                }
            return true;
        }

    }
}
