#pragma once

#include "gentle.h"

using namespace std;
namespace fs = std::filesystem;

#define CFL_NUMBER  1.0
#define TOLERANCE  0.001
#define WETDRY_CRITERIA 0.000001
#define ITER_NR 20000
#define SOIL_SATURATION_RATIO_CRITERIA 0.99
#define PONDING_DEPTH_CRITERIA_M 0.001
#define GRAVITY_ACC 9.8 

const double CONST_MIN_SLOPE = 0.000001;
const double CONST_EXPONENTIAL_NUMBER_UNSATURATED_K = 6.4;

const double CONST_DEPTH_TO_BEDROCK = 20;// 암반까지의 깊이[m]
const double CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN = 10;// 산악지역에서의 암반까지의 깊이[m]
const double CONST_DEPTH_TO_UNCONFINED_GROUNDWATERTABEL = 10;// 비피압대수층까지의 깊이[m]
const double CONST_UAQ_HEIGHT_FROM_BEDROCK = 5;// 암반으로부터 포화된 비피압대수층 상단까지의 높이[m]

//const string CONST_GMP_FILE_EXTENSION = ".gmp";
// 지정셀들 전체 출력 텍스트파일
const string CONST_TAG_DISCHARGE = "_Discharge.out";
const string CONST_TAG_DISCHARGE_PTAVE = "_Discharge_Ave.out";  // 출력기간 동안의 평균값 출력
const string CONST_TAG_DEPTH = "_Depth.out";
const string CONST_TAG_PRCP_GRID = "_PRCPGrid.out";
const string CONST_TAG_PRCP_MEAN = "_PRCPUpMean.out";
const string CONST_TAG_FC_STORAGE = "_FCStorage.out";
const string CONST_TAG_FC_INFLOW = "_FCinflow.out";
const string CONST_TAG_FC_INFLOW_PTAVE = "_FCinflow_Ave.out";  // 출력기간 동안의 평균값 출력

// wp 별 출력
const string CONST_TAG_WP = "_WP_";

// 래스터 파일 출력 폴더
const string CONST_DIST_SSR_DIRECTORY_TAG = "_SSRD";
const string CONST_DIST_RF_DIRECTORY_TAG = "_PRCPD";
const string CONST_DIST_RFACC_DIRECTORY_TAG = "_PRCPAccD";
const string CONST_DIST_FLOW_DIRECTORY_TAG = "_FlowD";

const string CONST_DIST_PET_DIRECTORY_TAG = "_PETD";
const string CONST_DIST_AET_DIRECTORY_TAG = "_AETD";
const string CONST_DIST_INTERCEPTION_DIRECTORY_TAG = "_INTCPD";
const string CONST_DIST_SNOWMELT_DIRECTORY_TAG = "_SnowMD";

// 래스터 파일 이름
const string CONST_DIST_SSR_FILE_HEAD = "ss_";
const string CONST_DIST_RF_FILE_HEAD = "prcp_";
const string CONST_DIST_RFACC_FILE_HEAD = "prcpc_";
const string CONST_DIST_FLOW_FILE_HEAD = "flow_";

const string CONST_DIST_PET_FILE_HEAD = "pet_";
const string CONST_DIST_AET_FILE_HEAD = "aet_";
const string CONST_DIST_INTERCEPTION_FILE_HEAD = "intcp_";
const string CONST_DIST_SNOWMELT_FILE_HEAD = "sm_";


const string CONST_OUTPUT_TABLE_TIME_FIELD_NAME = "DataTime";
const string CONST_OUTPUT_TABLE_MEAN_PRCP_FIELD_NAME = "PRCP_Mean";


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
	ReservoirOutflow, // 상류모의, 저류량 고려하지 않고, 댐에서의 방류량만 고려함
	Inlet,  // 상류 모의하지 않는것. 저류량 고려하지 않고, inlet grid에서의 outfow 만 고려함.
	SinkFlow, // 상류모의, 입력된 sink flow data 고려함. 저수지 고려안함.
	SourceFlow, // 상류모의, 입력된 source flow data 고려함. 저수지 고려안함.
	ReservoirOperation, // 상류모의, 저수지 고려, 방류량은 operation rule에 의해서 결정됨. 사용자 입력 인터페이스 구현하지 않음.
	// 저류량-방류량, 유입량-방류량 관계식을 이용해서 소스코드에 반영 가능
	DetentionPond,
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

enum class flowDirection8
{
	//N=1, NE=2, E=4, SE=8, S=16,  SW=32, W=64, NW=128, NONE=0
	E8 = 1, NE8 = 2, N8 = 3, NW8 = 4, W8 = 5, SW8 = 6, S8 = 7, SE8 = 8, None8 = 0
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
	Normal, //2021.01.19. SingleEvent를 Normal로 수정
	//Normal_PE_SSR,
	RealTime,
	None
};

//불포화 투수계수 산정 방법
enum class unSaturatedKType //pyGRMdll.py 에 있는 구조체와 내용 맞춘다. 순서도 맞게 해야 한다.
{
	Constant = 1,
	Linear = 2,
	Exponential = 3,
	None = 0
};

enum class PETmethod  //pyGRMdll.py 에 있는 구조체와 내용 맞춘다. 순서도 맞게 해야 한다.
{
	BC = 1, //BlaneyCriddle
	FPM = 2, //FAOPenmanMonteith
	HMN = 3, //Hamon
	HRGV = 4, //Hargreaves
	PT = 5, //PriestleyTaylor	
	//JH = 6,	 //JensenHaise
	//TRC = 7, //Turc
	//Constant = 8,
	UserET = 9, // PET 혹은 AET
	None = 0
};

enum class SnowMeltMethod  //pyGRMdll.py 에 있는 구조체와 내용 맞춘다. 순서도 맞게 해야 한다.
{
	
	Anderson = 1,
	Constant = 8,
	UserData = 9,
	None = 0
};

enum class InterceptionMethod  //pyGRMdll.py 에 있는 구조체와 내용 맞춘다. 순서도 맞게 해야 한다.
{
	LAIRatio = 1,
	Constant = 8,
	UserData = 9,
	None = 0
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

enum class whatWeatherData
{	
	Precipitation,
	TempMax,
	TempMin,
	DaytimeLength,
	DewPointTemp,
	SolarRad,
	WindSpeed, 
	UserET,
	SnowpackTemp,
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
	int sProjectSettings = 0; //0::비활성, 1: 활성
	int sSubWatershedSettings = 0; //0:비활성, 1: 활성
	int sChannelSettings = 0; //0:비활성, 1: 활성
	int sPETnSowMeltSettings = 0;//0:비활성, 1: 활성
	int sWatchPoints = 0; //0:비활성, 1: 활성
	int sGreenAmptParameter = 0; //0:비활성, 1: 활성
	int sSoilDepth = 0; //0:비활성, 1: 활성
	int sLandCover = 0; //0:비활성, 1: 활성
	int sFlowControlGrid = 0; //0:비활성, 1: 활성
	int sRTenv = 0; //0:비활성, 1: 활성
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
	const string DEMFile = "DEMFile"; // v2025
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
	const string PrecipitationDataFile_01 = "RainfallDataFile";
	const string PrecipitationDataFile_02 = "PrecipitationDataFile";
	const string PrecipitationInterval_min_01 = "RainfallInterval";
	const string PrecipitationInterval_min_02 = "RainfallInterval_min";
	const string PrecipitationInterval_min_03 = "PrecipitationInterval_min";

	// continuous================
	const string TemperatureMaxInterval_min = "TemperatureMaxInterval_min";
	const string TemperatureMaxDataFile = "TemperatureMaxDataFile";

	const string TemperatureMinInterval_min = "TemperatureMinInterval_min";
	const string TemperatureMinDataFile = "TemperatureMinDataFile";

	const string DaytimeLengthInterval_min = "DaytimeLengthInterval_min";
	const string DaytimeLengthDataFile = "DaytimeLengthDataFile";

	const string DaytimeHoursRatioDataFile = "DaytimeHoursRatioDataFile";
	const string BlaneyCriddleCoefDataFile = "BlaneyCriddleCoefDataFile";

	const string SolarRadiationInterval_min = "SolarRadiationInterval_min";
	const string SolarRadiationDataFile = "SolarRadiationDataFile";

	// v2025=====================================================
	const string DewPointTemperatureDataFile = "DewPointTemperatureDataFile";
	const string DewPointTemperatureInterval_min = "DewPointTemperatureInterval_min";

	const string WindSpeedDataFile = "WindSpeedDataFile";
	const string WindSpeedInterval_min = "WindSpeedInterval_min";

	const string UserPETDataFile = "UserPETDataFile";
	const string UserAETDataFile = "UserAETDataFile";
	const string UserETDataInterval_min = "UserETDataInterval_min";

	const string SnowPackTemperatureInInterval_min = "SnowPackTemperatureInterval_min";
	const string SnowPackTemperatureDataFile = "SnowPackTemperatureDataFile";
	const string LatitudeTOP_degree = "LatitudeTOP_degree";  //위도. 남반구에서는 - 값
	// v2025===================================================
	
	//==========================
	const string MaxDegreeOfParallelism = "MaxDegreeOfParallelism";
	const string SimulationStartingTime_01 = "SimulStartingTime"; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	const string SimulationStartingTime_02 = "SimulationStartingTime"; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용

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
	const string SimulateEvapotranspiration = "SimulateEvapotranspiration";
	const string SimulateSnowMelt = "SimulateSnowMelt";
	const string SimulateInterception = "SimulateInterception";
	//==========================

	const string SimulateFlowControl = "SimulateFlowControl";

	const string PrintOption = "PrintOption";
	const string PrintAveValue = "PrintAveValue";
	const string AveValueTimeInterval_min = "AveValueTimeInterval_min";
	const string ValueSeparator = "ValueSeparator";
	const string MakeIMGFile = "MakeIMGFile";
	const string MakeASCFile = "MakeASCFile";
	const string MakeSoilSaturationDistFile = "MakeSoilSaturationDistFile";
	const string MakeRfDistFile = "MakeRfDistFile";
	const string MakeRFaccDistFile = "MakeRFaccDistFile";
	const string MakeFlowDistFile = "MakeFlowDistFile";
	const string MakePETDistFile = "MakePETDistFile";
	const string MakeAETDistFile = "MakeAETDistFile";
	const string WriteLog = "WriteLog";
	// SubWatershedSettings table
	const string ID_SWP = "ID";
	const string IniSaturation = "IniSaturation";
	const string IniLossPRCP_mm = "IniLossPRCP_mm";
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

	const string InterceptionMethod = "InterceptionMethod";
	const string PETMethod = "PETMethod";
	const string ETCoef = "ETCoef";
	const string SnowmeltMethod = "SnowmeltMethod";
	const string TempSnowRain = "TempSnowRain";
	const string SnowmeltingTemp = "SnowmeltingTemp";
	const string SnowCovRatio = "SnowCovRatio";
	const string SnowmeltCoef = "SnowmeltCoef";
	// continuous================

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
	const string AutoROM_MaxOutFlow_CMS = "AutoROM_MaxOutFlow_CMS";
	const string ROType = "ROType";
	const string ROConstRatio = "ROConstRatio";
	const string ROConstDischarge = "ROConstDischarge";
	const string ROConstDischargeDuration_hr = "ROConstDischargeDuration_hr";
	// 저류지관련
	const string DP_QT_StoD_CMS = "DP_QT_StoD_CMS";
	const string DP_Qi_max_CMS = "DP_Qi_max_CMS";
	const string DP_Qo_max_CMS = "DP_Qo_max_CMS";
	const string DP_Wdi_m = "DP_Wdi_m";
	const string DP_Ws_m = "DP_Ws_m";
	const string DP_Cr_StoD = "DP_Cr_StoD";

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
	string dataTime; // 분단위
	double value;
} timeSeries;

typedef struct _projectFileInfo
{
	string fpn_prj = "";
	string fp_prj = "";
	string fn_withoutExt_prj = "";
	string fn_prj = "";
	fs::file_time_type prjfileSavedTime;
} projectfilePathInfo;

typedef struct _swsParameters  //pyGRMdll.py 에 있는 구조체와 내용 맞춘다. 순서도 맞게 해야 한다.
{
	int wsid = -1;
	double iniSaturation =-1.0;
	double iniLossPRCP_mm = -1.0;
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
	//double latidute_degreeC = -100.0;
	double etCoeff = -1.0;
	SnowMeltMethod snowMeltMethod = SnowMeltMethod::None;
	double tempSnowRain = 0.0; // 강수가 강설로 되는 온도
	double smeltingTemp = 0.0; // snow pack에서 일 승화량
	double snowCovRatio = 0.0; // 눈으로 덮힌 부분의 면적비
	double smeltCoef = 0.0;
	// continuous==============

	int userSet = -1; //1 : true, 0 : false. dll와 gui에서 사용됨. grm에서는 gmp 파일의 매개변수를 그대로 읽어서 사용
} swsParameters;


typedef struct _wsNetwork
{
	map <int, vector<int>> wsidsNearbyUp; //상류는 여러개일 수 있다.
	map <int, int> wsidNearbyDown; //하류는 하나다
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
	map<int, double> prcpiReadSumUpWS_mPs;// 현재 wp 상류에 대해 원시자료에서 읽은 강수량(강수강도 rfi).[m/s] 
	map<int, double> prcpUpWSAveForDt_mm; // 현재 wp 상류에 대해 dt(계산시간 간격) 동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
	map<int, double> prcpUpWSAveForPT_mm;// 현재 wp 상류에 대해 출력시간 간격) 동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
	map<int, double> prcpUpWSAveTotal_mm;//현재 watch point 상류의 평균강우량의 누적값[mm]
	map<int, double> prcpWPGridForPdT_mm; // Watchpoint 격자에 대한 출력시간 간격 동안의 누적강우량. 원시자료를 이용해서 계산된값.[mm]
	map<int, double> prcpWPGridTotal_mm; // Watchpoint 격자에 대한 누적강우량[mm]
	map<int, double> q_cms_print;  // 이 값은 writeDischargefile에서 계산되고, writeWPoutputFile에서 사용된다.
	map<int, double> Q_sumPdTforAVE_m3; // Watchpoint 격자에 대한 전체유량[m3]. 출력시간 간격동안의 누적값. 출력 간격 평균값(cms) 계산할때 사용됨
	map<int, double> Q_sumPTforAVE_m3_print; // 이 값은 writeDischargefile에서 계산되고, writeWPoutputFile에서 사용된다.

	map<int, double> pet_grid_sumPdT_mm; // 출력 기간 동안의 격자 누적 값. 2022.10.26
	map<int, double> aet_grid_sumPdT_mm; // 출력 기간 동안의 격자 누적 값. 2022.10.26
	map<int, double> snowM_grid_sumPdT_mm; // 출력 기간 동안의 격자 누적 값. 2022.10.26

	map<int, int>cvCountAllup;
} wpSimData;


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
	int fcDT_min = -1;
	string fpnFCData = "";
	double iniStorage_m3 = -1.0;
	double maxStorage_m3 = -1.0;
	double NormalHighStorage_m3 = -1.0;
	double RestrictedStorage_m3 = -1.0;
	string RestrictedPeriod_Start = ""; // mmMddD 포맷 혹은 시간단위 숫자 e.g. 12M06D or 56
	string RestrictedPeriod_End = "";// mmMddD 포맷 혹은 시간단위 숫자
	int RestrictedPeriod_Start_min = -1;
	int RestrictedPeriod_End_min = -1;
	int restricedP_SM = -1;
	int restricedP_SD = -1;
	int restricedP_EM = -1;
	int restricedP_ED = -1;
	double autoROMmaxOutflow_cms = -1.0;

	reservoirOperationType roType = reservoirOperationType::None;
	double roConstR = -1.0;
	double roConstQ_cms = -1.0;
	double roConstQDuration_hr = -1.0;

	//저류지 관련
	double dp_QT_StoD_CMS = -1;
	double dp_Qi_max_CMS = -1;
	double dp_Qo_max_CMS = -1;
	double dp_Wdi_m = -1;
	double dp_Ws_m = -1;
	double dp_Cr_StoD = -1;
} flowControlinfo;

typedef struct _flowControlCellAndData
{
	vector<int> cvidxsinlet;
	vector<int> cvidxsFCcell;
	map<int, vector<timeSeries>> inputFlowDataFCType1_m3Ps; //<idx, data>, 분단위 FCType1에 대한 자료
	map<int, vector<timeSeries>> inputFlowDataFCType2_m3Ps; //<idx, data>, 분단위 FCType2에 대한 자료, Sink or Source flow 만 입력
	map<int, vector<timeSeries>> inputFlowDataFCType3_m3Ps; //<idx, data>, 분단위 FCType3에 대한 자료, Sink or Source flow 만 입력
	map<int, int> curDorder;// <idx, order>현재 적용될 데이터의 순서
	map<int, double> inflowSumPdT_m3; // 출력 기간동안 누적 유입량 m3, 출력기간동안의 평균 유입량[cms] 계산시 사용
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
	string value = ""; // mean 에서는 기상자료 값, asc에서는 파일 이름
	string FilePath = "";
	string FileName = "";
	map<int, double> vForEachRegion; // 각 영역별 값
} weatherData;

typedef struct _grmOutFiles
{
	// 지정셀 전체 출력 파일 경로, 이름
	string ofpnDischarge;	
	string ofpnDischargePTAve;
	string ofpnDepth;
	string ofpnPRCPGrid;
	string ofpnPRCPMean;
	string ofpnFCStorage;
	string ofpnFCinflow;
	string ofpnFCinflowPTAve;

	// 래스터 파일 경로
	string ofpSSRDistribution;
	string ofpPRCPDistribution;
	string ofpPRCPAccDistribution;
	string ofpFlowDistribution;
	string ofpPETDistribution;
	string ofpAETDistribution;

	string ofpINTCPDistribution;
	string ofpSnowMDistribution;

	// watchpoint 별 출력 파일 경로, 이름
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

	int cvidxMaxFac;//fac가 가장 큰 셀(최하류셀 등)의 1차원 배열 번호
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
	double slopeCH = 0.0;//하도셀과 하도+지표면 흐름 속성을 가지는 격자에 부여되는 하천경사(m/m)
	int cellValue = -1;//하천차수
	double chBaseWidth = 0.0;//하천의 바닥폭[m]
	double chBaseWidthByLayer = 0.0;//하폭레이어에서 받은 하폭[m]
	double chRC = 0.0;//현재의 channel CV의 하도조도계수
	double csaCH_ori = 0.0;//t 시간에서의 유출해석 전의 현재 channel CV의 초기 흐름단면적[m^2]
	double csaCH = 0.0;//t 시간에서의 유출해석 결과 현재 channel CV의 흐름단면적[m^2]
	double csaChAddedByOFinCHnOFcell = 0.0;//하도+지표면 흐름셀에서, 지표면 흐름에 의한 하도흐름 단면적 증가분
	double hCH_ori = 0.0;//t 시간에서의 유출해석 전의 현재 channel CV의 초기 수심[m]
	double hCH = 0.0;//t 시간에서의 유출해석 결과 현재 channel CV의 수심[m]
	double uCH = 0.0; //t 시간에서의 유출해석 결과 현재 channel CV의 유속[m/s]
	double QCH_m3Ps = 0.0;//t 시간에서의 유출해석 결과 현재 channel CV의 유량[m^3/s]
	double iniQCH_m3Ps = 0.0;//하천셀에서의 초기 유량을 파일로 받을때 설정되는 값[m^3/s]
	double chSideSlopeLeft = 0.0;//현재의 channel CV의 좌측 제방 경사
	double chSideSlopeRight = 0.0;//현재의 channel CV의 우측 제방 경사
	double bankCoeff = 0.0;// 현재의 channel CV의 제방 계수. 계산 편의를 위해서 channel CV 별로 미리계산한 값
	double chURBaseWidth_m = 0.0;//현재 channel CV의 복단면 고수부지 바닥 폭[m]
	double chLRHeight = 0.0;// 현재 channel CV의 복단면 고수부지의 수심[m]
	int isCompoundCS = -1;//현재의 channel CV가 복단면인지(true), 단단면(false)인지를 나타내는 변수
	double chLRArea_m2 = 0.0;// 복단면 channel 중 하층부의 면적[m^2]
} cvStreamAtt;

typedef struct _cvpos
{
	int xCol;  // 좌상단 0 부터 시작
	int yRow; // 좌상단 0 부터 시작
	int wsid = -1; //유역 ID, 해당 raster cell의 값
	int intLat_degree = -9999; // 각셀의 위도.배열 인덱스가, cv 인덱스와 같게 한다.남반구는 - 값
	double lat_rad = -9999;
} cvpos;

typedef struct _cvAtt
{
	//int wsid = -1; //유역 ID, 해당 raster cell의 값
	cellFlowType flowType;//셀의 종류, 지표면흐름, 하도흐름, 지표면+하도
	double slopeOF = 0.0; //지표면 해석에 적용되는 overland flow 셀의 경사(m/m)
	double slope = 0.0; //셀의 경사(m/m). 경사 파일에서 읽은 값.
	flowDirection8 fdir;//흐름방향
	int fac = -1;//흐름누적수, 자신의 셀을 제외하고, 상류에 있는 격자 개수. 최소값은 0 최대값은 유효셀 개수 -1
	double dxDownHalf_m = 0.0;//격자 중심으로부터 하류방향 격자면까지의 거리
	double dxWSum = 0.0;//격자 중심으로부터 상류방향 격자면까지의 거리합
	vector<int> neighborCVidxFlowintoMe;//현재 셀로 흘러 들어오는 인접셀의 ID, 최대 7개
	int downCellidxToFlow = -1; //흘러갈 직하류셀의 ID
	int effCVnFlowintoCVw = -1;//인접상류셀 중 실제 유출이 발생하는 셀들의 개수
	double cvdx_m;//모델링에 적용할 검사체적의 X방향 길이
	vector<int> downWPCVidx;//현재 CV 하류에 있는 watchpoint 들의 idxes 들
	int toBeSimulated = 0; // -1 : false, 1 : true //현재의 CV가 모의할 셀인지 아닌지 표시
	cvStreamAtt stream;//현재 CV가 Stream 일경우 즉, eCellType이 Channel 혹은 OverlandAndChannel일 경우 부여되는 속성
	int isStream = 0; // 현재 cv가 stream 인지 아닌지
	double uOF = 0.0;//t 시간에서 유출해석 결과 overland flow 검사체적의 유속
	double hOF = 0.0;  //t 시간에서 유출해석 결과 overland flow 검사체적의 수심
	double csaOF = 0.0;//t 시간에서 유출해석 결과 overland flow의 흐름 단면적
	double qOF_m2Ps = 0.0; //단위폭당 overland flow 유량
	double QOF_m3Ps = 0.0;//t 시간에서의 유출해석 결과 overland flow의 유량 [m^3/s]
	double QSSF_m3Ps = 0.0;//t 시간에서의 현재 셀에서 다음셀로 지표하에서 유출되는 유량 [m^3/s]
	double QsumCVw_dt_m3 = 0.0;//상류인접 CV에서 현재 CV로 유입되는 유량 단순합. 이건 CVi에서의 연속방정식, 고려하지 않은 단순 합.[m^3/dt]
	double QsumCVw_m3Ps = 0.0;//상류인접 CV에서 현재 CV로 유입되는 유량 단순합. 이건 CVi에서의 연속방정식, 고려하지 않은 단순 합.[m^3/s]
	double rfApp_mPdt = 0.0;//dt 시간 동안의 강우량
	double rfiRead_mPsec = 0.0;//현재 강우입력자료에서 읽은 강우강도 m/s rfi. 
	double rfiRead_After_iniLoss_mPsec = 0.0;//현재 강우입력자료에서 읽은 강우강도 m/s rfi. 강우의 초기손실 적용 후.
	double rfiRead_tm1_mPsec = 0.0;//이전 시간의 강우강도 m/s rfi.
	double rfEff_dt_m = 0.0;//dt시간 동안의 유효강우량
	double rf_PDT_m = 0.0;//출력 시간간격 동안의 누적 강우량[m]
	double rfAccRead_fromStart_mm = 0.0;//전체 기간의 누적 강우량[mm]
	double soilWaterC_m = 0.0;//토양수분함량. t 시간까지의 누적 침투량[m], 토양깊이가 아니고, 수심이다.
	double soilWaterC_tm1_m = 0.0;//토양수분함량. t-1 시간까지의 누적 침투량[m]. 수심
	double ifRatef_mPsec = 0.0;//t 시간에서 계산된 침투률[m/s]
	double ifRatef_tm1_mPsec = 0.0;//t-1 시간에서 적용된 침투률[m/s]
	double ifF_mPdt = 0.0;//t 시간에서 계산된 dt 시간동안의 침투량[m/dt]
	int isAfterSaturated = 0;// -1 : false, 1: true
	soilTextureCode stCode;
	int stCellValue = -1;//토성레이어의 값, VAT참조 // 0 값은 상수를 의미하게 한다.
	double hc_K_mPsec = 0.0;//현재 CV 토양의 수리전도도[m/s] 모델링 적용값
	double hc_Kori_mPsec = 0.0;//현재 CV 토양의 수리전도도[m/s] GRM default
	double effPorosity_ThetaE = 0.0;//현재 CV 토양의 유효공극률 모델링 적용값. 무차원. 0~1
	double effPorosity_ThetaEori = 0.0;//현재 CV 토양의 유효공극률 grm default. 무차원. 0~1
	double porosity_Eta = 0.0;//현재 CV 토양의 공극률 모델링 적용값. 무차원. 0~1
	double porosity_EtaOri = 0.0;//현재 CV 토양의 공극률 GRM default. 무차원. 0~1
	double wfsh_Psi_m = 0.0;//현재 CV 토양의 습윤전선흡인수두[m] 모델링 적용값
	double wfsh_PsiOri_m = 0.0;//현재 CV 토양의 습윤전선흡인수두[m] grm default
	double soilMoistureChange_DTheta = 0.0;//토양습윤변화량
	soilDepthCode sdCode;
	int sdCellValue = -1;//토심레이어의 값, VAT 참조 // 0 값은 상수를 의미하게 한다.
	double sd_m = 0.0;//현재 CV의 토양심 모델링 적용 값[m].
	double sdOri_m = 0.0;//현재 CV의 토양심 GRM default 값[m].
	double sdEffAsWaterDepth_m = 0.0;//현재 CV의 유효토양심 값[m]. 토양심에서 유효공극률을 곱한 값
	double iniSSR = 0.0;//현재 CV 토양의 초기포화도. 무차원. 0~1
	double effSR_Se = 0.0; // //현재 CV 토양의 유효포화도. 무차원. 0~1 무차원 %/100
	double ssr = 0.0;//토양의 현재 포화도
	unSaturatedKType ukType;
	double coefUK = 0.0;
	double hUAQfromChannelBed_m = 0.0;  //하도셀에서 비피압대수층의 수심(하도바닥에서의 높이)[m].
	double hUAQfromBedrock_m = 0.0; //암반으로부터 비피압대수층의 상단부까지의 깊이[m]. 토양깊이.
	double sdToBedrock_m = 0.0;//현재 CV 토양의 암반까지의 깊이[m]. 지표면에서 암반까지의 깊이임.
	double bfQ_m3Ps = 0.0;//현재 CV의 기저유출량 [m^3/s]
	landCoverCode lcCode;
	int lcCellValue = -1;//토지피복레이어의 값, VAT 참조 // 0 값은 상수를 의미하게 한다.
	double imperviousR = 0.0;//현재 CV 토지피복의 불투수율. 무차원, 0~1.
	double rcOF = 0.0;//현재 CV 토지피복의 모델링 적용 지표면 조도계수
	double rcOFori = 0.0;//현재 CV 토지피복의 grm default 지표면 조도계수
	flowControlType fcType1=flowControlType::None;//현재 CV에 부여된 Flow control 종류
	flowControlType fcType2 = flowControlType::None;//Sink와 Source만 설정. 다른 속성과 중복설정하기 위한 것
	flowControlType fcType3 = flowControlType::None;//Sink와 Source만 설정. 다른 속성과 중복설정하기 위한 것

	double tempMaxPday = -9999.0; // degreeC
	double tempMinPday = -9999.0;// degreeC
	double solarRad_MJpM2 = -1.0; // 기상청 일사량 단위는 MJ/m^2 이다. 2024.11.25
	double daytimeLength_hrs = -1.0;
	double dewPointTemp = -9999;// 이슬점 온도 degreeC
	double windSpeed_mps = -1; // 풍속 m/s
	double userET_mm = -9999;

	InterceptionMethod intcpMethod;
	double canopyR = 0.0;
	double intcpMaxWaterCanopy_m = 0.0;//현재 CV 토지피복에서의 최대 차단량 [m].
	double intcpAcc_m = 0.0; // 누적 차단량

	PETmethod petMethod;
	double pet_mPdt = 0.0; //잠재 증발산량
	double pet_PDT_m = 0.0; // 출력 시간 간격 동안이 누적 잠재증발산량.
	double aet_mPdt = 0.0; //실제 증발산량
	double aet_LS_mPdt = 0.0; // 2022.11.30. 지면에서의 실제 증발산량. aet에서 aet_canopy를 제외한 값. 지표면 수심과 토양 수분의 손실 계산에 이용됨.
	double aet_PDT_m = 0.0; // 출력 시간 간격 동안이 누적 실제증발산량.
	double etCoef = 0.0;

	SnowMeltMethod smMethod;
	double spackAcc_m = 0.0; // snow pack 깊이
	double smelt_mPdt = 0.0; //융설량 	
	double spackTemp = -9999.0;
	double smeltTSR = -9999.0;
	double smeltingT = 0.0;
	double snowCovR = 0.0;
	double smeltCoef = -9999.0;

	double storageCumulative_m3 = 0.0;//현재 CV에서 flow control 모의시 누적 저류량[m^3]
	double storageAddedForDTbyRF_m3 = 0.0;//현재 CV에서 flow control 모의시 dt 시간동안의 강우에 의해서 추가되는 저류량[m^3/dt]

	double DP_storageCumulative_m3 = 0.0;//현재 CV에서 설정된 detention pond 누적 저류량[m^3]
	double DP_inflow_m3Ps = 0.0;//현재 CV에서 설정된 detention pond 유입량[m^3/s]
	double DP_outflow_m3Ps = 0.0;//현재 CV에서 설정된 detention pond 유출량[m^3/s]
} cvAtt;

typedef struct _projectFile
{
	simulationType simType = simulationType::None;
	string	fpnDomain = "";
	string fpnProjection = "";
	string fpnSlope = "";
	string fpnFD = "";
	string fpnFA = "";
	string fpnDEM = "";
	string fpnStream = "";
	string fpnChannelWidth = "";
	string fpniniSSR = "";
	string fpniniChFlow = "";
	fileOrConstant lcDataType = fileOrConstant::None;
	string fpnLC = "";
	string fpnLCVat = ""; // 모델에서 직접 이용되지는 않는다. GUI에서 이용된다. 모델에서는 gmp 파일에 있는 매개변수 이용함
	string fpnLAI="";
	double cnstRoughnessC = 0.0;
	double cnstImperviousR = 0.0;
	fileOrConstant stDataType = fileOrConstant::None;
	string fpnST = "";
	string fpnSTVat = ""; // 모델에서 직접 이용되지는 않는다. GUI에서 이용된다. 모델에서는 gmp 파일에 있는 매개변수 이용함
	double cnstSoilPorosity = 0.0;
	double cnstSoilEffPorosity = 0.0;
	double cnstSoilWFSH = 0.0;
	double cnstSoilHydraulicK = 0.0;
	fileOrConstant sdDataType = fileOrConstant::None;
	string fpnSD = "";
	string fpnSDVat = ""; // 모델에서 직접 이용되지는 않는다. GUI에서 이용된다. 모델에서는 gmp 파일에 있는 매개변수 이용함
	double cnstSoilDepth = 0.0;
	flowDirectionType fdType = flowDirectionType::None;

	weatherDataType rfDataType = weatherDataType::None;
	string fpnRainfallData = "";
	int rfinterval_min = -1;

	// continuous =====================
	weatherDataType tempMaxDataType = weatherDataType::None;
	string fpnTempMaxData = "";
	int tempMaxInterval_min = -1;
	weatherDataType tempMinDataType = weatherDataType::None;
	string fpnTempMinData = "";
	int tempMinInterval_min = -1;
	
	weatherDataType DTLDataType = weatherDataType::None;// day time length
	string fpnDTLData = "";// day time length
	int DTLDataInterval_min = -1; // day time length

	weatherDataType solarRadDataType = weatherDataType::None;
	string fpnSolarRadData = "";
	int solarRadInterval_min = -1;

	string fpnBlaneyCriddleK = "";

	// v2025 ===================================
	weatherDataType dewPointTempDataType = weatherDataType::None;
	string fpnDewPointTemp = "";
	int dewPointTempInterval_min = -1;

	weatherDataType windSpeedDataType = weatherDataType::None;
	string fpnWindSpeed = "";
	int windSpeedInterval_min = -1;

	weatherDataType userPETDataType = weatherDataType::None;
	string fpnUserPET = "";
	weatherDataType userAETDataType = weatherDataType::None;
	string fpnUserAET = "";
	int userETInterval_min = -1;
	// v2025 ===================================

	weatherDataType snowpackTempDataType = weatherDataType::None;
	string fpnSnowpackTempData = "";
	int snowpackTempInterval_min = -1;
	double latitudeTOP_degree = -9999.0;  //위도. 남반구에서는 - 값
	// continuous =====================

	int mdp = 0;
	string simStartTime = ""; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용. 실시간에서는 yyyymmddHHMM 포맷
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
	int makePETDistFile = 0;// true : 1, false : -1
	int makeAETDistFile = 0;// true : 1, false : -1

	GRMPrintType printOption = GRMPrintType::None;
	int printAveValue = 0;// true : 1, false : -1 // 출력기간 평균 값 출력 여부
	int dtPrintAveValue_min = 0;
	int dtPrintAveValue_sec = 0;
	string vSeparator = "\t";

	int forSimulation = 0;

	map <int, swsParameters> swps; // <wsid, paras>
	map <int, channelSettingInfo> css; //<wsid. paras>
	map <int, vector<flowControlinfo>> fcs; // <idx, paras>  // 같은 셀에 여러개의 FC 설정을 위해서 두번째를  vector<flowControlinfo>로 한다.
	vector <wpLocationRC> wps; // 
	vector <soilTextureInfo> sts;
	vector <soilDepthInfo> sds;
	vector <landCoverInfo> lcs;

	int isDateTimeFormat = 0;// true : 1, false : -1
	int isinletExist = 0;// true : 1, false : -1
	int demFileApplied = 0; // true : 1, false : -1
	int streamFileApplied = 0;
	int cwFileApplied = 0;
	int icfFileApplied = 0;
	int issrFileApplied = 0;
	int makeASCorIMGfile = 0;

	cpu_gpu_info cpusi; //MP 수정
	int deleteAllFilesExceptDischargeOut = -1;
	int writeLog = -1;
	
} projectFile;

typedef struct _thisSimulation
{
	int enforceFCautoROM = -1;// true : 1, false : -1
	int grmStarted = 0;
	int stopSim = 0;
	int dataNumTotal_rf = 0;
	int num_FA = 0; //값이 서로 다른 흐름누적수 값의 개수
	double rfAveForDT_m = 0;
	double rfAveSumAllCells_PdT_m = 0; // 출력기간 동안에서의 도메인 내에 있는 모든 셀 값의 합
	double rfAveSumAllCells_PdTave_m = 0; // 출력기간 동안에서의 도메인 내에 있는 모든 셀 값의 합
	double rfAveSumAllCells_PT_m_bak = 0; // 출력기간 동안에서의 도메인 내에 있는 모든 셀 값의 합 백업
	double rfAveSumAllCells_PTave_m_bak = 0; // 출력기간 동안에서의 도메인 내에 있는 모든 셀 값의 합 백업
	double rfiSumAllCellsInCurRFData_mPs; //rfi : rf intensity

	// 증발산, 융설 계산에 사용되는 파일 자료
	int wdUsed_tempMax = -1;// true : 1, false : -1
	int wdUsed_tempMin = -1;// true : 1, false : -1
	int wdUsed_DTL = -1;// true : 1, false : -1 // daytime length
	int wdUsed_solarR = -1;// true : 1, false : -1
	int wdUsed_dewPointTemp = -1;// true : 1, false : -1
	int wdUsed_windSpeed = -1;// true : 1, false : -1
	int wdUsed_userET = -1;// true : 1 이면 PET, 2이면 AET ; false : -1 
	int wdUsed_snowPackTemp = -1;// true : 1, false : -1
	int isUsed_BlaneyCriddleK_File = -1;
	int isUsed_DEM = -1;// true : 1, false : -1
	int isUsed_Latitude = -1;// true : 1, false : -1

	int dtsec = 0;
	int dtsecUsed_tm1 = 0;
	int dtMaxLimit_sec = 0;
	int dtMinLimit_sec = 0;
	int zeroTimePrinted = 0;
	int simEnding_sec = -1;
	int simDuration_min = 0;
	
	int tsec_tm1 = 0;
	int targetTtoP_sec = 0;
	int targetTtoP_AVE_sec = 0;
	int TtoP_ave_check_sec = 0;
	int cvsbT_sec = 0;
	int isbak = 0;

	int runByAnalyzer = 0;

	std::tm time_thisSimStarted; 
	double vMaxInThisStep=-9999.0; // m/s

	//int tCurYear = 0;
	int tCurMonth = 0;
	int tCurDay = 0;
	int tDayOfYear = 0;
	double laiRatioCurDay = 0.0;

	int showFileProgress = 0; // true : 1, false : -1
} thisSimulation;

typedef struct _globalVinner // 계산 루프로 전달하기 위한 최소한의 전역 변수. gpu 고려
{
	int mdp = 0;//-1일 경우는 최대 값으로 자동으로 설정, 1일 경우는 serial 계산
} globalVinner;

double calBFlowAndGetCSAaddedByBFlow(int i, 
	int dtsec, 	double cellSize_m);//i는 cv array index
void calBFLateralMovement(int i,
	int facMin, double dY_m, double dtsec);

void calChannelAUH(int i); // 하도 흐름에서 유속, 수심, 단면적 계산
void calChannelFlow(int i, double chCSACVw_tp1);
void calDetentionPond(int i, double nowTmin);
void calEffectiveRFbyInfiltration(int i, int dtrf_sec, int dtsec);
void calFCReservoirOutFlow(int i, double nowTmin); //i는 cv array index

void cal_h_SWC_OFbyAET(int i); // 증발산에 의한 지표면 수심과 토양 수분 계산
void cal_h_CHbyAET(int i);// 증발산에 의한 하도 수심 계산

void calinterception(int i);
void calET(int i);
void calSnowMelt(int i);

void calOverlandFlow(int i, double hCVw_tp1,
	double effDy_m);
void calPET_BlaneyCriddle(int i);
void calPET_Hamon(int i);
void calPET_FPM(int i);
void calPET_PriestleyTaylor(int i);
void calPET_Hargreaves(int i);
void calPET_userET(int i, int dataInterval_min, int whatUserData);

void calReservoirOperation(int i, double nowTmin);
void calReservoirOutFlowInReservoirOperation(int i,
	double Qout_cms, double dy_m);
double calRFlowAndSSFlow(int i,
	int dtsec, double dy_m); // 현재 cv의 Return flow는 상류에서 유입되는 ssflwo로 계산하고, 현재 cv에서의 ssf는 현재 셀의 수분함량으로 계산한다.
void calSinkOrSourceFlow(int i, double nowTmin, flowControlType fcType, int fcTypeOrder);
void calValuesDuringPT(int dtsec);
int checkWeatherDataByDomain();
int compareWeatherDataWithDomain(string fpn_in_wd, weatherDataType wdType,
	string dataString);
int compareASCwithDomain(string fpn_in, string dataString, 
	ascRasterHeader inHeader, int nCols, int nRows, double dx );
int checkWPpositions();


// reservoir outflow, reservoir operation 을 모두 AutoROM으로 강제로 변경
void convertFCtoAutoROM(string strDate, int cvid, int ifc); // 시간, cvid, flow control index

void disposeDynamicVars();
int deleteAllOutputFiles();

// 프로젝트 파일과 로그파일도 지워진다. 주의 필요
int deleteAllFilesExceptDischarge();
double getActualVP_Ea(int i); // 실제 증기압
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
flowDirection8 getFlowDirection(int fdirV, 
	flowDirectionType fdType);
double getMeanRFValueToPrintForAllCells(double cinterp); // 도메인 내에 있는 모든 셀의 평균 값 출력을 위한 인터폴레이션 값
double getMeanRFValueToPrintAveForAllCells(double cinterp);
double getOverlandFlowDepthCVw(int i);
projectfilePathInfo getProjectFileInfo(string fpn_prj);
double getPsychroConst(int i); // psychrometric constant
double getRa(double lat_rad); //extraterrestrial radiation MJ/m^2/day
double getRn(int i, double ea); // net radiation MJ/m^2/day, ea : actual 증기압
weatherDataType getWeatherDataTypeByDataFile(string fpn_wdata);
void grmHelp();

void updateCVbyHydroComps(int i); // i는 cv array index
int initOutputFiles();
void initRasterOutput();
void initThisSimulation();
void initRFvars();
void initValuesAfterPrinting(int nowTP_min, int printAveValueNow); //출력할 때 마다 초기화 되어야 하는 변수들 처리
int initWatershedNetwork();
int initWPinfos();
int isNormalChannelSettingInfo(channelSettingInfo *csi);
int isNormalFlowControlinfo(flowControlinfo *fci);
int isNormalSwsParameter(swsParameters *swp);
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
void makeIMG_pet();
void makeIMG_aet();

void makeASC_ssr();
void makeASC_rf();
void makeASC_rfacc();
void makeASC_flow();
void makeASC_pet();
void makeASC_aet();

int makeNewOutputFiles();
int makeRasterOutput(int nowTmin);

int openProjectFile(int forceRealTime);
int openPrjAndSetupModel(int forceRealTime);//1:true, -1:false
int outputManager(int nowTsec); // , int nowRForder);
vector<weatherData> readAndSetWeatherData(string fpn_in_wd, weatherDataType wdType,
	int dataInterval_min, string dataString);
int readDomainFaFileAndSetupCV();
int readDomainAddtionalRasterDataAndSetCV();
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
int readXmlRowWatchPoint(string aline, 
	wpLocationRC *wpl);
int readXmlRowSoilTextureInfo(string aline,
	soilTextureInfo* st);
int readXmlRowSoilDepth(string aline,
	soilDepthInfo* sd);
int readXmlRowLandCover(string aline,
	landCoverInfo* lc);

int setBlaneyCriddleK();
int setCVbyLCConstant();
int setCVbySTConstant();
int setCVbySDConstant();
int setCVRF(int order);
void setCVRFintensityAndDTrf_Zero();

int assignWDtoCV(int cvidx, whatWeatherData wwd, double wdValue);
int setCVweatherData(int order, string dataString, weatherDataType wdType,
	weatherData* vectWd, whatWeatherData wwd);

void setCVStartingCondition(double iniflow);
int setDomainAndCVBasicinfo();
int setFlowNetwork();
int setLAIRatio();

void setNoFluxCVCH(int i);
void setNoFluxCVOF(int i);

int setRainfallData();
int setRasterOutputArray();

// 포화증기압곡선의 기울기
//void setSVPGradient();
int setTemperatureMax();
int setTemperatureMin();
int setDaytimeLength();
int setSunDurationRatioData(); 
int setSnowPackTemp();
int setSolarRadiation();

int setDewPointTemp();
int setWindSpeed();
int setUserET(int dataType); //dataType : PET (1), AET (2)

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
	double cinterp, int writeAVE, int timeToP_AVE_min);
void writeDischargeFile(string tStrToPrint, 
	double cinterp); 
void writeDischargeAveFile(string tStrToPrint,
	double cinterp);
void writeFCOutputFile(string tStrToPrint,
	double cinterp);
void writeFCAveOutputFile(string tStrToPrint,
	double cinterp);
void writeWPoutputFile(string tStrToPrint,
	double cinterp); // 이거 출력은 유량 때문에 writeDischargeFile()에 종속되어 있다.
void writeRainfallOutputFile(string tStrToPrint,
	double cinterp);
void writeSimProgress(int elapsedT_min);

inline double  getinterpolatedVLinear(double firstV,
	double nextV, double cinterp);
inline double rfintensity_mPsec(double rf_mm, 
	double dtrf_sec);

inline void setNoInfiltrationParameters(int i);
inline void setWaterAreaInfiltrationPars(int i);
inline double soilSSRbyCumulF(double cumulinfiltration,
	double effSoilDepth);



inline double vByManningEq(double hydraulicRaidus,
	double slope, double nCoeff)
{
	return pow(hydraulicRaidus, 0.66667)* sqrt(slope) / nCoeff;
}

inline double hByManningEqForOF(double Q_cms,
	double nCoeff, double slope, double dy_m)
{
	return nCoeff * Q_cms / dy_m / pow(sqrt(slope), 0.6);  // 0.6 = 3/5;
}


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
	string FDtype = "";

	grmWSinfo(string fdirType, string fpnDEM, string fpnDomain,
		string slopeFPN, string fpnFdir, string fpnFac,
		string fpnStream = "", string fpnLandCover = "",
		string fpnSoilTexture = "", string fpnSoilDepth = "",
		string fpnIniSoilSaturationRatio = "",
		string pfnIniChannelFlow = "",
		string fpnChannelWidth = "");
	grmWSinfo(string gmpFPN);
	~grmWSinfo();

	bool isInWatershedArea(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	vector<int> upStreamWSIDs(int currentWSID);
	int upStreamWSCount(int currentWSID);
	vector<int> downStreamWSIDs(int currentWSID);
	int downStreamWSCount(int currentWSID);
	int watershedID(int colXAryidx, int rowYAryidx); // 배열 인덱스 사용
	string flowDirection(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	int flowAccumulation(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	double slope(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	int streamValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	string cellFlowType(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	int landCoverValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	int soilTextureValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	int soilDepthValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
	vector<string> allCellsInUpstreamArea(int colXAryidx, int rowYAryidx);
	int cellCountInUpstreamArea(int colXAryidx, //  Select all cells in upstream area of a input cell position. Return string list of cell positions - "column, row".
		int rowYAryidx);

	// Just update memory. The gmp file is not revised.
	// If you want to revise the gmp file, you need a writing process using updated parameters in memory.
	//  To get the updated paramters in memory for a subwatershed, you can use subwatershedPars() function.
	bool setOneSWSParsAndUpdateAllSWSUsingNetwork(int wsid, double iniSat, double iniLossPRCP_mm,
		double minSlopeLandSurface, unSaturatedKType unSKType, double coefUnsK,
		double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
		int dryStreamOrder, double ccLCRoughness,
		double ccPorosity, double ccWFSuctionHead, double ccSoilHydraulicCond,
		double ccSoilDepth,
		InterceptionMethod interceptMethod,
		PETmethod potentialETMethod, double etCoeff,
		SnowMeltMethod snowMeltMethod, double smeltTSR, double smeltingTemp,
		double snowCovRatio, double smeltCoef,
		double iniFlow = 0);
	void updateAllSubWatershedParametersUsingNetwork();

	// 입력파일로 인스턴싱 했을 경우에는 setOneSWSParsAndUpdateAllSWSUsingNetwork() 이거 호출 후에 사용 가능
	swsParameters subwatershedPars(int wsid);
	bool removeUserParametersSetting(int wsid);
};






