#include <io.h>
#include <filesystem>
#include <map>
#include<vector>
#include <omp.h>
#include <string>
#include<math.h>

#include "gentle.h"
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern int** cvans;
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
    cvans = new int* [di.nCols];
    for (int i = 0; i < di.nCols; ++i) {
        cvans[i] = new int[di.nRows];
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
                cvans[cx][ry] = cvid - 1;// 모든셀. cvid가 아니고, 1차원 배열의 인덱스를 저장. 
            }
            else {
                cvans[cx][ry] = -1;// 모든셀. cvid가 아니고, 1차원 배열의 인덱스를 저장. 모의영역 외부는 -1.
            }
        }
    }
    cvs = new cvAtt[cvsv.size()];
    copy(cvsv.begin(), cvsv.end(), cvs);
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
            int idx = cvans[cx][ry];
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
            int idx = cvans[cx][ry];
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
            int idx = cvans[cx][ry];
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

int readLandCoverFile(string fpnLC, int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY)
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
    int isnormal = 1;
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRowY; ry++) {
        for (int cx = 0; cx < nColX; cx++) {
            int idx = cvAryidx[cx][ry];
            if (idx >= 0) {
                int v = lcFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    cvs1D[idx].lcCellValue = v;
                }
                else {
                    string outstr = "Land cover file (" + fpnLC + ") has invalid value.\n";
                    writeLog(fpnLog, outstr, -1, 1);
                    isnormal = -1;
                }
            }

        }
    }
    return isnormal;
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
            int idx = cvans[cx][ry];
            if (idx >= 0) {
                int v = stFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    if (stvat.find(v) != stvat.end()) {// 현재 셀값이 키로 등록되어 있는지 확인
                        soilTextureInfo st = stvat[v];
                        vBak = v; // 여기서 최신 셀의 값
                        cvs[idx].stCellValue = v;
                        cvs[idx].porosity_EtaOri = st.porosity;
                        cvs[idx].effPorosity_ThetaEori = st.effectivePorosity;
                        cvs[idx].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
                        cvs[idx].HydraulicC_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                        cvs[idx].stCode = st.stCode;
                    }
                    else {
                        string outstr = "Soil texture VAT file [" + prj.fpnSTVat
                            + "] or current project file do not have the soil texture value (" + to_string(v) + ").\n"
                            + "Check the soil texture file or soil texture VAT file. \n";
                        writeLog(fpnLog, outstr, -1, 1);
                        return -1;
                    }
                }
            }
            else { // 셀값으로 정상적인 값(>0)이 입력되어 있지 않으면.. 가장 인접한(최신의) 값으로 설정한다.
                soilTextureInfo st = stvat[vBak];
                cvs[idx].stCellValue = vBak;
                cvs[idx].porosity_EtaOri = st.porosity;
                cvs[idx].effPorosity_ThetaEori = st.effectivePorosity;
                cvs[idx].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
                cvs[idx].HydraulicC_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                cvs[idx].stCode = st.stCode;
            }
        }
    }
    return 1;
}

int setCVbySTConstant()
{
    if (prj.cnstSoilEffPorosity == -1) {
        string outstr = "Soil texture constant effective porosity is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstSoilHydraulicK == -1) {
        string outstr = "Soil texture constant hydraulic conductivity is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstSoilPorosity == -1) {
        string outstr = "Soil texture constant porosity is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstSoilWFSH == -1) {
        string outstr = "Soil texture constant wetting front suction head is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }

    int nRy = di.nRows;
    int nCx = di.nCols;
    omp_set_num_threads(prj.maxDegreeOfParallelism);
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvans[cx][ry];
            if (idx >= 0) {
                cvs[idx].stCellValue = 0;// 이 값은 상수를 의미하게 한다.
                cvs[idx].porosity_EtaOri = prj.cnstSoilPorosity;
                cvs[idx].effPorosity_ThetaEori = prj.cnstSoilEffPorosity;
                cvs[idx].wfsh_PsiOri_m = prj.cnstSoilWFSH / 100.0;  // cm -> m
                cvs[idx].HydraulicC_Kori_mPsec = prj.cnstSoilHydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                cvs[idx].stCode =  soilTextureCode::CONSTV;
            }
        }
    }
    return 1;
}

int readSoilTextureFile(string fpnST, int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY)
{
    if (fpnST == "" || _access(fpnST.c_str(), 0) != 0) {
        string outstr = "Soil texture file (" + fpnST + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    if (nColX < 1 || nRowY < 1) {
        writeLog(fpnLog, "The number of columns or rows have to be greater than 0.\n", -1, 1);
        return -1;
    }
    ascRasterFile stFile = ascRasterFile(fpnST);
    omp_set_num_threads(prj.maxDegreeOfParallelism);
    int isnormal = 1;
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRowY; ry++) {
        for (int cx = 0; cx < nColX; cx++) {
            int idx = cvAryidx[cx][ry];
            if (idx >= 0) {
                int v = stFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    cvs1D[idx].stCellValue = v;
                }
                else {
                    string outstr = "Soil texture file (" + fpnST + ") has invalid value.\n";
                    writeLog(fpnLog, outstr, -1, 1);
                    isnormal = -1;
                }
            }
        }
    }
    return isnormal;
}


int readSoilDepthFileAndSetCVbyVAT()
{
    if (prj.fpnSD == "" || _access(prj.fpnSD.c_str(), 0) != 0) {
        string outstr = "Soil depth file (" + prj.fpnSD + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnSDVat == "" || _access(prj.fpnSDVat.c_str(), 0) != 0) {
        string outstr = "Soil depth VAT file (" + prj.fpnSDVat + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    ascRasterFile sdFile = ascRasterFile(prj.fpnSD);
    int nRy = di.nRows;
    int nCx = di.nCols;
    map<int, soilDepthInfo> sdvat;
    for (int n = 0; n < prj.sds.size(); n++) {
        int sdkey = prj.sds[n].sdGridValue;
        if (sdvat.find(sdkey) == sdvat.end()) {
            sdvat[sdkey] = prj.sds[n];
        }
    }
    map<int, soilDepthInfo>::iterator iter;
    iter = sdvat.begin();
    int vBak = iter->first;
    //오류 값에 인접 셀값을 설정하기 위해, 병렬로 하지 않는다.
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvans[cx][ry];
            if (idx >= 0) {
                int v = sdFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    if (sdvat.find(v) != sdvat.end()) {// 현재 셀값이 키로 등록되어 있는지 확인
                        soilDepthInfo sd = sdvat[v];
                        vBak = v; // 여기서 최신 셀의 값
                        cvs[idx].sdCellValue = v;
                        cvs[idx].sdOri_m = sd.soilDepth / 100.0;   // cm ->  m
                        cvs[idx].sdCode = sd.sdCode; 
                    }
                    else {
                        string outstr = "Soil depth VAT file [" + prj.fpnSDVat
                            + "] or current project file do not have the soil depth value ("
                            + to_string(v) + ").\n"
                            + "Check the soil depth file or soil depth VAT file. \n";
                        writeLog(fpnLog, outstr, -1, 1);
                        return -1;
                    }
                }
                else { // 셀값으로 정상적인 값(>0)이 입력되어 있지 않으면.. 가장 인접한(최신의) 값으로 설정한다.
                    soilDepthInfo sd = sdvat[vBak];
                    cvs[idx].sdCellValue = vBak;
                    cvs[idx].sdOri_m = sd.soilDepth / 100.0;   // cm ->  m
                    cvs[idx].sdCode = sd.sdCode;
                }
            }
        }
    }
    return 1;
}

int setCVbySDConstant()
{
    if (prj.cnstSoilDepth == -1) {
        string outstr = "Constant soil depth value is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    int nRy = di.nRows;
    int nCx = di.nCols;
    omp_set_num_threads(prj.maxDegreeOfParallelism);
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvans[cx][ry];
            if (idx >= 0) {
                cvs[idx].sdCellValue = 0;// 이 값은 상수를 의미하게 한다.
                cvs[idx].sdOri_m = prj.cnstSoilDepth / 100.0;   // cm ->  m
                cvs[idx].sdCode = soilDepthCode::CONSTV;
            }
        }
    }
    return 1;
}

int readSoilDepthFile(string fpnSD, int** cvAryidx, cvAtt* cvs1D, int nColX, int nRowY)
{
    if (fpnSD == "" || _access(fpnSD.c_str(), 0) != 0) {
        string outstr = "Soil depth file (" + fpnSD + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    if (nColX < 1 || nRowY < 1) {
        writeLog(fpnLog, "The number of columns or rows have to be greater than 0.\n", -1, 1);
        return -1;
    }
    ascRasterFile sdFile = ascRasterFile(fpnSD);
    omp_set_num_threads(prj.maxDegreeOfParallelism);
    int isnormal = 1;
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRowY; ry++) {
        for (int cx = 0; cx < nColX; cx++) {
            int idx = cvAryidx[cx][ry];
            if (idx >= 0) {
                int v = sdFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    cvs1D[idx].sdCellValue = v;
                }
                else {
                    string outstr = "Soil depth file (" + fpnSD + ") has invalid value.\n";
                    writeLog(fpnLog, outstr, -1, 1);
                    isnormal = -1;
                }
            }
        }
    }
    return isnormal;
}

int setFlowNetwork()
{
    initWatershedNetwork(); // 여기서 유역 네트워크 정보 초기화
    omp_set_num_threads(prj.maxDegreeOfParallelism);
#pragma omp parallel for schedule(guided)
    //여기서 셀별 네트워크 정보 초기화
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int idx = cvans[cx][ry];// current cell index
            if (idx >= 0) {
                cvs[idx].neighborCVIDsFlowIntoMe.clear();
                cvs[idx].downStreamWPCVIDs.clear();
            }
        }
    }

    double halfDX_Diag_m;
    double halfDXperp_m;
    halfDX_Diag_m = di.cellSize * sqrt(2) / 2.0;
    halfDXperp_m = di.cellSize / 2.0;
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++)
        {
            int cidx = cvans[cx][ry];// current cell index
            if (cidx >= 0) {
                double dxe;
                int tCx; // 하류방향 대상 셀의 x array index
                int tRy; // 하류방향 대상 셀의 y array index
                // 좌상단이 0,0 이다... 즉, 북쪽이면, row-1, 동쪽이면 col +1
                switch (cvs[cidx].fdir) {
                case flowDirection8::NE8: {
                    tCx = cx + 1;
                    tRy = ry - 1;
                    dxe = halfDX_Diag_m;
                    break;
                }
                case flowDirection8::E8: {
                    tCx = cx + 1;
                    tRy = ry;
                    dxe = halfDXperp_m;
                    break;
                }
                case flowDirection8::SE8: {
                    tCx = cx + 1;
                    tRy = ry + 1;
                    dxe = halfDX_Diag_m;
                    break;
                }
                case flowDirection8::S8: {
                    tCx = cx;
                    tRy = ry + 1;
                    dxe = halfDXperp_m;
                    break;
                }
                case flowDirection8::SW8: {
                    tCx = cx - 1;
                    tRy = ry + 1;
                    dxe = halfDX_Diag_m;
                    break;
                }
                case flowDirection8::W8: {
                    tCx = cx - 1;
                    tRy = ry;
                    dxe = halfDXperp_m;
                    break;
                }
                case flowDirection8::NW8: {
                    tCx = cx - 1;
                    tRy = ry - 1;
                    dxe = halfDX_Diag_m;
                    break;
                }
                case flowDirection8::N8: {
                    tCx = cx;
                    tRy = ry - 1;
                    dxe = halfDXperp_m;
                    break;
                }
                case flowDirection8::None8: {
                    tCx = -1;
                    tRy = -1;
                    dxe = halfDXperp_m;
                    break;
                }
                }

                if (tCx >= 0 && tCx < di.nCols && tRy >= 0 && tRy < di.nRows) {
                    //하류 셀이 전체 raster 영역 내부이면,
                    if (cvans[tCx][tRy] == -1) {// 하류 셀이 effect 셀 영역 외부에 있으면,
                        int wsidKey = cvs[cidx].wsid; // 이건 현재셀이 포함된 ws의 id
                        //di.wsn.wsOutletCVids 는 readDomainFileAndSetupCV() 에서 초기화 되어 있다.
                        if (di.wsn.wsOutletCVID.find[wsidKey] == di.wsn.wsOutletCVID.end() ||
                            cvs[cidx].fac > cvs[di.wsn.wsOutletCVID[wsidKey] - 1].fac) {
                            // 현재 ws에 대한 outlet셀이 지정되지 않았거나, 
                            //이미 지정되어 있는 셀의 fac 보다 현재 셀의 fac가 크면
                            di.wsn.wsOutletCVID[wsidKey] = cvs[cidx].cvid;
                        }
                    }
                    else {
                        int tidx = cvans[tCx][tRy]; // target cell index
                        cvs[tidx].neighborCVIDsFlowIntoMe.push_back(cvs[cidx].cvid);
                        cvs[tidx].dxWSum = cvs[tidx].dxWSum + dxe;
                        cvs[cidx].downCellidToFlow = cvs[tidx].cvid;// 흘러갈 방향의 cellid를 현재 셀의 정보에 기록
                        if (cvs[cidx].wsid != cvs[tidx].wsid) {
                            if (di.wsn.wsidNearbyDown[cvs[cidx].wsid] != cvs[tidx].wsid) {
                                di.wsn.wsidNearbyDown[cvs[cidx].wsid] = cvs[tidx].wsid;
                                di.wsn.wsOutletCVID[cvs[cidx].wsid] = cvs[cidx].cvid;
                            }
                            vector<int> v = di.wsn.wsidsNearbyUp[cvs[tidx].wsid];
                            if (std::find(v.begin(), v.end(), cvs[cidx].wsid) == v.end()) {
                                di.wsn.wsidsNearbyUp[cvs[tidx].wsid].push_back(cvs[cidx].wsid);
                            }
                        }
                    }
                    cvs[cidx].dxDownHalf_m = dxe;
                }
                else {// 하류셀이 전체 raster 파일 영역 외부이면,
                    cvs[cidx].downCellidToFlow = -1;
                    cvs[cidx].dxDownHalf_m = dxe;
                    int wsidKey = cvs[cidx].wsid; // 이건 현재셀이 포함된 ws의 id
                    if (di.wsn.wsOutletCVID.find[wsidKey] == di.wsn.wsOutletCVID.end() ||
                        cvs[cidx].fac > cvs[di.wsn.wsOutletCVID[wsidKey] - 1].fac) {
                        // 현재 ws에 대한 outlet셀이 지정되지 않았거나, 
                        //이미 지정되어 있는 셀의 fac 보다 현재 셀의 fac가 크면
                        di.wsn.wsOutletCVID[wsidKey] = cvs[cidx].cvid;
                    }
                }
            }
        }
        if (updateWatershedNetwork() == -1) {
            return -1;
        }
    }
    return 1;
}


int initWatershedNetwork()
{
    di.wsn.wsidsNearbyUp.clear();
    di.wsn.wsidNearbyDown.clear();
    di.wsn.wsidsAllUp.clear();
    di.wsn.wsidsAllDown.clear();
    di.wsn.mdWSIDs.clear();
    di.wsn.wsOutletCVID.clear();
    di.wsn.mdWSIDofCurrentWS.clear();
    for (int n = 0; n < di.dmids.size(); n++) {
        vector<int> v;
        int wsid = di.dmids[n];
        di.wsn.wsidsNearbyUp[wsid] = v;
        di.wsn.wsidNearbyDown[wsid] = -1;
        di.wsn.wsidsAllUp[wsid] = v;
        di.wsn.wsidsAllDown[wsid] = v;
        di.wsn.wsOutletCVID[wsid] = -1;
        di.wsn.mdWSIDofCurrentWS[wsid] = -1;
    }
    return 1;
}

int updateWatershedNetwork()
{
    for (int wsid_cur : di.dmids) { //우선 인접한 유역 id를 추가하고
        for (int wsid_nu : di.wsn.wsidsNearbyUp[wsid_cur]) {
            di.wsn.wsidsAllUp[wsid_cur].push_back(wsid_nu);
        }
        int wsid_nd = di.wsn.wsidNearbyDown[wsid_cur];
        if (wsid_nd > 0) {
            di.wsn.wsidsAllDown[wsid_cur].push_back(wsid_nd);
        }
    }

    for (int wsid_cur : di.dmids) {// 상하류 wsid를 추가한다.
        vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];
        vector<int> downIDs = di.wsn.wsidsAllDown[wsid_cur];
        for (int uid : upIDs) {
            for (int did : downIDs) {
                if (getVectorIndex(di.wsn.wsidsAllUp[did], uid) == -1) {
                    di.wsn.wsidsAllUp[did].push_back(uid);
                }
                if (getVectorIndex(di.wsn.wsidsAllDown[uid], did) == -1) {
                    di.wsn.wsidsAllDown[uid].push_back(did);
                }
            }
        }
    }

    for (int wsid_cur : di.dmids) {//최하류 wsid 목록을 만들고
        if (di.wsn.wsidNearbyDown[wsid_cur] == -1) {
            di.wsn.mdWSIDs.push_back(wsid_cur);
        }
    }

    for (int wsid_cur : di.dmids) {//특정 wsid에 대한 최하류 wsid를 설정한다.
        for (int wsid_md : di.wsn.mdWSIDs) {
            if (wsid_cur == wsid_md 
                || getVectorIndex(di.wsn.wsidsAllUp[wsid_md], wsid_cur) != -1) {
                di.wsn.mdWSIDofCurrentWS[wsid_cur] = wsid_md;
            }
        }
    }

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


