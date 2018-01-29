using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace gentle
{
   public class cRasterHeader
    {
        private int mNumberCols;
        private int mNumberRows;
        private double mXllcorner;
        private double mYllcorner;
        private double mCellsize;

        private string mNODATAvalue;
        public int numberCols
        {
            get
            {
                return mNumberCols;
            }
            set
            {
                mNumberCols = value;
            }
        }

        public int numberRows
        {
            get
            {
                return mNumberRows;
            }
            set
            {
                mNumberRows = value;
            }
        }

        public double cellsize
        {
            get
            {
                return mCellsize;
            }
            set
            {
                mCellsize = value;
            }
        }

        public double xllcorner
        {
            get
            {
                return mXllcorner;
            }
            set
            {
                mXllcorner = value;
            }
        }

        public double yllcorner
        {
            get
            {
                return mYllcorner;
            }
            set
            {
                mYllcorner = value;
            }
        }

        public int nodataValue
        {
            get
            {
                int v;
                if (int.TryParse(mNODATAvalue, out v))
                {
                    return v;
                }
                else
                {
                    return -1;
                }                
            }
            set
            {
                mNODATAvalue = value.ToString ();
            }
        }
    }
}
