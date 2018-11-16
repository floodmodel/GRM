
namespace GRMCore
{
   public class cNumberBound
    {
        public string ValueName;
        public decimal Min;
        public decimal Max;
        public bool IncludeMinvalue;
        public bool IncludeMaxValue;

        public cNumberBound(string valueName, decimal min, decimal max, bool includeMinvalue = true, bool includeMaxValue = true)
        {
            ValueName = valueName;
            SetBound(min, max);
            IncludeMinvalue = includeMinvalue;
            IncludeMaxValue = includeMaxValue;
        }

        public void SetBound(decimal min, decimal max)
        {
            Min = min;
            Max = max;
            if (Min > Max)
            {
                Max = min;
                Min = max;
            }
        }

        public bool Validate(string strValue)
        {
            decimal value;
            if (!decimal.TryParse(strValue, out value))
                return false;
            return Validate(value);
        }

        public bool Validate(decimal value)
        {
            if (IncludeMinvalue)
            {
                if (value < Min)
                    return false;
            }
            else if (value <= Min)
                return false;

            if (IncludeMaxValue)
            {
                if (value > Max)
                    return false;
            }
            else if (value >= Max)
                return false;
            return true;
        }

        public string ErrorMsg
        {
            get
            {
                string minSymbol;
                string maxSymbol;
                if (IncludeMinvalue)
                    minSymbol = "≤";
                else
                    minSymbol = "<";
                if (IncludeMaxValue)
                    maxSymbol = "≤";
                else
                    maxSymbol = "<";
                return string.Format("[{0}] is invalid. {3}  {1}{4}{0}{5}{2}", ValueName, Min, Max, "\r\n", minSymbol, maxSymbol);
            }
        }
    }
}
