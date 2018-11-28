using System;
using System.Collections.Generic;
using System.IO;

namespace GRMCore
{
   public class cGetWatershedInfo
    {
        public cProject grmPrj = new cProject();
        public cGRM.FlowDirectionType mfdType;
        public string mstreamFPN;
        public string mlandCoverFPN;
        public string msoilTextureFPN;
        public string msoilDepthFPN;
        public string miniSoilSaturationFPN;
        public string miniChannelFlowFPN;

        public cGetWatershedInfo(string fdirType, string watershedFPN, string slopeFPN, string fdirFPN, string facFPN, string streamFPN = "", string landCoverFPN = "", string soilTextureFPN = "", string soilDepthFPN = "", string iniSoilSaturationFPN = "", string iniChannelFlowFPN = "")
        {
            // Console.WriteLine(watershedFPN + " watershedFPN instancing argument file")
            // Console.WriteLine(File.Exists(watershedFPN).ToString)
            // Console.WriteLine(slopeFPN + " slopeFPN instancing argument file")
            // Console.WriteLine(File.Exists(slopeFPN).ToString)
            // Console.WriteLine(fdirFPN + " fdirFPN instancing argument file")
            // Console.WriteLine(File.Exists(fdirFPN).ToString)
            // Console.WriteLine(facFPN + " facFPN instancing argument file")
            // Console.WriteLine(File.Exists(facFPN).ToString)
            // Console.WriteLine(streamFPN + " streamFPN instancing argument file")
            // Console.WriteLine(File.Exists(streamFPN).ToString)
            // Console.WriteLine(landCoverFPN + " landCoverFPN instancing argument file")
            // Console.WriteLine(File.Exists(landCoverFPN).ToString)
            // Console.WriteLine(soilTextureFPN + " soilTextureFPN instancing argument file")
            // Console.WriteLine(File.Exists(soilTextureFPN).ToString)
            // Console.WriteLine(soilDepthFPN + " soilDepthFPN instancing argument file")
            // Console.WriteLine(File.Exists(soilDepthFPN).ToString)
            // Console.WriteLine(iniSoilSaturationFPN + " iniSoilSaturationFPN instancing argument file")
            // Console.WriteLine(File.Exists(iniSoilSaturationFPN).ToString)
            // Console.WriteLine(iniChannelFlowFPN + " iniChannelFlowFPN instancing argument file")
            // Console.WriteLine(File.Exists(iniChannelFlowFPN).ToString)
            cReadGeoFileAndSetInfo.ReadLayerWSandSetBasicInfo(watershedFPN, grmPrj.watershed, ref grmPrj.WSCells, ref grmPrj.CVs , ref grmPrj.dmInfo,
                ref grmPrj.WSNetwork, ref grmPrj .subWSPar);
            //grmPrj.WSCells=cReadGeoFileAndSetInfo.ReadLayerWSandSetBasicInfo(watershedFPN, grmPrj.watershed);
            //grmPrj.WSNetwork = new cWatershedNetwork(grmPrj.watershed.WSIDList);
            //grmPrj.subWSPar.SetSubWSkeys(grmPrj.watershed.WSIDList);
            cReadGeoFileAndSetInfo.ReadLayerSlope(slopeFPN, 
                grmPrj.WSCells, grmPrj .watershed .colCount , grmPrj .watershed.rowCount,
                true);

            switch (fdirType)
            {
                case  nameof(cGRM.FlowDirectionType.StartsFromN):
                    {
                        mfdType = cGRM.FlowDirectionType.StartsFromN;
                        break;
                    }

                case nameof(cGRM.FlowDirectionType.StartsFromNE):
                    {
                        mfdType = cGRM.FlowDirectionType.StartsFromNE;
                        break;
                    }

                case nameof(cGRM.FlowDirectionType.StartsFromE):
                    {
                        mfdType = cGRM.FlowDirectionType.StartsFromE;
                        break;
                    }

                case nameof(cGRM.FlowDirectionType.StartsFromE_TauDEM):
                    {
                        mfdType = cGRM.FlowDirectionType.StartsFromE_TauDEM;
                        break;
                    }

                default:
                    {
                        mfdType = cGRM.FlowDirectionType.StartsFromE_TauDEM;
                        break;
                    }
            }
            cReadGeoFileAndSetInfo.ReadLayerFdir(fdirFPN,
                grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount, mfdType, true);
            cReadGeoFileAndSetInfo.ReadLayerFAcc(facFPN, grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount, true);
            if (streamFPN != "" && File.Exists(streamFPN))
            {
                cReadGeoFileAndSetInfo.ReadLayerStream(streamFPN, grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount, true);
                mstreamFPN = streamFPN;
            }
            if (landCoverFPN != "" && File.Exists(landCoverFPN))
            {
                cReadGeoFileAndSetInfo.ReadLandCoverFile(landCoverFPN, 
                    grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount,                    true);
                mlandCoverFPN = landCoverFPN;
            }
            if (soilTextureFPN != "" && File.Exists(soilTextureFPN))
            {
                cReadGeoFileAndSetInfo.ReadSoilTextureFile(soilTextureFPN,
                      grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount, true);
                msoilTextureFPN = soilTextureFPN;
            }
            if (soilDepthFPN != "" && File.Exists(soilDepthFPN))
            {
                cReadGeoFileAndSetInfo.ReadSoilDepthFile(soilDepthFPN,
                      grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount, true);
                msoilDepthFPN = soilDepthFPN;
            }
            if (iniSoilSaturationFPN != "" && File.Exists(iniSoilSaturationFPN))
            {
                cReadGeoFileAndSetInfo.ReadLayerInitialSoilSaturation(iniSoilSaturationFPN,
                      grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount, true);
                miniSoilSaturationFPN = iniSoilSaturationFPN;
            }
            if (iniChannelFlowFPN != "" && File.Exists(iniChannelFlowFPN))
            {
                cReadGeoFileAndSetInfo.ReadLayerInitialChannelFlow(iniChannelFlowFPN,
                      grmPrj.WSCells, grmPrj.watershed.colCount, grmPrj.watershed.rowCount, true);
                miniChannelFlowFPN = iniChannelFlowFPN;
            }
            grmPrj.SetGridNetworkFlowInformation();
            grmPrj.InitControlVolumeAttribute();
        }

        public cGetWatershedInfo(string gmpFPN)
        {
            try
            {
                cProject.OpenProject(gmpFPN, false);
                grmPrj = cProject.Current;
                if (cProject.Current.SetupModelParametersAfterProjectFileWasOpened() == false)
                {
                    cGRM.writelogAndConsole("GRM setup was failed !!!", true, true);
                    return;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        public bool IsInWatershedArea(int colXArrayIdx, int rowYArrayIdx)
        {
            int id = grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].WSID;
            if (id > 0)
                return true;
            else
                return false;
        }

        public int mostDownStreamCellArrayXColPosition()
        {
            return grmPrj.dmInfo[grmPrj.mMostDownCellArrayNumber].XCol;
        }

        public int mostDownStreamCellArrayYRowPosition()
        {
            return grmPrj.dmInfo[grmPrj.mMostDownCellArrayNumber].YRow;
        }

        /// <summary>
        ///   Watershed ID list.
        ///   </summary>
        ///   <returns></returns>
        public int[] WSIDsAll()
        {
            return grmPrj.WSNetwork.WSIDsAll.ToArray();
        }

        /// <summary>
        ///   Watershed count.
        ///   </summary>
        ///   <returns></returns>
        public int WScount()
        {
            return grmPrj.WSNetwork.WSIDsAll.Count;
        }

        // 2018.06.26. 최
        // 이것을 리스트로 바꾼다. 
        // 연결되지 않는 여러개 유역 영역을 가지는 정보에서 모든 최하류 유역 id를 받는다.
        public List<int> mostDownStreamWSIDs()
        {
            return grmPrj.WSNetwork.MostDownstreamWSIDs;
        }

        public List<int> upStreamWSIDs(int currentWSID)
        {
            return grmPrj.WSNetwork.WSIDsAllUps(currentWSID);
        }

        public int upStreamWSCount(int currentWSID)
        {
            return grmPrj.WSNetwork.WSIDsAllUps(currentWSID).Count;
        }

        public List<int> downStreamWSIDs(int currentWSID)
        {
            return grmPrj.WSNetwork.WSIDsAllDowns(currentWSID);
        }

        public int downStreamWSCount(int currentWSID)
        {
            return grmPrj.WSNetwork.WSIDsAllDowns(currentWSID).Count;
        }

        public int watershedID(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].WSID;
            else
                return 0;
        }

        public int cellCountInWatershed()
        {
            return grmPrj.CVCount;
        }

        public string flowDirection(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].FDir.ToString();
            else
                return null;
        }

        public int flowAccumulation(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].FAc;
            else
                return -1;
        }

        public double slope(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].Slope;
            else
                return default(Double);
        }

        public int streamValue(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true && mstreamFPN != "")
            {
                if (grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].IsStream)
                    return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].mStreamAttr.ChStrOrder;
            }
            return -1;
        }

        public string cellFlowType(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].FlowType.ToString();
            else
                return null;
        }

        public int landCoverValue(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].LandCoverValue;
            else
                return -1;
        }

        public int soilTextureValue(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].SoilTextureValue;
            else
                return -1;
        }

        public int soilDepthValue(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
                return grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].SoilDepthTypeValue;
            else
                return -1;
        }

        /// <summary>
        ///    Select all cells in upstream area of a input cell position. Return string array of cell positions - "column, row".
        ///   </summary>
        ///   <param name="colXArrayIdx"></param>
        ///   <param name="rowYArrayIdx"></param>
        ///   <returns></returns>
        public string[] allCellsInUpstreamArea(int colXArrayIdx, int rowYArrayIdx)
        {
            if (IsInWatershedArea(colXArrayIdx, rowYArrayIdx) == true)
            {
                List<int> cvids = new List<int>();
                int startingBaseCVID = grmPrj.WSCells[colXArrayIdx, rowYArrayIdx].CVID;
                cvids = grmPrj.getAllUpstreamCells(startingBaseCVID);
                if (cvids != null)
                {
                    string[] cellsArray = new string[cvids.Count - 1 + 1];
                    int idx = 0;
                    foreach (int cvid in cvids)
                    {
                        int colx = grmPrj.dmInfo[cvid - 1].XCol;
                        int rowy = grmPrj.dmInfo[cvid - 1].YRow;
                        string cellpos = colx.ToString() + ", " + rowy.ToString();
                        cellsArray[idx] = cellpos;
                        idx += 1;
                    }
                    return cellsArray;
                }
                else
                    return null;
            }
            else
                return null;
        }

        public bool SetOneSWSParametersAndUpdateAllSWSUsingNetwork(int wsid, double iniSat, 
            double minSlopeLandSurface, string UnsKType, double coefUnsK, 
            double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
            int dryStreamOrder, double ccLCRoughness, 
            double ccSoilDepth, double ccPorosity, double ccWFSuctionHead, 
            double ccSoilHydraulicCond, double iniFlow = 0)
        {
            try
            {
                {
                    grmPrj.subWSPar.userPars[wsid].iniSaturation = iniSat;
                    grmPrj.subWSPar.userPars[wsid].minSlopeOF = minSlopeLandSurface;
                    grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Linear.ToString();
                    if (UnsKType.ToLower()== cGRM.UnSaturatedKType.Linear.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Linear.ToString(); }
                    if (UnsKType.ToLower() == cGRM.UnSaturatedKType.Exponential.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Exponential.ToString(); }
                    if (UnsKType.ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Constant.ToString(); }
                    if (UnsKType.ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Constant.ToString(); }
                    grmPrj.subWSPar.userPars[wsid].coefUK = coefUnsK;
                    grmPrj.subWSPar.userPars[wsid].minSlopeChBed = minSlopeChannel;
                    grmPrj.subWSPar.userPars[wsid].minChBaseWidth = minChannelBaseWidth;
                    grmPrj.subWSPar.userPars[wsid].chRoughness = roughnessChannel;
                    grmPrj.subWSPar.userPars[wsid].dryStreamOrder = dryStreamOrder;
                    grmPrj.subWSPar.userPars[wsid].ccLCRoughness = ccLCRoughness;
                    grmPrj.subWSPar.userPars[wsid].ccSoilDepth = ccSoilDepth;
                    grmPrj.subWSPar.userPars[wsid].ccPorosity = ccPorosity;
                    grmPrj.subWSPar.userPars[wsid].ccWFSuctionHead = ccWFSuctionHead;
                    grmPrj.subWSPar.userPars[wsid].ccHydraulicK = ccSoilHydraulicCond;
                    grmPrj.subWSPar.userPars[wsid].iniFlow = iniFlow;
                    grmPrj.subWSPar.userPars[wsid].isUserSet = true;
                }
                cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(grmPrj);
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return false;
            }
        }

        /// <summary>
        ///    This method is applied to update all the subwatersheds parameters when there are more than 1 subwatershed.
        ///    Before this method is called, user set parameters must have been updated for each user set watershed
        ///    by using [ grmPrj.SubWSPar.userPars[wsid] property]
        ///    And after this method is called, all the paramters in all the watersheds would be updated by using user set parameters.
        ///   </summary>
        public void UpdateAllSubWatershedParametersUsingNetwork()
        {
            if (WScount() > 1)
                cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(grmPrj);
        }

        public cUserParameters subwatershedPars(int wsid)
        {
            return grmPrj.subWSPar.userPars[wsid];
        }

        public bool RemoveUserParametersSetting(int wsid)
        {
            try
            {
                grmPrj.subWSPar.userPars[wsid].isUserSet = false;
                cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(grmPrj);
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return false;
            }
        }


        public double cellSize()
        {
            return grmPrj.watershed.mCellSize;
        }
    }
}
