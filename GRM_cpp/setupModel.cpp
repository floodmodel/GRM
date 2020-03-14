#include <string>

#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;

extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern int** cvais;
extern cvAtt* cvs;

extern map<int, vector<int>> cvaisToFA; //fa별 cv array idex 목록
extern wpinfo wpis;

int setupModelAfterOpenProjectFile()
{
	if (setDomainAndCVBasicinfo() == -1) { return -1; }
	if (initWPinfos() == -1) { return - 1; }
	if (prj.simFlowControl == 1) {
		if (initFCCellinfoAndData() == -1) { return -1; }
	}


	//cProject.Current.UpdateCVbyUserSettings();
	//cProject.Current.UpdateDownstreamWPforAllCVs();
	//cGRM.Start();
	//sThisSimulation.mGRMSetupIsNormal = true;
	//if (mProject.mSimulationType != cGRM.SimulationType.RealTime)
	//{
	//	sThisSimulation.mRFDataCountInThisEvent = mProject.rainfall.mlstRainfallData.Count;
	//}
	return 1;
}

int setDomainAndCVBasicinfo()
{
	if (readDomainFileAndSetupCV() == -1) { return -1; }
	if (readSlopeFdirFacStreamCwCfSsrFileAndSetCV() == -1) { return -1; }
	if (prj.lcDataType == fileOrConstant::File) {
		if (readLandCoverFileAndSetCVbyVAT() == -1) { return -1; }
	}
	else if (prj.lcDataType == fileOrConstant::Constant) {
		if (setCVbyLCConstant() == -1) { return -1; }
	}
	if (prj.stDataType == fileOrConstant::File) {
		if (readSoilTextureFileAndSetCVbyVAT() == -1) { return -1; }
	}
	else if (prj.stDataType == fileOrConstant::Constant) {
		if (setCVbySTConstant() == -1) { return -1; }
	}
	if (prj.sdDataType == fileOrConstant::File) {
		if (readSoilDepthFileAndSetCVbyVAT() == -1) { return -1; }
	}
	else if (prj.sdDataType == fileOrConstant::Constant) {
		if (setCVbySDConstant() == -1) { return -1; }
	}
	if (setFlowNetwork() == -1) { return -1; }
	if (setupWithFaAndNetwork() == -1) { return -1; }
	return 1;
}

int setupWithFaAndNetwork()
{
	di.facMostUpChannelCell = di.cellCountNotNull;//우선 최대값으로 초기화
	di.facMax = -1;
	di.facMin = INT_MAX;
	cvaisToFA.clear();
	for (int i = 0; i < di.cellCountNotNull; i++) {
		//cvs[i].fcType = flowControlType::None;
		double dxw;
		if (cvs[i].neighborCVIDsFlowIntoMe.size() > 0) {
			dxw = cvs[i].dxWSum / (double)cvs[i].neighborCVIDsFlowIntoMe.size();
		}
		else {
			dxw = cvs[i].dxDownHalf_m;
		}
		//cvs[i].cvdx_m = cvs[i].dxDownHalf_m + dxw; 이것 적용하지 않는 것으로 수정. 상류 유입량이 w 끝으로 들어오는 것으로 계산..2015.03.12
		cvs[i].cvdx_m = cvs[i].dxDownHalf_m * 2.0;
		if (cvs[i].fac > di.facMax) {
			di.facMax = cvs[i].fac;
			di.cvanMaxFac = i;
		}
		if (cvs[i].fac < di.facMin) {
			di.facMin = cvs[i].fac;
		}
		// 하도 매개변수 받고
		if (cvs[i].flowType == cellFlowType::ChannelFlow &&
			cvs[i].fac < di.facMostUpChannelCell) {
			di.facMostUpChannelCell = cvs[i].fac;
		}
		// FA별 cvid 저장
		cvaisToFA[cvs[i].fac].push_back(i);
	}

	// cross section 정보 wsid 오류 확인
	if (prj.css.size() > 0) {
		vector<int> ks;
		map<int, channelSettingInfo>::iterator iter;
		map<int, channelSettingInfo>::iterator iter_end;
		iter_end = prj.css.end();
		for (iter = prj.css.begin(); iter != iter_end; ++iter) {
			ks.push_back(iter->first);
		}
		for (int i : ks) {
			if (getVectorIndex(di.wsn.mdWSIDs, i) == -1) {
				//저장된 css 키가 최하류 wsid 리스트에 없다면,
				string outstr = "[" + to_string(i) + "] is not most downstream watershed ID.\n";
				writeLog(fpnLog, outstr, 1, 1);
				return -1;
			}
		}
	}
	return 1;
}

int initWPinfos()
{
	int isnormal = -1;
	wpis.rfReadIntensitySumUpWS_mPs.clear();
	wpis.rfUpWSAveForDt_mm.clear();
	wpis.rfUpWSAveForDtPrintout_mm.clear();
	wpis.rfUpWSAveTotal_mm.clear();
	wpis.rfWPGridForDtPrintout_mm.clear();
	wpis.rfWPGridTotal_mm.clear();
	wpis.mTotalFlow_cms.clear();
	wpis.mTotalDepth_m.clear();
	wpis.maxFlow_cms.clear();
	wpis.maxDepth_m.clear();
	wpis.maxFlowTime.clear();
	wpis.maxDepthTime.clear();
	wpis.qFromFCData_cms.clear();
	wpis.qprint_cms.clear();
	wpis.FpnWpOut.clear();
	wpis.wpCVIDs.clear();

	for (int i = 0; i < prj.wps.size(); ++i) {
		int cx = prj.wps[i].wpColX;
		int ry = prj.wps[i].wpRowY;
		int cvid = cvais[cx][ry] + 1;
		wpis.wpCVIDs.push_back(cvid);
	}
	isnormal = 1;
	return isnormal;
}