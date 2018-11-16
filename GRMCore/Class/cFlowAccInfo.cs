using System;
using System.Collections.Generic;

namespace GRMCore
{
    public class cFlowAccInfo
    {
        private Dictionary<int, List<int>> mDic = new Dictionary<int, List<int>>();
        private Dictionary<int, int[]> mFacArrayIndices = new Dictionary<int, int[]>();
        private bool mConvertedToArray = false;


        /// <summary>
        /// 현재 흐름누적수에 해당하는 CV 목록에 자신의 CVAN을 추가함
        /// </summary>
        /// <param name="accum"></param>
        /// <param name="cvan"></param>
        /// <remarks></remarks>
        public void Add(int accum, int cvan)
        {
            int key = accum;
            if (!mDic.ContainsKey(key))
            {
                mDic.Add(key, new List<int>());
            }
            mDic[key].Add(cvan);
        }

        /// <summary>
        /// 현재의 흐름누적수를 가지는 cv array 번호 목록을 반환함
        /// </summary>
        /// <param name="accum"></param>
        /// <value></value>
        /// <returns></returns>
        /// <remarks></remarks>
        public int[] this[int accum]
        {
            get
            {
                if (mConvertedToArray == false) { convertListToArray(); }
                if (mFacArrayIndices.ContainsKey(accum) == true)
                    return mFacArrayIndices[accum];
                else
                    return null;
            }
        }
//public Array GetCVANs(int accum)
//        {
//            if (mConvertedToArray == false) { convertListToArray(); }
//            if (mFacArrayIndices.ContainsKey(accum) == true)
//                return mFacArrayIndices[accum];
//            else
//                return null;
//        }

        //public Array this[int accum]
        //{
        //    get
        //    {
        //        int key = accum;
        //        if (mDic.ContainsKey(key) == true)
        //            return mDic[key].ToArray();
        //        else
        //            return null;
        //    }
        //}

        private void convertListToArray()
        {
            foreach (int ak in mDic.Keys)
            {
                if (!mFacArrayIndices.ContainsKey(ak))
                {
                    mFacArrayIndices.Add(ak, new int[mDic[ak].Count]);
                    mFacArrayIndices[ak] = mDic[ak].ToArray();
                }
            }
            mConvertedToArray = true;
        }

        public void Clear()
        {
            mDic.Clear();
        }
    }
}
