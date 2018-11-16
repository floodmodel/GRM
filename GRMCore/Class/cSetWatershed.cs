using System.Collections.Generic;

namespace GRMCore
{
    public class cSetWatershed
    {
        public string mFPN_watershed;
        public string mFPN_slope;
        public string mFPN_fdir;
        public string mFPN_fac;
        public string mFPN_stream;
        public string mFPN_channelWidth;
        public string mFPN_initialChannelFlow;
        public string mFPN_initialSoilSaturationRatio;

        /// <summary>
        ///   격자의 크기[m]
        ///   </summary>
        ///   <remarks></remarks>
        public double mCellSize = -1;
        public cGRM.FlowDirectionType mFDType;

        /// <summary>
        ///   격자 가로 셀 개수
        ///   </summary>
        ///   <remarks></remarks>
        public int colCount = -1;

        /// <summary>
        ///   격자 세로 셀 개수
        ///   </summary>
        ///   <remarks></remarks>
        public int rowCount = -1;
        public double mxllcorner;
        public double myllcorner;

        /// <summary>
        ///   대상 유역의 소유역 id 리스트
        ///   </summary>
        ///   <remarks></remarks>
        public List<int> mWSIDList;

        /// <summary>
        ///   하도셀 중 최상류셀의 흐름누적수
        ///   </summary>
        ///   <remarks></remarks>
        public int mFacMostUpChannelCell;

        /// <summary>
        ///   최대흐름 누적수
        ///   </summary>
        ///   <remarks></remarks>
        public int mFacMax;
        public int mFacMin;

        /// <summary>
        ///   소유역별 셀 개수
        ///   </summary>
        ///   <remarks></remarks>
        public Dictionary<int, List<int>> mCVidListForEachWS;

        public cSetWatershed()
        {
            mWSIDList = new List<int>();
            mCVidListForEachWS = new Dictionary<int, List<int>>();
        }

        public void SetValues(Dataset.GRMProject prjdb)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];
            row.WatershedFile = mFPN_watershed;
            row.SlopeFile = mFPN_slope;
            row.FlowDirectionFile = mFPN_fdir;
            row.FlowAccumFile = mFPN_fac;
            row.StreamFile = mFPN_stream;
            row.ChannelWidthFile = mFPN_channelWidth;
            row.InitialSoilSaturationRatioFile = mFPN_initialSoilSaturationRatio;
            row.InitialChannelFlowFile = mFPN_initialChannelFlow;
            // .GridCellSize = CStr(mCellSize)
            row.FlowDirectionType = mFDType.ToString();
        }

        public void GetValues(Dataset.GRMProject prjdb)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];
            if (!row.IsWatershedFileNull())
                mFPN_watershed = row.WatershedFile;
            if (!row.IsSlopeFileNull())
                mFPN_slope = row.SlopeFile;
            if (!row.IsFlowDirectionFileNull())
                mFPN_fdir = row.FlowDirectionFile;
            if (!row.IsFlowAccumFileNull())
                mFPN_fac = row.FlowAccumFile;
            if (!row.IsStreamFileNull())
                mFPN_stream = row.StreamFile;
            if (row.IsChannelWidthFileNull() == false)
                mFPN_channelWidth = row.ChannelWidthFile;
            if (row.IsInitialChannelFlowFileNull() == false)
                mFPN_initialChannelFlow = row.InitialChannelFlowFile;
            if (row.IsInitialSoilSaturationRatioFileNull() == false)
                mFPN_initialSoilSaturationRatio = row.InitialSoilSaturationRatioFile;
            // If .IsGridCellSizeNull Then
            // mCellSize = -1
            // Else
            // mCellSize = CInt(.GridCellSize)
            // End If
            if (!row.IsFlowDirectionTypeNull())
            {
                switch (row.FlowDirectionType)
                {
                    case nameof(cGRM.FlowDirectionType.StartsFromN):
                        {
                            mFDType = cGRM.FlowDirectionType.StartsFromN;
                            break;
                        }
                    case nameof(cGRM.FlowDirectionType.StartsFromNE):
                        {
                            mFDType = cGRM.FlowDirectionType.StartsFromNE;
                            break;
                        }
                    case nameof(cGRM.FlowDirectionType.StartsFromE):
                        {
                            mFDType = cGRM.FlowDirectionType.StartsFromE;
                            break;
                        }
                    case nameof(cGRM.FlowDirectionType.StartsFromE_TauDEM):
                        {
                            mFDType = cGRM.FlowDirectionType.StartsFromE_TauDEM;
                            break;
                        }
                    default:
                        {
                            mFDType = cGRM.FlowDirectionType.StartsFromE_TauDEM;
                            break;
                        }
                }
            }
            else
            { mFDType = cGRM.FlowDirectionType.StartsFromE_TauDEM; }
        }

        public bool HasStreamLayer
        {
            get
            {
                return !string.IsNullOrEmpty(mFPN_stream);
            }
        }

        public bool HasChannelWidthLayer
        {
            get
            {
                return !string.IsNullOrEmpty(mFPN_channelWidth);
            }
        }

        public bool IsSet
        {
            get
            {
                return !string.IsNullOrEmpty(mFPN_watershed);
            }
        }

        /// <summary>
        ///   분석대상 유역의 소유역 id 리스트
        ///   </summary>
        ///   <value></value>
        ///   <returns></returns>
        ///   <remarks></remarks>
        public List<int> WSIDList
        {
            get
            {
                mWSIDList.Sort();
                return mWSIDList;
            }
        }

        public int CellCountforEachWSID(int wsid)
        {
            return mCVidListForEachWS[wsid].Count;
        }
    }
}
