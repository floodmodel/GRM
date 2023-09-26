#include "stdafx.h"
#include "gentle.h"
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern projectFile prj;
extern grmOutFiles ofs;
extern fs::path fpnLog;

extern cvAtt* cvs;
extern int** cvais;
extern domaininfo di;

//const int CONST_imgH = 370;
//const int CONST_imgW = 370;
thread* th_makeASC_ssr;
thread* th_makeASC_rf;
thread* th_makeASC_rfacc;
thread* th_makeASC_flow;

thread* th_makeASC_pet;
thread* th_makeASC_et;
thread* th_makeASC_intcp;
thread* th_makeASC_sm;

thread* th_makeIMG_ssr;
thread* th_makeIMG_rf;
thread* th_makeIMG_rfacc;
thread* th_makeIMG_flow;

thread* th_makeIMG_pet;
thread* th_makeIMG_et;
thread* th_makeIMG_intcp;
thread* th_makeIMG_sm;

double** ssrAry; // 토양포화도 2차원 배열
double** rfAry;  // 강우 2차원 배열
double** rfaccAry; // 누적강우 2차원배열
double** QAry; // 순간 유량 2차원 배열
//double** ssrAryL;
//double** rfAryL;
//double** rfaccAryL;
//double** QAryL;
int bssr =0;
int brf = 0;
int brfacc = 0;
int bQ = 0;

int bpet = 0;
int bet = 0;
int bintcp = 0;
int bsm = 0;

int bimg = 0;
int basc = 0;
string fpn_ssr_img = "";
string fpn_rf_img = "";
string fpn_rfacc_img = "";
string fpn_Q_img = "";

string fpn_pet_img = "";
string fpn_et_img = "";
string fpn_intcp_img = "";
string fpn_sm_img = "";

string fpn_ssr_asc = "";
string fpn_rf_asc = "";
string fpn_rfacc_asc = "";
string fpn_Q_asc = "";

string fpn_pet_asc = "";
string fpn_et_asc = "";
string fpn_intcp_asc = "";
string fpn_sm_asc = "";


void initRasterOutput()
{
    if (prj.makeSoilSaturationDistFile == 1) {
        ssrAry = new double* [di.nCols];
        for (int i = 0; i < di.nCols; ++i) {
            ssrAry[i] = new double[di.nRows];
        }
    }
    if (prj.makeRfDistFile == 1) {
        rfAry = new double* [di.nCols];
        for (int i = 0; i < di.nCols; ++i) {
            rfAry[i] = new double[di.nRows];
        }
    }
    if (prj.makeRFaccDistFile == 1) {
        rfaccAry = new double* [di.nCols];
        for (int i = 0; i < di.nCols; ++i) {
            rfaccAry[i] = new double[di.nRows];
        }
    }
    if (prj.makeFlowDistFile == 1) {
        QAry = new double* [di.nCols];
        for (int i = 0; i < di.nCols; ++i) {
            QAry[i] = new double[di.nRows];
        }
    }
    bssr = prj.makeSoilSaturationDistFile;
    brf = prj.makeRfDistFile;
    brfacc = prj.makeRFaccDistFile;
    bQ = prj.makeFlowDistFile;

    bimg = prj.makeIMGFile;
    basc = prj.makeASCFile;
}

int makeRasterOutput(int nowTmin)
{
    string tToP = "";
    if (prj.isDateTimeFormat == 1) {
        tToP = timeElaspedToDateTimeFormat2(prj.simStartTime,
            nowTmin * 60, timeUnitToShow::toM, 
            dateTimeFormat::yyyymmddHHMMSS);
	}
	else {
		double tsec = nowTmin / 60.0;
		tToP = dtos(tsec, 2);
	}
    setRasterOutputArray();
    if (bssr == 1) {
        if (fs::exists(ofs.ofpSSRDistribution) == false) {
            cout << "ERROR : The folder for soil saturation ratio distribution file is not exist. " << endl;
            return -1;
        }
        if (bimg == 1) {
            fpn_ssr_img = ofs.ofpSSRDistribution + "\\"
                + CONST_DIST_SSR_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_ssr = new thread(makeIMG_ssr);
        }
        if (basc == 1) {
            fpn_ssr_asc = ofs.ofpSSRDistribution + "\\"
                + CONST_DIST_SSR_FILE_HEAD + tToP + ".asc";
            th_makeASC_ssr = new thread(makeASC_ssr);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_ssr_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }            
        }
    }

    if (brf == 1) {
        if (fs::exists(ofs.ofpPRCPDistribution) == false) {
            cout << "ERROR : The folder for rainfall distribution file is not exist. " << endl;
            return -1;
        }
        if (bimg == 1) {
            fpn_rf_img = ofs.ofpPRCPDistribution + "\\"
                + CONST_DIST_RF_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_rf = new thread(makeIMG_rf);
        }
        if (basc == 1) {
            fpn_rf_asc = ofs.ofpPRCPDistribution + "\\"
                + CONST_DIST_RF_FILE_HEAD + tToP + ".asc";
            th_makeASC_rf = new thread(makeASC_rf);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_rf_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }
        }
    }

    if (brfacc == 1) {
        if (fs::exists(ofs.ofpPRCPAccDistribution) == false) {
            cout << "ERROR : The folder for cumulative rainfall distribution file is not exist. " << endl;
            return -1;
        }
        if (bimg == 1) {
            fpn_rfacc_img = ofs.ofpPRCPAccDistribution + "\\"
                + CONST_DIST_RFACC_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_rfacc = new thread(makeIMG_rfacc);
        }
        if (basc == 1) {
            fpn_rfacc_asc = ofs.ofpPRCPAccDistribution + "\\"
                + CONST_DIST_RFACC_FILE_HEAD + tToP + ".asc";
            th_makeASC_rfacc = new thread(makeASC_rfacc);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_rfacc_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }
        }
    }
    if (bQ == 1) {
        if (fs::exists(ofs.ofpFlowDistribution) == false) {
            cout << "ERROR : The folder for flow distribution file is not exist. " << endl;
            return -1;
        }
        if (bimg == 1) {
            fpn_Q_img = ofs.ofpFlowDistribution + "\\"
                + CONST_DIST_FLOW_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_flow = new thread(makeIMG_flow);
        }
        if (basc == 1) {
            fpn_Q_asc = ofs.ofpFlowDistribution + "\\"
                + CONST_DIST_FLOW_FILE_HEAD + tToP + ".asc";
            th_makeASC_flow = new thread(makeASC_flow);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_Q_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }
        }
    }

	if (bpet == 1) {
		if (fs::exists(ofs.ofpPETDistribution) == false) {
			cout << "ERROR : The folder for potential evapotranspiration distribution file is not exist. " << endl;
			return -1;
		}
		if (bimg == 1) {
			fpn_pet_img = ofs.ofpPETDistribution + "\\"
				+ CONST_DIST_PET_FILE_HEAD + tToP + ".bmp";
			th_makeIMG_pet = new thread(makeIMG_flow);
		}
		if (basc == 1) {
			fpn_pet_asc = ofs.ofpPETDistribution + "\\"
				+ CONST_DIST_PET_FILE_HEAD + tToP + ".asc";
			th_makeASC_pet = new thread(makeASC_flow);
			if (prj.fpnProjection != "") {
				string ofpn = replaceText(fpn_pet_asc, ".asc", ".prj");
				fs::copy(prj.fpnProjection, ofpn);
			}
		}
	}
	if (bet == 1) {
		if (fs::exists(ofs.ofpETDistribution) == false) {
			cout << "ERROR : The folder for evapotranspiration distribution file is not exist. " << endl;
			return -1;
		}
		if (bimg == 1) {
			fpn_et_img = ofs.ofpETDistribution + "\\"
				+ CONST_DIST_ET_FILE_HEAD + tToP + ".bmp";
			th_makeIMG_et = new thread(makeIMG_flow);
		}
		if (basc == 1) {
			fpn_et_asc = ofs.ofpETDistribution + "\\"
				+ CONST_DIST_ET_FILE_HEAD + tToP + ".asc";
			th_makeASC_et = new thread(makeASC_flow);
			if (prj.fpnProjection != "") {
				string ofpn = replaceText(fpn_et_asc, ".asc", ".prj");
				fs::copy(prj.fpnProjection, ofpn);
			}
		}
	}
	if (bintcp == 1) {
		if (fs::exists(ofs.ofpINTCPDistribution) == false) {
			cout << "ERROR : The folder for interception distribution file is not exist. " << endl;
			return -1;
		}
		if (bimg == 1) {
			fpn_intcp_img = ofs.ofpINTCPDistribution + "\\"
				+ CONST_DIST_INTERCEPTION_FILE_HEAD + tToP + ".bmp";
			th_makeIMG_intcp = new thread(makeIMG_flow);
		}
		if (basc == 1) {
			fpn_intcp_asc = ofs.ofpINTCPDistribution + "\\"
				+ CONST_DIST_INTERCEPTION_FILE_HEAD + tToP + ".asc";
			th_makeASC_intcp = new thread(makeASC_flow);
			if (prj.fpnProjection != "") {
				string ofpn = replaceText(fpn_intcp_asc, ".asc", ".prj");
				fs::copy(prj.fpnProjection, ofpn);
			}
		}
	}
	if (bsm == 1) {
		if (fs::exists(ofs.ofpSnowMDistribution) == false) {
			cout << "ERROR : The folder for interception distribution file is not exist. " << endl;
			return -1;
		}
		if (bimg == 1) {
			fpn_sm_img = ofs.ofpSnowMDistribution + "\\"
				+ CONST_DIST_SNOWMELT_FILE_HEAD + tToP + ".bmp";
			th_makeIMG_sm = new thread(makeIMG_flow);
		}
		if (basc == 1) {
			fpn_sm_asc = ofs.ofpSnowMDistribution + "\\"
				+ CONST_DIST_SNOWMELT_FILE_HEAD + tToP + ".asc";
			th_makeASC_sm = new thread(makeASC_flow);
			if (prj.fpnProjection != "") {
				string ofpn = replaceText(fpn_sm_asc, ".asc", ".prj");
				fs::copy(prj.fpnProjection, ofpn);
			}
		}
	}
    joinOutputThreads(); // 이과정 있어야, raster 파일 모두 쓰고난 후 프로그램 종료된다.
}


int setRasterOutputArray()
{
#pragma omp parallel for 
    for (int ry = 0; ry < di.nRows; ++ry) {
        for (int cx = 0; cx < di.nCols; ++cx) {
            int i = cvais[cx][ry];
            if (i > 0 && cvs[i].toBeSimulated == 1) {
                if (bssr == 1) {
                    ssrAry[cx][ry] = cvs[i].ssr;
                }
                if (brf == 1) {
                    rfAry[cx][ry] = cvs[i].rf_dtPrint_m * 1000;
                }
                if (brfacc == 1) {
                    rfaccAry[cx][ry] = cvs[i].rfAccRead_fromStart_m * 1000;
                }
                if (bQ == 1) {
                    double v;
                    if (cvs[i].flowType == cellFlowType::OverlandFlow) {
                        v = cvs[i].QOF_m3Ps;
                    }
                    else {
                        v = cvs[i].stream.QCH_m3Ps;
                    }
                    QAry[cx][ry] = v;
                }
                cvs[i].rf_dtPrint_m = 0; // 여기서 바로 초기화 한다.
            }
            else {
                if (bssr == 1) {
                    ssrAry[cx][ry] = -9999;
                }
                if (brf == 1) {
                    rfAry[cx][ry] = -9999;
                }
                if (brfacc == 1) {
                    rfaccAry[cx][ry] = -9999;
                }
                if (bQ == 1) {
                    QAry[cx][ry] = -9999;
                }
            }
        }
    }
    return 1;
}

void makeIMG_ssr()
{
    makeBMPFileUsingArrayGTzero_InParallel( fpn_ssr_img, ssrAry,
        di.nCols, di.nRows, rendererType::Risk, 1.0, di.nodata_value);
}

void makeIMG_rf()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpn_rf_img, rfAry,
        di.nCols, di.nRows, rendererType::Depth, 100.0, di.nodata_value);
}

void makeIMG_rfacc()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpn_rfacc_img, rfaccAry,
        di.nCols, di.nRows, rendererType::Depth, 500.0, di.nodata_value);
}

void makeIMG_flow()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpn_Q_img, QAry,
        di.nCols, di.nRows, rendererType::Depth, 2000.0, di.nodata_value);
}

void makeASC_ssr()
{
    makeASCTextFile( fpn_ssr_asc, di.headerStringAll,
       ssrAry, di.nCols, di.nRows, 2, di.nodata_value);
}

void makeASC_rf()
{
    makeASCTextFile(fpn_rf_asc, di.headerStringAll,
        rfAry, di.nCols, di.nRows, 2, di.nodata_value);
}
void makeASC_rfacc()
{
    makeASCTextFile(fpn_rfacc_asc, di.headerStringAll,
        rfaccAry, di.nCols, di.nRows, 2, di.nodata_value);
}

void makeASC_flow()
{
    makeASCTextFile(fpn_Q_asc, di.headerStringAll,
        QAry, di.nCols, di.nRows, 2, di.nodata_value);
}

void joinOutputThreads()
{
    if (th_makeASC_ssr != NULL && th_makeASC_ssr->joinable() == true) {
        th_makeASC_ssr->join();
    }
    if (th_makeASC_rf != NULL && th_makeASC_rf->joinable() == true) {
        th_makeASC_rf->join();
    }
    if (th_makeASC_rfacc != NULL && th_makeASC_rfacc->joinable() == true) {
        th_makeASC_rfacc->join();
    }
    if (th_makeASC_flow != NULL && th_makeASC_flow->joinable() == true) {
        th_makeASC_flow->join();
    }

    if (th_makeIMG_ssr != NULL && th_makeIMG_ssr->joinable() == true) {
        th_makeIMG_ssr->join();
    }
    if (th_makeIMG_rf != NULL && th_makeIMG_rf->joinable() == true) {
        th_makeIMG_rf->join();
    }
    if (th_makeIMG_rfacc != NULL && th_makeIMG_rfacc->joinable() == true) {
        th_makeIMG_rfacc->join();
    }
    if (th_makeIMG_flow != NULL && th_makeIMG_flow->joinable() == true) {
        th_makeIMG_flow->join();
    }
}