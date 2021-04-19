#include <io.h>
#include <string>


#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern domaininfo di;

projectfilePathInfo getProjectFileInfo(string fpn_prj)
{
	projectfilePathInfo  pfi;
	fs::path fpn_prj_in = fs::path(fpn_prj.c_str());
	pfi.fpn_prj = fpn_prj_in.string();
	pfi.fp_prj = fpn_prj_in.parent_path().string();
	fs::path fn = fpn_prj_in.filename();
	pfi.fn_prj = fn.string();
	pfi.fn_withoutExt_prj = fn.replace_extension().string();
	pfi.prjfileSavedTime = fs::last_write_time(fpn_prj_in);
	fpnLog = fs::path(fpn_prj.c_str()).replace_extension(".log");
	return pfi;
}

int openProjectFile(int forceRealTime)
{
	/*ifstream prjfile(ppi.fpn_prj);
	if (prjfile.is_open() == false) { return -1; }*/
	if (_access(ppi.fpn_prj.c_str(), 0) != 0) {
		 return -1; 
	}
	//prj.swps.clear();
	//prj.css.clear();
	//prj.wps.clear();
	//prj.sts.clear();
	//prj.sds.clear();
	//prj.lcs.clear();
	projectFileFieldName fn;
	swsParameters* aswp;
	aswp = new swsParameters;
	channelSettingInfo* acs;
	acs = new channelSettingInfo;
	flowControlinfo* afc;
	afc = new flowControlinfo;
	PETnSMinfo* apetsm;
	apetsm = new PETnSMinfo;
	wpLocationRC* awp;
	awp = new wpLocationRC;
	soilTextureInfo* ast;
	ast = new soilTextureInfo;
	soilDepthInfo* asd;
	asd = new soilDepthInfo;
	landCoverInfo* alc;
	alc = new landCoverInfo;

	projectFileTable pt;
	vector<string> prjFile;
	int sbProjectSettings = 0; //0::비활성, 1: 활성
	int sbSubWatershedSettings = 0; //0:비활성, 1: 활성
	int sbChannelSettings = 0; //0:비활성, 1: 활성
	int sbFlowControlGrid = 0; //0:비활성, 1: 활성	
	int sbPETnSowMelt = 0; //0:비활성, 1: 활성	
	int sbWatchPoints = 0; //0:비활성, 1: 활성
	int sbGreenAmptParameter = 0; //0:비활성, 1: 활성
	int sbSoilDepth = 0; //0:비활성, 1: 활성
	int sbLandCover = 0; //0:비활성, 1: 활성

	int sbRTenv = 0; //0:비활성, 1: 활성
	prjFile = readTextFileToStringVector(ppi.fpn_prj);
	int LineCount = prjFile.size();

	for (int i = 0; i < LineCount; ++i) {
		string aline = prjFile[i];
		pt.sProjectSettings = getTableStateByXmlLineByLine(aline, pt.nProjectSettings);
		pt.sSubWatershedSettings = getTableStateByXmlLineByLine(aline, pt.nSubWatershedSettings);
		pt.sChannelSettings = getTableStateByXmlLineByLine(aline, pt.nChannelSettings);
		pt.sFlowControlGrid = getTableStateByXmlLineByLine(aline, pt.nFlowControlGrid);
		pt.sWatchPoints = getTableStateByXmlLineByLine(aline, pt.nWatchPoints);
		pt.sPETnSowMeltSettings = getTableStateByXmlLineByLine(aline, pt.nPETnSnowMeltSettings);
		pt.sGreenAmptParameter = getTableStateByXmlLineByLine(aline, pt.nGreenAmptParameter);
		pt.sSoilDepth = getTableStateByXmlLineByLine(aline, pt.nSoilDepth);
		pt.sLandCover = getTableStateByXmlLineByLine(aline, pt.nLandCover);

		if (pt.sProjectSettings == 1 || pt.sProjectSettings == 2) { // getTableStateByXmlLine 함수를 사용할 경우에는 2 값을 가질 수 있다.
			sbProjectSettings = 1;
		}
		if (pt.sSubWatershedSettings == 1 || pt.sSubWatershedSettings == 2) {
			sbSubWatershedSettings = 1;
		}
		if (pt.sChannelSettings == 1 || pt.sChannelSettings == 2) {
			sbChannelSettings = 1;
		}
		if (pt.sFlowControlGrid == 1 || pt.sFlowControlGrid == 2) {
			sbFlowControlGrid = 1;
		}
		if (pt.sPETnSowMeltSettings == 1|| pt.sPETnSowMeltSettings==2) {
			sbPETnSowMelt = 1;
		}
		if (pt.sWatchPoints == 1 || pt.sWatchPoints == 2) {
			sbWatchPoints = 1;
		}
		if (pt.sGreenAmptParameter == 1 || pt.sGreenAmptParameter == 2) {
			sbGreenAmptParameter = 1;
		}
		if (pt.sSoilDepth == 1 || pt.sSoilDepth == 2) {
			sbSoilDepth = 1;
		}
		if (pt.sLandCover == 1 || pt.sLandCover == 2) {
			sbLandCover = 1;
		}
		if (trim(aline) == "<" + pt.nProjectSettings + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nSubWatershedSettings + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nChannelSettings + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nFlowControlGrid + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nPETnSnowMeltSettings + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nWatchPoints + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nGreenAmptParameter + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nSoilDepth + ">") {
			continue;
		}
		if (trim(aline) == "<" + pt.nLandCover + ">") {
			continue;
		}

		if (sbProjectSettings == 1 && pt.sProjectSettings == 0) {
			sbProjectSettings = 0;
			continue;
		}
		if (sbProjectSettings == 1 && pt.sProjectSettings != 0) {
			sbProjectSettings = 1;
			if (readXmlRowProjectSettings(aline) == -1) {
				return -1;
			}
			continue;
		}
		if (sbSubWatershedSettings == 1 && pt.sSubWatershedSettings == 0) {
			sbSubWatershedSettings = 0;
			if (aswp->wsid > 0 && isNormalSwsParameter(aswp) == 1) {
				prj.swps[aswp->wsid] = *aswp;
				aswp = new swsParameters;
			}
			continue;
		}
		if (sbSubWatershedSettings == 1 && pt.sSubWatershedSettings != 0) {
			sbSubWatershedSettings = 1;
			if (readXmlRowSubWatershedSettings(aline, aswp) == -1) {
				return -1;
			}
			continue;
		}

		if (sbChannelSettings == 1 && pt.sChannelSettings == 0) {
			sbChannelSettings = 0;
			if (acs->mdWsid > 0 && isNormalChannelSettingInfo(acs) == 1) {
				prj.css[acs->mdWsid] = *acs;
				acs = new channelSettingInfo;
			}
			continue;
		}
		if (sbChannelSettings == 1 && pt.sChannelSettings != 0) {
			sbChannelSettings = 1;
			if (readXmlRowChannelSettings(aline, acs) == -1) {
				return -1;
			}
			continue;
		}
		if (sbFlowControlGrid == 1 && pt.sFlowControlGrid == 0) {
			sbFlowControlGrid = 0;
			if (afc->fcName != "" && isNormalFlowControlinfo(afc) == 1) {
				int n = prj.fcs.size();
				prj.fcs[n] = *afc;// 우선 idx를 키로 사용. updateFCCellinfoAndData()에서 cvid를 키로 업데이트				afc = new flowControlinfo;
			}
			continue;
		}
		if (sbFlowControlGrid == 1 && pt.sFlowControlGrid != 0) {
			sbFlowControlGrid = 1;
			if (readXmlRowFlowControlGrid(aline, afc) == -1) {
				return -1;
			}
			continue;
		}
		if (sbPETnSowMelt == 1 && pt.sPETnSowMeltSettings == 0) {
			sbPETnSowMelt = 0;
			if (apetsm->wsid > 0 && isNormalPETnSnowMelt(apetsm) == 1) {
				prj.petsms[apetsm->wsid] = *apetsm;
				apetsm = new PETnSMinfo;
			}
			continue;
		}
		if (sbPETnSowMelt == 1 && pt.sPETnSowMeltSettings != 0) {
			sbPETnSowMelt = 1;
			if (readXmlPETnSnowMelt(aline, apetsm) == -1) {
				return -1;
			}
			continue;
		}
		if (sbWatchPoints == 1 && pt.sWatchPoints == 0) {
			sbWatchPoints = 0;
			if (awp->wpName != "" && isNormalWatchPointInfo(awp) == 1) {
				prj.wps.push_back(*awp);
				awp = new wpLocationRC;
			}
			continue;
		}
		if (sbWatchPoints == 1 && pt.sWatchPoints != 0) {
			sbWatchPoints = 1;
			if (readXmlRowWatchPoint(aline, awp) == -1) {
				return -1;
			}
			continue;
		}

		if (prj.stDataType == fileOrConstant::File) {
			if (sbGreenAmptParameter == 1 && pt.sGreenAmptParameter == 0) {
				sbGreenAmptParameter = 0;
				if (ast->stGridValue > 0 && isNormalSoilTextureInfo(ast) == 1) {
					prj.sts.push_back(*ast);
					ast = new soilTextureInfo;
				}
				continue;
			}
			if (sbGreenAmptParameter == 1 && pt.sGreenAmptParameter != 0) {
				sbGreenAmptParameter = 1;
				if (readXmlRowSoilTextureInfo(aline, ast) == -1) {
					return -1;
				}
				continue;
			}
		}

		if (prj.sdDataType == fileOrConstant::File) {
			if (sbSoilDepth == 1 && pt.sSoilDepth == 0) {
				sbSoilDepth = 0;
				if (asd->sdGridValue > 0 && isNormalSoilDepthInfo(asd) == 1) {
					prj.sds.push_back(*asd);
					asd = new soilDepthInfo;
				}
				continue;
			}
			if (sbSoilDepth == 1 && pt.sSoilDepth != 0) {
				sbSoilDepth = 1;
				if (readXmlRowSoilDepth(aline, asd) == -1) {
					return -1;
				}
				continue;
			}
		}

		if (prj.lcDataType == fileOrConstant::File) {
			if (sbLandCover == 1 && pt.sLandCover == 0) {
				sbLandCover = 0;
				if (alc->lcGridValue > 0 && isNormalLandCoverInfo(alc) == 1) {
					prj.lcs.push_back(*alc);
					alc = new landCoverInfo;
				}
				continue;
			}
			if (sbLandCover == 1 && pt.sLandCover != 0) {
				sbLandCover = 1;
				if (readXmlRowLandCover(aline, alc) == -1) {
					return -1;
				}
				continue;
			}
		}
	}

	// 여기서 부터 검증
	prj.applyFC = -1;
	if (prj.simFlowControl == 1 && prj.fcs.size() > 0) {
		prj.applyFC = 1;
		map<int, flowControlinfo>::iterator iter;
		for (iter = prj.fcs.begin(); iter != prj.fcs.end(); ++iter) {
			int idx = iter->first;
			flowControlinfo afci;
			afci = prj.fcs[idx];
			if (afci.roType != reservoirOperationType::None
				&& (afci.maxStorage_m3 <= 0
					|| afci.maxStorageR <= 0)) {
				writeLog(fpnLog, " Max. storage and max. storage ratio must be greater than zero when reservoir operation type is applied.\n", 1, 1);
				return -1;
			}
			if (afci.fcType != flowControlType::Inlet) {
				int bsimStorage = 1;
				if (afci.iniStorage_m3 < 0) {
					writeLog(fpnLog, "Ini. storage of reservoir ["
						+ afci.fcName + "] was set to '0'.\n", 1, 1);
					prj.fcs[idx].iniStorage_m3 = 0.0;
					bsimStorage = -1;
				}
				if (afci.maxStorage_m3 <= 0) {
					writeLog(fpnLog, "Max storage of reservoir ["
						+ afci.fcName + "] was set to '0'.\n", 1, 1);
					prj.fcs[idx].maxStorage_m3 = 0.0;
					bsimStorage = -1;
				}
				if (afci.maxStorageR <= 0) {
					writeLog(fpnLog, "Max storage ratio of reservoir ["
						+ afci.fcName + "] was set to '0.0'.\n", 1, 1);
					prj.fcs[idx].maxStorageR = 0.0;
					bsimStorage = -1;
				}
				if (bsimStorage == -1) {
					writeLog(fpnLog, "The storage of reservoir ["
						+ afci.fcName + "] will not be simulated.\n", 1, 1);
				}
				if (afci.iniStorage_m3 > afci.maxStorage_m3 * afci.maxStorageR) {
					double effStorage = afci.maxStorage_m3 * afci.maxStorageR;
					writeLog(fpnLog, "[" + afci.fcName + "] Initial storage(" + dtos(afci.iniStorage_m3, 0)
						+ "m^3) is greater than effective storage(" + dtos(effStorage, 0) + "m^3). \n", 1, 1);
					writeLog(fpnLog, "[" + afci.fcName + "] Initial storage was set to effective storage(" 
						+ dtos(effStorage, 0) + "m^3).\n", 1, 1);
					// 초기저수량이 유효저수량보다 크면, 초기저수량을 유효저수량으로 설정한다.
					prj.fcs[idx].iniStorage_m3 = afci.maxStorage_m3 * afci.maxStorageR;

				}

			}
		}
	}

	// 이건 continuous 용 =====================
	if (prj.simEvTr == 1)	{
		if (prj.petsms.size() < 1) {
			writeLog(fpnLog, "PET data is invalid.\n", 1, 1);
			return -1;
		}
		map<int, PETnSMinfo>::iterator iter;
		for (iter = prj.petsms.begin(); iter != prj.petsms.end(); ++iter) {
			int idx = iter->first;
			PETnSMinfo apetsm;
			apetsm = prj.petsms[idx];
			if (apetsm.petMethod == PETmethod::UserData) {
				if (apetsm.fpnPET == "" || _access(apetsm.fpnPET.c_str(), 0) == 0) {
					writeLog(fpnLog, "PET data file is invalid.\n", 1, 1);
					return -1;
				}
			}
			else if (apetsm.petMethod != PETmethod::None) {
				if (apetsm.PETcoeffPlant <= 0.0 || apetsm.PETcoeffPlant>1) {
					writeLog(fpnLog, "PET coefficient for plant in the area ["
						+ to_string(idx) + "] is invalid.\n", 1, 1);
					writeLog(fpnLog, "PET coefficient for plant has to be (greater than 0) and (equal or less than 1).\n", 1, 1);
					return -1;
				}
				if (apetsm.PETcoeffSoil <= 0.0 || apetsm.PETcoeffSoil > 1) {
					writeLog(fpnLog, "PET coefficient for soil in the area ["
						+ to_string(idx) + "] is invalid.\n", 1, 1);
					writeLog(fpnLog, "PET coefficient for soil has to be (greater than 0) and (equal or less than 1).\n", 1, 1);
					return -1;
				}
			}
		}
	}

	if (prj.simSnowMelt == 1)	{
		if (prj.petsms.size() < 1) {
			writeLog(fpnLog, "Snow melt data is invalid.\n", 1, 1);
			return -1;
		}
		map<int, PETnSMinfo>::iterator iter;
		for (iter = prj.petsms.begin(); iter != prj.petsms.end(); ++iter) {
			int idx = iter->first;
			PETnSMinfo apetsm;
			apetsm = prj.petsms[idx];
			if (apetsm.smMethod == snowMeltMethod::UserData) {
				if (apetsm.fpnSnowMelt == "" || _access(apetsm.fpnSnowMelt.c_str(), 0) == 0) {
					writeLog(fpnLog, "Snow melt data file is invalid.\n", 1, 1);
					return -1;
				}
			}
		}
	}
	//=====================

	if (prj.lcDataType == fileOrConstant::File) {
		if (prj.fpnLC == "") {
			writeLog(fpnLog, "Land cover file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnLCVat == "") {
			writeLog(fpnLog, "Land cover VAT file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.lcs.size() == 0) {
			writeLog(fpnLog, "Land cover attributes from the files are invalid.\n", 1, 1);
			return -1;
		}
	}
	else if (prj.lcDataType == fileOrConstant::Constant) {
		if (prj.cnstRoughnessC == 0.0) {
			writeLog(fpnLog, "Land cover constant roughness is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstImperviousR < 0.0) {
			writeLog(fpnLog, "Land cover constant impervious ratio is invalid.\n", 1, 1);
			return -1;
		}
	}
	if (prj.stDataType == fileOrConstant::File) {
		if (prj.fpnSD == "") {
			writeLog(fpnLog, "Soil texture file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnSDVat == "") {
			writeLog(fpnLog, "Soil texture VAT file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.sts.size() == 0) {
			writeLog(fpnLog, "Soil texture attributes from the files are invalid.\n", 1, 1);
			return -1;
		}
	}
	else if (prj.stDataType == fileOrConstant::Constant) {
		if (prj.cnstSoilEffPorosity < 0) {
			writeLog(fpnLog, "Soil texture constant effective porosity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilHydraulicK < 0) {
			writeLog(fpnLog, "Soil texture constant hydraulic conductivity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilPorosity < 0) {
			writeLog(fpnLog, "Soil texture constant porosity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilWFSH < 0) {
			writeLog(fpnLog, "Soil texture constant wetting front suction head is invalid.\n", 1, 1);
			return -1;
		}
	}
	if (prj.sdDataType == fileOrConstant::File) {
		if (prj.fpnSD == "") {
			writeLog(fpnLog, "Soil depth file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnSDVat == "") {
			writeLog(fpnLog, "Soil depth VAT file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.sds.size() == 0) {
			writeLog(fpnLog, "Soil depth attributes from the files are invalid.\n", 1, 1);
			return -1;
		}
	}
	else if (prj.sdDataType == fileOrConstant::Constant) {
		if (prj.cnstSoilDepth < 0) {
			writeLog(fpnLog, "Soil depth constant value is invalid.\n", 1, 1);
			return -1;
		}
	}

	if (prj.dtsec == 0) {
		if (prj.isFixedTimeStep == 1) {
			writeLog(fpnLog, "In fixed dt, the calculation time step ["
				+ to_string(prj.dtsec) + "] is invalid.\n", 1, 1);
			return -1;
		}
		else if (prj.isFixedTimeStep == -1) {
			writeLog(fpnLog, "In adaptive dt, the calculation time step ["
				+ to_string(prj.dtsec) + "] is changed to 1 minute.\n", 1, -1);
		}
	}

	if (prj.mdp == -1) {
		if (prj.cpusi.totalNumOfLP != 0) {
			prj.mdp = prj.cpusi.totalNumOfLP;
		}
		else {
			prj.mdp = 12;  //omp_get_max_threads()를 사용하면 최대 cpu를 적용하므로 grm에서는 과도한 경우가 많다..
			writeLog(fpnLog, "The number of CPUs could not be encountered. Max. degree of parallelism was set to 12.\n"
				, 1, 1);
		}
	}

	//di.dmids.clear();
	//map<int, domaininfo>::iterator iter;
	//for (int n = 0; n < prj.swps.size(); n++) {
	//	di.dmids.push_back(prj.swps[n].wsid);
	//}	

	if (prj.simType == simulationType::Normal) {
		if (setRainfallData() == -1) { return -1; }
	}

	if (forceRealTime == 1) {
		prj.simType = simulationType::RealTime;
	}

	if (prj.makeASCFile == 1 || prj.makeIMGFile == 1) {
		prj.makeASCorIMGfile = 1;
		if (prj.makeRfDistFile == 1 || prj.makeRFaccDistFile == 1) {
			prj.makeRFraster = 1;
		}
		else {
			prj.makeRFraster = -1;
		}
	}
	else {
		prj.makeASCorIMGfile = -1;
	}

	if (aswp != NULL) {
		delete aswp;
	}
	if (acs != NULL) {
		delete acs;
	}
	if (afc != NULL) {
		delete afc;
	}
	if (awp != NULL) {
		delete awp;
	}
	if (ast != NULL) {
		delete ast;
	}
	if (asd != NULL) {
		delete asd;
	}
	if (alc != NULL) {
		delete alc;
	}
	return 1;
}


int readXmlRowProjectSettings(string aline)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find(fldName.GRMSimulationType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GRMSimulationType);
		//prj.simType = simulationType::None;
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(SingleEvent))) { // 과거의 gmp에서 SingleEvent도 Normal로 설정한다.
				prj.simType = simulationType::Normal;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Normal))) {
				prj.simType = simulationType::Normal;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(RealTime))) {
				prj.simType = simulationType::RealTime;
			}
			else {
				writeLog(fpnLog, "Simulation type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Simulation type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.DomainFile_01) != string::npos
		|| aline.find(fldName.DomainFile_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DomainFile_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.DomainFile_02);
		}
		prj.fpnDomain = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnDomain = vString;

			fs::path fpnProjection;
			fpnProjection = fs::path(prj.fpnDomain.c_str()).replace_extension(".prj");
			if (fs::exists(fpnProjection) == true) {
				prj.fpnProjection = fpnProjection.string();
			}
			else {
				prj.fpnProjection = "";
			}
		}
		else {
			writeLog(fpnLog, "DEM file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.SlopeFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SlopeFile);
		prj.fpnSlope = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnSlope = vString;
		}
		else {
			writeLog(fpnLog, "Slope file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.FlowDirectionFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowDirectionFile);
		prj.fpnFD = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnFD = vString;
		}
		else {
			writeLog(fpnLog, "Flow direction file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.FlowAccumFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowAccumFile);
		prj.fpnFA = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnFA = vString;
		}
		else {
			writeLog(fpnLog, "Flow accumulation file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.StreamFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.StreamFile);
		prj.fpnStream = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnStream = vString;
			prj.streamFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLog(fpnLog, "Stream file was not set.\n", 1, -1);
			}
			else {
				writeLog(fpnLog, "Stream file [" + vString + "] is invalid.\n", 1, 1);
			}
			prj.streamFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.ChannelWidthFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthFile);
		prj.fpnChannelWidth = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnChannelWidth = vString;
			prj.cwFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLog(fpnLog, "Channel width file was not set.\n", 1, -1);
			}
			else {
				writeLog(fpnLog, "Channel width file [" + vString + "] is invalid.\n", 1, 1);
			}

			prj.cwFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.InitialSoilSaturationRatioFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.InitialSoilSaturationRatioFile);
		prj.fpniniSSR = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpniniSSR = vString;
			prj.issrFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLog(fpnLog, "Soil saturation ratio file was not set.\n", 1, -1);
			}
			else {
				writeLog(fpnLog, "Soil saturation ratio file [" + vString + "] is invalid.\n", 1, 1);
			}

			prj.issrFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.InitialChannelFlowFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.InitialChannelFlowFile);
		prj.fpniniChFlow = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpniniChFlow = vString;
			prj.icfFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLog(fpnLog, "Initial stream flow file was not set.\n", 1, -1);
			}
			else {
				writeLog(fpnLog, "Initial stream flow file [" + vString + "] is invalid.\n", 1, 1);
			}
			prj.icfFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.LandCoverDataType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LandCoverDataType);
		//prj.lcDataType = fileOrConstant::None;
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(File))) {
				prj.lcDataType = fileOrConstant::File;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Constant))) {
				prj.lcDataType = fileOrConstant::Constant;
			}
			else {
				writeLog(fpnLog, "Land cover data type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Land cover data type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.LandCoverFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LandCoverFile);
		prj.fpnLC = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnLC = vString;
		}
		return 1;
	}

	if (aline.find(fldName.LandCoverVATFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LandCoverVATFile);
		prj.fpnLCVat = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnLCVat = vString;
		}
		return 1;
	}

	if (aline.find(fldName.ConstantRoughnessCoeff) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ConstantRoughnessCoeff);
		if (vString != "") {
			prj.cnstRoughnessC = stod(vString);
		}
		return 1;
	}

	if (aline.find(fldName.ConstantImperviousRatio) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ConstantImperviousRatio);
		if (vString != "") {
			prj.cnstImperviousR = stod(vString);
		}
		return 1;
	}

	if (aline.find(fldName.SoilTextureDataType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilTextureDataType);
		//prj.stDataType = fileOrConstant::None;
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(File))) {
				prj.stDataType = fileOrConstant::File;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Constant))) {
				prj.stDataType = fileOrConstant::Constant;
			}
			else {
				writeLog(fpnLog, "Soil texture data type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Soil texture data type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.SoilTextureFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilTextureFile);
		prj.fpnST = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnST = vString;
		}
		return 1;
	}

	if (aline.find(fldName.SoilTextureVATFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilTextureVATFile);
		prj.fpnSTVat = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnSTVat = vString;
		}
		return 1;
	}

	if (aline.find(fldName.ConstantSoilPorosity) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ConstantSoilPorosity);
		if (vString != "") {
			prj.cnstSoilPorosity = stod(vString);
		}
		return 1;
	}
	if (aline.find(fldName.ConstantSoilEffPorosity) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ConstantSoilEffPorosity);
		if (vString != "") {
			prj.cnstSoilEffPorosity = stod(vString);
		}
		return 1;
	}
	if (aline.find(fldName.ConstantSoilWettingFrontSuctionHead) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ConstantSoilWettingFrontSuctionHead);
		if (vString != "") {
			prj.cnstSoilWFSH = stod(vString);
		}
		return 1;
	}
	if (aline.find(fldName.ConstantSoilHydraulicConductivity) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ConstantSoilHydraulicConductivity);
		if (vString != "") {
			prj.cnstSoilHydraulicK = stod(vString);
		}
		return 1;
	}

	if (aline.find(fldName.SoilDepthDataType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilDepthDataType);
		//prj.sdDataType = fileOrConstant::None;
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(File))) {
				prj.sdDataType = fileOrConstant::File;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Constant))) {
				prj.sdDataType = fileOrConstant::Constant;
			}
			else {
				writeLog(fpnLog, "Soil depth data type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Soil depth data type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SoilDepthFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilDepthFile);
		prj.fpnSD = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnSD = vString;
		}
		return 1;
	}
	if (aline.find(fldName.SoilDepthVATFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilDepthVATFile);
		prj.fpnSDVat = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnSDVat = vString;
		}
		return 1;
	}
	if (aline.find(fldName.ConstantSoilDepth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ConstantSoilDepth);
		if (vString != "") {
			prj.cnstSoilDepth = stod(vString);
		}
		return 1;
	}

	if (aline.find(fldName.RainfallDataType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.RainfallDataType);
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(TextFileMAP))) {
				prj.rfDataType = rainfallDataType::TextFileMAP;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(TextFileASCgrid))) {
				prj.rfDataType = rainfallDataType::TextFileASCgrid;
			}
			else {
				writeLog(fpnLog, "Rainfall data type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Rainfall data data type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.RainfallDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.RainfallDataFile);
		prj.fpnRainfallData = "";
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			prj.fpnRainfallData = vString;
		}
		else if (prj.simType == simulationType::Normal) {
			writeLog(fpnLog, "Rainfall data file [" + vString + "] was not set.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.RainfallInterval_min_01) != string::npos
		|| aline.find(fldName.RainfallInterval_min_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.RainfallInterval_min_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.RainfallInterval_min_02);
		}
		if (vString != "") {
			prj.rfinterval_min = stoi(vString);
		}
		else if (prj.simType == simulationType::Normal) {
			writeLog(fpnLog, "Rainfall data time interval was not set.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.FlowDirectionType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowDirectionType);
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(StartsFromNE))) {
				prj.fdType = flowDirectionType::StartsFromNE;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(StartsFromN))) {
				prj.fdType = flowDirectionType::StartsFromN;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(StartsFromE))) {
				prj.fdType = flowDirectionType::StartsFromE;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(StartsFromE_TauDEM))) {
				prj.fdType = flowDirectionType::StartsFromE_TauDEM;
			}
			else {
				writeLog(fpnLog, "Flow direction type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Flow direction type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.MaxDegreeOfParallelism) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MaxDegreeOfParallelism);
		if (vString != "" && stoi(vString) != 0 && stoi(vString) >= -1) {
			prj.mdp = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Max. degree of parallelism was not set. Maximum value [-1] was assigned.\n", 1, 1);
			prj.mdp = -1;
		}
		return 1;
	}

	if (aline.find(fldName.SimulStartingTime) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulStartingTime);
		if (vString != "") {
			prj.simStartTime = vString;
			if (isNumeric(vString) == true) {
				prj.isDateTimeFormat = -1;
			}
			else {
				prj.isDateTimeFormat = 1;
			}

		}
		else {
			prj.isDateTimeFormat = -1;
			prj.simStartTime = "0";
		}
		return 1;
	}

	if (aline.find(fldName.SimulationDuration_hr_01) != string::npos
		|| aline.find(fldName.SimulationDuration_hr_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulationDuration_hr_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.SimulationDuration_hr_02);
		}
		if (vString != "" && stod(vString) >= 0) {
			prj.simDuration_hr = stod(vString);
		}
		else {
			writeLog(fpnLog, "Simulation duration is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.ComputationalTimeStep_min_01) != string::npos
		|| aline.find(fldName.ComputationalTimeStep_min_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ComputationalTimeStep_min_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.ComputationalTimeStep_min_02);
		}
		if (vString != "" && stod(vString) > 0) {
			prj.dtsec = stoi(vString) * 60;
		}
		return 1;
	}

	if (aline.find(fldName.IsFixedTimeStep) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.IsFixedTimeStep);
		prj.isFixedTimeStep = -1;
		if (lower(vString) == "true") {
			prj.isFixedTimeStep = 1;
		}
		return 1;
	}

	if (aline.find(fldName.OutputTimeStep_min_01) != string::npos
		|| aline.find(fldName.OutputTimeStep_min_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.OutputTimeStep_min_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.OutputTimeStep_min_02);
		}
		if (vString != "") {
			prj.dtPrint_min = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Print out time step is invalid.\n", 1, 1);
			return -1;
		}

		return 1;
	}

	if (aline.find(fldName.SimulateInfiltration) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateInfiltration);
		prj.simInfiltration = -1;
		if (lower(vString) == "true") {
			prj.simInfiltration = 1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateSubsurfaceFlow) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateSubsurfaceFlow);
		prj.simSubsurfaceFlow = -1;
		if (lower(vString) == "true") {
			prj.simSubsurfaceFlow = 1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateBaseFlow) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateBaseFlow);
		prj.simBaseFlow = -1;
		if (lower(vString) == "true") {
			prj.simBaseFlow = 1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateEvTr) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateEvTr);
		prj.simEvTr = -1;
		if (lower(vString) == "true") {
			prj.simEvTr = 1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateSnowMelt) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateSnowMelt);
		prj.simSnowMelt = -1;
		if (lower(vString) == "true") {
			prj.simSnowMelt = 1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateFlowControl) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateFlowControl);
		prj.simFlowControl = -1;
		if (lower(vString) == "true") {
			prj.simFlowControl = 1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeIMGFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeIMGFile);
		prj.makeIMGFile = -1;
		if (lower(vString) == "true") {
			prj.makeIMGFile = 1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeASCFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeASCFile);
		prj.makeASCFile = -1;
		if (lower(vString) == "true") {
			prj.makeASCFile = 1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeSoilSaturationDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeSoilSaturationDistFile);
		prj.makeSoilSaturationDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeSoilSaturationDistFile = 1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeRfDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeRfDistFile);
		prj.makeRfDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeRfDistFile = 1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeRFaccDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeRFaccDistFile);
		prj.makeRFaccDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeRFaccDistFile = 1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeFlowDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeFlowDistFile);
		prj.makeFlowDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeFlowDistFile = 1;
		}
		return 1;
	}

	if (aline.find(fldName.PrintOption) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PrintOption);
		if (vString != "") {
			if (lower(vString) == "all") {
				prj.printOption = GRMPrintType::All;
			}else if (lower(vString) == "dischargefile") {
				prj.printOption = GRMPrintType::DischargeFile;
			}
			else if (lower(vString) == "dischargefileq") {
				prj.printOption = GRMPrintType::DischargeFileQ;
			}
			else if (lower(vString) == "allq") {
				prj.printOption = GRMPrintType::AllQ;
			}
			else {
				writeLog(fpnLog, "Print option is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Print option is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	//if (aline.find(fn.WriteLog) != string::npos) {
	//	vString = getValueStringFromXmlLine(aline, fn.WriteLog);
	//	prj.writeConsole = -1;
	//	if (lower(vString) == "true") {
	//		prj.writeConsole = 1;
	//	}
	//	return 1;
	//}
	return 1;
}

int readXmlRowLandCover(string aline, landCoverInfo* lc)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find(fldName.GridValue_LC) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GridValue_LC);
		if (vString != "" && stoi(vString) >= 0) {
			lc->lcGridValue = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Land cover raster value in gmp file is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.GRMCode_LC) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GRMCode_LC);
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(WATR))) {
				lc->lcCode = landCoverCode::WATR;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(URBN))) {
				lc->lcCode = landCoverCode::URBN;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(BARE))) {
				lc->lcCode = landCoverCode::BARE;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(WTLD))) {
				lc->lcCode = landCoverCode::WTLD;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(GRSS))) {
				lc->lcCode = landCoverCode::GRSS;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(FRST))) {
				lc->lcCode = landCoverCode::FRST;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(AGRL))) {
				lc->lcCode = landCoverCode::AGRL;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(USER))) {
				lc->lcCode = landCoverCode::USER;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(CONSTV))) {
				lc->lcCode = landCoverCode::CONSTV;
			}
			else {
				writeLog(fpnLog, "Land cover code name of the raster value ["
					+ to_string(lc->lcGridValue) + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		else {
			writeLog(fpnLog, "Land cover code name of the raster value ["
				+ to_string(lc->lcGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
	}
	if (aline.find(fldName.RoughnessCoeff) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.RoughnessCoeff);
		if (vString != "") {
			lc->RoughnessCoefficient = stod(vString);
		}
		else {
			writeLog(fpnLog, "Land cover roughness coefficient of the raster value ["
				+ to_string(lc->lcGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ImperviousR) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ImperviousR);
		if (vString != "") {
			lc->ImperviousRatio = stod(vString);
		}
		else {
			writeLog(fpnLog, "Land cover impervious ratio of the raster value ["
				+ to_string(lc->lcGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	return 1;
}

int  readXmlRowSoilDepth(string aline,	soilDepthInfo* sd)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find(fldName.GridValue_SD) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GridValue_SD);
		if (vString != "" && stoi(vString) >= 0) {
			sd->sdGridValue = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Soil depth raster value in gmp file is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.GRMCode_SD) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GRMCode_SD);
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(D))) {
				sd->sdCode = soilDepthCode::D;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(MDMS))) {
				sd->sdCode = soilDepthCode::M;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(M))) {
				sd->sdCode = soilDepthCode::M;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(S))) {
				sd->sdCode = soilDepthCode::S;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(VD))) {
				sd->sdCode = soilDepthCode::VD;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(VS))) {
				sd->sdCode = soilDepthCode::VS;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(USER))) {
				sd->sdCode = soilDepthCode::USER;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(CONSTV))) {
				sd->sdCode = soilDepthCode::CONSTV;
			}
			else {
				writeLog(fpnLog, "Soil depth code name of the raster value ["
					+ to_string(sd->sdGridValue) + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		else {
			writeLog(fpnLog, "Soil depth code name of the raster value ["
				+ to_string(sd->sdGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
	}
	if (aline.find(fldName.SoilDepthValue_01) != string::npos
		|| aline.find(fldName.SoilDepthValue_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilDepthValue_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.SoilDepthValue_02);
		}
		if (vString != "") {
			sd->soilDepth = stod(vString);
		}
		else {
			writeLog(fpnLog, "Soil depth value of the raster value ["
				+ to_string(sd->sdGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	return 1;
}
int readXmlRowSoilTextureInfo(string aline, soilTextureInfo* st)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find(fldName.GridValue_ST) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GridValue_ST);
		if (vString != "" && stoi(vString) >= 0) {
			st->stGridValue = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Soil texture raster value in gmp file is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.GRMCode_ST) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GRMCode_ST);
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(C))) {
				st->stCode = soilTextureCode::C;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(CL))) {
				st->stCode = soilTextureCode::CL;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(L))) {
				st->stCode = soilTextureCode::L;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(LS))) {
				st->stCode = soilTextureCode::LS;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(S))) {
				st->stCode = soilTextureCode::S;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SC))) {
				st->stCode = soilTextureCode::SC;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SCL))) {
				st->stCode = soilTextureCode::SCL;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SiC))) {
				st->stCode = soilTextureCode::SiC;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SiCL))) {
				st->stCode = soilTextureCode::SiCL;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SiL))) {
				st->stCode = soilTextureCode::SiL;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SL))) {
				st->stCode = soilTextureCode::SL;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(USER))) {
				st->stCode = soilTextureCode::USER;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(CONSTV))) {
				st->stCode = soilTextureCode::CONSTV;
			}
			else {
				writeLog(fpnLog, "Soil texture code name of the raster value ["
					+ to_string(st->stGridValue) + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		else {
			writeLog(fpnLog, "Soil texture code name of the raster value ["
				+ to_string(st->stGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
	}
	if (aline.find("<"+fldName.Porosity+">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.Porosity);
		if (vString != "") {
			st->porosity = stod(vString);
		}
		else {
			writeLog(fpnLog, "Soil porosity of the raster value ["
				+ to_string(st->stGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find("<"+fldName.EffectivePorosity+">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.EffectivePorosity);
		if (vString != "") {
			st->effectivePorosity = stod(vString);
		}
		else {
			writeLog(fpnLog, "Soil effective porosity of the raster value ["
				+ to_string(st->stGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.WFSuctionHead) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.WFSuctionHead);
		if (vString != "") {
			st->WFSuctionHead = stod(vString);
		}
		else {
			writeLog(fpnLog, "Soil wetting front suction head of the raster value ["
				+ to_string(st->stGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.HydraulicConductivity) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.HydraulicConductivity);
		if (vString != "") {
			st->hydraulicK = stod(vString);
		}
		else {
			writeLog(fpnLog, "Soil hydraulic conductivity of the raster value ["
				+ to_string(st->stGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	return 1;
}

int readXmlRowWatchPoint(string aline, wpLocationRC* wpl)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find(fldName.Name_WP) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.Name_WP);
		if (vString != "") {
			wpl->wpName = vString;
		}
		return 1;
	}
	if (aline.find(fldName.ColX_WP) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ColX_WP);
		if (vString != "") {
			wpl->wpColX = stoi(vString);
		}
		return 1;
	}
	if (aline.find(fldName.RowY_WP) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.RowY_WP);
		if (vString != "") {
			wpl->wpRowY = stoi(vString);
		}
		return 1;
	}
	return 1;
}

// 이건 continuous 용 =======================================================
int readXmlPETnSnowMelt(string aline, PETnSMinfo* petsmi) {
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find("<" + fldName.ID_PETSM + ">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ID_PETSM);
		if (vString != "" && stoi(vString) > 0) {
			petsmi->wsid = stoi(vString);
		}
		else {
			writeLog(fpnLog, "ID in PET and snow melt data is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.PETMethod) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PETMethod);
		PETmethod etm = PETmethod::notSet; // 이것으로 설정되면, 애러로 처리
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(UserData))) {
				etm = PETmethod::UserData;// constant로 하고 그 값을 0으로 하면 증발산 없음.
			}
			else if (lower(vString) == lower(ENUM_TO_STR(PenmanMonteith))) {
				etm = PETmethod::PenmanMonteith;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(PriestleyTaylor))) {
				etm = PETmethod::PriestleyTaylor;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Hargreaves))) {
				etm = PETmethod::Hargreaves;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(JensenHaise))) {
				etm = PETmethod::JensenHaise;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(BlaneyCriddle))) {
				etm = PETmethod::BlaneyCriddle;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Hamon))) {
				etm = PETmethod::Hamon;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Turc))) {
				etm = PETmethod::Turc;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(None))) {
				etm = PETmethod::None;
			}
			else {
				writeLog(fpnLog, "Evaportranspiration method in the watershed ["
					+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Evaportranspiration method in the watershed ["
				+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		petsmi->petMethod = etm;
		return 1;
	}
	if (aline.find(fldName.PETDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PETDataFile);
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			petsmi->fpnPET = vString;
		}
		else  if (vString != ""){
			writeLog(fpnLog, "PET data file in the watershed ["
				+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.PETcoeffPlant) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PETcoeffPlant);
		if (vString != "" ){
			if (stod(vString) >= 0.0) {
				petsmi->PETcoeffPlant = stod(vString);
			}
			else {
				writeLog(fpnLog, "PET coefficient for plant in the area ["
					+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		return 1;
	}
	if (aline.find(fldName.PETcoeffSoil) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PETcoeffSoil);
		if (vString != "") {
			if (stod(vString) >= 0.0) {
				petsmi->PETcoeffSoil = stod(vString);
			}
			else {
				writeLog(fpnLog, "PET coefficient for soil in the area ["
					+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		return 1;
	}
	if (aline.find(fldName.SnowMeltMethod) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowMeltMethod);
		snowMeltMethod smm = snowMeltMethod::notSet; // 이것으로 설정되면, 애러로 처리 
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(UserData))) {
				smm = snowMeltMethod::UserData; // constant로 하고 그 값을 0으로 하면 융설 없음.
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Amethod))) {
				smm = snowMeltMethod::Amethod;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(None))) {
				smm = snowMeltMethod::None;
			}
			else {
				writeLog(fpnLog, "Snow melt method in the watershed ["
					+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Snow melt method in the watershed ["
				+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		petsmi->smMethod = smm;
		return 1;
	}
	if (aline.find(fldName.SnowMeltDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowMeltDataFile);
	    if (vString != "" && _access(vString.c_str(), 0) == 0) {
			petsmi->fpnSnowMelt = vString;
		}
		else  if (vString != ""){
			writeLog(fpnLog, "Snow melt data file in the watershed ["
				+ to_string(petsmi->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	return 1;
}

int readXmlRowFlowControlGrid(string aline, flowControlinfo* fci) {
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find("<"+fldName.Name_FCG+">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.Name_FCG);
		if (vString != "") {
			fci->fcName = vString;
		}
		return 1;
	}
	if (aline.find(fldName.ColX_FCG) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ColX_FCG);
		if (vString != "" && stoi(vString) >= 0) {
			fci->fcColX = stoi(vString);
		}
		else {
			writeLog(fpnLog, "ColX value of [" + fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.RowY_FCG) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.RowY_FCG);
		if (vString != "" && stoi(vString) >= 0) {
			fci->fcRowY = stoi(vString);
		}
		else {
			writeLog(fpnLog, "RowY  value of [" + fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ControlType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ControlType);
		flowControlType  afct = flowControlType::None;
		if (vString != "") {
			if (lower(vString) == "inlet") {
				afct = flowControlType::Inlet;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(ReservoirOperation))) {
				afct = flowControlType::ReservoirOperation;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(ReservoirOutflow))) {
				afct = flowControlType::ReservoirOutflow;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SinkFlow))) {
				afct = flowControlType::SinkFlow;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(SourceFlow))) {
				afct = flowControlType::SourceFlow;
			}
			else {
				writeLog(fpnLog, "Flow control type of ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Flow control type of ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		fci->fcType = afct;
		return 1;
	}
	if (aline.find(fldName.FCDT_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FCDT_min);
		if (vString != "" && stoi(vString) >= 0) {
			fci->fcDT_min = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Flow control data time interval of ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.FlowDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowDataFile);
		if (vString != "" && _access(vString.c_str(), 0) == 0) {
			fci->fpnFCData = vString;
		}
		else if (vString != ""){
			writeLog(fpnLog, "Flow control data file of ["
				+ fci->fcName + "]("+ vString+") is invalid.\n", 1, 1);
		//	return -1;
		}
	}
/*	if (fci->fcType == flowControlType::ReservoirOperation
		|| fci->fcType == flowControlType::SourceFlow
		|| fci->fcType == flowControlType::SinkFlow)*/ 
	if (fci->fcType != flowControlType::Inlet) {
		if (aline.find(fldName.IniStorage) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.IniStorage);
			vString = replaceText(vString, ",", "");
			if (vString != "" && stod(vString) >= 0) {
				fci->iniStorage_m3 = stod(vString);
			}
			else if(fci->fcType == flowControlType::ReservoirOperation){
				writeLog(fpnLog, "Ini. storage of reservoir ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		if (aline.find("<"+fldName.MaxStorage+">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.MaxStorage);
			vString = replaceText(vString, ",", "");
			if (vString != "" && stod(vString) >= 0) {// max storage must be greater than zero.
				if (fci->fcType == flowControlType::ReservoirOperation
					&& stod(vString) == 0) {
					writeLog(fpnLog, "Max. storage of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				fci->maxStorage_m3 = stod(vString);
			}
			else if(fci->fcType == flowControlType::ReservoirOperation){
				writeLog(fpnLog, "Max. storage of reservoir ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		if (aline.find("<"+fldName.MaxStorageR+">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.MaxStorageR);
			if (vString != "" && stod(vString) >= 0) {// max storage must be greater than zero.
				if (fci->fcType == flowControlType::ReservoirOperation
					&& stod(vString) == 0) {
					writeLog(fpnLog, "Max. storage ratio of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				fci->maxStorageR = stod(vString);
			}
			else if(fci->fcType == flowControlType::ReservoirOperation) {
				writeLog(fpnLog, "Max. storage ratio of reservoir ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		if (aline.find(fldName.ROType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.ROType);
			reservoirOperationType  arot = reservoirOperationType::None;
			if (vString != "") {
				if (lower(vString) == lower(ENUM_TO_STR(AutoROM))) {
					arot = reservoirOperationType::AutoROM;
				}
				else if (lower(vString) == lower(ENUM_TO_STR(ConstantQ))) {
					arot = reservoirOperationType::ConstantQ;
				}
				else if (lower(vString) == lower(ENUM_TO_STR(RigidROM))) {
					arot = reservoirOperationType::RigidROM;
				}
				else if (lower(vString) == lower(ENUM_TO_STR(SDEqation))) {
					arot = reservoirOperationType::SDEqation;
				}
				else {
					writeLog(fpnLog, "Reservoir operation type of ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else if(fci->fcType == flowControlType::ReservoirOperation){
				writeLog(fpnLog, "Reservoir operation type of ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			fci->roType = arot;
			return 1;
		}
		if (fci->roType == reservoirOperationType::ConstantQ) {
			if (aline.find(fldName.ROConstQ) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fldName.ROConstQ);
				if (vString != "" && stod(vString) >= 0) {
					fci->roConstQ_cms = stod(vString);
				}
				else {
					writeLog(fpnLog, "Constant outlfow of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				return 1;
			}
			if (aline.find(fldName.ROConstQDuration) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fldName.ROConstQDuration);
				if (vString != "" && stod(vString) >= 0) {
					fci->roConstQDuration_hr = stod(vString);
				}
				else {
					writeLog(fpnLog, "Constant outlfow duration of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				return 1;
			}
		}
	}
	return 1;
}

int readXmlRowChannelSettings(string aline, channelSettingInfo *csi)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find("<"+fldName.WSID_CH+">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.WSID_CH);
		if (vString != "" && stoi(vString) > 0) {
			csi->mdWsid = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Most downstream watershed ID for channel setting is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CrossSectionType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CrossSectionType);
		crossSectionType acst = crossSectionType::None;
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(CSCompound))) {
				acst = crossSectionType::CSCompound;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(CSSingle))) {
				acst = crossSectionType::CSSingle;
			}
			else {
				writeLog(fpnLog, "Channel cross section type in the watershed ["
					+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Channel cross section type in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		csi->csType = acst;
		return 1;
	}
	if (aline.find(fldName.SingleCSChannelWidthType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SingleCSChannelWidthType);
		channelWidthType  acwt = channelWidthType::None;
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(CWEquation))) {
				acwt = channelWidthType::CWEquation;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(CWGeneration))) {
				acwt = channelWidthType::CWGeneration;
			}
			else {
				writeLog(fpnLog, "Channel width type in the watershed ["
					+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Channel width type in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		csi->csWidthType = acwt;
		return 1;
	}
	if (aline.find(fldName.ChannelWidthEQc) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthEQc);
		if (vString != "" && stod(vString) > 0) {
			csi->cwEQc = stod(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWEquation) {
			writeLog(fpnLog, "EQc parameter for channel width eq. in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ChannelWidthEQd) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthEQd);
		if (vString != "" && stod(vString) > 0) {
			csi->cwEQd = stod(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWEquation) {
			writeLog(fpnLog, "EQd parameter for channel width eq. in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ChannelWidthEQe) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthEQe);
		if (vString != "" && stod(vString) > 0) {
			csi->cwEQe = stod(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWEquation) {
			writeLog(fpnLog, "EQe parameter for channel width eq. in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ChannelWidthMostDownStream) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthMostDownStream);
		vString = replaceText(vString, ",", "");
		if (vString != "" && stod(vString) > 0) {
			csi->cwMostDownStream = stod(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWGeneration) {
			writeLog(fpnLog, "The channel width at the most down stream in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.LowerRegionHeight) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LowerRegionHeight);
		if (vString != "" && stod(vString) > 0) {
			csi->lowRHeight = stod(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLog(fpnLog, "Lower region height parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.LowerRegionBaseWidth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LowerRegionBaseWidth);
		if (vString != "" && stod(vString) > 0) {
			csi->lowRBaseWidth = stod(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLog(fpnLog, "Lower region base width parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.UpperRegionBaseWidth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UpperRegionBaseWidth);
		if (vString != "" && stod(vString) > 0) {
			csi->highRBaseWidth = stod(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLog(fpnLog, "Upper region base width parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CompoundCSChannelWidthLimit) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CompoundCSChannelWidthLimit);
		if (vString != "" && stod(vString) > 0) {
			csi->compoundCSChannelWidthLimit = stod(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLog(fpnLog, "Compound cross section width limit parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.BankSideSlopeRight) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.BankSideSlopeRight);
		if (vString != "" && stod(vString) > 0) {
			csi->bankSlopeRight = stod(vString);
		}
		else {
			writeLog(fpnLog, "Right bank side slope parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.BankSideSlopeLeft) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.BankSideSlopeLeft);
		if (vString != "" && stod(vString) > 0) {
			csi->bankSlopeLeft = stod(vString);
		}
		else {
			writeLog(fpnLog, "Left bank side slope parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	return 1;

}

int readXmlRowSubWatershedSettings(string aline, swsParameters * ssp)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find("<"+fldName.ID_SWP+">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ID_SWP);
		if (vString != "" && stoi(vString) > 0) {
			ssp->wsid = stoi(vString);
		}
		else {
			writeLog(fpnLog, "ID in subWatershed data is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.IniSaturation) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.IniSaturation);
		if (vString != "" && stod(vString) >= 0) {
			ssp->iniSaturation = stod(vString);
		}
		else {
			writeLog(fpnLog, "Ini. saturation ratio in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.MinSlopeOF) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MinSlopeOF);
		if (vString != "" && stod(vString) > 0) {
			ssp->minSlopeOF = stod(vString);
		}
		else {
			writeLog(fpnLog, "Minimum land surface slope for overland flow in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.UnsaturatedKType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UnsaturatedKType);
		unSaturatedKType uskt = unSaturatedKType::None;
		if (vString != "") {
			if (lower(vString) == lower(ENUM_TO_STR(Constant))) {
				uskt = unSaturatedKType::Constant;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Linear))) {
				uskt = unSaturatedKType::Linear;
			}
			else if (lower(vString) == lower(ENUM_TO_STR(Exponential))) {
				uskt = unSaturatedKType::Exponential;
			}
			else {
				writeLog(fpnLog, "Unsaturated K type in the watershed ["
					+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLog(fpnLog, "Unsaturated K type in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		ssp->unSatKType = uskt;
		return 1;
	}
	if (aline.find(fldName.CoefUnsaturatedK) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CoefUnsaturatedK);
		if (vString != "" && stod(vString) > 0) {
			ssp->coefUnsaturatedK = stod(vString);
		}
		else {
			writeLog(fpnLog, "Hydraulic conductivity for unsaturated soil in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MinSlopeChBed) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MinSlopeChBed);
		if (vString != "" && stod(vString) > 0) {
			ssp->minSlopeChBed = stod(vString);
		}
		else {
			writeLog(fpnLog, "Minimum slope of channel bed in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MinChBaseWidth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MinChBaseWidth);
		if (vString != "" && stod(vString) > 0) {
			ssp->minChBaseWidth = stod(vString);
		}
		else {
			writeLog(fpnLog, "Minimum value of channel width in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ChRoughness) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChRoughness);
		if (vString != "" && stod(vString) > 0) {
			ssp->chRoughness = stod(vString);
		}
		else {
			writeLog(fpnLog, "Roughness coefficient of channel in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.DryStreamOrder) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DryStreamOrder);
		if (vString != "" && stoi(vString) >= 0) {
			ssp->dryStreamOrder = stoi(vString);
		}
		else {
			writeLog(fpnLog, "Dry stream order in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.IniFlow) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.IniFlow);
		if (vString != "" && stod(vString) >= 0) {
			ssp->iniFlow = stod(vString);
		}
		else {
			writeLog(fpnLog, "Initial stream flow in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CalCoefLCRoughness) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CalCoefLCRoughness);
		if (vString != "" && stod(vString) > 0) {
			ssp->ccLCRoughness = stod(vString);
		}
		else {
			writeLog(fpnLog, "Calibration coeff. of roughness in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CalCoefPorosity) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CalCoefPorosity);
		if (vString != "" && stod(vString) > 0) {
			ssp->ccPorosity = stod(vString);
		}
		else {
			writeLog(fpnLog, "Calibration coeff. of porosity in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CalCoefWFSuctionHead) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CalCoefWFSuctionHead);
		if (vString != "" && stod(vString) > 0) {
			ssp->ccWFSuctionHead = stod(vString);
		}
		else {
			writeLog(fpnLog, "Calibration coeff. of wetting front suction head in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CalCoefHydraulicK) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CalCoefHydraulicK);
		if (vString != "" && stod(vString) > 0) {
			ssp->ccHydraulicK = stod(vString);
		}
		else {
			writeLog(fpnLog, "Calibration coeff. of hydraulic conductivity in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CalCoefSoilDepth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CalCoefSoilDepth);
		if (vString != "" && stod(vString) > 0) {
			ssp->ccSoilDepth = stod(vString);
		}
		else {
			writeLog(fpnLog, "Calibration coeff. of soil depth in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.UserSet) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UserSet);
		if (lower(vString) == "true") {
			ssp->userSet = 1;
		}
		else if (lower(vString) == "false") {
			ssp->userSet = 0;
		}
		return 1;
	}
	return 1;
}


int isNormalChannelSettingInfo(channelSettingInfo *ci)
{
	channelSettingInfo aci_ini; //여기서 생성된 초기값과 서로 비교
	if (ci->mdWsid == aci_ini.mdWsid) { return -1; }
	if (ci->csType == aci_ini.csType) { return -1; }
	if (ci->csWidthType == aci_ini.csWidthType) { return -1; }
	if (ci->csWidthType == channelWidthType::CWEquation) {
		if (ci->cwEQc == aci_ini.cwEQc) { return -1; }
		if (ci->cwEQd == aci_ini.cwEQd) { return -1; }
		if (ci->cwEQe == aci_ini.cwEQe) { return -1; }
	}
	else if (ci->csWidthType == channelWidthType::CWGeneration) {
		if (ci->cwMostDownStream == aci_ini.cwMostDownStream) { return -1; }
	}
	if (ci->csType == crossSectionType::CSCompound) {
		if (ci->lowRHeight == aci_ini.lowRHeight) { return -1; }
		if (ci->lowRBaseWidth == aci_ini.lowRBaseWidth) { return -1; }
		if (ci->highRBaseWidth == aci_ini.highRBaseWidth) { return -1; }
		if (ci->compoundCSChannelWidthLimit == aci_ini.compoundCSChannelWidthLimit) { return -1; }
	}
	if (ci->bankSlopeRight == aci_ini.bankSlopeRight) { return -1; }
	if (ci->bankSlopeLeft == aci_ini.bankSlopeLeft) { return -1; }
	return 1;
}

int isNormalFlowControlinfo(flowControlinfo* fci)
{
	flowControlinfo afc_ini;
	//여기서 생성된 초기값과 서로 비교
	if (fci->fcName == afc_ini.fcName) { return -1; }
	if (fci->fcColX == afc_ini.fcColX) { return -1; }
	if (fci->fcRowY == afc_ini.fcRowY) { return -1; }
	if (fci->fcType == afc_ini.fcType) { return -1; }
	if (fci->fcDT_min == afc_ini.fcDT_min) { return -1; }
	if (fci->fcType == flowControlType::ReservoirOperation) {
		if (fci->iniStorage_m3 == afc_ini.iniStorage_m3) { return -1; }
		if (fci->maxStorage_m3 == afc_ini.maxStorage_m3) { return -1; }
		if (fci->maxStorageR == afc_ini.maxStorageR) { return -1; }
		if (fci->roType == afc_ini.roType) { return -1; }
		if (fci->roType == reservoirOperationType::ConstantQ) {
			if (fci->roConstQ_cms == afc_ini.roConstQ_cms) { return -1; }
			if (fci->roConstQDuration_hr == afc_ini.roConstQDuration_hr) { return -1; }
		}
	}
	else if (fci->fpnFCData == afc_ini.fpnFCData) { return -1; }
	return 1;
}


int isNormalSwsParameter(swsParameters *ssp)
{
	swsParameters swsp_ini;//여기서 생성된 초기값과 서로 비교
	if (ssp->wsid == swsp_ini.wsid) { return -1; }
	if (ssp->iniSaturation == swsp_ini.iniSaturation) { return -1; }
	if (ssp->unSatKType == swsp_ini.unSatKType) { return -1; }
	if (ssp->coefUnsaturatedK == swsp_ini.coefUnsaturatedK) { return -1; }
	if (ssp->minSlopeOF == swsp_ini.minSlopeOF) { return -1; }
	if (ssp->minSlopeChBed == swsp_ini.minSlopeChBed) { return -1; }
	if (ssp->minChBaseWidth == swsp_ini.minChBaseWidth) { return -1; }
	if (ssp->chRoughness == swsp_ini.chRoughness) { return -1; }
	if (ssp->dryStreamOrder == swsp_ini.dryStreamOrder) { return -1; }
	if (ssp->iniFlow == swsp_ini.iniFlow) { return -1; }
	if (ssp->ccLCRoughness == swsp_ini.ccLCRoughness) { return -1; }
	if (ssp->ccPorosity == swsp_ini.ccPorosity) { return -1; }
	if (ssp->ccWFSuctionHead == swsp_ini.ccWFSuctionHead) { return -1; }
	if (ssp->ccHydraulicK == swsp_ini.ccHydraulicK) { return -1; }
	if (ssp->ccSoilDepth == swsp_ini.ccSoilDepth) { return -1; }
	if (ssp->userSet == swsp_ini.userSet) { return -1; }
	return 1;
}

// 이건 continuous 용 =====================
int isNormalPETnSnowMelt(PETnSMinfo* petsmi)
{
	PETnSMinfo petsmi_ini; //여기서 생성된 초기값과 서로 비교
	if (petsmi->petMethod == petsmi_ini.petMethod) { return -1; }
	if (petsmi->petMethod == PETmethod::UserData) {
		if (petsmi->fpnPET == petsmi_ini.fpnPET) { return -1; }
	}
	if (petsmi->smMethod == petsmi_ini.smMethod) { return -1; }
	if (petsmi->smMethod == snowMeltMethod::UserData) {
		if (petsmi->fpnSnowMelt == petsmi_ini.fpnSnowMelt) { return -1; }
	}
	return 1;
}

int isNormalWatchPointInfo(wpLocationRC * wpL)
{
	wpLocationRC wpi_ini;//여기서 생성된 초기값과 서로 비교
	if (wpL->wpName == wpi_ini.wpName){ return -1; };
	if (wpL->wpColX == wpi_ini.wpColX) { return -1; };
	if (wpL->wpRowY == wpi_ini.wpRowY) { return -1; };
	return 1;
}

int isNormalSoilTextureInfo(soilTextureInfo *st)
{
	soilTextureInfo sti;//여기서 생성된 초기값과 서로 비교
	if (st->stGridValue == sti.stGridValue) { return -1; }
	if (st->stCode == sti.stCode) { return -1; }
	if (st->porosity == sti.porosity) { return -1; }
	if (st->effectivePorosity == sti.effectivePorosity) { return -1; }
	if (st->WFSuctionHead == sti.WFSuctionHead) { return -1; }
	if (st->hydraulicK == sti.hydraulicK) { return -1; }
	return 1;
}

int isNormalSoilDepthInfo(soilDepthInfo* sd)
{
	soilDepthInfo sdi;//여기서 생성된 초기값과 서로 비교
	if (sd->sdGridValue == sdi.sdGridValue) { return -1; }
	if (sd->sdCode == sdi.sdCode) { return -1; }
	if (sd->soilDepth == sdi.soilDepth ) { return -1; }
	return 1;
}

int isNormalLandCoverInfo(landCoverInfo *lc)
{
	landCoverInfo lci;//여기서 생성된 초기값과 서로 비교하는 것으로 수정 필요?
	if(lc->lcGridValue == lci.lcGridValue) { return -1; }
	if (lc->lcCode == lci.lcCode) { return -1; }
	if (lc->RoughnessCoefficient == lci.RoughnessCoefficient) { return -1; }
	if (lc->ImperviousRatio == lci.ImperviousRatio) { return -1; }
	return 1;
}

void updateAllSWSParsUsingNetwork()
{
	for (int wsid : di.dmids) {
		if (prj.swps[wsid].userSet == 1) {
			vector<int> wsidToExclude;
			for (int upsid : di.wsn.wsidsAllUp[wsid]) {
				if (prj.swps[upsid].userSet == 1) {
					if (getVectorIndex(wsidToExclude, upsid) == -1) {
						wsidToExclude.push_back(upsid);
					}
					for (int upupID : di.wsn.wsidsAllUp[upsid]) {
						if (getVectorIndex(wsidToExclude, upupID) == -1) {
							wsidToExclude.push_back(upupID);
						}
					}
				}
			}
			for (int upsid : di.wsn.wsidsAllUp[wsid]) {
				if (getVectorIndex(wsidToExclude, upsid) == -1) {
					updateOneSWSParsWithOtherSWSParsSet(upsid, wsid);
				}
			}
		}
	}
}

bool updateOneSWSParsWithOtherSWSParsSet(int targetWSid, int referenceWSid)
{
	swsParameters spars = prj.swps[referenceWSid];
	prj.swps[targetWSid].iniSaturation = spars.iniSaturation;
	prj.swps[targetWSid].minSlopeOF = spars.minSlopeOF;
	prj.swps[targetWSid].unSatKType = spars.unSatKType;
	prj.swps[targetWSid].coefUnsaturatedK = spars.coefUnsaturatedK;
	prj.swps[targetWSid].minSlopeChBed = spars.minSlopeChBed;
	prj.swps[targetWSid].minChBaseWidth = spars.minChBaseWidth;
	prj.swps[targetWSid].chRoughness = spars.chRoughness;
	prj.swps[targetWSid].dryStreamOrder = spars.dryStreamOrder;
	prj.swps[targetWSid].ccLCRoughness = spars.ccLCRoughness;
	prj.swps[targetWSid].ccPorosity = spars.ccPorosity;
	prj.swps[targetWSid].ccWFSuctionHead = spars.ccWFSuctionHead;
	prj.swps[targetWSid].ccHydraulicK = spars.ccHydraulicK;
	prj.swps[targetWSid].ccSoilDepth = spars.ccSoilDepth;
	if (prj.swps[targetWSid].userSet != 1) {
		prj.swps[targetWSid].iniFlow = 0;
		prj.swps[targetWSid].userSet = 0;
	}
	return true;
}





