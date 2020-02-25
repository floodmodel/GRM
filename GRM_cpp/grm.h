#pragma once

#include "gentle.h"


enum class channelWidthType
{
	CWEquation,
	CWGeneration
};

enum class CrossSectionType
{
	CSSingle,
	CSCompound
};

enum class fcDataSourceType
{
	UserEdit,
	Constant,
	TextFile,
	ReservoirOperation
};

enum class FlowControlType
{
	ReservoirOutflow, // �������, ������ ������� �ʰ�, �￡���� ������� �����
	Inlet,  // ��� �������� �ʴ°�. ������ ������� �ʰ�, inlet grid������ outfow �� �����.
	SinkFlow, // �������, �Էµ� sink flow data �����. ������ �������.
	SourceFlow, // �������, �Էµ� source flow data �����. ������ �������.
	ReservoirOperation, // �������, ������ ���, ������� operation rule�� ���ؼ� ������. ����� �Է� �������̽� �������� ����.
	// ������-�����, ���Է�-����� ������� �̿��ؼ� �ҽ��ڵ忡 �ݿ� ����
	NONE
};

enum class ReservoirOperationType
{
	AutoROM,
	RigidROM,
	ConstantQ,
	SDEqation
};

enum class flowDirectionType
{
	StartsFromNE,
	StartsFromN,
	StartsFromE,
	StartsFromE_TauDEM
};

enum class GRMPrintType
{
	All,
	DischargeFileQ,
	AllQ
};

enum class simulationType
{
	SingleEvent,
	SingleEventPE_SSR,
	RealTime
};

enum class unSaturatedKType
{
	Constant,
	Linear,
	Exponential
};


typedef struct _swsParameters
{
	int wsid = 0;
	double iniSaturation = 0.0;
	double minSlopeOF = 0.0;
	unSaturatedKType unsaturatedKType;
	double coefUnsaturatedK = 0.0;
	double minSlopeChBed = 0.0;
	double minChBaseWidth = 0.0;
	double chRoughness = 0.0;
	int dryStreamOrder = 0;
	double iniFlow = 0.0;
	double ccLCRoughness = 0.0;
	double ccPorosity = 0.0;
	double ccWFSuctionHead = 0.0;
	double ccHydraulicK = 0.0;
	double ccSoilDepth = 0.0;
	int userSet = 0;
} swsParameters;

typedef struct _watchPointInfo
{
	string wpName;
	int wpColX;
	int wpRowY;
} watchPointInfo;

typedef struct _channelWidthInfo
{
	int mdWsid = 0;
	CrossSectionType csType;
	channelWidthType csWidthType;
	double cwEQc;
	double cwEQd;
	double cwEQe;
	double cwMostDownStream;
	double lowRegionHeight;
	double lowRegionBaseWidth;
	double upRegionBaseWidth;
	double compoundCSChannelWidthLimit;
	double bankSlopeRight;
	double bankSlopeLeft;
} channelWidthInfo;

typedef struct _flowControlinfo
{
	string fcName;
	int fcColX = 0;
	int fcRowY = 0;
	FlowControlType ControlType;
	double fcDT = 0.0;
	string fcDataFile;
	double iniStorage = 0.0;
	double maxStorage = 0.0;
	double maxStorageR = 0.0;
	ReservoirOperationType roType;
	double roConstQ = 0.0;
	double roConstQDuration = 0.0;
} flowControlinfo;


typedef struct _soilTextureInfo
{
	int stGridValue;
	string soilTextureName;
	double porosity;
	double effectivePorosity;
	double wfSoilSuctionHead;
	double hydraulicK;
} soilTextureInfo;

typedef struct _soilDepthInfo
{
	int sdGridValue;
	string sdName;
	double soilDepth;
} soilDepthInfo;

typedef struct _landCoverInfo
{
	int lcGridValue;
	string landCoverName;
	double RoughnessCoefficient;
	double ImperviousRatio;
} landCoverInfo;


typedef struct _projectFile
{
	simulationType simType;
	string	domainFile;
	string slopeFile;
	string fdFile;
	string faFile;
	string streamFile;
	string channelWidthFile;
	string iniSSRFile;
	fileOrConstant lcDataType;
	string lcFile;
	string lcVATFile;
	double cnstRoughnessC;
	double CnstImperviousR;
	fileOrConstant stDataType;
	string stFile;
	string stVATFile;
	double cnstSoilPorosity;
	double cnstSoilEffPorosity;
	double cnstSoilWFSH;
	double cnstSoilHydraulicK;
	fileOrConstant sdDataType;
	string sdFile;
	string sdVATFile;
	double cnstSoilDepth;
	string iniChannelFlowFile;
	rainfallDataType rfDataType;
	double rfinterval_min;
	string RainfallDataFile;
	flowDirectionType fdType;
	int MaxDegreeOfParallelism;
	string SimulStartingTime = ""; // ������� �Է� ������  2017-11-28 23:10 ���� ���
	int isDateTimeFormat = 0;// true : 1, false : -1
	double simDuration_hr = 0.0;
	double calTimeStep_sec = 0.0;
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
	GRMPrintType printOption;
	int writeLog = 0;// true : 1, false : -1

	vector <swsParameters> swps;
	vector<watchPointInfo> wps;
	vector<flowControlinfo> fcs;
	vector <channelWidthInfo> cws;
	vector <soilTextureInfo> sts;
	vector <soilDepthInfo> sds;
	vector <landCoverInfo> lcs;
} projectFile;



void disposeDynamicVars();
void grmHelp();
int openPrjAndSetupModel();
int runGRM();