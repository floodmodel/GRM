#pragma once

#include "gentle.h"

using namespace std;
namespace fs = std::filesystem;

//const string CONST_GMP_FILE_EXTENSION = ".gmp";
const string CONST_TAG_DISCHARGE = "Discharge.out";
const string CONST_TAG_DEPTH = "Depth.out";
const string CONST_TAG_RFGRID = "RFGrid.out";
const string CONST_TAG_RFMEAN = "RFUpMean.out";
//const string CONST_TAG_FCAPP = "FCData.out";
const string CONST_TAG_FCSTORAGE = "FCStorage.out";
//const string CONST_TAG_SWSPARSTEXTFILE = "SWSPars.out";
const string CONST_DIST_SSR_DIRECTORY_TAG = "SSD";
const string CONST_DIST_RF_DIRECTORY_TAG = "RFD";
const string CONST_DIST_RFACC_DIRECTORY_TAG = "RFAccD";
const string CONST_DIST_FLOW_DIRECTORY_TAG = "FlowD";
const string CONST_DIST_SSR_FILE_HEAD = "ss_";
const string CONST_DIST_RF_FILE_HEAD = "rf_";
const string CONST_DIST_RFACC_FILE_HEAD = "rfc_";
const string CONST_DIST_FLOW_FILE_HEAD = "flow_";

const string CONST_OUTPUT_TABLE_TIME_FIELD_NAME = "DataTime";
const string CONST_OUTPUT_TABLE_MEAN_RAINFALL_FIELD_NAME = "Rainfall_Mean";


const double CONST_CFL_NUMBER = 1.0;

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
	DischargeFileQ,
	AllQ,
	None
};

enum class simulationType
{
	SingleEvent,
	SingleEventPE_SSR,
	RealTime,
	None
};

enum class unSaturatedKType
{
	Constant,
	Linear,
	Exponential,
	None
};

typedef struct _projectFileInfo
{
	string fpn_prj = "";
	//string fpn_log = "";
	string fp_prj = "";
	string fn_withoutExt_prj = "";
	string fn_prj = "";
	fs::file_time_type prjfileSavedTime;
} projectfilePathInfo;

typedef struct _swsParameters
{
	int wsid = -1;
	double iniSaturation =-1.0;
	double minSlopeOF = -1.0;
	unSaturatedKType unSatKType= unSaturatedKType::None;
	double coefUnsaturatedK = -1.0;
	double minSlopeChBed = -1.0;
	double minChBaseWidth = -1.0;
	double chRoughness = -1.0;
	int dryStreamOrder = -1;
	double iniFlow = -1.0;
	double ccLCRoughness = -1.0;
	double ccPorosity = -1.0;
	double ccWFSuctionHead = -1.0;
	double ccHydraulicK = -1.0;
	double ccSoilDepth = -1.0;
	int userSet = 0;
} swsParameters;

typedef struct _watchPointInfo
{
	string wpName = "";
	int wpColX = -1;
	int wpRowY = -1;
} watchPointInfo;

typedef struct _channelSettingInfo
{
	int mdWsid = -1;
	crossSectionType csType = crossSectionType::None;
	channelWidthType csWidthType= channelWidthType::None;
	double cwEQc = -1.0;
	double cwEQd = -1.0;
	double cwEQe = -1.0;
	double cwMostDownStream = -1.0;
	double lowRegionHeight = -1.0;
	double lowRegionBaseWidth = -1.0;
	double upRegionBaseWidth = -1.0;
	double compoundCSChannelWidthLimit = -1.0;
	double bankSlopeRight = -1.0;
	double bankSlopeLeft = -1.0;
} channelSettingInfo;

typedef struct _flowControlinfo
{
	string fcName = "";
	int fcColX = -1;
	int fcRowY = -1;
	flowControlType fcType = flowControlType::None;
	double fcDT = 0.0;
	string fcDataFile = "";
	double iniStorage = -1.0;
	double maxStorage = -1.0;
	double maxStorageR = -1.0;
	reservoirOperationType roType= reservoirOperationType::None;
	double roConstQ = -1.0;
	double roConstQDuration = -1.0;
} flowControlinfo;


typedef struct _soilTextureInfo
{
	int stGridValue=-1;
	string stName = "";
	double porosity = -1.0;
	double effectivePorosity = -1.0;
	double WFSuctionHead = -1.0;
	double hydraulicK = -1.0;
} soilTextureInfo;

typedef struct _soilDepthInfo
{
	int sdGridValue = -1;
	string sdName = "";
	double soilDepth = -1.0;
} soilDepthInfo;

typedef struct _landCoverInfo
{
	int lcGridValue = -1;
	string lcName = "";
	double RoughnessCoefficient = -1;
	double ImperviousRatio = -1;
} landCoverInfo;

typedef struct _grmOutFiles
{
	string ofnpDischarge;
	string ofpnDepth;
	string ofpnRFGrid;
	string ofpnRFMean;
	//string OFNPFCData;
	string OFNPFCStorage;
	//string OFNPSwsPars;
	string OFPSSRDistribution;
	string OFPRFDistribution;
	string OFPRFAccDistribution;
	string OFPFlowDistribution;
} grmOutFiles;


typedef struct _projectFile
{
	simulationType simType = simulationType::None;
	string	fpnDomain = "";
	string fpnSlope = "";
	string fpnFD = "";
	string fpnFA = "";
	string fpnStream = "";
	string fpnChannelWidth = "";
	string fpnIniSSR = "";
	string fnpIniChannelFlow = "";
	fileOrConstant lcDataType = fileOrConstant::None;
	string fpnLC = "";
	string fpnLCVat = "";
	double cnstRoughnessC = -1.0;
	double cnstImperviousR = -1.0;
	fileOrConstant stDataType = fileOrConstant::None;
	string fpnST = "";
	string fpnSTVat = "";
	double cnstSoilPorosity = -1.0;
	double cnstSoilEffPorosity = -1.0;
	double cnstSoilWFSH = -1.0;
	double cnstSoilHydraulicK = -1;
	fileOrConstant sdDataType = fileOrConstant::None;
	string fpnSD = "";
	string fpnSDVat = "";
	double cnstSoilDepth = -1.0;
	rainfallDataType rfDataType = rainfallDataType::NoneRF;
	string fpnRainfallData = "";
	double rfinterval_min = -1.0;
	flowDirectionType fdType = flowDirectionType::None;
	int maxDegreeOfParallelism = 0;
	string simulStartingTime = ""; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	double simDuration_hr = -1.0;
	double dtsec = -1.0;
	int IsFixedTimeStep = 0;// true : 1, false : -1
	double printTimeStep_min = -1;
	int simInfiltration = 0;// true : 1, false : -1
	int simSubsurfaceFlow = 0;// true : 1, false : -1
	int simBaseFlow = 0;// true : 1, false : -1
	int simFlowControl = 0;// true : 1, false : -1

	int makeIMGFile = 0;// true : 1, false : -1
	int makeASCFile = 0;// true : 1, false : -1
	int makeSoilSaturationDistFile = 0;// true : 1, false : -1
	int makeRfDistFile = 0;// true : 1, false : -1
	int makeRFaccDistFile = 0;// true : 1, false : -1
	int makeFlowDistFile = 0;// true : 1, false : -1
	GRMPrintType printOption = GRMPrintType::None;
	int writeLog = 0;// true : 1, false : -1

	vector <swsParameters> swps;
	vector <channelSettingInfo> css;
	vector <flowControlinfo> fcs;
	vector <watchPointInfo> wps;
	vector <soilTextureInfo> sts;
	vector <soilDepthInfo> sds;
	vector <landCoverInfo> lcs;

	int isDateTimeFormat = 0;// true : 1, false : -1

	CPUsInfo cpusi;
	int deleteAllFilesExceptDischargeOut = -1;

} projectFile;

typedef struct _projectFileFieldName
{
	const string GRMSimulationType = "GRMSimulationType";
	const string DomainFile = "DomainFile";
	const string SlopeFile = "SlopeFile";
	const string FlowDirectionFile = "FlowDirectionFile";
	const string FlowAccumFile = "FlowAccumFile";
	const string StreamFile = "StreamFile";
	const string ChannelWidthFile = "ChannelWidthFile";
	const string InitialSoilSaturationRatioFile = "InitialSoilSaturationRatioFile";
	const string InitialChannelFlowFile = "InitialChannelFlowFile";
	const string LandCoverDataType = "LandCoverDataType";
	const string LandCoverFile = "LandCoverFile";
	const string LandCoverVATFile = "LandCoverVATFile";
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
	const string RainfallDataType = "RainfallDataType";
	const string RainfallDataFile = "RainfallDataFile";
	const string RainfallInterval = "RainfallInterval_min";
	const string FlowDirectionType = "FlowDirectionType";
	const string MaxDegreeOfParallelism = "MaxDegreeOfParallelism";
	const string SimulStartingTime = "SimulStartingTime"; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	const string SimulationDuration = "SimulationDuration_hr";
	const string ComputationalTimeStep = "ComputationalTimeStep";
	const string IsFixedTimeStep = "IsFixedTimeStep";
	const string OutputTimeStep = "OutputTimeStep";
	const string SimulateInfiltration = "SimulateInfiltration";
	const string SimulateSubsurfaceFlow = "SimulateSubsurfaceFlow";
	const string SimulateBaseFlow = "SimulateBaseFlow";
	const string SimulateFlowControl = "SimulateFlowControl";
	const string MakeIMGFile = "MakeIMGFile";
	const string MakeASCFile = "MakeASCFile";
	const string MakeSoilSaturationDistFile = "MakeSoilSaturationDistFile";
	const string MakeRfDistFile = "MakeRfDistFile";
	const string MakeRFaccDistFile = "MakeRFaccDistFile";
	const string MakeFlowDistFile = "MakeFlowDistFile";
	const string PrintOption = "PrintOption";
	const string WriteLog = "WriteLog";
	const string SWSID = "SWSID";
	const string IniSaturation = "IniSaturation";
	const string MinSlopeOF = "MinSlopeOF";
	const string UnsaturatedKType = "UnsaturatedKType";
	const string CoefUnsaturatedK = "CoefUnsaturatedK";
	const string MinSlopeChBed = "MinSlopeChBed";
	const string MinChBaseWidth = "MinChBaseWidth";
	const string ChRoughness = "ChRoughness";
	const string DryStreamOrder = "DryStreamOrder";
	const string IniFlow = "IniFlow";
	const string CalCoefLCRoughness = "CalCoefLCRoughness";
	const string CalCoefPorosity = "CalCoefPorosity";
	const string CalCoefWFSuctionHead = "CalCoefWFSuctionHead";
	const string CalCoefHydraulicK = "CalCoefHydraulicK";
	const string CalCoefSoilDepth = "CalCoefSoilDepth";
	const string UserSet = "UserSet";
	const string MDWSID = "MDWSID";
	const string CrossSectionType = "CrossSectionType";
	const string SingleCSChannelWidthType = "SingleCSChannelWidthType";
	const string ChannelWidthEQc = "ChannelWidthEQc";
	const string ChannelWidthEQd = "ChannelWidthEQd";
	const string ChannelWidthEQe = "ChannelWidthEQe";
	const string ChannelWidthMostDownStream = "ChannelWidthMostDownStream";
	const string LowerRegionHeight = "LowerRegionHeight";
	const string LowerRegionBaseWidth = "LowerRegionBaseWidth";
	const string UpperRegionBaseWidth = "UpperRegionBaseWidth";
	const string CompoundCSChannelWidthLimit = "CompoundCSChannelWidthLimit";
	const string BankSideSlopeRight = "BankSideSlopeRight";
	const string BankSideSlopeLeft = "BankSideSlopeLeft";
	const string FCName = "FCName";
	const string FCColX = "FCColX";
	const string FCRowY = "FCRowY";
	const string ControlType = "ControlType";
	const string FCDT = "FCDT";
	const string FlowDataFile = "FlowDataFile";
	const string IniStorage = "IniStorage";
	const string MaxStorage = "MaxStorage";
	const string MaxStorageR = "MaxStorageR";
	const string ROType = "ROType";
	const string ROConstQ = "ROConstQ";
	const string ROConstQDuration = "ROConstQDuration";
	const string WPName = "WPName";
	const string WPColX = "WPColX";
	const string WPRowY = "WPRowY";
	const string STGridValue = "STGridValue";
	const string GRMCodeST = "GRMCodeST";
	const string STPorosity = "STPorosity";
	const string STEffectivePorosity = "STEffectivePorosity";
	const string STWFSuctionHead = "STWFSuctionHead";
	const string STHydraulicConductivity = "STHydraulicConductivity";
	const string SDGridValue = "SDGridValue";
	const string GRMCodeSD = "GRMCodeSD";
	const string SDSoilDepth = "SDSoilDepth";
	const string LCGridValue = "LCGridValue";
	const string GRMCodeLC = "GRMCodeLC";
	const string LCRoughnessCoeff = "LCRoughnessCoeff";
	const string ImperviousRatio = "LCImperviousR";
} projectFileFieldName;

typedef struct _rainfallData
{
	int Order=-1;
	string DataTime="";
	string Rainfall = ""; // map 에서는 강우량 값, asc에서는 파일 이름
	string FilePath = "";
	string FileName = "";
} rainfallData;

void disposeDynamicVars();

projectfilePathInfo getProjectFileInfo(string fpn_prj);
int setRainfallData();
int getSwpsVectorIndex(int wsid);
void grmHelp();

int initOutputFiles();
int isNormalChannelSettingInfo(channelSettingInfo aci);
int isNormalFlowControlinfo(flowControlinfo afc);
int isNormalSwsParameter(swsParameters assp);
int isNormalWatchPointInfo(watchPointInfo awp);
int isNormalSoilTextureInfo(soilTextureInfo ast);
int isNormalSoilDepthInfo(soilDepthInfo asd);
int isNormalLandCoverInfo(landCoverInfo alc);

channelSettingInfo nullChannelSettingInfo();
flowControlinfo nullFlowControlinfo();
swsParameters nullSwsParameters();
watchPointInfo nullWatchPointInfo();
soilTextureInfo nullSoilTextureInfo();
soilDepthInfo nullSoilDepthInfo();
landCoverInfo nullLandCoverInfo();

int openProjectFile();
int openPrjAndSetupModel(int forceRealTime);//1:true, -1:false
int runGRM();
int startSingleEventRun();


