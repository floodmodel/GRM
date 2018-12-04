using System;
using System.Collections.Generic;
using System.IO;
using System.Data;
using gentle;
using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;

namespace GRMCore
{
    public class cProject //: IDisposable
    {

        bool disposed = false;
        SafeHandle handle = new SafeFileHandle(IntPtr.Zero, true);
        private static cProject mProject;
        public cGRM.SimulationType mSimulationType;
        public cSetWatershed watershed;
        public cSetLandcover landcover;
        public cSetGreenAmpt GreenAmpt;
        public cSetSoilDepth soilDepth;
        public cRainfall rainfall;
        /// <summary>
        ///   SetupGRM tab에서 설정되는 정보
        ///   </summary>
        ///   <remarks></remarks>
        public cSetGeneralSimulEnvironment generalSimulEnv;

        /// <summary>
        ///   Channel tab에서 설정되는 정보
        ///   </summary>
        ///   <remarks></remarks>
        public cSetChannel channel;

        /// <summary>
        ///   Watchpoint tab에서 설정되는 정보
        ///   </summary>
        ///   <remarks></remarks>
        public cSetWatchPoint watchPoint;

        /// <summary>
        ///   Flow control tab에서 설정되는 정보
        ///   </summary>
        ///   <remarks></remarks>
        public cFlowControl fcGrid;

        /// <summary>
        ///   Watershed parameter tab에서 설정되는 정보
        ///   </summary>
        ///   <remarks></remarks>
        public cSetSubWatershedParameter subWSPar;

        public double MaxIniflowSetByUser
        {
            get
            {
                double max = 0;
                foreach (int id in cProject.Current.watershed.WSIDList)
                {
                    if (mProject.subWSPar.userPars[id].isUserSet == true && mProject.subWSPar.userPars[id].iniFlow > 0)
                    {
                        if (max < mProject.subWSPar.userPars[id].iniFlow)
                        { max = mProject.subWSPar.userPars[id].iniFlow; }
                    }
                }
                return max;
            }
        }



        public cWatershedNetwork WSNetwork;
        /// <summary>
        ///   최하류의 cv 배열 번호, cellid는 mMostDownCellArrayNumber+1
        ///   </summary>
        ///   <remarks></remarks>
        public int mMostDownCellArrayNumber;
        public cFlowAccInfo mCVANsForEachFA = new cFlowAccInfo();


        public cTSData.TimeSeriesInfoInTSDB gstObTSfromDB;


        public Dataset.GRMProject PrjFile;
        public string ProjectPathName;
        public string ProjectPath;
        public string ProjectNameWithExtension;
        public string ProjectNameOnly;
        public string GeoDataPrjPathName;
        public string OFNPDischarge;
        public string OFNPDepth;
        public string OFNPRFGrid;
        public string OFNPRFMean;
        public string OFNPFCData;
        public string OFNPFCStorage;
        public string OFNPSwsPars;
        public string OFPSSRDistribution;
        public string OFPRFDistribution;
        public string OFPRFAccDistribution;
        public string OFPFlowDistribution;
        public List<string> mImgFPN_dist_SSR;
        public List<string> mImgFPN_dist_RF;
        public List<string> mImgFPN_dist_RFAcc;
        public List<string> mImgFPN_dist_Flow;


        /// <summary>
        ///   검사체적 메모리 공간
        ///   </summary>
        ///   <remarks></remarks>
        public cCVAttribute[,] WSCells;
        public cCVAttribute[] CVs;
        public sDomain[] dmInfo;

        //public cSetTSData mTSSummary;

        public cProject()
        {
            watershed = new cSetWatershed();
            landcover = new cSetLandcover();
            GreenAmpt = new cSetGreenAmpt();
            soilDepth = new cSetSoilDepth();
            rainfall = new cRainfall();
            generalSimulEnv = new cSetGeneralSimulEnvironment();
            channel = new cSetChannel();
            watchPoint = new cSetWatchPoint();
            fcGrid = new cFlowControl();
            subWSPar = new cSetSubWatershedParameter();

        }

        public void Dispose()
        {
            Dispose(true);
            // Suppress finalization.
            GC.SuppressFinalize(this);
        }

        // Protected implementation of Dispose pattern.
        protected virtual void Dispose(bool disposing)
        {
            if (disposed)
                return;

            if (disposing)
            {
                handle.Dispose();
                // Free any other managed objects here.
                //
            }

            disposed = true;
        }

        public static cProject Current
        {
            get
            {
                return mProject;
            }
        }


        /// <summary>
        ///   분석을 위한 메모리격자 공간을 초기화
        ///   </summary>
        ///   <remarks>
        ///   레이어 누락인 경우 FileNotFoundException 던짐
        ///   </remarks>
        public bool SetBasicCVInfo()
        {
            try
            {
                if (!watershed.IsSet || !landcover.IsSet || !GreenAmpt.IsSet || !soilDepth.IsSet)
                    throw new InvalidOperationException();
                if (mProject.mSimulationType == cGRM.SimulationType.SingleEvent & !rainfall.IsSet)
                    throw new InvalidOperationException();
                Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)PrjFile.ProjectSettings.Rows[0];
                cReadGeoFileAndSetInfo.ReadLayerWSandSetBasicInfo(row.WatershedFile, watershed, ref WSCells, ref CVs, ref dmInfo, ref WSNetwork, ref subWSPar);
                cReadGeoFileAndSetInfo.ReadLayerSlope(row.SlopeFile, WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel);
                cReadGeoFileAndSetInfo.ReadLayerFdir(row.FlowDirectionFile, WSCells, watershed.colCount, watershed.rowCount, watershed.mFDType, sThisSimulation.IsParallel);
                cReadGeoFileAndSetInfo.ReadLayerFAcc(row.FlowAccumFile, WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel);
                string FPNstream = "";
                string FPNchannelWidth = "";
                string FPNiniSSR = "";
                string FPNiniChannelFlow = "";
                string FPNlc = "";
                string FPNst = "";
                string FPNsd = "";
                if (row.IsStreamFileNull() == false && File.Exists(row.StreamFile) == true)
                {
                    FPNstream = row.StreamFile;
                    if (cReadGeoFileAndSetInfo.ReadLayerStream(row.StreamFile, 
                        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
                        cGRM.writelogAndConsole(string.Format("Some errors were occurred while reading stream file.. {0}", row.StreamFile.ToString()), true, true);
                }
                if (row.IsChannelWidthFileNull() == false && File.Exists(row.ChannelWidthFile) == true)
                {
                    FPNchannelWidth = row.ChannelWidthFile;
                    if (cReadGeoFileAndSetInfo.ReadLayerChannelWidth(row.ChannelWidthFile, 
                        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
                        Console.WriteLine(string.Format("Some errors were occurred while reading channel width file.. {0}", row.ChannelWidthFile.ToString()), true, true);
                }
                if (row.IsInitialSoilSaturationRatioFileNull() == false && File.Exists(row.InitialSoilSaturationRatioFile) == true)
                {
                    FPNiniSSR = row.InitialSoilSaturationRatioFile;
                    if (cReadGeoFileAndSetInfo.ReadLayerInitialSoilSaturation(row.InitialSoilSaturationRatioFile, 
                        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
                        cGRM.writelogAndConsole(string.Format("Some errors were occurred while reading initial soil saturation file.. {0}", row.InitialSoilSaturationRatioFile.ToString()), true, true);
                }
                if (row.IsInitialChannelFlowFileNull() == false && File.Exists(row.InitialChannelFlowFile) == true)
                {
                    FPNiniChannelFlow = row.InitialChannelFlowFile;
                    if (cReadGeoFileAndSetInfo.ReadLayerInitialChannelFlow(row.InitialChannelFlowFile,
                        WSCells, watershed.colCount, watershed.rowCount, 
                        sThisSimulation.IsParallel) == false)
                        cGRM.writelogAndConsole(string.Format("Some errors were occurred while reading initial channel flow file.. {0}", row.InitialChannelFlowFile.ToString()), true, true);
                }

                if (landcover.mLandCoverDataType.Equals(cGRM.FileOrConst.File) && row.IsLandCoverFileNull() == false && File.Exists(row.LandCoverFile))
                {
                    if (!landcover.IsSet) { return false; }
                    FPNlc = row.LandCoverFile;
                    if (cReadGeoFileAndSetInfo.ReadLandCoverFileAndSetVAT(row.LandCoverFile, landcover,
                        WSCells, watershed.colCount, watershed.rowCount,
                        sThisSimulation.IsParallel) == false)
                        return false;
                }
                else if (cReadGeoFileAndSetInfo.SetLandCoverAttUsingConstant(landcover,
                    WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
                { return false; }

                if (GreenAmpt.mSoilTextureDataType.Equals(cGRM.FileOrConst.File) && row.IsSoilTextureFileNull() == false && File.Exists(row.SoilTextureFile))
                {
                    if (!GreenAmpt.IsSet) { return false; }
                    FPNst = row.SoilTextureFile;
                    if (cReadGeoFileAndSetInfo.ReadSoilTextureFileAndSetVAT(row.SoilTextureFile, GreenAmpt,
                        WSCells, watershed.colCount, watershed.rowCount, 
                        sThisSimulation.IsParallel) == false)
                        return false;
                }
                else if (cReadGeoFileAndSetInfo.SetSoilTextureAttUsingConstant(GreenAmpt,
                        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
                    return false;

                if (soilDepth.mSoilDepthDataType.Equals(cGRM.FileOrConst.File) && row.IsSoilDepthFileNull() == false && File.Exists(row.SoilDepthFile))
                {
                    if (!soilDepth.IsSet) { return false; }
                    FPNsd = row.SlopeFile;
                    if (cReadGeoFileAndSetInfo.ReadSoilDepthFileAndSetVAT(row.SoilDepthFile, soilDepth,
                        WSCells, watershed.colCount, watershed.rowCount, 
                        sThisSimulation.IsParallel) == false)
                        return false;
                }
                else if (cReadGeoFileAndSetInfo.SetSoilDepthAttUsingConstant(soilDepth,
                        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
                    return false;
                SetGridNetworkFlowInformation();
                InitControlVolumeAttribute();
                return true;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        ///   계산되는 속성의 초기화
        ///   </summary>
        ///   <remarks></remarks>
        public void InitControlVolumeAttribute()
        {
            watershed.mFacMostUpChannelCell = CVCount;
            mCVANsForEachFA.Clear();
            watershed.mFacMax = 0;
            watershed.mFacMin = int.MaxValue;
            for (int cvan = 0; cvan < CVCount; cvan++)
            {
                cCVAttribute acv = CVs[cvan];
                acv.FCType = cFlowControl.FlowControlType.NONE;
                acv.toBeSimulated = 1;
                acv.DownStreamWPCVids = new List<int>();
                double deltaXw;
                if (acv.NeighborCVidFlowIntoMe.Count > 0)
                    deltaXw = acv.deltaXwSum / (double)acv.NeighborCVidFlowIntoMe.Count;
                else
                    deltaXw = acv.DeltaXDownHalf_m;
                acv.CVDeltaX_m = acv.DeltaXDownHalf_m * 2;
                // FA별 cvid 저장
                mCVANsForEachFA.Add(acv.FAc, cvan);

                if (acv.FAc > watershed.mFacMax)
                {
                    watershed.mFacMax = acv.FAc;
                    mMostDownCellArrayNumber = cvan;
                }

                if (acv.FAc < watershed.mFacMin)
                    watershed.mFacMin = acv.FAc;

                // 하도 매개변수 받고
                if (acv.FlowType == cGRM.CellFlowType.ChannelFlow)
                {
                    if (acv.FAc < watershed.mFacMostUpChannelCell)
                        watershed.mFacMostUpChannelCell = acv.FAc;
                }
            }
        }

        /// <summary>
        ///   자신으로 유입되는 인접셀의 개수와 cellidInWatershed를 입력함.
        ///   </summary>
        ///   <remarks></remarks>
        public void SetGridNetworkFlowInformation()
        {
            double DeltaXDiagonalHalf_m;
            double DeltaXPerpendicularHalf_m;
            DeltaXDiagonalHalf_m = watershed.mCellSize * Math.Sqrt(2) / 2.0;
            DeltaXPerpendicularHalf_m = watershed.mCellSize / 2.0;
            for (int row = 0; row < watershed.rowCount; row++)
            {
                for (int col = 0; col < watershed.colCount; col++)
                {
                    if (WSCells[col, row] == null)
                        continue;
                    cCVAttribute cell = WSCells[col, row];
                    if (cell.NeighborCVidFlowIntoMe == null)
                        cell.NeighborCVidFlowIntoMe = new List<int>();
                    cCVAttribute targetCell = new cCVAttribute() ;
                    double deltaXe;
                    int targetC;
                    int targetR;
                    {
                        // 좌상단이 0,0 이다... 즉, 북쪽이면, row-1, 동쪽이면 col +1
                        switch (cell.FDir)
                        {
                            case  cGRM.GRMFlowDirectionD8.NE:
                                {
                                    targetC = col + 1;
                                    targetR = row - 1;
                                    deltaXe = DeltaXDiagonalHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.E:
                                {
                                    targetC = col + 1;
                                    targetR = row;
                                    deltaXe = DeltaXPerpendicularHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.SE:
                                {
                                    targetC = col + 1;
                                    targetR = row + 1;
                                    deltaXe = DeltaXDiagonalHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.S:
                                {
                                    targetC = col;
                                    targetR = row + 1;
                                    deltaXe = DeltaXPerpendicularHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.SW:
                                {
                                    targetC = col - 1;
                                    targetR = row + 1;
                                    deltaXe = DeltaXDiagonalHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.W:
                                {
                                    targetC = col - 1;
                                    targetR = row;
                                    deltaXe = DeltaXPerpendicularHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.NW:
                                {
                                    targetC = col - 1;
                                    targetR = row - 1;
                                    deltaXe = DeltaXDiagonalHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.N:
                                {
                                    targetC = col;
                                    targetR = row - 1;
                                    deltaXe = DeltaXPerpendicularHalf_m;
                                    break;
                                }

                            case  cGRM.GRMFlowDirectionD8.NONE:
                                {
                                    targetC = -1;
                                    targetR = -1;
                                    deltaXe = DeltaXPerpendicularHalf_m;
                                    break;
                                }

                            default:
                                {
                                    throw new InvalidOperationException();
                                }
                        }

                        if (IsInBound(targetC, targetR))
                        {
                            targetCell = WSCells[targetC, targetR];
                            if (targetCell == null)
                                WSNetwork.SetWSoutletCVID(cell.WSID, cell.CVID);
                            else
                            {
                                if (targetCell.NeighborCVidFlowIntoMe == null)
                                    targetCell.NeighborCVidFlowIntoMe = new List<int>();
                                targetCell.NeighborCVidFlowIntoMe.Add(cell.CVID); // 현재의 cellid를 하류셀의 정보에 기록
                                targetCell.deltaXwSum = targetCell.deltaXwSum + deltaXe;
                                cell.DownCellidToFlow = targetCell.CVID;  // 흘러갈 방향의 cellid를 현재 셀의 정보에 기록
                                if (!(cell.WSID == targetCell.WSID))
                                {
                                    if (WSNetwork.WSIDsNearbyDown(cell.WSID) != targetCell.WSID)
                                    {
                                        WSNetwork.AddWSIDdown(cell.WSID, targetCell.WSID);
                                        WSNetwork.SetWSoutletCVID(cell.WSID, cell.CVID);
                                    }
                                    // If cell.XCol = 97 AndAlso cell.YRow = 17 Then 
                                    if (!WSNetwork.WSIDsNearbyUp(targetCell.WSID).Contains(cell.WSID))
                                        WSNetwork.AddWSIDup(targetCell.WSID, cell.WSID);
                                }
                            }
                            cell.DeltaXDownHalf_m = deltaXe;
                        }
                        else
                        {
                            cell.DownCellidToFlow = -1;
                            cell.DeltaXDownHalf_m = deltaXe;
                            WSNetwork.SetWSoutletCVID(cell.WSID, cell.CVID);
                        }
                    }
                }
            }
            WSNetwork.UpdateAllDownsAndUpsNetwork();
        }

        public List<int> getAllUpstreamCells(int startingBaseCVID)
        {
            try
            {
                bool bAllisEnded = false;
                List<int> cvidsOfAllUpstreamCells = new List<int>();
                List<int> baseCVids;
                List<int> currentUpstreamCells;
                baseCVids = new List<int>();
                cvidsOfAllUpstreamCells.Add(startingBaseCVID);
                baseCVids.Add(startingBaseCVID);
                while (!bAllisEnded == true)
                {
                    int intCountAL = baseCVids.Count;
                    currentUpstreamCells = new List<int>();
                    bAllisEnded = true;
                    foreach (int baseCVID in baseCVids)
                    {
                        int intNowCVArrayNum = System.Convert.ToInt32(baseCVID) - 1;
                        int countUpCells = CVs[intNowCVArrayNum].NeighborCVidFlowIntoMe.Count;
                        if (countUpCells > 0)
                        {
                            bAllisEnded = false;
                            foreach (int CVidFlowIntoMe in CVs[intNowCVArrayNum].NeighborCVidFlowIntoMe)
                            {
                                int upArrayNum = CVidFlowIntoMe - 1;
                                int colX = dmInfo[upArrayNum].XCol;
                                int rowY = dmInfo[upArrayNum].YRow;
                                currentUpstreamCells.Add(upArrayNum + 1); // CVid == Cellid == CVArrayNumber+1
                                cvidsOfAllUpstreamCells.Add(upArrayNum + 1);
                            }
                        }
                    }
                    baseCVids = new List<int>();
                    baseCVids = currentUpstreamCells;
                }
                return cvidsOfAllUpstreamCells;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return null;
            }
        }

        private bool IsInBound(int colIdx, int rowIdy)
        {
            return colIdx >= 0 && colIdx < watershed.colCount && rowIdy >= 0 && rowIdy < watershed.rowCount;
        }        

        public void UpdateCVbyUserSettings()
        {
            for (int intR = 0; intR < watershed.rowCount ; intR++)
            {
                for (int intC = 0; intC < watershed.colCount ; intC++)
                {
                    if (WSCells[intC, intR] == null)
                        continue;
                    cCVAttribute cell = WSCells[intC, intR];
                    int wsid = cell.WSID;
                    cUserParameters ups = subWSPar.userPars[wsid];
                    cell.DownStreamWPCVids.Clear();
                    cell.toBeSimulated = 1;
                    if (cell.FlowType == cGRM.CellFlowType.ChannelNOverlandFlow)
                        cell.FlowType = cGRM.CellFlowType.ChannelFlow;
                    // 지표면 경사
                    if (cell.Slope < ups.minSlopeOF)
                        cell.SlopeOF = ups.minSlopeOF;
                    else
                        cell.SlopeOF = cell.Slope;
                    cell.RoughnessCoeffOF = cell.RoughnessCoeffOFori * ups.ccLCRoughness;

                    // 하천
                    if (watershed.HasStreamLayer && cell.FlowType == cGRM.CellFlowType.ChannelFlow)
                    {
                        cell.mStreamAttr.RoughnessCoeffCH = ups.chRoughness;
                        cell.mStreamAttr.chSideSlopeLeft = channel.mLeftBankSlope;
                        cell.mStreamAttr.chSideSlopeRight =channel.mRightBankSlope;
                        cell.mStreamAttr.mChBankCoeff = 1 /channel.mLeftBankSlope + 1 / channel.mRightBankSlope;
                        if (cell.Slope < ups.minSlopeChBed)
                            cell.mStreamAttr.chBedSlope = ups.minSlopeChBed;
                        else
                            cell.mStreamAttr.chBedSlope = cell.Slope;

                        if (channel.mCrossSectionType == cSetCrossSection.CSTypeEnum.CSSingle)
                        {
                            cSetCSSingle cs = new cSetCSSingle();
                            cs = (cSetCSSingle)channel.mCrossSection;
                            if (cs.mCSSingleWidthType == cSetCSSingle.CSSingleChannelWidthType.CWEquation)
                                cell.mStreamAttr.ChBaseWidth = cs.mCWEc * Math.Pow((cell.FAc + 1) * (watershed.mCellSize * watershed.mCellSize / 1000000.0), cs.mCWEd)
/ Math.Pow(cell.mStreamAttr.chBedSlope, cs.mCWEe);
                            else
                                cell.mStreamAttr.ChBaseWidth = cell.FAc * cs.mMaxChannelWidthSingleCS / (double)FacMax;
                            if (!string.IsNullOrEmpty(watershed.mFPN_channelWidth) && cell.mStreamAttr.ChBaseWidthByLayer > 0)
                                cell.mStreamAttr.ChBaseWidth = cell.mStreamAttr.ChBaseWidthByLayer;
                            cell.mStreamAttr.chUpperRBaseWidth_m = 0;
                            cell.mStreamAttr.chIsCompoundCS = false;
                            cell.mStreamAttr.chLowerRArea_m2 = 0;
                        }
                        else
                        {
                            cSetCSCompound cs = new cSetCSCompound();
                            cs = (cSetCSCompound)channel.mCrossSection;
                            cell.mStreamAttr.ChBaseWidth = cell.FAc * cs.mLowerRegionBaseWidth / (double)FacMax;
                            if (cell.mStreamAttr.ChBaseWidth < cs.mCompoundCSCriteriaChannelWidth)
                            {
                                cell.mStreamAttr.chIsCompoundCS = false;
                                cell.mStreamAttr.chUpperRBaseWidth_m = 0;
                                cell.mStreamAttr.chLowerRHeight = 0;
                                cell.mStreamAttr.chLowerRArea_m2 = 0;
                            }
                            else
                            {
                                cell.mStreamAttr.chIsCompoundCS = true;
                                cell.mStreamAttr.chUpperRBaseWidth_m = cell.FAc * cs.mUpperRegionBaseWidth / (double)FacMax;
                                cell.mStreamAttr.chLowerRHeight = cell.FAc * cs.mLowerRegionHeight / (double)FacMax;
                                cFVMSolver mFVMSolver = new cFVMSolver();
                                cell.mStreamAttr.chLowerRArea_m2 = mFVMSolver.GetChannelCrossSectionAreaUsingChannelFlowDepth(cell.mStreamAttr.ChBaseWidth, cell.mStreamAttr.mChBankCoeff, cell.mStreamAttr.chLowerRHeight, false, cell.mStreamAttr.chLowerRHeight, cell.mStreamAttr.chLowerRArea_m2, 0);
                            }
                        }
                        // 최소 하폭
                        if (cell.mStreamAttr.ChBaseWidth < ups.minChBaseWidth)
                            cell.mStreamAttr.ChBaseWidth = ups.minChBaseWidth;
                        if (cell.mStreamAttr.ChBaseWidth < watershed.mCellSize)
                            cell.FlowType = cGRM.CellFlowType.ChannelNOverlandFlow;
                    }

                    // 토양
                    if (watershed.mFPN_initialSoilSaturationRatio == "" || File.Exists(watershed.mFPN_initialSoilSaturationRatio) == false)
                        cell.InitialSaturation = ups.iniSaturation;
                    else
                    {
                    }
                    if (cell.FlowType == cGRM.CellFlowType.ChannelFlow || cell.LandCoverCode == cSetLandcover.LandCoverCode.WATR || cell.LandCoverCode == cSetLandcover.LandCoverCode.WTLD)
                        cell.soilSaturationRatio = 1;
                    else { cell.soilSaturationRatio = cell.InitialSaturation; }

                    cell.UKType = cGRM.UnSaturatedKType.Linear;
                    if (ups.UKType.ToLower() == cGRM.UnSaturatedKType.Linear.ToString().ToLower())
                    { cell.UKType = cGRM.UnSaturatedKType.Linear; }
                    if (ups.UKType.ToLower() == cGRM.UnSaturatedKType.Exponential.ToString().ToLower())
                    { cell.UKType = cGRM.UnSaturatedKType.Exponential; }
                    if (ups.UKType.ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower())
                    { cell.UKType = cGRM.UnSaturatedKType.Constant; }

                    cell.coefUK = ups.coefUK;
                    cell.porosityEta = cell.PorosityEtaOri * ups.ccPorosity;
                    if (cell.porosityEta >= 1)
                        cell.porosityEta = 0.99;
                    if (cell.porosityEta <= 0)
                        cell.porosityEta = 0.01;
                    cell.effectivePorosityThetaE = cell.EffectivePorosityThetaEori * ups.ccPorosity;   // 유효 공극율의 보정은 공극률 보정계수를 함께 사용한다.
                    if (cell.effectivePorosityThetaE >= 1)
                        cell.effectivePorosityThetaE = 0.99;
                    if (cell.effectivePorosityThetaE <= 0)
                        cell.effectivePorosityThetaE = 0.01;
                    cell.wettingFrontSuctionHeadPsi_m = cell.WettingFrontSuctionHeadPsiOri_m * ups.ccWFSuctionHead;
                    cell.hydraulicConductK_mPsec = cell.HydraulicConductKori_mPsec * ups.ccHydraulicK;
                    cell.soilDepth_m = cell.SoilDepthOri_m * ups.ccSoilDepth;
                    cell.SoilDepthEffectiveAsWaterDepth_m = cell.soilDepth_m * cell.effectivePorosityThetaE;
                    cell.soilWaterContent_m = cell.SoilDepthEffectiveAsWaterDepth_m * cell.soilSaturationRatio;
                    cell.soilWaterContent_tM1_m = cell.soilWaterContent_m;

                    cell.SoilDepthToBedrock_m = cGRM.CONST_DEPTH_TO_BEDROCK; // 암반까지의 깊이를 20m로 가정, 산악지역에서는 5m
                    if (cell.LandCoverCode == cSetLandcover.LandCoverCode.FRST)
                        cell.SoilDepthToBedrock_m = cGRM.CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN;
                }
            }

            // Flow control
            if (generalSimulEnv.mbSimulateFlowControl == true && fcGrid.FCCellCount > 0)
            {
                foreach (int cvid in fcGrid.FCGridCVidList)
                {
                    DataRow[] rows = fcGrid.mdtFCGridInfo.Select("CVID = " + cvid);
                    Dataset.GRMProject.FlowControlGridRow row;
                    row = (Dataset.GRMProject.FlowControlGridRow)rows[0];
                    switch (row.ControlType)
                    {
                        case  nameof(cFlowControl.FlowControlType.Inlet):
                            {
                                CVs[cvid - 1].FCType = cFlowControl.FlowControlType.Inlet;
                                break;
                            }

                        case nameof(cFlowControl.FlowControlType.ReservoirOperation):
                            {
                                CVs[cvid - 1].FCType = cFlowControl.FlowControlType.ReservoirOperation;
                                break;
                            }

                        case nameof(cFlowControl.FlowControlType.ReservoirOutflow):
                            {
                                CVs[cvid - 1].FCType = cFlowControl.FlowControlType.ReservoirOutflow;
                                break;
                            }

                        case nameof(cFlowControl.FlowControlType.SinkFlow):
                            {
                                CVs[cvid - 1].FCType = cFlowControl.FlowControlType.SinkFlow;
                                break;
                            }

                        case nameof(cFlowControl.FlowControlType.SourceFlow):
                            {
                                CVs[cvid - 1].FCType = cFlowControl.FlowControlType.SourceFlow;
                                break;
                            }

                        default:
                            {
                                throw new InvalidDataException();
                            }
                    }
                }
            }

            // Inlet
            if (generalSimulEnv.mbSimulateFlowControl == true && fcGrid.InletExisted)
            {
                bool bEnded = false;
                List<int> lBaseCVid;
                List<int> lNewCVid;
                lBaseCVid = new List<int>();
                lBaseCVid = fcGrid.InletCVidList;
                while (!bEnded == true)
                {
                    lNewCVid = new List<int>();
                    bEnded = true;
                    foreach (int cvidBase in lBaseCVid)
                    {
                        int cvan = cvidBase - 1;
                        if (CVs[cvan].NeighborCVidFlowIntoMe.Count > 0)
                        {
                            bEnded = false;
                            foreach (int cvidFlowIntoMe in CVs[cvan].NeighborCVidFlowIntoMe)
                            {
                                CVs[cvidFlowIntoMe - 1].toBeSimulated = -1;
                                lNewCVid.Add(cvidFlowIntoMe);
                            }
                        }
                    }
                    lBaseCVid = new List<int>();
                    lBaseCVid = lNewCVid;
                }
            }
        }


        /// <summary>
        ///   업데이트된 하폭 정보를 이용해서 wp 격자의 celltype을 업데이트 하고,
        ///   WP를 기준으로 상류의 셀을 검색해서 모든 격자에 대해서 하류에 있는 wp 정보를 설정한다.
        ///   각 cv에 대해서 맨 마지막에 추가된 wpcvid가 해당 cv와 가장 가까운 wp임.
        ///   </summary>
        ///   <remarks></remarks>
        public void UpdateDownstreamWPforAllCVs()
        {
            List<int> lCVidBase;
            List<int> lCVidNew;
            lCVidBase = new List<int>();

            foreach (Dataset.GRMProject.WatchPointsRow row in watchPoint.mdtWatchPointInfo.Rows)
            {
                int nowCVid = row.CVID;
                int nowAN = nowCVid - 1;
                // 출발점은 현재의 wp 격자
                lCVidBase.Add(nowCVid);
                CVs[nowAN].DownStreamWPCVids.Add(nowCVid);
                bool bEnded = false;
                while (!bEnded == true)
                {
                    lCVidNew = new List<int>();
                    bEnded = true;
                    foreach (int cvidBase in lCVidBase)
                    {
                        nowAN = cvidBase - 1;
                        if (CVs[nowAN].NeighborCVidFlowIntoMe.Count > 0)
                        {
                            bEnded = false;
                            foreach (int cvid in CVs[nowAN].NeighborCVidFlowIntoMe)
                            {
                                CVs[cvid - 1].DownStreamWPCVids.Add(nowCVid);
                                lCVidNew.Add(cvid);
                            }
                        }
                    }
                    lCVidBase = new List<int>();
                    lCVidBase = lCVidNew;
                }
            }
        }


        /// <summary>
        ///   모델링 대상 영역의 검사체적 개수
        ///   </summary>
        ///   <value></value>
        ///   <returns></returns>
        ///   <remarks></remarks>
        public int CVCount
        {
            get
            {
                if (WSCells == null)
                    return -1;
                else
                    return CVs.Length;
            }
        }

        public int FacMax
        {
            get
            {
                if (watershed == null)
                    return -1;
                else
                    return watershed.mFacMax;
            }
        }


        public int FacMin
        {
            get
            {
                if (watershed == null)
                    return -1;
                else
                    return watershed.mFacMin;
            }
        }


        /// <summary>
        ///   출력 시간간격[min]
        ///   </summary>
        ///   <value></value>
        ///   <returns></returns>
        ///   <remarks></remarks>
        public int DTPrintOut
        {
            get
            {
                return generalSimulEnv.mPrintOutTimeStepMIN.Value;
            }
        }


        /// <summary>
        ///   Project file을 읽어서 mProject 변수에 설정
        ///   </summary>
        ///   <param name="prjFPN"></param>
        ///   <remarks></remarks>
        public static bool OpenProject(string prjFPN, bool forceRealTime)
        {
            //try
            //{
                if (string.IsNullOrEmpty(prjFPN) || !File.Exists(prjFPN))
                {
                    throw new FileNotFoundException();
                }
                mProject = new cProject();
                mProject.PrjFile = new Dataset.GRMProject();
                mProject.PrjFile.ReadXml(prjFPN);
                Dataset.GRMProject.ProjectSettingsDataTable dtPrjSettings = mProject.PrjFile.ProjectSettings;
                Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)dtPrjSettings.Rows[0];
            mProject.ProjectNameWithExtension = Path.GetFileName(prjFPN);
                    mProject.ProjectPathName = prjFPN;
                    mProject.ProjectPath = Path.GetDirectoryName(prjFPN);
                    mProject.ProjectNameOnly = Path.GetFileNameWithoutExtension(prjFPN);
                    if (!row.IsGRMSimulationTypeNull())
                    {
                        if (row.GRMSimulationType == cGRM.SimulationType.SingleEvent.ToString())
                            mProject.mSimulationType = cGRM.SimulationType.SingleEvent;
                        else if (row.GRMSimulationType == cGRM.SimulationType.RealTime.ToString())
                            mProject.mSimulationType = cGRM.SimulationType.RealTime;
                    }
                    if (forceRealTime == true)
                        mProject.mSimulationType = cGRM.SimulationType.RealTime;
                    mProject.OFNPDischarge = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + cGRM.CONST_TAG_DISCHARGE);
                    mProject.OFNPDepth = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + cGRM.CONST_TAG_DEPTH);
                    mProject.OFNPRFGrid = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + cGRM.CONST_TAG_RFGRID);
                    mProject.OFNPRFMean = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + cGRM.CONST_TAG_RFMEAN);
                    mProject.OFNPFCData = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + cGRM.CONST_TAG_FCAPP);
                    mProject.OFNPFCStorage = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + cGRM.CONST_TAG_FCSTORAGE);
                    mProject.OFNPSwsPars = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + cGRM.CONST_TAG_SWSPARSTEXTFILE);
                    mProject.OFPSSRDistribution = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_SSR_DIRECTORY_TAG);
                    mProject.OFPRFDistribution = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_RF_DIRECTORY_TAG);
                    mProject.OFPRFAccDistribution = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_RFACC_DIRECTORY_TAG);
                    mProject.OFPFlowDistribution = Path.Combine(mProject.ProjectPath, mProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_FLOW_DIRECTORY_TAG);
                if (mProject.mSimulationType == cGRM.SimulationType.RealTime)
                    changeOutputFileDisk(cRealTime.CONST_Output_File_Target_DISK);
                mProject.generalSimulEnv.GetValues(mProject.PrjFile);
                mProject.subWSPar.GetValues(mProject.PrjFile);
                mProject.watershed.GetValues(mProject.PrjFile);
                mProject.landcover.GetValues(mProject.PrjFile);
                mProject.GreenAmpt.GetValues(mProject.PrjFile);
                mProject.soilDepth.GetValues(mProject.PrjFile);
                mProject.watchPoint.GetValues(mProject.PrjFile);
                mProject.channel.GetValues(mProject.PrjFile);
                // mProject.mEstimatedDist.GetValues(mProject.mPrjFile)

                if (mProject.mSimulationType == cGRM.SimulationType.SingleEvent)
                { mProject.rainfall.GetValues(mProject); }

                if (mProject.generalSimulEnv.mbSimulateFlowControl == true)
                { mProject.fcGrid.GetValues(mProject); }

                sThisSimulation.dtsec = System.Convert.ToInt32(row.ComputationalTimeStep) * 60;
                if (sThisSimulation.dtsec > System.Convert.ToInt32(mProject.generalSimulEnv.mPrintOutTimeStepMIN * 30))
                    sThisSimulation.dtsec = System.Convert.ToInt32(mProject.generalSimulEnv.mPrintOutTimeStepMIN * 30);
                sThisSimulation.IsParallel = false;
                sThisSimulation.MaxDegreeOfParallelism = 0;
                if (row.IsIsParallelNull() == false && row.IsParallel.ToLower() == "true")
                    sThisSimulation.IsParallel = true;
                if (sThisSimulation.IsParallel == true && row.IsMaxDegreeOfParallelismNull() == false)
                {
                    sThisSimulation.MaxDegreeOfParallelism = System.Convert.ToInt32(row.MaxDegreeOfParallelism);
                    if (sThisSimulation.MaxDegreeOfParallelism < -1 || sThisSimulation.MaxDegreeOfParallelism == 0)
                    {
                        cGRM.writelogAndConsole("MaxDegreeOfParallelism value must be equal or greater than -1. And zero is not allowed.  Parallel option could not applied.", cGRM.bwriteLog, true);
                        sThisSimulation.IsParallel = false;
                    }
                }

                sThisSimulation.IsFixedTimeStep = true;
                if (row.IsIsFixedTimeStepNull() == false && row.IsFixedTimeStep.ToLower() == "false")
                    sThisSimulation.IsFixedTimeStep = false;

                mProject.watershed.WSIDList.Clear();
                foreach (int id in mProject.subWSPar.userPars.Keys)
                    mProject.watershed.WSIDList.Add(id);

            cGRM.bwriteLog = false;
            cGRM.fpnlog = prjFPN.Replace(".gmp", ".log");
            if (row.IsWriteLogNull() == false && row.WriteLog.ToString() == "true")
                {
                    cGRM.bwriteLog = true;
                    if (File.Exists(cGRM.fpnlog))
                        File.Delete(cGRM.fpnlog);
                }
                return true;
            //}
            //catch (Exception ex)
            //{
            //    cGRM.writelogAndConsole("Open project failed.", true, true);
            //    Console.WriteLine(ex.ToString());
            //    return false;
            //}
        }

        private static bool changeOutputFileDisk(char targetDisk)
        {
            // bmp 등  생성 경로 disk 조정
            mProject.OFPSSRDistribution = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFPSSRDistribution);
            mProject.OFPRFDistribution = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFPRFDistribution);
            mProject.OFPRFAccDistribution = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFPRFAccDistribution);
            mProject.OFPFlowDistribution = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFPFlowDistribution);
            mProject.OFNPDischarge = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFNPDischarge);
            mProject.OFNPDepth = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFNPDepth);
            mProject.OFNPRFGrid = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFNPRFGrid);
            mProject.OFNPRFMean = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFNPRFMean);
            mProject.OFNPFCData = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFNPFCData);
            mProject.OFNPFCStorage = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFNPFCStorage);
            mProject.OFNPSwsPars = cRealTime_Common.IO_Path_ChangeDrive(targetDisk, mProject.OFNPSwsPars);
            return true;
        }

        public void SaveProject()
        {
            if (Directory.Exists(Path.GetDirectoryName(mProject.ProjectPath)))
            {
                Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)mProject.PrjFile.ProjectSettings.Rows[0];
                row.GRMSimulationType = mProject.mSimulationType.ToString();
                row.SimulStartingTime = mProject.generalSimulEnv.mSimStartDateTime;
                row.ComputationalTimeStep = System.Convert.ToString(sThisSimulation.dtsec / (double)60);
                mProject.generalSimulEnv.SetValues(mProject.PrjFile);
                mProject.subWSPar.SetValues(mProject.PrjFile);
                mProject.watershed.SetValues(mProject.PrjFile);
                mProject.landcover.SetValues(mProject.PrjFile);
                mProject.GreenAmpt.SetValues(mProject.PrjFile);
                mProject.soilDepth.SetValues(mProject.PrjFile);
                mProject.watchPoint.SetValues(mProject.PrjFile);
                mProject.channel.SetValues(mProject.PrjFile);

                mProject.PrjFile.WriteXml(mProject.ProjectPathName);

                if (mProject.mSimulationType == cGRM.SimulationType.SingleEvent)
                    Console.WriteLine(cProject.Current.ProjectPathName + " Is saved.  " + cGRM.BuildInfo.ProductName);
            }
            else
            {
                Console.WriteLine(string.Format("Current project path {0} Is Not exist.   ", Path.GetDirectoryName(mProject.ProjectPath)));
                return;
            }
        }

        public static void SaveAsProject(string prjPathName)
        {
            CopyAndUpdateFiles(mProject, prjPathName);
            SetProjectVariables(mProject, prjPathName);
            cProject.Current.SaveProject();
        }


        private static void CopyAndUpdateFiles(cProject SourceProject, string TargetPathName)
        {
            string TargetProjectPathNameOnly = Path.Combine(Path.GetDirectoryName(TargetPathName), Path.GetFileNameWithoutExtension(TargetPathName));
            string Header = string.Format("Project name : {0} {1} {2}", TargetPathName, "\t", string.Format(DateTime.Now.ToString(), "yyyy/MM/dd HH:mm"));
            // 유량 파일
            if (System.IO.File.Exists(SourceProject.OFNPDischarge) == true)
            {
                File.Copy(SourceProject.OFNPDischarge, TargetProjectPathNameOnly + cGRM.CONST_TAG_DISCHARGE);
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly + cGRM.CONST_TAG_DISCHARGE, TargetProjectPathNameOnly + cGRM.CONST_TAG_DISCHARGE, 1, Header);
            }

            // 수심파일
            if (System.IO.File.Exists(SourceProject.OFNPDepth) == true)
            {
                File.Copy(SourceProject.OFNPDepth, TargetProjectPathNameOnly + cGRM.CONST_TAG_DEPTH);
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly + cGRM.CONST_TAG_DEPTH, TargetProjectPathNameOnly + cGRM.CONST_TAG_DEPTH, 1, Header);
            }

            // 격자 강우량파일
            if (System.IO.File.Exists(SourceProject.OFNPRFGrid) == true)
            {
                File.Copy(SourceProject.OFNPRFGrid, TargetProjectPathNameOnly + cGRM.CONST_TAG_RFGRID);
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly + cGRM.CONST_TAG_RFGRID, TargetProjectPathNameOnly + cGRM.CONST_TAG_RFGRID, 1, Header);
            }

            // 평균 강우량 파일
            if (System.IO.File.Exists(SourceProject.OFNPRFMean) == true)
            {
                File.Copy(SourceProject.OFNPRFMean, TargetProjectPathNameOnly + cGRM.CONST_TAG_RFMEAN);
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly + cGRM.CONST_TAG_RFMEAN, TargetProjectPathNameOnly + cGRM.CONST_TAG_RFMEAN, 1, Header);
            }

            // flow control 적용 정보 파일
            if (System.IO.File.Exists(SourceProject.OFNPFCData) == true)
            {
                File.Copy(SourceProject.OFNPFCData, TargetProjectPathNameOnly + cGRM.CONST_TAG_FCAPP);
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly + cGRM.CONST_TAG_FCAPP, TargetProjectPathNameOnly + cGRM.CONST_TAG_FCAPP, 1, Header);
            }

            // 저수지 파일
            if (System.IO.File.Exists(SourceProject.OFNPFCStorage) == true)
            {
                File.Copy(SourceProject.OFNPFCStorage, TargetProjectPathNameOnly + cGRM.CONST_TAG_FCSTORAGE);
                cTextFile.ReplaceALineInTextFile(TargetProjectPathNameOnly + cGRM.CONST_TAG_FCSTORAGE, TargetProjectPathNameOnly + cGRM.CONST_TAG_FCSTORAGE, 1, Header);
            }

            // WP별 모의결과, MREQ 
            if (SourceProject.watchPoint.WPCount > 0)
            {
                for (int i = 0; i < SourceProject.watchPoint.WPCount; i++)
                {
                    string strWPName = mProject.watchPoint.mdtWatchPointInfo[i].Name.Replace(",", "_");
                    string strWPFName = SourceProject.ProjectPath + @"\" + SourceProject.ProjectNameOnly + "WP_" + strWPName + ".out";
                    string strMREFName = SourceProject.ProjectPath + @"\" + SourceProject.ProjectNameOnly + "MREQ_" + strWPName + ".out";
                    string TargetWPFpn = TargetProjectPathNameOnly + "WP_" + strWPName + ".out";
                    string TargetMREFpn = TargetProjectPathNameOnly + "MREQ_" + strWPName + ".out";
                    if (System.IO.File.Exists(strWPFName) == true)
                    {
                        File.Copy(strWPFName, TargetWPFpn);
                        cTextFile.ReplaceALineInTextFile(TargetWPFpn, TargetWPFpn, 1, Header);
                    }
                    if (System.IO.File.Exists(strMREFName) == true)
                    {
                        File.Copy(strMREFName, TargetMREFpn);
                        cTextFile.ReplaceALineInTextFile(TargetMREFpn, TargetMREFpn, 1, Header);
                    }
                }
            }
        }


        public static void SetProjectVariables(cProject TargetProject, string TargetPrjPathName)
        {
            TargetProject.ProjectNameWithExtension = Path.GetFileName(TargetPrjPathName);
            TargetProject.ProjectNameOnly = Path.GetFileNameWithoutExtension(TargetPrjPathName);
            TargetProject.ProjectPathName = TargetPrjPathName;
            TargetProject.ProjectPath = Path.GetDirectoryName(TargetPrjPathName);
            TargetProject.OFNPDischarge = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + cGRM.CONST_TAG_DISCHARGE);
            TargetProject.OFNPDepth = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + cGRM.CONST_TAG_DEPTH);
            TargetProject.OFNPRFGrid = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + cGRM.CONST_TAG_RFGRID);
            TargetProject.OFNPRFMean = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + cGRM.CONST_TAG_RFMEAN);
            TargetProject.OFNPFCData = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + cGRM.CONST_TAG_FCAPP);
            TargetProject.OFNPFCStorage = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + cGRM.CONST_TAG_FCSTORAGE);
            TargetProject.OFNPSwsPars = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + cGRM.CONST_TAG_SWSPARSTEXTFILE);
            TargetProject.OFPSSRDistribution = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_SSR_DIRECTORY_TAG);
            TargetProject.OFPRFDistribution = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_RF_DIRECTORY_TAG);
            TargetProject.OFPRFAccDistribution = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_RFACC_DIRECTORY_TAG);
            TargetProject.OFPFlowDistribution = Path.Combine(TargetProject.ProjectPath, TargetProject.ProjectNameOnly + "_" + cGRM.CONST_DIST_FLOW_DIRECTORY_TAG);

            if (TargetProject.PrjFile.ProjectSettings.Rows.Count > 0)
            {
                Dataset.GRMProject.ProjectSettingsDataTable dtSettings = TargetProject.PrjFile.ProjectSettings;
                Dataset.GRMProject.ProjectSettingsRow settings = (Dataset.GRMProject.ProjectSettingsRow)dtSettings.Rows[0];
                //settings.ProjectFile = TargetPrjPathName;
            }
        }

        public bool SetupModelParametersAfterProjectFileWasOpened()
        {
            if (cProject.Current.SetBasicCVInfo() == false) { return false; }
            if (cProject.Current.watchPoint.UpdatesWatchPointCVIDs(cProject.Current) == false) { return false; }
            if (mProject.generalSimulEnv.mbSimulateFlowControl == true)
            { cProject.Current.fcGrid.UpdateFCGridInfoAndData(cProject.Current); }
            cProject.Current.UpdateCVbyUserSettings();
            cProject.Current.UpdateDownstreamWPforAllCVs();
            cGRM.Start();
            sThisSimulation.mGRMSetupIsNormal = true;
            if (mProject.mSimulationType != cGRM.SimulationType.RealTime)
            { sThisSimulation.mRFDataCountInThisEvent = mProject.rainfall.mlstRainfallData.Count; }
            return true;
        }


        public static bool ValidateProjectFile(cProject prj)
        {
            Dataset.GRMProject.ProjectSettingsRow r = (Dataset.GRMProject.ProjectSettingsRow)prj.PrjFile.ProjectSettings.Rows[0];

            if (r.IsGRMSimulationTypeNull() || r.GRMSimulationType == "")
            {
                Console.WriteLine(string.Format("GRMSimulationType is invalid!!"));
                return false;
            }

            if (!r.IsWatershedFileNull() && r.WatershedFile != "")
            {
                if (File.Exists(r.WatershedFile) == false)
                {
                    Console.WriteLine(string.Format("Watershed file is not exist!! {0} {1}", "\r\n", r.WatershedFile));
                    return false;
                }
            }
            else
            {
                Console.WriteLine(string.Format("Watershed file is not exist!! {0} {1}", "\r\n", r.WatershedFile));
                return false;
            }

            if (!r.IsSlopeFileNull() && r.SlopeFile != "")
            {
                if (File.Exists(r.SlopeFile) == false)
                {
                    Console.WriteLine(string.Format("Slope file is not exist!! {0} {1}", "\r\n", r.SlopeFile));
                    return false;
                }
            }
            else
            {
                Console.WriteLine(string.Format("Slope file is not exist!! {0} {1}", "\r\n", r.SlopeFile));
                return false;
            }

            if (!r.IsFlowDirectionFileNull() && r.FlowDirectionFile != "")
            {
                if (File.Exists(r.FlowDirectionFile) == false)
                {
                    Console.WriteLine(string.Format("Flow direction file is not exist!! {0} {1}", "\r\n", r.FlowDirectionFile));
                    return false;
                }
            }
            else
            {
                Console.WriteLine(string.Format("Flow direction file is not exist!! {0} {1}", "\r\n", r.FlowDirectionFile));
                return false;
            }

            if (!r.IsFlowAccumFileNull() && r.FlowAccumFile != "")
            {
                if (File.Exists(r.FlowAccumFile) == false)
                {
                    Console.WriteLine(string.Format("Flow accumulation file is not exist!! {0} {1}", "\r\n", r.FlowAccumFile));
                    return false;
                }
            }
            else
            {
                Console.WriteLine(string.Format("Flow accumulation file is not exist!! {0} {1}", "\r\n", r.FlowAccumFile));
                return false;
            }

            if (!r.IsStreamFileNull() && r.StreamFile != "")
            {
                if (File.Exists(r.StreamFile) == false)
                {
                    Console.WriteLine(string.Format("Stream file is not exist!! {0} {1}", "\r\n", r.StreamFile));
                    return false;
                }
            }

            if (!r.IsChannelWidthFileNull() && r.ChannelWidthFile != "")
            {
                if (File.Exists(r.ChannelWidthFile) == false)
                {
                    Console.WriteLine(string.Format("Channel width file is not exist!! {0} {1}", "\r\n", r.ChannelWidthFile));
                    return false;
                }
            }

            if (r.IsLandCoverDataTypeNull() || r.LandCoverDataType == "")
            {
                Console.WriteLine(string.Format("LandCoverDataType is invalid!!"));
                return false;
            }

            if (!r.IsLandCoverFileNull() && r.LandCoverFile != "")
            {
                if (File.Exists(r.LandCoverFile) == false)
                {
                    Console.WriteLine(string.Format("Land cover file is not exist!! {0} {1}", "\r\n", r.LandCoverFile));
                    return false;
                }
                else if (!r.IsLandCoverVATFileNull() && r.LandCoverVATFile != "")
                {
                    if (File.Exists(r.LandCoverVATFile) == false)
                    {
                        Console.WriteLine(string.Format("Land cover VAT file is not exist!! {0} {1}", "\r\n", r.LandCoverVATFile));
                        return false;
                    }
                }
                else
                {
                    Console.WriteLine(string.Format("Land cover VAT file is invalid!!"));
                    return false;
                }
            }

            if (!r.IsConstantRoughnessCoeffNull() && r.ConstantRoughnessCoeff != "")
            {
                double v=0;
                if (double.TryParse(r.ConstantRoughnessCoeff, out v) == false)
                {
                    Console.WriteLine(string.Format("ConstantRoughnessCoeff is invalid!! {0} {1}", "\r\n", r.ConstantRoughnessCoeff));
                    return false;
                }
            }

            if (!r.IsConstantImperviousRatioNull() && r.ConstantImperviousRatio != "")
            {
                double v;
                if (double.TryParse(r.ConstantImperviousRatio, out v) == false)
                {
                    Console.WriteLine(string.Format("ConstantImperviousRatio is invalid!! {0} {1}", "\r\n", r.ConstantImperviousRatio));
                    return false;
                }
            }

            if (r.IsSoilTextureDataTypeNull() || r.SoilTextureDataType == "")
            {
                Console.WriteLine(string.Format("SoilTextureDataType is invalid!!"));
                return false;
            }

            if (!r.IsSoilTextureFileNull() && r.SoilTextureFile != "")
            {
                if (File.Exists(r.SoilTextureFile) == false)
                {
                    Console.WriteLine(string.Format("Soil texture file is not exist!! {0} {1}", "\r\n", r.SoilTextureFile));
                    return false;
                }
                else if (!r.IsSoilTextureVATFileNull() && r.SoilTextureVATFile != "")
                {
                    if (File.Exists(r.SoilTextureVATFile) == false)
                    {
                        Console.WriteLine(string.Format("Soil texture VAT file is not exist!! {0} {1}", "\r\n", r.SoilTextureVATFile));
                        return false;
                    }
                }
                else
                {
                    Console.WriteLine(string.Format("Soil texture VAT file is invalid!!"));
                    return false;
                }
            }

            if (!r.IsConstantSoilPorosityNull() && r.ConstantSoilPorosity != "")
            {
                double v=0;
                if (double.TryParse(r.ConstantSoilPorosity, out v) == false)
                {
                    Console.WriteLine(string.Format("ConstantSoilPorosity is invalid!! {0} {1}", "\r\n", r.ConstantSoilPorosity));
                    return false;
                }
            }

            if (!r.IsConstantSoilEffPorosityNull() && r.ConstantSoilEffPorosity != "")
            {
                double v = 0;
                if (double.TryParse(r.ConstantSoilEffPorosity, out v) == false)
                {
                    Console.WriteLine(string.Format("ConstantSoilEffPorosity is invalid!! {0} {1}", "\r\n", r.ConstantSoilEffPorosity));
                    return false;
                }
            }

            if (!r.IsConstantSoilWettingFrontSuctionHeadNull() && r.ConstantSoilWettingFrontSuctionHead != "")
            {
                double v = 0;
                if (double.TryParse(r.ConstantSoilWettingFrontSuctionHead, out v) == false)
                {
                    Console.WriteLine(string.Format("ConstantSoilWettingFrontSuctionHead is invalid!! {0} {1}", "\r\n", r.ConstantSoilWettingFrontSuctionHead));
                    return false;
                }
            }

            if (!r.IsConstantSoilHydraulicConductivityNull() && r.ConstantSoilHydraulicConductivity != "")
            {
                double v = 0;
                if (double.TryParse(r.ConstantSoilHydraulicConductivity, out v) == false)
                {
                    Console.WriteLine(string.Format("ConstantSoilHydraulicConductivity is invalid!! {0} {1}", "\r\n", r.ConstantSoilHydraulicConductivity));
                    return false;
                }
            }

            if (r.IsSoilDepthDataTypeNull() || r.SoilDepthDataType == "")
            {
                Console.WriteLine(string.Format("SoilDepthDataType is invalid!!"));
                return false;
            }

            if (!r.IsSoilDepthFileNull() && r.SoilTextureFile != "")
            {
                if (File.Exists(r.SoilDepthFile) == false)
                {
                    Console.WriteLine(string.Format("Soil depth file is not exist!! {0} {1}", "\r\n", r.SoilTextureFile));
                    return false;
                }
                else if (!r.IsSoilDepthVATFileNull() && r.SoilDepthVATFile != "")
                {
                    if (File.Exists(r.SoilDepthVATFile) == false)
                    {
                        Console.WriteLine(string.Format("Soil depth VAT file is not exist!! {0} {1}", "\r\n", r.SoilDepthVATFile));
                        return false;
                    }
                }
                else
                {
                    Console.WriteLine(string.Format("Soil depth VAT file is invalid!!"));
                    return false;
                }
            }

            if (!r.IsConstantSoilDepthNull() && r.ConstantSoilDepth != "")
            {
                double v = 0;
                if (double.TryParse(r.ConstantSoilDepth, out v) == false)
                {
                    Console.WriteLine(string.Format("ConstantSoilDepth is invalid!! {0} {1}", "\r\n", r.ConstantSoilDepth));
                    return false;
                }
            }

            if (!r.IsInitialSoilSaturationRatioFileNull() && r.InitialSoilSaturationRatioFile != "")
            {
                if (File.Exists(r.InitialSoilSaturationRatioFile) == false)
                {
                    Console.WriteLine(string.Format("InitialSoilSaturationRatioFile is not exist!! {0} {1}", "\r\n", r.InitialSoilSaturationRatioFile));
                    return false;
                }
            }

            if (!r.IsInitialChannelFlowFileNull() && r.InitialChannelFlowFile != "")
            {
                if (File.Exists(r.InitialChannelFlowFile) == false)
                {
                    Console.WriteLine(string.Format("InitialChannelFlowFile is not exist!! {0} {1}", "\r\n", r.InitialChannelFlowFile));
                    return false;
                }
            }

            if (r.IsRainfallDataTypeNull() || r.RainfallDataType == "")
            {
                Console.WriteLine(string.Format("RainfallDataType is invalid!!"));
                return false;
            }

            if (!r.IsRainfallIntervalNull() && r.RainfallInterval != "")
            {
                int v=0;
                if (int.TryParse(r.RainfallInterval, out v) == false)
                {
                    Console.WriteLine(string.Format("Rainfall data interval is invalid!! {0} {1}", "\r\n", r.RainfallInterval));
                    return false;
                }
            }
            else
                return false;

            if (!r.IsRainfallDataFileNull() && r.RainfallDataFile != "")
            {
                if (File.Exists(r.RainfallDataFile) == false)
                {
                    Console.WriteLine(string.Format("Rainfall data file is not exist!! {0} {1}", "\r\n", r.RainfallDataFile));
                    return false;
                }
            }
            else
                return false;

            if (r.IsFlowDirectionTypeNull() || r.FlowDirectionType == "")
            {
                Console.WriteLine(string.Format("FlowDirectionType is invalid!!"));
                return false;
            }

            // If Not r.IsGridCellSizeNull() AndAlso r.GridCellSize <> "" Then
            // Dim v As Single
            // If Single.TryParse(r.GridCellSize, v) = False Then
            // Console.WriteLine(String.Format("Grid cell size is invalid!! {0} {1}", vbCrLf, r.GridCellSize))
            // Return False
            // End If
            // Else
            // Return False
            // End If

            if (!r.IsIsParallelNull() && r.IsParallel == "")
            {
                Console.WriteLine(string.Format("IsParallel option is invalid!!"));
                return false;
            }

            if (!r.IsMaxDegreeOfParallelismNull() && r.MaxDegreeOfParallelism != "")
            {
                int v=0;
                if (int.TryParse(r.MaxDegreeOfParallelism, out v) == false)
                {
                    Console.WriteLine(string.Format("Grid cell size is invalid!! {0} {1}", "\r\n", r.MaxDegreeOfParallelism));
                    return false;
                }
            }

            // If r.IsSimulStartingTimeNull() OrElse r.SimulStartingTime = "" Then
            // Console.WriteLine(String.Format("Simulation starting time is invalid!! Simulation starting time will be set to '0'"))
            // 'Return False
            // End If

            if (!r.IsSimulationDurationNull() && r.SimulationDuration != "")
            {
                int v=0;
                if (int.TryParse(r.SimulationDuration, out v) == false)
                {
                    Console.WriteLine(string.Format("Simulation duration is invalid!! {0} {1}", "\r\n", r.SimulationDuration));
                    return false;
                }
            }
            else
                return false;

            if (!r.IsComputationalTimeStepNull() && r.ComputationalTimeStep != "")
            {
                int v=0;
                if (int.TryParse(r.ComputationalTimeStep, out v) == false)
                {
                    Console.WriteLine(string.Format("Computational time step is invalid!! {0} {1}", "\r\n", r.ComputationalTimeStep));
                    return false;
                }
            }
            else
                return false;

            if (!r.IsIsFixedTimeStepNull() && r.IsFixedTimeStep == "")
            {
                Console.WriteLine(string.Format("IsFixedTimeStep is invalid!!"));
                return false;
            }

            if (!r.IsOutputTimeStepNull() && r.OutputTimeStep != "")
            {
                int v=0;
                if (int.TryParse(r.OutputTimeStep, out v) == false)
                {
                    Console.WriteLine(string.Format("Output time step is invalid!! {0} {1}", "\r\n", r.OutputTimeStep));
                    return false;
                }
            }
            else
                return false;

            if (!r.IsCrossSectionTypeNull() && r.CrossSectionType == "")
            {
                Console.WriteLine(string.Format("CrossSectionType is invalid!!"));
                return false;
            }

            if (!r.IsSingleCSChannelWidthTypeNull() && r.SingleCSChannelWidthType == "")
            {
                Console.WriteLine(string.Format("SingleCSChannelWidthType is invalid!!"));
                return false;
            }

            if (!r.IsChannelWidthEQcNull() && r.ChannelWidthEQc != "")
            {
                double v = 0;
                if (double.TryParse(r.ChannelWidthEQc, out v) == false)
                {
                    Console.WriteLine(string.Format("ChannelWidthEQc is invalid!! {0} {1}", "\r\n", r.ChannelWidthEQc));
                    return false;
                }
            }

            if (!r.IsChannelWidthEQdNull() && r.ChannelWidthEQd != "")
            {
                double v = 0;
                if (double.TryParse(r.ChannelWidthEQd, out v) == false)
                {
                    Console.WriteLine(string.Format("ChannelWidthEQd is invalid!! {0} {1}", "\r\n", r.ChannelWidthEQd));
                    return false;
                }
            }

            if (!r.IsChannelWidthEQeNull() && r.ChannelWidthEQe != "")
            {
                double v = 0;
                if (double.TryParse(r.ChannelWidthEQe, out v) == false)
                {
                    Console.WriteLine(string.Format("ChannelWidthEQe is invalid!! {0} {1}", "\r\n", r.ChannelWidthEQe));
                    return false;
                }
            }

            if (!r.IsChannelWidthMostDownStreamNull() && r.ChannelWidthMostDownStream != "")
            {
                double v = 0;
                if (double.TryParse(r.ChannelWidthMostDownStream, out v) == false)
                {
                    Console.WriteLine(string.Format("ChannelWidthMostDownStream is invalid!! {0} {1}", "\r\n", r.ChannelWidthMostDownStream));
                    return false;
                }
            }

            if (!r.IsLowerRegionHeightNull() && r.LowerRegionHeight != "")
            {
                double v = 0;
                if (double.TryParse(r.LowerRegionHeight, out v) == false)
                {
                    Console.WriteLine(string.Format("LowerRegionHeight is invalid!! {0} {1}", "\r\n", r.LowerRegionHeight));
                    return false;
                }
            }

            if (!r.IsLowerRegionBaseWidthNull() && r.LowerRegionBaseWidth != "")
            {
                double v = 0;
                if (double.TryParse(r.LowerRegionBaseWidth, out v) == false)
                {
                    Console.WriteLine(string.Format("LowerRegionBaseWidth is invalid!! {0} {1}", "\r\n", r.LowerRegionBaseWidth));
                    return false;
                }
            }

            if (!r.IsUpperRegionBaseWidthNull() && r.UpperRegionBaseWidth != "")
            {
                double v = 0;
                if (double.TryParse(r.UpperRegionBaseWidth, out v) == false)
                {
                    Console.WriteLine(string.Format("UpperRegionBaseWidth is invalid!! {0} {1}", "\r\n", r.UpperRegionBaseWidth));
                    return false;
                }
            }

            // If Not r.IsCompoundCSIniFlowDepthNull() AndAlso r.CompoundCSIniFlowDepth <> "" Then
            // Dim v As Single
            // If Single.TryParse(r.CompoundCSIniFlowDepth, v) = False Then
            // Console.WriteLine(String.Format("CompoundCSIniFlowDepth is invalid!! {0} {1}", vbCrLf, r.CompoundCSIniFlowDepth))
            // Return False
            // End If
            // End If

            if (!r.IsCompoundCSChannelWidthLimitNull() && r.CompoundCSChannelWidthLimit != "")
            {
                double v = 0;
                if (double.TryParse(r.CompoundCSChannelWidthLimit, out v) == false)
                {
                    Console.WriteLine(string.Format("CompoundCSChannelWidthLimit is invalid!! {0} {1}", "\r\n", r.CompoundCSChannelWidthLimit));
                    return false;
                }
            }

            if (!r.IsBankSideSlopeRightNull() && r.BankSideSlopeRight != "")
            {
                double v = 0;
                if (double.TryParse(r.BankSideSlopeRight, out v) == false)
                {
                    Console.WriteLine(string.Format("BankSideSlopeRight is invalid!! {0} {1}", "\r\n", r.BankSideSlopeRight));
                    return false;
                }
            }

            if (!r.IsBankSideSlopeLeftNull() && r.BankSideSlopeLeft != "")
            {
                double v = 0;
                if (double.TryParse(r.BankSideSlopeLeft, out v) == false)
                {
                    Console.WriteLine(string.Format("BankSideSlopeLeft is invalid!! {0} {1}", "\r\n", r.BankSideSlopeLeft));
                    return false;
                }
            }

            if (r.IsSimulateInfiltrationNull() || r.SimulateInfiltration == "")
            {
                Console.WriteLine(string.Format("SimulateInfiltration option  is invalid!!"));
                return false;
            }

            if (r.IsSimulateSubsurfaceFlowNull() || r.SimulateSubsurfaceFlow == "")
            {
                Console.WriteLine(string.Format("SimulateSubsurfaceFlow option  is invalid!!"));
                return false;
            }

            if (r.IsSimulateBaseFlowNull() || r.SimulateBaseFlow == "")
            {
                Console.WriteLine(string.Format("SimulateBaseFlow option  is invalid!!"));
                return false;
            }

            if (r.IsSimulateFlowControlNull() || r.SimulateFlowControl == "")
            {
                Console.WriteLine(string.Format("SimulateFlowControl option  is invalid!!"));
                return false;
            }

            if (r.IsMakeIMGFileNull() || r.MakeIMGFile == "")
            {
                Console.WriteLine(string.Format("MakeIMGFile option  is invalid!!"));
                return false;
            }

            if (r.IsMakeASCFileNull() || r.MakeASCFile == "")
            {
                Console.WriteLine(string.Format("MakeASCFile option  is invalid!!"));
                return false;
            }

            if (r.IsMakeSoilSaturationDistFileNull() || r.MakeSoilSaturationDistFile == "")
            {
                Console.WriteLine(string.Format("MakeSoilSaturationDistFile option  is invalid!!"));
                return false;
            }

            if (r.IsMakeRfDistFileNull() || r.MakeRfDistFile == "")
            {
                Console.WriteLine(string.Format("MakeRfDistFile option  is invalid!!"));
                return false;
            }

            if (r.IsMakeRFaccDistFileNull() || r.MakeRFaccDistFile == "")
            {
                Console.WriteLine(string.Format("MakeRFaccDistFile option  is invalid!!"));
                return false;
            }

            if (r.IsMakeFlowDistFileNull() || r.MakeFlowDistFile == "")
            {
                Console.WriteLine(string.Format("MakeFlowDistFile option  is invalid!!"));
                return false;
            }

            if (r.IsPrintOptionNull() || r.PrintOption == "")
            {
                Console.WriteLine(string.Format("PrintOption is invalid!!"));
                return false;
            }

            if (!r.IsWriteLogNull() && r.WriteLog == "")
            {
                Console.WriteLine(string.Format("WriteLog option  is invalid!!"));
                return false;
            }
            return true;
        }
    }
}
