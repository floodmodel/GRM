using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data;
using System.Windows.Forms;

namespace gentle
{
   public class cComTools
    {
        public static string GetTimeToPrintOut(bool bDateTimeFormat, string startDateTime, int nowT_MIN_elapsed)
        {
            if (bDateTimeFormat == true)
            {
                string tv = Convert.ToDateTime(startDateTime).Add(new System.TimeSpan(0, nowT_MIN_elapsed, 0)).ToString("yyyy/MM/dd HH:mm");
                return tv;
            }
            else
            {
                return String.Format(((double) nowT_MIN_elapsed / 60).ToString("F2"));
            }
        }

        public static string GetTimeToPrintOut(bool bDateTimeFormat, string startDateTime, int nowT_MIN_elapsed, string stringFormat)
        {
            if (bDateTimeFormat == true)
            {
                return String.Format(Convert.ToDateTime(startDateTime).Add(new System.TimeSpan(0, nowT_MIN_elapsed, 0)).ToString (),
                    stringFormat);
            }
            else
            {
                return String.Format((nowT_MIN_elapsed / 60).ToString("F"));
            }
        }

        public static bool IsNumeric(string value)
        {
            float v = 0;
            return float.TryParse(value, out v);
        }

        public static string GetTimeStringFromDateTimeFormat(string nowTimeToPrintOut)
        {
            nowTimeToPrintOut = nowTimeToPrintOut.Replace("/", "");
            nowTimeToPrintOut = nowTimeToPrintOut.Replace( " ", "");
            nowTimeToPrintOut = nowTimeToPrintOut.Replace( ":", "");
            nowTimeToPrintOut = nowTimeToPrintOut.Replace( "-", "");

            return nowTimeToPrintOut;
        }

        /// <summary>
        /// 문자열에 공백 있을 경우 필수 사용
        /// </summary>
        /// <param name="str"></param>
        /// <returns></returns>
        /// <remarks></remarks>
        public static string SetDQ(string str)
        {
            string result = "";
            result = "\"" + str + "\"";
            return result;
        }

        public static long GetNumericRoundDown(double dblInputValue)
        {
            int intPointPosition = 0;
            intPointPosition = dblInputValue.ToString ().IndexOf (".");
            if (intPointPosition == 0)
            {
                return Convert.ToInt64(dblInputValue);
            }
            else
            {
                return Convert.ToInt64(Convert.ToString(dblInputValue).Substring(0, intPointPosition));
            }
        }

        public static string GetNowTimeToPrintOut(string strStartingTime, int TimeStepToOut_MIN, int intNowOrder)
        {
            string strNowTimeToPrintOut = null;
            try
            {
                strNowTimeToPrintOut = string.Format( Convert.ToDateTime(strStartingTime).Add(new System.TimeSpan(0, TimeStepToOut_MIN * intNowOrder, 0)).ToString(), "yyyy/MM/dd HH:mm");
                strNowTimeToPrintOut = strNowTimeToPrintOut.Replace( "/", "");
                strNowTimeToPrintOut = strNowTimeToPrintOut.Replace( " ", "");
                strNowTimeToPrintOut = strNowTimeToPrintOut.Replace( ":", "");
                strNowTimeToPrintOut = strNowTimeToPrintOut.Replace( "-", "");
                return strNowTimeToPrintOut;
            }
            catch (Exception ex)
            {
                return  "-1";
                throw ex;
            }
        }

        public static List<string> GetListFromDataTable(DataTable indt, int cidx)
        {
            List<string> nl = new List<string>();
            for (int nr = 0; nr <= indt.Rows.Count - 1; nr++)
            {
                nl.Add(indt.Rows[nr].ItemArray[cidx].ToString());
            }
            return nl;
        }

        public static List<string> GetTimeListToPrintout(string rainfallStartDateTime, int TimeInterval_Min, int ListCountToSet)
        {
            List<string> l = new List<string>();
            if (ListCountToSet > 1)
            {
                for (int n = 0; n <= ListCountToSet - 1; n++)
                {
                    int travel_min = n * TimeInterval_Min;
                    string T = Convert.ToDateTime(rainfallStartDateTime).Add(new System.TimeSpan(0, travel_min, 0)).ToString("yyyy/MM/dd HH:mm") ;
                    l.Add(T);
                }
            }
            return l;
        }

        public static void ClearTextInControl(params Control[] targets)
        {
            foreach (Control c in targets)
            {
                c.Text = "";
            }
        }

        public static string getTailFromString(string strIn, string strSeparator)
        {
            int pos1 = 0;
            pos1 = strIn.IndexOf(strSeparator);
            return strIn.Substring(pos1 + 1).Trim();
        }

        public static string getHeadFromString(string strIn, string strSeparator)
        {
            int pos1 = 0;
            pos1 = strIn.IndexOf(strSeparator);
            return strIn.Substring(0, pos1 + 1).Trim();
        }
    }
}
