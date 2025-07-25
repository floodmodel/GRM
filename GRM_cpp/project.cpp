#include "stdafx.h"
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;
namespace fs = std::filesystem;

vector<flowControlinfo> fcinfos;
extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

extern domaininfo di;
extern thisSimulation ts;

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
	projectFileFieldName fn;
	swsParameters* aswp;
	aswp = new swsParameters;
	channelSettingInfo* acs;
	acs = new channelSettingInfo;
	flowControlinfo* afc;
	afc = new flowControlinfo;
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
	int sbWatchPoints = 0; //0:비활성, 1: 활성
	int sbGreenAmptParameter = 0; //0:비활성, 1: 활성
	int sbSoilDepth = 0; //0:비활성, 1: 활성
	int sbLandCover = 0; //0:비활성, 1: 활성

	int sbRTenv = 0; //0:비활성, 1: 활성
	prjFile = readTextFileToStringVector(ppi.fpn_prj);
	int LineCount = prjFile.size();

	for (int i = 0; i < LineCount; ++i) {
		string aline = prjFile[i];
		aline = eraseCR(aline);
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
			else {
				writeLogString(fpnLog, "ERROR : SubWatershedSettings data in [" + to_string(aswp->wsid)
					+ "] is invalid.\n", 1, 1);
				return -1;
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
			else {
				writeLogString(fpnLog, "ERROR : ChannelSettings data in [" + to_string(acs->mdWsid)
					+ "] is invalid.\n", 1, 1);
				return -1;
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
				int n = fcinfos.size();
				fcinfos.push_back(*afc);// 우선 idx를 키로 사용. updateFCCellinfoAndData()에서 cvid를 키로 업데이트
			}
			else {
				writeLogString(fpnLog, "ERROR : FlowControlGrid data in [" + afc->fcName
					+ "] is invalid.\n", 1, 1);
				return -1;
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

		if (sbWatchPoints == 1 && pt.sWatchPoints == 0) {
			sbWatchPoints = 0;
			if (awp->wpName != "" && isNormalWatchPointInfo(awp) == 1) {
				prj.wps.push_back(*awp);
				awp = new wpLocationRC;
			}
			else {
				writeLogString(fpnLog, "ERROR : WatchPoints data in [" + awp->wpName
					+ "] is invalid.\n", 1, 1);
				return -1;
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
				else {
					writeLogString(fpnLog, "ERROR : GreenAmptParameter data in [" + to_string(ast->stGridValue)
						+ "] is invalid.\n", 1, 1);
					return -1;
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
				else {
					writeLogString(fpnLog, "ERROR : SoilDepthInfo data in [" + to_string(asd->sdGridValue)
						+ "] is invalid.\n", 1, 1);
					return -1;
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
				else {
					writeLogString(fpnLog, "ERROR : LandCoverInfo data in [" + to_string(alc->lcGridValue)
						+ "] is invalid.\n", 1, 1);
					return -1;
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
	// flow control 관련
	if (prj.simFlowControl == 1 && fcinfos.size() > 0) {
		for (int i = 0; i < fcinfos.size(); ++i) {
			flowControlinfo afci;
			afci = fcinfos[i];
			for (int n = 0; n < fcinfos.size(); ++n) {
				if (n != i) {
					if (afci.fcColX == fcinfos[n].fcColX
						&& afci.fcRowY == fcinfos[n].fcRowY) {
						if (afci.fcType == fcinfos[n].fcType) {
							writeLogString(fpnLog, "ERROR : The same flow control type was assigned in (colX: "
								+ to_string(afci.fcColX) + ", rowY: " + to_string(afci.fcRowY) + ").\n", 1, 1);
							return -1;
						}
					}
				}
			}

			// 2023. 11.23. inlet, sinkflow, sourceflow 에서는 저수지 제원 입력 하지 않는다. AutoROM으로 자동으로 전환도 못하게 한다. 
			if ((afci.fcType == flowControlType::ReservoirOutflow && ts.enforceFCautoROM == 1)
				|| afci.fcType == flowControlType::ReservoirOperation
				|| afci.fcType == flowControlType::DetentionPond) {

				if (afci.iniStorage_m3 < 0) {
					writeLogString(fpnLog, "WARNNING : [" + afci.fcName + "] Ini. storage of the reservoir is smaller than '0'. '0' is applied.\n", 1, 1);
					fcinfos[i].iniStorage_m3 = 0.0;
				}

				if (afci.fcType == flowControlType::ReservoirOperation
					|| (afci.fcType == flowControlType::ReservoirOutflow && ts.enforceFCautoROM == 1)) {
					if (afci.maxStorage_m3 < 0 || afci.NormalHighStorage_m3 < 0
						|| afci.RestrictedStorage_m3 < 0) {
						writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] MaxStorage, NormalHighStorage, and  RestrictedStorage must not be negative value.\n", 1, 1);
						return -1;
					}
					if (prj.isDateTimeFormat == 1) {
						if (isNumeric(afci.RestrictedPeriod_Start) == true) {
							writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] RestrictedPeriod_Start value must have 'mmMddD' format.\n", 1, 1);
							return -1;
						}
						else {
							fcinfos[i].restricedP_SM = stoi(afci.RestrictedPeriod_Start.substr(0, 2));
							fcinfos[i].restricedP_SD = stoi(afci.RestrictedPeriod_Start.substr(3, 2));
						}
						if (isNumeric(afci.RestrictedPeriod_End) == true) {
							writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] RestrictedPeriod_End value must have 'mmMddD' format.\n", 1, 1);
							return -1;
						}
						else {
							fcinfos[i].restricedP_EM = stoi(afci.RestrictedPeriod_End.substr(0, 2));
							fcinfos[i].restricedP_ED = stoi(afci.RestrictedPeriod_End.substr(3, 2));
						}
						if (afci.restricedP_SM > afci.restricedP_EM) {
							writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] Restriced storage period values are invalid.\n", 1, 1);
							return -1;
						}
						if (afci.restricedP_SM == afci.restricedP_EM) {
							if (afci.restricedP_SD > afci.restricedP_ED) {
								writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] Restriced storage period values are invalid.\n", 1, 1);
								return -1;
							}
						}
					}
					if (afci.NormalHighStorage_m3 > afci.maxStorage_m3) {
						writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] NormalHighStorage storage(" + dtos(afci.NormalHighStorage_m3, 0)
							+ "m^3) is greater than MaxStorage storage(" + dtos(afci.maxStorage_m3, 0) + "m^3). \n", 1, 1);
						return -1;
					}
					if (afci.RestrictedStorage_m3 > afci.maxStorage_m3) {
						writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] RestrictedStorage storage(" + dtos(afci.RestrictedStorage_m3, 0)
							+ "m^3) is greater than MaxStorage storage(" + dtos(afci.maxStorage_m3, 0) + "m^3). \n", 1, 1);
						return -1;
					}

					if (prj.isDateTimeFormat == -1) {
						if (isNumeric(afci.RestrictedPeriod_Start) == false) {
							writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] RestrictedPeriod_Start has to be numeric value.\n", 1, 1);
							return -1;
						}
						else {
							fcinfos[i].RestrictedPeriod_Start_min = stoi(afci.RestrictedPeriod_Start) * 60;
						}
						if (isNumeric(afci.RestrictedPeriod_End) == false) {
							writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] RestrictedPeriod_End has to be numeric value.\n", 1, 1);
							return -1;
						}
						else {
							fcinfos[i].RestrictedPeriod_End_min = stoi(afci.RestrictedPeriod_End) * 60;
						}
						if (afci.RestrictedPeriod_Start_min > afci.RestrictedPeriod_End_min) {
							writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] Restriced storage period values are invalid.\n", 1, 1);
							return -1;
						}
					}
					if (afci.roType == reservoirOperationType::AutoROM && afci.autoROMmaxOutflow_cms < 0) {
						writeLogString(fpnLog, "WARNNING : [" + afci.fcName + "] AutoROM max outflow of the reservoir is smaller than '0'. '0' is applied and the outflow is not limited.\n", 1, 1);
						fcinfos[i].autoROMmaxOutflow_cms = 0.0;
					}
				}

				if (afci.iniStorage_m3 > afci.maxStorage_m3) {
					writeLogString(fpnLog, "ERROR : [" + afci.fcName + "] Initial storage(" + dtos(afci.iniStorage_m3, 0)
						+ "m^3) is greater than MaxStorage storage(" + dtos(afci.maxStorage_m3, 0) + "m^3). \n", 1, 1);
					return -1;
				}
			}
			else if (ts.enforceFCautoROM == 1) {// inlet, sinkflow, sourceflow 인경우, AutoROM으로 자동 전환 못하게 한다.
				writeLogString(fpnLog, "ERROR : [" + afci.fcName + "]. Inlet, Sinkflow, or Sourceflow cannot be converted to AutoROM. '/a' option cannot be applied.\n", 1, 1);
				return -1;
			}

		}
	}
	else {
		prj.simFlowControl = -1;
		prj.isinletExist = -1;
	}

	// continuous =====================
	if (prj.simEvaportranspiration == 1) {
		if (prj.isDateTimeFormat == -1) {
			writeLogString(fpnLog, "ERROR : The type of simulation time must be [date and time format] for calculating evaportranspiration.\n", 1, 1);
			return -1;
		}
		// 기온은 모든 잠재증발산량 산정 방법에서 사용되므로, 여기서 검토
		if (prj.tempMaxDataType == weatherDataType::None) {
			writeLogString(fpnLog, "ERROR : Max temperature data type is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.tempMaxInterval_min <= 0) {
			writeLogString(fpnLog, "ERROR : Max temperature data interval is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnTempMaxData == "") {
			writeLogString(fpnLog, "ERROR : Max temperature data file is invalid.\n", 1, 1);
			return -1;
		}
		ts.wdUsed_tempMax = 1;

		if (prj.tempMinDataType == weatherDataType::None) {
			writeLogString(fpnLog, "ERROR : Min temperature data type is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.tempMinInterval_min <= 0) {
			writeLogString(fpnLog, "ERROR : Min temperature data interval is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnTempMinData == "") {
			writeLogString(fpnLog, "ERROR : Min temperature data file is invalid.\n", 1, 1);
			return -1;
		}
		ts.wdUsed_tempMin = 1;

		for (const auto& mpair : prj.swps) {
			if (prj.swps[mpair.first].potentialETMethod == PETmethod::None) {
				writeLogString(fpnLog, "ERROR : The potential ET method was not set [watershed id = "
					+ to_string(prj.swps[mpair.first].wsid) + "].\n", 1, 1);
				return -1;
			}

			// 방법별 기상자료 확인 =========================
			// 소유역별 잠재증발산 산정 방법에 따라서 필요한 자료가 다르므로, 여기서 검토한다.
			if (prj.swps[mpair.first].potentialETMethod == PETmethod::BC) {
				if (prj.fpnBlaneyCriddleK == "") {
					writeLogString(fpnLog, "ERROR : The file of crop ceofficient data in Blaney-Criddle method is invalid.\n", 1, 1);
					return -1;
				}
				ts.isUsed_BlaneyCriddleK_File = 1;

				if (prj.latitudeTOP_degree == -9999.0) {
					writeLogString(fpnLog, "ERROR : Latitude of the top of the domain is invalid.\n", 1, 1);
					return -1;
				}
				ts.isUsed_Latitude = 1;
			}
			if (prj.swps[mpair.first].potentialETMethod == PETmethod::HMN) {
				// 일조시간
				if (prj.DTLDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Daytime length data type is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.DTLDataInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : Daytime length data interval is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.fpnDTLData == "") {
					writeLogString(fpnLog, "ERROR : Daytime length data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.wdUsed_DTL = 1;
			}
			if (prj.swps[mpair.first].potentialETMethod == PETmethod::HRGV) {
				// 일사량
				if (prj.solarRadDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Solar radiation data type is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.solarRadInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : Solar radiation data interval is invalid.\n", 1, 1);
					return -1;
				}
				//DEM
				if (prj.latitudeTOP_degree ==-9999.0) {
					writeLogString(fpnLog, "ERROR : Latitude of the top of the domain is invalid.\n", 1, 1);
					return -1;
				}
				ts.isUsed_Latitude = 1;
			}

			if (prj.swps[mpair.first].potentialETMethod == PETmethod::FPM) { // 일사량은 이 방법들 모두에서 사용된다.
				// 일사량
				if (prj.solarRadDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Solar radiation data type is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.solarRadInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : Solar radiation data interval is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.fpnSolarRadData == "") {
					writeLogString(fpnLog, "ERROR : Solar radiation data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.wdUsed_solarR = 1;

				// 풍속
				if (prj.windSpeedDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Wind speed data type is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.windSpeedInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : Wind speed data interval is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.fpnWindSpeed == "") {
					writeLogString(fpnLog, "ERROR : Wind speed data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.wdUsed_windSpeed = 1;

				// 이슬점 온도
				if (prj.dewPointTempDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Dew point temperature data type is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.dewPointTempInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : Dew point temperature data interval is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.fpnDewPointTemp == "") {
					writeLogString(fpnLog, "ERROR : Dew point temperature data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.wdUsed_dewPointTemp = 1;
				//DEM
				if (prj.fpnDEM == "") {
					writeLogString(fpnLog, "ERROR : Elevation data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.isUsed_DEM = 1;
				if (prj.latitudeTOP_degree == -9999.0) {
					writeLogString(fpnLog, "ERROR : Latitude of the top of the domain is invalid.\n", 1, 1);
					return -1;
				}
				ts.isUsed_Latitude = 1;
			}

			if (prj.swps[mpair.first].potentialETMethod == PETmethod::PT) {
				// 일사량
				if (prj.solarRadDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Solar radiation data type is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.solarRadInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : Solar radiation data interval is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.fpnSolarRadData == "") {
					writeLogString(fpnLog, "ERROR : Solar radiation data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.wdUsed_solarR = 1;

				// 이슬점 온도
				if (prj.dewPointTempDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Dew point temperature data type is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.dewPointTempInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : Dew point temperature data interval is invalid.\n", 1, 1);
					return -1;
				}
				if (prj.fpnDewPointTemp == "") {
					writeLogString(fpnLog, "ERROR : Dew point temperature data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.wdUsed_dewPointTemp = 1;

				//DEM
				if (prj.fpnDEM == "") {
					writeLogString(fpnLog, "ERROR : Elevation data file is invalid.\n", 1, 1);
					return -1;
				}
				ts.isUsed_DEM = 1;
				if (prj.latitudeTOP_degree == -9999.0) {
					writeLogString(fpnLog, "ERROR : Latitude of the top of the domain is invalid.\n", 1, 1);
					return -1;
				}
				ts.isUsed_Latitude = 1;
			}

			if (prj.swps[mpair.first].potentialETMethod == PETmethod::UserET) {
				bool isPET = true;
				bool isAET = true;
				ts.wdUsed_userET = -1;
				if (prj.userPETDataType == weatherDataType::None) {
					//writeLogString(fpnLog, "ERROR : User potential evapotranspiration data type is invalid.\n", 1, 1);
					//return -1;
					isPET = false;
				}
				if (prj.fpnUserPET == "") {
					isPET = false;
				}

				if (prj.userAETDataType == weatherDataType::None) {
					isAET = false;
				}
				if (prj.fpnUserAET == "") {
					isAET = false;
				}


				if (isPET == false && isAET == false) {
					writeLogString(fpnLog, "ERROR : User potential (or actual) evapotranspiration data file (or data type) is invalid.\n", 1, 1);
					return -1;
				}

				if (isPET == true) {
					ts.wdUsed_userET = 1;
				}

				if (isAET == true) { // pet와 aet 모두 true 이면, aet를 우선적으로 사용한다.
					ts.wdUsed_userET = 2;
				}

				if (prj.userETInterval_min <= 0) {
					writeLogString(fpnLog, "ERROR : User evapotranspiration data interval is invalid.\n", 1, 1);
					ts.wdUsed_userET = -1;
					return -1;

				}				
			}
			// 방법별 기상자료 확인 =========================

			if (prj.swps[mpair.first].etCoeff == -1.0) {
				writeLogString(fpnLog, "ERROR : The evaportranspiration coefficient of the watershed ["
					+ to_string(prj.swps[mpair.first].wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}

		int nlc = prj.lcs.size();
		for (int n = 0; n < nlc; ++n) {
			if (prj.lcs[n].CanopyRatio < 0
				|| prj.lcs[n].CanopyRatio >1) {
				writeLogString(fpnLog, "ERROR : Land cover canopy ratio of the raster value ["
					+ to_string(prj.lcs[n].lcGridValue) + "] of the land cover data is greater than 1 or wrong vaule.\n", 1, 1);
				return -1;
			}
		}
	}

	if (prj.simSnowMelt == 1) {
		if (prj.isDateTimeFormat == -1) {
			writeLogString(fpnLog, "ERROR : The type of simulation time must be [date and time format] for calculating snow melt.\n", 1, 1);
			return -1;
		}
		if (prj.tempMaxDataType == weatherDataType::None) {
			writeLogString(fpnLog, "ERROR : Max temperature data type is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.tempMaxInterval_min <= 0) {
			writeLogString(fpnLog, "ERROR : Max temperature data interval is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnTempMaxData == "") {
			writeLogString(fpnLog, "ERROR : Max temperature data file is invalid.\n", 1, 1);
			return -1;
		}
		ts.wdUsed_tempMax = 1;

		if (prj.tempMinDataType == weatherDataType::None) {
			writeLogString(fpnLog, "ERROR : Min temperature data type is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.tempMinInterval_min <= 0) {
			writeLogString(fpnLog, "ERROR : Min temperature data interval is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnTempMinData == "") {
			writeLogString(fpnLog, "ERROR : Min temperature data file is invalid.\n", 1, 1);
			return -1;
		}
		ts.wdUsed_tempMin = 1;

		if (prj.snowpackTempDataType == weatherDataType::None) {
			writeLogString(fpnLog, "ERROR : Snowpack temperature data type is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnSnowpackTempData == "") {
			writeLogString(fpnLog, "ERROR : Snowpack temperature data file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.snowpackTempInterval_min <= 0) {
			writeLogString(fpnLog, "ERROR : Snowpack temperature data interval is invalid.\n", 1, 1);
			return -1;
		}
		ts.wdUsed_snowPackTemp = 1;

		for (const auto& mpair : prj.swps) {
			if (prj.swps[mpair.first].snowMeltMethod == SnowMeltMethod::None) {
				writeLogString(fpnLog, "ERROR : The snow melt method was not set [watershed id = "
					+ to_string(prj.swps[mpair.first].wsid) + "].\n", 1, 1);
				return -1;
			}
			if (prj.swps[mpair.first].tempSnowRain < -9000 || prj.swps[mpair.first].tempSnowRain >20) { // 최대값을 20도로 가정한다.
				writeLogString(fpnLog, "ERROR : The value of the threshold temperature dividing snowfall and rainfall (SnowmeltTSR) is invalid [watershed id = "
					+ to_string(prj.swps[mpair.first].wsid) + "].\n", 1, 1);
				return -1;
			}
			if (prj.swps[mpair.first].smeltingTemp < -9000 || prj.swps[mpair.first].smeltingTemp>20) { // 최대값을 20도로 가정한다.
				writeLogString(fpnLog, "ERROR : The value of snow melting temperature (SnowmeltingTemp) is invalid [watershed id = "
					+ to_string(prj.swps[mpair.first].wsid) + "].\n", 1, 1);
				return -1;
			}
			if (prj.swps[mpair.first].snowCovRatio < 0 || prj.swps[mpair.first].snowCovRatio>1) {
				writeLogString(fpnLog, "ERROR : The value of snowpack coverage is invalid [watershed id = "
					+ to_string(prj.swps[mpair.first].wsid) + "].\n", 1, 1);
				return -1;
			}
			if (prj.swps[mpair.first].smeltCoef < 0) {
				writeLogString(fpnLog, "ERROR : The value of snow melt coefficient is invalid [watershed id = "
					+ to_string(prj.swps[mpair.first].wsid) + "].\n", 1, 1);
				return -1;
			}
		}

	}

	if (prj.simInterception == 1) {
		if (prj.isDateTimeFormat == -1) {
			writeLogString(fpnLog, "ERROR : The type of simulation time must be [date and time format] for calculating rainfall interception.\n", 1, 1);
			return -1;
		}
		if (prj.fpnLAI == "") {
			writeLogString(fpnLog, "ERROR : The LAI file is invalid.\n", 1, 1);
			return -1;
		}

		int nlc = prj.lcs.size();
		for (int n = 0; n < nlc; ++n) {
			if (prj.lcs[n].CanopyRatio < 0
				|| prj.lcs[n].CanopyRatio >1) {
				writeLogString(fpnLog, "ERROR : Land cover canopy ratio of the raster value ["
					+ to_string(prj.lcs[n].lcGridValue) + "] of the land cover data is greater than 1 or wrong vaule.\n", 1, 1);
				return -1;
			}
			if (prj.lcs[n].InterceptionMaxWaterCanopy_mm < 0) {
				writeLogString(fpnLog, "ERROR : Max interception water depth value in the raster value ["
					+ to_string(prj.lcs[n].lcGridValue) + "] of the land cover data is invalid.\n", 1, 1);
				return -1;
			}
		}
		for (const auto& mpair : prj.swps) {
			if (prj.swps[mpair.first].interceptMethod == InterceptionMethod::None) {
				writeLogString(fpnLog, "ERROR : The interception method was not set [watershed id = "
					+ to_string(prj.swps[mpair.first].wsid) + "].\n", 1, 1);
				return -1;
			}
		}
	}

	if (ts.wdUsed_tempMax == -1) {
		prj.tempMaxInterval_min = 0;
	}
	if (ts.wdUsed_tempMin == -1) {
		prj.tempMinInterval_min = 0;
	}
	if (ts.wdUsed_DTL == -1) {
		prj.DTLDataInterval_min = 0;
	}
	if (ts.wdUsed_solarR == -1) {
		prj.solarRadInterval_min = 0;
	}
	if (ts.wdUsed_dewPointTemp == -1) {
		prj.dewPointTempInterval_min = 0;
	}
	if (ts.wdUsed_windSpeed == -1) {
		prj.windSpeedInterval_min = 0;
	}
	if (ts.wdUsed_userET == -1) {
		prj.userETInterval_min = 0;
	}
	if (ts.wdUsed_snowPackTemp == -1) {
		prj.snowpackTempInterval_min = 0;
	}
	// continuous =====================


	// land cover ===============
	if (prj.lcDataType == fileOrConstant::File) {
		if (prj.fpnLC == "") {
			writeLogString(fpnLog, "ERROR : Land cover file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnLCVat == "") {
			writeLogString(fpnLog, "ERROR : Land cover VAT file is invalid.\n", 1, 1);
			return -1;
		}
		int nlc = prj.lcs.size();
		if (nlc == 0) {
			writeLogString(fpnLog, "ERROR : Land cover attributes from the files are invalid.\n", 1, 1);
			return -1;
		}
		else {
			for (int n = 0; n < nlc; ++n) {
				if (prj.lcs[n].ImperviousRatio < 0
					|| prj.lcs[n].ImperviousRatio >1) {
					writeLogString(fpnLog, "ERROR : Land cover impervious ratio of the raster value ["
						+ to_string(prj.lcs[n].lcGridValue) + "] of the land cover data is greater than 1 or wrong vaule.\n", 1, 1);
					return -1;
				}
				if (prj.lcs[n].RoughnessCoefficient < 0) {
					writeLogString(fpnLog, "ERROR : Land cover impervious ratio of the raster value ["
						+ to_string(prj.lcs[n].lcGridValue) + "] of the land cover data is greater than 1 or wrong vaule.\n", 1, 1);
					return -1;
				}
			}
		}
	}
	else if (prj.lcDataType == fileOrConstant::Constant) {
		if (prj.cnstRoughnessC == 0.0) {
			writeLogString(fpnLog, "ERROR : Land cover constant roughness is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstImperviousR < 0.0) {
			writeLogString(fpnLog, "ERROR : Land cover constant impervious ratio is invalid.\n", 1, 1);
			return -1;
		}
	}
	// ============== land cover

	if (prj.stDataType == fileOrConstant::File) {
		if (prj.fpnST == "") {
			writeLogString(fpnLog, "ERROR : Soil texture file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnSTVat == "") {
			writeLogString(fpnLog, "ERROR : Soil texture VAT file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.sts.size() == 0) {
			writeLogString(fpnLog, "ERROR : Soil texture attributes from the files are invalid.\n", 1, 1);
			return -1;
		}
	}
	else if (prj.stDataType == fileOrConstant::Constant) {
		if (prj.cnstSoilEffPorosity < 0) {
			writeLogString(fpnLog, "ERROR : Soil texture constant effective porosity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilHydraulicK < 0) {
			writeLogString(fpnLog, "ERROR : Soil texture constant hydraulic conductivity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilPorosity < 0) {
			writeLogString(fpnLog, "ERROR : Soil texture constant porosity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilWFSH < 0) {
			writeLogString(fpnLog, "ERROR : Soil texture constant wetting front suction head is invalid.\n", 1, 1);
			return -1;
		}
	}
	if (prj.sdDataType == fileOrConstant::File) {
		if (prj.fpnSD == "") {
			writeLogString(fpnLog, "ERROR : Soil depth file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnSDVat == "") {
			writeLogString(fpnLog, "ERROR : Soil depth VAT file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.sds.size() == 0) {
			writeLogString(fpnLog, "ERROR : Soil depth attributes from the files are invalid.\n", 1, 1);
			return -1;
		}
	}
	else if (prj.sdDataType == fileOrConstant::Constant) {
		if (prj.cnstSoilDepth < 0) {
			writeLogString(fpnLog, "ERROR : Soil depth constant value is invalid.\n", 1, 1);
			return -1;
		}
	}

	if (prj.printOption == GRMPrintType::AverageFile ||
		prj.printOption == GRMPrintType::AverageFileQ) {
		if (prj.printAveValue != 1) {
			writeLogString(fpnLog, "ERROR : Set [printAveValue as 'true'] to print average values.\n", 1, 1);
			return -1;
		}
	}

	if (prj.printAveValue == 1) {
		if (prj.dtPrintAveValue_min == 0 || prj.dtPrintAveValue_min < prj.dtPrint_min) {
			string outString = "WARNNING : The average value calculation time step (AveValueTimeInterval_min) is invalid.\n";
			outString = outString + "WARNNING : The average value calculation time step was set to the same value of 'OutputTimeStep_min'("
				+ to_string(prj.dtPrint_min) + "min).\n";
			writeLogString(fpnLog, outString, 1, 1);
			prj.dtPrintAveValue_min = prj.dtPrint_min;
		}
		if (prj.dtPrintAveValue_min % prj.dtPrint_min) { // 나머지가 있으면
			writeLogString(fpnLog, "WARNNING : AveValueTimeInterval_min have to be the multiple value of OutputTimeStep_min.\n", 1, 1);
			int share_print = prj.dtPrintAveValue_min / prj.dtPrint_min;
			int dtPrintAveValue_new = share_print * prj.dtPrint_min;
			writeLogString(fpnLog, "WARNNING : AveValueTimeInterval_min was set to " + to_string(dtPrintAveValue_new) + ".\n", 1, 1);
			prj.dtPrintAveValue_min = dtPrintAveValue_new;
		}
		prj.dtPrintAveValue_sec = prj.dtPrintAveValue_min * 60;
	}

	if (prj.dtsec == 0) {
		if (prj.isFixedTimeStep == 1) {
			writeLogString(fpnLog, "ERROR : In fixed dt, the calculation time step ["
				+ to_string(prj.dtsec) + "] is invalid.\n", 1, 1);
			return -1;
		}
		else if (prj.isFixedTimeStep == -1) {
			writeLogString(fpnLog, "WARNNING : In adaptive dt, the calculation time step ["
				+ to_string(prj.dtsec) + "] is changed to 1 minute as starting value.\n", 1, -1);
		}
	}

	if (prj.mdp == -1) {
		if (prj.cpusi.totalNumOfLP != 0) {
			prj.mdp = prj.cpusi.totalNumOfLP;
		}
		else {
			prj.mdp = 12;  //omp_get_max_threads()를 사용하면 최대 cpu를 적용하므로 grm에서는 과도한 경우가 많다..
			string outString = "The number of CPUs could not be encountered. Max. degree of parallelism was set to 12.\n";
			outString = outString + "Change the value of <MaxDegreeOfParallelism> in the gmp file to change Max. degree of parallelism.\n";
			writeLogString(fpnLog, outString, 1, 1);
		}
	}

	// 기상자료를 읽어서 변수에 할당
	if (prj.simType == simulationType::Normal) {
		if (setRainfallData() == -1) { return -1; }

		// continuous =================
		if (prj.simEvaportranspiration == 1 || prj.simSnowMelt == 1) {
			if (setLAIRatio() == -1) { return -1; }
			if (setSunDurationRatioData() == -1) { return -1; }
			if (ts.wdUsed_tempMax == 1) {
				if (setTemperatureMax() == -1) { return -1; }
			}
			if (ts.wdUsed_tempMin == 1) {
				if (setTemperatureMin() == -1) { return -1; }
			}
			if (ts.wdUsed_DTL == 1) {
				if (setDaytimeLength() == -1) { return -1; }
			}
			if (ts.isUsed_BlaneyCriddleK_File == 1) { 
				if (setBlaneyCriddleK() == -1) { return -1; }
			}
			if (ts.wdUsed_solarR == 1) {
				if (setSolarRadiation() == -1) { return -1; }
			}

			if (ts.wdUsed_dewPointTemp == 1) {
				if (setDewPointTemp() == -1) { return -1; }
			}
			if (ts.wdUsed_windSpeed == 1) {
				if (setWindSpeed() == -1) { return -1; }
			}
			if (ts.wdUsed_userET > 0) {
				if (setUserET(ts.wdUsed_userET) == -1) { return -1; }
			}
			if (ts.wdUsed_snowPackTemp == 1) {
				if (setSnowPackTemp() == -1) { return -1; }
			}
			// continuous =================
		}
	}

	//기타 검증
	if (prj.dtPrint_min == 0) {
		writeLogString(fpnLog, "ERROR : Print out time step ["
			+ to_string(prj.dtPrint_min) + "] is invalid.\n", 1, 1);
		return -1;
	}
	if (prj.simDuration_hr == 0) {
		writeLogString(fpnLog, "ERROR : Simulation duration ["
			+ to_string(prj.simDuration_hr) + "] is invalid.\n", 1, 1);
		return -1;
	}

	if (forceRealTime == 1) {
		prj.simType = simulationType::RealTime;
	}

	if (prj.makeASCFile == 1 || prj.makeIMGFile == 1) {
		prj.makeASCorIMGfile = 1;
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
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(SingleEvent))) { // 과거의 gmp에서 SingleEvent도 Normal로 설정한다.
				prj.simType = simulationType::Normal;
			}
			else if (vStringL == lower(ENUM_TO_STR(Normal))) {
				prj.simType = simulationType::Normal;
			}
			else if (vStringL == lower(ENUM_TO_STR(RealTime))) {
				prj.simType = simulationType::RealTime;
			}
			else {
				writeLogString(fpnLog, "ERROR : Simulation type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Simulation type is invalid.\n", 1, 1);
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
		// wsl에서는 드라이브 문자는 소문자로 표시한다. D: 드라이브는 d로 표시
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnDomain = vString;

			fs::path fpnProjection;
			fpnProjection = fs::path(prj.fpnDomain.c_str()).replace_extension(".prj");
			if (fs::exists(lower(fpnProjection.string())) == true) {
				prj.fpnProjection = fpnProjection.string();
			}
			else {
				prj.fpnProjection = "";
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Domain file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.SlopeFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SlopeFile);
		prj.fpnSlope = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnSlope = vString;
		}
		else {
			writeLogString(fpnLog, "ERROR : Slope file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.FlowDirectionFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowDirectionFile);
		prj.fpnFD = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnFD = vString;
		}
		else {
			writeLogString(fpnLog, "ERROR : Flow direction file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.FlowDirectionType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowDirectionType);
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(StartsFromNE))) {
				prj.fdType = flowDirectionType::StartsFromNE;
			}
			else if (vStringL == lower(ENUM_TO_STR(StartsFromN))) {
				prj.fdType = flowDirectionType::StartsFromN;
			}
			else if (vStringL == lower(ENUM_TO_STR(StartsFromE))) {
				prj.fdType = flowDirectionType::StartsFromE;
			}
			else if (vStringL == lower(ENUM_TO_STR(StartsFromE_TauDEM))) {
				prj.fdType = flowDirectionType::StartsFromE_TauDEM;
			}
			else {
				writeLogString(fpnLog, "ERROR : Flow direction type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Flow direction type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.FlowAccumFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowAccumFile);
		prj.fpnFA = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnFA = vString;
		}
		else {
			writeLogString(fpnLog, "ERROR : Flow accumulation file [" + vString + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	//v2025==================
	if (aline.find(fldName.DEMFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DEMFile);
		prj.fpnDEM = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnDEM = vString;
		}
		else {
			// 모의조건에 따른 검증은 다른 곳에서 기상자료와 함께 하자.
			//writeLogString(fpnLog, "ERROR : DEM file [" + vString + "] is invalid.\n", 1, 1);
			//return -1;
		}
		return 1;
	}
	//v2025==================

	if (aline.find(fldName.StreamFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.StreamFile);
		prj.fpnStream = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnStream = vString;
			prj.streamFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLogString(fpnLog, "WARNNING : Stream file was not set.\n", 1, -1);
			}
			else {
				writeLogString(fpnLog, "WARNNING : Stream file [" + vString + "] is invalid.\n", 1, 1);
			}
			prj.streamFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.ChannelWidthFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthFile);
		prj.fpnChannelWidth = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnChannelWidth = vString;
			prj.cwFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLogString(fpnLog, "Channel width file was not set.\n", 1, -1);
			}
			else {
				writeLogString(fpnLog, "WARNNING : Channel width file [" + vString + "] is invalid.\n", 1, 1);
			}

			prj.cwFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.InitialSoilSaturationRatioFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.InitialSoilSaturationRatioFile);
		prj.fpniniSSR = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpniniSSR = vString;
			prj.issrFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLogString(fpnLog, "Soil saturation ratio file was not set.\n", 1, -1);
			}
			else {
				writeLogString(fpnLog, "WARNNING : Soil saturation ratio file [" + vString + "] is invalid.\n", 1, 1);
			}

			prj.issrFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.InitialChannelFlowFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.InitialChannelFlowFile);
		prj.fpniniChFlow = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpniniChFlow = vString;
			prj.icfFileApplied = 1;
		}
		else {
			if (vString == "") {
				writeLogString(fpnLog, "Initial stream flow file was not set.\n", 1, -1);
			}
			else {
				writeLogString(fpnLog, "WARNNING : Initial stream flow file [" + vString + "] is invalid.\n", 1, 1);
			}
			prj.icfFileApplied = -1;
		}
		return 1;
	}

	if (aline.find(fldName.LandCoverDataType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LandCoverDataType);
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(File))) {
				prj.lcDataType = fileOrConstant::File;
			}
			else if (vStringL == lower(ENUM_TO_STR(Constant))) {
				prj.lcDataType = fileOrConstant::Constant;
			}
			else {
				writeLogString(fpnLog, "ERROR : Land cover data type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Land cover data type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.LandCoverFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LandCoverFile);
		prj.fpnLC = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnLC = vString;
		}
		return 1;
	}

	if (aline.find(fldName.LandCoverVATFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LandCoverVATFile);
		prj.fpnLCVat = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnLCVat = vString;
		}
		return 1;
	}

	if (aline.find(fldName.LAIFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LAIFile);
		prj.fpnLAI = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnLAI = vString;
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
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(File))) {
				prj.stDataType = fileOrConstant::File;
			}
			else if (vStringL == lower(ENUM_TO_STR(Constant))) {
				prj.stDataType = fileOrConstant::Constant;
			}
			else {
				writeLogString(fpnLog, "ERROR : Soil texture data type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Soil texture data type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.SoilTextureFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilTextureFile);
		prj.fpnST = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnST = vString;
		}
		return 1;
	}

	if (aline.find(fldName.SoilTextureVATFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilTextureVATFile);
		prj.fpnSTVat = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
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
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(File))) {
				prj.sdDataType = fileOrConstant::File;
			}
			else if (vStringL == lower(ENUM_TO_STR(Constant))) {
				prj.sdDataType = fileOrConstant::Constant;
			}
			else {
				writeLogString(fpnLog, "ERROR : Soil depth data type is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Soil depth data type is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SoilDepthFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilDepthFile);
		prj.fpnSD = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnSD = vString;
		}
		return 1;
	}
	if (aline.find(fldName.SoilDepthVATFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SoilDepthVATFile);
		prj.fpnSDVat = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
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

	if (aline.find(fldName.PrecipitationDataFile_01) != string::npos
		|| aline.find(fldName.PrecipitationDataFile_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PrecipitationDataFile_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.PrecipitationDataFile_02);
		}
		prj.fpnRainfallData = "";
		prj.rfDataType = weatherDataType::None;
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnRainfallData = vString;
			prj.rfDataType = getWeatherDataTypeByDataFile(vString);
			if (prj.rfDataType == weatherDataType::None) {
				writeLogString(fpnLog, "ERROR : The precipitation data in the file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else if (prj.simType == simulationType::Normal) {
			writeLogString(fpnLog, "ERROR : Precipitation data file [" + vString + "] was not set or invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.PrecipitationInterval_min_01) != string::npos
		|| aline.find(fldName.PrecipitationInterval_min_02) != string::npos
		|| aline.find(fldName.PrecipitationInterval_min_03) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PrecipitationInterval_min_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.PrecipitationInterval_min_02);
		}
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.PrecipitationInterval_min_03);
		}
		if (vString != "" && stoi_comma(vString)>0) {
			prj.rfinterval_min = stoi_comma(vString);
		}
		else if (prj.simType == simulationType::Normal) {
			writeLogString(fpnLog, "ERROR : Precipitation data time interval is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	// continuous================
	if (aline.find(fldName.TemperatureMaxDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.TemperatureMaxDataFile);
		prj.fpnTempMaxData = "";
		prj.tempMaxDataType = weatherDataType::None;
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnTempMaxData = vString;
			prj.tempMaxDataType = getWeatherDataTypeByDataFile(vString);
			if (prj.tempMaxDataType == weatherDataType::None) {
				writeLogString(fpnLog, "ERROR : Max temperature data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else if (prj.simType == simulationType::Normal) { 
			if (vString != "") {//옵션이므로 이 경우에만 애러처리 한다.
				writeLogString(fpnLog, "ERROR : Max temperature data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}			
		}
		return 1;
	}
	if (aline.find(fldName.TemperatureMaxInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.TemperatureMaxInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			if (t_min != 1440) {
				string err = "ERROR : Max temperature data time interval is invalid.\n";
				err = err + "           Max temperature data time interval have to be 1440 minutes.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.tempMaxInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal ) {//옵션이므로 여기서 애러처리 안한다.
			prj.tempMaxInterval_min = 0;
		}
		return 1;
	}

	if (aline.find(fldName.TemperatureMinDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.TemperatureMinDataFile);
		prj.fpnTempMinData = "";
		prj.tempMinDataType = weatherDataType::None;
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnTempMinData = vString;
			prj.tempMinDataType = getWeatherDataTypeByDataFile(vString);
			if (prj.tempMinDataType == weatherDataType::None) {
				writeLogString(fpnLog, "ERROR : Min temperature data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else if (prj.simType == simulationType::Normal) {
			if (vString != "") {//옵션이므로 이 경우에만 애러처리 한다.
				writeLogString(fpnLog, "ERROR : Min temperature data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		return 1;
	}
	if (aline.find(fldName.TemperatureMinInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.TemperatureMinInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			if (t_min != 1440) {
				string err = "ERROR : Min temperature data time interval is invalid.\n";
				err = err + "           Min temperature data time interval have to be 1440 minutes.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.tempMinInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal) {//옵션이므로 여기서 애러처리 안한다.
			//writeLog(fpnLog, "WARNNING : Min temperature data time interval was not set.\n", 1, 1);
			prj.tempMinInterval_min = 0;
		}
		return 1;
	}

	if (aline.find(fldName.DaytimeLengthDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DaytimeLengthDataFile);
		prj.fpnDTLData = "";
		prj.DTLDataType = weatherDataType::None;
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnDTLData = vString;
			prj.DTLDataType = getWeatherDataTypeByDataFile(vString);
			if (prj.DTLDataType == weatherDataType::None) {
				writeLogString(fpnLog, "ERROR : Daytime length data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else if (prj.simType == simulationType::Normal) {
			if (vString != "") {//옵션이므로 이 경우에만 애러처리 한다.
				writeLogString(fpnLog, "ERROR : Daytime length data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		return 1;
	}
	if (aline.find(fldName.DaytimeLengthInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DaytimeLengthInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			if (t_min != 1440) {
				string err = "ERROR : Daytime length data time interval is invalid.\n";
				err = err + "           Daytime length data time interval have to be 1440 minutes.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.DTLDataInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal) {//옵션이므로 여기서 애러처리 안한다.
			//writeLog(fpnLog, "WARNNING : Daytime length data time interval was not set.\n", 1, 1);
			prj.DTLDataInterval_min = 0;
		}
		return 1;
	}

	if (aline.find(fldName.BlaneyCriddleCoefDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.BlaneyCriddleCoefDataFile);
		prj.fpnBlaneyCriddleK = "";
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnBlaneyCriddleK = vString;
		}
		else if (prj.simType == simulationType::Normal) {
			if (vString != "") {//옵션이므로 이 경우에만 애러처리 한다.
				writeLogString(fpnLog, "ERROR : The file of K ceofficient data in Blaney-Criddle method [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		return 1;
	}

	if (aline.find(fldName.SolarRadiationDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SolarRadiationDataFile);
		prj.fpnSolarRadData = "";
		prj.solarRadDataType = weatherDataType::None;
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnSolarRadData = vString;
			prj.solarRadDataType = getWeatherDataTypeByDataFile(vString);
			if (prj.solarRadDataType == weatherDataType::None) {
				writeLogString(fpnLog, "ERROR : Solar radiation data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else if (prj.simType == simulationType::Normal) {
			if (vString != "") {//옵션이므로 이 경우에만 애러처리 한다.
				writeLogString(fpnLog, "ERROR : Solar radiation data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		return 1;
	}
	if (aline.find(fldName.SolarRadiationInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SolarRadiationInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			if (t_min != 1440) {
				string err = "ERROR : Solar radiation data time interval is invalid.\n";
				err = err + "            Solar radiation data time interval have to be 1440 minutes.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.solarRadInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal) {//옵션이므로 여기서 애러처리 안한다.
			//writeLog(fpnLog, "WARNNING : Solar radiation data time interval was not set.\n", 1, 1);
			prj.solarRadInterval_min = 0;
		}
		return 1;
	}

	//v2025==================
	if (aline.find(fldName.DewPointTemperatureDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DewPointTemperatureDataFile);
		prj.fpnDewPointTemp = "";
		prj.dewPointTempDataType = weatherDataType::None;
		if (vString != "") {
			if (fs::exists(lower(vString)) == true) {
				prj.fpnDewPointTemp = vString;
				prj.dewPointTempDataType = getWeatherDataTypeByDataFile(vString);
				if (prj.dewPointTempDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Dew point temperature data file [" + vString + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else if (prj.simType == simulationType::Normal) {
				writeLogString(fpnLog, "ERROR : Dew point temperature data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
	}
	if (aline.find(fldName.DewPointTemperatureInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DewPointTemperatureInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			if (t_min != 1440) {
				string err = "ERROR : Dew point temperature data time interval is invalid.\n";
				err = err + "            Dew point temperature data time interval have to be 1440 minutes.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.dewPointTempInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal) {
			prj.dewPointTempInterval_min = 0;
		}
		return 1;
	}

	if (aline.find(fldName.WindSpeedDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.WindSpeedDataFile);
		prj.fpnWindSpeed = "";
		prj.windSpeedDataType = weatherDataType::None;
		if (vString != "") {
			if (fs::exists(lower(vString)) == true) {
				prj.fpnWindSpeed = vString;
				prj.windSpeedDataType = getWeatherDataTypeByDataFile(vString);
				if (prj.windSpeedDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : Wind speed data file [" + vString + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else if (prj.simType == simulationType::Normal) {
				writeLogString(fpnLog, "ERROR : Wind speed data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
	}
	if (aline.find(fldName.WindSpeedInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.WindSpeedInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			if (t_min != 1440) {
				string err = "ERROR : Wind speed data time interval is invalid.\n";
				err = err + "            Wind speed data time interval have to be 1440 minutes.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.windSpeedInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal) {
			prj.windSpeedInterval_min = 0;
		}
		return 1;
	}

	if (aline.find(fldName.UserPETDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UserPETDataFile);
		prj.fpnUserPET = "";
		prj.userPETDataType = weatherDataType::None;
		if (vString != "") {
			if (fs::exists(lower(vString)) == true) {
				prj.fpnUserPET = vString;
				prj.userPETDataType = getWeatherDataTypeByDataFile(vString);
				if (prj.userPETDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : User potential evapotranspiration data file [" + vString + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else if (prj.simType == simulationType::Normal) {
				writeLogString(fpnLog, "ERROR : User potential evapotranspiration data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
	}
	if (aline.find(fldName.UserAETDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UserAETDataFile);
		prj.fpnUserAET = "";
		prj.userAETDataType = weatherDataType::None;
		if (vString != "") {
			if (fs::exists(lower(vString)) == true) {
				prj.fpnUserAET = vString;
				prj.userAETDataType = getWeatherDataTypeByDataFile(vString);
				if (prj.userAETDataType == weatherDataType::None) {
					writeLogString(fpnLog, "ERROR : User actual evapotranspiration data file [" + vString + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else if (prj.simType == simulationType::Normal) {
				writeLogString(fpnLog, "ERROR : User actual evapotranspiration data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
	}
	if (aline.find(fldName.UserETDataInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UserETDataInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			// 사용자 지정 PET는 1일 단위가 아닐 수도 있다. 
			if (t_min <= 0) { 
				string err = "ERROR : User potential evapotranspiration data time interval is invalid.\n";
				err = err + "            User potential evapotranspiration data time interval have to be positive integer number.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.userETInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal) {
			prj.userETInterval_min = 0;
		}
		return 1;
	}
	//v2025==================



	if (aline.find(fldName.SnowPackTemperatureDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowPackTemperatureDataFile);
		prj.fpnSnowpackTempData = "";
		prj.snowpackTempDataType = weatherDataType::None;
		if (vString != "" && fs::exists(lower(vString)) == true) {
			prj.fpnSnowpackTempData = vString;
			prj.snowpackTempDataType = getWeatherDataTypeByDataFile(vString);
			if (prj.snowpackTempDataType == weatherDataType::None) {
				writeLogString(fpnLog, "ERROR : Snowpack temperature data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else if (prj.simType == simulationType::Normal) {
			if (vString != "") {//옵션이므로 이 경우에만 애러처리 한다.
				writeLogString(fpnLog, "ERROR : Snowpack temperature data file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		return 1;
	}
	if (aline.find(fldName.SnowPackTemperatureInInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowPackTemperatureInInterval_min);
		if (vString != "") {
			int t_min = stoi_comma(vString);
			if (t_min != 1440) {
				string err = "ERROR : Snowpack temperature data time interval is invalid.\n";
				err = err + "           Snowpack temperature data time interval have to be 1440 minutes.\n";
				writeLogString(fpnLog, err, 1, 1);
				return -1;
			}
			prj.snowpackTempInterval_min = t_min;
		}
		else if (prj.simType == simulationType::Normal) {//옵션이므로 여기서 애러처리 안한다.
			//writeLog(fpnLog, "WARNNING : Snowpack temperature data time interval was not set.\n", 1, 1);
			prj.snowpackTempInterval_min = 0;
		}
		return 1;
	}

	if (aline.find(fldName.LatitudeTOP_degree) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LatitudeTOP_degree);
		prj.latitudeTOP_degree = -9999.0;
		if (vString != "" && isNumericDbl(vString) == true) {
			prj.latitudeTOP_degree = stod_comma(vString);			
		}
		else {// 증발산 옵션이므로, 애러처리 안한다.
			//writeLog(fpnLog, "WARNNING : ET coefficient of the watershed ["
			//	+ to_string(ssp->wsid) + "] is invalid.\n", 1, -1);
		}
		return 1;
	}
	//===continuous =================================

	if (aline.find(fldName.MaxDegreeOfParallelism) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MaxDegreeOfParallelism);
		if (vString != "" && stoi_comma(vString) != 0 && stoi_comma(vString) >= -1) {
			prj.mdp = stoi_comma(vString);
		}
		else {
			writeLogString(fpnLog, "Max. degree of parallelism was not set. Maximum value [-1] was assigned.\n", 1, 1);
			prj.mdp = -1;
		}
		return 1;
	}

	if (aline.find(fldName.SimulationStartingTime_01) != string::npos
		|| aline.find(fldName.SimulationStartingTime_02) != string::npos) {		
		vString = getValueStringFromXmlLine(aline, fldName.SimulationStartingTime_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.SimulationStartingTime_02);
		}
		if (vString != "") {
			prj.simStartTime = vString;
			if (isNumeric(vString) == true) {
				prj.isDateTimeFormat = -1;
			}
			else {
				prj.isDateTimeFormat = 1;
				tm t;
				t= stringToDateTime2(vString, true); 
				if (t.tm_year == 0 || t.tm_mon == 0
					|| t.tm_mday == 0) {
					writeLogString(fpnLog, "ERROR : Simulation starting time is invalid. Check the input format. \n", 1, 1);
					return -1;
				}
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
		if (vString != "" && stod_comma(vString) >= 0) {
			prj.simDuration_hr = stod_comma(vString);
		}
		else {
			writeLogString(fpnLog, "ERROR : Simulation duration is invalid.\n", 1, 1);
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
		if (vString != "" && stod_comma(vString) > 0) {
			prj.dtsec = stoi_comma(vString) * 60;
		}
		return 1;
	}

	if (aline.find(fldName.IsFixedTimeStep) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.IsFixedTimeStep);
		prj.isFixedTimeStep = -1;
		if (lower(vString) == "true") {
			prj.isFixedTimeStep = 1;
		}
		else if (lower(vString) == "false") {
			prj.isFixedTimeStep = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : Computational time step type is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
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
			prj.dtPrint_min = stoi_comma(vString);
		}
		else {
			writeLogString(fpnLog, "ERROR : Print out time step is invalid.\n", 1, 1);
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
		else if (lower(vString) == "false") {
			prj.simInfiltration = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : SimulateInfiltration option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateSubsurfaceFlow) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateSubsurfaceFlow);
		prj.simSubsurfaceFlow = -1;
		if (lower(vString) == "true") {
			prj.simSubsurfaceFlow = 1;
		}
		else if (lower(vString) == "false") {
			prj.simSubsurfaceFlow = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : SimulateSubsurfaceFlow option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateBaseFlow) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateBaseFlow);
		prj.simBaseFlow = -1;
		if (lower(vString) == "true") {
			prj.simBaseFlow = 1;
		}
		else if (lower(vString) == "false") {
			prj.simBaseFlow = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : SimulateBaseFlow option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateEvapotranspiration) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateEvapotranspiration);
		prj.simEvaportranspiration = -1;
		if (lower(vString) == "true") {
			prj.simEvaportranspiration = 1;
		}
		else if (lower(vString) == "false") {
			prj.simEvaportranspiration = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : SimulateEvaportranspiration option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateSnowMelt) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateSnowMelt);
		prj.simSnowMelt = -1;
		if (lower(vString) == "true") {
			prj.simSnowMelt = 1;
		}
		else if (lower(vString) == "false") {
			prj.simSnowMelt = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : SimulateSnowMelt option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateInterception) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateInterception);
		prj.simInterception = -1;
		if (lower(vString) == "true") {
			prj.simInterception = 1;
		}
		else if (lower(vString) == "false") {
			prj.simInterception = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : SimulateInterception option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.SimulateFlowControl) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SimulateFlowControl);
		prj.simFlowControl = -1;
		if (lower(vString) == "true") {
			prj.simFlowControl = 1;
		}
		else if (lower(vString) == "false") {
			prj.simFlowControl = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : SimulateFlowControl option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	// prj.printOption에 상관없이 IMG 파일 출력 가능
	if (aline.find(fldName.MakeIMGFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeIMGFile);
		prj.makeIMGFile = -1;
		if (lower(vString) == "true") {
			prj.makeIMGFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makeIMGFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakeIMGFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	// prj.printOption에 상관없이 ASCII 파일 출력 가능
	if (aline.find(fldName.MakeASCFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeASCFile);
		prj.makeASCFile = -1;
		if (lower(vString) == "true") {
			prj.makeASCFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makeASCFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakeASCFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeSoilSaturationDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeSoilSaturationDistFile);
		prj.makeSoilSaturationDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeSoilSaturationDistFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makeSoilSaturationDistFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakeSoilSaturationDistFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeRfDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeRfDistFile);
		prj.makeRfDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeRfDistFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makeRfDistFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakeRfDistFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeRFaccDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeRFaccDistFile);
		prj.makeRFaccDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeRFaccDistFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makeRFaccDistFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakeRFaccDistFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeFlowDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeFlowDistFile);
		prj.makeFlowDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeFlowDistFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makeFlowDistFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakeFlowDistFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MakePETDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakePETDistFile);
		prj.makePETDistFile = -1;
		if (lower(vString) == "true") {
			prj.makePETDistFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makePETDistFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakePETDistFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MakeAETDistFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MakeAETDistFile);
		prj.makeAETDistFile = -1;
		if (lower(vString) == "true") {
			prj.makeAETDistFile = 1;
		}
		else if (lower(vString) == "false") {
			prj.makeAETDistFile = -1;
		}
		else {
			writeLogString(fpnLog, "ERROR : MakeAETDistFile option is invalid. Set 'True' or 'False'.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.PrintOption) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PrintOption);
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == "all") {
				prj.printOption = GRMPrintType::All;
			}
			else if (vStringL == "dischargefile") {
				prj.printOption = GRMPrintType::DischargeFile;
			}
			else if (vStringL == "dischargeandfcfile") {
				prj.printOption = GRMPrintType::DischargeAndFcFile;
			}
			else if (vStringL == "averagefile") {
				prj.printOption = GRMPrintType::AverageFile;
			}
			else if (vStringL == "dischargefileq") {
				prj.printOption = GRMPrintType::DischargeFileQ;
			}
			else if (vStringL == "averagefileq") {
				prj.printOption = GRMPrintType::AverageFileQ;
			}
			else if (vStringL == "allq") {
				prj.printOption = GRMPrintType::AllQ;
			}
			else {
				writeLogString(fpnLog, "ERROR : Print option is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Print option is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.PrintAveValue) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PrintAveValue);
		prj.printAveValue = -1;
		if (lower(vString) == "true") {
			prj.printAveValue = 1;
		}
		return 1;
	}

	if (aline.find(fldName.AveValueTimeInterval_min) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.AveValueTimeInterval_min);
		prj.dtPrintAveValue_min = 0;
		if (vString != "") {
			prj.dtPrintAveValue_min = stoi_comma(vString);
		}
		return 1;
	}

	if (aline.find(fldName.ValueSeparator) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ValueSeparator);
		prj.vSeparator = "\t";
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == "space") {
				prj.vSeparator = " ";
			}
			else if (vStringL == "comma") {
				prj.vSeparator = ",";
			}
			else if (vStringL == "tab") {
				prj.vSeparator = "\t";
			}
			else {
				prj.vSeparator = "\t";
			}
		}
		return 1;
	}

	if (aline.find(fldName.WriteLog) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.WriteLog);
		prj.writeLog = -1;
		if (lower(vString) == "true") {
			prj.writeLog = 1;
		}
		return 1;
	}
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
			writeLogString(fpnLog, "ERROR : Land cover raster value in gmp file is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.GRMCode_LC) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GRMCode_LC);
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(WATR))) {
				lc->lcCode = landCoverCode::WATR;
			}
			else if (vStringL == lower(ENUM_TO_STR(URBN))) {
				lc->lcCode = landCoverCode::URBN;
			}
			else if (vStringL == lower(ENUM_TO_STR(BARE))) {
				lc->lcCode = landCoverCode::BARE;
			}
			else if (vStringL == lower(ENUM_TO_STR(WTLD))) {
				lc->lcCode = landCoverCode::WTLD;
			}
			else if (vStringL == lower(ENUM_TO_STR(GRSS))) {
				lc->lcCode = landCoverCode::GRSS;
			}
			else if (vStringL == lower(ENUM_TO_STR(FRST))) {
				lc->lcCode = landCoverCode::FRST;
			}
			else if (vStringL == lower(ENUM_TO_STR(AGRL))) {
				lc->lcCode = landCoverCode::AGRL;
			}
			else if (vStringL == lower(ENUM_TO_STR(USER))) {
				lc->lcCode = landCoverCode::USER;
			}
			else if (vStringL == lower(ENUM_TO_STR(CONSTV))) {
				lc->lcCode = landCoverCode::CONSTV;
			}
			else {
				writeLogString(fpnLog, "ERROR : Land cover code name of the raster value ["
					+ to_string(lc->lcGridValue) + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		else {
			writeLogString(fpnLog, "ERROR : Land cover code name of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Land cover roughness coefficient of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Land cover impervious ratio of the raster value ["
				+ to_string(lc->lcGridValue) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.CanopyRatio) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CanopyRatio);
		lc->CanopyRatio = -1.0;
		if (vString != "") {// 연속형에서 조건부 입력이므로 여기서 애러검토 안한다.
			lc->CanopyRatio = stod(vString);
		}
		return 1;
	}

	if (aline.find(fldName.InterceptionMaxWaterCanopy_mm) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.InterceptionMaxWaterCanopy_mm);
		lc->InterceptionMaxWaterCanopy_mm = -1.0;
		if (vString != "" && stod(vString) >= 0) {// 연속형에서 조건부 입력이므로 여기서 애러검토 안한다.
			lc->InterceptionMaxWaterCanopy_mm = stod(vString);
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
			writeLogString(fpnLog, "ERROR : Soil depth raster value in gmp file is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.GRMCode_SD) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GRMCode_SD);
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(D))) {
				sd->sdCode = soilDepthCode::D;
			}
			else if (vStringL == lower(ENUM_TO_STR(MDMS))) {
				sd->sdCode = soilDepthCode::M;
			}
			else if (vStringL == lower(ENUM_TO_STR(M))) {
				sd->sdCode = soilDepthCode::M;
			}
			else if (vStringL == lower(ENUM_TO_STR(S))) {
				sd->sdCode = soilDepthCode::S;
			}
			else if (vStringL == lower(ENUM_TO_STR(VD))) {
				sd->sdCode = soilDepthCode::VD;
			}
			else if (vStringL == lower(ENUM_TO_STR(VS))) {
				sd->sdCode = soilDepthCode::VS;
			}
			else if (vStringL == lower(ENUM_TO_STR(USER))) {
				sd->sdCode = soilDepthCode::USER;
			}
			else if (vStringL == lower(ENUM_TO_STR(CONSTV))) {
				sd->sdCode = soilDepthCode::CONSTV;
			}
			else {
				writeLogString(fpnLog, "ERROR : Soil depth code name of the raster value ["
					+ to_string(sd->sdGridValue) + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		else {
			writeLogString(fpnLog, "ERROR : Soil depth code name of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Soil depth value of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Soil texture raster value in gmp file is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.GRMCode_ST) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.GRMCode_ST);
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(C))) {
				st->stCode = soilTextureCode::C;
			}
			else if (vStringL == lower(ENUM_TO_STR(CL))) {
				st->stCode = soilTextureCode::CL;
			}
			else if (vStringL == lower(ENUM_TO_STR(L))) {
				st->stCode = soilTextureCode::L;
			}
			else if (vStringL == lower(ENUM_TO_STR(LS))) {
				st->stCode = soilTextureCode::LS;
			}
			else if (vStringL == lower(ENUM_TO_STR(S))) {
				st->stCode = soilTextureCode::S;
			}
			else if (vStringL == lower(ENUM_TO_STR(SC))) {
				st->stCode = soilTextureCode::SC;
			}
			else if (vStringL == lower(ENUM_TO_STR(SCL))) {
				st->stCode = soilTextureCode::SCL;
			}
			else if (vStringL == lower(ENUM_TO_STR(SiC))) {
				st->stCode = soilTextureCode::SiC;
			}
			else if (vStringL == lower(ENUM_TO_STR(SiCL))) {
				st->stCode = soilTextureCode::SiCL;
			}
			else if (vStringL == lower(ENUM_TO_STR(SiL))) {
				st->stCode = soilTextureCode::SiL;
			}
			else if (vStringL == lower(ENUM_TO_STR(SL))) {
				st->stCode = soilTextureCode::SL;
			}
			else if (vStringL == lower(ENUM_TO_STR(USER))) {
				st->stCode = soilTextureCode::USER;
			}
			else if (vStringL == lower(ENUM_TO_STR(CONSTV))) {
				st->stCode = soilTextureCode::CONSTV;
			}
			else {
				writeLogString(fpnLog, "ERROR : Soil texture code name of the raster value ["
					+ to_string(st->stGridValue) + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		else {
			writeLogString(fpnLog, "ERROR : Soil texture code name of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Soil porosity of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Soil effective porosity of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Soil wetting front suction head of the raster value ["
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
			writeLogString(fpnLog, "ERROR : Soil hydraulic conductivity of the raster value ["
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
			writeLogString(fpnLog, "ERROR : ColX value of [" + fci->fcName + "] is invalid.\n", 1, 1);
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
			writeLogString(fpnLog, "ERROR : RowY  value of [" + fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ControlType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ControlType);
		flowControlType  afct = flowControlType::None;
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == "inlet") {
				afct = flowControlType::Inlet;
			}
			else if (vStringL == lower(ENUM_TO_STR(ReservoirOperation))) {
				afct = flowControlType::ReservoirOperation;
			}
			else if (vStringL == lower(ENUM_TO_STR(ReservoirOutflow))) {
				afct = flowControlType::ReservoirOutflow;
			}
			else if (vStringL == lower(ENUM_TO_STR(SinkFlow))) {
				afct = flowControlType::SinkFlow;
			}
			else if (vStringL == lower(ENUM_TO_STR(SourceFlow))) {
				afct = flowControlType::SourceFlow;
			}
			else if (vStringL == lower(ENUM_TO_STR(DetentionPond))) {
				afct = flowControlType::DetentionPond;
			}
			else {
				writeLogString(fpnLog, "ERROR : Flow control type of ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Flow control type of ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		fci->fcType = afct;
		return 1;
	}
	if (aline.find(fldName.FCDT_min_02) != string::npos
		|| aline.find(fldName.FCDT_min_01) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FCDT_min_02);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.FCDT_min_01);
		}
		if (vString != "" && stoi(vString) >= 0) {
			fci->fcDT_min = stoi(vString);
		}
		else if (fci->fcType == flowControlType::Inlet 
			|| fci->fcType == flowControlType::ReservoirOutflow)
		{
			writeLogString(fpnLog, "ERROR : Flow control data time interval of ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.FlowDataFile) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.FlowDataFile);
		if (vString != "" && fs::exists(lower(vString)) == true) {
			fci->fpnFCData = vString;
		}
		else if (vString != ""){
			writeLogString(fpnLog, "ERROR : Flow control data file of ["
				+ fci->fcName + "]("+ vString+") is invalid.\n", 1, 1);
		}
	}

	if (fci->fcType != flowControlType::Inlet) {
		if (aline.find(fldName.IniStorage) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.IniStorage);
			if (vString != "" && stod_comma(vString) >= 0) {
				fci->iniStorage_m3 = stod_comma(vString);
			}
			else if(fci->fcType == flowControlType::ReservoirOperation){
				writeLogString(fpnLog, "ERROR : Ini. storage of reservoir ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		if (aline.find("<"+fldName.MaxStorage+">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.MaxStorage);
			if (vString != "" && stod_comma(vString) >= 0) {// max storage must be greater than zero.
				if (fci->fcType == flowControlType::ReservoirOperation
					&& stod_comma(vString) == 0) {
					writeLogString(fpnLog, "ERROR : Max. storage of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				fci->maxStorage_m3 = stod_comma(vString);
			}
			else if(fci->fcType == flowControlType::ReservoirOperation){
				writeLogString(fpnLog, "ERROR : Max. storage of reservoir ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		if (aline.find("<" + fldName.NormalHighStorage + ">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.NormalHighStorage);
			if (vString != "" && stod_comma(vString) >= 0) {// max storage must be greater than zero.
				if (fci->fcType == flowControlType::ReservoirOperation
					&& stod_comma(vString) == 0) {
					writeLogString(fpnLog, "ERROR : NormalHighStorage of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				fci->NormalHighStorage_m3 = stod_comma(vString);
			}
			else if (fci->fcType == flowControlType::ReservoirOperation) {
				writeLogString(fpnLog, "ERROR : NormalHighStorage of reservoir ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		if (aline.find("<" + fldName.RestrictedStorage + ">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.RestrictedStorage);
			if (vString != "" && stod_comma(vString) >= 0) {// max storage must be greater than zero.
				if (fci->fcType == flowControlType::ReservoirOperation
					&& stod_comma(vString) == 0) {
					writeLogString(fpnLog, "ERROR : RestrictedStorage of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				fci->RestrictedStorage_m3 = stod_comma(vString);
			}
			else if (fci->fcType == flowControlType::ReservoirOperation) {
				writeLogString(fpnLog, "ERROR : RestrictedStorage of reservoir ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			return 1;
		}
		if (aline.find("<" + fldName.RestrictedPeriod_Start + ">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.RestrictedPeriod_Start);
			if (vString != "") {
				fci->RestrictedPeriod_Start= vString;
			}
			return 1;
		}
		if (aline.find("<" + fldName.RestrictedPeriod_End + ">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.RestrictedPeriod_End);
			if (vString != "") {
				fci->RestrictedPeriod_End = vString;
			}
			return 1;
		}
		if (aline.find("<" + fldName.AutoROM_MaxOutFlow_CMS + ">") != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.AutoROM_MaxOutFlow_CMS);
			if (vString != "") {
				fci->autoROMmaxOutflow_cms = stod_comma(vString);
			}
			return 1;
		}

		if (aline.find(fldName.ROType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fldName.ROType);
			reservoirOperationType  arot = reservoirOperationType::None;
			if (vString != "") {
				string vStringL = lower(vString);
				if (vStringL == lower(ENUM_TO_STR(AutoROM))) {
					arot = reservoirOperationType::AutoROM;
				}
				else if (vStringL == lower(ENUM_TO_STR(ConstantQ))) {
					arot = reservoirOperationType::ConstantQ;
				}
				else if (vStringL == lower(ENUM_TO_STR(RigidROM))) {
					arot = reservoirOperationType::RigidROM;
				}
				else if (vStringL == lower(ENUM_TO_STR(SDEqation))) {
					arot = reservoirOperationType::SDEqation;
				}
				else {
					writeLogString(fpnLog, "ERROR : Reservoir operation type of ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else if(fci->fcType == flowControlType::ReservoirOperation){
				writeLogString(fpnLog, "ERROR : Reservoir operation type of ["
					+ fci->fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			fci->roType = arot;
			return 1;
		}
		if (fci->roType == reservoirOperationType::RigidROM) {
			if (aline.find(fldName.ROConstRatio) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fldName.ROConstRatio);
				if (vString != "" && stod_comma(vString) >= 0 && stod_comma(vString) <= 1.0) {
					fci->roConstR = stod_comma(vString);
				}
				else {
					writeLogString(fpnLog, "ERROR : Constant outlfow ratio of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				return 1;
			}
			if (aline.find(fldName.ROConstDischarge) != string::npos
				&& aline.find(fldName.ROConstDischargeDuration_hr) == string::npos) {
				vString = getValueStringFromXmlLine(aline, fldName.ROConstDischarge);
				if (vString != "" && stod_comma(vString) >= 0) {
					fci->roConstQ_cms = stod_comma(vString);
				}
				else {
					writeLogString(fpnLog, "ERROR : Constant outlfow of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				return 1;
			}
		}
		if (fci->roType == reservoirOperationType::ConstantQ) {
			if (aline.find(fldName.ROConstDischarge) != string::npos
				&& aline.find(fldName.ROConstDischargeDuration_hr) == string::npos) {
				vString = getValueStringFromXmlLine(aline, fldName.ROConstDischarge);
				if (vString != "" && stod_comma(vString) >= 0) {
					fci->roConstQ_cms = stod_comma(vString);
				}
				else {
					writeLogString(fpnLog, "ERROR : Constant outlfow of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				return 1;
			}
			if (aline.find(fldName.ROConstDischargeDuration_hr) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fldName.ROConstDischargeDuration_hr);
				if (vString != "" && stod_comma(vString) >= 0) {
					fci->roConstQDuration_hr = stod_comma(vString);
				}
				else {
					writeLogString(fpnLog, "ERROR : Constant outlfow duration of reservoir ["
						+ fci->fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				return 1;
			}
		}
	}

	//저류지 관련
	if (aline.find("<" + fldName.DP_QT_StoD_CMS + ">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DP_QT_StoD_CMS);
		if (vString != "" && stod_comma(vString) >= 0) {
			fci->dp_QT_StoD_CMS = stod_comma(vString);
		}
		else if (fci->fcType == flowControlType::DetentionPond) {
			writeLogString(fpnLog, "ERROR : The value of "+ fldName.DP_QT_StoD_CMS+" in ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find("<" + fldName.DP_Qi_max_CMS + ">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DP_Qi_max_CMS);
		if (vString != "" && stod_comma(vString) >= 0) {
			fci->dp_Qi_max_CMS = stod_comma(vString);
		}
		else if (fci->fcType == flowControlType::DetentionPond) {
			writeLogString(fpnLog, "ERROR : The value of " + fldName.DP_Qi_max_CMS + " in ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find("<" + fldName.DP_Qo_max_CMS + ">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DP_Qo_max_CMS);
		if (vString != "" && stod_comma(vString) >= 0) {
			fci->dp_Qo_max_CMS = stod_comma(vString);
		}
		else if (fci->fcType == flowControlType::DetentionPond) {
			writeLogString(fpnLog, "ERROR : The value of " + fldName.DP_Qo_max_CMS + " in ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find("<" + fldName.DP_Wdi_m + ">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DP_Wdi_m);
		if (vString != "" && stod_comma(vString) >= 0) {
			fci->dp_Wdi_m = stod_comma(vString);
		}
		else if (fci->fcType == flowControlType::DetentionPond) {
			writeLogString(fpnLog, "ERROR : The value of " + fldName.DP_Wdi_m + " in ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find("<" + fldName.DP_Ws_m + ">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DP_Ws_m);
		if (vString != "" && stod_comma(vString) >= 0) {
			fci->dp_Ws_m = stod_comma(vString);
		}
		else if (fci->fcType == flowControlType::DetentionPond) {
			writeLogString(fpnLog, "ERROR : The value of " + fldName.DP_Ws_m + " in ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find("<" + fldName.DP_Cr_StoD + ">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.DP_Cr_StoD);
		if (vString != "" && stod_comma(vString) >= 0) {
			fci->dp_Cr_StoD = stod_comma(vString);
		}
		else if (fci->fcType == flowControlType::DetentionPond) {
			writeLogString(fpnLog, "ERROR : The value of " + fldName.DP_Cr_StoD + " in ["
				+ fci->fcName + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	return 1;
}

int readXmlRowChannelSettings(string aline, channelSettingInfo *csi)
{
	string vString = "";
	projectFileFieldName fldName;
	if (aline.find("<"+fldName.WSID_CH+">") != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.WSID_CH);
		if (vString != "" && stoi_comma(vString) > 0) {
			csi->mdWsid = stoi_comma(vString);
		}
		else {
			writeLogString(fpnLog, "ERROR : Most downstream watershed ID for channel setting is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CrossSectionType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CrossSectionType);
		crossSectionType acst = crossSectionType::None;
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(CSCompound))) {
				acst = crossSectionType::CSCompound;
			}
			else if (vStringL == lower(ENUM_TO_STR(CSSingle))) {
				acst = crossSectionType::CSSingle;
			}
			else {
				writeLogString(fpnLog, "ERROR : Channel cross section type in the watershed ["
					+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Channel cross section type in the watershed ["
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
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(CWEquation))) {
				acwt = channelWidthType::CWEquation;
			}
			else if (vStringL == lower(ENUM_TO_STR(CWGeneration))) {
				acwt = channelWidthType::CWGeneration;
			}
			else {
				writeLogString(fpnLog, "ERROR : Channel width type in the watershed ["
					+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Channel width type in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		csi->csWidthType = acwt;
		return 1;
	}
	if (aline.find(fldName.ChannelWidthEQc) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthEQc);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->cwEQc = stod_comma(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWEquation) {
			writeLogString(fpnLog, "ERROR : EQc parameter for channel width eq. in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ChannelWidthEQd) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthEQd);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->cwEQd = stod_comma(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWEquation) {
			writeLogString(fpnLog, "ERROR : EQd parameter for channel width eq. in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ChannelWidthEQe) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthEQe);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->cwEQe = stod_comma(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWEquation) {
			writeLogString(fpnLog, "ERROR : EQe parameter for channel width eq. in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.ChannelWidthMostDownStream_m_01) != string::npos
		|| aline.find(fldName.ChannelWidthMostDownStream_m_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthMostDownStream_m_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.ChannelWidthMostDownStream_m_02);
		}
		if (vString != "" && stod_comma(vString) > 0) {
			csi->cwMostDownStream = stod_comma(vString);
		}
		else if (csi->csWidthType == channelWidthType::CWGeneration) {
			writeLogString(fpnLog, "ERROR : The channel width at the most down stream in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.LowerRegionHeight) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LowerRegionHeight);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->lowRHeight = stod_comma(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLogString(fpnLog, "ERROR : Lower region height parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.LowerRegionBaseWidth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.LowerRegionBaseWidth);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->lowRBaseWidth = stod_comma(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLogString(fpnLog, "ERROR : Lower region base width parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.UpperRegionBaseWidth) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UpperRegionBaseWidth);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->highRBaseWidth = stod_comma(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLogString(fpnLog, "ERROR : Upper region base width parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.CompoundCSChannelWidthLimit) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.CompoundCSChannelWidthLimit);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->compoundCSChannelWidthLimit = stod_comma(vString);
		}
		else if (csi->csType == crossSectionType::CSCompound) {
			writeLogString(fpnLog, "ERROR : Compound cross section width limit parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.BankSideSlopeRight) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.BankSideSlopeRight);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->bankSlopeRight = stod_comma(vString);
		}
		else {
			writeLogString(fpnLog, "ERROR : Right bank side slope parameter in the watershed ["
				+ to_string(csi->mdWsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.BankSideSlopeLeft) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.BankSideSlopeLeft);
		if (vString != "" && stod_comma(vString) > 0) {
			csi->bankSlopeLeft = stod_comma(vString);
		}
		else {
			writeLogString(fpnLog, "ERROR : Left bank side slope parameter in the watershed ["
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
			writeLogString(fpnLog, "ERROR : ID in subWatershed data is invalid.\n", 1, 1);
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
			writeLogString(fpnLog, "ERROR : Ini. saturation ratio in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}

	if (aline.find(fldName.IniLossPRCP_mm) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.IniLossPRCP_mm);
		ssp->iniLossPRCP_mm = 0.0;
		if (vString != "" && stod(vString) >= 0.0) {
			ssp->iniLossPRCP_mm = stod(vString);
		}
		else {
			writeLogString(fpnLog, "ERROR : Ini. loss of precipitation in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Minimum land surface slope for overland flow in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.UnsaturatedKType) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UnsaturatedKType);
		unSaturatedKType uskt = unSaturatedKType::None;
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(Constant))) {
				uskt = unSaturatedKType::Constant;
			}
			else if (vStringL == lower(ENUM_TO_STR(Linear))) {
				uskt = unSaturatedKType::Linear;
			}
			else if (vStringL == lower(ENUM_TO_STR(Exponential))) {
				uskt = unSaturatedKType::Exponential;
			}
			else {
				writeLogString(fpnLog, "ERROR : Unsaturated K type in the watershed ["
					+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Unsaturated K type in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Hydraulic conductivity for unsaturated soil in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Minimum slope of channel bed in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.MinChBaseWidth_m_01) != string::npos
		|| aline.find(fldName.MinChBaseWidth_m_02) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.MinChBaseWidth_m_01);
		if (vString == "") {
			vString = getValueStringFromXmlLine(aline, fldName.MinChBaseWidth_m_02);
		}
		if (vString != "" && stod_comma(vString) > 0) {
			ssp->minChBaseWidth = stod_comma(vString);
		}
		else {
			writeLogString(fpnLog, "ERROR : Minimum value of channel width in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Roughness coefficient of channel in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Dry stream order in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	if (aline.find(fldName.IniFlow) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.IniFlow);
		if (vString != "") {		
			if (stod(vString) >= 0) {
				ssp->iniFlow = stod(vString);
			}
			else {
				writeLogString(fpnLog, "WARNNING : Initial stream flow in the watershed ["
					+ to_string(ssp->wsid) + "] is negative value. 0 is applied. \n", 1, 1);
				ssp->iniFlow = 0.0;
			}
		}
		else {
			writeLogString(fpnLog, "ERROR : Initial stream flow in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Calibration coeff. of roughness in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Calibration coeff. of porosity in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Calibration coeff. of wetting front suction head in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Calibration coeff. of hydraulic conductivity in the watershed ["
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
			writeLogString(fpnLog, "ERROR : Calibration coeff. of soil depth in the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
		}
		return 1;
	}
	// continuous =========================
	if (aline.find(fldName.InterceptionMethod) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.InterceptionMethod);
		InterceptionMethod interceptM = InterceptionMethod::None;
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(LAIRatio))) {
				interceptM = InterceptionMethod::LAIRatio;
			}
			else if (vStringL == lower(ENUM_TO_STR(Constant))) {
				interceptM = InterceptionMethod::Constant;
			}
			else if (vStringL == lower(ENUM_TO_STR(UserData))) {
				interceptM = InterceptionMethod::UserData;
			}
			else { // 차단은 옵션이므로, 애러처리 안한다.
				//writeLog(fpnLog, "WARNNING : Interception method in the watershed ["
				//	+ to_string(ssp->wsid) + "] was not set.\n", 1, -1);
			}
		}
		ssp->interceptMethod = interceptM;
		return 1;
	}
	if (aline.find(fldName.PETMethod) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.PETMethod);
		PETmethod petM = PETmethod::None;
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(FPM))) {
				petM = PETmethod::FPM;
			}
			else if (vStringL == lower(ENUM_TO_STR(BC))) {
				petM = PETmethod::BC;
			}
			else if (vStringL == lower(ENUM_TO_STR(HMN))) {
				petM = PETmethod::HMN;
			}
			else if (vStringL == lower(ENUM_TO_STR(PT))) {
				petM = PETmethod::PT;
			}
			else if (vStringL == lower(ENUM_TO_STR(HRGV))) {
				petM = PETmethod::HRGV;
			}
			//else if (vStringL == lower(ENUM_TO_STR(JensenHaise))) {
			//	petM = PETmethod::JensenHaise;
			//}
			//else if (vStringL == lower(ENUM_TO_STR(Turc))) {
			//	petM = PETmethod::Turc;
			//}
			//else if (vStringL == lower(ENUM_TO_STR(Constant))) {
			//	petM = PETmethod::Constant;
			//}
			else if (vStringL == lower(ENUM_TO_STR(UserET))) {
				petM = PETmethod::UserET;
			}
			else { // 증발산 옵션이므로, 애러처리 안한다.
				//writeLog(fpnLog, "WARNNING : PET method in the watershed ["
				//	+ to_string(ssp->wsid) + "] was not set.\n", 1, -1);
			}
		}
		ssp->potentialETMethod = petM;
		return 1;
	}

	if (aline.find(fldName.ETCoef) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.ETCoef);
		ssp->etCoeff = -1.0;
		if (vString != "" && stod(vString) > 0) {
			ssp->etCoeff = stod(vString);
		}
		else {// 증발산 옵션이므로, 애러처리 안한다.
			//writeLog(fpnLog, "WARNNING : ET coefficient of the watershed ["
			//	+ to_string(ssp->wsid) + "] is invalid.\n", 1, -1);
		}
		return 1;
	}

	if (aline.find(fldName.SnowmeltMethod) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowmeltMethod);
		SnowMeltMethod snowMM = SnowMeltMethod::None;
		if (vString != "") {
			string vStringL = lower(vString);
			if (vStringL == lower(ENUM_TO_STR(Anderson))) {
				snowMM = SnowMeltMethod::Anderson;
			}
			else if (vStringL == lower(ENUM_TO_STR(Constant))) {
				snowMM = SnowMeltMethod::Constant;
			}
			else if (vStringL == lower(ENUM_TO_STR(UserData))) {
				snowMM = SnowMeltMethod::UserData;
			}
			else {// 융설은 옵션이므로, 애러처리 안한다.
				//writeLog(fpnLog, "WARNNING : Snow melt method in the watershed ["
				//	+ to_string(ssp->wsid) + "] was not set.\n", 1, -1);
			}
		}
		ssp->snowMeltMethod = snowMM;
		return 1;
	}
	if (aline.find(fldName.TempSnowRain) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.TempSnowRain);
		if (vString != "" && isNumeric(vString) == true) {
			ssp->tempSnowRain = stod(vString);
		}
		else { // 융설은 옵션이므로, 애러처리 안한다.
			ssp->tempSnowRain = -9999;
			//writeLog(fpnLog, "WARNNING : Snow melt TSR value in the watershed ["
			//	+ to_string(ssp->wsid) + "] is invalid.\n", 1, -1);
		}
		return 1;
	}
	if (aline.find(fldName.SnowmeltingTemp) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowmeltingTemp);
		if (vString != "" && stod(vString) >= 0) {
			ssp->smeltingTemp = stod(vString);
		}
		else {// 융설은 옵션이므로, 애러처리 안한다.
			ssp->smeltingTemp = -9999;
			//writeLog(fpnLog, "WARNNING : Snowmelting temperature value in the watershed ["
			//	+ to_string(ssp->wsid) + "] is invalid.\n", 1, -1);
		}
		return 1;
	}
	if (aline.find(fldName.SnowCovRatio) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowCovRatio);
		if (vString != "" && stod(vString) >= 0) {
			ssp->snowCovRatio = stod(vString);
		}
		else {// 융설은 옵션이므로, 애러처리 안한다.
			ssp->snowCovRatio = -1;
			//writeLog(fpnLog, "WARNNING : Snowpack coverage ratio value in the watershed ["
			//	+ to_string(ssp->wsid) + "] is invalid.\n", 1, -1);
		}
		return 1;
	}
	if (aline.find(fldName.SnowmeltCoef) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.SnowmeltCoef);
		if (vString != "" && stod(vString) >= 0) {
			ssp->smeltCoef = stod(vString);
		}
		else {// 융설은 옵션이므로, 애러처리 안한다.
			ssp->smeltCoef = -1;
			//writeLog(fpnLog, "WARNNING : Snowmelt coefficient value in the watershed ["
			//	+ to_string(ssp->wsid) + "] is invalid.\n", 1, -1);
		}
		return 1;
	}
	// continuous =========================

	if (aline.find(fldName.UserSet) != string::npos) {
		vString = getValueStringFromXmlLine(aline, fldName.UserSet);
		if (lower(vString) == "true") {
			ssp->userSet = 1;
		}
		else if (lower(vString) == "false") {
			ssp->userSet = 0;
		}
		else {
			writeLogString(fpnLog, "ERROR : [UserSet] value of the watershed ["
				+ to_string(ssp->wsid) + "] is invalid.\n", 1, 1);
			return -1;
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

	if (fci->fcType == flowControlType::ReservoirOperation
		|| fci->fcType == flowControlType::DetentionPond) {
		if (fci->iniStorage_m3 == afc_ini.iniStorage_m3) { return -1; }
		if (fci->maxStorage_m3 == afc_ini.maxStorage_m3) { return -1; }

		if (fci->fcType == flowControlType::ReservoirOperation) {
			if (fci->NormalHighStorage_m3 == afc_ini.NormalHighStorage_m3) { return -1; }
			if (fci->RestrictedStorage_m3 == afc_ini.RestrictedStorage_m3) { return -1; }
			if (fci->RestrictedPeriod_Start == afc_ini.RestrictedPeriod_Start) { return -1; }
			if (fci->RestrictedPeriod_End == afc_ini.RestrictedPeriod_End) { return -1; }
			if (prj.isDateTimeFormat == -1) {
				if (fci->RestrictedPeriod_Start_min == afc_ini.RestrictedPeriod_Start_min) { return -1; }
				if (fci->RestrictedPeriod_End_min == afc_ini.RestrictedPeriod_End_min) { return -1; }
			}
			if (fci->roType == afc_ini.roType) { return -1; }
			if (fci->roType == reservoirOperationType::ConstantQ) {
				if (fci->roConstQ_cms == afc_ini.roConstQ_cms) { return -1; }
				if (fci->roConstQDuration_hr == afc_ini.roConstQDuration_hr) { return -1; }
			}
		}

		if (fci->fcType == flowControlType::DetentionPond) {
			if (fci->dp_QT_StoD_CMS == afc_ini.dp_QT_StoD_CMS) { return -1; }
			if (fci->dp_Qi_max_CMS == afc_ini.dp_Qi_max_CMS) { return -1; }
			if (fci->dp_Qo_max_CMS == afc_ini.dp_Qo_max_CMS) { return -1; }
			if (fci->dp_Wdi_m == afc_ini.dp_Wdi_m) { return -1; }
			if (fci->dp_Ws_m == afc_ini.dp_Ws_m) { return -1; }
			if (fci->dp_Cr_StoD == afc_ini.dp_Cr_StoD) { return -1; }
		}
	}
	else {
		if (fci->fcDT_min == afc_ini.fcDT_min) { return -1; }
		if (fci->fpnFCData == afc_ini.fpnFCData) { return -1; }
	}
	return 1;
}


int isNormalSwsParameter(swsParameters *ssp)
{
	swsParameters swsp_ini;//여기서 생성된 초기값과 서로 비교
	if (ssp->wsid == swsp_ini.wsid) { return -1; }
	if (ssp->iniSaturation == swsp_ini.iniSaturation) { return -1; }
	if (ssp->iniLossPRCP_mm == swsp_ini.iniLossPRCP_mm) { return -1; }
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
	prj.swps[targetWSid].iniLossPRCP_mm = spars.iniLossPRCP_mm;
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
	prj.swps[targetWSid].interceptMethod = spars.interceptMethod;
	prj.swps[targetWSid].potentialETMethod = spars.potentialETMethod;
	prj.swps[targetWSid].etCoeff = spars.etCoeff;
	prj.swps[targetWSid].snowMeltMethod = spars.snowMeltMethod;
	prj.swps[targetWSid].tempSnowRain = spars.tempSnowRain;
	prj.swps[targetWSid].smeltingTemp = spars.smeltingTemp;
	prj.swps[targetWSid].snowCovRatio = spars.snowCovRatio;
	prj.swps[targetWSid].smeltCoef = spars.smeltCoef;

	if (prj.swps[targetWSid].userSet != 1) {
		prj.swps[targetWSid].iniFlow = 0;
		prj.swps[targetWSid].userSet = 0;
	}
	return true;
}



