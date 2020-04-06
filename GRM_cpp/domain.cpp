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
extern cvAtt* cvsb;


int readDomainFaFileAndSetupCV()
{
    if (prj.fpnDomain == "" || _access(prj.fpnDomain.c_str(), 0) != 0) {
        string outstr = "Domain file (" + prj.fpnDomain + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnFA == "" || _access(prj.fpnFA.c_str(), 0) != 0) {
        string outstr = "Flow accumulation file (" + prj.fpnFA + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }

    //���⼭ di �Ϲݻ��� �ʱ�ȭ, ����===============
    ascRasterFile dmFile = ascRasterFile(prj.fpnDomain);
    ascRasterFile facFile = ascRasterFile(prj.fpnFA);
    di.nRows = dmFile.header.nRows;
    di.nCols = dmFile.header.nCols;
    di.cellSize = dmFile.header.cellsize;
    di.xll = dmFile.header.xllcorner;
    di.yll = dmFile.header.yllcorner;
    di.headerStringAll = dmFile.headerStringAll;
    di.cvidxInEachRegion.clear();
    di.dmids.clear();
    di.facMax = -1;
    di.facMin = INT_MAX;
    //====================================
    cvais = new int* [di.nCols];
    for (int i = 0; i < di.nCols; ++i) {
        cvais[i] = new int[di.nRows];
    }
    vector<cvAtt> cvsv;
    int cvidx = 0;
    // cvidx�� ���������� �ο��ϱ� ���ؼ�, �� ������ ���ķ� ���� �ʴ´�..
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int wsid = (int)dmFile.valuesFromTL[cx][ry];
            if (wsid > 0) {
                cvAtt cv;
                cv.wsid = wsid;
                //cv.idx = cvidx; // CVid�� CV ����Ʈ�� �ε��� ��ȣ ������ �Է�. ��. 1 ���� ����
                cv.flowType = cellFlowType::OverlandFlow; // �켱 overland flow�� ����
                if (getVectorIndex(di.dmids, wsid) == -1) {//wsid�� vector�� ������, �߰��Ѵ�.
                    di.dmids.push_back(wsid);
                }
                di.cvidxInEachRegion[wsid].push_back(cvidx);
                cv.toBeSimulated = 1;
                cv.idx_xc = cx; 
                cv.idx_yr = ry; 
                cv.fac = (int)facFile.valuesFromTL[cx][ry];
                if (cv.fac > di.facMax) {
                    di.facMax = cv.fac;
                    di.cvidxMaxFac = cvidx;
                }
                if (cv.fac < di.facMin) {
                    di.facMin = cv.fac;
                }
                cvsv.push_back(cv); // ����� ��ȿ����
                cvais[cx][ry] = cvidx ;// ��缿. 1���� �迭�� �ε����� ����. 
                cvidx += 1;
            }
            else {
                cvais[cx][ry] = -1;// ��缿. 1���� �迭�� �ε����� ����. ���ǿ��� �ܺδ� -1.
            }

        }
    }
    di.cellNnotNull = cvidx;
    cvs = new cvAtt[cvsv.size()];
    cvsb = new cvAtt[cvsv.size()];
    copy(cvsv.begin(), cvsv.end(), cvs);
    return 1;
}

int readSlopeFdirStreamCwCfSsrFileAndSetCV()
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

    if (prj.streamFileApplied == 1
        && (prj.fpnStream == ""
            || _access(prj.fpnStream.c_str(), 0) != 0)) {
        string outstr = "Stream file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.streamFileApplied = -1;
    }

    if (prj.cwFileApplied == 1
        && (prj.fpnChannelWidth == ""
            || _access(prj.fpnChannelWidth.c_str(), 0) != 0)) {
        string outstr = "Channel width file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.cwFileApplied = -1;
    }

    if (prj.icfFileApplied == 1
        && (prj.fpniniChannelFlow == ""
            || _access(prj.fpniniChannelFlow.c_str(), 0) != 0)) {
        string outstr = "Initial stream flow file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.icfFileApplied = -1;
    }

    if (prj.issrFileApplied==1
        &&(prj.fpniniSSR == "" || _access(prj.fpniniSSR.c_str(), 0) != 0)) {
        string outstr = "Initial soil saturation ratio file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.issrFileApplied = -1;
    }

    ascRasterFile slopeFile = ascRasterFile(prj.fpnSlope);
    ascRasterFile fdirFile = ascRasterFile(prj.fpnFD);
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
    int cvCount = di.cellNnotNull;
#pragma omp parallel for //schedule(guided)
    for (int idx = 0; idx < cvCount; ++idx) {
        int cx = cvs[idx].idx_xc;
        int ry = cvs[idx].idx_yr;
        cvs[idx].slope = slopeFile.valuesFromTL[cx][ry];
        if (cvs[idx].slope <= 0.0) {
            cvs[idx].slope = CONST_MIN_SLOPE;
        }
        cvs[idx].fdir = getFlowDirection((int)fdirFile.valuesFromTL[cx][ry], prj.fdType);
        if (prj.streamFileApplied == 1) {
            cvs[idx].stream.chStrOrder = (int)streamFile->valuesFromTL[cx][ry];
            if (cvs[idx].stream.chStrOrder > 0) {
                cvs[idx].isStream = 1;
                cvs[idx].flowType = cellFlowType::ChannelFlow;
            }
            else {
                cvs[idx].isStream = -1;
                cvs[idx].flowType = cellFlowType::OverlandFlow;
            }
        }
        if (prj.cwFileApplied == 1) {
            double v = cwFile->valuesFromTL[cx][ry];
            if (v < 0) {
                cvs[idx].stream.chBaseWidthByLayer = 0.0;
            }
            else {
                cvs[idx].stream.chBaseWidthByLayer = v;
            }
        }
        else {
            cvs[idx].stream.chBaseWidthByLayer = 0.0;
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
        //    }
        //}
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
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)lcFile.valuesFromTL[cvs[i].idx_xc][cvs[i].idx_yr];
        if (v > 0) {
            if (lcvat.find(v) != lcvat.end()) {// ���� ������ Ű�� ��ϵǾ� �ִ��� Ȯ��
                landCoverInfo lc = lcvat[v];
                vBak = v; // ���⼭ �ֽ� ���� ��
                cvs[i].lcCellValue = v;
                cvs[i].rcOFori = lc.RoughnessCoefficient;
                cvs[i].imperviousR = lc.ImperviousRatio;
                cvs[i].lcCode = lc.lcCode;
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
            cvs[i].lcCellValue = vBak;
            cvs[i].rcOFori = lc.RoughnessCoefficient;
            cvs[i].imperviousR = lc.ImperviousRatio;
            cvs[i].lcCode = lc.lcCode;
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
    for (int i = 0; i < di.cellNnotNull; ++i) {
        cvs[i].lcCellValue = 0; // �� ���� ����� �ǹ��ϰ� �Ѵ�.
        cvs[i].rcOFori = prj.cnstRoughnessC;
        cvs[i].imperviousR = prj.cnstImperviousR;
        cvs[i].lcCode = landCoverCode::CONSTV;
    }
    return 1;
}

int readLandCoverFile(string fpnLC, int** cvAryidx, cvAtt* cvs1D, int effCellCount)
{
    if (fpnLC == "" || _access(fpnLC.c_str(), 0) != 0) {
        string outstr = "Land cover file (" + fpnLC + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    ascRasterFile lcFile = ascRasterFile(fpnLC);
    int isnormal = 1;
#pragma omp parallel for schedule(guided)
    for (int i = 0; i < effCellCount; ++i) {
        int v = (int)lcFile.valuesFromTL[cvs[i].idx_xc][cvs[i].idx_yr];;
        if (v > 0) {
            cvs1D[i].lcCellValue = v;
        }
        else {
            string outstr = "Land cover file (" + fpnLC + ") has invalid value.\n";
            writeLog(fpnLog, outstr, -1, 1);
            isnormal = -1;
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
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)stFile.valuesFromTL[cvs[i].idx_xc][cvs[i].idx_yr];
        if (v > 0) {
            if (stvat.find(v) != stvat.end()) {// ���� ������ Ű�� ��ϵǾ� �ִ��� Ȯ��
                soilTextureInfo st = stvat[v];
                vBak = v; // ���⼭ �ֽ� ���� ��
                cvs[i].stCellValue = v;
                cvs[i].porosity_EtaOri = st.porosity;
                cvs[i].effPorosity_ThetaEori = st.effectivePorosity;
                cvs[i].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
                cvs[i].hc_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                cvs[i].stCode = st.stCode;
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
            cvs[i].stCellValue = vBak;
            cvs[i].porosity_EtaOri = st.porosity;
            cvs[i].effPorosity_ThetaEori = st.effectivePorosity;
            cvs[i].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
            cvs[i].hc_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
            cvs[i].stCode = st.stCode;
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
    for (int i = 0; i < di.cellNnotNull; ++i) {
                cvs[i].stCellValue = 0;// �� ���� ����� �ǹ��ϰ� �Ѵ�.
                cvs[i].porosity_EtaOri = prj.cnstSoilPorosity;
                cvs[i].effPorosity_ThetaEori = prj.cnstSoilEffPorosity;
                cvs[i].wfsh_PsiOri_m = prj.cnstSoilWFSH / 100.0;  // cm -> m
                cvs[i].hc_Kori_mPsec = prj.cnstSoilHydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                cvs[i].stCode =  soilTextureCode::CONSTV;
            }
    return 1;
}

int readSoilTextureFile(string fpnST, int** cvAryidx, cvAtt* cvs1D, int effCellCount)
{
    if (fpnST == "" || _access(fpnST.c_str(), 0) != 0) {
        string outstr = "Soil texture file (" + fpnST + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    ascRasterFile stFile = ascRasterFile(fpnST);
    int isnormal = 1;
#pragma omp parallel for schedule(guided)
    for (int i = 0; i < effCellCount; ++i) {
        int v = (int)stFile.valuesFromTL[cvs[i].idx_xc][cvs[i].idx_yr];
        if (v > 0) {
            cvs1D[i].stCellValue = v;
        }
        else {
            string outstr = "Soil texture file (" + fpnST + ") has invalid value.\n";
            writeLog(fpnLog, outstr, -1, 1);
            isnormal = -1;
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
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)sdFile.valuesFromTL[cvs[i].idx_xc][cvs[i].idx_yr];
        if (v > 0) {
            if (sdvat.find(v) != sdvat.end()) {// ���� ������ Ű�� ��ϵǾ� �ִ��� Ȯ��
                soilDepthInfo sd = sdvat[v];
                vBak = v; // ���⼭ �ֽ� ���� ��
                cvs[i].sdCellValue = v;
                cvs[i].sdOri_m = sd.soilDepth / 100.0;   // cm ->  m
                cvs[i].sdCode = sd.sdCode;
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
            cvs[i].sdCellValue = vBak;
            cvs[i].sdOri_m = sd.soilDepth / 100.0;   // cm ->  m
            cvs[i].sdCode = sd.sdCode;
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
    for (int i = 0; i < di.cellNnotNull; ++i) {
        cvs[i].sdCellValue = 0;// �� ���� ����� �ǹ��ϰ� �Ѵ�.
        cvs[i].sdOri_m = prj.cnstSoilDepth / 100.0;   // cm ->  m
        cvs[i].sdCode = soilDepthCode::CONSTV;
    }
    return 1;
}

int readSoilDepthFile(string fpnSD, int** cvAryidx, cvAtt* cvs1D, int effCellCount)
{
    if (fpnSD == "" || _access(fpnSD.c_str(), 0) != 0) {
        string outstr = "Soil depth file (" + fpnSD + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    ascRasterFile sdFile = ascRasterFile(fpnSD);
    int isnormal = 1;
#pragma omp parallel for schedule(guided)
    for (int i = 0; i < effCellCount; ++i) {
        int v = (int)sdFile.valuesFromTL[cvs[i].idx_xc][cvs[i].idx_yr];
        if (v > 0) {
            cvs1D[i].sdCellValue = v;
        }
        else {
            string outstr = "Soil depth file (" + fpnSD + ") has invalid value.\n";
            writeLog(fpnLog, outstr, -1, 1);
            isnormal = -1;
        }
    }
    return isnormal;
}

int setFlowNetwork()
{
    initWatershedNetwork(); // ���⼭ ���� ��Ʈ��ũ ���� �ʱ�ȭ
#pragma omp parallel for schedule(guided)
    //���⼭ ���� ��Ʈ��ũ ���� �ʱ�ȭ
    for (int i = 0; i < di.cellNnotNull; ++i) {
        cvs[i].neighborCVidxFlowintoMe.clear();
        cvs[i].downWPCVidx.clear();
        if (di.facMax == di.cellNnotNull) {
            cvs[i].fac = cvs[i].fac - 1;
        }
    }
    if (di.facMax == di.cellNnotNull) {// �̰��� �ֻ�� �� fac=1�� ���, �̰��� fac=0���� �����.
        di.facMax = di.facMax - 1;
        di.facMin = di.facMin - 1;
    }
    
    double halfDX_Diag_m;
    double halfDXperp_m;
    halfDX_Diag_m = di.cellSize * sqrt(2) / 2.0;
    halfDXperp_m = di.cellSize / 2.0;
    for (int i = 0; i < di.cellNnotNull; ++i) {
        double dxe = 0.0;
        int tCx; // �Ϸ����� ��� ���� x array index
        int tRy; // �Ϸ����� ��� ���� y array index
        // �»���� 0,0 �̴�... ��, �����̸�, row-1, �����̸� col +1
        int cx = cvs[i].idx_xc;
        int ry = cvs[i].idx_yr;
        switch (cvs[i].fdir) {
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
                int wsidKey = cvs[i].wsid; // �̰� ���缿�� ���Ե� ws�� id
                //di.wsn.wsOutletCVids �� readDomainFileAndSetupCV() ���� �ʱ�ȭ �Ǿ� �ִ�.
                if (di.wsn.wsOutletidxs.find(wsidKey) == di.wsn.wsOutletidxs.end() ||
                    cvs[i].fac > cvs[di.wsn.wsOutletidxs[wsidKey]].fac) {
                    // ���� ws�� ���� outlet���� �������� �ʾҰų�, 
                    //�̹� �����Ǿ� �ִ� ���� fac ���� ���� ���� fac�� ũ��
                    di.wsn.wsOutletidxs[wsidKey] = i;
                    di.wsn.wsidNearbyDown[cvs[i].wsid] = -1;// �Ϸ� ���� eff ���� �ܺ��̸�, tidx�� cvs�� ������ �����..
                }
            }
            else {
                int tidx = cvais[tCx][tRy]; // target cell index
                cvs[tidx].neighborCVidxFlowintoMe.push_back(i);
                cvs[tidx].dxWSum = cvs[tidx].dxWSum + dxe;
                cvs[i].downCellidxToFlow = tidx;// �귯�� ������ cellid�� ���� ���� ������ ���
                if (cvs[i].wsid != cvs[tidx].wsid) {
                    if (di.wsn.wsidNearbyDown[cvs[i].wsid] != cvs[tidx].wsid) {
                        di.wsn.wsidNearbyDown[cvs[i].wsid] = cvs[tidx].wsid;
                        di.wsn.wsOutletidxs[cvs[i].wsid] = i;
                    }
                    vector<int> v = di.wsn.wsidsNearbyUp[cvs[tidx].wsid];
                    if (std::find(v.begin(), v.end(), cvs[i].wsid) == v.end()) {
                        di.wsn.wsidsNearbyUp[cvs[tidx].wsid].push_back(cvs[i].wsid);
                    }
                }
            }
            cvs[i].dxDownHalf_m = dxe;
        }
        else {// �Ϸ����� ��ü raster ���� ���� �ܺ��̸�,
            cvs[i].downCellidxToFlow = -1;
            cvs[i].dxDownHalf_m = dxe;
            int wsidKey = cvs[i].wsid; // �̰� ���缿�� ���Ե� ws�� id
            if (di.wsn.wsOutletidxs.find(wsidKey) == di.wsn.wsOutletidxs.end() ||
                cvs[i].fac > cvs[di.wsn.wsOutletidxs[wsidKey]].fac) {
                // ���� ws�� ���� outlet���� �������� �ʾҰų�, 
                //�̹� �����Ǿ� �ִ� ���� fac ���� ���� ���� fac�� ũ��
                di.wsn.wsOutletidxs[wsidKey] = i;
                di.wsn.wsidNearbyDown[cvs[i].wsid] = -1;
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
    di.wsn.wsOutletidxs.clear();
    di.wsn.mdWSIDofCurrentWS.clear();
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


