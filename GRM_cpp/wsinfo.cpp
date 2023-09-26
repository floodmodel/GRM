#include "stdafx.h"
#include "gentle.h"
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern projectFile prj;
extern fs::path fpnLog;

extern int** cvais;
extern cvAtt* cvs;
extern cvpos* cvps;
extern domaininfo di;
extern map<int, int*> cvaisToFA; //fa�� cv array idex ���

grmWSinfo::grmWSinfo(string fdirType, string fpnDomain,
    string fpnSlope, string fpnFdir, string fpnFac,
    string fpnStream, string fpnLandCover,
    string fpnSoilTexture, string fpnSoilDepth,
    string fpnIniSoilSaturationRatio, string pfnIniChannelFlow,
    string fpnChannelWidth)
{
    //prj.writeConsole = -1;
    prj.forSimulation = -1; // exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
	fs::path fpn_domain = fs::path(fpnDomain.c_str());
	string fp_domain = fpn_domain.parent_path().string();
    fpnLog = fp_domain+"\\"+"GRMdll.log";
    writeNewLog(fpnLog, "GRM.dll : grmWSinfo with input files was started.\n", 1, -1);

    if (_access(fpnDomain.c_str(), 0) != 0) {
        cout << "ERROR. GRM.dll : [" + fpnDomain + "] file is invalid" << endl;
        return;
    }
    if (_access(fpnSlope.c_str(), 0) != 0) {
        cout << "ERROR. GRM.dll : [" + fpnSlope + "] file is invalid" << endl;
        return;
    }
    if (_access(fpnFdir.c_str(), 0) != 0) {
        cout << "ERROR. GRM.dll : [" + fpnFdir + "] file is invalid" << endl;
        return;
    }
    if (_access(fpnFac.c_str(), 0) != 0) {
        cout << "ERROR. GRM.dll : [" + fpnFac + "] file is invalid" << endl;
        return;
    }
    if (trim(fdirType) == "") {
        cout << "ERROR. GRM.dll : Flow direction argument is invalid" << endl;
        return;
    }
    fs::path fpn_dm = fs::path(fpnDomain.c_str());
    string fp_dm = fpn_dm.parent_path().string();
    prj.fpnDomain = fpnDomain;
    prj.fpnFA = fpnFac;
    prj.fpnFD = fpnFdir;
    prj.fpnSlope = fpnSlope;
    prj.fpnStream = fpnStream;
    prj.fpniniSSR = fpnIniSoilSaturationRatio;
    prj.fpniniChFlow = pfnIniChannelFlow;
    prj.fpnChannelWidth = fpnChannelWidth;
    prj.fpnLC = fpnLandCover;
    prj.fpnST = fpnSoilTexture;
    prj.fpnSD = fpnSoilDepth;

    if (lower(fdirType) == lower(ENUM_TO_STR(StartsFromNE))) {
        prj.fdType = flowDirectionType::StartsFromNE;
    }
    else if (lower(fdirType) == lower(ENUM_TO_STR(StartsFromN))) {
        prj.fdType = flowDirectionType::StartsFromN;
    }
    else if (lower(fdirType) == lower(ENUM_TO_STR(StartsFromE))) {
        prj.fdType = flowDirectionType::StartsFromE;
    }
    else if (lower(fdirType) == lower(ENUM_TO_STR(StartsFromE_TauDEM))) {
        prj.fdType = flowDirectionType::StartsFromE_TauDEM;
    }
    else {
        cout << "ERROR. GRM.dll : Flow direction type is invalid." << endl;
        return;
    }
    if (prj.fpnStream != "" &&
        _access(prj.fpnStream.c_str(), 0) == 0) {
        prj.streamFileApplied = 1;
    }
    if (prj.fpniniSSR != "" &&
        _access(prj.fpniniSSR.c_str(), 0) == 0) {
        prj.issrFileApplied = 1;
    }
    if (prj.fpniniChFlow != "" &&
        _access(prj.fpniniChFlow.c_str(), 0) == 0) {
        prj.icfFileApplied = 1;
    }
    if (prj.fpnChannelWidth != "" &&
        _access(prj.fpnChannelWidth.c_str(), 0) == 0) {
        prj.cwFileApplied = 1;
    }

    if (prj.fpnLC != "" &&
        _access(prj.fpnLC.c_str(), 0) == 0) {
        prj.lcDataType = fileOrConstant::File;
    }
    else {
        prj.lcDataType = fileOrConstant::Constant;
    }

    if (prj.fpnST != "" &&
        _access(prj.fpnST.c_str(), 0) == 0) {
        prj.stDataType = fileOrConstant::File;
    }
    else {
        prj.stDataType = fileOrConstant::Constant;
    }

    if (prj.fpnSD != "" &&
        _access(prj.fpnSD.c_str(), 0) == 0) {
        prj.sdDataType = fileOrConstant::File;
    }
    else {
        prj.sdDataType = fileOrConstant::Constant;
    }

    readDomainFaFileAndSetupCV();
	if (readSlopeFdirStreamCwCfSsrFileAndSetCV() == -1) { // 0: false, 1: true
		return;
	}
    if (prj.lcDataType == fileOrConstant::File) {
        readLandCoverFile();
    }
    if (prj.stDataType == fileOrConstant::File) {
        readSoilTextureFile();
    }
    if (prj.sdDataType == fileOrConstant::File) {
        readSoilDepthFile();
    }
    setFlowNetwork();
    setupByFAandNetwork();
    byGMPfile = false;
    grmWSinfo::setPublicVariables();
    writeLog(fpnLog, "GRM.dll : grmWSinfo with input files was ended.\n", 1, -1);
}

grmWSinfo::grmWSinfo(string gmpFPN)
{
    //prj.writeConsole = -1;
    prj.forSimulation = -1;  // exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
    ppi=getProjectFileInfo(gmpFPN);
    fpnLog = "GRMdll.log";
    //cout << fpnLog << endl;
    writeNewLog(fpnLog, "GRM.dll : grmWSinfo with gmp file was started.\n", 1, -1);
    if (openPrjAndSetupModel(-1) == -1) {
        writeLog(fpnLog, "ERROR. GRM.dll : Model setup failed !!!\n", 1, 1);
        return;
    }
    byGMPfile = true;
    grmWSinfo::setPublicVariables();
    writeLog(fpnLog, "GRM.dll : grmWSinfo with gmp file was ended.\n", 1, -1);
}

grmWSinfo::~grmWSinfo()
{
    if (cvais != NULL) {
        for (int i = 0; i < di.nCols; ++i) {
            if (cvais[i] != NULL) { delete[] cvais[i]; }
        }
    }
    if (cvs != NULL) { delete[] cvs; }
    if (cvaisToFA.size() > 0) {
        map<int, int*>::iterator iter;
        map<int, int*> cvansTofa; //fa�� cvan ���
        for (iter = cvaisToFA.begin(); iter != cvaisToFA.end(); ++iter) {
            if (cvaisToFA[iter->first] != NULL) {
                delete[] cvaisToFA[iter->first];
            }
        }
    }
    //delete[] WSIDsAll;
}


void grmWSinfo::setPublicVariables()
{
    writeLog(fpnLog, "GRM.dll : setPublicVariables was started.\n", 1, -1);
    facMaxCellxCol = cvps[di.cvidxMaxFac].xCol;
    facMaxCellyRow = cvps[di.cvidxMaxFac].yRow;
    WSIDsAll = di.dmids;
    WScount = (int)di.dmids.size();
    mostDownStreamWSIDs = di.wsn.mdWSIDs;
    mostDownStreamWSCount = di.wsn.mdWSIDs.size();
    cellCountInWatershed = di.cellNnotNull;
    cellSize = di.cellSize;    
	FDtype = "NONE";
	if (prj.fdType == flowDirectionType::StartsFromNE) {
		FDtype = "StartsFromNE";
	}
	else if (prj.fdType == flowDirectionType::StartsFromN) {
		FDtype = "StartsFromN";
	}
	else if (prj.fdType == flowDirectionType::StartsFromE) {
		FDtype = "StartsFromE";
	}
	else if (prj.fdType == flowDirectionType::StartsFromE_TauDEM) {
		FDtype = "StartsFromE_TauDEM";
	}

	writeLog(fpnLog, "GRM.dll : setPublicVariables was ended.\n", 1, -1);
    writeLog(fpnLog, "GRM.dll : facMaxCellxCol : "+to_string(facMaxCellxCol)+"\n", 1, -1);
	writeLog(fpnLog, "GRM.dll : WScount : " + to_string(WScount) + "\n", 1, -1);
	writeLog(fpnLog, "GRM.dll : di.cellSize : " + to_string(di.cellSize) + "\n", 1, -1);
	writeLog(fpnLog, "GRM.dll : cellSize : " + to_string(cellSize) + "\n", 1, -1);
}

bool grmWSinfo::isInWatershedArea(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return true;
    }
    else {
        return false;
    }
}

vector<int> grmWSinfo::upStreamWSIDs(int currentWSID)
{
    return di.wsn.wsidsAllUp[currentWSID];
}

int grmWSinfo::upStreamWSCount(int currentWSID)
{
	int nws = 0;
	for (int id = 0; id < di.wsn.wsidsAllUp[currentWSID].size(); ++id) {
		if (di.wsn.wsidsAllUp[currentWSID][id] > 0) {
			nws++;
		}
	}
	return nws;
}

vector<int> grmWSinfo::downStreamWSIDs(int currentWSID)
{
    return di.wsn.wsidsAllDown[currentWSID];
}

int grmWSinfo::downStreamWSCount(int currentWSID)
{
	int nws = 0;
	for (int id = 0; id < di.wsn.wsidsAllDown[currentWSID].size(); ++id) {
		if (di.wsn.wsidsAllDown[currentWSID][id] > 0) {
			nws++;
		}
	}
	return nws;
}

int grmWSinfo::watershedID(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvps[idx].wsid;
    }
    else {
        return 0;
    }
}

string grmWSinfo::flowDirection(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        switch (cvs[idx].fdir) {
        case flowDirection8::E8:
            return ENUM_TO_STR(E8);
        case flowDirection8::N8:
            return ENUM_TO_STR(N8);
        case flowDirection8::NE8:
            return ENUM_TO_STR(NE8);
        case flowDirection8::NW8:
            return ENUM_TO_STR(NW8);
        case flowDirection8::S8:
            return ENUM_TO_STR(S8);
        case flowDirection8::SE8:
            return ENUM_TO_STR(SE8);
        case flowDirection8::SW8:
            return ENUM_TO_STR(SW8);
        case flowDirection8::W8:
            return ENUM_TO_STR(W8);
        case flowDirection8::None8:
            return ENUM_TO_STR(None8);
        }
        return ENUM_TO_STR(None8);
    }
    else {
        return "OFWB";
    }
}

int grmWSinfo::flowAccumulation(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvs[idx].fac;
    }
    else {
        return -1;
    }
}

double grmWSinfo::slope(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        if (byGMPfile == true) {
            if (cvs[idx].flowType == cellFlowType::OverlandFlow) {
                return cvs[idx].slopeOF;
            }
            else {
                return cvs[idx].stream.slopeCH;
            }
        }
        else {
            return cvs[idx].slope;
        }
    }
    else {
        return -1.0;
    }
}

int grmWSinfo::streamValue(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0 && prj.streamFileApplied == 1) {
        if (cvs[idx].isStream == 1) {
            return cvs[idx].stream.cellValue;
        }
        else {
            return 0;
        }
    }
    return -1;
}

string grmWSinfo::cellFlowType(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        switch (cvs[idx].flowType) {
        case cellFlowType::OverlandFlow:
            return ENUM_TO_STR(OverlandFlow);
        case cellFlowType::ChannelNOverlandFlow:
            return ENUM_TO_STR(ChannelNOverlandFlow);
        case cellFlowType::ChannelFlow:
            return ENUM_TO_STR(ChannelFlow);
        case cellFlowType::None:
            return ENUM_TO_STR(None);
        }
        return ENUM_TO_STR(None);
    }
    else {
        return "OFWB";
    }
}

int grmWSinfo::landCoverValue(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvs[idx].lcCellValue;
    }
    else    {
        return -1;
    }
}

int grmWSinfo::soilTextureValue(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvs[idx].stCellValue;
    }
    else {
        return -1;
    }
}

int grmWSinfo::soilDepthValue(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return  cvs[idx].sdCellValue;
    }
    else {
        return -1;
    }
}

vector<string> grmWSinfo::allCellsInUpstreamArea(int colXAryidx, int rowYAryidx)
{
    vector<string> cellsPos;
    int startingidx = cvais[colXAryidx][rowYAryidx];
    if (startingidx >= 0) {
        vector<int> cvidxs;
        cvidxs = getAllUpstreamCells(startingidx);
        if (cvidxs.size() > 0) {
            for (int idx : cvidxs) {
                int colx = cvps[idx].xCol;
                int rowy = cvps[idx].yRow;
                string cp = to_string(colx) + ", " + to_string(rowy);
                cellsPos.push_back(cp);
                idx += 1;
            }
        }
    }
    return cellsPos;
}


int grmWSinfo::cellCountInUpstreamArea(int colXAryidx,
    int rowYAryidx)
{
    vector<string> cellsPos;
    cellsPos = grmWSinfo::allCellsInUpstreamArea(colXAryidx, rowYAryidx);
    return cellsPos.size();
}

// If this class was instanced by using gmp file --"grmWS(string gmpFPN)".		
bool grmWSinfo::setOneSWSParsAndUpdateAllSWSUsingNetwork(int wsid, double iniSat,
		double minSlopeLandSurface, unSaturatedKType unSKType, double coefUnsK,
		double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
		int dryStreamOrder, double ccLCRoughness,
		double ccPorosity, double ccWFSuctionHead, double ccSoilHydraulicCond,
		double ccSoilDepth,
		InterceptionMethod interceptMethod,
		PETmethod potentialETMethod, double etCoeff,
		SnowMeltMethod snowMeltMethod, double smeltTSR, double smeltingTemp,
		double snowCovRatio, double smeltCoef,
		double iniFlow)
{
	prj.swps[wsid].wsid = wsid;
    prj.swps[wsid].iniSaturation = iniSat;
	//prj.swps[wsid].iniLossPRCP_mm = iniSat;
	
    prj.swps[wsid].minSlopeOF = minSlopeLandSurface;
    prj.swps[wsid].unSatKType = unSKType;
    prj.swps[wsid].coefUnsaturatedK = coefUnsK;
    prj.swps[wsid].minSlopeChBed = minSlopeChannel;
    prj.swps[wsid].minChBaseWidth = minChannelBaseWidth;
    prj.swps[wsid].chRoughness = roughnessChannel;
    prj.swps[wsid].dryStreamOrder = dryStreamOrder;
    prj.swps[wsid].ccLCRoughness = ccLCRoughness;
    prj.swps[wsid].ccPorosity = ccPorosity;
    prj.swps[wsid].ccWFSuctionHead = ccWFSuctionHead;
    prj.swps[wsid].ccHydraulicK = ccSoilHydraulicCond;
	prj.swps[wsid].ccSoilDepth = ccSoilDepth;
	prj.swps[wsid].interceptMethod = interceptMethod;
	prj.swps[wsid].potentialETMethod = potentialETMethod;
	prj.swps[wsid].etCoeff = etCoeff;
	prj.swps[wsid].snowMeltMethod = snowMeltMethod;
	prj.swps[wsid].tempSnowRain = smeltTSR;
	prj.swps[wsid].smeltingTemp = smeltingTemp;
	prj.swps[wsid].snowCovRatio = snowCovRatio;
	prj.swps[wsid].smeltCoef = smeltCoef;
    prj.swps[wsid].iniFlow = iniFlow;
    prj.swps[wsid].userSet = 1;
    updateAllSWSParsUsingNetwork();
    return true;
}

// If this class was instanced by using gmp file --"grmWS(string gmpFPN)".		
// This method is applied to update all the subwatersheds parameters 
// when there are more than 1 subwatershed.
// Before this method is called, user set parameters must have been updated for each user set watershed
// by using [ grmPrj.SubWSPar.userPars[wsid] property]
// And after this method is called, all the paramters in all the watersheds would be updated by using user set parameters.
void grmWSinfo::updateAllSubWatershedParametersUsingNetwork()
{
    updateAllSWSParsUsingNetwork();
}

swsParameters grmWSinfo::subwatershedPars(int wsid)
{
	prj.swps[wsid].wsid = wsid;
    return prj.swps[wsid];
}

bool grmWSinfo::removeUserParametersSetting(int wsid)
{
    prj.swps[wsid].userSet = 0;
    updateAllSWSParsUsingNetwork();
    return true;
}