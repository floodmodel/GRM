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
thread* th_makeASC_aet;
thread* th_makeASC_intcp;
thread* th_makeASC_sm;

thread* th_makeIMG_ssr;
thread* th_makeIMG_rf;
thread* th_makeIMG_rfacc;
thread* th_makeIMG_flow;

thread* th_makeIMG_pet;
thread* th_makeIMG_aet;
thread* th_makeIMG_intcp;
thread* th_makeIMG_sm;

double** ssrAry; // 토양포화도 2차원 배열
double** rfPDTAry;  // 출력 시간간격 동안의 누적강우 2차원 배열. mm
double** rfaccAry; // 누적강우 2차원배열
double** QAry; // 순간 유량 2차원 배열
double** petPDTAry; // 출력 시간간격 동안의 누적 잠재증발산 2차원 배열. mm
double** aetPDTAry; // 출력 시간간격 동안의 누적 실제증발산 2차원 배열. mm


int bssr =0;
int brf = 0;
int brfacc = 0;
int bQ = 0;
int bPET = 0;
int bAET = 0;

int bINTCP = 0;
int bSM = 0;

int bimg = 0;
int basc = 0;
string fpn_ssr_img = "";
string fpn_rfPDT_img = "";
string fpn_rfacc_img = "";
string fpn_Q_img = "";
string fpn_petPDT_img = "";
string fpn_aetPDT_img = "";

string fpn_intcp_img = "";
string fpn_sm_img = "";

string fpn_ssr_asc = "";
string fpn_rfPDT_asc = "";
string fpn_rfacc_asc = "";
string fpn_Q_asc = "";
string fpn_petPDT_asc = "";
string fpn_aetPDT_asc = "";
//string fpn_aet_asc = "";

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
        rfPDTAry = new double* [di.nCols];
        for (int i = 0; i < di.nCols; ++i) {
            rfPDTAry[i] = new double[di.nRows];
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
    if (prj.makePETDistFile == 1) {
        petPDTAry = new double* [di.nCols];
        for (int i = 0; i < di.nCols; ++i) {
            petPDTAry[i] = new double[di.nRows];
        }
    }
    if (prj.makeAETDistFile == 1) {
        aetPDTAry = new double* [di.nCols];
        for (int i = 0; i < di.nCols; ++i) {
            aetPDTAry[i] = new double[di.nRows];
        }
    }
    bssr = prj.makeSoilSaturationDistFile;
    brf = prj.makeRfDistFile;
    brfacc = prj.makeRFaccDistFile;
    bQ = prj.makeFlowDistFile;
    bPET = prj.makePETDistFile;
    bAET = prj.makeAETDistFile;

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
        if (fs::exists(lower(ofs.ofpSSRDistribution)) == false) {
            cout << "ERROR : The folder for soil saturation ratio distribution file is not exist. " << endl;
            return -1;
        }

        if (bimg == 1) {
            fpn_ssr_img = ofs.ofpSSRDistribution + "/"
                + CONST_DIST_SSR_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_ssr = new thread(makeIMG_ssr);
        }

        if (basc == 1) {
            fpn_ssr_asc = ofs.ofpSSRDistribution + "/"
                + CONST_DIST_SSR_FILE_HEAD + tToP + ".asc";
            th_makeASC_ssr = new thread(makeASC_ssr);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_ssr_asc, ".asc", ".prj");
                fs::copy(lower(prj.fpnProjection), ofpn);
            }            
        }
    }

    if (brf == 1) {
        if (fs::exists(lower(ofs.ofpPRCPDistribution)) == false) {
            cout << "ERROR : The folder for rainfall distribution file is not exist. " << endl;
            return -1;
        }

        if (bimg == 1) {
            fpn_rfPDT_img = ofs.ofpPRCPDistribution + "/"
                + CONST_DIST_RF_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_rf = new thread(makeIMG_rf);
        }
        if (basc == 1) {
            fpn_rfPDT_asc = ofs.ofpPRCPDistribution + "/"
                + CONST_DIST_RF_FILE_HEAD + tToP + ".asc";
            th_makeASC_rf = new thread(makeASC_rf);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_rfPDT_asc, ".asc", ".prj");
                fs::copy(lower(prj.fpnProjection), ofpn);
            }
        }
    }

    if (brfacc == 1) {
        if (fs::exists(lower(ofs.ofpPRCPAccDistribution)) == false) {
            cout << "ERROR : The folder for cumulative rainfall distribution file is not exist. " << endl;
            return -1;
        }

        if (bimg == 1) {
            fpn_rfacc_img = ofs.ofpPRCPAccDistribution + "/"
                + CONST_DIST_RFACC_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_rfacc = new thread(makeIMG_rfacc);
        }

        if (basc == 1) {
            fpn_rfacc_asc = ofs.ofpPRCPAccDistribution + "/"
                + CONST_DIST_RFACC_FILE_HEAD + tToP + ".asc";
            th_makeASC_rfacc = new thread(makeASC_rfacc);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_rfacc_asc, ".asc", ".prj");
                fs::copy(lower(prj.fpnProjection), ofpn);
            }
        }
    }
    if (bQ == 1) {
        if (fs::exists(lower(ofs.ofpFlowDistribution)) == false) {
            cout << "ERROR : The folder for flow distribution file is not exist. " << endl;
            return -1;
        }
        if (bimg == 1) {
            fpn_Q_img = ofs.ofpFlowDistribution + "/"
                + CONST_DIST_FLOW_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_flow = new thread(makeIMG_flow);
        }

        if (basc == 1) {
            fpn_Q_asc = ofs.ofpFlowDistribution + "/"
                + CONST_DIST_FLOW_FILE_HEAD + tToP + ".asc";
            th_makeASC_flow = new thread(makeASC_flow);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_Q_asc, ".asc", ".prj");
                fs::copy(lower(prj.fpnProjection), ofpn);
            }
        }
    }

	if (bPET == 1) {
		if (fs::exists(lower(ofs.ofpPETDistribution)) == false) {
			cout << "ERROR : The folder for potential evapotranspiration distribution file is not exist. " << endl;
			return -1;
		}

		if (bimg == 1) {
			fpn_petPDT_img = ofs.ofpPETDistribution + "/"
				+ CONST_DIST_PET_FILE_HEAD + tToP + ".bmp";
			th_makeIMG_pet = new thread(makeIMG_pet);
		}

		if (basc == 1) {
			fpn_petPDT_asc = ofs.ofpPETDistribution + "/"
				+ CONST_DIST_PET_FILE_HEAD + tToP + ".asc";
			th_makeASC_pet = new thread(makeASC_pet);
			if (prj.fpnProjection != "") {
				string ofpn = replaceText(fpn_petPDT_asc, ".asc", ".prj");
				fs::copy(lower(prj.fpnProjection), ofpn);
			}
		}
	}

    if (bAET == 1) {
        if (fs::exists(lower(ofs.ofpAETDistribution)) == false) {
            cout << "ERROR : The folder for actual evapotranspiration distribution file is not exist. " << endl;
            return -1;
        }

        if (bimg == 1) {
            fpn_aetPDT_img = ofs.ofpAETDistribution + "/"
                + CONST_DIST_AET_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_aet = new thread(makeIMG_aet);
        }

        if (basc == 1) {
            fpn_aetPDT_asc = ofs.ofpAETDistribution + "/"
                + CONST_DIST_AET_FILE_HEAD + tToP + ".asc";
            th_makeASC_aet = new thread(makeASC_aet);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpn_aetPDT_asc, ".asc", ".prj");
                fs::copy(lower(prj.fpnProjection), ofpn);
            }
        }
    }
	//if (bintcp == 1) {
	//	if (fs::exists(lower(ofs.ofpINTCPDistribution)) == false) {
	//		cout << "ERROR : The folder for interception distribution file is not exist. " << endl;
	//		return -1;
	//	}

	//	if (bimg == 1) {
	//		fpn_intcp_img = ofs.ofpINTCPDistribution + "/"
	//			+ CONST_DIST_INTERCEPTION_FILE_HEAD + tToP + ".bmp";
	//		th_makeIMG_intcp = new thread(makeIMG_flow);
	//	}

	//	if (basc == 1) {
	//		fpn_intcp_asc = ofs.ofpINTCPDistribution + "/"
	//			+ CONST_DIST_INTERCEPTION_FILE_HEAD + tToP + ".asc";
	//		th_makeASC_intcp = new thread(makeASC_flow);
	//		if (prj.fpnProjection != "") {
	//			string ofpn = replaceText(fpn_intcp_asc, ".asc", ".prj");
	//			fs::copy(lower(prj.fpnProjection), ofpn);
	//		}
	//	}
	//}
	//if (bsm == 1) {
	//	if (fs::exists(lower(ofs.ofpSnowMDistribution)) == false) {
	//		cout << "ERROR : The folder for interception distribution file is not exist. " << endl;
	//		return -1;
	//	}

	//	if (bimg == 1) {
	//		fpn_sm_img = ofs.ofpSnowMDistribution + "/"
	//			+ CONST_DIST_SNOWMELT_FILE_HEAD + tToP + ".bmp";
	//		th_makeIMG_sm = new thread(makeIMG_flow);
	//	}

	//	if (basc == 1) {
	//		fpn_sm_asc = ofs.ofpSnowMDistribution + "/"
	//			+ CONST_DIST_SNOWMELT_FILE_HEAD + tToP + ".asc";
	//		th_makeASC_sm = new thread(makeASC_flow);
	//		if (prj.fpnProjection != "") {
	//			string ofpn = replaceText(fpn_sm_asc, ".asc", ".prj");
	//			fs::copy(lower(prj.fpnProjection), ofpn);
	//		}
	//	}
	//}
    joinOutputThreads(); // 이과정 거쳐야 래스터 파일 다 쓰고 나서 프로그램 종료한다.
    return 1;
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
                    rfPDTAry[cx][ry] = cvs[i].rf_PDT_m * 1000;
                }
                if (brfacc == 1) {
                    rfaccAry[cx][ry] = cvs[i].rfAccRead_fromStart_mm;
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
                if (bPET == 1) {
                    petPDTAry[cx][ry] = cvs[i].pet_PDT_m * 1000;
                }
                if (bAET == 1) {
                    aetPDTAry[cx][ry] = cvs[i].aet_PDT_m * 1000;
                }

                cvs[i].rf_PDT_m = 0.0; // 여기서 바로 초기화 한다.
                cvs[i].pet_PDT_m = 0.0;
                cvs[i].aet_PDT_m = 0.0;
            }
            else {
                if (bssr == 1) {
                    ssrAry[cx][ry] = -9999;
                }
                if (brf == 1) {
                    rfPDTAry[cx][ry] = -9999;
                }
                if (brfacc == 1) {
                    rfaccAry[cx][ry] = -9999;
                }
                if (bQ == 1) {
                    QAry[cx][ry] = -9999;
                }
                if (bPET == 1) {
                    petPDTAry[cx][ry] = -9999;
                }
                if (bAET == 1) {
                    aetPDTAry[cx][ry] = -9999;
                }
            }
        }
    }
    return 1;
}

//#ifdef _WIN32// MP 추가
void makeIMG_ssr()
{
    makeBMPFileUsingArrayGTzero_InParallel( fpn_ssr_img, ssrAry,
        di.nCols, di.nRows, rendererType::Risk, 1.0, di.nodata_value);
}
void makeIMG_rf()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpn_rfPDT_img, rfPDTAry,
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

void makeIMG_pet()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpn_petPDT_img, petPDTAry,
        di.nCols, di.nRows, rendererType::Depth, 10.0, di.nodata_value);
}

void makeIMG_aet()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpn_aetPDT_img, aetPDTAry,
        di.nCols, di.nRows, rendererType::Depth, 10.0, di.nodata_value);
}
//#endif// MP 추가


void makeASC_ssr()
{
    makeASCTextFile( fpn_ssr_asc, di.headerStringAll,
       ssrAry, di.nCols, di.nRows, 2, di.nodata_value);
}

void makeASC_rf()
{
    makeASCTextFile(fpn_rfPDT_asc, di.headerStringAll,
        rfPDTAry, di.nCols, di.nRows, 2, di.nodata_value);
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

void makeASC_pet()
{
    makeASCTextFile(fpn_petPDT_asc, di.headerStringAll,
        petPDTAry, di.nCols, di.nRows, 2, di.nodata_value);
}

void makeASC_aet()
{
    makeASCTextFile(fpn_aetPDT_asc, di.headerStringAll,
        aetPDTAry, di.nCols, di.nRows, 2, di.nodata_value);
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
    if (th_makeASC_pet != NULL && th_makeASC_pet->joinable() == true) {
        th_makeASC_pet->join();
    }
    if (th_makeASC_aet != NULL && th_makeASC_aet->joinable() == true) {
        th_makeASC_aet->join();
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
    if (th_makeIMG_pet != NULL && th_makeIMG_pet->joinable() == true) {
        th_makeIMG_pet->join();
    }
    if (th_makeIMG_aet != NULL && th_makeIMG_aet->joinable() == true) {
        th_makeIMG_aet->join();
    }
}