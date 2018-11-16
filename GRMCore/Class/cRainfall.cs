using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using gentle;
using System.IO;

namespace GRMCore
{
    public class cRainfall
    {
        public enum RainfallDataType
        {
            TextFileMAP,
            TextFileASCgrid,
            TextFileASCgrid_mmPhr
        }

        public struct RainfallData
        {
            public int Order;
            public string DataTime;
            public string Rainfall; // map 에서는 강우량 값, asc에서는 파일 이름
            public string FilePath;
            public string FileName;
        }

        public Nullable<RainfallDataType> mRainfallDataType;

        /// <summary>
        ///   강우의 시간간격 [minutes]
        ///   </summary>
        ///   <remarks></remarks>
        public Nullable<int> mRainfallinterval;
        public List<RainfallData> mlstRainfallData;
        private string mRainfallDataFilePathName = "";

        /// <summary>
        ///   Delta T시간 동안의 유역평균강우량
        ///   </summary>
        ///   <remarks></remarks>
        public double mRFMeanForDt_m;

        public void GetValues(cProject prj)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prj.PrjFile.ProjectSettings.Rows[0];
            if (row.IsRainfallDataTypeNull() == false)
            {
                switch (row.RainfallDataType)
                {
                    case nameof(RainfallDataType.TextFileMAP):
                        {
                            mRainfallDataType = RainfallDataType.TextFileMAP;
                            break;
                        }

                    case nameof(RainfallDataType.TextFileASCgrid):
                        {
                            mRainfallDataType = RainfallDataType.TextFileASCgrid;
                            break;
                        }

                    case nameof(RainfallDataType.TextFileASCgrid_mmPhr):
                        {
                            mRainfallDataType = RainfallDataType.TextFileASCgrid_mmPhr;
                            break;
                        }
                }
                int v = 0;
                if (int.TryParse(row.RainfallInterval, out v) == true)
                { mRainfallinterval = v; }
                else
                { mRainfallinterval = 0; }

            }
            mRainfallDataFilePathName = row.RainfallDataFile;
            if (mRainfallDataType.HasValue == true)
            {
                mlstRainfallData = new List<RainfallData>();
                string[] Lines = System.IO.File.ReadAllLines(mRainfallDataFilePathName);
                for (int n = 0; n < Lines.Length; n++)
                {
                    if (Lines[n].Trim() == "") { break; }
                    RainfallData r = new RainfallData();
                    r.Order = n + 1;
                    switch (row.RainfallDataType)
                    {
                        case nameof(RainfallDataType.TextFileASCgrid):
                            {
                                r.Rainfall = Path.GetFileName(Lines[n].ToString());
                                r.FileName = Path.GetFileName(Lines[n].ToString());
                                r.FilePath = Path.GetDirectoryName(Lines[n].ToString());
                                break;
                            }
                        case nameof(RainfallDataType.TextFileASCgrid_mmPhr):
                            {
                                r.Rainfall = Path.GetFileName(Lines[n].ToString());
                                r.FileName = Path.GetFileName(Lines[n].ToString());
                                r.FilePath = Path.GetDirectoryName(Lines[n].ToString());
                                break;
                            }
                        case nameof(RainfallDataType.TextFileMAP):
                            {
                                r.Rainfall = Lines[n].ToString();
                                r.FileName = Path.GetFileName(mRainfallDataFilePathName);
                                r.FilePath = Path.GetDirectoryName(mRainfallDataFilePathName);
                                break;
                            }
                    }
                    if (prj.generalSimulEnv.mIsDateTimeFormat == true)
                        r.DataTime = cComTools.GetTimeToPrintOut(true, prj.generalSimulEnv.mSimStartDateTime, System.Convert.ToInt32(mRainfallinterval * n));
                    else
                        r.DataTime = System.Convert.ToString(mRainfallinterval * n);
                    mlstRainfallData.Add(r);
                }
            }
        }

        // Public Sub SetValues(ByVal prjdb As GRMProject)
        // Dim row As GRMProject.ProjectSettingsRow = CType(prjdb.ProjectSettings.Rows(0), GRMProject.ProjectSettingsRow)
        // If mRainfallDataType.HasValue Then
        // With row
        // .RainfallDataType = mRainfallDataType.ToString
        // .RainfallInterval = mRainfallinterval.Value
        // If mRFStartDateTime IsNot Nothing Then
        // .RainfallStartsFrom = mRFStartDateTime
        // .RainfallEndsAt = mRainfallEndDateTime
        // Else
        // .RainfallStartsFrom = Nothing
        // .RainfallEndsAt = Nothing
        // End If
        // .RainfallDuration = RFDuration
        // .RainfallDataFile = RFDataFilePathName
        // End With
        // mdtRainfallinfo.AcceptChanges()
        // For Each r As DataRow In mdtRainfallinfo.Rows
        // r.SetAdded()
        // Next
        // End If
        // End Sub

        public static void ReadRainfall(cProject project, cRainfall.RainfallDataType eRainfallDataType, List<RainfallData> lstRFData, int RFinterval_MIN, int nowRFOrder, bool isparallel)
        {
            int rfIntervalSEC = RFinterval_MIN * 60;
            RainfallData rfRow = project.rainfall.GetRFdataByOrder(lstRFData, nowRFOrder);
            string RFfpn = Path.Combine(rfRow.FilePath, rfRow.FileName);
            double cellSize = project.watershed.mCellSize;
            sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = 0;
            foreach (int wpCVID in project.watchPoint.WPCVidList)
            {
                project.watchPoint.RFReadIntensitySumUpWs_mPs[wpCVID] = 0;
            }
            try
            {
                if (eRainfallDataType == cRainfall.RainfallDataType.TextFileASCgrid || eRainfallDataType == cRainfall.RainfallDataType.TextFileASCgrid_mmPhr)
                {
                    cAscRasterReader ascReader = new cAscRasterReader(RFfpn);
                    int rowCount = project.watershed.rowCount;
                    int colCount = project.watershed.colCount;
                    if (isparallel == true)
                    {
                        ParallelOptions options = new ParallelOptions();
                        options.MaxDegreeOfParallelism = sThisSimulation.MaxDegreeOfParallelism;
                        Parallel.For(0, rowCount, options, delegate (int ry)
                        {
                            // Dim RFs As String() = ascReader.ValuesInOneRowFromTopLeft(ry)
                            for (int cx = 0; cx < colCount; cx++)
                            {
                                if (project.WSCells[cx, ry] == null || project.WSCells[cx, ry].toBeSimulated == -1)
                                    continue;
                                int cvan = project.WSCells[cx, ry].CVID - 1;
                                double inRF_mm = ascReader.ValueFromTL(cx, ry);
                                if (eRainfallDataType == cRainfall.RainfallDataType.TextFileASCgrid_mmPhr)
                                    inRF_mm = inRF_mm / (60.0 / (double)RFinterval_MIN);
                                CalRFintensity_mPsec(project.CVs[cvan], inRF_mm, rfIntervalSEC);
                            }
                        });
                        for (int ry = 0; ry < rowCount; ry++)
                        {
                            for (int cx = 0; cx < colCount; cx++)
                            {
                                if (project.WSCells[cx, ry] != null && project.WSCells[cx, ry].toBeSimulated == 1)
                                {
                                    sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = 
                                        sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.WSCells[cx, ry].RFReadintensity_mPsec;
                                    int cvan = project.WSCells[cx, ry].CVID - 1;
                                    CalRFSumForWPUpWSWithRFGrid(cvan);
                                }
                            }
                        }
                    }
                    else
                    {
                        for (int ry = 0; ry < rowCount; ry++)
                        {
                            // Dim RFs As String() = ascReader.ValuesInOneRowFromTopLeft(ry)
                            for (int cx = 0; cx < colCount; cx++)
                            {
                                if (project.WSCells[cx, ry] == null || project.WSCells[cx, ry].toBeSimulated == -1)
                                    continue;
                                int cvan = project.WSCells[cx, ry].CVID - 1;
                                double inRF_mm = ascReader.ValueFromTL(cx, ry);
                                if (eRainfallDataType == cRainfall.RainfallDataType.TextFileASCgrid_mmPhr)
                                {
                                    inRF_mm = inRF_mm / (60.0 / (double)RFinterval_MIN);
                                }
                                CalRFintensity_mPsec(project.CVs[cvan], inRF_mm, rfIntervalSEC);
                                sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = 
                                    sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.WSCells[cx, ry].RFReadintensity_mPsec;
                                CalRFSumForWPUpWSWithRFGrid(cvan);
                            }
                        }
                    }

                }
                else if (eRainfallDataType == cRainfall.RainfallDataType.TextFileMAP)
                {
                    double inRF_mm;
                    double v = 0;
                    if (double.TryParse(rfRow.Rainfall, out v) == true)
                    {
                        inRF_mm = v;
                    }
                    else
                    {
                        System.Console.WriteLine("Error: Can not read rainfall value!!" + "\r\n" + "Order = " + nowRFOrder.ToString());
                        return;
                    }
                    if (inRF_mm < 0) { inRF_mm = 0; }
                    for (int cvan = 0; cvan < project.CVCount; cvan++)
                    {
                        CalRFintensity_mPsec(project.CVs[cvan], inRF_mm, rfIntervalSEC);
                        sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs + project.CVs[cvan].RFReadintensity_mPsec;
                    }
                    CalRFSumForWPUpWSWithMAPValue(project.CVs[0].RFReadintensity_mPsec); // 모든 격자의 강우량 동일하므로.. 하나를 던저준다.
                }
                else
                {
                    System.Console.WriteLine("Error: Rainfall data type is invalid.");
                }
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("An error was occurred while reading rainfall data.");
                Console.WriteLine(ex.ToString());
                sThisSimulation.mGRMSetupIsNormal = false;
                return;
            }
        }

        /// <summary>
        ///   강우에서 차단만 고려된 강우강도와 dt 시간동안의 강우량 계산. 2008.02.28 아직 차단 모듈은 반영되지 않음. 
        ///   침투량이 고려된 것은 유효 강우량으로 입려됨.
        ///   </summary>
        ///   <param name="cv"></param>
        ///   <param name="rf_mm"></param>
        ///   <param name="rfIntevalSEC"></param>
        ///   <remarks></remarks>
        public static void CalRFintensity_mPsec(cCVAttribute cv, double rf_mm, int rfIntevalSEC)
        {
            {
                if (rf_mm <= 0)
                { cv.RFReadintensity_mPsec = 0.0; }
                else
                { cv.RFReadintensity_mPsec = rf_mm /1000.0 / (double)rfIntevalSEC; }
            }
        }

        public static void CalRF_mPdt(cCVAttribute cv, int dtsec, double cellSize)
        {
            double rf_mPs = cv.RFReadintensity_mPsec;
            if (rf_mPs == 0)
            { cv.RFApp_dt_meter = 0; }
            else
            { cv.RFApp_dt_meter = rf_mPs * dtsec * (cellSize / cv.CVDeltaX_m); }
        }


        public static void CalRFSumForWPUpWSWithRFGrid(int cvan)
        {
            foreach (int wpCVid in cProject.Current.CVs[cvan].DownStreamWPCVids)
            {
                    cProject prj = cProject.Current;
                    prj.watchPoint.RFReadIntensitySumUpWs_mPs[wpCVid] = prj.watchPoint.RFReadIntensitySumUpWs_mPs[wpCVid] + prj.CVs[cvan].RFReadintensity_mPsec;
            }
        }

        public static void CalRFSumForWPUpWSWithMAPValue(double ConstRFintensity_mPs)
        {
            foreach (int wpCVid in cProject.Current.watchPoint.WPCVidList)
            {
                cProject.Current.watchPoint.RFReadIntensitySumUpWs_mPs[wpCVid] = ConstRFintensity_mPs * (cProject.Current.CVs[wpCVid - 1].FAc + (1 - cProject.Current.FacMin));
            }
        }

        /// <summary>
        ///   모든 검사체적의 강우량을 0으로 설정
        ///   </summary>
        ///   <remarks>강우 지속시간이 지난 시점 부터는 강우강도와 dt 동안의 강우량은 0으로 세팅한다.</remarks>
        public static void SetRainfallintensity_mPsec_And_Rainfall_dt_meter_Zero(cProject project)
        {
            int intCVTotNumber = project.CVCount;
            int intWPTotCount = project.watchPoint.WPCount;
            for (int cvan = 0; cvan < intCVTotNumber ; cvan++)
            {
                project.CVs[cvan].RFReadintensity_mPsec = 0;
                project.CVs[cvan].RFApp_dt_meter = 0;
            }
            sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs = 0;
            foreach (int wpcvid in project.watchPoint.WPCVidList)
            {
                project.watchPoint.RFWPGridForDtPrintout_mm[wpcvid] = 0;
                project.watchPoint.RFUpWsMeanForDt_mm[wpcvid] = 0;
                project.watchPoint.RFReadIntensitySumUpWs_mPs[wpcvid] = 0;
                project.watchPoint.RFUpWsMeanForDtPrintout_mm[wpcvid] = 0;
            }
        }

        public static void CalCumulativeRFDuringDTPrintOut(cProject project, int dtsec)
        {
            sThisSimulation.mRFMeanForDT_m = (sThisSimulation.mRFIntensitySumForAllCellsInCurrentRFData_mPs * dtsec) /cProject.Current.CVCount;
            sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m = sThisSimulation.mRFMeanForAllCell_sumForDTprintOut_m + sThisSimulation.mRFMeanForDT_m;
            foreach (int wpcvid in project.watchPoint.WPCVidList)
            {
                int fac = project.CVs[wpcvid - 1].FAc;
                project.watchPoint.RFUpWsMeanForDt_mm[wpcvid] = project.watchPoint.RFReadIntensitySumUpWs_mPs[wpcvid] * dtsec * 1000 / (double)(fac + 1);
                project.watchPoint.RFUpWsMeanForDtPrintout_mm[wpcvid] = project.watchPoint.RFUpWsMeanForDtPrintout_mm[wpcvid] + project.watchPoint.RFUpWsMeanForDt_mm[wpcvid];
                project.watchPoint.RFWPGridForDtPrintout_mm[wpcvid] = project.watchPoint.RFWPGridForDtPrintout_mm[wpcvid]
                                    + cProject.Current.CVs[wpcvid - 1].RFReadintensity_mPsec * 1000 * dtsec;
            }
            if (project.generalSimulEnv.mbCreateASCFile == true || project.generalSimulEnv.mbCreateImageFile == true)
            {
                if (project.generalSimulEnv.mbShowRFdistribution == true || project.generalSimulEnv.mbShowRFaccDistribution == true)
                {
                    for (int cvan = 0; cvan < project.CVCount; cvan++)
                    {
                        project.CVs[cvan].RF_dtPrintOut_meter = project.CVs[cvan].RF_dtPrintOut_meter
                                           + project.CVs[cvan].RFReadintensity_mPsec * dtsec;
                        project.CVs[cvan].RFAcc_FromStartToNow_meter = project.CVs[cvan].RFAcc_FromStartToNow_meter
                             + project.CVs[cvan].RFReadintensity_mPsec * dtsec;
                    }
                }
            }
        }


        /// <summary>
        ///   강우의 지속기간[minute]
        ///   </summary>
        ///   <value></value>
        ///   <returns></returns>
        ///   <remarks></remarks>
        public int RFDuration
        {
            get
            {
                if (IsSet==true)
                    return mRainfallinterval.Value * mlstRainfallData.Count;
                else
                    throw new InvalidOperationException();
            }
        }

        public bool IsSet
        {
            get
            {
                return mRainfallDataType.HasValue;
            }
        }


        public string RFDataPathFirst
        {
            get
            {
                if (mlstRainfallData.Count > 0)
                    return mlstRainfallData[0].FilePath;
                else
                    return null;
            }
        }

        public string RFDataFilePathName
        {
            get
            {
                return mRainfallDataFilePathName;
            }
        }

        public RainfallData GetRFdataByOrder(List<RainfallData> dtRFinfo, int order)
        {
            if (dtRFinfo.Count == 0)
            {
                System.Console.WriteLine("Error: Rainfall data is not exist.   " + "\r\n" + "Order = " + order.ToString());
                return default(RainfallData);
            }
            for (int n = 0; n < dtRFinfo.Count; n++)
            {
                if (dtRFinfo[n].Order == order)
                {
                    return dtRFinfo[n];
                }
            }
            return default(RainfallData);
        }

        public int RFIntervalSEC
        {
            get
            {
                return System.Convert.ToInt32(mRainfallinterval * 60);
            }
        }
    }
}
