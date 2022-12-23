#pragma once

#include "gentle.h"

using namespace std;
namespace fs = std::filesystem;

#define CFL_NUMBER  1.0
#define TOLERANCE  0.001
#define WETDRY_CRITERIA 0.000001
#define ITER_NR 20000

//const string CONST_GMP_FILE_EXTENSION = ".gmp";
// �������� ��ü ��� �ؽ�Ʈ����
const string CONST_TAG_DISCHARGE = "_Discharge.out";
const string CONST_TAG_DISCHARGE_PTAVE = "_Discharge_Ave.out";  // ��±Ⱓ ������ ��հ� ���
const string CONST_TAG_DEPTH = "_Depth.out";
const string CONST_TAG_RF_GRID = "_RFGrid.out";
const string CONST_TAG_RF_MEAN = "_RFUpMean.out";
//const string CONST_TAG_FC_DATA_APP = "_FCData.out"; // 2022.10.17 �ּ�ó��
const string CONST_TAG_FC_STORAGE = "_FCStorage.out";
const string CONST_TAG_FC_INFLOW = "_FCinflow.out";
const string CONST_TAG_FC_INFLOW_PTAVE = "_FCinflow_Ave.out";  // ��±Ⱓ ������ ��հ� ���

// wp �� ���
const string CONST_TAG_WP = "_WP_";

// ������ ���� ��� ����
const string CONST_DIST_SSR_DIRECTORY_TAG = "_SSRD";
const string CONST_DIST_RF_DIRECTORY_TAG = "_RFD";
const string CONST_DIST_RFACC_DIRECTORY_TAG = "_RFAccD";
const string CONST_DIST_FLOW_DIRECTORY_TAG = "_FlowD";

const string CONST_DIST_PET_DIRECTORY_TAG = "_PETD";
const string CONST_DIST_ET_DIRECTORY_TAG = "_ETD";
const string CONST_DIST_INTERCEPTION_DIRECTORY_TAG = "_INTCPD";
const string CONST_DIST_SNOWMELT_DIRECTORY_TAG = "_SnowMD";

// ������ ���� �̸�
const string CONST_DIST_SSR_FILE_HEAD = "ss_";
const string CONST_DIST_RF_FILE_HEAD = "rf_";
const string CONST_DIST_RFACC_FILE_HEAD = "rfc_";
const string CONST_DIST_FLOW_FILE_HEAD = "flow_";

const string CONST_DIST_PET_FILE_HEAD = "pet_";
const string CONST_DIST_ET_FILE_HEAD = "et_";
const string CONST_DIST_INTERCEPTION_FILE_HEAD = "intcp_";
const string CONST_DIST_SNOWMELT_FILE_HEAD = "sm_";


const string CONST_OUTPUT_TABLE_TIME_FIELD_NAME = "DataTime";
const string CONST_OUTPUT_TABLE_MEAN_RAINFALL_FIELD_NAME = "Rainfall_Mean";

const double CONST_MIN_SLOPE = 0.000001;
const double CONST_EXPONENTIAL_NUMBER_UNSATURATED_K = 6.4;
//const double CONST_WEDGE_FLOW_COEFF = 1; // �ֻ�� ���� ���� �帧 ���� p�� ���ɿ� �������� ���

const double CONST_DEPTH_TO_BEDROCK = 20;// �Ϲݱ����� ����[m]
const double CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN = 10;// ������������� �Ϲݱ����� ����[m]
const double CONST_DEPTH_TO_UNCONFINED_GROUNDWATERTABEL = 10;// ���Ǿд���������� ����[m]
const double CONST_UAQ_HEIGHT_FROM_BEDROCK = 5;//   �Ϲݿ��� ���Ǿд���� ��ܱ����� ����[m]

enum class channelWidthType
{
	CWEquation,
	CWGeneration,
	None
};

enum class crossSectionType
{
	CSSingle,
	CSCompound,
	None
};

enum class fcDataSourceType
{
	UserEdit,
	Constant,
	TextFile,
	ReservoirOperation,
	None
};

enum class flowControlType
{
	ReservoirOutflow, // �������, ������ ������� �ʰ�, �￡���� ������� �����
	Inlet,  // ��� �������� �ʴ°�. ������ ������� �ʰ�, inlet grid������ outfow �� �����.
	SinkFlow, // �������, �Էµ� sink flow data �����. ������ �������.
	SourceFlow, // �������, �Էµ� source flow data �����. ������ �������.
	ReservoirOperation, // �������, ������ ���, ������� operation rule�� ���ؼ� ������. ����� �Է� �������̽� �������� ����.
	// ������-�����, ���Է�-����� ������� �̿��ؼ� �ҽ��ڵ忡 �ݿ� ����
	None
};

enum class reservoirOperationType
{
	AutoROM,
	RigidROM,
	ConstantQ,
	SDEqation,
	None
};

enum class flowDirectionType
{
	StartsFromNE,
	StartsFromN,
	StartsFromE,
	StartsFromE_TauDEM,
	None
};

enum class GRMPrintType
{
	All,
	DischargeFile,
	DischargeAndFcFile,
	AverageFile,
	DischargeFileQ,
	AverageFileQ,
	AllQ,
	None
};

enum class simulationType
{
	Normal, //2021.01.19. SingleEvent�� Normal�� ����
	Normal_PE_SSR,
	RealTime,
	None
};

//����ȭ ������� ���� ���
enum class unSaturatedKType //python �������̽��� �����.
{
	Constant = 0,
	Linear = 1,
	Exponential = 2,
	None = 3
};

enum class PETmethod
{
	PenmanMonteith = 1,
	BlaneyCriddle = 2,
	Hamon = 3,
	PriestleyTaylor = 4,
	Hargreaves = 5,
	JensenHaise = 6,	
	Turc = 7,
	Constant = 8,
	UserData = 9,
	None = 10
};

enum class SnowMeltMethod
{
	
	Anderson = 1,
	Constant = 8,
	UserData = 9,
	None = 10
};

enum class InterceptionMethod
{
	LAIRatio = 1,
	Constant = 8,
	UserData = 9,
	None = 10
};

enum class cellFlowType
{
    OverlandFlow,
    ChannelFlow,
    ChannelNOverlandFlow,
    None
} ;

enum class soilTextureCode
{
	C,
	CL,
	L,
	LS,
	S,
	SC,
	SCL,
	SiC,
	SiCL,
	SiL,
	SL,
	USER,
	CONSTV,
	None
};

enum class soilDepthCode
{
	D,
	MDMS, //MDMS
	M, //MDMS
	S,
	VD,
	VS,
	USER,
	CONSTV,
	None
};

enum class landCoverCode
{
	WATR,
	URBN,
	BARE,
	WTLD,
	GRSS,
	FRST,
	AGRL,
	USER,
	CONSTV,
	None
};

typedef struct _projectFileTable
{
	const string nProjectSettings = "ProjectSettings";
	const string nSubWatershedSettings = "SubWatershedSettings";
	const string nChannelSettings = "ChannelSettings";
	const string nPETnSnowMeltSettings = "PETnSnowMeltSettings";
	const string nWatchPoints = "WatchPoints";
	const string nGreenAmptParameter = "GreenAmptParameter";
	const string nSoilDepth = "SoilDepth";
	const string nLandCover = "LandCover";
	const string nFlowControlGrid = "FlowControlGrid";
	const string nRTenv = "RTenv";
	int sProjectSettings = 0; //0::��Ȱ��, 1: Ȱ��
	int sSubWatershedSettings = 0; //0:��Ȱ��, 1: Ȱ��
	int sChannelSettings = 0; //0:��Ȱ��, 1: Ȱ��
	int sPETnSowMeltSettings = 0;//0:��Ȱ��, 1: Ȱ��
	int sWatchPoints = 0; //0:��Ȱ��, 1: Ȱ��
	int sGreenAmptParameter = 0; //0:��Ȱ��, 1: Ȱ��
	int sSoilDepth = 0; //0:��Ȱ��, 1: Ȱ��
	int sLandCover = 0; //0:��Ȱ��, 1: Ȱ��
	int sFlowControlGrid = 0; //0:��Ȱ��, 1: Ȱ��
	int sRTenv = 0; //0:��Ȱ��, 1: Ȱ��
} projectFileTable;

typedef struct _projectFileFieldName
{
	const string GRMSimulationType = "GRMSimulationType";
	const string DomainFile_01 = "DomainFile";
	const string DomainFile_02 = "WatershedFile";
	const string SlopeFile = "SlopeFile";
	const string FlowDirectionFile = "FlowDirectionFile";
	const string FlowDirectionType = "FlowDirectionType";
	const string FlowAccumFile = "FlowAccumFile";
	const string StreamFile = "StreamFile";
	const string ChannelWidthFile = "ChannelWidthFile";
	const string InitialSoilSaturationRatioFile = "InitialSoilSaturationRatioFile";
	const string InitialChannelFlowFile = "InitialChannelFlowFile";
	const string LandCoverDataType = "LandCoverDataType";
	const string LandCoverFile = "LandCoverFile";
	const string LandCoverVATFile = "LandCoverVATFile";

	const string LAIFile = "LAIFile";

	const string ConstantRoughnessCoeff = "ConstantRoughnessCoeff";
	const string ConstantImperviousRatio = "ConstantImperviousRatio";
	const string SoilTextureDataType = "SoilTextureDataType";
	const string SoilTextureFile = "SoilTextureFile";
	const string SoilTextureVATFile = "SoilTextureVATFile";
	const string ConstantSoilPorosity = "ConstantSoilPorosity";
	const string ConstantSoilEffPorosity = "ConstantSoilEffPorosity";
	const string ConstantSoilWettingFrontSuctionHead = "ConstantSoilWettingFrontSuctionHead";
	const string ConstantSoilHydraulicConductivity = "ConstantSoilHydraulicConductivity";
	const string SoilDepthDataType = "SoilDepthDataType";
	const string SoilDepthFile = "SoilDepthFile";
	const string SoilDepthVATFile = "SoilDepthVATFile";
	const string ConstantSoilDepth = "ConstantSoilDepth";
	//const string PrecipitationDataType_01 = "RainfallDataType";
	//const string PrecipitationDataType_02 = "PrecipitationDataType";
	const string PrecipitationDataFile_01 = "RainfallDataFile";
	const string PrecipitationDataFile_02 = "PrecipitationDataFile";
	const string PrecipitationInterval_min_01 = "RainfallInterval";
	const string PrecipitationInterval_min_02 = "RainfallInterval_min";
	const string PrecipitationInterval_min_03 = "PrecipitationInterval_min";

	// continuous================
	//const string TemperatureMaxDataType = "TemperatureMaxDataType";
	const string TemperatureMaxInterval_min = "TemperatureMaxInterval_min";
	const string TemperatureMaxDataFile = "TemperatureMaxDataFile";

	//const string TemperatureMinDataType = "TemperatureMinDataType";
	const string TemperatureMinInterval_min = "TemperatureMinInterval_min";
	const string TemperatureMinDataFile = "TemperatureMinDataFile";

	//const string DurationOfSunshineDataType = "DurationOfSunshineDataType";
	const string DaytimeLengthInterval_min = "DaytimeLengthInterval_min";
	const string DaytimeLengthDataFile = "DaytimeLengthDataFile";

	const string DaytimeHoursRatioDataFile = "DaytimeHoursRatioDataFile";
	const string BlaneyCriddleCoefDataFile = "BlaneyCriddleCoefDataFile";
	
	//const string SolarRadiationDataType = "SolarRadiationDataType";
	const string SolarRadiationInterval_min = "SolarRadiationInterval_min";
	const string SolarRadiationDataFile = "SolarRadiationDataFile";

	//const string SnowPackTemperatureDataType = "SnowPackTemperatureDataType";
	const string SnowPackTemperatureInInterval_min = "SnowPackTemperatureInInterval_min";
	const string SnowPackTemperatureDataFile = "SnowPackTemperatureDataFile";

	//==========================
	const string MaxDegreeOfParallelism = "MaxDegreeOfParallelism";
	const string SimulStartingTime = "SimulStartingTime"; // ������� �Է� ������  2017-11-28 23:10 ���� ���
	const string SimulationDuration_hr_01 = "SimulationDuration";
	const string SimulationDuration_hr_02 = "SimulationDuration_hr";
	const string ComputationalTimeStep_min_01 = "ComputationalTimeStep";
	const string ComputationalTimeStep_min_02 = "ComputationalTimeStep_min";
	const string IsFixedTimeStep = "IsFixedTimeStep";
	const string OutputTimeStep_min_01 = "OutputTimeStep";
	const string OutputTimeStep_min_02 = "OutputTimeStep_min";

	const string SimulateInfiltration = "SimulateInfiltration";
	const string SimulateSubsurfaceFlow = "SimulateSubsurfaceFlow";
	const string SimulateBaseFlow = "SimulateBaseFlow";

	// continuous================
	const string SimulateEvaportranspiration = "SimulateEvaportranspiration";
	const string SimulateSnowMelt = "SimulateSnowMelt";
	const string SimulateInterception = "SimulateInterception";
	//==========================

	const string SimulateFlowControl = "SimulateFlowControl";
	const string MakeIMGFile = "MakeIMGFile";
	const string MakeASCFile = "MakeASCFile";
	const string MakeSoilSaturationDistFile = "MakeSoilSaturationDistFile";
	const string MakeRfDistFile = "MakeRfDistFile";
	const string MakeRFaccDistFile = "MakeRFaccDistFile";
	const string MakeFlowDistFile = "MakeFlowDistFile";
	const string PrintOption = "PrintOption";
	const string PrintAveValue = "PrintAveValue";
	const string WriteLog = "WriteLog";
	// SubWatershedSettings table
	const string ID_SWP = "ID";
	const string IniSaturation = "IniSaturation";
	const string UnsaturatedKType = "UnsaturatedKType";
	const string CoefUnsaturatedK = "CoefUnsaturatedK";
	const string MinSlopeOF = "MinSlopeOF";
	const string MinSlopeChBed = "MinSlopeChBed";
	const string MinChBaseWidth_m_01 = "MinChBaseWidth";
	const string MinChBaseWidth_m_02 = "MinChBaseWidth_m";
	const string ChRoughness = "ChRoughness";
	const string DryStreamOrder = "DryStreamOrder";
	const string IniFlow = "IniFlow";
	const string CalCoefLCRoughness = "CalCoefLCRoughness";
	const string CalCoefPorosity = "CalCoefPorosity";
	const string CalCoefWFSuctionHead = "CalCoefWFSuctionHead";
	const string CalCoefHydraulicK = "CalCoefHydraulicK";
	const string CalCoefSoilDepth = "CalCoefSoilDepth";

	// continuous================
	const string InterceptionMethod = "InterceptionMethod";
	const string PETMethod = "PETMethod";
	const string ETCoef = "ETCoef";
	const string SnowmeltMethod = "SnowmeltMethod";
	const string SnowmeltTSR = "SnowmeltTSR";
	const string SnowmeltingTemp = "SnowmeltingTemp";
	const string SnowCovRatio = "SnowCovRatio";	
	const string SnowmeltCoef = "SnowmeltCoef";
	// continuous================

	//	// PETnSowMelt table
	//	const string ID_PETSM = "ID";
	//const string PETMethod = "PETMethod";
	//const string PETDataFile = "PETDataFile";
	//const string PETcoeffPlant = "PETcoeffPlant";
	//const string PETcoeffSoil = "PETcoeffSoil";
	//const string SnowMeltMethod = "SnowMeltMethod";
	//const string SnowMeltDataFile = "SnowMeltDataFile";
	//ChannelSettings table
	const string UserSet = "UserSet";
	const string WSID_CH = "WSID";
	const string CrossSectionType = "CrossSectionType";
	const string SingleCSChannelWidthType = "SingleCSChannelWidthType";
	const string ChannelWidthEQc = "ChannelWidthEQc";
	const string ChannelWidthEQd = "ChannelWidthEQd";
	const string ChannelWidthEQe = "ChannelWidthEQe";
	const string ChannelWidthMostDownStream_m_01 = "ChannelWidthMostDownStream";
	const string ChannelWidthMostDownStream_m_02 = "ChannelWidthMostDownStream_m";
	const string LowerRegionHeight = "LowerRegionHeight";
	const string LowerRegionBaseWidth = "LowerRegionBaseWidth";
	const string UpperRegionBaseWidth = "UpperRegionBaseWidth";
	const string CompoundCSChannelWidthLimit = "CompoundCSChannelWidthLimit";
	const string BankSideSlopeRight = "BankSideSlopeRight";
	const string BankSideSlopeLeft = "BankSideSlopeLeft";
	// FlowControlGrid table
	const string Name_FCG = "Name";
	const string ColX_FCG = "ColX";
	const string RowY_FCG = "RowY";
	const string ControlType = "ControlType";
	const string FCDT_min_01 = "DT";
	const string FCDT_min_02 = "DT_min";
	const string FlowDataFile = "FlowDataFile";
	const string IniStorage = "IniStorage";
	const string MaxStorage = "MaxStorage";
	//const string MaxStorageR = "MaxStorageR";
	const string NormalHighStorage = "NormalHighStorage";
	const string RestrictedStorage = "RestrictedStorage";
	const string RestrictedPeriod_Start = "RestrictedPeriod_Start";
	const string RestrictedPeriod_End = "RestrictedPeriod_End";
	const string ROType = "ROType";
	const string ROConstQ = "ROConstQ";
	const string ROConstQDuration = "ROConstQDuration";

	// WatchPoint table
	const string Name_WP = "Name";
	const string ColX_WP = "ColX";
	const string RowY_WP = "RowY";
	// GreenAmptParameters table
	const string GridValue_ST = "GridValue";
	const string GRMCode_ST = "GRMCode";
	const string Porosity = "Porosity";
	const string EffectivePorosity = "EffectivePorosity";
	const string WFSuctionHead = "WFSoilSuctionHead";
	const string HydraulicConductivity = "HydraulicConductivity";
	// SoilDepth table
	const string GridValue_SD = "GridValue";
	const string GRMCode_SD = "GRMCode";
	const string SoilDepthValue_01 = "SoilDepth";
	const string SoilDepthValue_02 = "SoilDepth_cm";
	
	// LandCover table
	const string GridValue_LC = "GridValue";
	const string GRMCode_LC = "GRMCode";
	const string RoughnessCoeff = "RoughnessCoefficient";
	const string ImperviousR = "ImperviousRatio";
	const string CanopyRatio = "CanopyRatio";
	const string InterceptionMaxWaterCanopy_mm = "InterceptionMaxWaterCanopy_mm";
} projectFileFieldName;

typedef struct _timeSeries
{
	string dataTime; // �д���
	double value;
} timeSeries;

typedef struct _projectFileInfo
{
	string fpn_prj = "";
	//string fpn_log = "";
	string fp_prj = "";
	string fn_withoutExt_prj = "";
	string fn_prj = "";
	fs::file_time_type prjfileSavedTime;
} projectfilePathInfo;

typedef struct _swsParameters  //pyGRMdll.py �� �ִ� ����ü�� ���� �����. ������ �°� �ؾ� �Ѵ�.
{
	int wsid = -1;
	double iniSaturation =-1.0;
	double minSlopeOF = 0.0;
	unSaturatedKType unSatKType = unSaturatedKType::None;
	double coefUnsaturatedK = 0.0;
	double minSlopeChBed = 0.0;
	double minChBaseWidth = 0.0;
	double chRoughness = 0.0;
	int dryStreamOrder = -1;
	double iniFlow = -1.0;
	double ccLCRoughness = 0.0;
	double ccPorosity = 0.0;
	double ccWFSuctionHead = 0.0;
	double ccHydraulicK = 0.0;
	double ccSoilDepth = 0.0;
	
	// continuous==============
	InterceptionMethod interceptMethod = InterceptionMethod::None;
	PETmethod potentialETMethod = PETmethod::None;
	double etCoeff = -1.0;
	SnowMeltMethod snowMeltMethod = SnowMeltMethod::None;
	double smeltTSR = 0.0; // ������ ������ �Ǵ� �µ�
	double smeltingTemp = 0.0; // snow pack���� �� ��ȭ��
	double snowCovRatio = 0.0; // ������ ���� �κ��� ������
	double smeltCoef = 0.0;
	// continuous==============

	int userSet = -1; //1 : true, 0 : false. dll�� gui���� ����. grm������ gmp ������ �Ű������� �״�� �о ���
} swsParameters;


typedef struct _wsNetwork
{
	map <int, vector<int>> wsidsNearbyUp; //����� �������� �� �ִ�.
	map <int, int> wsidNearbyDown; //�Ϸ��� �ϳ���
	map <int, vector<int>> wsidsAllUp;
	map <int, vector<int>> wsidsAllDown;
	vector <int> mdWSIDs;
	map <int, int> wsOutletidxs;
	map <int, int> mdWSIDofCurrentWS;
} wsNetwork;

typedef struct _wpLocationRC
{
	string wpName = "";
	int wpColX = -1;
	int wpRowY = -1;
} wpLocationRC;

typedef struct _wpSimData {
	vector<int> wpCVidxes;
	map<int, string> wpNames; //<idx, wpname>
	//<idx, value>
	map<int, double> rfiReadSumUpWS_mPs;// ���� wp ����� ���� �����ڷῡ�� ���� ���췮(���찭�� rfi).[m/s] 
	map<int, double> rfUpWSAveForDt_mm; // ���� wp ����� ���� dt(���ð� ����) ������ ��հ��췮. �����ڷḦ �̿��ؼ� ���Ȱ�.[mm]
	map<int, double> rfUpWSAveForPT_mm;// ���� wp ����� ���� ��½ð� ����) ������ ��հ��췮. �����ڷḦ �̿��ؼ� ���Ȱ�.[mm]
	map<int, double> rfUpWSAveTotal_mm;//���� watch point ����� ��հ��췮�� ������[mm]
	map<int, double> rfWPGridForPT_mm; // Watchpoint ���ڿ� ���� ��½ð� ���� ������ �������췮. �����ڷḦ �̿��ؼ� ���Ȱ�.[mm]
	map<int, double> rfWPGridTotal_mm; // Watchpoint ���ڿ� ���� �������췮[mm]
	map<int, double> q_cms_print;  // �� ���� writeDischargefile���� ���ǰ�, writeWPoutputFile���� ���ȴ�.
	map<int, double> Q_sumPT_m3; // Watchpoint ���ڿ� ���� ��ü����[m3]. ��½ð� ���ݵ����� ������. ��� ���� ��հ�(cms) ����Ҷ� ����
	map<int, double> Q_sumPT_m3_print; // �� ���� writeDischargefile���� ���ǰ�, writeWPoutputFile���� ���ȴ�.
	//map<int, double> inflowSumPT_m3; // ��� �Ⱓ���� ���� ���Է� m3, ��±Ⱓ������ ��� ���Է�[cms] ���� ���

	map<int, double> pet_sumPT_mm; // ��� �Ⱓ ������ ���� ��. 2022.10.26
	map<int, double> aet_sumPT_mm; // ��� �Ⱓ ������ ���� ��. 2022.10.26
	map<int, double> snowM_sumPT_mm; // ��� �Ⱓ ������ ���� ��. 2022.10.26

//Todo : ������ ������, ����
	//map<int, double> totalFlow_cms; // Watchpoint ���ڿ� ���� ��ü����[cms]. ������.
	//map<int, double> totalDepth_m; // Watchpoint ���ڿ� �������[m] 
	//map<int, double> maxFlow_cms; // Watchpoint ���ڿ� ���� �ִ�����[cms]
	//map<int, double> maxDepth_m; // Watchpoint ���ڿ� ���� �ְ����[m]
	//map<int, string> maxFlowTime; // Watchpoint ���ڿ� ���� �ִ����� �ð�. ÷�νð�.
	//map<int, string> maxDepthTime; // Watchpoint ���ڿ� ���� �ְ���� �ð�. ÷�νð�
	//map<int, double> qFromFCData_cms; // �ش� wp���� Flow control�� ���ؼ� ���Ǵ� ���� // 2022.10.17 �ּ�ó��
	//map<int, string> fpnWpOut; // Watch point �� ���ǰ�� ����� ���� ���� �̸� ����
	map<int, int>cvCountAllup;
} wpSimData;

//typedef struct _wpinfo {
//	int wpCVidxes = -1;
//	string wpNames = ""; //<idx, wpname>
//   //<idx, value>
//	double rfiReadSumUpWS_mPs = 0.0;// ���� wp ����� ���� �����ڷῡ�� ���� ���췮(���찭�� rfi).[m/s] 
//	double rfUpWSAveForDt_mm = 0.0; // ���� wp ����� ���� dt(���ð� ����) ������ ��հ��췮. �����ڷḦ �̿��ؼ� ���Ȱ�.[mm]
//	double rfUpWSAveForDtP_mm = 0.0;// ���� wp ����� ���� ��½ð� ����) ������ ��հ��췮. �����ڷḦ �̿��ؼ� ���Ȱ�.[mm]
//	double rfUpWSAveTotal_mm = 0.0;//���� watch point ����� ��հ��췮�� ������[mm]
//	double rfWPGridForDtP_mm = 0.0; // Watchpoint ���ڿ� ���� ��½ð� ���� ������ �������췮. �����ڷḦ �̿��ؼ� ���Ȱ�.[mm]
//	double rfWPGridTotal_mm = 0.0; // Watchpoint ���ڿ� ���� �������췮[mm]
//	double totalFlow_cms; // Watchpoint ���ڿ� ���� ��ü����[cms]. ������.
//	//Todo : ������ ������, ����
//	double totalDepth_m = 0.0; // Watchpoint ���ڿ� �������[m] 
//	double maxFlow_cms = 0.0; // Watchpoint ���ڿ� ���� �ִ�����[cms]
//	double maxDepth_m = 0.0; // Watchpoint ���ڿ� ���� �ְ����[m]
//	string maxFlowTime = ""; // Watchpoint ���ڿ� ���� �ִ����� �ð�. ÷�νð�.
//	string maxDepthTime = ""; // Watchpoint ���ڿ� ���� �ְ���� �ð�. ÷�νð�
//	double qFromFCData_cms = 0.0; // �ش� wp���� Flow control�� ���ؼ� ���Ǵ� ����
//	double qprint_cms = 0.0;
//	//map<int, string> fpnWpOut; // Watch point �� ���ǰ�� ����� ���� ���� �̸� ����
//	int cvCountAllup;
//} wpinfo;

typedef struct _channelSettingInfo
{
	int mdWsid = -1;
	crossSectionType csType = crossSectionType::None;
	channelWidthType csWidthType= channelWidthType::None;
	double cwEQc = 0.0;
	double cwEQd = 0.0;
	double cwEQe = 0.0;
	double cwMostDownStream = 0.0;
	double lowRHeight = 0.0;
	double lowRBaseWidth = 0.0;
	double highRBaseWidth = 0.0;
	double compoundCSChannelWidthLimit = 0.0;
	double bankSlopeRight = 0.0;
	double bankSlopeLeft = 0.0;
} channelSettingInfo;

typedef struct _flowControlinfo
{
	string fcName = "";
	int fcColX = -1;
	int fcRowY = -1;
	flowControlType fcType = flowControlType::None;
	int fcDT_min = 0;
	string fpnFCData = "";
	double iniStorage_m3 = -1.0;
	double maxStorage_m3 = -1.0;
	//double maxStorageR = -1.0;
	double NormalHighStorage_m3 = -1.0;
	double RestrictedStorage_m3=-1.0;
	string RestrictedPeriod_Start = ""; // mmMddD ���� Ȥ�� �ð����� ����
	string RestrictedPeriod_End = "";// mmMddD ���� Ȥ�� �ð����� ����
	int RestrictedPeriod_Start_min = -1;
	int RestrictedPeriod_End_min = -1;
	int restricedP_SM = -1;
	int restricedP_SD = -1;
	int restricedP_EM = -1;
	int restricedP_ED = -1;

	reservoirOperationType roType= reservoirOperationType::None;
	double roConstQ_cms = -1.0;
	double roConstQDuration_hr = -1.0;
} flowControlinfo;

typedef struct _flowControlCellAndData
{
	//map <int, double> fcDataAppliedNowT_m3Ps;// <idx, value>������ �𵨸� �ð�(t)�� ����� flow control data �� // 2022.10.17 �ּ�ó��
	vector<int> cvidxsinlet;
	vector<int> cvidxsFCcell;
	map<int, vector<timeSeries>> inputFlowData_m3Ps; //<idx, data>, �д���
	map<int, int> curDorder;// <idx, order>���� ����� �������� ����
	map<int, double> inflowSumPT_m3; // ��� �Ⱓ���� ���� ���Է� m3, ��±Ⱓ������ ��� ���Է�[cms] ���� ���
} flowControlCellAndData;

typedef struct _soilTextureInfo
{
	int stGridValue=-1;
	soilTextureCode stCode = soilTextureCode::None;
	double porosity = 0.0;
	double effectivePorosity = 0.0;
	double WFSuctionHead = 0.0;
	double hydraulicK = 0.0;
} soilTextureInfo;

typedef struct _soilDepthInfo
{
	int sdGridValue = -1;
	soilDepthCode sdCode = soilDepthCode::None;
	double soilDepth = 0.0;
} soilDepthInfo;

typedef struct _landCoverInfo
{
	int lcGridValue = -1;
	landCoverCode lcCode = landCoverCode::None;
	double RoughnessCoefficient = 0.0;
	double ImperviousRatio = 0.0;
	double CanopyRatio = -1.0;
	double InterceptionMaxWaterCanopy_mm = -1.0;
} landCoverInfo;

typedef struct _weatherData
{
	int Order = -1;
	string DataTime = "";
	string value = ""; // mean ������ ����ڷ� ��, asc������ ���� �̸�
	string FilePath = "";
	string FileName = "";
	map<int, double> vForEachRegion; // �� ������ ��
} weatherData;

//typedef struct _PETnSnowMeltInfo
//{
//	// continuous �� �˻�
//	// �̰� continuous �� =====================
//	int wsid = -1;
//	PETmethod petMethod = PETmethod::notSet;
//	string fpnPET = "";
//	double PETcoeffPlant = -1.0;
//	double PETcoeffSoil = -1.0;
//	snowMeltMethod smMethod = snowMeltMethod::notSet;
//	string fpnSnowMelt = "";
//	// =====================
//} PETnSMinfo;

typedef struct _grmOutFiles
{
	// ������ ��ü ��� ���� ���, �̸�
	string ofpnDischarge;	
	string ofpnDischargePTAve;
	string ofpnDepth;
	string ofpnRFGrid;
	string ofpnRFMean;
	//string ofpnFCData; // 2022.10.17 �ּ�ó��
	string ofpnFCStorage;
	string ofpnFCinflow;
	string ofpnFCinflowPTAve;

	// ������ ���� ���
	string ofpSSRDistribution;
	string ofpRFDistribution;
	string ofpRFAccDistribution;
	string ofpFlowDistribution;

	string ofpPETDistribution;
	string ofpETDistribution;
	string ofpINTCPDistribution;
	string ofpSnowMDistribution;

	// watchpoint �� ��� ���� ���, �̸�
	map<int, string> ofpnWPs; //<idx, fpn>
} grmOutFiles;

typedef struct _domaininfo
{
	double dx = 0.0;
	int nRows = 0;
	int nCols = 0;
	double xll = 0.0;
	double yll = 0.0;
	double cellSize = 0.0;
	int nodata_value = -9999;
	string headerStringAll = "";

	int cvidxMaxFac;//fac�� ���� ū ��(���Ϸ��� ��)�� 1���� �迭 ��ȣ
	int cellNnotNull = 0;
	int cellNtobeSimulated = 0;
	int facMostUpChannelCell = -1;
	int facMax = -1;
	int facMin = INT_MAX;
	vector <int> dmids;
	map <int, vector<int>> cvidxInEachRegion;
	wsNetwork wsn;
} domaininfo;

typedef struct _cvStreamAtt
{
	double slopeCH = 0.0;//�ϵ����� �ϵ�+��ǥ�� �帧 �Ӽ��� ������ ���ڿ� �ο��Ǵ� ��õ���(m/m)
	int cellValue = -1;//��õ����
	double chBaseWidth = 0.0;//��õ�� �ٴ���[m]
	double chBaseWidthByLayer = 0.0;//�������̾�� ���� ����[m]
	double chRC = 0.0;//������ channel CV�� �ϵ��������
	double csaCH_ori = 0.0;//t �ð������� �����ؼ� ���� ���� channel CV�� �ʱ� �帧�ܸ���[m^2]
	double csaCH = 0.0;//t �ð������� �����ؼ� ��� ���� channel CV�� �帧�ܸ���[m^2]
	double csaChAddedByOFinCHnOFcell = 0.0;//�ϵ�+��ǥ�� �帧������, ��ǥ�� �帧�� ���� �ϵ��帧 �ܸ��� ������
	double hCH_ori = 0.0;//t �ð������� �����ؼ� ���� ���� channel CV�� �ʱ� ����[m]
	double hCH = 0.0;//t �ð������� �����ؼ� ��� ���� channel CV�� ����[m]
	double uCH = 0.0; //t �ð������� �����ؼ� ��� ���� channel CV�� ����[m/s]
	double QCH_m3Ps = 0.0;//t �ð������� �����ؼ� ��� ���� channel CV�� ����[m^3/s]
	double iniQCH_m3Ps = 0.0;//��õ�������� �ʱ� ������ ���Ϸ� ������ �����Ǵ� ��[m^3/s]
	double chSideSlopeLeft = 0.0;//������ channel CV�� ���� ���� ���
	double chSideSlopeRight = 0.0;//������ channel CV�� ���� ���� ���
	double bankCoeff = 0.0;// ������ channel CV�� ���� ���. ��� ���Ǹ� ���ؼ� channel CV ���� �̸������ ��
	double chURBaseWidth_m = 0.0;//���� channel CV�� ���ܸ� ������� �ٴ� ��[m]
	double chLRHeight = 0.0;// ���� channel CV�� ���ܸ� ��������� ����[m]
	int isCompoundCS = -1;//������ channel CV�� ���ܸ�����(true), �ܴܸ�(false)������ ��Ÿ���� ����
	double chLRArea_m2 = 0.0;// ���ܸ� channel �� �������� ����[m^2]
} cvStreamAtt;

typedef struct _cvpos
{
	int xCol;
	int yRow;
	int wsid = -1; //���� ID, �ش� raster cell�� ��
} cvpos;

typedef struct _cvAtt
{
	//int xCol;
	//int yRow;
	//int wsid = -1; //���� ID, �ش� raster cell�� ��
	cellFlowType flowType;//���� ����, ��ǥ���帧, �ϵ��帧, ��ǥ��+�ϵ�
	double slopeOF = 0.0; //��ǥ�� �ؼ��� ����Ǵ� overland flow ���� ���(m/m)
	double slope = 0.0; //���� ���(m/m)
	flowDirection8 fdir;//�帧����
	int fac = -1;//�帧������, �ڽ��� ���� �����ϰ�, ����� �ִ� ���� ����. �ּҰ��� 0 �ִ밪�� ��ȿ�� ���� -1
	double dxDownHalf_m = 0.0;//���� �߽����κ��� �Ϸ����� ���ڸ������ �Ÿ�
	double dxWSum = 0.0;//���� �߽����κ��� ������� ���ڸ������ �Ÿ���
	vector<int> neighborCVidxFlowintoMe;//���� ���� �귯 ������ �������� ID, �ִ� 7��
	int downCellidxToFlow = -1; //�귯�� ���Ϸ����� ID
	int effCVnFlowintoCVw = -1;//��������� �� ���� ������ �߻��ϴ� ������ ����
	double cvdx_m;//�𵨸��� ������ �˻�ü���� X���� ����
	vector<int> downWPCVidx;//���� CV �Ϸ��� �ִ� watchpoint ���� idxes ��
	int toBeSimulated = 0; // -1 : false, 1 : true //������ CV�� ������ ������ �ƴ��� ǥ��
	cvStreamAtt stream;//���� CV�� Stream �ϰ�� ��, eCellType�� Channel Ȥ�� OverlandAndChannel�� ��� �ο��Ǵ� �Ӽ�
	int isStream = 0; // ���� cv�� stream ���� �ƴ���
	//double hOF_ori = 0.0;//t �ð����� �����ؼ� ���� �� overland flow �˻�ü���� ����
	double uOF = 0.0;//t �ð����� �����ؼ� ��� overland flow �˻�ü���� ����
	double hOF = 0.0;  //t �ð����� �����ؼ� ��� overland flow �˻�ü���� ����
	double csaOF = 0.0;//t �ð����� �����ؼ� ��� overland flow�� �帧 �ܸ���
	//double qinflow_m2Ps = 0.0;//�������� ���� ������ ���� ����
	double qOF_m2Ps = 0.0; //�������� overland flow ����
	double QOF_m3Ps = 0.0;//t �ð������� �����ؼ� ��� overland flow�� ���� [m^3/s]
	double QSSF_m3Ps = 0.0;//t �ð������� ���� ������ �������� ��ǥ�Ͽ��� ����Ǵ� ���� [m^3/s]
	double QsumCVw_dt_m3 = 0.0;//������� CV���� ���� CV�� ���ԵǴ� ���� �ܼ���. �̰� CVi������ ���ӹ�����, ������� ���� �ܼ� ��.[m^3/dt]
	double QsumCVw_m3Ps = 0.0;//������� CV���� ���� CV�� ���ԵǴ� ���� �ܼ���. �̰� CVi������ ���ӹ�����, ������� ���� �ܼ� ��.[m^3/s]
	double rfApp_mPdt = 0.0;//dt �ð� ������ ���췮
	double rfiRead_mPsec = 0.0;//���� �����Է��ڷῡ�� ���� ���찭�� m/s rfi.
	double rfiRead_tm1_mPsec = 0.0;//���� �ð��� ���찭�� m/s rfi.
	double rfEff_dt_m = 0.0;//dt�ð� ������ ��ȿ���췮
	double rf_dtPrint_m = 0.0;//��� �ð����� ������ ���� ���췮[m]
	double rfAcc_fromStart_m = 0.0;//��ü �Ⱓ�� ���� ���췮[m]
	double soilWaterC_m = 0.0;//�������Է�. t �ð������� ���� ħ����[m], �����̰� �ƴϰ�, �����̴�.
	double soilWaterC_tm1_m = 0.0;//�������Է�. t-1 �ð������� ���� ħ����[m]. ����
	double ifRatef_mPsec = 0.0;//t �ð����� ���� ħ����[m/s]
	double ifRatef_tm1_mPsec = 0.0;//t-1 �ð����� ����� ħ����[m/s]
	double ifF_mPdt = 0.0;//t �ð����� ���� dt �ð������� ħ����[m/dt]
	int isAfterSaturated = 0;// -1 : false, 1: true
	soilTextureCode stCode;
	int stCellValue = -1;//�伺���̾��� ��, VAT���� // 0 ���� ����� �ǹ��ϰ� �Ѵ�.
	double hc_K_mPsec = 0.0;//���� CV ����� ����������[m/s] �𵨸� ���밪
	double hc_Kori_mPsec = 0.0;//���� CV ����� ����������[m/s] GRM default
	double effPorosity_ThetaE = 0.0;//���� CV ����� ��ȿ���ط� �𵨸� ���밪. ������. 0~1
	double effPorosity_ThetaEori = 0.0;//���� CV ����� ��ȿ���ط� grm default. ������. 0~1
	double porosity_Eta = 0.0;//���� CV ����� ���ط� �𵨸� ���밪. ������. 0~1
	double porosity_EtaOri = 0.0;//���� CV ����� ���ط� GRM default. ������. 0~1
	double wfsh_Psi_m = 0.0;//���� CV ����� �����������μ���[m] �𵨸� ���밪
	double wfsh_PsiOri_m = 0.0;//���� CV ����� �����������μ���[m] grm default
	double soilMoistureChange_DTheta = 0.0;//��������ȭ��
	soilDepthCode sdCode;
	int sdCellValue = -1;//��ɷ��̾��� ��, VAT ���� // 0 ���� ����� �ǹ��ϰ� �Ѵ�.
	double sd_m = 0.0;//���� CV�� ���� �𵨸� ���� ��[m].
	double sdOri_m = 0.0;//���� CV�� ���� GRM default ��[m].
	double sdEffAsWaterDepth_m = 0.0;//���� CV�� ��ȿ���� ��[m]. ���ɿ��� ��ȿ���ط��� ���� ��
	double iniSSR = 0.0;//���� CV ����� �ʱ���ȭ��. ������. 0~1
	double effSR_Se = 0.0; // //���� CV ����� ��ȿ��ȭ��. ������. 0~1 ������ %/100
	double ssr = 0.0;//����� ���� ��ȭ��
	unSaturatedKType ukType;
	double coefUK = 0.0;
	double hUAQfromChannelBed_m = 0.0; //�ϵ������� ���Ǿд������ ����(�ϵ��ٴڿ����� ����)[m].
	double hUAQfromBedrock_m = 0.0;///�Ϲ����κ��� ���Ǿд������ ��ܺα����� ����[m]. ������.
	double sdToBedrock_m = 0.0;//���� CV ����� �Ϲݱ����� ����[m]. ��ǥ�鿡�� �Ϲݱ����� ������.
	double bfQ_m3Ps = 0.0;//���� CV�� �������ⷮ [m^3/s]
	landCoverCode lcCode;
	int lcCellValue = -1;//�����Ǻ����̾��� ��, VAT ���� // 0 ���� ����� �ǹ��ϰ� �Ѵ�.
	double imperviousR = 0.0;//���� CV �����Ǻ��� ��������. ������, 0~1.
	double rcOF = 0.0;//���� CV �����Ǻ��� �𵨸� ���� ��ǥ�� �������
	double rcOFori = 0.0;//���� CV �����Ǻ��� grm default ��ǥ�� �������
	flowControlType fcType=flowControlType::None;//���� CV�� �ο��� Flow control ����

	InterceptionMethod intcpMethod;
	double canopyR = 0.0;
	double intcpMaxWaterCanopy_m = 0.0;//���� CV �����Ǻ������� �ִ� ���ܷ� [m].
	double intcpAcc_m = 0.0; // ���� ���ܷ�

	double tempMaxPday = -9999.0;
	double tempMinPday = -9999.0;
	double solarRad_mm = -1.0;
	double sunDur_hrs = -1.0;

	PETmethod petMethod;
	double aet_mPdt = 0.0; //���� ���߻귮
	double aet_LS_mPdt = 0.0; // 2022.11.30. ���鿡���� ���� ���߻귮. aet���� aet_canopy�� ������ ��. ��ǥ�� ���ɰ� ��� ������ �ս� ��꿡 �̿��.
	double pet_mPdt = 0.0; //���� ���߻귮
	double etCoef = 0.0;

	SnowMeltMethod smMethod;
	double spackAcc_m = 0.0; // snow pack ����
	double smelt_mPdt = 0.0; //������ 	
	double spackTemp = -9999.0;
	double smeltTSR = -9999.0;
	double smeltingT = 0.0;
	double snowCovR = 0.0;
	double smeltCoef = -9999.0;

	double storageCumulative_m3 = 0.0;//���� CV���� flow control ���ǽ� ���� ������[m^3]
	double storageAddedForDTbyRF_m3 = 0.0;//���� CV���� flow control ���ǽ� dt �ð������� ���쿡 ���ؼ� �߰��Ǵ� ������[m^3/dt]
} cvAtt;

typedef struct _projectFile
{
	simulationType simType = simulationType::None;
	string	fpnDomain = "";
	string fpnProjection = "";
	string fpnSlope = "";
	string fpnFD = "";
	string fpnFA = "";
	string fpnStream = "";
	string fpnChannelWidth = "";
	string fpniniSSR = "";
	string fpniniChFlow = "";
	fileOrConstant lcDataType = fileOrConstant::None;
	string fpnLC = "";
	string fpnLCVat = ""; // �𵨿��� ���� �̿������ �ʴ´�. GUI���� �̿�ȴ�. �𵨿����� gmp ���Ͽ� �ִ� �Ű����� �̿���
	string fpnLAI="";
	double cnstRoughnessC = 0.0;
	double cnstImperviousR = 0.0;
	fileOrConstant stDataType = fileOrConstant::None;
	string fpnST = "";
	string fpnSTVat = ""; // �𵨿��� ���� �̿������ �ʴ´�. GUI���� �̿�ȴ�. �𵨿����� gmp ���Ͽ� �ִ� �Ű����� �̿���
	double cnstSoilPorosity = 0.0;
	double cnstSoilEffPorosity = 0.0;
	double cnstSoilWFSH = 0.0;
	double cnstSoilHydraulicK = 0.0;
	fileOrConstant sdDataType = fileOrConstant::None;
	string fpnSD = "";
	string fpnSDVat = ""; // �𵨿��� ���� �̿������ �ʴ´�. GUI���� �̿�ȴ�. �𵨿����� gmp ���Ͽ� �ִ� �Ű����� �̿���
	double cnstSoilDepth = 0.0;

	weatherDataType rfDataType = weatherDataType::None;
	string fpnRainfallData = "";
	int rfinterval_min = -1;
	flowDirectionType fdType = flowDirectionType::None;

	// continuous =====================
	weatherDataType tempMaxDataType = weatherDataType::None;
	string fpnTempMaxData = "";
	int tempMaxInterval_min = -1;
	weatherDataType tempMinDataType = weatherDataType::None;
	string fpnTempMinData = "";
	int tempMinInterval_min = -1;
	
	weatherDataType durationOfSunDataType = weatherDataType::None;
	string fpnDurationOfSunData = "";
	int durationOfSunInterval_min = -1;

	weatherDataType solarRadDataType = weatherDataType::None;
	string fpnSolarRadData = "";
	int solarRadInterval_min = -1;

	//weatherDataType durationOfSunRatioDataType = weatherDataType::None;
	string fpnDurationOfSunRatioData = "";
	//int durationOfSunRatioInterval_min = -1;
	string fpnBlaneyCriddleK = "";

	weatherDataType snowpackTempDataType = weatherDataType::None;
	string fpnSnowpackTempData = "";
	int snowpackTempInterval_min = -1;
	// =====================

	int mdp = 0;
	string simStartTime = ""; // ������� �Է� ������  2017-11-28 23:10 ���� ���. �ǽð������� yyyymmddHHMM ����
	double simDuration_hr = 0.0;
	int dtsec = 0;
	int isFixedTimeStep = 0;// true : 1, false : -1
	int dtPrint_min = 0;

	int simInfiltration = 0;// true : 1, false : -1
	int simSubsurfaceFlow = 0;// true : 1, false : -1
	int simBaseFlow = 0;// true : 1, false : -1
	int simEvaportranspiration = 0;// true : 1, false : -1
	int simSnowMelt = 0;// true : 1, false : -1
	int simInterception = 0;// true : 1, false : -1
	int simFlowControl = 0;// true : 1, false : -1

	int makeIMGFile = 0;// true : 1, false : -1
	int makeASCFile = 0;// true : 1, false : -1
	int makeSoilSaturationDistFile = 0;// true : 1, false : -1
	int makeRfDistFile = 0;// true : 1, false : -1
	int makeRFaccDistFile = 0;// true : 1, false : -1
	int makeFlowDistFile = 0;// true : 1, false : -1
	GRMPrintType printOption = GRMPrintType::None;
	int printAveValue = 0;// true : 1, false : -1 // ��±Ⱓ ��� �� ��� ����

	//int writeConsole = 0;// true : 1, false : -1
	int forSimulation = 0;

	map <int, swsParameters> swps; // <wsid, paras>
	map <int, channelSettingInfo> css; //<wsid. paras>
	map <int, flowControlinfo> fcs; // <idx, paras>
	//map <int, PETnSMinfo> petsms; //<wsid. paras>
	vector <wpLocationRC> wps; // 
	vector <soilTextureInfo> sts;
	vector <soilDepthInfo> sds;
	vector <landCoverInfo> lcs;

	int isDateTimeFormat = 0;// true : 1, false : -1
	int isinletExist = 0;// true : 1, false : -1
	int streamFileApplied = 0;
	int cwFileApplied = 0;
	int icfFileApplied = 0;
	int issrFileApplied = 0;
	int makeASCorIMGfile = 0;
	int makeRFraster = 0;
	//int applyFC = 0;

	CPUsInfo cpusi;
	int deleteAllFilesExceptDischargeOut = -1;
	int writeLog = -1;
	
} projectFile;

typedef struct _thisSimulation
{
	//int setupGRMisNormal = 0; // Todo : �ʿ俩�� Ȯ�� �ʿ�.
	int enforceFCautoROM = -1;// true : 1, false : -1
	int grmStarted = 0;
	int stopSim = 0;
	int dataNumTotal_rf = 0;
	double rfAveForDT_m = 0;
	double rfAveSumAllCells_PT_m = 0; // ��±Ⱓ ���ȿ����� ������ ���� �ִ� ��� �� ���� ��
	double rfAveSumAllCells_PT_m_bak = 0; // ��±Ⱓ ���ȿ����� ������ ���� �ִ� ��� �� ���� �� ���
	double rfiSumAllCellsInCurRFData_mPs; //rfi : rf intensity

	int dataNumTotal_tempMax = 0;
	int dataNumTotal_tempMin = 0;
	int dataNumTotal_solarR = 0;
	int dataNumTotal_sunDur = 0;
	int dataNumTotal_snowPackTemp = 0;

	int dtsec = 0;
	int dtsecUsed_tm1 = 0;
	int dtMaxLimit_sec = 0;
	int dtMinLimit_sec = 0;
	int zeroTimePrinted = 0;
	int simEnding_sec = -1;
	int simDuration_min = 0;

	int tsec_tm1 = 0;
	int targetTtoP_sec = 0;
	//int iscvsb = -1; // �����ð������� cvs�� ����Ǿ� �ִ��� ����
	int cvsbT_sec = 0;
	int isbak = 0;

	int runByAnalyzer = 0;

	COleDateTime time_thisSimStarted;
	double vMaxInThisStep=-9999.0;

	int tCurMonth = 0;
	int tCurDay = 0;
	double laiRatioCurDay = 0.0;
} thisSimulation;

typedef struct _globalVinner // ��� ������ �����ϱ� ���� �ּ����� ���� ����. gpu ���
{
	int mdp = 0;//-1�� ���� �ִ� ������ �ڵ����� ����, 1�� ���� serial ���
} globalVinner;

double calBFlowAndGetCSAaddedByBFlow(int i, 
	int dtsec, 	double cellSize_m);//i�� cv array index
double calRFlowAndSSFlow(int i,
	int dtsec, double dy_m); // ���� cv�� Return flow�� ������� ���ԵǴ� ssflwo�� ����ϰ�, ���� cv������ ssf�� ���� ���� �����Է����� ����Ѵ�.
void calBFLateralMovement(int i,
	int facMin, double dY_m, double dtsec);
void calChannelFlow(int i, double chCSACVw_tp1);
void calEffectiveRFbyInfiltration(int i, int dtrf_sec, int dtsec);
void calFCReservoirOutFlow(int i, double nowTmin); //i�� cv array index

void cal_h_SWC_OFbyAET(int i); // ���߻꿡 ���� ��ǥ�� ���ɰ� ��� ���� ���
void cal_h_CHbyAET(int i);// ���߻꿡 ���� �ϵ� ���� ���

//void cal_h_OFbySM(int i);
//void cal_h_CHbySM(int i);
void calinterception(int i);
void calET(int i);
//int calMeanTemperature();
void calSnowMelt(int i);

void calOverlandFlow(int i, double hCVw_tp1,
	double effDy_m);
void calPET_BlaneyCriddle(int i);
void calPET_Hamon(int i);
void calPET_PriestleyTaylor(int i);
void calPET_Hargreaves(int i);


void calReservoirOperation(int i, double nowTmin);
void calReservoirOutFlowInReservoirOperation(int i,
	double Qout_cms, double dy_m);
void calSinkOrSourceFlow(int i, double nowTmin);
void calValuesDuringPT(int dtsec);
void convertFCtypeToAutoROM(string strDate, int cvid);

void disposeDynamicVars();
int deleteAllOutputFiles();

// ������Ʈ ���ϰ� �α����ϵ� ��������. ���� �ʿ�
int deleteAllFilesExceptDischarge();

vector<int> getAllUpstreamCells(int startingidx);
double getChCSAatCVW(int i);
double getChCSAbyFlowDepth(double LRBaseWidth, 
	double chBankConst,	double crossSectionDepth,
	int isCompoundCS, double LRHeight,
	double LRArea, double URBaseWidth);
double getChCSAusingQbyiteration(cvAtt cv, 
	double CSAini, double Q_m3Ps);
double getChCSAaddedBySSFlow(int i);
double getChCrossSectionPerimeter(double LRegionBaseWidth,
	double sideSlopeRightBank, double sideSlopeLeftBank,
	double crossSectionDepth, int isCompoundCS,
	double LRegionHeight, double LRegionArea,
	double URegionBaseWidth);
double getChDepthUsingCSA(double baseWidthLRegion,
	double chCSAinput, int isCompoundCS,
	double baseWidthURegion, double LRegionArea,
	double LRegionHeight, double chBankConst);
int getCVidxByFcName(string fcName);

int getDTsec(double dx, 
	double vMax, int dtMax_min, 
	int dtMin_min);
double getinfiltrationForDtAfterPonding(int i, int dtSEC,
	double CONSTGreenAmpt, double Kapp);
projectfilePathInfo getProjectFileInfo(string fpn_prj);
flowDirection8 getFlowDirection(int fdirV, 
	flowDirectionType fdType);
double getOverlandFlowDepthCVw(int i);
double getMeanRFValueToPrintForAllCells(double cinterp); // ������ ���� �ִ� ��� ���� ��� �� ����� ���� ���������̼� ��
weatherDataType getWeatherDataTypeByDataFile(string fpn_wdata);
void grmHelp();

void updateCVbyHydroComps(int i); // i�� cv array index
int initOutputFiles();
void initRasterOutput();
void initThisSimulation();
void initRFvars();
void initValuesAfterPrinting(); //����� �� ���� �ʱ�ȭ �Ǿ�� �ϴ� ������ ó��
int initWatershedNetwork();
int initWPinfos();
int isNormalChannelSettingInfo(channelSettingInfo *csi);
int isNormalFlowControlinfo(flowControlinfo *fci);
int isNormalSwsParameter(swsParameters *swp);
//int isNormalPETnSnowMelt(PETnSMinfo* petsmi);
int isNormalWatchPointInfo(wpLocationRC *wpL);
int isNormalSoilTextureInfo(soilTextureInfo *st);
int isNormalSoilDepthInfo(soilDepthInfo *sd);
int isNormalLandCoverInfo(landCoverInfo *lc);

void joinOutputThreads();

double Kunsaturated(cvAtt cv);
void Log_Performance_data(string strBasin, string strTag,
	string strDataTime, double dblElapTime);

void makeIMG_ssr();
void makeIMG_rf();
void makeIMG_rfacc();
void makeIMG_flow();
void makeASC_ssr();
void makeASC_rf();
void makeASC_rfacc();
void makeASC_flow();

int makeNewOutputFiles();
int makeRasterOutput(int nowTmin);

int openProjectFile(int forceRealTime);
int openPrjAndSetupModel(int forceRealTime);//1:true, -1:false
int outputManager(int nowTsec); // , int nowRForder);
vector<weatherData> readAndSetWeatherData(string fpn_in_wd, weatherDataType wdType,
	int dataInterval_min, string dataString);
int readDomainFaFileAndSetupCV();
int readSlopeFdirStreamCwCfSsrFileAndSetCV();
int readLandCoverFile();
int readLandCoverFileAndSetCVbyVAT();
int readSoilTextureFile();
int readSoilTextureFileAndSetCVbyVAT();
int readSoilDepthFile();
int readSoilDepthFileAndSetCVbyVAT();

int readXmlRowChannelSettings(string aline, 
	channelSettingInfo* csi);
int readXmlRowProjectSettings(string aline);
int readXmlRowSubWatershedSettings(string aline,
	swsParameters *swp);
int readXmlRowFlowControlGrid(string aline, 
	flowControlinfo *fci);
//int readXmlPETnSnowMelt(string aline,
//	PETnSMinfo* petsmi);
int readXmlRowWatchPoint(string aline, 
	wpLocationRC *wpl);
int readXmlRowSoilTextureInfo(string aline,
	soilTextureInfo* st);
int readXmlRowSoilDepth(string aline,
	soilDepthInfo* sd);
int readXmlRowLandCover(string aline,
	landCoverInfo* lc);

//void setCurLAIRatio();
int setBlaneyCriddleK();
int setCVbyLCConstant();
int setCVbySTConstant();
int setCVbySDConstant();
int setCVRF(int order);
void setCVRFintensityAndDTrf_Zero();

int setCVTempMax(int order);
void setCVTempMaxZero();
int setCVTempMin(int order);
void setCVTempMinZero();
int setCVSolarRad(int order);
void setCVSolarRZero();
int setCVSunDur(int order);
void setCVSunDurZero();
int setCVSnowpackTemp(int order);
void setCVSnowpackTempZero();

void setCVStartingCondition(double iniflow);
int setDomainAndCVBasicinfo();
int setFlowNetwork();
int setLAIRatio();
int setRainfallData();
int setRasterOutputArray();

// ��ȭ����а�� ����
void setSVPGradient();
int setTemperatureMax();
int setTemperatureMin();
int setDaytimeLength();
int setDaytimeHoursRatio();
int setSnowPackTemp();
int setSolarRadiation();

int setupAndStartSimulation();
int setupByFAandNetwork();
int setupModelAfterOpenProjectFile();
vector<int> setWSIDsInWeatherDataFile(string aStringLine);
int simulateRunoff(double nowTmin);
void simulateRunoffCore(int i, double nowTmin);
int startSimulation();
int startSingleRun(string fpnGMP, 
	int isPrediction, string outString);
int startGMPsRun(vector<string> gmpFiles, 
	int isPrediction, string outString);

double totalSSFfromCVwOFcell_m3Ps(int i);

int updateWatershedNetwork();
int updateCVbyUserSettings();
int updateFCCellinfoAndData();
void updateAllSWSParsUsingNetwork();
bool updateOneSWSParsWithOtherSWSParsSet(int TargetWSid,
	int ReferenceWSid);

int writeBySimType(int nowTP_min,
	double cinterp);
void writeDischargeFile(string tStrToPrint, 
	double cinterp); 
void writeDischargeAveFile(string tStrToPrint,
	double cinterp);
void writeFCOutputFile(string tStrToPrint,
	double cinterp);
void writeFCAveOutputFile(string tStrToPrint,
	double cinterp);
void writeWPoutputFile(string tStrToPrint,
	double cinterp); // �̰� ����� ���� ������ writeDischargeFile()�� ���ӵǾ� �ִ�.
void writeRainfallOutputFile(string tStrToPrint,
	double cinterp);
void writeSimProgress(int elapsedT_min);

//void writeDischargeOnly(double cinterp, // 2022.10. 20. ��� ���� �������� �ּ�ó��
//	int writeWPfiles, int writeFCfiles);
//void writeSimResult(string tStrToPrint,
//	double interCoef);
//void writeWPoutputFile(string nowTP, 
//	int i, double cinterp);

inline double  getinterpolatedVLinear(double firstV,
	double nextV, double cinterp);
inline double rfintensity_mPsec(double rf_mm, 
	double dtrf_sec);
inline void setNoFluxCVCH(int i);
inline void setNoFluxCVOF(int i);
inline void setNoInfiltrationParameters(int i);
inline void setWaterAreaInfiltrationPars(int i);
inline double soilSSRbyCumulF(double cumulinfiltration,
	double effSoilDepth, cellFlowType flowType);
inline  double vByManningEq(double hydraulicRaidus,
	double slope, double nCoeff);


// for extern "C"
class grmWSinfo {
private:
	void setPublicVariables();
	bool byGMPfile = false;

public:
	int facMaxCellxCol;
	int facMaxCellyRow;
	vector<int> WSIDsAll;
	int WScount = 0;
	vector<int> mostDownStreamWSIDs;
	int mostDownStreamWSCount;
	int cellCountInWatershed = 0;
	double cellSize = 0;

	grmWSinfo(string fdirType, string fpnDomain,
		string slopeFPN, string fpnFdir, string fpnFac,
		string fpnStream = "", string fpnLandCover = "",
		string fpnSoilTexture = "", string fpnSoilDepth = "",
		string fpnIniSoilSaturationRatio = "",
		string pfnIniChannelFlow = "",
		string fpnChannelWidth = "");
	grmWSinfo(string gmpFPN);
	~grmWSinfo();

	bool isInWatershedArea(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	vector<int> upStreamWSIDs(int currentWSID);
	int upStreamWSCount(int currentWSID);
	vector<int> downStreamWSIDs(int currentWSID);
	int downStreamWSCount(int currentWSID);
	int watershedID(int colXAryidx, int rowYAryidx); // �迭 �ε��� ���
	string flowDirection(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	int flowAccumulation(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	double slope(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	int streamValue(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	string cellFlowType(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	int landCoverValue(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	int soilTextureValue(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	int soilDepthValue(int colXAryidx, int rowYAryidx);// �迭 �ε��� ���
	vector<string> allCellsInUpstreamArea(int colXAryidx, int rowYAryidx);
	int cellCountInUpstreamArea(int colXAryidx, //  Select all cells in upstream area of a input cell position. Return string list of cell positions - "column, row".
		int rowYAryidx);

	// Just update memory. The gmp file is not revised.
	// If you want to revise the gmp file, you need a writing process using updated parameters in memory.
	//  To get the updated paramters in memory for a subwatershed, you can use subwatershedPars() function.
	bool setOneSWSParsAndUpdateAllSWSUsingNetwork(int wsid, double iniSat,
		double minSlopeLandSurface, unSaturatedKType unSKType, double coefUnsK,
		double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
		int dryStreamOrder, double ccLCRoughness,
		double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
		double ccSoilHydraulicCond, double iniFlow = 0);
	void updateAllSubWatershedParametersUsingNetwork();

	// �Է����Ϸ� �ν��Ͻ� ���� ��쿡�� setOneSWSParsAndUpdateAllSWSUsingNetwork() �̰� ȣ�� �Ŀ� ��� ����
	swsParameters subwatershedPars(int wsid);
	bool removeUserParametersSetting(int wsid);
};






