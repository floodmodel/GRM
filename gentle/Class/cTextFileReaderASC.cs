using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace gentle
{
    public class cTextFileReaderASC
    {
        private string[] mLines;
        private int mRowCountAll;
        private int mDataStartLineInASCfile;
        private float mDataValueOri;
        private cRasterHeader mHeader = new cRasterHeader();
        private string mHeaderStringAll;
        private string[] mSeparator = { " ", "\t", "," };

        public cTextFileReaderASC(string FPN)
        {
            mLines = System.IO.File.ReadAllLines(FPN, System.Text.Encoding.Default);
            mRowCountAll = mLines.Length;
            mDataStartLineInASCfile = GetDataStartLineInASCfile();

            if (mRowCountAll < mDataStartLineInASCfile)
            {
                Console.WriteLine(FPN + " has no data valuable. ");
                return;
            }
            GetHeaderInfo(mDataStartLineInASCfile);
            mHeaderStringAll = cTextFile.MakeHeaderString(mHeader.numberCols, mHeader.numberRows,
                 mHeader.xllcorner, mHeader.yllcorner, mHeader.cellsize, mHeader.nodataValue.ToString());
        }

        private void GetHeaderInfo(int HeaderLineCount)
        {
            for (int n = 0; n <= HeaderLineCount - 1; n++)
            {
                string line = mLines[n];
                string[] LineParts = line.Split(mSeparator, StringSplitOptions.RemoveEmptyEntries);
                int iv = 0;
                double dv = 0;
                switch (n)
                {
                    case 0:
                        if (int.TryParse(LineParts[1], out iv))
                        {
                            mHeader.numberCols = iv;
                        }
                        else
                        {
                            mHeader.numberCols = -1;
                        }
                        break;
                    case 1:
                        if (int.TryParse(LineParts[1], out iv))
                        {
                            mHeader.numberRows = iv;
                        }
                        else
                        {
                            mHeader.numberRows = -1;
                        }
                        break;
                    case 2:
                        if (double.TryParse(LineParts[1], out dv))
                        {
                            mHeader.xllcorner = dv;
                        }
                        else
                        {
                            mHeader.xllcorner = -1;
                        }
                        break;
                    case 3:
                        if (double.TryParse(LineParts[1], out dv))
                        {
                            mHeader.yllcorner = dv;
                        }
                        else
                        {
                            mHeader.yllcorner = -1;
                        }
                        break;
                    case 4:
                        if (double.TryParse(LineParts[1], out dv))
                        {
                            mHeader.cellsize = dv;
                        }
                        else
                        {
                            mHeader.cellsize = -1;
                        }
                        break;
                    case 5:
                        if (string.IsNullOrEmpty(LineParts[1]))
                        {
                            mHeader.nodataValue = -9999;
                        }
                        else
                        {
                            double v;
                            if (double.TryParse(LineParts[1], out v))
                            {
                                mHeader.nodataValue = v;
                            }
                            else
                            {
                                mHeader.nodataValue = -1;
                            }
                        }
                        break;
                }
            }
        }

        public int RowCountAll
        {
            get
            {
                return mRowCountAll;
            }
        }

        public cRasterHeader Header
        {
            get
            {
                return mHeader;
            }
        }

        public string HeaderStringAll
        {
            get
            {
                return mHeaderStringAll;
            }
        }

        public double bottom
        {
            get
            {
                return mHeader.yllcorner;
            }
        }

        public double top
        {
            get
            {
                return (mHeader.yllcorner + mHeader.numberRows * mHeader.cellsize);
            }
        }

        public double left
        {
            get
            {
                return mHeader.xllcorner ;
            }
        }

        public double right
        {
            get
            {
                return (mHeader.xllcorner  + mHeader.numberCols  * mHeader.cellsize );
            }
        }

        public double extentWidth
        {
            get
            {
                return (right - left);
            }
        }

        public double extentHeight
        {
            get
            {
                return (top - bottom);
            }
        }

        public double cellSize
        {
            get
            {
                return mHeader.cellsize ;
            }
        }

        public static bool CheckTwoGridLayerExtent( cTextFileReaderASC  GridBase, cTextFileReaderASC GridTarget)
        {
            //   cTextFileReaderASC oGridExtBase = new ct 
            //'    Dim oGridExtTarget As New cGrid(GridTarget)
            if (GridBase.Header.numberCols  != GridTarget.Header.numberCols ){ return false; }
            if (GridBase.Header.numberRows  != GridTarget.Header.numberRows) { return false; }
            if (GridBase.bottom  != GridTarget.bottom) { return false; }
            if (GridBase.top != GridTarget.top) { return false; }
            if (GridBase.left != GridTarget.left) { return false; }
            if (GridBase.right != GridTarget.right) { return false; }
                return true;
        }

        public static void MakeNewAsciiRasterFile(cTextFileReaderASC baseGrid,
                                             string fpn, cData.DataType dType, double defaultValue)
        {
            Console.WriteLine("This was not developed yet.");
        }



        //'Public Shared Function CreateNewGrid(baseGrid As MapWinGIS.Grid, _
        //'                                     fpn As String, dataType As MapWinGIS.GridDataType, _
        //'                                     defaultValue As Integer) As MapWinGIS.Grid
        //'    Dim dG As New MapWinGIS.Grid ' 빈 데이터셑 생성
        //'    'Dim hG As New MapWinGIS.GridHeader ' 빈 해더 생성
        //'    dG.CreateNew(fpn, baseGrid.Header, dataType, defaultValue, _
        //'                        True, MapWinGIS.GridFileType.GeoTiff)
        //'    dG.Save()
        //'    Return dG
        //'End Function


        /// <summary>
        /// Column and row numbers are started from zero
        /// </summary>
        /// <param name="xColNumber"></param>
        /// <param name="yRowNumber"></param>
        /// <returns></returns>
        public float ValueFromLL(int xColNumber, int yRowNumber)
        {
            int row = mRowCountAll - yRowNumber - 1;
            string[] LVals = mLines[row].Split(mSeparator, StringSplitOptions.RemoveEmptyEntries);
            float result = 0;
            if (float.TryParse(LVals[xColNumber], out result))
            {
                return result;
            }
            else
            {
                return -9999;
            }
        }

        /// <summary>
        /// Column and row numbers are started from zero
        /// </summary>
        /// <param name="xColNumber"></param>
        /// <param name="yRowNumber"></param>
        /// <returns></returns>
        public float ValueFromTL(int xColNumber, int yRowNumber)
        {
            string[] LVals = mLines[mDataStartLineInASCfile + yRowNumber - 1].Split(mSeparator, StringSplitOptions.RemoveEmptyEntries);
            float result = 0;
            if (float.TryParse(LVals[xColNumber], out result))
            {
                return result;
            }
            else
            {
                return -9999;
            }
        }

        /// <summary>
        /// Column and row numbers are started from zero
        /// </summary>
        /// <param name="xColNumber"></param>
        /// <param name="yRowNumber"></param>
        /// <returns></returns>
        public float ValueFromTLasNotNegative(int xColNumber, int yRowNumber)
        {
            mDataValueOri = ValueFromTL(xColNumber, yRowNumber);
            if (mDataValueOri < 0)
            {
                return 0;
            }
            else
            {
                return mDataValueOri;
            }
        }


        /// <summary>
        /// Column and row numbers are started from zero
        /// </summary>
        /// <param name="xcol"></param>
        /// <param name="yrow"></param>
        /// <returns></returns>
        public double ValueFromLLasNotNegative(int xcol, int yrow)
        {
            mDataValueOri = ValueFromLL(xcol, yrow);
            if (mDataValueOri < 0)
            {
                return 0;
            }
            else
            {
                return mDataValueOri;
            }
        }

        private int GetDataStartLineInASCfile()
        {
            for (int ln = 0; ln <= mLines.Length - 1; ln++)
            {
                string aline = mLines[ln];
                string[] LineParts = aline.Split(mSeparator, StringSplitOptions.RemoveEmptyEntries);
                float Val = 0;
                if (LineParts.Length > 0 && float.TryParse(LineParts[0], out Val) == true)
                {
                    return ln + 1;
                }
            }
            return -1;
        }

        public int DataStartLineInASCfile
        {
            get
            {
                return mDataStartLineInASCfile;
            }
        }

        public string[] ValuesInOneRowFromLowLeft(int yrow)
        {
            int row = mRowCountAll - yrow - 1;
            return mLines[row].Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries);
        }


        public string[] ValuesInOneRowFromTopLeft(int yrow)
        {

            int row = mDataStartLineInASCfile + yrow - 1;
            return mLines[row].Split(mSeparator, StringSplitOptions.RemoveEmptyEntries);
        }

        public string OneRowContainsValuesFromTop(int yrow)
        {
            int row = mDataStartLineInASCfile + yrow - 1;
            return mLines[row];
        }

        public float ValueAtColumeXFormOneRow(int xcol, string[] Values)
        {
            float result = 0;
            if (float.TryParse(Values[xcol], out result))
            {
                return result;
            }
            else
            {
                return -9999;
            }
        }

        public float ValueAtColumeXFormOneRowAsNotNegative(int xcol, string[] Values)
        {
            mDataValueOri = ValueAtColumeXFormOneRow(xcol, Values);
            if (mDataValueOri < 0)
            {
                return 0;
            }
            else
            {
                return mDataValueOri;
            }
        }

        public static bool CompareFiles(cTextFileReaderASC FileToReference, cTextFileReaderASC FileToCompare)
        {
            if (FileToReference.Header.numberCols != FileToCompare.Header.numberCols)
                return false;
            if (FileToReference.Header.numberRows != FileToCompare.Header.numberRows)
                return false;
            if (FileToReference.Header.cellsize != FileToCompare.Header.cellsize)
                return false;
            return true;
        }
    }
}
