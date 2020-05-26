using System;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using gentle;
using System.IO;


namespace GRMCore
{
    public class cFlowControl : ICloneable
    {
        public enum FCDataSourceType
        {
            UserEdit,
            Constant,
            TextFile,
            ReservoirOperation
        } 

        public enum FlowControlType
        {
            ReservoirOutflow, // 상류모의, 저류량 고려하지 않고, 댐에서의 방류량만 고려함
            Inlet,  // 상류 모의하지 않는것. 저류량 고려하지 않고, inlet grid에서의 outfow 만 고려함.
            SinkFlow, // 상류모의, 입력된 sink flow data 고려함. 저수지 고려안함.
            SourceFlow, // 상류모의, 입력된 source flow data 고려함. 저수지 고려안함.
            ReservoirOperation, // 상류모의, 저수지 고려, 방류량은 operation rule에 의해서 결정됨. 사용자 입력 인터페이스 구현하지 않음.
            // 저류량-방류량, 유입량-방류량 관계식을 이용해서 소스코드에 반영 가능
            NONE
        }

        public enum ROType
        {
            AutoROM,
            RigidROM,
            ConstantQ,
            SDEqation
        }

        public struct FCData
        {
            public int CVID;
            public int Order;
            public double Value;
            public string DataTime;
        }

        //public static readonly Color DEFAULT_FLOW_CONTROL_COLOR = Color.Maroon;
        private cFVMSolver mFVMsolver = new cFVMSolver();

        public Dataset.GRMProject.FlowControlGridDataTable mdtFCGridInfo;
        public DataTable mdtFCFlowData;
        //public Color mCellColor;

        /// <summary>
        /// 현재의 모델링 시간(t)에 적용된 flow control data 값
        /// </summary>
        /// <remarks></remarks>
        public SortedList<int, double> mFCdataToApplyNowT;
        private bool mInletExisted = false;
        private List<int> mInletList;

        /// <summary>
        /// 현재 계산 시간에서 적용된 fc 자료
        /// </summary>
        /// <remarks></remarks>
        private List<int> mFCGridCVidList;


        public int FCCellCount
        {
            get
            {
                return mdtFCGridInfo.Rows.Count;
            }
        }


        public void GetValues(cProject prj)
        {
            ColorConverter cc = new ColorConverter();
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prj.PrjFile.ProjectSettings.Rows[0];
            mdtFCGridInfo = new Dataset.GRMProject.FlowControlGridDataTable();
            mdtFCGridInfo = prj.PrjFile.FlowControlGrid;
            if (prj.mSimulationType == cGRM.SimulationType.SingleEvent)
            {
                mdtFCFlowData = new DataTable(); // 원 :2017.6.20 부터 사용
                mdtFCFlowData.Columns.Add(new global::System.Data.DataColumn("CVID", typeof(int), null, global::System.Data.MappingType.Element));
                mdtFCFlowData.Columns.Add(new global::System.Data.DataColumn("Value", typeof(double), null, global::System.Data.MappingType.Element));
                mdtFCFlowData.Columns.Add(new global::System.Data.DataColumn("DataTime", typeof(string), null, global::System.Data.MappingType.Element));
            }
        }


        public bool IsSet
        {
            get
            {
                if (mdtFCGridInfo == null)
                { return false; }
                if (mdtFCGridInfo.Rows.Count > 0)
                { return true; }
                return false;
            }
        }

        /// <summary>
        /// Flow control 설정 대상 격자의 CVID 정보를 업데이트 한다.
        /// </summary>
        /// <remarks></remarks>
        public void UpdateFCGridInfoAndData(cProject prj)
        {
            mInletList = new List<int>();
            mFCGridCVidList = new List<int>();
            mFCdataToApplyNowT = new SortedList<int, double>();
            foreach (Dataset.GRMProject.FlowControlGridRow r in mdtFCGridInfo)
            {
                r.CVID = prj.WSCells[r.ColX, r.RowY].CVID;
                mFCGridCVidList.Add(r.CVID);
                if (r.ControlType.ToString() == FlowControlType.Inlet.ToString())
                { mInletList.Add(r.CVID); }
                mFCdataToApplyNowT.Add(r.CVID, 0);
                if (prj.mSimulationType == cGRM.SimulationType.SingleEvent)
                {
                    if (r.ControlType != cFlowControl.FlowControlType.ReservoirOperation.ToString())
                    {
                        if (File.Exists(r.FlowDataFile))
                        {
                            string[] Lines = System.IO.File.ReadAllLines(r.FlowDataFile);
                            for (int n = 0; n < Lines.Length; n++)
                            {
                                DataRow ar = prj.fcGrid.mdtFCFlowData.NewRow();
                                ar["CVID"] = r.CVID;
                                ar["Value"] = System.Convert.ToDouble(Lines[n]);
                                if (prj.generalSimulEnv.mIsDateTimeFormat == true)
                                    ar["DataTime"] = cComTools.GetTimeToPrintOut(true, prj.generalSimulEnv.mSimStartDateTime, System.Convert.ToInt32(r.DT) * n);
                                else
                                    ar["DataTime"] = System.Convert.ToString(System.Convert.ToInt32(r.DT) * n);
                                mdtFCFlowData.Rows.Add(ar);
                            }
                        }
                    }
                }
            }
            if (mInletList.Count > 0)
            { mInletExisted = true; }
        }


        public bool InletExisted
        {
            get
            {
                return mInletExisted;
            }
        }

        /// <summary>
        /// Flow control 대상 격자 중 inlet 격자의 CVID 목록
        /// </summary>
        /// <value></value>
        /// <returns></returns>
        /// <remarks></remarks>
        public List<int> InletCVidList
        {
            get
            {
                return mInletList;
            }
        }

        /// <summary>
        /// Flow control 대상 격자의 CVID 목록
        /// </summary>
        /// <value></value>
        /// <returns></returns>
        /// <remarks></remarks>
        public List<int> FCGridCVidList
        {
            get
            {
                return mFCGridCVidList;
            }
        }

        public string GetFCName(int cvid)
        {
            foreach (Dataset.GRMProject.FlowControlGridRow r in mdtFCGridInfo.Rows)
            {
                if (r.CVID == cvid)
                { return r.Name; }
            }
            return "";
        }

        public void ConvertFCTypeToAutoROM(int cvid)//2019.10.01. 최. prediction 관련
        {
            DataRow[] drs = mdtFCGridInfo.Select(string.Format("CVID = {0}", cvid));
            Dataset.GRMProject.FlowControlGridRow r = (Dataset.GRMProject.FlowControlGridRow)drs[0];
            r.ControlType = cFlowControl.FlowControlType.ReservoirOperation.ToString();
            r.ROType = cFlowControl.ROType.AutoROM.ToString();
        }

        /// <summary>
        /// 계산하면서 업데이트 되는 자료만 백업한다.
        /// </summary>
        /// <returns></returns>
        /// <remarks></remarks>
        public object Clone()
        {
            cFlowControl cln = new cFlowControl();
            cln.mdtFCGridInfo = new Dataset.GRMProject.FlowControlGridDataTable();
            if (mdtFCGridInfo != null)
            {
                cln.mdtFCFlowData = new DataTable();
                cln.mFCdataToApplyNowT = new SortedList<int, double>();
                cln.mdtFCGridInfo = (Dataset.GRMProject.FlowControlGridDataTable)mdtFCGridInfo.Clone();
                if (mdtFCFlowData != null)
                { cln.mdtFCFlowData = (DataTable)mdtFCFlowData.Clone(); }
                if (this.mFCdataToApplyNowT != null)
                {
                    foreach (int k in this.mFCdataToApplyNowT.Keys)
                        cln.mFCdataToApplyNowT.Add(k, mFCdataToApplyNowT[k]);
                }
            }
            return cln;
        }


        public static void CalFCReservoirOperation(cProject project, int cvan, int nowT_MIN)
        {
            int dtsec = sThisSimulation.dtsec;
            cCVAttribute cv = project.CVs[cvan];
            int fcCVid = cv.CVID;
            DataRow[] Rows = project.fcGrid.mdtFCGridInfo.Select(string.Format("cvid = {0}", fcCVid));
            Dataset.GRMProject.FlowControlGridRow fcRow = (Dataset.GRMProject.FlowControlGridRow)Rows[0];
            int rowOrder;
            double QforSinkOrSourceFlow = 0;
            double cellsize = project.watershed.mCellSize;

            if (cv.FCType == cFlowControl.FlowControlType.SinkFlow || cv.FCType == cFlowControl.FlowControlType.SourceFlow)
            {
                rowOrder = System.Convert.ToInt32((nowT_MIN - 1) / System.Convert.ToInt32(fcRow.DT));
                DataView dv;
                dv = new DataView(project.fcGrid.mdtFCFlowData, string.Format("cvid={0}", fcCVid), " datetime asc", DataViewRowState.CurrentRows);
                DataTable dt = dv.Table;
                DataTable fcdt = (DataTable)dt;
                QforSinkOrSourceFlow = fcdt.Rows[rowOrder].Field<double>("Value");
            }
            switch (cv.FCType)
            {
                case cFlowControl.FlowControlType.SinkFlow:
                    {
                        cv.StorageCumulative_m3 = cv.StorageCumulative_m3 + cv.StorageAddedForDTfromRF_m3 + cv.QsumCVw_dt_m3 - QforSinkOrSourceFlow;
                        break;
                    }
                case cFlowControl.FlowControlType.SourceFlow:
                    {
                        cv.StorageCumulative_m3 = cv.StorageCumulative_m3 + cv.StorageAddedForDTfromRF_m3 + cv.QsumCVw_dt_m3 + QforSinkOrSourceFlow;
                        break;
                    }
                default:
                    {
                        cv.StorageCumulative_m3 = cv.StorageCumulative_m3 + cv.StorageAddedForDTfromRF_m3 + cv.QsumCVw_dt_m3;
                        break;
                    }
            }
            switch (fcRow.ROType.ToString())
            {
                case nameof(cFlowControl.ROType.AutoROM):
                    {
                        ApplyReservoirAutoROM(cv, cellsize, System.Convert.ToDouble(fcRow.MaxStorage) * System.Convert.ToDouble(fcRow.MaxStorageR));
                        break;
                    }
                case nameof(cFlowControl.ROType.RigidROM):
                    {
                        cv.StorageCumulative_m3 = cv.StorageCumulative_m3 - System.Convert.ToDouble(fcRow.ROConstQ) * dtsec;
                        ApplyReservoirRigidROM(cv, cellsize, System.Convert.ToDouble(fcRow.MaxStorage) * System.Convert.ToDouble(fcRow.MaxStorageR), System.Convert.ToDouble(fcRow.ROConstQ));
                        break;
                    }
                case nameof(cFlowControl.ROType.ConstantQ):
                    {
                        bool bOurflowDuration = false;
                        if (nowT_MIN <= System.Convert.ToDouble(fcRow.ROConstQDuration) * 60)
                        {
                            cv.StorageCumulative_m3 = cv.StorageCumulative_m3 - System.Convert.ToDouble(fcRow.ROConstQ) * dtsec;
                            bOurflowDuration = true;
                        }
                        ApplyReservoirConstantDischarge(cv, cellsize, System.Convert.ToDouble(fcRow.ROConstQ), 
                            System.Convert.ToDouble(fcRow.MaxStorage) * System.Convert.ToDouble(fcRow.MaxStorageR), bOurflowDuration);
                        break;
                    }
                case nameof(cFlowControl.ROType.SDEqation):
                    {
                        break;
                    }
            }
        }

        public static void ApplyReservoirAutoROM(cCVAttribute cv, double cellsize, double MaxStorageApp)
        {
            double sngQout_cms;
            double sngDY_m = cellsize;
            double sngQinput_m3 = cv.StorageAddedForDTfromRF_m3 + cv.QsumCVw_dt_m3;
            if (cv.StorageCumulative_m3 >= MaxStorageApp)
            {
                sngQout_cms = sngQinput_m3 / sThisSimulation.dtsec;  // 이때는 셀의 특성(크기, 경사, 조도 등...)에 상관없이 상류에서 유입되는 모든 양이 유출됨.
                cv.StorageCumulative_m3 = MaxStorageApp; // 누가저류량이 최대저류량 보다 같거나 크면, 더이상 누가되지 않고, 최대저류량을 유지한다.
            }
            else
            {
                sngQout_cms = 0;
            }
            CalReservoirOutFlowInReservoirOperation(cv, sngQout_cms, sngDY_m);
        }


        public static void ApplyReservoirRigidROM(cCVAttribute cv, double cellsize, double maxStorageApp, double RoQ_CONST_CMS)
        {
            double dy_m = cellsize;
            double sngQout_cms;
            double sngQinput_m3 = cv.StorageAddedForDTfromRF_m3 + cv.QsumCVw_dt_m3;
            if (cv.StorageCumulative_m3 >= maxStorageApp)
            {
                sngQout_cms = sngQinput_m3 / (double)sThisSimulation.dtsec;
                cv.StorageCumulative_m3 = maxStorageApp; // 누가저류량이 최대저류량 보다 같거나 크면, 더이상 누가되지 않고, 최대저류량을 유지한다.
            }
            else if (cv.StorageCumulative_m3 < RoQ_CONST_CMS * sThisSimulation.dtsec)
            {
                // 이경우는 주어진 유출량으로 유출하지 못하고, 그것보다 작은 양으로 유출된다는 의미
                // 즉 dt 시간에서 저류된 모든 양이 유출되는 유량으로 현재 저수지에서의 유출량을 계산해야 한다.
                // 이건 이번 저류량을 계산하기 전의 저류량
                double sngStorage_tM1 = cv.StorageCumulative_m3;
                if (sngStorage_tM1 < 0)
                { sngQout_cms = 0; }
                else
                { sngQout_cms = sngStorage_tM1 / (double)sThisSimulation.dtsec; }
                cv.StorageCumulative_m3 = 0;
            }
            else
            { sngQout_cms = RoQ_CONST_CMS; }
            CalReservoirOutFlowInReservoirOperation(cv, sngQout_cms, dy_m);
        }


        public static void ApplyReservoirConstantDischarge(cCVAttribute cv, double cellsize, 
            double RoQ_CONST_CMS, double maxStorageApp, bool bOutflowDuration)
        {
            double dy_m = cellsize;
            double sngQout_cms;
            if (bOutflowDuration == true)
            {
                if (cv.StorageCumulative_m3 <= 0)
                {
                    // 이경우는 주어진 유출량으로 유출하지 못하고, 그것보다 작은 양으로 유출된다는 의미
                    // 즉 dt 시간에서 저류된 모든 양이 유출되는 유량으로 현재 저수지에서의 유출량을 계산해야 한다.
                    double sngStorage_tM1 = (RoQ_CONST_CMS * sThisSimulation.dtsec) + cv.StorageCumulative_m3;
                    if (sngStorage_tM1 < 0)
                    { sngQout_cms = 0; }
                    else
                    { sngQout_cms = sngStorage_tM1 / (double)sThisSimulation.dtsec; }
                    cv.StorageCumulative_m3 = 0;
                }
                else
                {
                    sngQout_cms = RoQ_CONST_CMS;
                    cv.StorageCumulative_m3 = cv.StorageCumulative_m3 - RoQ_CONST_CMS;
                    if (cv.StorageCumulative_m3 >= maxStorageApp)
                    // Constant dischrage에서는 계속 누가 시킨다. 
                    // 누가저류량이 최대저류량 보다 같거나 크면, 더이상 누가되지 않고, 최대저류량을 유지한다.
                    { cv.StorageCumulative_m3 = maxStorageApp; }
                }
            }
            else
            { sngQout_cms = 0; }
            CalReservoirOutFlowInReservoirOperation(cv, sngQout_cms, dy_m);
        }


        private static void CalReservoirOutFlowInReservoirOperation(cCVAttribute cv, double sngQout_cms, double sngDY_m)
        {
            if (sngQout_cms > 0)
            {
                if (cv.FlowType == cGRM.CellFlowType.OverlandFlow)
                {
                    cv.QCVof_i_j_m3Ps = sngQout_cms;
                    cv.hCVof_i_j = cv.RoughnessCoeffOF * cv.QCVof_i_j_m3Ps / sngDY_m / Math.Pow(Math.Sqrt(cv.SlopeOF), 0.6);

                    cv.mStreamAttr.QCVch_i_j_m3Ps = 0;
                    cv.mStreamAttr.uCVch_i_j = 0;
                    cv.mStreamAttr.CSAch_i_j = 0;
                    cv.mStreamAttr.hCVch_i_j = 0;
                    cv.mStreamAttr.uCVch_i_j = 0;
                }
                else
                {
                    cv.QCVof_i_j_m3Ps = 0;
                    cv.hCVof_i_j = 0;
                    cv.mStreamAttr.QCVch_i_j_m3Ps = sngQout_cms;
                    cv.mStreamAttr.CSAch_i_j = cFVMSolver.CalChCSAFromQbyIteration(cv, cv.mStreamAttr.CSAch_i_j, cv.mStreamAttr.QCVch_i_j_m3Ps);
                    cv.mStreamAttr.uCVch_i_j = cv.mStreamAttr.QCVch_i_j_m3Ps / cv.mStreamAttr.CSAch_i_j;
                    cv.mStreamAttr.hCVch_i_j = cFVMSolver.GetChannelDepthUsingArea(cv.mStreamAttr.ChBaseWidth, cv.mStreamAttr.CSAch_i_j,
                        cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chUpperRBaseWidth_m, cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chLowerRHeight,
                        cv.mStreamAttr.mChBankCoeff);
                }
            }
            else
            {
                cv.hCVof_i_j = 0;
                cv.CSAof_i_j = 0;
                cv.QCVof_i_j_m3Ps = 0;
                cv.mStreamAttr.uCVch_i_j = 0;
                cv.mStreamAttr.CSAch_i_j = 0;
                cv.mStreamAttr.hCVch_i_j = 0;
                cv.mStreamAttr.uCVch_i_j = 0;
                cv.mStreamAttr.QCVch_i_j_m3Ps = 0;
            }
        }


        public static void CalFCReservoirOutFlow(cProject project, int nowT_MIN, int cvan)
        {
            int fcCVid = project.CVs[cvan].CVID;
            DataRow[] Rows = project.fcGrid.mdtFCGridInfo.Select(string.Format("cvid = {0}", fcCVid));
            Dataset.GRMProject.FlowControlGridRow fcRow = (Dataset.GRMProject.FlowControlGridRow)Rows[0];
            // 60 /60의 경우에는 intFCDataArrayNum=0을 써야 하지만 나눈 몫이 1이므로, 배열 번호가 넘어가게 된다.
            // 따라서, 59/60 을 만들기 위해서 (intNowTimeMin - 1)으로 한다.
            int rowOrder = System.Convert.ToInt32((nowT_MIN - 1) / System.Convert.ToInt32(fcRow.DT));
            DataRow[] fcDataRows = project.fcGrid.mdtFCFlowData.Select(string.Format("CVID = {0}", fcCVid));
            //if (rowOrder >= fcDataRows.Length) { rowOrder = fcDataRows.Length - 1; } // 이렇게 하면 마지막 데이터가 계속 사용

            cCVAttribute cv = project.CVs[cvan];
            if (cv.FlowType == cGRM.CellFlowType.OverlandFlow)
            { System.Console.WriteLine("ERROR: Reservoir outflow is simulated only in channel flow!!!"); }
            else
            {
                if (rowOrder < fcDataRows.Length) // 이렇게 하면 마지막 데이터 이후에는 모두 0 으로 적용됨
                {
                    double v = 0;
                    //if (double.TryParse(fcDataRows[rowOrder].Field<double>("value").ToString(), out v)  == false)     //2020.1.13 원 : DB 에서 data type이 조정 된 것 같다. 그래서. 아래와 같이 수정함.
                    if (double.TryParse(fcDataRows[rowOrder]["value"].ToString(), out v) == false)
                    //if (fcDataRows[rowOrder].Field<string>("value") == "-")
                    { cv.mStreamAttr.QCVch_i_j_m3Ps = 0; }
                    else
                    {
                        //2020.1.13 원 : DB 에서 data type이 조정 된 것 같다. 그래서. 아래와 같이 수정함.
                        double.TryParse(fcDataRows[rowOrder]["value"].ToString(), out cv.mStreamAttr.QCVch_i_j_m3Ps);
                    }
                }
                else
                {
                    cv.mStreamAttr.QCVch_i_j_m3Ps = 0;
                }

                cv.mStreamAttr.CSAch_i_j = cFVMSolver.CalChCSAFromQbyIteration(project.CVs[cvan], cv.mStreamAttr.CSAch_i_j, cv.mStreamAttr.QCVch_i_j_m3Ps);
                // Dim csa1 As Single = .mStreamAttr.CSAch_i_j
                // Dim chCSAini2 As Single = mFVMsolver.CalChCSAFromManningEQ(project.CVs[cvan], .mStreamAttr.CSAch_i_j, .mStreamAttr.QCVch_i_j_m3Ps)
                cv.mStreamAttr.hCVch_i_j = cFVMSolver.GetChannelDepthUsingArea(cv.mStreamAttr.ChBaseWidth, 
                    cv.mStreamAttr.CSAch_i_j, cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chUpperRBaseWidth_m, 
                    cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chLowerRHeight, cv.mStreamAttr.mChBankCoeff);
                cv.mStreamAttr.uCVch_i_j = cv.mStreamAttr.QCVch_i_j_m3Ps / (double)cv.mStreamAttr.CSAch_i_j;
            }
            project.fcGrid.mFCdataToApplyNowT[fcCVid] = cv.mStreamAttr.QCVch_i_j_m3Ps;   // 2018.9.3 원 : 다시 fcDataRows(rowOrder).Item("value") 가져오는거 보다 낳다고 판단했슴 
        }


        public static void CalFCSinkOrSourceFlow(cProject project, int nowT_MIN, int cvan)
        {
            double cellsize = project.watershed.mCellSize;
            int fcCVid = project.CVs[cvan].CVID;
            DataRow[] Rows = project.fcGrid.mdtFCGridInfo.Select(string.Format("cvid = {0}", fcCVid));
            Dataset.GRMProject.FlowControlGridRow fcRow = (Dataset.GRMProject.FlowControlGridRow)Rows[0];
            int rowOrder = System.Convert.ToInt32((nowT_MIN - 1) / System.Convert.ToInt32(fcRow.DT));
            DataView dv;
            dv = new DataView(project.fcGrid.mdtFCFlowData, string.Format("cvid={0}", fcCVid), " datatime asc", 
                DataViewRowState.CurrentRows);
            DataTable dt = dv.Table;
            DataTable fcdt = (DataTable)dt;
            if (rowOrder < fcdt.Rows.Count)
            {
                double QtoApp = fcdt.Rows[rowOrder].Field<double>("value");
                cCVAttribute cv = project.CVs[cvan];
                if (cv.FlowType == cGRM.CellFlowType.OverlandFlow)
                {
                    switch (cv.FCType)
                    {
                        case cFlowControl.FlowControlType.SinkFlow:
                            {
                                cv.QCVof_i_j_m3Ps = cv.QCVof_i_j_m3Ps - QtoApp;
                                break;
                            }

                        case cFlowControl.FlowControlType.SourceFlow:
                            {
                                cv.QCVof_i_j_m3Ps = cv.QCVof_i_j_m3Ps + QtoApp;
                                break;
                            }
                    }
                    if (cv.QCVof_i_j_m3Ps < 0)
                    { cv.QCVof_i_j_m3Ps = 0; }
                    cv.hCVof_i_j = Math.Pow(cv.RoughnessCoeffOF * cv.QCVof_i_j_m3Ps / cellsize 
                        / Math.Pow(cv.SlopeOF, 0.5), 0.6);
                    cv.CSAof_i_j = cv.hCVof_i_j * cellsize;
                    cv.mStreamAttr.uCVch_i_j = cv.QCVof_i_j_m3Ps / (double)cv.CSAof_i_j;
                }
                else
                {
                    switch (cv.FCType)
                    {
                        case cFlowControl.FlowControlType.SinkFlow:
                            {
                                cv.mStreamAttr.QCVch_i_j_m3Ps = cv.mStreamAttr.QCVch_i_j_m3Ps - QtoApp;
                                break;
                            }

                        case cFlowControl.FlowControlType.SourceFlow:
                            {
                                cv.mStreamAttr.QCVch_i_j_m3Ps = cv.mStreamAttr.QCVch_i_j_m3Ps + QtoApp;
                                break;
                            }
                    }
                    if (cv.mStreamAttr.QCVch_i_j_m3Ps < 0)
                    { cv.mStreamAttr.QCVch_i_j_m3Ps = 0; }
                    cv.mStreamAttr.CSAch_i_j = cFVMSolver.CalChCSAFromQbyIteration(cv, cv.mStreamAttr.CSAch_i_j, cv.mStreamAttr.QCVch_i_j_m3Ps);
                    cv.mStreamAttr.hCVch_i_j = cFVMSolver.GetChannelDepthUsingArea(cv.mStreamAttr.ChBaseWidth, 
                        cv.mStreamAttr.CSAch_i_j, cv.mStreamAttr.chIsCompoundCS, cv.mStreamAttr.chUpperRBaseWidth_m, 
                        cv.mStreamAttr.chLowerRArea_m2, cv.mStreamAttr.chLowerRHeight, cv.mStreamAttr.mChBankCoeff);
                    cv.mStreamAttr.uCVch_i_j = cv.mStreamAttr.QCVch_i_j_m3Ps / (double)cv.mStreamAttr.CSAch_i_j;
                }
                project.fcGrid.mFCdataToApplyNowT[fcCVid] = System.Convert.ToDouble(QtoApp);
            }
            else
            { project.fcGrid.mFCdataToApplyNowT[fcCVid] = 0; }
        }
    }
}
