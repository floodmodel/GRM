#include "stdafx.h"
#include "gentle.h"
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern int** cvais;
extern cvAtt* cvs;
extern cvpos* cvps;
extern cvAtt* cvsb;

int readDomainFaFileAndSetupCV()
{
    if (prj.fpnDomain == "" || _access(prj.fpnDomain.c_str(), 0) != 0) {
        string outstr = "ERROR : Domain file (" + prj.fpnDomain + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnFA == "" || _access(prj.fpnFA.c_str(), 0) != 0) {
        string outstr = "ERROR : Flow accumulation file (" + prj.fpnFA + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }

    //여기서 di 일반사항 초기화, 설정===============
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
    vector<cvpos> cvpv;
    int cvidx = 0;
    // cvidx를 순차적으로 부여하기 위해서, 이 과정은 병렬로 하지 않는다..
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int wsid = (int)dmFile.valuesFromTL[cx][ry];
            if (wsid > 0) {
                cvAtt cv;
                cvpos cp;
                //cv.idx = cvidx; // CVid를 CV 리스트의 인덱스 번호 값으로 입력. 즉. 1 부터 시작
                cv.flowType = cellFlowType::OverlandFlow; // 우선 overland flow로 설정
                if (getVectorIndex(di.dmids, wsid) == -1) {//wsid가 vector에 없으면, 추가한다.
                    di.dmids.push_back(wsid);
                }
                di.cvidxInEachRegion[wsid].push_back(cvidx);
                cv.toBeSimulated = 1;
                cv.fac = (int)facFile.valuesFromTL[cx][ry];
                if (cv.fac > di.facMax) {
                    di.facMax = cv.fac;
                    di.cvidxMaxFac = cvidx;
                }
                if (cv.fac < di.facMin) {
                    di.facMin = cv.fac;
                }
                cp.wsid = wsid;
                cp.xCol = cx;
                cp.yRow = ry;
                cvsv.push_back(cv); // 여기는 유효셀만
                cvpv.push_back(cp);
                cvais[cx][ry] = cvidx ;// 모든셀. 1차원 배열의 인덱스를 저장. 
                cvidx += 1;
            }
            else {
                cvais[cx][ry] = -1;// 모든셀. 1차원 배열의 인덱스를 저장. 모의영역 외부는 -1.
            }

        }
    }
    di.cellNnotNull = cvidx;
    cvs = new cvAtt[cvsv.size()];
    cvps = new cvpos[cvsv.size()];
    cvsb = new cvAtt[cvsv.size()];
    copy(cvsv.begin(), cvsv.end(), cvs);
    copy(cvpv.begin(), cvpv.end(), cvps);
    return 1;
}

int readSlopeFdirStreamCwCfSsrFileAndSetCV()
{
    if (prj.fpnSlope == "" || _access(prj.fpnSlope.c_str(), 0) != 0) {
        string outstr = "ERROR : Slope file (" + prj.fpnSlope + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnFD == "" || _access(prj.fpnFD.c_str(), 0) != 0) {
        string outstr = "ERROR : Flow direction file (" + prj.fpnFD + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }

    if (prj.streamFileApplied == 1
        && (prj.fpnStream == ""
            || _access(prj.fpnStream.c_str(), 0) != 0)) {
        string outstr = "WARNNING : Stream file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.streamFileApplied = -1;
    }

    if (prj.cwFileApplied == 1
        && (prj.fpnChannelWidth == ""
            || _access(prj.fpnChannelWidth.c_str(), 0) != 0)) {
        string outstr = "WARNNING : Channel width file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.cwFileApplied = -1;
    }

    if (prj.icfFileApplied == 1
        && (prj.fpniniChFlow == ""
            || _access(prj.fpniniChFlow.c_str(), 0) != 0)) {
        string outstr = "WARNNING : Initial stream flow file is invalid. Simulation continues.\n";
        writeLog(fpnLog, outstr, 1, -1);
        prj.icfFileApplied = -1;
    }

    if (prj.issrFileApplied==1
        &&(prj.fpniniSSR == "" || _access(prj.fpniniSSR.c_str(), 0) != 0)) {
        string outstr = "WARNNING : Initial soil saturation ratio file is invalid. Simulation continues.\n";
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
        cfFile = new ascRasterFile(prj.fpniniChFlow);
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
	int isNormal = 1; // -1: false, 1: true
#pragma omp parallel for //schedule(guided)
    for (int idx = 0; idx < cvCount; ++idx) {
        int cx = cvps[idx].xCol;
        int ry = cvps[idx].yRow;
        cvs[idx].slope = slopeFile.valuesFromTL[cx][ry];
        if (cvs[idx].slope <= 0.0) {
            cvs[idx].slope = CONST_MIN_SLOPE;
        }
        cvs[idx].fdir = getFlowDirection((int)fdirFile.valuesFromTL[cx][ry], prj.fdType);
		if (cvs[idx].fdir == flowDirection8::None8) {
			string outstr = "ERROR : Flow direction value is invalid (cell location : "
				+ to_string(cvps[idx].xCol) + ", " + to_string(cvps[idx].yRow) + ").\n";
			writeLog(fpnLog, outstr, 1, 1);
			isNormal= -1;
		}

        if (prj.streamFileApplied == 1) {
            cvs[idx].stream.cellValue = (int)streamFile->valuesFromTL[cx][ry];
            if (cvs[idx].stream.cellValue > 0) {
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
	if (isNormal == -1) {
		return -1;
	}
    return 1;
}


int readLandCoverFileAndSetCVbyVAT()
{
    if (prj.fpnLC == "" || _access(prj.fpnLC.c_str(), 0) != 0) {
        string outstr = "ERROR : Land cover file (" + prj.fpnLC + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnLCVat == "" || _access(prj.fpnLCVat.c_str(), 0) != 0) {//속성 대응 참고를 위해 이파일 있는지 본다.
        string outstr = "ERROR : Land cover VAT file (" + prj.fpnLCVat + ") is invalid.\n";
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
    //오류 값에 인접 셀값을 설정하기 위해, 병렬로 하지 않는다.
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)lcFile.valuesFromTL[cvps[i].xCol][cvps[i].yRow];
        if (v > 0) {
            if (lcvat.find(v) != lcvat.end()) {// 현재 셀값이 키로 등록되어 있는지 확인
                landCoverInfo lc = lcvat[v];
                vBak = v; // 여기서 최신 셀의 값
                cvs[i].lcCellValue = v;
                cvs[i].rcOFori = lc.RoughnessCoefficient;
                cvs[i].imperviousR = lc.ImperviousRatio;
				cvs[i].canopyR = lc.CanopyRatio;
				cvs[i].intcpMaxWaterCanopy_m = lc.InterceptionMaxWaterCanopy_mm / 1000.0;
                cvs[i].lcCode = lc.lcCode;
            }
            else {
                string outstr = "ERROR : Landcover VAT file [" + prj.fpnLCVat
                    + "] or current project file do not have the land cover value ("
                    + to_string(v) + ").\n"
                    + "Check the land cover file or land cover VAT file. \n";
                writeLog(fpnLog, outstr, -1, 1);
                return -1;
            }
        }
        else { // 셀값으로 정상적인 값(>0)이 입력되어 있지 않으면.. 가장 인접한(최신의) 값으로 설정한다.
            landCoverInfo lc = lcvat[vBak];
            cvs[i].lcCellValue = vBak;
            cvs[i].rcOFori = lc.RoughnessCoefficient;
            cvs[i].imperviousR = lc.ImperviousRatio;
            cvs[i].lcCode = lc.lcCode;
        }
    }
	lcvat.clear();
    return 1;
}

int setCVbyLCConstant()
{
    if (prj.cnstImperviousR == -1) {
        string outstr = "ERROR : Land cover constant impervious ratio is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstRoughnessC == -1) {
        string outstr = "ERROR : Land cover constant roughness coefficient is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    int nRy = di.nRows;
    int nCx = di.nCols;
#pragma omp parallel for //schedule(guided)
    for (int i = 0; i < di.cellNnotNull; ++i) {
        cvs[i].lcCellValue = 0; // 이 값은 상수를 의미하게 한다.
        cvs[i].rcOFori = prj.cnstRoughnessC;
        cvs[i].imperviousR = prj.cnstImperviousR;
        cvs[i].lcCode = landCoverCode::CONSTV;
    }
    return 1;
}

int readLandCoverFile()
{
    if (prj.fpnLC == "" || _access(prj.fpnLC.c_str(), 0) != 0) {
        string outstr = "ERROR : Land cover file (" + prj.fpnLC + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    ascRasterFile lcFile = ascRasterFile(prj.fpnLC);
    int isnormal = 1;
#pragma omp parallel for //schedule(guided)
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)lcFile.valuesFromTL[cvps[i].xCol][cvps[i].yRow];;
        if (v > 0) {
            cvs[i].lcCellValue = v;
        }
        else {
            string outstr = "WARNNING : Land cover file (" + prj.fpnLC + ") has invalid value.\n";
            writeLog(fpnLog, outstr, -1, 1);
            isnormal = -1;
        }
    }
    return isnormal;
}

int readSoilTextureFileAndSetCVbyVAT()
{
    if (prj.fpnST == "" || _access(prj.fpnST.c_str(), 0) != 0) {
        string outstr = "ERROR : Soil texture file (" + prj.fpnST + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnSTVat == "" || _access(prj.fpnSTVat.c_str(), 0) != 0) {//속성 대응 참고를 위해 이파일 있는지 본다.
        string outstr = "WARNNING : Soil texture VAT file (" + prj.fpnSTVat + ") is invalid.\n";
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
    //오류 값에 인접 셀값을 설정하기 위해, 병렬로 하지 않는다.
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)stFile.valuesFromTL[cvps[i].xCol][cvps[i].yRow];
        if (v > 0) {
            if (stvat.find(v) != stvat.end()) {// 현재 셀값이 키로 등록되어 있는지 확인
                soilTextureInfo st = stvat[v];
                vBak = v; // 여기서 최신 셀의 값
                cvs[i].stCellValue = v;
                cvs[i].porosity_EtaOri = st.porosity;
                cvs[i].effPorosity_ThetaEori = st.effectivePorosity;
                cvs[i].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
                cvs[i].hc_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                cvs[i].stCode = st.stCode;
            }
            else {
                string outstr = "ERROR : Soil texture VAT file [" + prj.fpnSTVat
                    + "] or current project file do not have the soil texture value (" + to_string(v) + ").\n"
                    + "Check the soil texture file or soil texture VAT file. \n";
                writeLog(fpnLog, outstr, -1, 1);
                return -1;
            }
        }
        else { // 셀값으로 정상적인 값(>0)이 입력되어 있지 않으면.. 가장 인접한(최신의) 값으로 설정한다.
            soilTextureInfo st = stvat[vBak];
            cvs[i].stCellValue = vBak;
            cvs[i].porosity_EtaOri = st.porosity;
            cvs[i].effPorosity_ThetaEori = st.effectivePorosity;
            cvs[i].wfsh_PsiOri_m = st.WFSuctionHead / 100.0;  // cm -> m
            cvs[i].hc_Kori_mPsec = st.hydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
            cvs[i].stCode = st.stCode;
        }
    }
	stvat.clear();
    return 1;
}

int setCVbySTConstant()
{
    if (prj.cnstSoilEffPorosity == -1) {
        string outstr = "ERROR : Soil texture constant effective porosity is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstSoilHydraulicK == -1) {
        string outstr = "ERROR : Soil texture constant hydraulic conductivity is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstSoilPorosity == -1) {
        string outstr = "ERROR : Soil texture constant porosity is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.cnstSoilWFSH == -1) {
        string outstr = "ERROR : Soil texture constant wetting front suction head is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }

    int nRy = di.nRows;
    int nCx = di.nCols;
#pragma omp parallel for //schedule(guided)
    for (int i = 0; i < di.cellNnotNull; ++i) {
                cvs[i].stCellValue = 0;// 이 값은 상수를 의미하게 한다.
                cvs[i].porosity_EtaOri = prj.cnstSoilPorosity;
                cvs[i].effPorosity_ThetaEori = prj.cnstSoilEffPorosity;
                cvs[i].wfsh_PsiOri_m = prj.cnstSoilWFSH / 100.0;  // cm -> m
                cvs[i].hc_Kori_mPsec = prj.cnstSoilHydraulicK / 100.0 / 3600.0;    // cm/hr -> m/s;
                cvs[i].stCode =  soilTextureCode::CONSTV;
            }
    return 1;
}

int readSoilTextureFile()
{
    if (prj.fpnST == "" || _access(prj.fpnST.c_str(), 0) != 0) {
        string outstr = "ERROR : Soil texture file (" + prj.fpnST + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    ascRasterFile stFile = ascRasterFile(prj.fpnST);
    int isnormal = 1;
#pragma omp parallel for// schedule(guided)
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)stFile.valuesFromTL[cvps[i].xCol][cvps[i].yRow];
        if (v > 0) {
            cvs[i].stCellValue = v;
        }
        else {
            string outstr = "WARNNING : Soil texture file (" + prj.fpnST + ") has invalid value.\n";
            writeLog(fpnLog, outstr, -1, 1);
            isnormal = -1;
        }
    }
    return isnormal;
}


int readSoilDepthFileAndSetCVbyVAT()
{
    if (prj.fpnSD == "" || _access(prj.fpnSD.c_str(), 0) != 0) {
        string outstr = "ERROR : Soil depth file (" + prj.fpnSD + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    if (prj.fpnSDVat == "" || _access(prj.fpnSDVat.c_str(), 0) != 0) {//속성 대응 참고를 위해 이파일 있는지 본다.
        string outstr = "WARNNING : Soil depth VAT file (" + prj.fpnSDVat + ") is invalid.\n";
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
    //오류 값에 인접 셀값을 설정하기 위해, 병렬로 하지 않는다.
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)sdFile.valuesFromTL[cvps[i].xCol][cvps[i].yRow];
        if (v > 0) {
            if (sdvat.find(v) != sdvat.end()) {// 현재 셀값이 키로 등록되어 있는지 확인
                soilDepthInfo sd = sdvat[v];
                vBak = v; // 여기서 최신 셀의 값
                cvs[i].sdCellValue = v;
                cvs[i].sdOri_m = sd.soilDepth / 100.0;   // cm ->  m
                cvs[i].sdCode = sd.sdCode;
            }
            else {
                string outstr = "ERROR : Soil depth VAT file [" + prj.fpnSDVat
                    + "] or current project file do not have the soil depth value ("
                    + to_string(v) + ").\n"
                    + "Check the soil depth file or soil depth VAT file. \n";
                writeLog(fpnLog, outstr, -1, 1);
                return -1;
            }
        }
        else { // 셀값으로 정상적인 값(>0)이 입력되어 있지 않으면.. 가장 인접한(최신의) 값으로 설정한다.
            soilDepthInfo sd = sdvat[vBak];
            cvs[i].sdCellValue = vBak;
            cvs[i].sdOri_m = sd.soilDepth / 100.0;   // cm ->  m
            cvs[i].sdCode = sd.sdCode;
        }
    }
	sdvat.clear();
    return 1;
}

int setCVbySDConstant()
{
    if (prj.cnstSoilDepth == -1) {
        string outstr = "ERROR : Constant soil depth value is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    int nRy = di.nRows;
    int nCx = di.nCols;
#pragma omp parallel for //schedule(guided)
    for (int i = 0; i < di.cellNnotNull; ++i) {
        cvs[i].sdCellValue = 0;// 이 값은 상수를 의미하게 한다.
        cvs[i].sdOri_m = prj.cnstSoilDepth / 100.0;   // cm ->  m
        cvs[i].sdCode = soilDepthCode::CONSTV;
    }
    return 1;
}

int readSoilDepthFile()
{
    if (prj.fpnSD == "" || _access(prj.fpnSD.c_str(), 0) != 0) {
        string outstr = "ERROR : Soil depth file (" + prj.fpnSD + ") is invalid.\n";
        writeLog(fpnLog, outstr, -1, 1);
        return -1;
    }
    ascRasterFile sdFile = ascRasterFile(prj.fpnSD);
    int isnormal = 1;
#pragma omp parallel for //schedule(guided)
    for (int i = 0; i < di.cellNnotNull; ++i) {
        int v = (int)sdFile.valuesFromTL[cvps[i].xCol][cvps[i].yRow];
        if (v > 0) {
            cvs[i].sdCellValue = v;
        }
        else {
            string outstr = "WARNNING : Soil depth file (" + prj.fpnSD + ") has invalid value.\n";
            writeLog(fpnLog, outstr, -1, 1);
            isnormal = -1;
        }
    }
    return isnormal;
}



flowDirection8 getFlowDirection(int fdirV, flowDirectionType fdt)
{
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
	else if (fdt == flowDirectionType::StartsFromNE) {
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
	else if (fdt == flowDirectionType::StartsFromN) {
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
	else if (fdt == flowDirectionType::StartsFromE) {
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
	return flowDirection8::None8;
}


