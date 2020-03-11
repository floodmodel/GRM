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

const double CONST_MIN_SLOPE = 0.000001;
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


typedef struct _wsNetwork
{
	map <int, vector<int>> wsidsNearbyUp; //상류는 여러개일 수 있다.
	//map <int, vector<int>> wsidsNearbyDown; 
	map <int, int> wsidNearbyDown; //하류는 하나다
	map <int, vector<int>> wsidsAllUp;
	map <int, vector<int>> wsidsAllDown;
	vector <int> mdWSIDs;
	map <int, int> wsOutletCVID;
	map <int, int> mdWSIDofCurrentWS;
} wsNetwork;

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
	double fcDT_min = 0.0;
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
	soilTextureCode stCode = soilTextureCode::None;
	double porosity = -1.0;
	double effectivePorosity = -1.0;
	double WFSuctionHead = -1.0;
	double hydraulicK = -1.0;
} soilTextureInfo;

typedef struct _soilDepthInfo
{
	int sdGridValue = -1;
	soilDepthCode sdCode = soilDepthCode::None;
	double soilDepth = -1.0;
} soilDepthInfo;

typedef struct _landCoverInfo
{
	int lcGridValue = -1;
	landCoverCode lcCode = landCoverCode::None;
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

	int cvanMaxFac;//fac가 가장 큰 셀(최하류셀 등)의 1차원 배열 번호
	int cellCountNotNull = 0;
	int facMostUpChannelCell = -1;
	int facMax = -1;
	int facMin = -1;
	vector <int> dmids;
	map <int, vector<int>> cvidsInEachRegion;
	wsNetwork wsn;
} domaininfo;


typedef struct _cvStreamAtt
{
	double chBedSlope = -1.0;//하도셀과 하도+지표면 흐름 속성을 가지는 격자에 부여되는 하천경사(m/m)
	int chStrOrder = -1;//하천차수
	double chBaseWidth = -1.0;//하천의 바닥폭[m]
	double chBaseWidthByLayer = -1.0;//하폭레이어에서 받은 하폭[m]
	double roughnessCoeffCH = -1.0;//현재의 channel CV의 하도조도계수
	double csaCh_ori = -1.0;//t 시간에서의 유출해석 전의 현재 channel CV의 초기 흐름단면적[m^2]
	double csaCh = -1.0;//t 시간에서의 유출해석 결과 현재 channel CV의 흐름단면적[m^2]
	double csaChAddedByOFinCHnOFcell = -1.0;//하도+지표면 흐름셀에서, 지표면 흐름에 의한 하도흐름 단면적 증가분
	double hCH_ori = -1.0;//t 시간에서의 유출해석 전의 현재 channel CV의 초기 수심[m]
	double hCH = -1.0;//t 시간에서의 유출해석 결과 현재 channel CV의 수심[m]
	double uCH = -1.0; //t 시간에서의 유출해석 결과 현재 channel CV의 유속[m/s]
	double QCH_m3Ps = -1.0;//t 시간에서의 유출해석 결과 현재 channel CV의 유량[m^3/s]
	double iniQCH_m3Ps = -1.0;//하천셀에서의 초기 유량을 파일로 받을때 설정되는 값[m^3/s]
	double chSideSlopeLeft = -1.0;//현재의 channel CV의 좌측 제방 경사
	double chSideSlopeRight = -1.0;//현재의 channel CV의 우측 제방 경사
	double chBankCoeff = -1.0;// 현재의 channel CV의 제방 계수. 계산 편의를 위해서 channel CV 별로 미리계산한 값
	double chUpperRBaseWidth_m = -1.0;//현재 channel CV의 복단면 고수부지 바닥 폭[m]
	double chLowerRHeight = -1.0;// 현재 channel CV의 복단면 고수부지의 수심[m]
	int isCompoundCS = -1;//현재의 channel CV가 복단면인지(true), 단단면(false)인지를 나타내는 변수
	double chLowerRArea_m2 = -1.0;// 복단면 channel 중 하층부의 면적[m^2]
} cvStreamAtt;

typedef struct _cvAtt
{
	//int x;// cellidx 검증용
	//int y;// cellidx 검증용
	int cvid = -1;//유역에 해당하는 셀에만 부여되는 일련번호    
	int wsid = -1; //유역 ID, 해당 raster cell의 값
	cellFlowType flowType;//셀의 종류, 지표면흐름, 하도흐름, 지표면+하도
	double slopeOF = -1.0; //지표면 해석에 적용되는 overland flow 셀의 경사(m/m)
	double slope = -1.0; //셀의 경사(m/m)
	flowDirection8 fdir;//흐름방향
	int fac = -1;//흐름누적수, 자신의 셀을 제외하고, 상류에 있는 격자 개수
	double dxDownHalf_m = -1.0;//격자 중심으로부터 하류방향 격자면까지의 거리
	double dxWSum = -1.0;//격자 중심으로부터 상류방향 격자면까지의 거리합
	vector<int> neighborCVIDsFlowIntoMe;//현재 셀로 흘러 들어오는 인접셀의 ID, 최대 7개
	int downCellidToFlow = -1; //흘러갈 직하류셀의 ID
	int effCVCountFlowintoCViW = -1;//인접상류셀 중 실제 유출이 발생하는 셀들의 개수
	double cvdx_m;//모델링에 적용할 검사체적의 X방향 길이
	vector<int> downStreamWPCVIDs;//현재 CV 하류에 있는 watchpoint 들의 CVid 들
	int toBeSimulated = 0; // -1 : false, 1 : true //현재의 CV가 모의할 셀인지 아닌지 표시
	cvStreamAtt stream;//현재 CV가 Stream 일경우 즉, eCellType이 Channel 혹은 OverlandAndChannel일 경우 부여되는 속성
	int isStream = 0; // 현재 cv가 stream 인지 아닌지
	double hOF_ori = -1.0;//t 시간에서 유출해석 시작 전 overland flow 검사체적의 수심
	double uOF = -1.0;//t 시간에서 유출해석 결과 overland flow 검사체적의 유속
	double hOF = -1.0;  //t 시간에서 유출해석 결과 overland flow 검사체적의 수심
	double csaOF = -1.0;//t 시간에서 유출해석 결과 overland flow의 흐름 단면적
	double qCVof_m2Ps = -1.0; //단위폭당 overland flow 유량
	double QCVof_m3Ps = -1.0;//t 시간에서의 유출해석 결과 overland flow의 유량 [m^3/s]
	double QSSF_m3Ps = -1.0;//t 시간에서의 현재 셀에서 다음셀로 지표하에서 유출되는 유량 [m^3/s]
	double QsumCVw_dt_m3 = -1.0;//상류인접 CV에서 현재 CV로 유입되는 유량 단순합. 이건 CVi에서의 연속방정식, 고려하지 않은 단순 합.[m^3/dt]
	double rfApp_dt_m = -1.0;//dt 시간 동안의 강우량
	double rfintensityRead_mPsec = -1.0;//현재 강우입력자료에서 읽은 강우강도 m/s
	double rfintensityRead_tm1_mPsec = -1.0;//이전 시간의 강우강도 m/s
	double rfEff_dt_m = -1.0;//dt시간 동안의 유효강우량
	double rf_dtPrintOut_m = -1.0;//출력 시간간격 동안의 누적 강우량[m]
	double rfAcc_FromStartToNow_m = -1.0;//전체 기간의 누적 강우량[m]
	double soilWaterContent_m = -1.0;//토양수분함량. t 시간까지의 누적 침투량[m], 토양깊이가 아니고, 수심이다.
	double soilWaterContent_tm1_m = -1.0;//토양수분함량. t-1 시간까지의 누적 침투량[m]. 수심
	double infiltRatef_mPsec = -1.0;//t 시간에서 계산된 침투률[m/s]
	double InfiltRatef_tM1_mPsec = -1.0;//t-1 시간에서 적용된 침투률[m/s]
	double InfiltrationF_mPdt = -1.0;//t 시간에서 계산된 dt 시간동안의 침투량[m/dt]
	int isAfterSaturated = 0;// -1 : false, 1: true
	soilTextureCode stCode;
	int stCellValue = -1;//토성레이어의 값, VAT참조 // 0 값은 상수를 의미하게 한다.
	double hydraulicC_K_mPsec = -1.0;//현재 CV 토양의 수리전도도[m/s] 모델링 적용값
	double HydraulicC_Kori_mPsec = -1.0;//현재 CV 토양의 수리전도도[m/s] GRM default
	double effPorosity_ThetaE = -1.0;//현재 CV 토양의 유효공극률 모델링 적용값. 무차원. 0~1
	double effPorosity_ThetaEori = -1.0;//현재 CV 토양의 유효공극률 grm default. 무차원. 0~1
	double porosity_Eta = -1.0;//현재 CV 토양의 공극률 모델링 적용값. 무차원. 0~1
	double porosity_EtaOri = -1.0;//현재 CV 토양의 공극률 GRM default. 무차원. 0~1
	double wfsh_Psi_m = -1.0;//현재 CV 토양의 습윤전선흡인수두[m] 모델링 적용값
	double wfsh_PsiOri_m = -1.0;//현재 CV 토양의 습윤전선흡인수두[m] grm default
	double soilMoistureChange_DTheta = -1.0;//토양습윤변화량
	soilDepthCode sdCode;
	int sdCellValue = -1;//토심레이어의 값, VAT 참조 // 0 값은 상수를 의미하게 한다.
	double sd_m = -1.0;//현재 CV의 토양심 모델링 적용 값[m].
	double sdOri_m = -1.0;//현재 CV의 토양심 GRM default 값[m].
	double sdEffAsWaterDepth_m = -1.0;//현재 CV의 유효토양심 값[m]. 토양심에서 유효공극률을 곱한 값
	double iniSR = -1.0;//현재 CV 토양의 초기포화도. 무차원. 0~1
	double effSR_Se = -1.0; // //현재 CV 토양의 유효포화도. 무차원. 0~1 무차원 %/100
	double ssr = -1.0;//토양의 현재 포화도
	unSaturatedKType ukType;
	double coefUK = -1.0;
	double hUAQfromChannelBed_m = -1.0; //하도셀에서 비피압대수층의 수심(하도바닥에서의 높이)[m].
	double hUAQfromBedrock_m = -1.0;///암반으로부터 비피압대수층의 상단부까지의 깊이[m]. 토양깊이.
	double sdToBedrock_m = -1.0;//현재 CV 토양의 암반까지의 깊이[m]. 지표면에서 암반까지의 깊이임.
	double bfQ_m3Ps = -1.0;//현재 CV의 기저유출량 [m^3/s]
	landCoverCode lcCode;
	int lcCellValue = -1;//토지피복레이어의 값, VAT 참조 // 0 값은 상수를 의미하게 한다.
	double imperviousR = -1.0;//현재 CV 토지피복의 불투수율. 무차원, 0~1.
	double Interception_m = -1.0;//현재 CV 토지피복에서의 차단량 [m].
	double roughnessCoeffOF = -1.0;//현재 CV 토지피복의 모델링 적용 지표면 조도계수
	double roughnessCoeffOFori = -1.0;//현재 CV 토지피복의 grm default 지표면 조도계수
	flowControlType fcType;//현재 CV에 부여된 Flow control 종류
	double storageCumulative_m3 = -1.0;//현재 CV에서 flow control 모의시 누적 저류량[m^3]
	double StorageAddedForDTfromRF_m3 = -1.0;//현재 CV에서 flow control 모의시 dt 시간동안의 강우에 의해서 추가되는 저류량[m^3/dt]
} cvAtt;

typedef struct _globalVinner // 계산 루프로 전달하기 위한 최소한의 전역 변수. gpu 고려
{
	int mdp = 0;//-1일 경우는 최대 값으로 자동으로 설정, 1일 경우는 serial 계산
} globalVinner;

typedef struct _projectFile
{
	simulationType simType = simulationType::None;
	string	fpnDomain = "";
	string fpnSlope = "";
	string fpnFD = "";
	string fpnFA = "";
	string fpnStream = "";
	string fpnChannelWidth = "";
	string fpniniSSR = "";
	string fpniniChannelFlow = "";
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

	map <int, swsParameters> swps;
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
flowDirection8 getFlowDirection(int fdirV, flowDirectionType fdType);
void grmHelp();

int initControlVolume();
int initOutputFiles();
int initWatershedNetwork();
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

int openProjectFile(int forceRealTime);
int openPrjAndSetupModel(int forceRealTime);//1:true, -1:false

int readDomainFileAndSetupCV();
int readSlopeFdirFacStreamCWiniSSRiniCF();
int readLandCoverFileAndSetCVbyVAT();
int readSoilTextureFileAndSetCVbyVAT();
int readSoilDepthFileAndSetCVbyVAT();

int runGRM();

int setDomainAndCVBasicinfo();
int setCVbyLCConstant();
int setCVbySTConstant();
int setCVbySDConstant();
int setFlowNetwork();
int setRainfallData();
int setupModelAfterOpenProjectFile();
int startSingleEventRun();

int updateWatershedNetwork();


// extern C
int readLandCoverFile(string fpnLC, 
	int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY);
int readSoilTextureFile(string fpnST, 
	int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY);
int readSoilDepthFile(string fpnSD, 
	int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY);


