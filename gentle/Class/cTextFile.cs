using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Data;

namespace gentle
{
    public class cTextFile
    {
        public enum ValueSeparator
        {
            CSV,
            SPACE,
            TAB,
            ALL
        };

        public static bool ConfirmDeleteFiles(List<string> FilePathNames)
        {
            bool bAlldeleted = false;
            int n = 0;
            while (!(bAlldeleted == true))
            {
                n += 1;
                foreach (string fpn in FilePathNames)
                {
                    if (File.Exists(fpn) == true)
                    {
                        File.Delete(fpn);
                    }
                }
                foreach (string fpn in FilePathNames)
                {
                    if (File.Exists(fpn) == false)
                    {
                        bAlldeleted = true;
                    }
                    else
                    {
                        bAlldeleted = false;
                        break; 
                    }
                }
                if (n > 100)
                    return false;
            }
            return true;
        }

        public static bool ConfirmDeleteFiles(string FilePathNames)
        {
            bool bAlldeleted = false;
            int n = 0;
            while (!(bAlldeleted == true))
            {
                n += 1;
                if (File.Exists(FilePathNames))
                {
                    File.Delete(FilePathNames);
                }
                if (File.Exists(FilePathNames) == false)
                {
                    bAlldeleted = true;
                }
                else
                {
                    bAlldeleted = false;
                }
                if (n > 100)
                    return false;
            }
            return true;
        }


        public static string MakeHeaderString(int ncols, int nrows, double xll, double yll, double cellSize, string nodataValue)
        {
            string header = "";
            header = header + "ncols" + " " + Convert.ToString(ncols) + "\r\n";
            header = header + "nrows" + " " + Convert.ToString(nrows) + "\r\n";
            header = header + "xllcorner" + " " + Convert.ToString(xll) + "\r\n";
            header = header + "yllcorner" + " " + Convert.ToString(yll) + "\r\n";
            header = header + "cellsize" + " " + Convert.ToString(cellSize) + "\r\n";
            header = header + "NODATA_value" + " " + nodataValue + "\r\n";
            return header;
        }


        public static bool MakeASCTextFile(string fpn, int ncols, int nrows, double xll, double yll, double cellSize, string nodataValue, string[] rowsArray)
        {
            string header = cTextFile.MakeHeaderString(ncols, nrows, xll, yll, cellSize, nodataValue);
            File.AppendAllText(fpn, header);
            for (int n = 0; n <= rowsArray.Length - 1; n++)
            {
                File.AppendAllText(fpn, rowsArray[n] + "\r\n");
            }
            return true;
        }

        public static bool MakeASCTextFile(string fpn, string allHeader, string[] strArray)
        {
            File.AppendAllText(fpn, allHeader);
            for (int n = 0; n <= strArray.Length - 1; n++)
            {
                File.AppendAllText(fpn, strArray[n] + "\r\n");
            }
            return true;
        }

        public static bool MakeASCTextFile(string fpn, int ncols, int nrows, double xll, double yll, float cellSize, string nodataValue, double[,] array, int decimalPartN)
        {
            string header = cTextFile.MakeHeaderString(ncols, nrows, xll, yll, cellSize, nodataValue);
            File.AppendAllText(fpn, header);
            WriteTwoDimData(fpn, array, decimalPartN);
            //for (int nr = 0; nr <= nrows - 1; nr++)
            //{
            //    string arow = "";
            //    for (int nc = 0; nc <= ncols - 1; nc++)
            //    {
            //        arow = arow + array[nc, nr].ToString() + " ";
            //    }
            //    arow = arow.Trim() + "\r\n";
            //    File.AppendAllText(fpn, arow);
            //}
            return true;
        }


        public static bool MakeASCTextFile(string fpn, string allHeader, double[,] array, int decimalPartN)
        {
            File.AppendAllText(fpn, allHeader);
            WriteTwoDimData(fpn, array, decimalPartN);
            //string rows = "";
            //for (int nr = 0; nr <= array.GetLength(1) - 1; nr++)
            //{
            //    string arow = "";
            //    for (int nc = 0; nc <= array.GetLength(0) - 1; nc++)
            //    {
            //        arow = arow + array[nc, nr].ToString() + " ";
            //    }
            //    arow = arow.Trim() + "\r\n";
            //    rows = rows + arow;
            //    if (nr % 2 == 0)
            //    {
            //        File.AppendAllText(fpn, rows); 
            //        rows = "";
            //    }
            //    //File.AppendAllText(fpn, arow); // 이방법이 더 빠르다.
            //}
            //File.AppendAllText(fpn, rows);
            ////File.AppendAllText(fpn, rows); // 이방법이 더 느리다
            return true;
        }


        public static double[,] addTwoDimArrayOfASCraster(double[,] inArray1, double[,] inArray2, double nodataValue, bool allowNegative = false)
        {
            double[,] array = new double[inArray1.GetLength(0), inArray1.GetLength(1)];

            //for (int y = 0; y < array.GetLength(1) ; y++)
            //{
            //    for (int x = 0; x < array.GetLength(0); x++)
            //    {
            //        if (inArray1[x, y] == nodataValue || inArray2[x, y] == nodataValue)
            //        {
            //            if (inArray1[x, y] == nodataValue & inArray2[x, y] == nodataValue)
            //            {
            //                array[x, y] = nodataValue;//둘다 null 이면,  null
            //            }
            //            else
            //            {
            //                if (inArray1[x, y] == nodataValue)
            //                {
            //                    inArray1[x, y] = 0; //둘중 하나가 null이 아니면, 0으로
            //                }
            //                if (inArray2[x, y] == nodataValue)
            //                {
            //                    inArray2[x, y] = 0; //둘중 하나가 null이 아니면, 0으로
            //                }
            //                if (allowNegative == false)
            //                {
            //                    if (inArray1[x, y] < 0) { inArray1[x, y] = 0; }
            //                    if (inArray2[x, y] < 0) { inArray2[x, y] = 0; }
            //                }
            //                array[x, y] = inArray1[x, y] + inArray2[x, y];
            //            }
            //        }
            //        else
            //        {
            //            if (allowNegative == false)
            //            {
            //                if (inArray1[x, y] < 0) { inArray1[x, y] = 0; }
            //                if (inArray2[x, y] < 0) { inArray2[x, y] = 0; }
            //            }
            //            array[x, y] = inArray1[x, y] + inArray2[x, y];

            //        }
            //    }
            //}

            var options = new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount  };

            Parallel.For(0, array.GetLength(1), options, delegate (int y)
            {
                for (int x = 0; x < array.GetLength(0); x++)
                {
                    if (inArray1[x, y] == nodataValue || inArray2[x, y] == nodataValue)
                    {
                        if (inArray1[x, y] == nodataValue & inArray2[x, y] == nodataValue)
                        {
                            array[x, y] = nodataValue;//둘다 null 이면,  null
                        }
                        else
                        {
                            if (inArray1[x, y] == nodataValue)
                            {
                                inArray1[x, y] = 0; //둘중 하나가 null이 아니면, 0으로
                            }
                            if (inArray2[x, y] == nodataValue)
                            {
                                inArray2[x, y] = 0; //둘중 하나가 null이 아니면, 0으로
                            }
                            if (allowNegative == false)
                            {
                                if (inArray1[x, y] < 0) { inArray1[x, y] = 0; }
                                if (inArray2[x, y] < 0) { inArray2[x, y] = 0; }
                            }
                            array[x, y] = inArray1[x, y] + inArray2[x, y];
                        }
                    }
                    else
                    {
                        if (allowNegative == false)
                        {
                            if (inArray1[x, y] < 0) { inArray1[x, y] = 0; }
                            if (inArray2[x, y] < 0) { inArray2[x, y] = 0; }
                        }
                        array[x, y] = inArray1[x, y] + inArray2[x, y];

                    }
                }
            });

            return array;
        }

        private static void WriteTwoDimData(string fpn, double[,] array, int decimalPartNum)
        {
            string dpn = "";
            if (decimalPartNum == 1) { dpn = "F1"; }
            if (decimalPartNum == 2) { dpn = "F2"; }
            if (decimalPartNum == 3) { dpn = "F3"; }
            if (decimalPartNum == 4) { dpn = "F4"; }
            if (decimalPartNum == 4) { dpn = "F5"; }
            StringBuilder sbALL = new StringBuilder();
            for (int nr = 0; nr <= array.GetLength(1) - 1; nr++)
            {
                for (int nc = 0; nc <= array.GetLength(0) - 1; nc++)
                {
                    sbALL.Append(array[nc, nr].ToString(dpn) +" ");
                }
                sbALL.Append("\r\n");
            }
            File.AppendAllText(fpn, sbALL.ToString());
        }


        private static void WriteTwoDimData_old(string fpn, double[,] array)
        {
            string rows = "";
            for (int nr = 0; nr <= array.GetLength(1) - 1; nr++)
            {
                string arow = "";
                for (int nc = 0; nc <= array.GetLength(0) - 1; nc++)
                {
                    arow = arow + array[nc, nr].ToString("F2") + " ";
                }
                arow = arow.Trim() + "\r\n";
                rows = rows + arow;
                if (nr % 2 == 0) //이부분 더 크게 하면 오히려 늦어 진다..
                {
                    File.AppendAllText(fpn, rows);
                    rows = "";
                }
            }
            File.AppendAllText(fpn, rows);
        }

        public static bool MakeASCTextFileAsParallel(string fpn, string allHeader, double[,] array)
        {
            File.AppendAllText(fpn, allHeader);
            int rowYcount = array.GetLength(1);
            int colXcount = array.GetLength(0);
            //var options = new ParallelOptions { MaxDegreeOfParallelism = -1 };
            var options = new ParallelOptions { MaxDegreeOfParallelism = Environment .ProcessorCount*2 };
            string[] rows = new string[rowYcount];
            StringBuilder[] sbs= new StringBuilder[rowYcount];
            Parallel.For(0, rowYcount, options, delegate (int ry)
            {
                sbs[ry] = new StringBuilder();
                for (int cx = 0; cx < colXcount; cx++)
                {
                    sbs[ry].Append(array[cx, ry].ToString("F2") + " ");
                }
                sbs[ry].Append("\r\n");
            });
            StringBuilder sbALL = new StringBuilder();
            for (int nr = 0; nr < rowYcount; nr++)
            {
                sbALL.Append(sbs[nr]);
            }
            File.AppendAllText(fpn, sbALL.ToString());
            return true;
        }


        public static bool MakeASCTextFileAsParallel_old(string fpn, string allHeader, double[,] array)
        {
            File.AppendAllText(fpn, allHeader);
            int rowYcount = array.GetLength(1);
            int colXcount = array.GetLength(0);
            //var options = new ParallelOptions { MaxDegreeOfParallelism = -1 };
            var options = new ParallelOptions { MaxDegreeOfParallelism = Environment .ProcessorCount *4 };
            string[] rows = new string[rowYcount];
            Parallel.For(0, rowYcount, options, delegate (int ry)
            {
                string arow = "";
                for (int cx = 0; cx < colXcount; cx++)
                {
                    arow = arow + array[cx, ry].ToString() + " ";
                }
                arow = arow.Trim() + "\r\n";
                rows[ry] = arow;
            });
            for (int nr = 0; nr < rowYcount; nr++)
            {
                File.AppendAllText(fpn, rows[nr]);
            }
            return true;
        }

        public static SortedList<int, string> ReadVatFile(string sourceFPN)
        {
            SortedList<int, string> values = new SortedList<int, string>();
            StreamReader reader = new StreamReader(sourceFPN, System.Text.Encoding.Default);
            while (!reader.EndOfStream)
            {
                string line = reader.ReadLine();
                string[] parts = line.Split(new string[] { "," }, StringSplitOptions.RemoveEmptyEntries);
                int attrValue = 0;
                if (parts != null && parts.Length == 2 && int.TryParse(parts[0], out attrValue)
                                && !values.ContainsKey(attrValue))
                {
                    values.Add(attrValue, parts[1]);
                }
                else
                {
                    Console.WriteLine(string.Format("Values in VAT file ({0}) are invalid, or attributes count are more than 1.{1}", sourceFPN, "\r\n"));
                    Console.WriteLine(string.Format("Each grid value must have one attribute[gridValue, attributeValue]."));
                }
            }
            reader.Close();
            return values;
        }

        public static SortedList<int, string[]> ReadVatFile(string sourceFPN, ValueSeparator separator = ValueSeparator.CSV)
        {
            SortedList<int, string[]> values = new SortedList<int, string[]>();
            StreamReader reader = new StreamReader(sourceFPN, System.Text.Encoding.Default);
            while (!reader.EndOfStream)
            {
                string line = reader.ReadLine();
                string[] sep = GetTextFileValueSeparator(separator);
                string[] parts = line.Split( sep, StringSplitOptions.RemoveEmptyEntries);
                if (parts != null && parts.Length > 1)
                {
                    int keyvalue = 0;
                    if (int.TryParse(parts[0], out keyvalue) && !values.ContainsKey(keyvalue))
                    {
                        string[] attValues = new string[parts.Length - 1];
                        for (int ne = 0; ne <= parts.Length - 2; ne++)
                        {
                            attValues[ne] = parts[ne + 1];
                        }
                        values.Add(keyvalue, attValues);
                    }
                }
            }
            reader.Close();
            return values;
        }
        
        private static string[] GetTextFileValueSeparator(cTextFile.ValueSeparator valueSeparator)
        {
            string[] sepArray = null;
            switch (valueSeparator)
            {
                case cTextFile.ValueSeparator.CSV:
                    sepArray = new string[] { "," };
                    break;
                case cTextFile.ValueSeparator.SPACE:
                    sepArray = new string[] { " " };
                    break;
                case cTextFile.ValueSeparator.TAB:
                    sepArray = new string[] { "\r\n" };
                    break;
                case cTextFile.ValueSeparator.ALL:
                    sepArray = new string[] { ",", " ", "\r\n" };
                    break;
                default:
                    sepArray = new string[] { ",", " ", "\r\n" };
                    break;
            }
            return sepArray;
        }


        public static void getTextInTextFile(string strSourceFPN, string strTagetFPN,
            int startingLineIndex , int endingLineIndex , int colidx, bool onlyNumeric)
        {
            string[] seps = GetTextFileValueSeparator(ValueSeparator.ALL);
            string[] Lines = System.IO.File.ReadAllLines(strSourceFPN);
            StringBuilder sb = new StringBuilder();
            if(startingLineIndex ==0 && endingLineIndex ==0)
            {
                endingLineIndex = Lines.Length;
            }
            for (int n = 0; n < Lines.Length ; n++)
            {
                if  (n>=startingLineIndex && n<= endingLineIndex )
                {
                    string[] texts = Lines[n].Split(seps, StringSplitOptions.RemoveEmptyEntries);
                    string v="" ;
                    if (texts.Length > colidx)
                    {
                        if (onlyNumeric = true && cComTools.IsNumeric(texts[colidx]) == true)
                        {
                            v = texts[colidx].Trim();
                        }
                        else
                        {
                            v = texts[colidx].Trim();
                        }
                    }
                    if (v!="")
                    {
                        sb.Append(v + "\r\n");
                    }
                }
                if (n==endingLineIndex )
                {
                    break;
                }
            }
            File.AppendAllText(strTagetFPN, sb.ToString());
        }

        public static void getTextInTextFile(string strSourceFPN, string strTagetFPN, 
          string startingText, string endingText, int colidx, bool onlyNumeric)
        {

            string[] seps = GetTextFileValueSeparator(ValueSeparator.ALL);
            string[] Lines = System.IO.File.ReadAllLines(strSourceFPN);
            StringBuilder sb = new StringBuilder();
            bool started = false;
            bool endingConditionApplied = true;
            if (startingText == "") { started = true; }
            if (endingText == "") { endingConditionApplied = false; }
            for (int n = 0; n <= Lines.Length - 1; n++)
            {
                if (started==false && Lines[n].Contains(startingText))
                {
                    started = true;
                }
                if (started ==true)
                {
                    string[] texts = Lines[n].Split(seps, StringSplitOptions.RemoveEmptyEntries);
                    string v="";
                    if (texts.Length >colidx)
                    {
                        if (onlyNumeric = true && cComTools.IsNumeric(texts[colidx]) == true)
                        {
                            v = texts[colidx].Trim();
                        }
                        else
                        {
                            v = texts[colidx].Trim();
                        }
                    }
                    if (v != "")
                    {
                        sb.Append(v + "\r\n");
                    }
                }
                if (started==true && endingConditionApplied == true && Lines[n].Contains(endingText))
                {
                    break;
                }
            }
            File.AppendAllText(strTagetFPN, sb.ToString());
        }


        /// <summary>
        /// 이건 수정할 라인의 번호를 미리 알고 있거나, 포함된 문자 중 일부를 알고 있을때 사용
        /// </summary>
        /// <param name="strSourceFPN"></param>
        /// <param name="strTagetFPN"></param>
        /// <param name="TagetLine"></param>
        /// <param name="strTextToReplace"></param>
        /// <remarks></remarks>
        public static void ReplaceALineInTextFile(string strSourceFPN, string strTagetFPN, int TagetLine, string strTextToReplace)
        {
            try
            {
                string[] Lines = System.IO.File.ReadAllLines(strSourceFPN);
                Lines[TagetLine - 1] = strTextToReplace;
                System.IO.File.WriteAllLines(strTagetFPN, Lines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// 이건 수정할 라인의 번호를 미리 알고 있거나, 포함된 문자 중 일부를 알고 있을때 사용
        /// </summary>
        /// <param name="strSourceFNP"></param>
        /// <param name="strTagetFNP"></param>
        /// <param name="ContainedTextInALine"></param>
        /// <param name="strTextToReplace"></param>
        /// <remarks></remarks>
        public static void ReplaceALineInTextFile(string strSourceFNP, string strTagetFNP, string ContainedTextInALine, string strTextToReplace)
        {
            try
            {
                string[] Lines = System.IO.File.ReadAllLines(strSourceFNP);
                for (int n = 0; n <= Lines.Length - 1; n++)
                {
                    if (Lines[n].Contains(ContainedTextInALine))
                    {
                        Lines[n] = strTextToReplace;
                    }
                }
                System.IO.File.WriteAllLines(strTagetFNP, Lines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public static string[] ReplaceALineInStringArray(string[] sourceArray, string textToFindInALine, string textToReplace)
        {
            try
            {
                for (int n = 0; n <= sourceArray.Length - 1; n++)
                {
                    if (sourceArray[n].Contains(textToFindInALine))
                    {
                        sourceArray[n] = textToReplace;
                    }
                }
                return sourceArray;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public static void ReplaceTextInTextFile(string strSourceFNP, string strTagetFNP, string strTextToFind, string strTextToReplace,
            int startingLineIndex = 0, int endingLineIndex = 0)
        {
            try
            {
                string[] strLines = System.IO.File.ReadAllLines(strSourceFNP);
                int intTotCountLine = strLines.Length;
                int intNLine = 0;
                string strOneLine = null;
                for (intNLine = 0; intNLine <= intTotCountLine - 1; intNLine++)
                {
                    if (endingLineIndex > 0 && intNLine > endingLineIndex)
                    {
                        break;
                    }
                    if (intNLine >= startingLineIndex)
                    {
                        strOneLine = Convert.ToString(strLines[intNLine]);
                        strLines[intNLine] = strOneLine.Replace(strTextToFind, strTextToReplace);
                    }
                }
                System.IO.File.WriteAllLines(strTagetFNP, strLines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public static void ReplaceTextInTextFile(string strSourceFNP, string strTextToFind, string strTextToReplace,
             int startingLineIndex = 0, int endingLineIndex = 0)
        {
            try
            {
                string[] strLines = System.IO.File.ReadAllLines(strSourceFNP);
                int intTotCountLine = strLines.Length;
                int intNLine = 0;
                string strOneLine = null;
                for (intNLine = 0; intNLine <= intTotCountLine - 1; intNLine++)
                {
                    if (endingLineIndex > 0 && intNLine > endingLineIndex)
                    {
                        break;
                    }
                    if (intNLine >= startingLineIndex)
                    {
                        strOneLine = Convert.ToString(strLines[intNLine]);
                        strLines[intNLine] = strOneLine.Replace(strTextToFind, strTextToReplace);
                    }
                }
                System.IO.File.WriteAllLines(strSourceFNP, strLines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }


        public static void ReplaceTextInASCiiRasterRange(cTextFileReaderASC inAscRaster, string strTagetFNP,
            string strTextToFind, string strTextToReplace,
             int tlXcol, int tlYrow, int lrXcol, int lrYrow)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(inAscRaster.HeaderStringAll+"\r\n");
            for (int r = 0; r < inAscRaster.Header.numberRows; r++)
            {
                if (r >= tlYrow && r <= lrYrow)
                {
                    string[] valuesInaRow = inAscRaster.ValuesInOneRowFromTopLeft(r);

                    for (int c = tlXcol; c <= lrXcol; c++)
                    {
                        if (valuesInaRow[c] == strTextToFind)
                        {
                            valuesInaRow[c] = strTextToReplace;
                        }
                    }

                    StringBuilder sbAline = new StringBuilder();
                    for (int c = 0; c < valuesInaRow.Length; c++)
                    {
                        sbAline.Append(valuesInaRow[c]+" ");
                    }
                    sb.Append(sbAline.ToString()+ "\r\n");
                }
                else
                {
                    sb.Append(inAscRaster.OneRowContainsValuesFromTop(r)+ "\r\n");
                }
            }
            File.AppendAllText(strTagetFNP,sb.ToString());

        }

        public static string[] getTextLineArrayFromText(string inString)
        {
            string[] instr;
            string[] outstr= new string [10];

            return outstr;
        }

        public static void ReplaceLinesInTextFile(string strSourceFNP, string strTagetFNP, int staringLineNum, int endingLineNum, string[] textLinesToReplace)
        {
            try
            {
                string[] strLines = System.IO.File.ReadAllLines(strSourceFNP);
                int intTotCountLine = strLines.Length;
                int intNLine = 0;
                string strOneLine = null;
                int nl = 0;
                for (intNLine = staringLineNum-1; intNLine < endingLineNum ; intNLine++)
                {
                    strLines[intNLine] = textLinesToReplace[nl];
                    nl++;
                }
                System.IO.File.WriteAllLines(strTagetFNP, strLines);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public static DataTable ReadTextFileAndSetDataTable(string FPNsource, cTextFile.ValueSeparator valueSeparator, int fieldCount = 0)
        {
            try
            {
                if (string.IsNullOrEmpty(FPNsource) || File.Exists(FPNsource) == false)
                {
                    Console.WriteLine("Source text file and data is invalid.   ");
                    return null;
                }
                StreamReader reader = new StreamReader(FPNsource, System.Text.Encoding.Default);
                DataTable dt = new DataTable();
                dt.BeginLoadData();
                int intL = 0;
                while (!reader.EndOfStream)
                {
                    string line = reader.ReadLine();
                    string[] sep = GetTextFileValueSeparator(valueSeparator);
                    string[] parts = line.Split(sep, StringSplitOptions.RemoveEmptyEntries);
                    if (string.IsNullOrEmpty(parts[0].ToString().Trim()))
                    {
                        Console.WriteLine(String.Format("{0} line has empty value. Exit reading text file process.", intL + 1));
                        break;
                    }
                    int nFieldCount = parts.Length;
                    if (intL == 0)
                    {
                        if (fieldCount > 0 && nFieldCount != fieldCount)
                        {
                            Console.WriteLine("Data series number in text file is different with user settings.   ");
                            return null;
                        }
                        foreach (string ele in parts)
                        {
                            dt.Columns.Add(ele.ToString());
                        }
                    }
                    else
                    {
                        DataRow nr = dt.NewRow();
                        for (int nG1 = 0; nG1 <= nFieldCount - 1; nG1++)
                        {
                            nr.ItemArray[nG1] = parts[nG1].ToString();
                        }
                        dt.Rows.Add(nr);
                    }
                    intL += 1;
                }
                dt.EndLoadData();
                return dt;
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }


        public static string[] ReadGRMoutFileAndMakeStringArray(string[] BaseString, string FPNsource, 
            int rowNtoBeginRead, int colNtoRead, string colName, string[] SeparatorInSourceFile, 
            string SeparatorInReturnArray, bool valueAsInteger)
        {
            if (string.IsNullOrEmpty(FPNsource) || File.Exists(FPNsource) == false)
            {
                Console.WriteLine("Source text file and data is invalid.   ");
                return null;
            }
            string[] Lines = System.IO.File.ReadAllLines(FPNsource);
            int nr = 0;
            if (BaseString[0] == null || string.IsNullOrEmpty(BaseString[0].ToString().Trim()))
            {
                BaseString[nr] = colName;
            }
            else
            {
                BaseString[0] = BaseString[0] + SeparatorInReturnArray + colName;
            }
            for (int nl = 0; nl <= Lines.Length - 1; nl++)
            {
                if (nl >= rowNtoBeginRead - 1)
                {
                    nr += 1;
                    if ((nr + 1) > BaseString.Length)
                    {
                        Console.WriteLine("Input file contains more row count than base string array.   ");
                        break; 
                    }
                    string aLine = Lines[nl];
                    string[] parts = aLine.Split(SeparatorInSourceFile, StringSplitOptions.RemoveEmptyEntries);
                    string value = null;
                    StringBuilder stbAline = new StringBuilder();
                    if (valueAsInteger == true)
                    {
                        value = Convert.ToInt32(parts[colNtoRead - 1]).ToString().Trim();
                    }
                    else
                    {
                        value = parts[colNtoRead - 1].ToString().Trim();
                    }
                    if (BaseString[nr] == null || string.IsNullOrEmpty(BaseString[nr].ToString().Trim()))
                    {
                        BaseString[nr] = value;
                    }
                    else
                    {
                        BaseString[nr] = BaseString[nr] + SeparatorInReturnArray + value;
                    }
                }
            }
            return BaseString;
        }

        public static int GetValueStartingRowNumber(string fpnSource, cTextFile.ValueSeparator valueSeparator, string timeFieldName = "")
        {
            string[] Lines = System.IO.File.ReadAllLines(fpnSource);
            int nr = 0;
            string[] sepArray = GetTextFileValueSeparator(valueSeparator);
            for (int nl = 0; nl <= Lines.Length - 1; nl++)
            {
                string aLine = Lines[nl];
                string[] parts = aLine.Split(sepArray, StringSplitOptions.RemoveEmptyEntries);
                if (parts.Length > 1)
                {
                    if (!string.IsNullOrEmpty(timeFieldName) && parts[0] == timeFieldName)
                    {
                        nr = nl + 2;
                        break; 
                    }
                }
            }
            return nr;
        }
    }
}
