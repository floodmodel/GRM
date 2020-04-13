#include <io.h>
#include <string>
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
extern map<int, int*> cvaisToFA; //fa별 cv array idex 목록

grmWSinfo::grmWSinfo(string fdirType, string fpnDomain,
    string fpnSlope, string fpnFdir, string fpnFac,
    string fpnStream, string fpnLandCover,
    string fpnSoilTexture, string fpnSoilDepth,
    string fpnIniSoilSaturationRatio, string pfnIniChannelFlow,
    string fpnChannelWidth)
{
    if (_access(fpnDomain.c_str(), 0) != 0) {
        cout << "[" + fpnDomain + "] file is invalid" << endl;
        return;
    }
    if (_access(fpnSlope.c_str(), 0) != 0) {
        cout << "[" + fpnSlope + "] file is invalid" << endl;
        return;
    }
    if (_access(fpnFdir.c_str(), 0) != 0) {
        cout << "[" + fpnFdir + "] file is invalid" << endl;
        return;
    }
    if (_access(fpnFac.c_str(), 0) != 0) {
        cout << "[" + fpnFac + "] file is invalid" << endl;
        return;
    }
    if (trim(fdirType) == "") {
        cout << "Flow direction argument is invalid" << endl;
        return;
    }
    fs::path fpn_dm = fs::path(fpnDomain.c_str());
    string fp_dm = fpn_dm.parent_path().string();
    fpnLog = fp_dm + "\\grmdll.log";
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
        cout << "Flow direction type is invalid." << endl;
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

    if (setDomainAndCVBasicinfo() == -1) {
        writeLog(fpnLog, "Model setup failed !!!\n", 1, 1);
        return;
    }
    if (setupByFAandNetwork() == -1) {
        writeLog(fpnLog, "Network setup failed !!!\n", 1, 1);
        return;
    }
    byGMPfile = false;
    grmWSinfo::setPublicVariables();
}

grmWSinfo::grmWSinfo(string gmpFPN)
{
    cout << "1. " << gmpFPN << endl;
    ppi=getProjectFileInfo(gmpFPN);
    cout << "2. " << ppi.fpn_prj << endl;
    if (openPrjAndSetupModel(-1) == -1) {
        writeLog(fpnLog, "Model setup failed !!!\n", 1, 1);
        return;
    }
    byGMPfile = true;
    cout <<"3. "<< gmpFPN << endl;
    grmWSinfo::setPublicVariables();
    cout << "6. " << "setPublicVariables. ok" << endl;
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
        map<int, int*> cvansTofa; //fa별 cvan 목록
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
    facMaxCellxCol = cvps[di.cvidxMaxFac].xCol;
    facMaxCellyRow = cvps[di.cvidxMaxFac].yRow;
    WSIDsAll = new int[di.dmids.size()];
    copy(di.dmids.begin(), di.dmids.end(), WSIDsAll);
    //WSIDsAll = new int[di.dmids.size()];
    //copy(di.dmids.begin(), di.dmids.end(), WSIDsAll);
    WScount = (int)di.dmids.size();
    mostDownStreamWSIDs = new int[di.wsn.mdWSIDs.size()];
    copy(di.wsn.mdWSIDs.begin(), di.wsn.mdWSIDs.end(), 
        mostDownStreamWSIDs);
    //mostDownStreamWSIDs = new int[di.wsn.mdWSIDs.size()];
    //copy(di.wsn.mdWSIDs.begin(), di.wsn.mdWSIDs.end(), mostDownStreamWSIDs);
    cellCountInWatershed = di.cellNnotNull;
    cellSize = di.cellSize;    
    cout << "5. " << "setPublicVariables. facMaxCellxCol : " << facMaxCellxCol << endl;
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

int* grmWSinfo::upStreamWSIDs(int currentWSID)
{
    vector<int>  idsv = di.wsn.wsidsAllUp[currentWSID];
    int* ids = new int[idsv.size()];
    copy(idsv.begin(), idsv.end(), ids);
    return ids;
    //return di.wsn.wsidsAllUp[currentWSID];
}

int grmWSinfo::upStreamWSCount(int currentWSID)
{
    return  di.wsn.wsidsAllUp[currentWSID].size();
}

int * grmWSinfo::downStreamWSIDs(int currentWSID)
{
    vector<int>  idsv = di.wsn.wsidsAllDown[currentWSID];
    int* ids = new int[idsv.size()];
    copy(idsv.begin(), idsv.end(), ids);
    return ids;
    //return di.wsn.wsidsAllDown[currentWSID];
}

int grmWSinfo::downStreamWSCount(int currentWSID)
{
    return di.wsn.wsidsAllDown[currentWSID].size();
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
        return ENUM_TO_STR(cvs[idx].fdir);
    }
    else {
        return "NULL";
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
        return ENUM_TO_STR(cvs[idx].flowType);
    }
    else {
        return "NULL";
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

string* grmWSinfo::allCellsInUpstreamArea(int colXAryidx, int rowYAryidx)
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
    string* cps;
    cps = new string[cellsPos.size()];
    copy(cellsPos.begin(), cellsPos.end(), cps);
    return cps;
    //return cellsPos;
}

// If this class was instanced by using gmp file --"grmWS(string gmpFPN)".		
bool grmWSinfo::setOneSWSParsAndUpdateAllSWSUsingNetwork(int wsid, double iniSat,
    double minSlopeLandSurface, string unSKType, double coefUnsK,
    double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
    int dryStreamOrder, double ccLCRoughness,
    double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
    double ccSoilHydraulicCond, double iniFlow)
{
    prj.swps[wsid].iniSaturation = iniSat;
    prj.swps[wsid].minSlopeOF = minSlopeLandSurface;
    prj.swps[wsid].unSatKType = unSaturatedKType::Linear;
        unSaturatedKType uskt = unSaturatedKType::None;
        if (unSKType != "") {
            if (lower(unSKType) == lower(ENUM_TO_STR(Constant))) {
                prj.swps[wsid].unSatKType = unSaturatedKType::Constant;
            }
            else if (lower(unSKType) == lower(ENUM_TO_STR(Linear))) {
                prj.swps[wsid].unSatKType = unSaturatedKType::Linear;
            }
            else if (lower(unSKType) == lower(ENUM_TO_STR(Exponential))) {
                prj.swps[wsid].unSatKType = unSaturatedKType::Exponential;
            }
            else {
                writeLog(fpnLog, "Unsaturated K type in the watershed ["
                    + to_string(wsid) + "] is invalid.\n", 1, 1);
                return false;
            }
        }
    prj.swps[wsid].coefUnsaturatedK = coefUnsK;
    prj.swps[wsid].minSlopeChBed = minSlopeChannel;
    prj.swps[wsid].minChBaseWidth = minChannelBaseWidth;
    prj.swps[wsid].chRoughness = roughnessChannel;
    prj.swps[wsid].dryStreamOrder = dryStreamOrder;
    prj.swps[wsid].ccLCRoughness = ccLCRoughness;
    prj.swps[wsid].ccSoilDepth = ccSoilDepth;
    prj.swps[wsid].ccPorosity = ccPorosity;
    prj.swps[wsid].ccWFSuctionHead = ccWFSuctionHead;
    prj.swps[wsid].ccHydraulicK = ccSoilHydraulicCond;
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
    return prj.swps[wsid];
}

bool grmWSinfo::removeUserParametersSetting(int wsid)
{
    prj.swps[wsid].userSet = -1;
    updateAllSWSParsUsingNetwork();
    return true;
}