#include <io.h>
#include <string>
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern projectFile prj;
extern fs::path fpnLog;

extern int** cvais;
extern cvAtt* cvs;
extern domaininfo di;
extern map<int, int*> cvaisToFA; //fa별 cv array idex 목록

int GRM::grmWS::grmPlus(int a, int b)
{
return a + b;
}
int GRM::grmWS::grmMultiple(int a, int b)
{
return a * b; 
}


GRM::grmWS::grmWS(string fdirType, string fpnDomain,
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
    GRM::grmWS::setPublicVariables();
}

GRM::grmWS::grmWS(string gmpFPN)
{
    getProjectFileInfo(gmpFPN);
    if (openPrjAndSetupModel(-1) == -1) {
        writeLog(fpnLog, "Model setup failed !!!\n", 1, 1);
        return;
    }
    byGMPfile = true;
    GRM::grmWS::setPublicVariables();
}

GRM::grmWS::~grmWS()
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


int GRM::grmWS::setPublicVariables()
{
    mostDownStreamCell.xCol = cvs[di.cvidxMaxFac].xCol;
    mostDownStreamCell.yRow = cvs[di.cvidxMaxFac].yRow;    
    WSIDsAll = di.dmids;
    //WSIDsAll = new int[di.dmids.size()];
    //copy(di.dmids.begin(), di.dmids.end(), WSIDsAll);
    WScount = (int)di.dmids.size();
    mostDownStreamWSIDs = di.wsn.mdWSIDs;
    //mostDownStreamWSIDs = new int[di.wsn.mdWSIDs.size()];
    //copy(di.wsn.mdWSIDs.begin(), di.wsn.mdWSIDs.end(), mostDownStreamWSIDs);
    cellCountInWatershed = di.cellNnotNull;


}


bool GRM::grmWS::IsInWatershedArea(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return true;
    }
    else {
        return false;
    }
}

vector<int> GRM::grmWS::upStreamWSIDs(int currentWSID)
{
    return di.wsn.wsidsAllUp[currentWSID];
}

int GRM::grmWS::upStreamWSCount(int currentWSID)
{
    return  di.wsn.wsidsAllUp[currentWSID].size();
}

vector<int> GRM::grmWS::downStreamWSIDs(int currentWSID)
{
    return di.wsn.wsidsAllDown[currentWSID];
}

int GRM::grmWS::downStreamWSCount(int currentWSID)
{
    return di.wsn.wsidsAllDown[currentWSID].size();
}

int GRM::grmWS::watershedID(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvs[idx].wsid;
    }
    else {
        return 0;
    }
}

string GRM::grmWS::flowDirection(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return ENUM_TO_STR(cvs[idx].fdir);
    }
    else {
        return "NULL";
    }
}

int GRM::grmWS::flowAccumulation(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvs[idx].fac;
    }
    else {
        return -1;
    }
}

double GRM::grmWS::slope(int colXAryidx, int rowYAryidx)
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

int GRM::grmWS::streamValue(int colXAryidx, int rowYAryidx)
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

string GRM::grmWS::cellFlowType(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return ENUM_TO_STR(cvs[idx].flowType);
    }
    else {
        return "NULL";
    }
}

int GRM::grmWS::landCoverValue(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvs[idx].lcCellValue;
    }
    else    {
        return -1;
    }
}

int GRM::grmWS::soilTextureValue(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return cvs[idx].stCellValue;
    }
    else {
        return -1;
    }
}

int GRM::grmWS::soilDepthValue(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        return  cvs[idx].sdCellValue;
    }
    else {
        return -1;
    }
}

vector<string> GRM::grmWS::allCellsInUpstreamArea(int colXAryidx, int rowYAryidx)
{
    int idx = cvais[colXAryidx][rowYAryidx];
    if (idx >= 0) {
        vector<int> cvidxs;
        int startingBaseCVidx = idx;
        cvidxs  = grmPrj.getAllUpstreamCells(startingBaseCVidx);
        if (cvidxs != null)
        {
            string[] cellsArray = new string[cvids.Count - 1 + 1];
            int idx = 0;
            foreach(int cvid : cvids)
            {
                int colx = grmPrj.dmInfo[cvid - 1].XCol;
                int rowy = grmPrj.dmInfo[cvid - 1].YRow;
                string cellpos = colx.ToString() + ", " + rowy.ToString();
                cellsArray[idx] = cellpos;
                idx += 1;
            }
            return cellsArray;
        }
        else
            return null;
    }
    else
        return null;
}

//public bool SetOneSWSParametersAndUpdateAllSWSUsingNetwork(int wsid, double iniSat,
//    double minSlopeLandSurface, string UnsKType, double coefUnsK,
//    double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
//    int dryStreamOrder, double ccLCRoughness,
//    double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
//    double ccSoilHydraulicCond, double iniFlow = 0)
//{
//    try
//    {
//        {
//            grmPrj.subWSPar.userPars[wsid].iniSaturation = iniSat;
//            grmPrj.subWSPar.userPars[wsid].minSlopeOF = minSlopeLandSurface;
//            grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Linear.ToString();
//            if (UnsKType.ToLower() == cGRM.UnSaturatedKType.Linear.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Linear.ToString(); }
//            if (UnsKType.ToLower() == cGRM.UnSaturatedKType.Exponential.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Exponential.ToString(); }
//            if (UnsKType.ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Constant.ToString(); }
//            if (UnsKType.ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower()) { grmPrj.subWSPar.userPars[wsid].UKType = cGRM.UnSaturatedKType.Constant.ToString(); }
//            grmPrj.subWSPar.userPars[wsid].coefUK = coefUnsK;
//            grmPrj.subWSPar.userPars[wsid].minSlopeChBed = minSlopeChannel;
//            grmPrj.subWSPar.userPars[wsid].minChBaseWidth = minChannelBaseWidth;
//            grmPrj.subWSPar.userPars[wsid].chRoughness = roughnessChannel;
//            grmPrj.subWSPar.userPars[wsid].dryStreamOrder = dryStreamOrder;
//            grmPrj.subWSPar.userPars[wsid].ccLCRoughness = ccLCRoughness;
//            grmPrj.subWSPar.userPars[wsid].ccSoilDepth = ccSoilDepth;
//            grmPrj.subWSPar.userPars[wsid].ccPorosity = ccPorosity;
//            grmPrj.subWSPar.userPars[wsid].ccWFSuctionHead = ccWFSuctionHead;
//            grmPrj.subWSPar.userPars[wsid].ccHydraulicK = ccSoilHydraulicCond;
//            grmPrj.subWSPar.userPars[wsid].iniFlow = iniFlow;
//            grmPrj.subWSPar.userPars[wsid].isUserSet = true;
//        }
//        cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(grmPrj);
//        return true;
//    }
//    catch (Exception ex)
//    {
//        Console.WriteLine(ex.ToString());
//        return false;
//    }
//}

///// <summary>
/////    This method is applied to update all the subwatersheds parameters when there are more than 1 subwatershed.
/////    Before this method is called, user set parameters must have been updated for each user set watershed
/////    by using [ grmPrj.SubWSPar.userPars[wsid] property]
/////    And after this method is called, all the paramters in all the watersheds would be updated by using user set parameters.
/////   </summary>
//public void UpdateAllSubWatershedParametersUsingNetwork()
//{
//    if (WScount() > 1)
//    {
//        cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(grmPrj);
//    }
//}

//public cUserParameters subwatershedPars(int wsid)
//{
//    return grmPrj.subWSPar.userPars[wsid];
//}

//public bool RemoveUserParametersSetting(int wsid)
//{
//    try
//    {
//        grmPrj.subWSPar.userPars[wsid].isUserSet = false;
//        cSetSubWatershedParameter.UpdateSubWSParametersForWSNetwork(grmPrj);
//        return true;
//    }
//    catch (Exception ex)
//    {
//        Console.WriteLine(ex.ToString());
//        return false;
//    }
//}


//public double cellSize()
//{
//    return grmPrj.watershed.mCellSize;
//}

