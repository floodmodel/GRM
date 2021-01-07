#include <io.h>
#include <string>
#include <thread>
#include <algorithm>

#include "grm.h"
#include "gentle.h"

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

thread* th_makeIMG_ssr;
thread* th_makeIMG_rf;
thread* th_makeIMG_rfacc;
thread* th_makeIMG_flow;

double** ssrAry;
double** rfAry;
double** rfaccAry;
double** QAry;
//double** ssrAryL;
//double** rfAryL;
//double** rfaccAryL;
//double** QAryL;
int bssr =0;
int brf = 0;
int brfacc = 0;
int bQ = 0;
int bimg = 0;
int basc = 0;
string fpnssr_img = "";
string fpnrf_img = "";
string fpnrfacc_img = "";
string fpnQ_img = "";
string fpnssr_asc = "";
string fpnrf_asc = "";
string fpnrfacc_asc = "";
string fpnQ_asc = "";

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

void makeRasterOutput(int nowTmin)
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
            cout << "The folder for soil saturation ratio distribution file is not exist. " << endl;
            return;
        }
        if (bimg == 1) {
            fpnssr_img = ofs.ofpSSRDistribution + "\\"
                + CONST_DIST_SSR_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_ssr = new thread(makeIMG_ssr);
        }
        if (basc == 1) {
            fpnssr_asc = ofs.ofpSSRDistribution + "\\"
                + CONST_DIST_SSR_FILE_HEAD + tToP + ".asc";
            th_makeASC_ssr = new thread(makeASC_ssr);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpnssr_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }            
        }
    }

    if (brf == 1) {
        if (fs::exists(ofs.ofpRFDistribution) == false) {
            cout << "The folder for rainfall distribution file is not exist. " << endl;
            return;
        }
        if (bimg == 1) {
            fpnrf_img = ofs.ofpRFDistribution + "\\"
                + CONST_DIST_RF_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_rf = new thread(makeIMG_rf);
        }
        if (basc == 1) {
            fpnrf_asc = ofs.ofpRFDistribution + "\\"
                + CONST_DIST_RF_FILE_HEAD + tToP + ".asc";
            th_makeASC_rf = new thread(makeASC_rf);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpnrf_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }
        }
    }

    if (brfacc == 1) {
        if (fs::exists(ofs.ofpRFAccDistribution) == false) {
            cout << "The folder for cumulative rainfall distribution file is not exist. " << endl;
            return;
        }
        if (bimg == 1) {
            fpnrfacc_img = ofs.ofpRFAccDistribution + "\\"
                + CONST_DIST_RFACC_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_rfacc = new thread(makeIMG_rfacc);
        }
        if (basc == 1) {
            fpnrfacc_asc = ofs.ofpRFAccDistribution + "\\"
                + CONST_DIST_RFACC_FILE_HEAD + tToP + ".asc";
            th_makeASC_rfacc = new thread(makeASC_rfacc);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpnrfacc_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }
        }
    }
    if (bQ == 1) {
        if (fs::exists(ofs.ofpFlowDistribution) == false) {
            cout << "The folder for flow distribution file is not exist. " << endl;
            return;
        }
        if (bimg == 1) {
            fpnQ_img = ofs.ofpFlowDistribution + "\\"
                + CONST_DIST_FLOW_FILE_HEAD + tToP + ".bmp";
            th_makeIMG_flow = new thread(makeIMG_flow);
        }
        if (basc == 1) {
            fpnQ_asc = ofs.ofpFlowDistribution + "\\"
                + CONST_DIST_FLOW_FILE_HEAD + tToP + ".asc";
            th_makeASC_flow = new thread(makeASC_flow);
            if (prj.fpnProjection != "") {
                string ofpn = replaceText(fpnQ_asc, ".asc", ".prj");
                fs::copy(prj.fpnProjection, ofpn);
            }
        }
    }
    //joinOutputThreads();
}

//int setRasterOutputArray()
//{
//
//#pragma omp parallel for 
//    for (int ry = 0; ry < di.nRows; ++ry) {
//        for (int cx = 0; cx < di.nCols; ++cx) {
//            int i = cvais[cx][ry];
//            if (i > 0 && cvs[i].toBeSimulated == 1) {
//                if (bssr == 1) {
//                    ssrAryL[cx][ry] = cvs[i].ssr;
//                }
//                if (brf == 1) {
//                    rfAryL[cx][ry] = cvs[i].rf_dtPrint_m * 1000;
//                }
//                if (brfacc == 1) {
//                    rfaccAryL[cx][ry] = cvs[i].rfAcc_fromStart_m * 1000;
//                }
//                if (bQ == 1) {
//                    double v;
//                    if (cvs[i].flowType == cellFlowType::OverlandFlow) {
//                        v = cvs[i].QOF_m3Ps;
//                    }
//                    else {
//                        v = cvs[i].stream.QCH_m3Ps;
//                    }
//                    QAryL[cx][ry] = v;
//                }
//                cvs[i].rf_dtPrint_m = 0; // 여기서 바로 초기화 한다.
//            }
//            else {
//                if (bssr == 1) {
//                    ssrAryL[cx][ry] = -9999;
//                }
//                if (brf == 1) {
//                    rfAryL[cx][ry] = -9999;
//                }
//                if (brfacc == 1) {
//                    rfaccAryL[cx][ry] = -9999;
//                }
//                if (bQ == 1) {
//                    QAryL[cx][ry] = -9999;
//                }
//            }
//        }
//    }
//    if (bssr == 1) {
//        copy(&ssrAryL[0][0], &ssrAryL[0][0] + di.nRows * di.nCols, &ssrAry[0][0]);
//    }
//    if (brf == 1) {
//        copy(&rfAryL[0][0], &rfAryL[0][0] + di.nRows * di.nCols, &rfAry[0][0]);
//    }
//    if (brfacc == 1) {
//        copy(&rfaccAryL[0][0], &rfaccAryL[0][0] + di.nRows * di.nCols, &rfaccAry[0][0]);
//    }
//    if (bQ == 1) {
//        copy(&QAryL[0][0], &QAryL[0][0] + di.nRows * di.nCols, &QAry[0][0]);
//    }
//    return 1;
//}

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
                    rfaccAry[cx][ry] = cvs[i].rfAcc_fromStart_m * 1000;
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
    makeBMPFileUsingArrayGTzero_InParallel( fpnssr_img, ssrAry,
        di.nCols, di.nRows, rendererType::Risk, 1, di.nodata_value);
}

void makeIMG_rf()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpnrf_img, rfAry,
        di.nCols, di.nRows, rendererType::Depth, 100, di.nodata_value);
}

void makeIMG_rfacc()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpnrfacc_img, rfaccAry,
        di.nCols, di.nRows, rendererType::Depth, 500, di.nodata_value);
}

void makeIMG_flow()
{
    makeBMPFileUsingArrayGTzero_InParallel(fpnQ_img, QAry,
        di.nCols, di.nRows, rendererType::Depth, 2000, di.nodata_value);
}

void makeASC_ssr()
{
    makeASCTextFile( fpnssr_asc, di.headerStringAll,
       ssrAry, di.nCols, di.nRows, 2, di.nodata_value);
}

void makeASC_rf()
{
    makeASCTextFile(fpnrf_asc, di.headerStringAll,
        rfAry, di.nCols, di.nRows, 2, di.nodata_value);
}
void makeASC_rfacc()
{
    makeASCTextFile(fpnrfacc_asc, di.headerStringAll,
        rfaccAry, di.nCols, di.nRows, 2, di.nodata_value);
}

void makeASC_flow()
{
    makeASCTextFile(fpnQ_asc, di.headerStringAll,
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