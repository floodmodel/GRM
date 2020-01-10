using System;
using System.Diagnostics;
using System.IO;

namespace GRMCore
{
    public class cGRM
    {
        public enum GRMOutputType
        {
            Discharge,
            Rainfall,
            RainfallCumulative,
            SoilSaturation
        }

        public enum GRMPrintType
        {
            All,
            DischargeFileQ,
            AllQ
        }

        public enum FileOrConst
        {
            File = 0,
            Constant = 1
        }

        public enum CellFlowType : byte
        {
            OverlandFlow,
            ChannelFlow,
            ChannelNOverlandFlow
        }

        public enum SimulationType
        {
            SingleEvent,
            SingleEventPE_SSR,
            RealTime
        }

        public enum DataType
        {
            DTByte,
            DTShort,
            DTInteger,
            DTSingle,
            DTDouble
        }

        public enum GRMParametersAbbreviation
        {
            ISSR, // Initial soil saturation ratio
            MSLS, // Minimum slope of land surface
            MSCB, // Minimum slope of channel bed
            MCW, // Minimum channel width
            CRC, // Channel roughness coefficient
            CLCRC, // Calibration coefficient of Land cover roughness coefficient
            CSD, // Calibration coefficient of soil depth
            CSP, // Calibration coefficient of soil porosity
            CSWS, // Calibration coefficient of soil wetting front suction head
            CSHC // Calibration coefficient of soil hydraulic conductivity
        }

        public enum OutputTableName
        {
            OutputDischarge,
            OutputDepth,
            RainfallWPGrid,
            RainfallWPUpMean,
            FCDataWP,
            FCResStorage,
            WPTS
        }

        public enum GRMFlowDirectionD8 : byte
        {
            NE,
            E,
            SE,
            S,
            SW,
            W,
            NW,
            N,
            NONE
        }

        public enum FlowDirectionType
        {
            StartsFromNE,
            StartsFromN,
            StartsFromE,
            StartsFromE_TauDEM
        }

        public enum UnSaturatedKType
        {
            Constant,
            Linear,
            Exponential
        }


        public static readonly string LONG_DATE_FORMAT = "yyyy/MM/dd HH:mm";
        public static string fpnlog = "";
        public static bool bwriteLog;

        // 결과파일 Tag
        public const string CONST_TAG_DISCHARGE = "Discharge.out";
        public const string CONST_TAG_DEPTH = "Depth.out";
        public const string CONST_TAG_RFGRID = "RFGrid.out";
        public const string CONST_TAG_RFMEAN = "RFUpMean.out";
        public const string CONST_TAG_FCAPP = "FCData.out";
        public const string CONST_TAG_FCSTORAGE = "FCStorage.out";
        public const string CONST_TAG_SWSPARSTEXTFILE = "SWSPars.out";
        public const string CONST_OUTPUT_TABLE_TIME_FIELD_NAME = "DataTime";
        public const string CONST_OUTPUT_TABLE_MEAN_RAINFALL_FIELD_NAME = "Rainfall_Mean";
        public const string CONST_DIST_SSR_DIRECTORY_TAG = "SSD";
        public const string CONST_DIST_RF_DIRECTORY_TAG = "RFD";
        public const string CONST_DIST_RFACC_DIRECTORY_TAG = "RFAccD";
        public const string CONST_DIST_FLOW_DIRECTORY_TAG = "FlowD";
        public const string CONST_DIST_SSR_FILE_HEAD = "ss_";
        public const string CONST_DIST_RF_FILE_HEAD = "rf_";
        public const string CONST_DIST_RFACC_FILE_HEAD = "rfc_";
        public const string CONST_DIST_FLOW_FILE_HEAD = "flow_";

        public const int CONST_MAX_CVW = 7; // 인접상류셀의 최대 개수는 7개

        public const int CONST_MAX_DT_LIMIT_SEC = 600;
        public const int CONST_MIN_DT_LIMIT_SEC = 60;
        public const double CONST_GRAVITY = 9.81;
        public const double CONST_CFL_NUMBER = 1.0;
        public const double CONST_EXPONENTIAL_NUMBER_UNSATURATED_K = 6.4;


        /// <summary>
        ///   최상류 셀의 쐐기 흐름 계산시 p의 수심에 곱해지는 계수
        ///   </summary>
        ///   <remarks></remarks>
        public const double CONST_WEDGE_FLOW_COEFF = 1;
        public const double CONST_WET_AND_DRY_CRITERIA = 0.000001F;
        public const double CONST_TOLERANCE = 0.001F;

        /// <summary>
        ///   암반까지의 깊이[m]
        ///   </summary>
        ///   <remarks></remarks>
        public const double CONST_DEPTH_TO_BEDROCK = 20;
        /// <summary>
        ///   산악지역에서의 암반까지의 깊이[m]
        ///   </summary>
        ///   <remarks></remarks>
        public const double CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN = 10;
        /// <summary>
        ///   비피압대수층까지의 깊이[m]
        ///   </summary>
        ///   <remarks></remarks>
        public const double CONST_DEPTH_TO_UNCONFINED_GROUNDWATERTABEL = 10;
        /// <summary>
        ///   암반에서 비피압대수층 상단까지의 깊이[m]
        ///   </summary>
        ///   <remarks></remarks>
        public const double CONST_UAQ_HEIGHT_FROM_BEDROCK = 5;

         // Private Shared mRange As cParametersRange '이건 GUI에서 필요한 것

        private static bool mGrmStarted;
        private static string mStaticXmlFPN;
        
        public static void Start()
        {
            mGrmStarted = true;
        }

        //public static DataType GetGRMDataTypeByName(string inType)
        //{
        //    switch (inType)
        //    {
        //        case  DataType.DTByte.ToString():
        //            {
        //                return DataType.DTByte;
        //            }

        //        case  DataType.DTShort.ToString():
        //            {
        //                return DataType.DTShort;
        //            }

        //        case  DataType.DTInteger.ToString():
        //            {
        //                return DataType.DTInteger;
        //            }

        //        case  DataType.DTSingle.ToString():
        //            {
        //                return DataType.DTSingle;
        //            }

        //        case  DataType.DTDouble.ToString():
        //            {
        //                return DataType.DTDouble;
        //            }

        //        default:
        //            {
        //                return default(DataType);
        //            }
        //    }
        //}

        public static int GetFDAngleNumber(GRMFlowDirectionD8 fd)
        {
            switch (fd)
            {
                case GRMFlowDirectionD8.NW:
                    {
                        return 315;
                    }

                case GRMFlowDirectionD8.W:
                    {
                        return 270;
                    }

                case GRMFlowDirectionD8.SW:
                    {
                        return 225;
                    }

                case GRMFlowDirectionD8.S:
                    {
                        return 180;
                    }

                case GRMFlowDirectionD8.SE:
                    {
                        return 135;
                    }

                case GRMFlowDirectionD8.E:
                    {
                        return 90;
                    }

                case GRMFlowDirectionD8.NE:
                    {
                        return 45;
                    }

                case GRMFlowDirectionD8.N:
                    {
                        return 0;
                    }

                default:
                    {
                        return -1;
                    }
            }
        }

        public static string AboutInfo_GRM()
        {
            try
            {
                string strToSay;
                strToSay = "Product name : " + cGRM.BuildInfo.ProductName + "       " + "\r\n";
                strToSay = strToSay + "Version : " + cGRM.BuildInfo.ProductVersion + "       " + "\r\n";
                strToSay = strToSay + "File name : " + cGRM.BuildInfo.FileName + "       " + "\r\n";
                strToSay = strToSay + "File description : " + cGRM.BuildInfo.FileDescription + "       " + "\r\n";
                strToSay = strToSay + "Company name : " + cGRM.BuildInfo.CompanyName + "       " + "\r\n";
                strToSay = strToSay + "Comments : " + cGRM.BuildInfo.Comments + "       " + "\r\n";
                strToSay = strToSay + "Homepage : http://www.hygis.net" + "       " + "\r\n";
                strToSay = strToSay + "Contact : @kict.re.kr" + "       " + "\r\n";
                return strToSay;
            }
            catch (Exception ex)
            {
                System.Console.WriteLine(ex.ToString());
                return "";
            }
        }

        public static void writelogAndConsole(string logtxt, bool bwriteLog = false, bool bwriteConsole = false)
        {
            if (bwriteConsole == true)
            { Console.WriteLine(string.Format(logtxt)); }
            if (bwriteLog == true)
            //File.AppendAllText(fpnlog,
            //string.Format("{0:yyyy-MM-dd HH:mm ss}", DateTime.Now) + " " + logtxt + vbCrLf)
            //{ File.AppendAllText(fpnlog, logtxt + "\r\n"); }
            {
                File.AppendAllText(fpnlog,
                    string.Format("{0:yyyy-MM-dd HH:mm:ss}, ", DateTime.Now)
                    + logtxt + "\r\n");
            }
        }


        public static FileVersionInfo BuildInfo
        {
            get
            {
               string fpn= Path.Combine(System.Reflection.Assembly.GetExecutingAssembly().Location);
                return FileVersionInfo.GetVersionInfo(fpn); ;
            }
        }

        public static FileVersionInfo getFileBuildInfo(string FPN)
        {
            return FileVersionInfo.GetVersionInfo(FPN);
        }

        public static bool Started
        {
            get
            {
                return mGrmStarted;
            }
        }
    }
}
