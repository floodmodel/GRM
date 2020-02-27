#pragma once

#include "gentle.h"

using namespace std;
namespace fs = std::filesystem;

//const string CONST_GMP_FILE_EXTENSION = ".gmp";


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

//typedef struct _swsParameters
//{
//	int wsid = 0;
//	double iniSaturation = 0.0;
//	double minSlopeOF = 0.0;
//	unSaturatedKType unSatKType= unSaturatedKType::None;
//	double coefUnsaturatedK = 0.0;
//	double minSlopeChBed = 0.0;
//	double minChBaseWidth = 0.0;
//	double chRoughness = 0.0;
//	int dryStreamOrder = 0;
//	double iniFlow = 0.0;
//	double ccLCRoughness = 0.0;
//	double ccPorosity = 0.0;
//	double ccWFSuctionHead = 0.0;
//	double ccHydraulicK = 0.0;
//	double ccSoilDepth = 0.0;
//	int userSet = 0;
//} swsParameters;

typedef struct _watchPointInfo
{
	string wpName = "";
	int wpColX = -1;
	int wpRowY = -1;
} watchPointInfo;

typedef struct _channelWidthInfo
{
	int mdWsid = 0;
	crossSectionType csType = crossSectionType::None;
	channelWidthType csWidthType= channelWidthType::None;
	double cwEQc = 0.0;
	double cwEQd = 0.0;
	double cwEQe = 0.0;
	double cwMostDownStream = 0.0;
	double lowRegionHeight = 0.0;
	double lowRegionBaseWidth = 0.0;
	double upRegionBaseWidth = 0.0;
	double compoundCSChannelWidthLimit = 0.0;
	double bankSlopeRight = 0.0;
	double bankSlopeLeft = 0.0;
} channelWidthInfo;

typedef struct _flowControlinfo
{
	string fcName = "";
	int fcColX = 0;
	int fcRowY = 0;
	flowControlType fcType = flowControlType::None;
	double fcDT = 0.0;
	string fcDataFile = "";
	double iniStorage = 0.0;
	double maxStorage = 0.0;
	double maxStorageR = 0.0;
	reservoirOperationType roType= reservoirOperationType::None;
	double roConstQ = 0.0;
	double roConstQDuration = 0.0;
} flowControlinfo;


typedef struct _soilTextureInfo
{
	int stGridValue;
	string stName = "";
	double porosity = 0.0;
	double effectivePorosity = 0.0;
	double wfSoilSuctionHead = 0.0;
	double hydraulicK = 0.0;
} soilTextureInfo;

typedef struct _soilDepthInfo
{
	int sdGridValue = 0;
	string sdName = "";
	double soilDepth = 0.0;
} soilDepthInfo;

typedef struct _landCoverInfo
{
	int lcGridValue = 0;
	string lcName = "";
	double RoughnessCoefficient = 0.0;
	double ImperviousRatio = 0.0;
} landCoverInfo;


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
	double cnstRoughnessC = 0.0;
	double cnstImperviousR = 0.0;
	fileOrConstant stDataType = fileOrConstant::None;
	string fpnST = "";
	string fpnSTVat = "";
	double cnstSoilPorosity = 0.0;
	double cnstSoilEffPorosity = 0.0;
	double cnstSoilWFSH = 0.0;
	double cnstSoilHydraulicK = 0.0;
	fileOrConstant sdDataType = fileOrConstant::None;
	string fpnSD = "";
	string fpnSDVat = "";
	double cnstSoilDepth = 0.0;
	rainfallDataType rfDataType= rainfallDataType::NoneRF;
	string RainfallDataFile = "";
	double rfinterval_min = 0.0;
	flowDirectionType fdType= flowDirectionType::None;
	int maxDegreeOfParallelism = 0;
	string SimulStartingTime = ""; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	double simDuration_hr = 0.0;
	double dtsec = 0.0;
	int IsFixedTimeStep = 0;// true : 1, false : -1
	double printTimeStep_min = 0.0;
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

	//vector <swsParameters> swps;
	vector<int> wsid ;
	vector<double> iniSaturation;
	vector<double> minSlopeOF;
	vector < unSaturatedKType> unSatKType;
	vector<double> coefUnsaturatedK;
	vector<double> minSlopeChBed;
	vector<double> minChBaseWidth;
	vector<double> chRoughness;
	vector<int> dryStreamOrder;
	vector<double> iniFlow;
	vector<double>ccLCRoughness;
	vector<double> ccPorosity;
	vector<double> ccWFSuctionHead;
	vector<double> ccHydraulicK;
	vector<double> ccSoilDepth;
	vector<int> userSet;// true : 1, false : -1

	vector <watchPointInfo> wps;
	vector <flowControlinfo> fcs;
	vector <channelWidthInfo> cws;
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
	const string LandCoverVatFile = "LandCoverVatFile";
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
	const string WSID = "WSID";
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
	const string  SingleCSChannelWidthType = "SingleCSChannelWidthType";
	const string  ChannelWidthEQc = "ChannelWidthEQc";
	const string  ChannelWidthEQd = "ChannelWidthEQd";
	const string  ChannelWidthEQe = "ChannelWidthEQe";
	const string  ChannelWidthMostDownStream = "ChannelWidthMostDownStream";
	const string  LowerRegionHeight = "LowerRegionHeight";
	const string LowerRegionBaseWidth = "LowerRegionBaseWidth";
	const string UpperRegionBaseWidth = "UpperRegionBaseWidth";
	const string CompoundCSChannelWidthLimit = "CompoundCSChannelWidthLimit";
	const string BankSideSlopeRight = "BankSideSlopeRight";
	const string BankSideSlopeLeft = "BankSideSlopeLeft";
	const string WPName = "WPName";
	const string WPColX = "WPColX";
	const string WPRowY = "WPRowY";
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


void disposeDynamicVars();
projectfilePathInfo getProjectFileInfo(string fpn_prj);
void grmHelp();
int openProjectFile();
int openPrjAndSetupModel();
int runGRM();
int startSingleEventRun();
