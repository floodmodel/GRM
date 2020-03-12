
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;


extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern cvAtt** cvans;
extern cvAtt* cvs;

extern map<int, int*> cvansTofa; //fa별 cvan 목록

int setupModelAfterOpenProjectFile()
{
	if (setDomainAndCVBasicinfo() == -1) { return -1; }
	//if (cProject.Current.watchPoint.UpdatesWatchPointCVIDs(cProject.Current) == false) { return -1; }
	//if (channel.CrossSections.Count > 0)
	//{
	//	foreach(int wsid in channel.CrossSections.Keys)
	//	{
	//		if (WSNetwork.MostDownstreamWSIDs.Contains(wsid) == false)
	//		{
	//			cGRM.writelogAndConsole(string.Format("{0} is not most downstream watershed ID.", wsid), cGRM.bwriteLog, true);
	//			return -1;
	//		}
	//	}
	//}
	//if (mProject.generalSimulEnv.mbSimulateFlowControl == true)
	//{
	//	cProject.Current.fcGrid.UpdateFCGridInfoAndData(cProject.Current);
	//}
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
	readDomainFileAndSetupCV();
	readSlopeFdirFacStreamCwCfSsrFileAndSetCV();


	if (prj.lcDataType == fileOrConstant::File)	{
		if (readLandCoverFileAndSetCVbyVAT() == -1) {
			return -1;
		}
	}
	else if (prj.lcDataType == fileOrConstant::Constant) {
		if (setCVbyLCConstant() == -1) {
			return -1;
		}
	}

	if (prj.stDataType == fileOrConstant::File) {
		if (readSoilTextureFileAndSetCVbyVAT() == -1) {
			return -1;
		}
	}
	else if (prj.stDataType == fileOrConstant::Constant) {
		if (setCVbySTConstant() == -1) {
			return -1;
		}
	}

	if (prj.sdDataType == fileOrConstant::File) {
		if (readSoilDepthFileAndSetCVbyVAT() == -1) {
			return -1;
		}
	}
	else if (prj.sdDataType == fileOrConstant::Constant) {
		if (setCVbySDConstant() == -1) {
			return -1;
		}
	}

	setFlowNetwork();
	InitControlVolumeAttribute();
	return 1;
}

int InitControlVolumeAttribute()
{
	di.facMostUpChannelCell = di.cellCountNotNull;//우선 최대값으로 초기화
	map<int, vector<int>> cvans_fav;
	di.facMax = -1;
	di.facMin = INT_MAX;
	for (int i = 0; i < di.cellCountNotNull; i++) {
		cvs[i].fcType = flowControlType::None;
		cvs[i].toBeSimulated = 1;
		cvs[i].downStreamWPCVIDs.clear();
		double dxw;
		if (cvs[i].neighborCVIDsFlowIntoMe.size() > 0) {
			dxw = cvs[i].dxWSum / (double)cvs[i].neighborCVIDsFlowIntoMe.size();
		}
		else {
			dxw = cvs[i].dxDownHalf_m;
		}
		//cvs[i].cvdx_m = cvs[i].dxDownHalf_m + dxw; 이것 적용하지 않는 것으로 수정. 상류 유입량이 w 끝으로 들어오는 것으로 계산..2015.03.12
		cvs[i].cvdx_m = cvs[i].dxDownHalf_m * 2.0;
		// FA별 cvid 저장
		cvans_fav[cvs[i].fac].push_back(i);
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
	}

	// fac cvan 정보를 전역변수 배열로 저장
	cvansTofa.clear(); 두번째 요소를 백터로 하면 어떨까?
	map<int, vector<int>>::iterator iter;
	for (iter = cvans_fav.begin(); iter != cvans_fav.end(); ++iter) {
		cvansTofa[iter->first] = new int[iter->second.size()];
		copy(iter->second.begin(), iter->second.end(), cvansTofa[iter->first]);
	}
	return 1;
}