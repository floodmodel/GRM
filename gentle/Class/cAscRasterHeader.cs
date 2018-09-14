using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace gentle
{
   public class cAscRasterHeader
    {
        public  int numberCols;
        public int numberRows;
        public double xllcorner;
        public double yllcorner;
        public double cellsize;
        public int nodataValue;
        public int headerEndingLineIndex;
        public int dataStartingLineIndex;
    }

    public class cRasterExtent
    {
        public  double bottom;
        public double top;
        public double left;
        public double right;
        public double extentWidth;
        public double extentHeight;

        public cRasterExtent(cAscRasterHeader header)
        {
            bottom = header.yllcorner;
            top = header.yllcorner + header.numberRows * header.cellsize;
            left= header.xllcorner;
            right = header.xllcorner + header.numberCols * header.cellsize;
            extentWidth = right - left;
            extentHeight = top - bottom;
        }
    }
}
