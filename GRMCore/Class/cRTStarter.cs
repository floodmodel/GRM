using System;
using System.Data;
using System.IO;
using System.Data.SqlClient;

namespace GRMCore
{
    public class cRTStarter
    {
        public event RTStatusEventHandler RTStatus;
        private cRealTime GRMRT = new cRealTime();
        public delegate void RTStatusEventHandler(string strMSG);

        private cProject RTProject;
        private const float CONST_PIC_WIDTH = 1024;
        private const float CONST_PIC_HEIGHT = 768;
        private string mFPN_RTEnv;
        private string mProjectFPN;
        private string mFP_RFLayerFP;
        // Private mFPN_FCData As String '현재 유역에서 읽을 fc data 파일. db 매개로 수정하였으므로 필요없음.
        private string mSimulationStartingTime;
        private cRainfall.RainfallDataType mRainfallDataType;
        private bool mbDWS_EXIST;
        private bool mbIsFC;
        private int mCWCellColXToConnectDW;
        private int mCWCellRowYToConnectDW;
        private int mDWCellColXToConnectCW;
        private int mDWCellRowYToConnectCW;
        // Private mFPN_DWSS_FCDATA As String  '하류 유역의 fc data로 사용할 파일. db 매개로 수정하였으므로 필요없음.
        private int mRFInterval_MIN;
        private int mOutPutInterval_MIN;

        /// <summary>
        /// 외부 코드에서 ref 파일만 가지고 RT 실행시킬때 이거 호출하면 됨
        /// </summary>
        /// <param name="fpn_REF"></param>
        /// <param name="dtStart">런타임 확인을 위해 사용되는 모델링 시작 시간</param>
        /// <param name="RTStartDateTime">GUI 등으로 부터 모의기간 시작시간 설정된 것을 받을때 사용</param>
        /// <remarks></remarks>
        public cRTStarter(string fpn_REF, string strGUID, DateTime dtStart, string RTStartDateTime = "")
        {
            mFPN_RTEnv = fpn_REF;
            UpdateRTVariablesUsingEnvFile(mFPN_RTEnv, RTStartDateTime); // 여기서 파일로 설정
            cRealTime_Common.g_performance_log_GUID = strGUID;
            cRealTime_Common.g_dtStart_from_MonitorEXE = dtStart;
        }

        private bool UpdateRTVariablesUsingEnvFile(string rtEnvFPN, string RTStartDateTime = "")
        {
            Dataset.GRMProject.RTenvDataTable dtEnv = new Dataset.GRMProject.RTenvDataTable();
            dtEnv.ReadXml(rtEnvFPN);
            Dataset.GRMProject.RTenvRow r = (Dataset.GRMProject.RTenvRow)dtEnv.Rows[0];
            mProjectFPN = r.ProjectFPN;
            mFP_RFLayerFP = r.RTRFfolderName;
            if (r.IsFC == true)
                mbIsFC = true;
            else
                // mFPN_FCData = ""
                mbIsFC = false;
            mbDWS_EXIST = r.IsDWSExist;
            if (mbDWS_EXIST == true)
            {
                mCWCellColXToConnectDW = r.CWCellColXToConnectDW;
                mCWCellRowYToConnectDW = r.CWCellRowYToConnectDW;
                mDWCellColXToConnectCW = r.DWCellColXToConnectCW;
                mDWCellRowYToConnectCW = r.DWCellRowYToConnectCW;
            }
            mRFInterval_MIN = r.RFInterval_min;
            mOutPutInterval_MIN = r.OutputInterval_min;
            if (RTStartDateTime == "")
            { mSimulationStartingTime = r.RTstartingTime; }
            else
            { mSimulationStartingTime = RTStartDateTime; }
            return true;
        }


        public void SetUpAndStartGRMRT()
        {
            SetupRT();
            UpateGRMGeneralParsByUserSettiing();
            GRMRT.RunGRMRT();
        }


        public void SetupRT()
        {
            GRMRT = null;
            RTProject = null;
            cRealTime.InitializeGRMRT();
            GRMRT = cRealTime.Current;
            GRMRT.SetupGRM(mProjectFPN);
            RTProject = cProject.Current;

            if (true)
            {
                System.Data.SqlClient.SqlConnection oSQLCon = new System.Data.SqlClient.SqlConnection(cRealTime_DBMS.g_strDBMSCnn);
                if (oSQLCon.State == ConnectionState.Closed)
                    oSQLCon.Open();

                // Dim x As New Xml.Serialization.XmlSerializer(RTProject.GetType)
                // The Serialize method Is used To serialize an Object To XML. Serialize Is overloaded And can send output To a TextWriter, Stream, Or XmlWriter Object. In this example, you send the output to the console
                // x.Serialize(Console.Out, RTProject)
                // x.Serialize()

                // monitor에서 id 를 넘겨줄 필요 없다.
                string strGMPALL = File.ReadAllText(mProjectFPN);

                // Dim strSql As String = String.Format("insert into runmeta ([runmeta],[who],gmp) values('{0}','{1}')", "누가 언제 어디서 어떻게 run 했는지..", "user1")
                // Dim oSQLCMD As New SqlClient.SqlCommand(strSql, oSQLCon)

                SqlCommand oSQLCMD = new SqlCommand("INSERT INTO runmeta (runmeta, [who], gmp,run_starttime) VALUES (@runmeta, @who, @gmp,@run_starttime)", oSQLCon);
                oSQLCMD.Parameters.AddWithValue("@runmeta", "누가 언제 어디서 어떻게 run 했는지..");
                oSQLCMD.Parameters.AddWithValue("@who", "user1");
                oSQLCMD.Parameters.AddWithValue("@gmp", strGMPALL);
                oSQLCMD.Parameters.AddWithValue("@run_starttime", mSimulationStartingTime);


                int intRetVal = oSQLCMD.ExecuteNonQuery();

                if (intRetVal == 1)
                {
                    DataTable dt1 = new DataTable();
                    string strSQL1 = "select top 1 runid from runmeta order by runid desc";
                    SqlDataAdapter oSqlDataAdapter = new SqlDataAdapter(strSQL1, cRealTime_DBMS.g_strDBMSCnn);
                    oSqlDataAdapter.Fill(dt1);
                    long lngID= (long)dt1.Rows[0][0];
                    int intID = Convert.ToInt32(lngID); //int intID = dt1.Rows[0].Field<int>(0);    // by ice. 2018.11.21

                    if (cRealTime_Common.g_performance_log_GUID == intID.ToString())
                    {
                    }
                    else
                        Console.WriteLine("Warning : If g_performance_log_GUID <> intID.ToString() Then");

                    cRealTime_DBMS.g_RunID = intID; // monitor 에서 받은거 보다. DB 우선 
                    oSqlDataAdapter.Dispose();
                }
                else
                    Console.WriteLine("runmeta logging error");
                oSQLCon.Close();
            }
        }


        /// <summary>
        /// 여기서 사용자 설정 환경 매개변수로 업데이트
        /// </summary>
        /// <remarks></remarks>
        private void UpateGRMGeneralParsByUserSettiing()
        {
            RTProject.rainfall.mRainfallinterval = mRFInterval_MIN;
            RTProject.generalSimulEnv.mIsDateTimeFormat = true;
            RTProject.mSimulationType = cGRM.SimulationType.RealTime;
            GRMRT.mDtPrintOutRT_min = mOutPutInterval_MIN;
            GRMRT.mRFStartDateTimeRT = mSimulationStartingTime;
            GRMRT.mRainfallDataTypeRT = cRainfall.RainfallDataType.TextFileASCgrid;
            GRMRT.mRfFilePathRT = mFP_RFLayerFP;
            GRMRT.mPicWidth = CONST_PIC_WIDTH;
            GRMRT.mPicHeight = CONST_PIC_HEIGHT;
            GRMRT.mbIsDWSS = mbDWS_EXIST;
            if (GRMRT.mbIsDWSS == true)
            {
                GRMRT.mCWCellColX_ToConnectDW = mCWCellColXToConnectDW;
                GRMRT.mCWCellRowY_ToConnectDW = mCWCellRowYToConnectDW;
                GRMRT.mDWCellColX_ToConnectCW = mDWCellColXToConnectCW;
                GRMRT.mDWCellRowY_ToConnectCW = mDWCellRowYToConnectCW;
            }
            RTProject.generalSimulEnv.mSimStartDateTime = DateTime.ParseExact(GRMRT.mRFStartDateTimeRT, "yyyyMMddHHmm", System.Globalization.CultureInfo.InvariantCulture).ToString("yyyy-MM-dd HH:mm"); // 2018.8.11 원 : 중요 한 추가. 이 조치 안하면 0이고,  이후 포화도 등 png img 제작 등에서 time tag 계산에서 오류...
        }

        /// <summary>
        /// 유역 매개변수 업데이트는 여기서 수행..
        /// </summary>
        /// <param name="wsid"></param>
        /// <param name="iniSat"></param>
        /// <param name="minSlopeLandSurface"></param>
        /// <param name="minSlopeChannel"></param>
        /// <param name="minChannelBaseWidth"></param>
        /// <param name="roughnessChannel"></param>
        /// <param name="dryStreamOrder"></param>
        /// <param name="ccLCRoughness"></param>
        /// <param name="ccSoilDepth"></param>
        /// <param name="ccPorosity"></param>
        /// <param name="ccWFSuctionHead"></param>
        /// <param name="ccSoilHydraulicCond"></param>
        /// <param name="iniFlow"></param>
        /// <returns></returns>
        public bool UpdateWSPars(int wsid, float iniSat, float minSlopeLandSurface, string UnsKType, float coefUnsK, float minSlopeChannel, float minChannelBaseWidth, float roughnessChannel, int dryStreamOrder, float ccLCRoughness, float ccSoilDepth, float ccPorosity, float ccWFSuctionHead, float ccSoilHydraulicCond, float iniFlow = 0)
        {
            RTProject.subWSPar.userPars[wsid].iniSaturation = iniSat;
            RTProject.subWSPar.userPars[wsid].minSlopeOF = minSlopeLandSurface;
            RTProject.subWSPar.userPars[wsid].UKType = UnsKType;
            RTProject.subWSPar.userPars[wsid].coefUK = coefUnsK;
            RTProject.subWSPar.userPars[wsid].minSlopeChBed = minSlopeChannel;
            RTProject.subWSPar.userPars[wsid].minChBaseWidth = minChannelBaseWidth;
            RTProject.subWSPar.userPars[wsid].chRoughness = roughnessChannel;
            RTProject.subWSPar.userPars[wsid].dryStreamOrder = dryStreamOrder;
            RTProject.subWSPar.userPars[wsid].ccLCRoughness = ccLCRoughness;
            RTProject.subWSPar.userPars[wsid].ccSoilDepth = ccSoilDepth;
            RTProject.subWSPar.userPars[wsid].ccPorosity = ccPorosity;
            RTProject.subWSPar.userPars[wsid].ccWFSuctionHead = ccWFSuctionHead;
            RTProject.subWSPar.userPars[wsid].ccHydraulicK = ccSoilHydraulicCond;
            RTProject.subWSPar.userPars[wsid].iniFlow = iniFlow;
            RTProject.subWSPar.userPars[wsid].isUserSet = true;
            cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(RTProject);
            return true;
        }

        /// <summary>
        /// GRM 프로젝트 파일 저장
        /// </summary>
        /// <remarks></remarks>
        public void SaveParsToProjectFile()
        {
            RTProject.SaveProject();
        }

        /// <summary>
        /// 실시간 모델링 종료
        /// </summary>
        /// <remarks></remarks>
        public void StopRTsimulation()
        {
            GRMRT.StopGRM();
        }

        private void GRMRT_RTStatus(string strMSG)
        {
            RTStatus(strMSG);
        }

        public string PrjName
        {
            get
            {
                return System.IO.Path.GetFileNameWithoutExtension(mFPN_RTEnv);
            }
        }
    }
}
