using System;
using System.Collections.Generic;
//using System.Linq;
//using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using System.IO;
using System.Drawing.Imaging;
using System.Windows.Forms;


namespace gentle
{
    public class cImg
    {
        private Dictionary<int, Color> miniRendererColors = new Dictionary<int, Color>();
        public enum RendererType
        {
            Risk,
            WaterDepth
        };

        public enum RendererRange
        {
            RendererFrom0to1,
            RendererFrom0to2,
            RendererFrom0to5,
            RendererFrom0to10,
            RendererFrom0to50,
            RendererFrom0to100,
            RendererFrom0to200,
            RendererFrom0to500,
            RendererFrom0to1000,
            RendererFrom0to5000,
            RendererFrom0to10000,
            RendererFrom0to30000,
            RendererFrom0to50000,
            RendererFrom0to100000
        };


        public cImg(cImg.RendererType renderingType)
        {
            InitializeRendererColor(renderingType);

        }

        private bool InitializeRendererColor(cImg.RendererType renderingType)
        {
            try
            {
                int cr = 0;
                int cg = 0;
                int cb = 0;
                //총 22개
                if (renderingType == RendererType.Risk)
                {
                    cr = 255;
                    cg = 255;
                    cb = 235;
                    miniRendererColors.Add(0, Color.FromArgb(255, cr, cg, cb));
                    cr = 255;
                    cg = 240;
                    cb = 150;
                    miniRendererColors.Add(1, Color.FromArgb(255, cr, cg, cb));
                    cr = 204;
                    cg = 255;
                    cb = 204;
                    miniRendererColors.Add(2, Color.FromArgb(255, cr, cg, cb));
                    cr = 191;
                    cg = 248;
                    cb = 135;
                    miniRendererColors.Add(3, Color.FromArgb(255, cr, cg, cb));
                    cr = 102;
                    cg = 204;
                    cb = 102;
                    miniRendererColors.Add(4, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 128;
                    cb = 0;
                    miniRendererColors.Add(5, Color.FromArgb(255, cr, cg, cb));
                    cr = 119;
                    cg = 249;
                    cb = 220;
                    miniRendererColors.Add(6, Color.FromArgb(255, cr, cg, cb));

                    cr = 204;
                    cg = 238;
                    cb = 255;
                    miniRendererColors.Add(7, Color.FromArgb(255, cr, cg, cb));
                    cr = 153;
                    cg = 204;
                    cb = 255;
                    miniRendererColors.Add(8, Color.FromArgb(255, cr, cg, cb));
                    cr = 153;
                    cg = 153;
                    cb = 255;
                    miniRendererColors.Add(9, Color.FromArgb(255, cr, cg, cb));

                    cr = 102;
                    cg = 102;
                    cb = 255;
                    miniRendererColors.Add(10, Color.FromArgb(255, cr, cg, cb));
                    cr = 51;
                    cg = 51;
                    cb = 255;
                    miniRendererColors.Add(11, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 255;
                    miniRendererColors.Add(12, Color.FromArgb(255, cr, cg, cb));

                    cr = 115;
                    cg = 2;
                    cb = 253;
                    miniRendererColors.Add(13, Color.FromArgb(255, cr, cg, cb));
                    cr = 153;
                    cg = 51;
                    cb = 255;
                    miniRendererColors.Add(14, Color.FromArgb(255, cr, cg, cb));
                    cr = 204;
                    cg = 102;
                    cb = 255;
                    miniRendererColors.Add(15, Color.FromArgb(255, cr, cg, cb));
                    cr = 255;
                    cg = 151;
                    cb = 255;
                    miniRendererColors.Add(16, Color.FromArgb(255, cr, cg, cb));

                    cr = 255;
                    cg = 102;
                    cb = 204;
                    miniRendererColors.Add(17, Color.FromArgb(255, cr, cg, cb));
                    cr = 255;
                    cg = 68;
                    cb = 255;
                    miniRendererColors.Add(18, Color.FromArgb(255, cr, cg, cb));
                    cr = 255;
                    cg = 0;
                    cb = 200;
                    miniRendererColors.Add(19, Color.FromArgb(255, cr, cg, cb));
                    cr = 240;
                    cg = 0;
                    cb = 100;
                    miniRendererColors.Add(20, Color.FromArgb(255, cr, cg, cb));
                    cr = 255;
                    cg = 0;
                    cb = 0;
                    miniRendererColors.Add(21, Color.FromArgb(255, cr, cg, cb));
                }

                if (renderingType == RendererType.WaterDepth)
                {
                    //cr = 200;
                    //cg = 255;
                    //cb = 255;
                    cr = 255;
                    cg =233;
                    cb = 210;
                    miniRendererColors.Add(0, Color.FromArgb(255, cr, cg, cb));
                    cr = 150;
                    cg = 255;
                    cb = 255;
                    miniRendererColors.Add(1, Color.FromArgb(255, cr, cg, cb));
                    cr = 100;
                    cg = 255;
                    cb = 255;
                    miniRendererColors.Add(2, Color.FromArgb(255, cr, cg, cb));
                    cr = 50;
                    cg = 255;
                    cb = 255;
                    miniRendererColors.Add(3, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 255;
                    cb = 255;
                    miniRendererColors.Add(4, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 225;
                    cb = 255;
                    miniRendererColors.Add(5, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 200;
                    cb = 255;
                    miniRendererColors.Add(6, Color.FromArgb(255, cr, cg, cb));

                    cr = 0;
                    cg = 175;
                    cb = 255;
                    miniRendererColors.Add(7, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 150;
                    cb = 255;
                    miniRendererColors.Add(8, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 125;
                    cb = 255;
                    miniRendererColors.Add(9, Color.FromArgb(255, cr, cg, cb));

                    cr = 0;
                    cg = 100;
                    cb = 255;
                    miniRendererColors.Add(10, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 75;
                    cb = 255;
                    miniRendererColors.Add(11, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 50;
                    cb = 255;
                    miniRendererColors.Add(12, Color.FromArgb(255, cr, cg, cb));

                    cr = 0;
                    cg = 25;
                    cb = 255;
                    miniRendererColors.Add(13, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 255;
                    miniRendererColors.Add(14, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 225;
                    miniRendererColors.Add(15, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 200;
                    miniRendererColors.Add(16, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 175;
                    miniRendererColors.Add(17, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 150;
                    miniRendererColors.Add(18, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 125;
                    miniRendererColors.Add(19, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg =0;
                    cb = 100;
                    miniRendererColors.Add(20, Color.FromArgb(255, cr, cg, cb));
                    cr = 0;
                    cg = 0;
                    cb = 75;
                    miniRendererColors.Add(21, Color.FromArgb(255, cr, cg, cb));
                }

                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return false;
            }
        }

        public static Bitmap GetBMPImageByFileStream(string fpnSource)
        {
            FileStream streamBMP = new FileStream(fpnSource, FileMode.Open);
            Bitmap img = (Bitmap)Bitmap.FromStream(streamBMP);

            streamBMP.Dispose();
            return img;
        }

        public Bitmap MakeImgFileAndGetImgUsingArrayFromTL(string imgFPNtoMake, double[,] array, float imgWidth,
            float imgHeight, RendererRange rangeType, double nullValue = -9999)
        {
            try
            {
                int colxCount = array.GetLength(0);
                int rowyCount = array.GetLength(1);
                int CellCount = (colxCount * rowyCount);
                int CellWbmp = 0;
                int CellHbmp = 0;
                CellWbmp = Convert.ToInt32(imgWidth / colxCount);
                CellHbmp = Convert.ToInt32(imgHeight / rowyCount);
                if (CellWbmp < CellHbmp)
                {
                    CellHbmp = CellWbmp;
                }
                else
                {
                    CellWbmp = CellHbmp;
                }
                Bitmap bm = new Bitmap(Convert.ToInt32(colxCount * CellWbmp) + 1, Convert.ToInt32(rowyCount * CellHbmp) + 1);
                Graphics gr = Graphics.FromImage(bm);
                //gr.Clear(Color.White);
                for (int r = 0; r < rowyCount; r++)
                {
                    for (int c = 0; c < colxCount; c++)
                    {
                        Rectangle rec = new Rectangle(c * CellWbmp, r * CellHbmp, CellWbmp, CellHbmp);
                        Color cToShow = DefaultNullColor;
                        if (double.TryParse(array[c, r].ToString(), out double dv) == true)
                        {
                            cToShow = GetColorFromMemoryRendererInDifferentInterval(dv, rangeType, nullValue);
                        }
                        SolidBrush brsh = new SolidBrush(cToShow);
                        gr.FillRectangle(brsh, rec);
                    }
                }

                ////이건 pixel 단위로 처리
                //var options = new ParallelOptions { MaxDegreeOfParallelism = -1 };
                //Parallel.For(0, bm.Height, options, y =>
                //{
                //    for (int x = 0; x < bm.Width; x++)
                //    {
                //        Color cToShow = DefaultNullColor;
                //        if (double.TryParse(array[x, y].ToString(), out double dv) == true)
                //        {
                //            cToShow = GetColorFromMemoryRendererInDifferentInterval(dv, rangeType, nullValue);
                //        }

                //        bm.SetPixel(x, y, cToShow);
                //    }
                //});

                ////병렬로 하니까.. 애러가 난다.. 진행이 안된다. gr, bm을 공유할 수 없다.
                //var options = new ParallelOptions { MaxDegreeOfParallelism = -1 };
                //Parallel.For(0, rowyCount, options, delegate (int r)
                //{
                //    for (int c = 0; c < colxCount; c++)
                //    {
                //        Rectangle rec = new Rectangle(c * CellWbmp, r * CellHbmp, CellWbmp, CellHbmp);
                //        Color cToShow = DefaultNullColor;
                //        if (double.TryParse(array[c, r].ToString(), out double dv) == true)
                //        {
                //            cToShow = GetColorFromMemoryRendererInDifferentInterval(dv, rangeType, nullValue);
                //        }
                //        SolidBrush brsh = new SolidBrush(cToShow);
                //        gr.FillRectangle(brsh, rec);
                //    }
                //});
                bm.Save(imgFPNtoMake, ImageFormat.Png);
                return bm;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return Nothing;
            }
        }



        public void MakeImgFileUsingArrayFromTL(string imgFPNtoMake, double[,] array, float imgWidth,
            float imgHeight, RendererRange rangeType, double nullValue = -9999)
        {
            try
            {
                int colxCount = array.GetLength(0);
                int rowyCount = array.GetLength(1);
                int CellCount = (colxCount * rowyCount);
                int CellWbmp = 0;
                int CellHbmp = 0;
                CellWbmp = Convert.ToInt32(imgWidth / colxCount);
                CellHbmp = Convert.ToInt32(imgHeight / rowyCount);
                if (CellWbmp < CellHbmp)
                {
                    CellHbmp = CellWbmp;
                }
                else
                {
                    CellWbmp = CellHbmp;
                }
                Bitmap bm = new Bitmap(Convert.ToInt32(colxCount * CellWbmp) + 1, Convert.ToInt32(rowyCount * CellHbmp) + 1);
                Graphics gr = Graphics.FromImage(bm);
                for (int r = 0; r < rowyCount; r++)
                {
                    for (int c = 0; c < colxCount; c++)
                    {
                        Rectangle rec = new Rectangle(c * CellWbmp, r * CellHbmp, CellWbmp, CellHbmp);
                        Color cToShow = DefaultNullColor;
                        if (double.TryParse(array[c, r].ToString(), out double dv) == true)
                        {
                            cToShow = GetColorFromMemoryRendererInDifferentInterval(dv, rangeType, nullValue);
                        }
                        SolidBrush brsh = new SolidBrush(cToShow);
                        gr.FillRectangle(brsh, rec);
                    }
                }
                bm.Save(imgFPNtoMake, ImageFormat.Png);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        public Bitmap MakeImgFileAndGetImgUsingArrayFromTL_InParallel(string imgFPNtoMake, double[,] array, float imgWidth,
           float imgHeight, RendererRange rangeType, double nullValue = -9999)
        {
            try
            {
                unsafe
                {
                    int colxCount = array.GetLength(0);
                    int rowyCount = array.GetLength(1);
                    int CellCount = (colxCount * rowyCount);
                    int CellWbmp = 0;
                    int CellHbmp = 0;
                    CellWbmp = Convert.ToInt32(imgWidth / colxCount);
                    CellHbmp = Convert.ToInt32(imgHeight / rowyCount);
                    if (CellHbmp <= 0) { CellHbmp = 1; }
                    if (CellWbmp <= 0) { CellWbmp = 1; }
                    if (CellWbmp < CellHbmp)
                    {
                        CellHbmp = CellWbmp;
                    }
                    else
                    {
                        CellWbmp = CellHbmp;
                    }
                    Bitmap bm = new Bitmap(Convert.ToInt32(colxCount * CellWbmp) + 1, Convert.ToInt32(rowyCount * CellHbmp) + 1);
                    //Bitmap bm = new Bitmap(Convert.ToInt32(imgWidth) + 1, Convert.ToInt32(imgHeight) + 1);
                    BitmapData bitmapData = bm.LockBits(new Rectangle(0, 0, bm.Width, bm.Height), ImageLockMode.ReadWrite, bm.PixelFormat);
                    int bytesPerPixel = System.Drawing.Bitmap.GetPixelFormatSize(bitmapData.PixelFormat) / 8;
                    int heightInPixels = bitmapData.Height;
                    int widthInBytes = bitmapData.Width * bytesPerPixel;
                    byte* PtrFirstPixel = (byte*)bitmapData.Scan0;
                    Parallel.For(0, rowyCount, y =>
                    {
                    //    for (int y = 0; y < rowyCount; y++)
                    //{
                        byte* currentLine = PtrFirstPixel + (y * bitmapData.Stride);
                        for (int x = 0; x < colxCount; x++)
                        {
                            Color cToShow = DefaultNullColor;
                            if (double.TryParse(array[x, y].ToString(), out double dv) == true)
                            {
                                    cToShow = GetColorFromMemoryRendererInDifferentInterval(dv, rangeType, nullValue);
                            }
                            currentLine[x * bytesPerPixel] = (byte)cToShow.B;
                            currentLine[x * bytesPerPixel + 1] = (byte)cToShow.G;
                            currentLine[x * bytesPerPixel + 2] = (byte)cToShow.R;
                            currentLine[x * bytesPerPixel + 3] = (byte)255;
                        }
                        //}
                    });

                    bm.UnlockBits(bitmapData);
                    bm.Save(imgFPNtoMake, ImageFormat.Png);
                    return bm;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return Nothing;
            }
        }

        public Bitmap MakeImgFileAndGetImgSameIntervalUsingArrayFromTL_InParallel(string imgFPNtoMake, double[,] array, float imgWidth,
         float imgHeight, double rendererMaxV, double nullValue = -9999)
        {
            try
            {
                unsafe
                {
                    int colxCount = array.GetLength(0);
                    int rowyCount = array.GetLength(1);
                    int CellCount = (colxCount * rowyCount);
                    int CellWbmp = 0;
                    int CellHbmp = 0;
                    CellWbmp = Convert.ToInt32(imgWidth / colxCount);
                    CellHbmp = Convert.ToInt32(imgHeight / rowyCount);
                    if (CellHbmp <= 0) { CellHbmp = 1; }
                    if (CellWbmp <= 0) { CellWbmp = 1; }
                    if (CellWbmp < CellHbmp)
                    {
                        CellHbmp = CellWbmp;
                    }
                    else
                    {
                        CellWbmp = CellHbmp;
                    }
                    Bitmap bm = new Bitmap(Convert.ToInt32(colxCount * CellWbmp) + 1, Convert.ToInt32(rowyCount * CellHbmp) + 1);
                    BitmapData bitmapData = bm.LockBits(new Rectangle(0, 0, bm.Width, bm.Height), ImageLockMode.ReadWrite, bm.PixelFormat);
                    int bytesPerPixel = System.Drawing.Bitmap.GetPixelFormatSize(bitmapData.PixelFormat) / 8;
                    int heightInPixels = bitmapData.Height;
                    int widthInBytes = bitmapData.Width * bytesPerPixel;
                    byte* PtrFirstPixel = (byte*)bitmapData.Scan0;
                    Parallel.For(0, rowyCount, y =>
                    {
                        byte* currentLine = PtrFirstPixel + (y * bitmapData.Stride);
                        for (int x = 0; x < colxCount; x++)
                        {
                            Color cToShow = DefaultNullColor;
                            if (double.TryParse(array[x, y].ToString(), out double dv) == true)
                            {
                                    cToShow = GetColorFromMemoryRendererInSameInterval(dv, rendererMaxV, nullValue);
                            }
                            currentLine[x * bytesPerPixel] = (byte)cToShow.B;
                            currentLine[x * bytesPerPixel + 1] = (byte)cToShow.G;
                            currentLine[x * bytesPerPixel + 2] = (byte)cToShow.R;
                            currentLine[x * bytesPerPixel + 3] = (byte)255;
                        }
                    });

                    bm.UnlockBits(bitmapData);
                    bm.Save(imgFPNtoMake, ImageFormat.Png);
                    return bm;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return Nothing;
            }
        }



        public bool MakeImgFileUsingASCfileFromTL(string inASCFPN, string imgFpnToMake, RendererRange rangeType, float width,
            float height, Color defaultColor, double nullValue = -9999)
        {
            gentle.cAscRasterReader ascF = new cAscRasterReader(inASCFPN);
            int LayerCellWcount = ascF.Header.numberRows;
            int LayerCellHcount = ascF.Header.numberRows;
            int CellCount = (LayerCellWcount * LayerCellHcount);
            int CellWbmp = 0;
            int CellHbmp = 0;
            CellWbmp = Convert.ToInt32(width / LayerCellWcount);
            CellHbmp = Convert.ToInt32(height / LayerCellHcount);
            if (CellWbmp < CellHbmp)
            {
                CellHbmp = CellWbmp;
            }
            else
            {
                CellWbmp = CellHbmp;
            }
            Bitmap bm = new Bitmap(Convert.ToInt32(LayerCellWcount * CellWbmp) + 1, Convert.ToInt32(LayerCellHcount * CellHbmp) + 1);
            Graphics gr = Graphics.FromImage(bm);
            gr.Clear(Color.White);
            for (int r = 0; r <= LayerCellHcount - 1; r++)
            {
                //string[] aRow = ascF.ValuesInOneRowFromTopLeft(r);
                for (int c = 0; c <= LayerCellWcount - 1; c++)
                {
                    Rectangle rec = new Rectangle(c * CellWbmp, r * CellHbmp, CellWbmp, CellHbmp);
                    Color cToShow = default(Color);
                    cToShow = defaultColor;
                    float value = Convert.ToSingle(ascF.ValueFromTL(c, r));
                    cToShow = GetColorFromMemoryRendererInDifferentInterval(value, rangeType, nullValue);
                    SolidBrush brsh = new SolidBrush(cToShow);
                    gr.FillRectangle(brsh, rec);
                }
            }
            bm.Save(imgFpnToMake, ImageFormat.Png);
            return true;
        }


        public bool MakeImgFileUsingASCfileFromTL(string inASCFPN, string imgFPNtoMake, RendererRange rangeType, double nullValue=-9999)
        {
            cAscRasterReader ascF = new cAscRasterReader(inASCFPN);
            int LayerCellWcount = ascF.Header.numberCols;
            int LayerCellHcount = ascF.Header.numberRows;
            int CellCount = (LayerCellWcount * LayerCellHcount);
            int CellWbmp = 1;
            int CellHbmp = 1;
            Bitmap bm = new Bitmap(CellWbmp * LayerCellWcount, CellHbmp * LayerCellHcount);
            Graphics gr = Graphics.FromImage(bm);
            gr.Clear(Color.White);
            //int ncols = 0;
            //int nrows = 0;
            for (int r = 0; r <= LayerCellHcount - 1; r++)
            {
                //string[] aRow = ascF.ValuesInOneRowFromTopLeft(r);
                for (int c = 0; c <= LayerCellWcount - 1; c++)
                {
                    Rectangle rec = new Rectangle(c * CellWbmp, r * CellHbmp, CellWbmp, CellHbmp);
                    Color cToShow = default(Color);
                    double value = Convert.ToDouble(ascF .ValueFromTL (c,r));
                    cToShow = GetColorFromMemoryRendererInDifferentInterval(value, rangeType, nullValue);
                    SolidBrush brsh = new SolidBrush(cToShow);
                    gr.FillRectangle(brsh, rec);
                }
            }
            bm.Save(imgFPNtoMake, ImageFormat.Png);
            return true;
        }

        public static Image AutosizeImage(string ImagePN, PictureBox picBox, PictureBoxSizeMode pSizeMode = PictureBoxSizeMode.CenterImage)
        {
            try
            {
                picBox.Image = null;
                picBox.SizeMode = pSizeMode;
                if (System.IO.File.Exists(ImagePN))
                {
                    Bitmap imgOrg = default(Bitmap);
                    Bitmap imgShow = default(Bitmap);
                    Graphics g = default(Graphics);
                    double divideBy = 0;
                    double divideByH = 0;
                    double divideByW = 0;
                    imgOrg = GetBMPImageByFileStream(ImagePN);
                    divideByW = imgOrg.Width / picBox.Width;
                    divideByH = imgOrg.Height / picBox.Height;

                    //===============================================
                    // 이건 pb 보다 작거나 같거나 채우는 것,, 작은 것을 큰 곳에 채울 경우 흐려짐.
                    //If divideByW > divideByH Then
                    //    divideBy = divideByW
                    //Else
                    //    divideBy = divideByH
                    //End If

                    //imgShow = New Bitmap(CInt(CDbl(imgOrg.Width) / divideBy), CInt(CDbl(imgOrg.Height) / divideBy))
                    //imgShow.SetResolution(imgOrg.HorizontalResolution, imgOrg.VerticalResolution)
                    //g = Graphics.FromImage(imgShow)
                    //g.InterpolationMode = Drawing2D.InterpolationMode.HighQualityBicubic
                    //g.DrawImage(imgOrg, New Rectangle(0, 0, CInt(CDbl(imgOrg.Width) / divideBy), CInt(CDbl(imgOrg.Height) / divideBy)), 0, 0, imgOrg.Width, imgOrg.Height, GraphicsUnit.Pixel)
                    //g.Dispose()
                    //===============================================

                    //===============================================
                    // 이건 pb 보다 큰 이미지를 채우는 것,, 작은 것은 작게 보임
                    if (divideByW > 1 | divideByH > 1)
                    {
                        if (divideByW > divideByH)
                        {
                            divideBy = divideByW;
                        }
                        else
                        {
                            divideBy = divideByH;
                        }
                        imgShow = new Bitmap(Convert.ToInt32(Convert.ToDouble(imgOrg.Width) / divideBy), Convert.ToInt32(Convert.ToDouble(imgOrg.Height) / divideBy));
                        imgShow.SetResolution(imgOrg.HorizontalResolution, imgOrg.VerticalResolution);
                        g = Graphics.FromImage(imgShow);
                        g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
                        g.DrawImage(imgOrg, new Rectangle(0, 0, Convert.ToInt32(Convert.ToDouble(imgOrg.Width) / divideBy), Convert.ToInt32(Convert.ToDouble(imgOrg.Height) / divideBy)), 0, 0, imgOrg.Width, imgOrg.Height, GraphicsUnit.Pixel);
                        g.Dispose();
                    }
                    else
                    {
                        imgShow = new Bitmap(imgOrg.Width, imgOrg.Height);
                        imgShow.SetResolution(imgOrg.HorizontalResolution, imgOrg.VerticalResolution);
                        g = Graphics.FromImage(imgShow);
                        g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
                        g.DrawImage(imgOrg, new Rectangle(0, 0, imgOrg.Width, imgOrg.Height), 0, 0, imgOrg.Width, imgOrg.Height, GraphicsUnit.Pixel);
                        g.Dispose();
                    }
                    //===============================================
                    imgOrg.Dispose();
                    return imgShow;
                }
                else
                {
                    return null;
                }
            }
            catch (Exception ex)
            {
             Console .WriteLine (ex);
                return null;
            }
        }

        public Color GetColorFromMemoryRendererInDifferentInterval(double value, cImg.RendererRange inRangeType, double nullValue=-9999)
        {
            try
            {
                //int hLimint = 0;
                int c1top = 0;
                int c2top = 0;
                int c3top = 0;
                double  divider1 = 0;
                double divider2 = 0;
                double divider3 = 0;
                if (value == nullValue) { return DefaultNullColor; }                 
                if (value == 0) { return miniRendererColors[0]; }                    
                switch (inRangeType)
                {
                    case RendererRange.RendererFrom0to1:
                        if (value > 1)
                        {
                            return miniRendererColors[21];
                        }
                        else
                        {
                            return miniRendererColors[(int)((value * 400) / 20)];
                        }
                    case RendererRange.RendererFrom0to2:
                        if (value > 2)
                        {
                            return miniRendererColors[21];
                        }
                        else
                        {
                            return miniRendererColors[(int)((value * 200) / 20)];
                        }
                    case RendererRange.RendererFrom0to5:
                        c1top = 1;
                        divider1 = 0.2;
                        c2top = 3;
                        divider2 = 0.2;
                        c3top = 5;
                        divider3 = 0.5;
                        break;
                    case RendererRange.RendererFrom0to10:
                        c1top = 1;
                        divider1 = 0.2;
                        c2top = 7;
                        divider2 = 0.5;
                        c3top = 10;
                        divider3 = 1;
                        break;
                    case RendererRange.RendererFrom0to50:
                        c1top = 10;
                        divider1 = 2;
                        c2top = 30;
                        divider2 = 2;
                        c3top = 50;
                        divider3 = 5;
                        break;
                    case RendererRange.RendererFrom0to100:
                        c1top = 10;
                        divider1 = 2;
                        c2top = 70;
                        divider2 = 5;
                        c3top = 100;
                        divider3 = 10;
                        break;
                    case RendererRange.RendererFrom0to200:
                        c1top = 10;
                        divider1 = 2;
                        c2top = 120;
                        divider2 = 10;
                        c3top = 200;
                        divider3 = 20;
                        break;
                    case RendererRange.RendererFrom0to500:
                        c1top = 100;
                        divider1 = 20;
                        c2top = 300;
                        divider2 = 20;
                        c3top = 500;
                        divider3 = 50;
                        break;
                    case RendererRange.RendererFrom0to1000:
                        c1top = 100;
                        divider1 = 20;
                        c2top = 700;
                        divider2 = 50;
                        c3top = 1000;
                        divider3 = 100;
                        break;
                    case RendererRange.RendererFrom0to5000:
                        c1top = 1000;
                        divider1 = 200;
                        c2top = 3000;
                        divider2 = 200;
                        c3top = 5000;
                        divider3 = 500;
                        break;
                    case RendererRange.RendererFrom0to10000:
                        c1top = 1000;
                        divider1 = 200;
                        c2top = 7000;
                        divider2 = 500;
                        c3top = 10000;
                        divider3 = 1000;
                        break;
                    case RendererRange.RendererFrom0to30000:
                        c1top = 1000;
                        divider1 = 200;
                        c2top = 10000;
                        divider2 = 1000;
                        c3top = 30000;
                        divider3 = 5000;
                        break;
                    case RendererRange.RendererFrom0to50000:
                        c1top = 10000;
                        divider1 = 2000;
                        c2top = 30000;
                        divider2 = 2000;
                        c3top = 50000;
                        divider3 = 5000;
                        break;
                    case RendererRange.RendererFrom0to100000:
                        c1top = 10000;
                        divider1 = 2000;
                        c2top = 70000;
                        divider2 = 5000;
                        c3top = 100000;
                        divider3 = 10000;
                        break;
                }
                int valueToCal = 0;
                int dToCal1 = 0;
                int dToCal2 = 0;
                int dToCal3 = 0;
                if (value < 1 || divider1 < 1 || divider2 < 1 || divider3 < 1)
                {
                    valueToCal = (int) (value * 100);
                    dToCal1 = (int)(divider1 * 100);
                    dToCal2 = (int)(divider2 * 100);
                    dToCal3 = (int)(divider3 * 100);
                }
                else
                {
                    valueToCal = (int)value;
                    dToCal1 = (int)divider1;
                    dToCal2 = (int)divider2;
                    dToCal3 = (int)divider3;
                }
                if (value <= c1top) { return miniRendererColors[valueToCal / dToCal1]; }          
                if (c1top < value && value <= c2top) { return miniRendererColors[valueToCal / dToCal2]; }                    
                if (c2top < value && value <= c3top) { return miniRendererColors[valueToCal / dToCal3]; }                   
                if (value > c3top) { return miniRendererColors[21]; }
                return DefaultNullColor;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return DefaultNullColor;
            }
        }


        public Color GetColorFromMemoryRendererInSameInterval(double value, double maxV, double nodataValue)
        {
            try
            {
                if (value < 0 || value ==nodataValue )
                {
                    return DefaultNullColor;
                }
                else if (value == 0)
                {
                    return miniRendererColors[0];
                }
                else if (value > maxV)
                {
                    return miniRendererColors[21];
                }
                else
                {
                    double multiple = 400 / maxV;
                    return miniRendererColors[(int)(value* multiple / 20)];
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return DefaultNullColor;
            }
        }


        public Color GetColorFromMemoryRendererInSameInterval(float value, cImg.RendererRange inRendererType, double nodataValue)
        {
            try
            {
                int hLimint = 0;
                double divider = 0;
                switch (inRendererType)
                {
                    case RendererRange.RendererFrom0to1:
                        hLimint = 1;
                        divider = 0.05;
                        break;
                    case RendererRange.RendererFrom0to10:
                        hLimint = 10;
                        divider = 0.5;
                        break;
                    case RendererRange.RendererFrom0to50:
                        hLimint = 50;
                        divider = 2.5;
                        break;
                    case RendererRange.RendererFrom0to100:
                        hLimint = 100;
                        divider = 5;
                        break;
                    case RendererRange.RendererFrom0to200:
                        hLimint = 200;
                        divider = 10;
                        break;
                    case RendererRange.RendererFrom0to500:
                        hLimint = 500;
                        divider = 25;
                        break;
                    case RendererRange.RendererFrom0to1000:
                        hLimint = 1000;
                        divider = 50;
                        break;
                    case RendererRange.RendererFrom0to5000:
                        hLimint = 5000;
                        divider = 250;
                        break;
                    case RendererRange.RendererFrom0to10000:
                        hLimint = 10000;
                        divider = 500;
                        break;
                    case RendererRange.RendererFrom0to30000:
                        hLimint = 30000;
                        divider = 1500;
                        break;
                    case RendererRange.RendererFrom0to50000:
                        hLimint = 50000;
                        divider = 2500;
                        break;
                    case RendererRange.RendererFrom0to100000:
                        hLimint = 100000;
                        divider = 5000;
                        break;
                    default:
                        return DefaultNullColor;
                }
                if (value < 0|| value== nodataValue)
                {
                    return DefaultNullColor;
                }
                else if (value == 0)
                {
                    return miniRendererColors[0];
                }
                else if (value > hLimint)
                {
                    return miniRendererColors[21];
                }
                else
                {
                    return miniRendererColors[(int)(value / divider)];
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return DefaultNullColor;
            }
        }


        /// <summary>
        /// 흰색
        /// </summary>
        /// <value></value>
        /// <returns></returns>
        /// <remarks></remarks>
        public Color DefaultNullColor
        {
            get
            {
                //return Color.FromArgb(255, 255, 255, 255);
                return Color.Transparent;
            }
        }


        public Color iniColor(int key)
        {        
                return miniRendererColors[key];      
        }

        public Dictionary<int, Color> iniColors
        {
            get
            {
                return miniRendererColors;
            }
        }

        public Bitmap Nothing
        {
            get
            {
                return Nothing;
            }
        }
    }
}
