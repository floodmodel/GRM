#pragma once

#include "gentle.h"

using namespace std;
namespace fs = std::filesystem;

//const string CONST_GMP_FILE_EXTENSION = ".gmp";
const string CONST_TAG_DISCHARGE = "Discharge.out";
const string CONST_TAG_DEPTH = "Depth.out";
const string CONST_TAG_RFGRID = "RFGrid.out";
const string CONST_TAG_RFMEAN = "RFUpMean.out";
const string CONST_TAG_FCDATA_APP = "FCData.out";
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

const double CONST_MIN_SLOPE = 0.000001;
const double CONST_CFL_NUMBER = 1.0;
const double CONST_EXPONENTIAL_NUMBER_UNSATURATED_K = 6.4;
const double CONST_WEDGE_FLOW_COEFF = 1; // 최상류 셀의 쐐기 흐름 계산시 p의 수심에 곱해지는 계수
const double CONST_WET_AND_DRY_CRITERIA = 0.000001;
const double CONST_TOLERANCE = 0.001;
const double CONST_DEPTH_TO_BEDROCK = 20;// 암반까지의 깊이[m]
const double CONST_DEPTH_TO_BEDROCK_FOR_MOUNTAIN = 10;// 산악지역에서의 암반까지의 깊이[m]
const double CONST_DEPTH_TO_UNCONFINED_GROUNDWATERTABEL = 10;// 비피압대수층까지의 깊이[m]
const double CONST_UAQ_HEIGHT_FROM_BEDROCK = 5;//   암반에서 비피압대수층 상단까지의 높이[m]



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
	M,
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
	const string RainfallInterval_min = "RainfallInterval_min";
	const string FlowDirectionType = "FlowDirectionType";
	const string MaxDegreeOfParallelism = "MaxDegreeOfParallelism";
	const string SimulStartingTime = "SimulStartingTime"; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	const string SimulationDuration_hr = "SimulationDuration_hr";
	const string ComputationalTimeStep_min = "ComputationalTimeStep_min";
	const string IsFixedTimeStep = "IsFixedTimeStep";
	const string OutputTimeStep_min = "OutputTimeStep_min";
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
	const string FCDT_min = "FCDT_min";
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
	const string LCImperviousR = "LCImperviousR";
} projectFileFieldName;

typedef struct _timeSeries
{
	string dataTime; // 분단위
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

typedef struct _swsParameters
{
	int wsid = -1;
	double iniSaturation =0.0;
	double minSlopeOF = 0.0;
	unSaturatedKType unSatKType= unSaturatedKType::None;
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
	int userSet = 0;
} swsParameters;


typedef struct _wsNetwork
{
	map <int, vector<int>> wsidsNearbyUp; //상류는 여러개일 수 있다.
	//map <int, vector<int>> wsidsNearbyDown; 
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

typedef struct _wpinfo {
	vector<int> wpCVidxes;
	map<int, string> wpNames; //<idx, wpname>
	//<idx, value>
	map<int, double> rfiReadSumUpWS_mPs;// 현재 wp 상류에 대해 원시자료에서 읽은 강우량(강우강도 rfi).[m/s] 
	map<int, double> rfUpWSAveForDt_mm; // 현재 wp 상류에 대해 dt(계산시간 간격) 동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
	map<int, double> rfUpWSAveForDtP_mm;// 현재 wp 상류에 대해 출력시간 간격) 동안의 평균강우량. 원시자료를 이용해서 계산된값.[mm]
	map<int, double> rfUpWSAveTotal_mm;//현재 watch point 상류의 평균강우량의 누적값[mm]
	map<int, double> rfWPGridForDtP_mm; // Watchpoint 격자에 대한 출력시간 간격 동안의 누적강우량. 원시자료를 이용해서 계산된값.[mm]
	map<int, double> rfWPGridTotal_mm; // Watchpoint 격자에 대한 누적강우량[mm]
	map<int, double> totalFlow_cms; // Watchpoint 격자에 대한 전체유량[cms]. 누적값.
	//Todo : 사용되지 않으면, 삭제
	map<int, double> totalDepth_m; // Watchpoint 격자에 총유출고[m] 
	map<int, double> maxFlow_cms; // Watchpoint 격자에 대한 최대유량[cms]
	map<int, double> maxDepth_m; // Watchpoint 격자에 대한 최고수심[m]
	map<int, string> maxFlowTime; // Watchpoint 격자에 대한 최대유량 시간. 첨두시간.
	map<int, string> maxDepthTime; // Watchpoint 격자에 대한 최고수심 시간. 첨두시간
	map<int, double> qFromFCData_cms; // 해당 wp에서 Flow control에 의해서 계산되는 유량
	map<int, double> qprint_cms;
	//map<int, string> fpnWpOut; // Watch point 별 모의결과 출력을 위한 파일 이름 저장
	map<int, int>cvCountAllup;
} wpinfo;

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
	double iniStorage_m3 = 0.0;
	double maxStorage_m3 = 0.0;
	double maxStorageR = 0.0;
	reservoirOperationType roType= reservoirOperationType::None;
	double roConstQ_cms = 0.0;
	double roConstQDuration_hr = 0.0;
} flowControlinfo;

typedef struct _flowControlCellAndData
{
	map <int, double> fcDataAppliedNowT_m3Ps;// <idx, value>현재의 모델링 시간(t)에 적용된 flow control data 값
	vector<int> cvidxsinlet;
	vector<int> cvidxsFCcell;
	map<int, vector<timeSeries>> flowData_m3Ps; //<idx, data>, 분단위
	map<int, int> curDorder;// <idx, order>현재 적용될 데이터의 순서
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
} landCoverInfo;

typedef struct _rainfallData
{
	int Order = -1;
	string DataTime = "";
	string Rainfall = ""; // map 에서는 강우량 값, asc에서는 파일 이름
	string FilePath = "";
	string FileName = "";
} rainfallData;

typedef struct _grmOutFiles
{
	string ofpnDischarge;
	string ofpnDepth;
	string ofpnRFGrid;
	string ofpnRFMean;
	string ofpnFCData;
	string ofpnFCStorage;
	//string OFNPSwsPars;
	string ofpSSRDistribution;
	string ofpRFDistribution;
	string ofpRFAccDistribution;
	string ofpFlowDistribution;
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
	int facMin = 0;
	vector <int> dmids;
	map <int, vector<int>> cvidxInEachRegion;
	wsNetwork wsn;
} domaininfo;

typedef struct _cvStreamAtt
{
	double chBedSlope = 0.0;//하도셀과 하도+지표면 흐름 속성을 가지는 격자에 부여되는 하천경사(m/m)
	int chStrOrder = -1;//하천차수
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

typedef struct _cvAtt
{
	int idx_xr;
	int idx_yc;
	int wsid = -1; //유역 ID, 해당 raster cell의 값
	cellFlowType flowType;//셀의 종류, 지표면흐름, 하도흐름, 지표면+하도
	double slopeOF = 0.0; //지표면 해석에 적용되는 overland flow 셀의 경사(m/m)
	double slope = 0.0; //셀의 경사(m/m)
	flowDirection8 fdir;//흐름방향
	int fac = -1;//흐름누적수, 자신의 셀을 제외하고, 상류에 있는 격자 개수
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
	double hOF_ori = 0.0;//t 시간에서 유출해석 시작 전 overland flow 검사체적의 수심
	double uOF = 0.0;//t 시간에서 유출해석 결과 overland flow 검사체적의 유속
	double hOF = 0.0;  //t 시간에서 유출해석 결과 overland flow 검사체적의 수심
	double csaOF = 0.0;//t 시간에서 유출해석 결과 overland flow의 흐름 단면적
	double qOF_m2Ps = 0.0; //단위폭당 overland flow 유량
	double QOF_m3Ps = 0.0;//t 시간에서의 유출해석 결과 overland flow의 유량 [m^3/s]
	double QSSF_m3Ps = 0.0;//t 시간에서의 현재 셀에서 다음셀로 지표하에서 유출되는 유량 [m^3/s]
	double QsumCVw_dt_m3 = 0.0;//상류인접 CV에서 현재 CV로 유입되는 유량 단순합. 이건 CVi에서의 연속방정식, 고려하지 않은 단순 합.[m^3/dt]
	double rfApp_dt_m = 0.0;//dt 시간 동안의 강우량
	double rfiRead_mPsec = 0.0;//현재 강우입력자료에서 읽은 강우강도 m/s rfi.
	double rfiRead_tm1_mPsec = 0.0;//이전 시간의 강우강도 m/s rfi.
	double rfEff_dt_m = 0.0;//dt시간 동안의 유효강우량
	double rf_dtPrint_m = 0.0;//출력 시간간격 동안의 누적 강우량[m]
	double rfAcc_fromStart_m = 0.0;//전체 기간의 누적 강우량[m]
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
	double hUAQfromChannelBed_m = 0.0; //하도셀에서 비피압대수층의 수심(하도바닥에서의 높이)[m].
	double hUAQfromBedrock_m = 0.0;///암반으로부터 비피압대수층의 상단부까지의 깊이[m]. 토양깊이.
	double sdToBedrock_m = 0.0;//현재 CV 토양의 암반까지의 깊이[m]. 지표면에서 암반까지의 깊이임.
	double bfQ_m3Ps = 0.0;//현재 CV의 기저유출량 [m^3/s]
	landCoverCode lcCode;
	int lcCellValue = -1;//토지피복레이어의 값, VAT 참조 // 0 값은 상수를 의미하게 한다.
	double imperviousR = 0.0;//현재 CV 토지피복의 불투수율. 무차원, 0~1.
	double Interception_m = 0.0;//현재 CV 토지피복에서의 차단량 [m].
	double rcOF = 0.0;//현재 CV 토지피복의 모델링 적용 지표면 조도계수
	double rcOFori = 0.0;//현재 CV 토지피복의 grm default 지표면 조도계수
	flowControlType fcType=flowControlType::None;//현재 CV에 부여된 Flow control 종류
	double storageCumulative_m3 = 0.0;//현재 CV에서 flow control 모의시 누적 저류량[m^3]
	double storageAddedForDTbyRF_m3 = 0.0;//현재 CV에서 flow control 모의시 dt 시간동안의 강우에 의해서 추가되는 저류량[m^3/dt]
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
	string fpniniChannelFlow = "";
	fileOrConstant lcDataType = fileOrConstant::None;
	string fpnLC = "";
	string fpnLCVat = ""; // 모델에서 직접 이용되지는 않는다. GUI에서 이용된다. 모델에서는 gmp 파일에 있는 매개변수 이용함
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
	rainfallDataType rfDataType = rainfallDataType::NoneRF;
	string fpnRainfallData = "";
	int rfinterval_min = -1;
	flowDirectionType fdType = flowDirectionType::None;
	int mdp = 0;
	string simStartTime = ""; // 년월일의 입력 포맷은  2017-11-28 23:10 으로 사용
	double simDuration_hr = 0.0;
	int dtsec = 0;
	int IsFixedTimeStep = 0;// true : 1, false : -1
	int dtPrint_min = 0;
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

	map <int, swsParameters> swps; // <wsid, paras>
	map <int, channelSettingInfo> css; //<wsid. paras>
	map <int, flowControlinfo> fcs; // <idx, paras>
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

	CPUsInfo cpusi;
	int deleteAllFilesExceptDischargeOut = -1;
	
} projectFile;




typedef struct _thisSimulation
{
	int setupGRMisNormal = 0; // Todo : 필요여부 확인 필요.
	int grmStarted = 0;
	int stopSim = 0;
	int rfDataCountTotal = -1;
	double rfAveForDT_m = 0;
	double rfAveSumAllCells_dtP_m = 0;
	double rfiSumAllCellsInCurRFData_mPs; //rfi : rf intensity
	int dtsec = 0;
	int dtsecUsed_tm1 = 0;
	int dtMaxLimit_sec = 0;
	int dtMinLimit_sec = 0;
	int zeroTimePrinted = 0;
	int time_simEnding_sec = -1;

	int tsec_tm1 = 0;
	int targetTtoP_sec = 0;
	//int iscvsb = -1; // 이전시간에서의 cvs가 백업되어 있는지 여부
	int cvsbT_sec = 0;
	int isbak = 0;

	int runByAnalyzer = 0;

	tm g_RT_tStart_from_MonitorEXE;
	COleDateTime time_thisSimStarted;

	double vMaxInThisStep;
	
} thisSimulation;


typedef struct _globalVinner // 계산 루프로 전달하기 위한 최소한의 전역 변수. gpu 고려
{
	int mdp = 0;//-1일 경우는 최대 값으로 자동으로 설정, 1일 경우는 serial 계산
} globalVinner;


double calBFlowAndGetCSAaddedByBFlow(int i, 
	int dtsec, 	double cellSize_m);//i는 cv array index
double calRFlowAndSSFlow(int i,
	int dtsec, double dy_m); // 현재 cv의 Return flow는 상류에서 유입되는 ssflwo로 계산하고, 현재 cv에서의 ssf는 현재 셀의 수분함량으로 계산한다.
void calBFLateralMovement(int i,
	int facMin, double dY_m, double dtsec);
void calChannelFlow(int i, double chCSACVw_tp1);
void calCumulRFduringDTP(int dtsec);
void calFCReservoirOutFlow(int i, double nowTmin); //i는 cv array index
void calEffectiveRainfall(int i, int dtrf_sec, int dtsec);
void calOverlandFlow(int i, double hCVw_tp1,
	double effDy_m);
void calReservoirOperation(int i, double nowTmin);
void calReservoirOutFlowInReservoirOperation(int i,
	double Qout_cms, double dy_m);
void calSinkOrSourceFlow(int i, double nowTmin);

void disposeDynamicVars();
int deleteAllOutputFiles();
int deleteAllFilesExceptDischarge();

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

int getDTsec(double cfln, double dx, 
	double vMax, int dtMax_min, 
	int dtMin_min);
double getinfiltrationForDtAfterPonding(int i, int dtSEC,
	double CONSTGreenAmpt, double Kapp);
projectfilePathInfo getProjectFileInfo(string fpn_prj);
flowDirection8 getFlowDirection(int fdirV, 
	flowDirectionType fdType);
double getOverlandFlowDepthCVw(int i);
void grmHelp();

void updatetCVbyRFandSoil(int i); // i는 cv array index
int initOutputFiles();
void initRasterOutput();
void initThisSimulation();
int initWatershedNetwork();
int initWPinfos();
int isNormalChannelSettingInfo(channelSettingInfo aci);
int isNormalFlowControlinfo(flowControlinfo afc);
int isNormalSwsParameter(swsParameters assp);
int isNormalWatchPointInfo(wpLocationRC awp);
int isNormalSoilTextureInfo(soilTextureInfo ast);
int isNormalSoilDepthInfo(soilDepthInfo asd);
int isNormalLandCoverInfo(landCoverInfo alc);

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
void makeRasterOutput(int nowTmin);

channelSettingInfo nullChannelSettingInfo();
flowControlinfo nullFlowControlinfo();
swsParameters nullSwsParameters();
wpLocationRC nullWatchPointInfo();
soilTextureInfo nullSoilTextureInfo();
soilDepthInfo nullSoilDepthInfo();
landCoverInfo nullLandCoverInfo();

int openProjectFile(int forceRealTime);
int openPrjAndSetupModel(int forceRealTime);//1:true, -1:false
void outputManager(int nowTsec,
	int nowRForder);

int readDomainFileAndSetupCV();
int readSlopeFdirFacStreamCwCfSsrFileAndSetCV();
int readLandCoverFileAndSetCVbyVAT();
int readSoilTextureFileAndSetCVbyVAT();
int readSoilDepthFileAndSetCVbyVAT();

int setDomainAndCVBasicinfo();
int setCVbyLCConstant();
int setCVbySTConstant();
int setCVbySDConstant();
int setCVRF(int order);
void setCVStartingCondition(double iniflow);
int setFlowNetwork();
int setRainfallData();
void setRFintensityAndDTrf_Zero();
int setRasterOutputArray();
int setupByFAandNetwork();
int setupModelAfterOpenProjectFile();
int simulateSingleEvent();
int simulateRunoff(double nowTmin);
void simulateRunoffCore(int i, double nowTmin);
int startSimulationSingleEvent();

double totalSSFfromCVwOFcell_m3Ps(int i);

int updateWatershedNetwork();
int updateCVbyUserSettings();
int updateFCCellinfoAndData();

void writeBySimType(int nowTP_min,
	double cinterp);
void writeDischargeOnly(double cinterp, 
	int writeWPfiles);
void writeSimStep(int elapsedT_min);
void writeSingleEvent(int nowTmin, 
	double interCoef);
void writeWPouput(string nowTP, 
	int i, double cinterp);

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




// extern C
int readLandCoverFile(string fpnLC, 
	int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY);
int readSoilTextureFile(string fpnST, 
	int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY);
int readSoilDepthFile(string fpnSD, 
	int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY);


