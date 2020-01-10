
using System.Collections.Generic;


namespace GRMCore
{
    public class cWatershedNetwork
    {
        private List<int> mWSidList;
        private SortedList<int, List<int>> mWSIDsNearbyUp = new SortedList<int, List<int>>();
        private SortedList<int, List<int>> mWSIDsNearbyDown = new SortedList<int, List<int>>();
        private SortedList<int, List<int>> mWSIDsAllUps = new SortedList<int, List<int>>();
        private SortedList<int, List<int>> mWSIDsAllDowns = new SortedList<int, List<int>>();
        private List<int> mMostDownstreamWSIDs;

        private Dictionary<int, int> mWSoutletCVids = new Dictionary<int, int>();
        public Dictionary<int, int> mMostDownStreamWSIDofCurrentWS = new Dictionary<int, int>();

        /// <summary>
        ///   모든 유역에 대해서 상하류 셀 리스트를 0으로 초기화
        ///   </summary>
        ///   <param name="WSList"></param>
        ///   <remarks></remarks>
        public cWatershedNetwork(List<int> WSList)
        {
            mWSidList = WSList;
            mWSIDsNearbyUp.Clear();
            mWSIDsNearbyDown.Clear();
            mWSoutletCVids.Clear();
            foreach (int i in mWSidList)
            {
                mWSIDsNearbyUp.Add(i, new List<int>());
                mWSIDsNearbyDown.Add(i, new List<int>());
                mWSoutletCVids.Add(i, -1);
            }
            mMostDownstreamWSIDs = new List<int>();
        }

        public void UpdateAllDownsAndUpsNetwork()
        {
            mWSIDsAllUps.Clear();
            mWSIDsAllDowns.Clear();
            foreach (int i in mWSidList)
            {
                mWSIDsAllUps.Add(i, new List<int>());
                mWSIDsAllDowns.Add(i, new List<int>());
                foreach (int id in mWSIDsNearbyUp[i])
                {
                    mWSIDsAllUps[i].Add(id);
                }
                foreach (int id in mWSIDsNearbyDown[i])
                {
                    mWSIDsAllDowns[i].Add(id);
                }
            }

            foreach (int nowID in mWSidList)
            {
                List<int> upIDs = new List<int>();
                List<int> downIDs = new List<int>();
                upIDs = mWSIDsAllUps[nowID];
                downIDs = mWSIDsAllDowns[nowID];
                foreach (int upID in upIDs)
                {
                    foreach (int downID in downIDs)
                    {
                        if (!mWSIDsAllUps[downID].Contains(upID))
                        {
                            mWSIDsAllUps[downID].Add(upID);
                        }
                        if (!mWSIDsAllDowns[upID].Contains(downID))
                        {
                            mWSIDsAllDowns[upID].Add(downID);
                        }
                    }
                }
            }
            foreach (int nowID in mWSidList)
            {
                if (mWSIDsNearbyDown[nowID].Count == 0)
                {
                    mMostDownstreamWSIDs.Add(nowID);
                }
            }

            foreach (int nowID in mWSidList)
            {
                foreach (int mdwsid in mMostDownstreamWSIDs)
                {
                    if (nowID == mdwsid
                        || mWSIDsAllUps[mdwsid].Contains(nowID) == true)
                    {
                        mMostDownStreamWSIDofCurrentWS.Add(nowID, mdwsid);
                        break;
                    }
                }
            }
        }

        public void SetWSoutletCVID(int wsid, int cvid)
        {
            mWSoutletCVids[wsid] = cvid;
        }

        public void AddWSIDup(int NowWSID, int WSIDup)
        {
            mWSIDsNearbyUp[NowWSID].Add(WSIDup);
        }

        public void AddWSIDdown(int NowWSID, int WSIDdown)
        {
            mWSIDsNearbyDown[NowWSID].Add(WSIDdown);
        }

        public void ClearUpstreamWSIDs()
        {
            mWSIDsNearbyUp.Clear();
        }

        public void ClearDownstreamWSIDs()
        {
            mWSIDsNearbyDown.Clear();
        }

        public int WSoutletCVID(int wsid)
        {
            return mWSoutletCVids[wsid];
        }

        public List<int> WSIDsAllDowns(int NowWSID)
        {
            mWSIDsAllDowns[NowWSID].Sort();
            return mWSIDsAllDowns[NowWSID];
        }

        public List<int> WSIDsAllUps(int NowWSID)
        {
            mWSIDsAllUps[NowWSID].Sort();
            return mWSIDsAllUps[NowWSID];
        }

        public List<int> MostDownstreamWSIDs
        {
            get
            {
                return mMostDownstreamWSIDs;
            }
        }

        public int WSIDsNearbyDown(int NowWSID)
        {
            if (mWSIDsNearbyDown[NowWSID].Count > 0)
            {
                List<int> lst = mWSIDsNearbyDown[NowWSID];
                return lst[0];
            }
            else
            { return -1; }
        }

        public List<int> WSIDsNearbyUp(int NowWSID)
        {
            mWSIDsNearbyUp[NowWSID].Sort();
            return mWSIDsNearbyUp[NowWSID];
        }

        public List<int> WSIDsAll
        {
            get
            {
                return mWSidList;
            }
        }
    }
}
