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
extern int** cvais;
extern cvAtt* cvs;


int readDomainFileAndSetupCV()
{
    if (prj.fpnDomain == "" || _access(prj.fpnDomain.c_str(), 0) != 0) {
        string outstr = "Domain file (" + prj.fpnDomain + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    //���⼭ di �Ϲݻ��� �ʱ�ȭ, ����===============
    ascRasterFile dmFile = ascRasterFile(prj.fpnDomain);
    di.nRows = dmFile.header.nRows;
    di.nCols = dmFile.header.nCols;
    di.cellSize = dmFile.header.cellsize;
    di.xll = dmFile.header.xllcorner;
    di.yll = dmFile.header.yllcorner;
    di.headerStringAll = dmFile.headerStringAll;
    di.cvidsInEachRegion.clear();
    di.dmids.clear();
    //====================================
    cvais = new int* [di.nCols];
    for (int i = 0; i < di.nCols; ++i) {
        cvais[i] = new int[di.nRows];
    }
    vector<cvAtt> cvsv;
    int cvid = 0;
    // cvid�� ���������� �ο��ϱ� ���ؼ�, �� ������ ���ķ� ���� �ʴ´�..
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int wsid = dmFile.valuesFromTL[cx][ry];
            if (wsid > 0) {
                cvAtt cv;
                cv.wsid = wsid;
                cvid += 1;
                cv.cvid = cvid; // CVid�� CV ����Ʈ�� �ε��� ��ȣ +1 �� ������ �Է�. ��. 1 ���� ����
                cv.flowType = cellFlowType::OverlandFlow; // �켱 overland flow�� ����
                if (getVectorIndex(di.dmids, wsid) == -1) {//wsid�� vector�� ������, �߰��Ѵ�.
                    di.dmids.push_back(wsid);
                }
                di.cvidsInEachRegion[wsid].push_back(cvid);
                cv.toBeSimulated = 1;
                cv.idx_xr = cx; // cellidx ������
                cv.idx_yc = ry; // cellidx ������
                cvsv.push_back(cv); // ����� ��ȿ����
                cvais[cx][ry] = cvid - 1;// ��缿. cvid�� �ƴϰ�, 1���� �迭�� �ε����� ����. 
            }
            else {
                cvais[cx][ry] = -1;// ��缿. cvid�� �ƴϰ�, 1���� �迭�� �ε����� ����. ���ǿ��� �ܺδ� -1.
            }
        }
    }
    di.cellNnotNull = cvid;
    cvs = new cvAtt[cvsv.size()];
    copy(cvsv.begin(), cvsv.end(), cvs);
    return 1;
}

int readSlopeFdirFacStreamCwCfSsrFileAndSetCV()
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
    if (prj.streamFileApplied == 1
        && (prj.fpnStream == ""
            || _access(prj.fpnStream.c_str(), 0) != 0)) {
        string outstr = "Stream file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.streamFileApplied = -1;
    }
    else {
        prj.streamFileApplied = 1;
    }
    if (prj.cwFileApplied == 1
        && (prj.fpnChannelWidth == ""
            || _access(prj.fpnChannelWidth.c_str(), 0) != 0)) {
        string outstr = "Channel width file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.cwFileApplied = -1;
    }
    else {
        prj.cwFileApplied = 1;
    }
    if (prj.icfFileApplied == 1
        && (prj.fpniniChannelFlow == ""
            || _access(prj.fpniniChannelFlow.c_str(), 0) != 0)) {
        string outstr = "Initial stream flow file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.icfFileApplied = -1;
    }
    else {
        prj.icfFileApplied = 1;
    }
    if (prj.issrFileApplied==1
        &&(prj.fpniniSSR == "" || _access(prj.fpniniSSR.c_str(), 0) != 0)) {
        string outstr = "Initial soil saturation ratio file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.issrFileApplied = -1;
    }
    else {
        prj.issrFileApplied = 1;
    }
    ascRasterFile slopeFile = ascRasterFile(prj.fpnSlope);
    ascRasterFile fdirFile = ascRasterFile(prj.fpnFD);
    ascRasterFile facFile = ascRasterFile(prj.fpnFA);
    ascRasterFile* streamFile;
    ascRasterFile* cwFile;
    ascRasterFile* cfFile;
    ascRasterFile* ssrFile;
    if (prj.streamFileApplied == 1) {
        streamFile = new ascRasterFile(prj.fpnStream);
    }
    else {
        streamFile = NULL;
    }
    if (prj.cwFileApplied == 1) {
        cwFile = new ascRasterFile(prj.fpnChannelWidth);
    }
    else {
        cwFile = NULL;
    }
    if (prj.icfFileApplied == 1) {
        cfFile = new ascRasterFile(prj.fpniniChannelFlow);
    }
    else {
        cfFile = NULL;
    }
    if (prj.issrFileApplied == 1) {
        ssrFile = new ascRasterFile(prj.fpniniSSR);
    }
    else {
        ssrFile = NULL;
    }

    int nRy = di.nRows;
    int nCx = di.nCols;
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvais[cx][ry];
            if (idx >= 0) {
                cvs[idx].slope = slopeFile.valuesFromTL[cx][ry];
                if (cvs[idx].slope <= 0.0) {
                    cvs[idx].slope = CONST_MIN_SLOPE;
                }
                cvs[idx].fdir = getFlowDirection((int)fdirFile.valuesFromTL[cx][ry], prj.fdType);
                cvs[idx].fac = facFile.valuesFromTL[cx][ry];
                if (prj.streamFileApplied == 1) {
                    cvs[idx].stream.chStrOrder = streamFile->valuesFromTL[cx][ry];
                    if (cvs[idx].stream.chStrOrder > 0) {
                        cvs[idx].isStream = 1;
                        cvs[idx].flowType = cellFlowType::ChannelFlow;
                    }
                    else {
                        cvs[idx].isStream = -1;
                        cvs[idx].flowType = cellFlowType::ChannelFlow;
                    }
                }
                if (prj.cwFileApplied == 1) {
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
                if (prj.icfFileApplied == 1) {
                    double v = cfFile->valuesFromTL[cx][ry];
                    if (v < 0) {
                        cvs[idx].stream.iniQCH_m3Ps = 0;
                    }
                    else {
                        cvs[idx].stream.iniQCH_m3Ps = v;
                    }
                }
                if (prj.issrFileApplied == 1) {
                    double v = ssrFile->valuesFromTL[cx][ry];
                    if (v < 0) {
                        cvs[idx].iniSSR = 0;
                    }
                    else {
                        cvs[idx].iniSSR = v;
                    }
                }
            }
        }
    }

    if (prj.streamFileApplied == 1 && streamFile != NULL) {
        delete streamFile;
    }
    if (prj.cwFileApplied == 1 && cwFile != NULL) {
        delete cwFile;
    }
    if (prj.icfFileApplied == 1 && cfFile != NULL) {
        delete cfFile;
    }
    if (prj.issrFileApplied == 1 && ssrFile != NULL) {
        delete ssrFile;
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
    if (prj.fpnLCVat == "" || _access(prj.fpnLCVat.c_str(), 0) != 0) {//�Ӽ� ���� ���� ���� ������ �ִ��� ����.
        string outstr = "Land cover VAT file (" + prj.fpnLCVat + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
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
    //���� ���� ���� ������ �����ϱ� ����, ���ķ� ���� �ʴ´�.
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvais[cx][ry];
            if (idx >= 0) {
                int v = lcFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    if (lcvat.find(v) != lcvat.end()) {// ���� ������ Ű�� ��ϵǾ� �ִ��� Ȯ��
                        landCoverInfo lc = lcvat[v];
                        vBak = v; // ���⼭ �ֽ� ���� ��
                        cvs[idx].lcCellValue = v;
                        cvs[idx].rcOFori = lc.RoughnessCoefficient;
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
                else { // �������� �������� ��(>0)�� �ԷµǾ� ���� ������.. ���� ������(�ֽ���) ������ �����Ѵ�.
                    landCoverInfo lc = lcvat[vBak];
                    cvs[idx].lcCellValue = vBak;
                    cvs[idx].rcOFori = lc.RoughnessCoefficient;
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
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvais[cx][ry];
            if (idx >= 0) {
                    cvs[idx].lcCellValue = 0; // �� ���� ����� �ǹ��ϰ� �Ѵ�.
                    cvs[idx].rcOFori = prj.cnstRoughnessC;
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
    if (prj.fpnSTVat == "" || _access(prj.fpnSTVat.c_str(), 0) != 0) {//�Ӽ� ���� ���� ���� ������ �ִ��� ����.
        string outstr = "Soil texture VAT file (" + prj.fpnSTVat + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
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
    //���� ���� ���� ������ �����ϱ� ����, ���ķ� ���� �ʴ´�.
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvais[cx][ry];
            if (idx >= 0) {
                int v = stFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    if (stvat.find(v) != stvat.end()) {// ���� ������ Ű�� ��ϵǾ� �ִ��� Ȯ��
                        soilTextureInfo st = stvat[v];
                        vBak = v; // ���⼭ �ֽ� ���� ��
                        cvs[idx].stCellValue = v;
                        cvs[idx].porosity_EtaOri = st.porosity;
                        cvs[idx].effPorosity_ThetaEori = st.effectivePorosity;
                        cvs[idx].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
                        cvs[idx].hc_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
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
                else { // �������� �������� ��(>0)�� �ԷµǾ� ���� ������.. ���� ������(�ֽ���) ������ �����Ѵ�.
                    soilTextureInfo st = stvat[vBak];
                    cvs[idx].stCellValue = vBak;
                    cvs[idx].porosity_EtaOri = st.porosity;
                    cvs[idx].effPorosity_ThetaEori = st.effectivePorosity;
                    cvs[idx].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
                    cvs[idx].hc_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                    cvs[idx].stCode = st.stCode;
                }
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
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvais[cx][ry];
            if (idx >= 0) {
                cvs[idx].stCellValue = 0;// �� ���� ����� �ǹ��ϰ� �Ѵ�.
                cvs[idx].porosity_EtaOri = prj.cnstSoilPorosity;
                cvs[idx].effPorosity_ThetaEori = prj.cnstSoilEffPorosity;
                cvs[idx].wfsh_PsiOri_m = prj.cnstSoilWFSH / 100.0;  // cm -> m
                cvs[idx].hc_Kori_mPsec = prj.cnstSoilHydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
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
    if (prj.fpnSDVat == "" || _access(prj.fpnSDVat.c_str(), 0) != 0) {//�Ӽ� ���� ���� ���� ������ �ִ��� ����.
        string outstr = "Soil depth VAT file (" + prj.fpnSDVat + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
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
    //���� ���� ���� ������ �����ϱ� ����, ���ķ� ���� �ʴ´�.
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvais[cx][ry];
            if (idx >= 0) {
                int v = sdFile.valuesFromTL[cx][ry];
                if (v > 0) {
                    if (sdvat.find(v) != sdvat.end()) {// ���� ������ Ű�� ��ϵǾ� �ִ��� Ȯ��
                        soilDepthInfo sd = sdvat[v];
                        vBak = v; // ���⼭ �ֽ� ���� ��
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
                else { // �������� �������� ��(>0)�� �ԷµǾ� ���� ������.. ���� ������(�ֽ���) ������ �����Ѵ�.
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
#pragma omp parallel for schedule(guided)
    for (int ry = 0; ry < nRy; ry++) {
        for (int cx = 0; cx < nCx; cx++) {
            int idx = cvais[cx][ry];
            if (idx >= 0) {
                cvs[idx].sdCellValue = 0;// �� ���� ����� �ǹ��ϰ� �Ѵ�.
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
    initWatershedNetwork(); // ���⼭ ���� ��Ʈ��ũ ���� �ʱ�ȭ
#pragma omp parallel for schedule(guided)
    //���⼭ ���� ��Ʈ��ũ ���� �ʱ�ȭ
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int idx = cvais[cx][ry];// current cell index
            if (idx >= 0) {
                cvs[idx].neighborCVIDsFlowintoMe.clear();
                cvs[idx].downWPCVIDs.clear();
            }
        }
    }

    double halfDX_Diag_m;
    double halfDXperp_m;
    halfDX_Diag_m = di.cellSize * sqrt(2) / 2.0;
    halfDXperp_m = di.cellSize / 2.0;
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int cidx = cvais[cx][ry];// current cell index
            if (cidx >= 0) {
                double dxe = -1.0;
                int tCx; // �Ϸ����� ��� ���� x array index
                int tRy; // �Ϸ����� ��� ���� y array index
                // �»���� 0,0 �̴�... ��, �����̸�, row-1, �����̸� col +1
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
                    //�Ϸ� ���� ��ü raster ���� �����̸�,
                    if (cvais[tCx][tRy] == -1) {// �Ϸ� ���� effect �� ���� �ܺο� ������,
                        int wsidKey = cvs[cidx].wsid; // �̰� ���缿�� ���Ե� ws�� id
                        //di.wsn.wsOutletCVids �� readDomainFileAndSetupCV() ���� �ʱ�ȭ �Ǿ� �ִ�.
                        if (di.wsn.wsOutletCVID.find(wsidKey) == di.wsn.wsOutletCVID.end() ||
                            cvs[cidx].fac > cvs[di.wsn.wsOutletCVID[wsidKey] - 1].fac) {
                            // ���� ws�� ���� outlet���� �������� �ʾҰų�, 
                            //�̹� �����Ǿ� �ִ� ���� fac ���� ���� ���� fac�� ũ��
                            di.wsn.wsOutletCVID[wsidKey] = cvs[cidx].cvid;
                            di.wsn.wsidNearbyDown[cvs[cidx].wsid] = -1;// �Ϸ� ���� eff ���� �ܺ��̸�, tidx�� cvs�� ������ �����..
                        }
                    }
                    else {
                        int tidx = cvais[tCx][tRy]; // target cell index
                        cvs[tidx].neighborCVIDsFlowintoMe.push_back(cvs[cidx].cvid);
                        cvs[tidx].dxWSum = cvs[tidx].dxWSum + dxe;
                        cvs[cidx].downCellidToFlow = cvs[tidx].cvid;// �귯�� ������ cellid�� ���� ���� ������ ���
                        if (cvs[cidx].wsid != cvs[tidx].wsid) {
                            if (di.wsn.wsidNearbyDown[cvs[cidx].wsid] != cvs[tidx].wsid) {
                                di.wsn.wsidNearbyDown[cvs[cidx].wsid] = cvs[tidx].wsid; 
                                di.wsn.wsOutletCVID[cvs[cidx].wsid] = cvs[cidx].cvid;
                                int oid = di.wsn.wsOutletCVID[cvs[cidx].wsid];
                                int a = 1;
                            }
                            vector<int> v = di.wsn.wsidsNearbyUp[cvs[tidx].wsid];
                            if (std::find(v.begin(), v.end(), cvs[cidx].wsid) == v.end()) {
                                di.wsn.wsidsNearbyUp[cvs[tidx].wsid].push_back(cvs[cidx].wsid);
                            }
                        }
                    }
                    cvs[cidx].dxDownHalf_m = dxe;
                }
                else {// �Ϸ����� ��ü raster ���� ���� �ܺ��̸�,
                    cvs[cidx].downCellidToFlow = -1;
                    cvs[cidx].dxDownHalf_m = dxe;
                    int wsidKey = cvs[cidx].wsid; // �̰� ���缿�� ���Ե� ws�� id
                    if (di.wsn.wsOutletCVID.find(wsidKey) == di.wsn.wsOutletCVID.end() ||
                        cvs[cidx].fac > cvs[di.wsn.wsOutletCVID[wsidKey] - 1].fac) {
                        // ���� ws�� ���� outlet���� �������� �ʾҰų�, 
                        //�̹� �����Ǿ� �ִ� ���� fac ���� ���� ���� fac�� ũ��
                        di.wsn.wsOutletCVID[wsidKey] = cvs[cidx].cvid;
                        di.wsn.wsidNearbyDown[cvs[cidx].wsid] = -1;
                    }
                }
            }
        }
    }
    if (updateWatershedNetwork() == -1) {
        return -1;
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
    //for (int n = 0; n < di.dmids.size(); n++) {
    //    vector<int> v;
    //    int wsid = di.dmids[n];
    //    di.wsn.wsidsNearbyUp[wsid] = v;
    //    di.wsn.wsidNearbyDown[wsid] = -1;
    //    di.wsn.wsidsAllUp[wsid] = v;
    //    di.wsn.wsidsAllDown[wsid] = v;
    //    di.wsn.wsOutletCVID[wsid] = -1;
    //    di.wsn.mdWSIDofCurrentWS[wsid] = -1;
    //}
    return 1;
}

int updateWatershedNetwork()
{
    for (int wsid_cur : di.dmids) { //�켱 ������ ���� id�� �߰��ϰ�
        for (int wsid_nu : di.wsn.wsidsNearbyUp[wsid_cur]) {
            di.wsn.wsidsAllUp[wsid_cur].push_back(wsid_nu);
        }
        int wsid_nd = di.wsn.wsidNearbyDown[wsid_cur];
        if (wsid_nd > 0) {
            di.wsn.wsidsAllDown[wsid_cur].push_back(wsid_nd);
        }
    }

    for (int wsid_cur : di.dmids) {// ���Ϸ� wsid�� �߰��Ѵ�.
        vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];//���⼭ wsid_cur�� ���� key�� ����, key�� �߰��ǰ�, size 0 �� �����ȴ�.
        if (upIDs.size() == 0) {
            di.wsn.wsidsAllUp[wsid_cur].push_back(-1); // �׷��� -1�� ������ �߰��Ѵ�.
        }
        vector<int> downIDs = di.wsn.wsidsAllDown[wsid_cur];
        if (downIDs.size() == 0) {
            di.wsn.wsidsAllDown[wsid_cur].push_back(-1);
        }
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

    for (int wsid_cur : di.dmids) {//���Ϸ� wsid ����� �����
        if (di.wsn.wsidNearbyDown[wsid_cur] == -1) {
            di.wsn.mdWSIDs.push_back(wsid_cur);
        }
    }

    for (int wsid_cur : di.dmids) {//Ư�� wsid�� ���� ���Ϸ� wsid�� �����Ѵ�.
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


