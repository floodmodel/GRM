using System;
using System.Collections.Generic;
using System.Drawing;


namespace GRMCore
{
    public class cSetWatchPoint : ICloneable
    {
        public static readonly Color DEFAULT_WATCH_POINT_COLOR = Color.Fuchsia;
        public Dataset.GRMProject.WatchPointsDataTable mdtWatchPointInfo;
        //public Color mCellColor;
        private List<int> mWatchPointCVidList;

        /// <summary>
        ///   현재 watch point 상류에 대해 원시자료에서 읽은 강우량.[m/s] 
        ///   향후, 차단 고려시, 차단된 강우량은 제외한 땅에 떨어진 강우량
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> RFReadIntensitySumUpWs_mPs;

        /// <summary>
        ///   현재 watch point 상류에 대해 dt(계산시간간격)
        ///   시간동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> RFUpWsMeanForDt_mm;

        /// <summary>
        ///   현재 watch point 상류의 출력시간간격
        ///   동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> RFUpWsMeanForDtPrintout_mm;

        /// <summary>
        ///   현재 watch point 상류의 평균강우량의 누적값[mm]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> RFUpWsMeanTotal_mm;

        /// <summary>
        ///   Watchpoint 격자에 대한 출력시간간격
        ///   동안의 누적강우량. 원시자료를 이용해서 계산된값.[mm]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> RFWPGridForDtPrintout_mm;

        /// <summary>
        ///   Watchpoint 격자에 대한 누적강우량[mm]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> RFWPGridTotal_mm;

        /// <summary>
        ///   Watchpoint 격자에 대한 전체유량[cms]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> mTotalFlow_cms;


        /// <summary>
        ///   Watchpoint 격자에 총유출고[m]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> mTotalDepth_m;

        /// <summary>
        ///   Watchpoint 격자에 대한 최대유량[cms]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> MaxFlow_cms;

        /// <summary>
        ///   Watchpoint 격자에 대한 최고수심[m]
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> MaxDepth_m;

        /// <summary>
        ///   Watchpoint 격자에 대한 최대유량 시간. 첨두시간.
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, string> MaxFlowTime;

        /// <summary>
        ///   Watchpoint 격자에 대한 최고수심 시간. 첨두시간
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, string> MaxDepthTime;

        /// <summary>
        ///   해당 wp에서 Flow control에 의해서 계산되는 유량
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, double> QfromFCDataCMS;

        public SortedList<int, double> Qprint_cms;

        /// <summary>
        ///   Watch point 별 모의결과 출력을 위한 파일 이름 저장
        ///   </summary>
        ///   <remarks></remarks>
        public SortedList<int, string> FpnWpOut;


        public cSetWatchPoint()
        {
            RFReadIntensitySumUpWs_mPs = new SortedList<int, double>();
            RFUpWsMeanForDt_mm = new SortedList<int, double>();
            RFUpWsMeanForDtPrintout_mm = new SortedList<int, double>();
            RFUpWsMeanTotal_mm = new SortedList<int, double>();
            RFWPGridForDtPrintout_mm = new SortedList<int, double>();
            RFWPGridTotal_mm = new SortedList<int, double>();
            mTotalFlow_cms = new SortedList<int, double>();
            mTotalDepth_m = new SortedList<int, double>();
            MaxFlow_cms = new SortedList<int, double>();
            MaxDepth_m = new SortedList<int, double>();
            MaxFlowTime = new SortedList<int, string>();
            MaxDepthTime = new SortedList<int, string>();
            QfromFCDataCMS = new SortedList<int, double>();
            Qprint_cms = new SortedList<int, double>();
            FpnWpOut = new SortedList<int, string>();
            mWatchPointCVidList = new List<int>();
        }

        /// <summary>
        ///   Watchpoint 별 강우량 값 입력을 위한 리스트 변수의 key(CVid)를 업데이트 하고
        ///   Watchpoint CVid 리스트를 설정한다.
        ///   </summary>
        ///   <remarks></remarks>
        public bool UpdatesWatchPointCVIDs(cProject prj)
        {
            RFReadIntensitySumUpWs_mPs.Clear();
            RFUpWsMeanForDt_mm.Clear();
            RFUpWsMeanForDtPrintout_mm.Clear();
            RFUpWsMeanTotal_mm.Clear();
            RFWPGridForDtPrintout_mm.Clear();
            RFWPGridTotal_mm.Clear();
            mTotalFlow_cms.Clear();
            mTotalDepth_m.Clear();
            MaxFlow_cms.Clear();
            MaxDepth_m.Clear();
            MaxFlowTime.Clear();
            MaxDepthTime.Clear();
            QfromFCDataCMS.Clear();
            Qprint_cms.Clear();
            FpnWpOut.Clear();
            mWatchPointCVidList.Clear();
            foreach (Dataset.GRMProject.WatchPointsRow row in mdtWatchPointInfo)
            {
                row.CVID = prj.WSCells[row.ColX, row.RowY].CVID;
                int cvid = row.CVID;
                if (mWatchPointCVidList.Contains(cvid))
                {
                    cGRM.writelogAndConsole("ERROR : Two or more watch points were set at the same cell !!!", true, true);
                    return false;
                }
                else
                {
                    mWatchPointCVidList.Add(cvid);
                }
                RFReadIntensitySumUpWs_mPs.Add(cvid, default(Double));
                RFUpWsMeanForDt_mm.Add(cvid, default(Double));
                RFUpWsMeanForDtPrintout_mm.Add(cvid, default(Double));
                RFUpWsMeanTotal_mm.Add(cvid, default(Double));
                RFWPGridForDtPrintout_mm.Add(cvid, default(Double));
                RFWPGridTotal_mm.Add(cvid, default(Double));
                mTotalFlow_cms.Add(cvid, default(Double));
                mTotalDepth_m.Add(cvid, default(Double));
                MaxFlow_cms.Add(cvid, default(Double));
                MaxDepth_m.Add(cvid, default(Double));
                MaxFlowTime.Add(cvid, null);
                MaxDepthTime.Add(cvid, null);
                QfromFCDataCMS.Add(cvid, default(Double));
                Qprint_cms.Add(cvid, default(Double));
                FpnWpOut.Add(cvid, null);
            }
            return true;
        }

        public bool IsSet
        {
            get
            {
                return mdtWatchPointInfo != null;
            }
        }

        public void GetValues(Dataset.GRMProject prjdb)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];
            ColorConverter cc = new ColorConverter();
            if (prjdb.WatchPoints.Rows.Count > 0)
            {
                mdtWatchPointInfo = new Dataset.GRMProject.WatchPointsDataTable();
                mdtWatchPointInfo = prjdb.WatchPoints;
            }
        }

        public void SetValues(Dataset.GRMProject prjds)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjds.ProjectSettings.Rows[0];
            if (IsSet)
            {
                mdtWatchPointInfo.AcceptChanges();
                if (mdtWatchPointInfo.Rows.Count > 0)
                {
                    foreach (Dataset.GRMProject.WatchPointsRow r in mdtWatchPointInfo.Rows)
                    {
                        r.SetAdded();
                    }
                }
            }
        }


        public string wpName(int cvid)
        {
            foreach (Dataset.GRMProject.WatchPointsRow row in mdtWatchPointInfo.Rows)
            {
                if (row.CVID == cvid)
                {
                    return row.Name.Replace(",", "_");
                }
            }
            return "-9999";
        }


        public int WPCount
        {
            get
            {
                if (mdtWatchPointInfo == null)
                {
                    return 0;
                }
                else
                {
                    return mdtWatchPointInfo.Rows.Count;
                }
            }
        }

        //public Color CellColor
        //{
        //    get
        //    {
        //        return mCellColor;
        //    }
        //}

        public List<int> WPCVidList
        {
            get
            {
                return mWatchPointCVidList;
            }
        }

        public object Clone()
        {
            cSetWatchPoint cln = new cSetWatchPoint();
            if (this.RFUpWsMeanForDtPrintout_mm != null)
            {
                foreach (int k in mWatchPointCVidList)
                {
                    if (this.RFReadIntensitySumUpWs_mPs != null && RFReadIntensitySumUpWs_mPs.Keys.Contains(k))
                    {
                        cln.RFReadIntensitySumUpWs_mPs.Add(k, RFReadIntensitySumUpWs_mPs[k]);
                    }
                    if (this.RFUpWsMeanForDt_mm != null && RFUpWsMeanForDt_mm.Keys.Contains(k))
                    {
                        cln.RFUpWsMeanForDt_mm.Add(k, RFUpWsMeanForDt_mm[k]);
                    }
                    if (this.RFUpWsMeanForDtPrintout_mm != null && RFUpWsMeanForDtPrintout_mm.Keys.Contains(k))
                    {
                        cln.RFUpWsMeanForDtPrintout_mm.Add(k, RFUpWsMeanForDtPrintout_mm[k]);
                    }
                    if (this.RFUpWsMeanTotal_mm != null && RFUpWsMeanTotal_mm.Keys.Contains(k))
                    {
                        cln.RFUpWsMeanTotal_mm.Add(k, RFUpWsMeanTotal_mm[k]);
                    }
                    if (this.RFWPGridForDtPrintout_mm != null && RFWPGridForDtPrintout_mm.Keys.Contains(k))
                    {
                        cln.RFWPGridForDtPrintout_mm.Add(k, RFWPGridForDtPrintout_mm[k]);
                    }
                    if (this.RFWPGridTotal_mm != null && RFWPGridTotal_mm.Keys.Contains(k))
                    {
                        cln.RFWPGridTotal_mm.Add(k, RFWPGridTotal_mm[k]);
                    }
                    if (this.mTotalFlow_cms != null && mTotalFlow_cms.Keys.Contains(k))
                    {
                        cln.mTotalFlow_cms.Add(k, mTotalFlow_cms[k]);
                    }
                    if (this.mTotalDepth_m != null && mTotalDepth_m.Keys.Contains(k))
                    {
                        cln.mTotalDepth_m.Add(k, mTotalDepth_m[k]);
                    }
                    if (this.MaxFlow_cms != null && MaxFlow_cms.Keys.Contains(k))
                    {
                        cln.MaxFlow_cms.Add(k, MaxFlow_cms[k]);
                    }
                    if (this.MaxDepth_m != null && MaxDepth_m.Keys.Contains(k))
                    {
                        cln.MaxDepth_m.Add(k, MaxDepth_m[k]);
                    }
                    if (this.MaxFlowTime != null && MaxFlowTime.Keys.Contains(k))
                    {
                        cln.MaxFlowTime.Add(k, MaxFlowTime[k]);
                    }
                    if (this.MaxDepthTime != null && MaxDepthTime.Keys.Contains(k))
                    {
                        cln.MaxDepthTime.Add(k, MaxDepthTime[k]);
                    }
                    if (this.QfromFCDataCMS != null && QfromFCDataCMS.Keys.Contains(k))
                    {
                        cln.QfromFCDataCMS.Add(k, QfromFCDataCMS[k]);
                    }
                    if (this.Qprint_cms != null && Qprint_cms.Keys.Contains(k))
                    {
                        cln.Qprint_cms.Add(k, Qprint_cms[k]);
                    }
                    if (this.FpnWpOut != null && FpnWpOut.Keys.Contains(k))
                    { cln.FpnWpOut.Add(k, FpnWpOut[k]); }
                    cln.mWatchPointCVidList.Add(k);
                }
            }
            return cln;
        }
    }
}
