using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace gentle
{
    public class cTimeChecker
    {
        private string mFPNout;
        private Nullable<System.DateTime> mTfrom;

        private string mPositionFrom;
        public cTimeChecker(string fpnOut)
        {
            mFPNout = fpnOut;
            if (File.Exists(fpnOut))
                File.Delete(fpnOut);
        }
        /// <summary>
        /// 경과시간 측정을 위해,, 시작시간 설정
        /// </summary>
        /// <returns></returns>
        /// <remarks></remarks>
        public bool SetStartingTime(string positionFrom)
        {
            mPositionFrom = positionFrom;
            mTfrom = DateTime.Now;
            return true;
        }

        /// <summary>
        /// 설정된 시작시간에서 현재까지의 경과시간[milli seconds]
        /// </summary>
        /// <returns></returns>
        /// <remarks></remarks>
        public int CalTimeDifference_milliSEC(string positionNow, int nowT_MIN)
        {
            if (mTfrom == null)
            {
                Console.WriteLine("Starting time is not set");
                return -1;
            }
            TimeSpan ts = default(TimeSpan);
            ts = DateTime.Now.Subtract(Convert.ToDateTime(mTfrom));
            int toPrint = Convert.ToInt32(ts.TotalMilliseconds);
            File.AppendAllText(mFPNout, Convert.ToString(nowT_MIN) + "\t" + mPositionFrom + "\t" + positionNow + "\t" + Convert.ToString(toPrint) + "\r\n");
            return toPrint;
        }


        public static int GetTimeDiffereceAsSEC(System.DateTime tStart, System.DateTime tNow)
        {
            TimeSpan ts = tNow.Subtract(Convert.ToDateTime(tStart));
            return Convert.ToInt32(ts.TotalSeconds);
        }

        public static int GetTimeDiffereceAsMilliSEC(System.DateTime tStart, System.DateTime tNow)
        {
            TimeSpan ts = tNow.Subtract(Convert.ToDateTime(tStart));
            return Convert.ToInt32(ts.TotalMilliseconds);
        }
    }
}
