#include <io.h>
#include <filesystem>
#include <map>
#include<vector>
#include <omp.h>
#include <string>

#include "gentle.h"
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern int** cellidx;
extern cvAtt* cvs;


int readDomainFileAndSetupCV()
{
    if (prj.fpnDomain == "" || _access(prj.fpnDomain.c_str(), 0) != 0) {
        string outstr = "Domain file (" + prj.fpnDomain + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    ascRasterFile dmFile = ascRasterFile(prj.fpnDomain);
    di.nRows = dmFile.header.nRows;
    di.nCols = dmFile.header.nCols;
    di.cellSize = dmFile.header.cellsize;
    di.xll = dmFile.header.xllcorner;
    di.yll = dmFile.header.yllcorner;
    // dim ary(n) 하면, vb.net에서는 0~n까지 n+1개의 배열요소 생성. c#에서는 0~(n-1) 까지 n 개의 요소 생성
    cellidx = new int* [di.nCols];
    for (int i = 0; i < di.nCols; ++i) {
        cellidx[i] = new int[di.nRows];
    }
    vector<cvAtt> cvsv;
    di.cvidsInEachRegion.clear();
    int cvid = 0;
    // cvid를 순차적으로 부여하기 위해서, 이 과정은 병렬로 하지 않는다..
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int wsid = dmFile.valuesFromTL[cx][ry];
            if (wsid > 0) {
                cvAtt cv;
                cv.wsid = wsid;
                cvid += 1;
                cv.cvid = cvid; // CVid를 CV 리스트의 인덱스 번호 +1 의 값으로 입력. 즉. 1 부터 시작
                cv.flowType = cellFlowType::OverlandFlow; // 우선 overland flow로 설정
                if (getVectorIndex(di.dmids, wsid) != -1) {
                    di.dmids.push_back(wsid);
                }
                //if (di.cvidsInEachRegion.count(wsid) == 0) {
                //    vector<int> v;
                //    v.push_back(cvid);
                //    di.cvidsInEachRegion[wsid] = v;
                //}
                //else {
                di.cvidsInEachRegion[wsid].push_back(cvid);
                //}
                cv.toBeSimulated = 1;
                //cv.x = cx; // cellidx 검증용
                //cv.y = ry; // cellidx 검증용
                cvsv.push_back(cv); // 여기는 유효셀만
                cellidx[cx][ry] = cvid - 1;// 모든셀. cvid가 아니고, 1차원 배열의 인덱스를 저장. 
            }
            else {
                cellidx[cx][ry] = -1;// 모든셀. cvid가 아니고, 1차원 배열의 인덱스를 저장. 모의영역 외부는 -1.
            }
        }
    }
    initWatershedNetwork();
    cvs = new cvAtt[cvsv.size()];
    copy(cvsv.begin(), cvsv.end(), cvs);
    return 1;
}

int initWatershedNetwork()
{
    di.wsn.wsidsNearbyUp.clear();
    di.wsn.wsidsNearbyDown.clear();
    di.wsn.wsidsAllUp.clear();
    di.wsn.wsidsAllDown.clear();
    di.wsn.mdWSIDs.clear();
    di.wsn.wsOutletCVids.clear();
    di.wsn.mdWSIDofCurrentWS.clear();
    for (int n = 0; n < di.dmids.size(); n++) {
        vector<int> v;
        int id = di.dmids[n];
        di.wsn.wsidsNearbyUp[id]=v;
        di.wsn.wsidsNearbyDown[id] = v;
        di.wsn.wsidsAllUp[id] = v;
        di.wsn.wsidsAllDown[id] = v;
        di.wsn.wsOutletCVids[id] = -1;
        di.wsn.mdWSIDofCurrentWS[id] = -1;
    }
    return 1;
}

int readSlopeFdirFacStreamCWiniSSRiniCF()
{
    if (prj.fpnSlope == "" || _access(prj.fpnSlope.c_str(), 0) != 0) {
        string outstr = "Slope file (" + prj.fpnSlope + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnFD == "" || _access(prj.fpnFD.c_str(), 0) != 0) {
        string outstr = "Flow direction file (" + prj.fpnFD + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnFA == "" || _access(prj.fpnFA.c_str(), 0) != 0) {
        string outstr = "Flow accumulation file (" + prj.fpnFA + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    int isStream = -1;
    int isCW = -1;
    int isCF = -1;
    int isSSR = -1;
    if (prj.fpnStream == "" || _access(prj.fpnStream.c_str(), 0) != 0) {
        string outstr = "Stream file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
    }
    else {
        isStream = 1;
    }
    if (prj.fpnChannelWidth == "" || _access(prj.fpnChannelWidth.c_str(), 0) != 0) {
        string outstr = "Channel width file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
    }
    else {
        isCW = 1;
    }
    if (prj.fpniniChannelFlow == "" || _access(prj.fpniniChannelFlow.c_str(), 0) != 0) {
        string outstr = "Initial stream flow file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
    }
    else {
        isCF = 1;
    }
    if (prj.fpniniSSR == "" || _access(prj.fpniniSSR.c_str(), 0) != 0) {
        string outstr = "Initial soil saturation ratio file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
    }
    else {
        isSSR = 1;
    }
    ascRasterFile slopeFile = ascRasterFile(prj.fpnSlope);
    ascRasterFile fdirFile = ascRasterFile(prj.fpnFD);
    ascRasterFile facFile = ascRasterFile(prj.fpnFA);
    ascRasterFile* streamFile;
    ascRasterFile* cwFile;
    ascRasterFile* cfFile;
    ascRasterFile* ssrFile;
    if (isStream == 1) {
        streamFile = &ascRasterFile(prj.fpnStream);
    }
    if (isCW == 1) {
        cwFile = &ascRasterFile(prj.fpnChannelWidth);
    }
    if (isCF == 1) {
        cfFile = &ascRasterFile(prj.fpniniChannelFlow);
    }
    if (isSSR == 1) {
        cwFile = &ascRasterFile(prj.fpniniSSR);
    }
    int nRy = di.nRows;
    int nCx = di.nCols;
    omp_set_num_threads(prj.maxDegreeOfParallelism);
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cellidx[cx][ry];
            if (idx >= 0) {
                cvs[idx].slope = slopeFile.valuesFromTL[cx][ry];
                if (cvs[idx].slope <= 0.0) {
                    cvs[idx].slope = CONST_MIN_SLOPE;
                }
                cvs[idx].fdir = getFlowDirection((int)fdirFile.valuesFromTL[cx][ry], prj.fdType);
                cvs[idx].fac = facFile.valuesFromTL[cx][ry];
                if (isStream == 1) {
                    cvs[idx].flowType = cellFlowType::ChannelFlow;
                    cvs[idx].stream.chStrOrder = streamFile->valuesFromTL[cx][ry];
                    if (cvs[idx].stream.chStrOrder > 0) {
                        cvs[idx].isStream = 1;
                    }
                    else {
                        cvs[idx].isStream = -1;
                    }
                }
                if (isCW == 1) {
                    double v = cwFile->valuesFromTL[cx][ry];
                    if (v < 0) {
                        cvs[idx].stream.chBaseWidthByLayer = 0;
                    }
                    else {
                        cvs[idx].stream.chBaseWidthByLayer = v;
                    }
                }
                else {
                    cvs[idx].stream.chBaseWidthByLayer = -1;
                }
                if (isCF == 1) {
                    double v = cfFile->valuesFromTL[cx][ry];
                    if (v < 0) {
                        cvs[idx].stream.iniQCH_m3Ps = 0;
                    }
                    else {
                        cvs[idx].stream.iniQCH_m3Ps = v;
                    }
                }
                if (isSSR == 1) {
                    double v = ssrFile->valuesFromTL[cx][ry];
                    if (v < 0) {
                        cvs[idx].iniSR = 0;
                    }
                    else {
                        cvs[idx].iniSR = v;
                    }
                }
            }
        }
    }
    return 1;
}

int readLandCoverFileAndSetCVbyVAT()
{
    if (prj.fpnLC == "" || _access(prj.fpnLC.c_str(), 0) != 0) {
        string outstr = "Land cover file (" + prj.fpnLC + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnLCVat == "" || _access(prj.fpnLCVat.c_str(), 0) != 0) {
        string outstr = "Land cover VAT file (" + prj.fpnLCVat + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    ascRasterFile lcFile = ascRasterFile(prj.fpnLC);
    int nRy = di.nRows;
    int nCx = di.nCols;
    map<int, landCoverInfo> lcvat;
    for (int n = 0; n < prj.lcs.size(); n++) {
        int lckey = prj.lcs[n].lcGridValue;
        if (lcvat.find(lckey) == lcvat.end()) {
            lcvat[lckey] = prj.lcs[n];
        }
    }
    map<int, landCoverInfo>::iterator iter;
    iter = lcvat.begin();
    int vBak = iter->first;
    //오류 값에 인접 셀값을 설정하기 위해, 병렬로 하지 않는다.
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cellidx[cx][ry];
            if (idx >= 0) {
                int v = lcFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    if (lcvat.find(v) != lcvat.end()) {// 현재 셀값이 키로 등록되어 있는지 확인
                        landCoverInfo lc = lcvat[v];
                        vBak = v; // 여기서 최신 셀의 값
                        cvs[idx].lcCellValue = v;
                        cvs[idx].roughnessCoeffOFori = lc.RoughnessCoefficient;
                        cvs[idx].imperviousR = lc.ImperviousRatio;
                        cvs[idx].lcCode = lc.lcCode;
                    }
                    else {
                        string outstr = "Landcover VAT file [" + prj.fpnLCVat
                            + "] or current project file do not have the land cover value (" 
                            + to_string(v) + ").\n"
                            + "Check the land cover file or land cover VAT file. \n";
                        writeLog(fpnLog, outstr, -1, 1);
                        return -1;
                    }
                }
                else { // 셀값으로 정상적인 값(>0)이 입력되어 있지 않으면.. 가장 인접한(최신의) 값으로 설정한다.
                    landCoverInfo lc = lcvat[vBak];
                    cvs[idx].lcCellValue = vBak;
                    cvs[idx].roughnessCoeffOFori = lc.RoughnessCoefficient;
                    cvs[idx].imperviousR = lc.ImperviousRatio;
                    cvs[idx].lcCode = lc.lcCode;
                }
            }
        }
    }
    return 1;
}

int setCVbyLCConstant()
{
    if (prj.cnstImperviousR == -1) {
        string outstr = "Land cover constant impervious ratio is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstRoughnessC == -1) {
        string outstr = "Land cover constant roughness coefficient is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    int nRy = di.nRows;
    int nCx = di.nCols;
    omp_set_num_threads(prj.maxDegreeOfParallelism);
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cellidx[cx][ry];
            if (idx >= 0) {
                    cvs[idx].lcCellValue = 0; // 이 값은 상수를 의미하게 한다.
                    cvs[idx].roughnessCoeffOFori = prj.cnstRoughnessC;
                    cvs[idx].imperviousR = prj.cnstImperviousR;
                    cvs[idx].lcCode = landCoverCode::CONSTV;
            }
        }
    }
    return 1;
}

int readLandCoverFile(string fpnLC, cvAtt* cells, int nColX, int nRowY)
{
    if (fpnLC == "" || _access(fpnLC.c_str(), 0) != 0) {
        string outstr = "Land cover file (" + fpnLC + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    if (nColX < 1 || nRowY < 1)    {
        writeLog(fpnLog, "The number of columns or rows have to be greater than 0.\n", -1, 1);
        return -1;
    }
    ascRasterFile lcFile = ascRasterFile(fpnLC);
    omp_set_num_threads(prj.maxDegreeOfParallelism);
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRowY; ry++) {
        for (int cx = 0; cx < nColX; cx++) {
            int idx = cellidx[cx][ry];
            if (idx >= 0) {
                int v = lcFile.valuesFromTL[cx][ry];
                cells[idx].lcCellValue = v; // 이 값은 상수를 의미하게 한다.
            }
        }
    }
    return 1;
}


int readSoilTextureFileAndSetCVbyVAT()
{
    if (prj.fpnST == "" || _access(prj.fpnST.c_str(), 0) != 0) {
        string outstr = "Soil texture file (" + prj.fpnST + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnSTVat == "" || _access(prj.fpnSTVat.c_str(), 0) != 0) {
        string outstr = "Soil texture VAT file (" + prj.fpnSTVat + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    ascRasterFile stFile = ascRasterFile(prj.fpnST);
    int nRy = di.nRows;
    int nCx = di.nCols;
    map<int, soilTextureInfo> stvat;
    for (int n = 0; n < prj.sts.size(); n++) {
        int stkey = prj.sts[n].stGridValue;
        if (stvat.find(stkey) == stvat.end()) {
            stvat[stkey] = prj.sts[n];
        }
    }
    map<int, soilTextureInfo>::iterator iter;
    iter = stvat.begin();
    int vBak = iter->first;
    //오류 값에 인접 셀값을 설정하기 위해, 병렬로 하지 않는다.
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cellidx[cx][ry];
            if (idx >= 0) {
                int v = stFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    //if (lcvat.find(v) != lcvat.end()) {// 현재 셀값이 키로 등록되어 있는지 확인
    //                    landCoverInfo lc = lcvat[v];
    //                    vBak = v; // 여기서 최신 셀의 값
    //                    cvs[idx].lcCellValue = v;
    //                    cvs[idx].roughnessCoeffOFori = lc.RoughnessCoefficient;
    //                    cvs[idx].imperviousR = lc.ImperviousRatio;
    //                    cvs[idx].lcCode = lc.lcCode;
                    //}
                    //else {
    //                    string outstr = "Landcover VAT file [" + prj.fpnLCVat
    //                        + "] or current project file do not have the land cover value (" + to_string(v) + ").\n"
    //                        + "Check the land cover file or land cover VAT file. \n";
    //                    writeLog(fpnLog, outstr, -1, 1);
    //                    return -1;
                    //}
                //}
                }
                else { // 셀값으로 정상적인 값(>0)이 입력되어 있지 않으면.. 가장 인접한(최신의) 값으로 설정한다.
    //                landCoverInfo lc = lcvat[vBak];
    //                cvs[idx].lcCellValue = vBak;
    //                cvs[idx].roughnessCoeffOFori = lc.RoughnessCoefficient;
    //                cvs[idx].imperviousR = lc.ImperviousRatio;
    //                cvs[idx].lcCode = lc.lcCode;
                }
            }
        }
    }
    return 1;
}

int setCVbySTConstant()
{
    return 1;
}

int readSoilTextureFile()
{
    return 1;
}

int readSoilDepthFileAndSetCVbyVAT()
{
    return 1;
}

int setCVbySDConstant()
{
    return 1;
}

int readSolDepthFile()
{
    return 1;
}



flowDirection8 getFlowDirection(int fdirV, flowDirectionType fdt)
{
    if (fdt == flowDirectionType::StartsFromNE)
    {
        switch (fdirV) {
        case 1:
            return flowDirection8::NE8;
        case 2:
            return flowDirection8::E8;
        case 4:
            return flowDirection8::SE8;
        case 8:
            return flowDirection8::S8;
        case 16:
            return flowDirection8::SW8;
        case 32:
            return flowDirection8::W8;
        case 64:
            return flowDirection8::NW8;
        case 128:
            return flowDirection8::N8;
        default:
            return flowDirection8::None8;
        }
    }
    if (fdt == flowDirectionType::StartsFromN) {
        switch (fdirV) {
        case 1:
            return flowDirection8::N8;
        case 2:
            return flowDirection8::NE8;
        case 4:
            return flowDirection8::E8;
        case 8:
            return flowDirection8::SE8;
        case 16:
            return flowDirection8::S8;
        case 32:
            return flowDirection8::SW8;
        case 64:
            return flowDirection8::W8;
        case 128:
            return flowDirection8::NW8;
        default:
            return flowDirection8::None8;
        }
    }
    if (fdt == flowDirectionType::StartsFromE) {
        switch (fdirV) {
        case 1:
            return flowDirection8::E8;
        case 2:
            return flowDirection8::SE8;
        case 4:
            return flowDirection8::S8;
        case 8:
            return flowDirection8::SW8;
        case 16:
            return flowDirection8::W8;
        case 32:
            return flowDirection8::NW8;
        case 64:
            return flowDirection8::N8;
        case 128:
            return flowDirection8::NE8;
        default:
            return flowDirection8::None8;
        }
    }
    if (fdt == flowDirectionType::StartsFromE_TauDEM) {
        switch (fdirV) {
        case 1:
            return flowDirection8::E8;
        case 2:
            return flowDirection8::NE8;
        case 3:
            return flowDirection8::N8;
        case 4:
            return flowDirection8::NW8;
        case 5:
            return flowDirection8::W8;
        case 6:
            return flowDirection8::SW8;
        case 7:
            return flowDirection8::S8;
        case 8:
            return flowDirection8::SE8;
        default:
            return flowDirection8::None8;
        }
    }
    return flowDirection8::None8;
}



//int readSlopeFile()
//{
//    if (prj.fpnSlope == "" || _access(prj.fpnSlope.c_str(), 0) != 0) {
//        string outstr = "Slope file (" + prj.fpnSlope + ") is invalid.\n";
//        writeLog(fpnLog, outstr, 1, 1);
//        return -1;
//    }
//    ascRasterFile slopeFile = ascRasterFile(prj.fpnSlope);
//    int nRy = di.nRows;
//    int nCx = di.nCols;
//    omp_set_num_threads(prj.maxDegreeOfParallelism);
//    #pragma omp parallel for schedule(guided)
//    for (int ry = 0; ry < nRy; ry++) {
//        for (int cx = 0; cx < nCx; cx++) {
//            int idx = cellidx[cx][ry];
//            if (idx>= 0) {
//                cvs[idx].slope = slopeFile.valuesFromTL[cx][ry];
//                if (cvs[idx].slope <= 0.0) {
//                    cvs[idx].slope = CONST_MIN_SLOPE;
//                }
//            }
//        }
//    }
//    return 1;
//}
//
//int  readFdirFile()
//{
//    if (prj.fpnFD == "" || _access(prj.fpnFD.c_str(), 0) != 0) {
//        string outstr = "Flow direction file (" + prj.fpnFD + ") is invalid.\n";
//        writeLog(fpnLog, outstr, 1, 1);
//        return -1;
//    }
//    ascRasterFile fdirFile = ascRasterFile(prj.fpnFD);
//    int nRy = di.nRows;
//    int nCx = di.nCols;
//    omp_set_num_threads(prj.maxDegreeOfParallelism);
//#pragma omp parallel for schedule(guided)
//    for (int ry = 0; ry < nRy; ry++) {
//        for (int cx = 0; cx < nCx; cx++)
//        {
//            int idx = cellidx[cx][ry];
//            if (idx >= 0) {
//                cvs[idx].fdir = getFlowDirection((int)fdirFile.valuesFromTL[cx][ry], prj.fdType);
//            }
//        }
//    }
//}
//
//int readFacFile()
//{
//    if (prj.fpnFA == "" || _access(prj.fpnFA.c_str(), 0) != 0) {
//        string outstr = "Flow accumulation file (" + prj.fpnFA + ") is invalid.\n";
//        writeLog(fpnLog, outstr, 1, 1);
//        return -1;
//    }
//    ascRasterFile facFile = ascRasterFile(prj.fpnFA);
//    int nRy = di.nRows;
//    int nCx = di.nCols;
//    omp_set_num_threads(prj.maxDegreeOfParallelism);
//#pragma omp parallel for schedule(guided)
//    for (int ry = 0; ry < nRy; ry++) {
//        for (int cx = 0; cx < nCx; cx++)
//        {
//            int idx = cellidx[cx][ry];
//            if (idx >= 0) {
//                cvs[idx].fac = facFile.valuesFromTL[cx][ry];
//            }
//        }
//    }
//}
