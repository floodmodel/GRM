//
//#include <iostream>
//#include <stdio.h>
//#include <fstream>
//#include <filesystem>
#include <io.h>
#include <string>


#include "gentle.h"
#include "grm.h"

using namespace std;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

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


int openProjectFile()
{
	ifstream prjfile(ppi.fpn_prj);
	if (prjfile.is_open() == false) { return -1; }
	string aline;
	projectFileFieldName fn;
	swsParameters assp;
	channelSettingInfo acs;
	flowControlinfo afc;
	watchPointInfo awp;
	soilTextureInfo ast;
	soilDepthInfo asd;
	landCoverInfo alc;

	while (getline(prjfile, aline))
	{
		string vString = "";
		if (aline.find(fn.GRMSimulationType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.GRMSimulationType);
			//prj.simType = simulationType::None;
			if (vString != "") {
				if (toLower(vString) == "singleevent") {
					prj.simType = simulationType::SingleEvent;
				}
				else if (toLower(vString) == "realtime") {
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
			continue;
		}

		if (aline.find(fn.DomainFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.DomainFile);
			prj.fpnDomain = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnDomain = vString;
			}
			else {
				writeLog(fpnLog, "DEM file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}

		if (aline.find(fn.SlopeFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SlopeFile);
			prj.fpnSlope = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnSlope = vString;
			}
			else {
				writeLog(fpnLog, "Slope file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}

		if (aline.find(fn.FlowDirectionFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FlowDirectionFile);
			prj.fpnFD = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnFD = vString;
			}
			else {
				writeLog(fpnLog, "Flow direction file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}

		if (aline.find(fn.FlowAccumFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FlowAccumFile);
			prj.fpnFA = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnFA = vString;
			}
			else {
				writeLog(fpnLog, "Flow accumulation file [" + vString + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}

		if (aline.find(fn.StreamFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.StreamFile);
			prj.fpnStream = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnStream = vString;
			}
			else {
				writeLog(fpnLog, "Stream file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.ChannelWidthFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthFile);
			prj.fpnChannelWidth = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnChannelWidth = vString;
			}
			else {
				writeLog(fpnLog, "Channel width file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.InitialSoilSaturationRatioFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.InitialSoilSaturationRatioFile);
			prj.fpnIniSSR = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnIniSSR = vString;
			}
			else {
				writeLog(fpnLog, "Soil saturation ratio file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.InitialChannelFlowFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.InitialChannelFlowFile);
			prj.fnpIniChannelFlow = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fnpIniChannelFlow = vString;
			}
			else {
				writeLog(fpnLog, "Soil saturation ratio file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.LandCoverDataType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LandCoverDataType);
			//prj.lcDataType = fileOrConstant::None;
			if (vString != "") {
				if (toLower(vString) == "file") {
					prj.lcDataType = fileOrConstant::File;
				}
				else if (toLower(vString) == "constant") {
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
			continue;
		}

		if (aline.find(fn.LandCoverFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LandCoverFile);
			prj.fpnLC = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnLC = vString;
			}
			continue;
		}

		if (aline.find(fn.LandCoverVATFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LandCoverVATFile);
			prj.fpnLCVat = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnLCVat = vString;
			}
			continue;
		}

		if (aline.find(fn.ConstantRoughnessCoeff) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantRoughnessCoeff);
			if (vString != "") {
				prj.cnstRoughnessC = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.ConstantImperviousRatio) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantImperviousRatio);
			if (vString != "") {
				prj.cnstImperviousR = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.SoilTextureDataType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilTextureDataType);
			//prj.stDataType = fileOrConstant::None;
			if (vString != "") {
				if (toLower(vString) == "file") {
					prj.stDataType = fileOrConstant::File;
				}
				else if (toLower(vString) == "constant") {
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
			continue;
		}

		if (aline.find(fn.SoilTextureFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilTextureFile);
			prj.fpnST = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnST = vString;
			}
			continue;
		}

		if (aline.find(fn.SoilTextureVATFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilTextureVATFile);
			prj.fpnSTVat = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnSTVat = vString;
			}
			continue;
		}

		if (aline.find(fn.ConstantSoilPorosity) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilPorosity);
			if (vString != "") {
				prj.cnstSoilPorosity = stod(vString);
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilEffPorosity) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilEffPorosity);
			if (vString != "") {
				prj.cnstSoilEffPorosity = stod(vString);
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilWettingFrontSuctionHead) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilWettingFrontSuctionHead);
			if (vString != "") {
				prj.cnstSoilWFSH = stod(vString);
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilHydraulicConductivity) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilHydraulicConductivity);
			if (vString != "") {
				prj.cnstSoilHydraulicK = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.SoilDepthDataType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilDepthDataType);
			//prj.sdDataType = fileOrConstant::None;
			if (vString != "") {
				if (toLower(vString) == "file") {
					prj.sdDataType = fileOrConstant::File;
				}
				else if (toLower(vString) == "constant") {
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
			continue;
		}
		if (aline.find(fn.SoilDepthFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilDepthFile);
			prj.fpnSD = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnSD = vString;
			}
			continue;
		}
		if (aline.find(fn.SoilDepthVATFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilDepthVATFile);
			prj.fpnSDVat = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnSDVat = vString;
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilDepth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilDepth);
			if (vString != "") {
				prj.cnstSoilDepth = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.RainfallDataType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.RainfallDataType);
			if (vString != "") {
				if (toLower(vString) == "textfilemap") {
					prj.rfDataType = rainfallDataType::TextFileMAP;
				}
				else if (toLower(vString) == "textfileascgrid") {
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
			continue;
		}
		if (aline.find(fn.RainfallDataFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.RainfallDataFile);
			prj.fpnRainfallData = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnRainfallData = vString;
			}
			else if (prj.simType == simulationType::SingleEvent) {
				writeLog(fpnLog, "Rainfall data file [" + vString + "] was not set.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.RainfallInterval) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.RainfallInterval);
			if (vString != "") {
				prj.rfinterval_min = stod(vString);
			}
			else if (prj.simType == simulationType::SingleEvent) {
				writeLog(fpnLog, "Rainfall data time interval was not set.\n", 1, 1);
				return -1;
			}
			continue;
		}

		if (aline.find(fn.FlowDirectionType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FlowDirectionType);
			if (vString != "") {
				if (toLower(vString) == "startsfromne") {
					prj.fdType = flowDirectionType::StartsFromNE;
				}
				else if (toLower(vString) == "startsfromn") {
					prj.fdType = flowDirectionType::StartsFromN;
				}
				else if (toLower(vString) == "startsfrome") {
					prj.fdType = flowDirectionType::StartsFromE;
				}
				else if (toLower(vString) == "startsfrome_taudem") {
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
			continue;
		}

		if (aline.find(fn.MaxDegreeOfParallelism) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MaxDegreeOfParallelism);
			if (vString != "" && stoi(vString)!=0 && stoi(vString) >= -1) {
				prj.maxDegreeOfParallelism = stoi(vString);
			}
			else {
				writeLog(fpnLog, "Max. degree of parallelism was not set. Maximum value [-1] was assigned.\n", 1, 1);
				prj.maxDegreeOfParallelism = -1;
			}
			continue;
		}

		if (aline.find(fn.SimulStartingTime) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulStartingTime);
			if (vString != "") {
				prj.simulStartingTime = vString;
			}
			else {
				prj.isDateTimeFormat = 1;
				prj.simulStartingTime = "0";
			}
			continue;
		}

		if (aline.find(fn.SimulationDuration) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulationDuration);
			if (vString != "" && stod(vString)>=0) {
				prj.simDuration_hr = stod(vString);
			}
			else {
				writeLog(fpnLog, "Simulation duration is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}

		if (aline.find(fn.ComputationalTimeStep) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ComputationalTimeStep);
			if (vString != "" && stod(vString) > 0) {
				prj.dtsec = stod(vString) * 60.0;
			}
			continue;
		}

		if (aline.find(fn.IsFixedTimeStep) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.IsFixedTimeStep);
			prj.IsFixedTimeStep = -1;
			if (toLower(vString) == "true") {
				prj.IsFixedTimeStep = 1;
			}
			continue;
		}

		if (aline.find(fn.OutputTimeStep) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.OutputTimeStep);
			if (vString != "") {
				prj.printTimeStep_min = stod(vString);
			}
			else {
				writeLog(fpnLog, "Print out time step is invalid.\n", 1, 1);
				return -1;
			}

			continue;
		}

		if (aline.find(fn.SimulateInfiltration) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulateInfiltration);
			prj.simInfiltration = -1;
			if (toLower(vString) == "true") {
				prj.simInfiltration = 1;
			}
			continue;
		}
		if (aline.find(fn.SimulateSubsurfaceFlow) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulateSubsurfaceFlow);
			prj.simSubsurfaceFlow = -1;
			if (toLower(vString) == "true") {
				prj.simSubsurfaceFlow = 1;
			}
			continue;
		}
		if (aline.find(fn.SimulateBaseFlow) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulateBaseFlow);
			prj.simBaseFlow = -1;
			if (toLower(vString) == "true") {
				prj.simBaseFlow = 1;
			}
			continue;
		}
		if (aline.find(fn.SimulateFlowControl) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulateFlowControl);
			prj.simFlowControl = -1;
			if (toLower(vString) == "true") {
				prj.simFlowControl = 1;
			}
			continue;
		}

		if (aline.find(fn.MakeIMGFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MakeIMGFile);
			prj.makeIMGFile = -1;
			if (toLower(vString) == "true") {
				prj.makeIMGFile = 1;
			}
			continue;
		}
		if (aline.find(fn.MakeASCFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MakeASCFile);
			prj.makeASCFile = -1;
			if (toLower(vString) == "true") {
				prj.makeASCFile = 1;
			}
			continue;
		}
		if (aline.find(fn.MakeSoilSaturationDistFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MakeSoilSaturationDistFile);
			prj.makeSoilSaturationDistFile = -1;
			if (toLower(vString) == "true") {
				prj.makeSoilSaturationDistFile = 1;
			}
			continue;
		}
		if (aline.find(fn.MakeRfDistFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MakeRfDistFile);
			prj.makeRfDistFile = -1;
			if (toLower(vString) == "true") {
				prj.makeRfDistFile = 1;
			}
			continue;
		}
		if (aline.find(fn.MakeRFaccDistFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MakeRFaccDistFile);
			prj.makeRFaccDistFile = -1;
			if (toLower(vString) == "true") {
				prj.makeRFaccDistFile = 1;
			}
			continue;
		}
		if (aline.find(fn.MakeFlowDistFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MakeFlowDistFile);
			prj.makeFlowDistFile = -1;
			if (toLower(vString) == "true") {
				prj.makeFlowDistFile = 1;
			}
			continue;
		}

		if (aline.find(fn.PrintOption) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.PrintOption);
			if (vString != "") {
				if (toLower(vString) == "all") {
					prj.printOption = GRMPrintType::All;
				}
				else if (toLower(vString) == "dischargefileq") {
					prj.printOption = GRMPrintType::DischargeFileQ;
				}
				else if (toLower(vString) == "allq") {
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
			continue;
		}
		if (aline.find(fn.WriteLog) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.WriteLog);
			prj.writeLog = -1;
			if (toLower(vString) == "true") {
				prj.writeLog = 1;
			}
			continue;
		}

		//subwatershed parameters. START ===========
		if (aline.find(fn.SWSID) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SWSID);
			if (vString != "" && stoi(vString) > 0) {
				assp.wsid = stoi(vString);
			}
			else {
				writeLog(fpnLog, "Watershed ID is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.IniSaturation) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.IniSaturation);
			if (vString != "" && stod(vString) >= 0) {
				assp.iniSaturation = stod(vString);
			}
			else {
				writeLog(fpnLog, "Ini. saturation ratio in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.MinSlopeOF) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MinSlopeOF);
			if (vString != "" && stod(vString) > 0) {
				assp.minSlopeOF = stod(vString);
			}
			else {
				writeLog(fpnLog, "Minimum land surface slope for overland flow in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.UnsaturatedKType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.UnsaturatedKType);
			unSaturatedKType uskt = unSaturatedKType::None;
			if (vString != "") {
				if (toLower(vString) == "constant") {
					uskt = unSaturatedKType::Constant;
				}
				else if (toLower(vString) == "linear") {
					uskt = unSaturatedKType::Linear;
				}
				else if (toLower(vString) == "exponential") {
					uskt = unSaturatedKType::Exponential;
				}
				else {
					writeLog(fpnLog, "Unsaturated K type in the watershed ["
						+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else {
				writeLog(fpnLog, "Unsaturated K type in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			assp.unSatKType = uskt;
		}
		if (aline.find(fn.CoefUnsaturatedK) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CoefUnsaturatedK);
			if (vString != "" && stod(vString) > 0) {
				assp.coefUnsaturatedK = stod(vString);
			}
			else {
				writeLog(fpnLog, "Hydraulic conductivity for unsaturated soil in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.MinSlopeChBed) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MinSlopeChBed);
			if (vString != "" && stod(vString) > 0) {
				assp.minSlopeChBed = stod(vString);
			}
			else {
				writeLog(fpnLog, "Minimum slope of channel bed in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.MinChBaseWidth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MinChBaseWidth);
			if (vString != "" && stod(vString) > 0) {
				assp.minChBaseWidth = stod(vString);
			}
			else {
				writeLog(fpnLog, "Minimum value of channel width in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.ChRoughness) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChRoughness);
			if (vString != "" && stod(vString) > 0) {
				assp.chRoughness = stod(vString);
			}
			else {
				writeLog(fpnLog, "Roughness coefficient of channel in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.DryStreamOrder) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.DryStreamOrder);
			if (vString != "" && stoi(vString) >= 0) {
				assp.dryStreamOrder = stoi(vString);
			}
			else {
				writeLog(fpnLog, "Dry stream order in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.IniFlow) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.IniFlow);
			if (vString != "" && stod(vString) >= 0) {
				assp.iniFlow = stod(vString);
			}
			else {
				writeLog(fpnLog, "Initial stream flow in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.CalCoefLCRoughness) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CalCoefLCRoughness);
			if (vString != "" && stod(vString) > 0) {
				assp.ccLCRoughness = stod(vString);
			}
			else {
				writeLog(fpnLog, "Calibration coeff. of roughness in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.CalCoefPorosity) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CalCoefPorosity);
			if (vString != "" && stod(vString) > 0) {
				assp.ccPorosity = stod(vString);
			}
			else {
				writeLog(fpnLog, "Calibration coeff. of porosity in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.CalCoefWFSuctionHead) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CalCoefWFSuctionHead);
			if (vString != "" && stod(vString) > 0) {
				assp.ccWFSuctionHead = stod(vString);
			}
			else {
				writeLog(fpnLog, "Calibration coeff. of wetting front suction head in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.CalCoefHydraulicK) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CalCoefHydraulicK);
			if (vString != "" && stod(vString) > 0) {
				assp.ccHydraulicK = stod(vString);
			}
			else {
				writeLog(fpnLog, "Calibration coeff. of hydraulic conductivity in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.CalCoefSoilDepth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CalCoefSoilDepth);
			if (vString != "" && stod(vString) > 0) {
				assp.ccSoilDepth = stod(vString);
			}
			else {
				writeLog(fpnLog, "Calibration coeff. of soil depth in the watershed ["
					+ to_string(assp.wsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.UserSet) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.UserSet);
			if (toLower(vString) == "true") {
				assp.userSet = 1;
			}
			else if (toLower(vString) == "false") {
				assp.userSet = -1;
			}
		}

		if (assp.wsid >0 && isNormalSwsParameter(assp) == 1) {
			prj.swps.push_back(assp);
			assp = nullSwsParameters();
			continue;
		}
		//subwatershed parameters ===========

		// channel setting info ===============
		if (aline.find(fn.MDWSID) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MDWSID);
			if (vString != "" && stoi(vString) > 0) {
				acs.mdWsid = stoi(vString);
			}
			else {
				writeLog(fpnLog, "Most downstream watershed ID for channel setting is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.CrossSectionType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CrossSectionType);
			crossSectionType acst = crossSectionType::None;
			if (vString != "") {
				if (toLower(vString) == "cscompound") {
					acst = crossSectionType::CSCompound;
				}
				else if (toLower(vString) == "cssingle") {
					acst = crossSectionType::CSSingle;
				}				
				else {
					writeLog(fpnLog, "Channel cross section type in the watershed ["
						+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else {
				writeLog(fpnLog, "Channel cross section type in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			acs.csType = acst;
		}
		if (aline.find(fn.SingleCSChannelWidthType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SingleCSChannelWidthType);
			channelWidthType  acwt = channelWidthType::None;
			if (vString != "") {
				if (toLower(vString) == "cwequation") {
					acwt = channelWidthType::CWEquation;
				}
				else if (toLower(vString) == "cwgeneration") {
					acwt = channelWidthType::CWGeneration;
				}
				else {
					writeLog(fpnLog, "Channel width type in the watershed ["
						+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else {
				writeLog(fpnLog, "Channel width type in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			acs.csWidthType = acwt;
		}
		if (aline.find(fn.ChannelWidthEQc) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthEQc);
			if (vString != "" && stod(vString) > 0) {
				acs.cwEQc = stod(vString);
			}
			else if (acs.csWidthType==channelWidthType::CWEquation){
				writeLog(fpnLog, "EQc parameter for channel width eq. in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.ChannelWidthEQd) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthEQd);
			if (vString != "" && stod(vString) > 0) {
				acs.cwEQd = stod(vString);
			}
			else if (acs.csWidthType == channelWidthType::CWEquation) {
				writeLog(fpnLog, "EQd parameter for channel width eq. in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.ChannelWidthEQe) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthEQe);
			if (vString != "" && stod(vString) > 0) {
				acs.cwEQe = stod(vString);
			}
			else if (acs.csWidthType == channelWidthType::CWEquation) {
				writeLog(fpnLog, "EQe parameter for channel width eq. in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.ChannelWidthMostDownStream) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthMostDownStream);
			if (vString != "" && stod(vString) > 0) {
				acs.cwMostDownStream = stod(vString);
			}
			else if (acs.csWidthType == channelWidthType::CWGeneration) {
				writeLog(fpnLog, "The channel width at the most down stream in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.LowerRegionHeight) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LowerRegionHeight);
			if (vString != "" && stod(vString) > 0) {
				acs.lowRegionHeight = stod(vString);
			}
			else if (acs.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Lower region height parameter in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.LowerRegionBaseWidth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LowerRegionBaseWidth);
			if (vString != "" && stod(vString) > 0) {
				acs.lowRegionBaseWidth = stod(vString);
			}
			else if (acs.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Lower region base width parameter in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.UpperRegionBaseWidth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.UpperRegionBaseWidth);
			if (vString != "" && stod(vString) > 0) {
				acs.upRegionBaseWidth = stod(vString);
			}
			else if (acs.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Upper region base width parameter in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.CompoundCSChannelWidthLimit) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CompoundCSChannelWidthLimit);
			if (vString != "" && stod(vString) > 0) {
				acs.compoundCSChannelWidthLimit = stod(vString);
			}
			else if (acs.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Compound cross section width limit parameter in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.BankSideSlopeRight) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.BankSideSlopeRight);
			if (vString != "" && stod(vString) > 0) {
				acs.bankSlopeRight = stod(vString);
			}
			else {
				writeLog(fpnLog, "Right bank side slope parameter in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.BankSideSlopeLeft) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.BankSideSlopeLeft);
			if (vString != "" && stod(vString) > 0) {
				acs.bankSlopeLeft = stod(vString);
			}
			else {
				writeLog(fpnLog, "Left bank side slope parameter in the watershed ["
					+ to_string(acs.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if ( acs.mdWsid>0 && isNormalChannelSettingInfo(acs) == 1) {
			prj.css.push_back(acs);
			acs = nullChannelSettingInfo();
			continue;
		}


		// flow control info ================
		if (aline.find(fn.FCName) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FCName);
			if (vString != "") {
				afc.fcName = vString;
			}
		}
		if (aline.find(fn.FCColX) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FCColX);
			if (vString != "" && stoi(vString) >= 0) {
				afc.fcColX = stoi(vString);
			}
			else{
				writeLog(fpnLog, "ColX  value of ["+ afc.fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.FCRowY) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FCRowY);
			if (vString != "" && stoi(vString) >= 0) {
				afc.fcRowY = stoi(vString);
			}
			else {
				writeLog(fpnLog, "RowY  value of [" + afc.fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.ControlType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ControlType);
			flowControlType  afct =  flowControlType::None;
			if (vString != "") {
				if (toLower(vString) == "inlet") {
					afct = flowControlType::Inlet;
				}
				else if (toLower(vString) == "reservoiroperation") {
					afct = flowControlType::ReservoirOperation;
				}
				else if (toLower(vString) == "reservoiroutflow") {
					afct = flowControlType::ReservoirOutflow;
				}
				else if (toLower(vString) == "sinkflow") {
					afct = flowControlType::SinkFlow;
				}
				else if (toLower(vString) == "sourceflow") {
					afct = flowControlType::SourceFlow;
				}
				else {
					writeLog(fpnLog, "Flow control type of ["
						+ afc.fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else {
				writeLog(fpnLog, "Flow control type of ["
					+ afc.fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
			afc.fcType= afct;
		}
		if (aline.find(fn.FCDT) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FCDT);
			if (vString != "" && stod(vString) >= 0) {
				afc.fcDT = stod(vString);
			}
			else {
				writeLog(fpnLog, "Flow control data time interval of [" 
					+ afc.fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (aline.find(fn.FlowDataFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FlowDataFile);
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				afc.fcDataFile = vString;
			}
			else {
				writeLog(fpnLog, "Flow control data file of [" 
					+ afc.fcName + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (afc.fcType == flowControlType::ReservoirOperation) {
			if (aline.find(fn.IniStorage) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.IniStorage);
				if (vString != "" && stod(vString) >= 0) {
					afc.iniStorage = stod(vString);
				}
				else {
					writeLog(fpnLog, "Ini. storage of reservoir ["
						+ afc.fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.MaxStorage) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.MaxStorage);
				if (vString != "" && stod(vString) >= 0) {
					afc.maxStorage = stod(vString);
				}
				else {
					writeLog(fpnLog, "Max. storage of reservoir ["
						+ afc.fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.MaxStorageR) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.MaxStorageR);
				if (vString != "" && stod(vString) >= 0) {
					afc.maxStorageR = stod(vString);
				}
				else {
					writeLog(fpnLog, "Max. storage ratio of reservoir ["
						+ afc.fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.ROType) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.ROType);
				reservoirOperationType  arot = reservoirOperationType::None;
				if (vString != "") {
					if (toLower(vString) == "autorom") {
						arot = reservoirOperationType::AutoROM;
					}
					else if (toLower(vString) == "constantq") {
						arot = reservoirOperationType::ConstantQ;
					}
					else if (toLower(vString) == "rigidrom") {
						arot = reservoirOperationType::RigidROM;
					}
					else if (toLower(vString) == "sdeqation") {
						arot = reservoirOperationType::SDEqation;
					}
					else {
						writeLog(fpnLog, "Reservoir operation type of ["
							+ afc.fcName + "] is invalid.\n", 1, 1);
						return -1;
					}
				}
				else {
					writeLog(fpnLog, "Reservoir operation type of ["
						+ afc.fcName + "] is invalid.\n", 1, 1);
					return -1;
				}
				afc.roType = arot;
			}
			if (afc.roType == reservoirOperationType::ConstantQ) {
				if (aline.find(fn.ROConstQ) != string::npos) {
					vString = getValueStringFromXmlLine(aline, fn.ROConstQ);
					if (vString != "" && stod(vString) >= 0) {
						afc.roConstQ = stod(vString);
					}
					else {
						writeLog(fpnLog, "Constant outlfow of reservoir ["
							+ afc.fcName + "] is invalid.\n", 1, 1);
						return -1;
					}
				}
				if (aline.find(fn.ROConstQDuration) != string::npos) {
					vString = getValueStringFromXmlLine(aline, fn.ROConstQDuration);
					if (vString != "" && stod(vString) >= 0) {
						afc.roConstQDuration = stod(vString);
					}
					else {
						writeLog(fpnLog, "Constant outlfow duration of reservoir ["
							+ afc.fcName + "] is invalid.\n", 1, 1);
						return -1;
					}
				}
			}
		}
		if (afc.fcName !="" && isNormalFlowControlinfo(afc) == 1) {
			prj.fcs.push_back(afc);
			afc = nullFlowControlinfo();
			continue;
		}
		// flow control info. =================

		// watch point ====================
		if (aline.find(fn.WPName) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.WPName);
			if (vString != "") {
				awp.wpName = vString;
			}
		}
		if (aline.find(fn.WPColX) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.WPColX);
			if (vString != "") {
				awp.wpColX = stoi(vString);
			}
		}
		if (aline.find(fn.WPRowY) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.WPRowY);
			if (vString != "") {
				awp.wpRowY = stoi(vString);
			}
		}
		if (awp.wpName!="" && isNormalWatchPointInfo(awp) == 1) {
			prj.wps.push_back(awp);
			awp = nullWatchPointInfo();
			continue;
		}
		// watch point =====================

		// soil texture =================
		if (prj.stDataType == fileOrConstant::File) {
			if (aline.find(fn.STGridValue) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.STGridValue);
				if (vString != "" && stoi(vString) >= 0) {
					ast.stGridValue = stoi(vString);
				}
				else {
					writeLog(fpnLog, "Soil texture raster value in gmp file is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.GRMCodeST) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.GRMCodeST);
				if (vString != "") {
					ast.stName = vString;
				}
				else {
					writeLog(fpnLog, "Soil texture name of the raster value ["
						+ to_string(ast.stGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.STPorosity) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.STPorosity);
				if (vString != "") {
					ast.porosity = stod(vString);
				}
				else {
					writeLog(fpnLog, "Soil porosity of the raster value ["
						+ to_string(ast.stGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.STEffectivePorosity) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.STEffectivePorosity);
				if (vString != "") {
					ast.effectivePorosity = stod(vString);
				}
				else {
					writeLog(fpnLog, "Soil effective porosity of the raster value ["
						+ to_string(ast.stGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.STWFSuctionHead) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.STWFSuctionHead);
				if (vString != "") {
					ast.WFSuctionHead = stod(vString);
				}
				else {
					writeLog(fpnLog, "Soil wetting front suction head of the raster value ["
						+ to_string(ast.stGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.STHydraulicConductivity) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.STHydraulicConductivity);
				if (vString != "") {
					ast.hydraulicK = stod(vString);
				}
				else {
					writeLog(fpnLog, "Soil hydraulic conductivity of the raster value ["
						+ to_string(ast.stGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}

			if (ast.stGridValue > 0 && isNormalSoilTextureInfo(ast) == 1) {
				prj.sts.push_back(ast);
				ast = nullSoilTextureInfo();
				continue;
			}
		}
		// soil texture =================

		// soil depth =================
		if (prj.sdDataType == fileOrConstant::File) {
			if (aline.find(fn.SDGridValue) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.SDGridValue);
				if (vString != "" && stoi(vString) >= 0) {
					asd.sdGridValue = stoi(vString);
				}
				else {
					writeLog(fpnLog, "Soil depth raster value in gmp file is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.GRMCodeSD) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.GRMCodeSD);
				if (vString != "") {
					asd.sdName = vString;
				}
				else {
					writeLog(fpnLog, "Soil depth name of the raster value ["
						+ to_string(asd.sdGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.SDSoilDepth) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.SDSoilDepth);
				if (vString != "") {
					asd.soilDepth = stod(vString);
				}
				else {
					writeLog(fpnLog, "Soil depth value of the raster value ["
						+ to_string(asd.sdGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}

			if (asd.sdGridValue > 0 && isNormalSoilDepthInfo(asd) == 1) {
				prj.sds.push_back(asd);
				asd = nullSoilDepthInfo();
				continue;
			}
		}
		// soil depth =================

		// land cover =================
		if (prj.lcDataType == fileOrConstant::File) {
			if (aline.find(fn.LCGridValue) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.LCGridValue);
				if (vString != "" && stoi(vString) >= 0) {
					alc.lcGridValue = stoi(vString);
				}
				else {
					writeLog(fpnLog, "Land cover raster value in gmp file is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.GRMCodeLC) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.GRMCodeLC);
				if (vString != "") {
					alc.lcName = vString;
				}
				else {
					writeLog(fpnLog, "Land cover name of the raster value ["
						+ to_string(alc.lcGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.LCRoughnessCoeff) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.LCRoughnessCoeff);
				if (vString != "") {
					alc.RoughnessCoefficient = stod(vString);
				}
				else {
					writeLog(fpnLog, "Land cover roughness coefficient of the raster value ["
						+ to_string(alc.lcGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (aline.find(fn.ImperviousRatio) != string::npos) {
				vString = getValueStringFromXmlLine(aline, fn.ImperviousRatio);
				if (vString != "") {
					alc.ImperviousRatio = stod(vString);
				}
				else {
					writeLog(fpnLog, "Land cover impervious ratio of the raster value ["
						+ to_string(alc.lcGridValue) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			if (alc.lcGridValue > 0 && isNormalLandCoverInfo(alc) == 1) {
				prj.lcs.push_back(alc);
				alc = nullLandCoverInfo();
				continue;
			}
		}
		// land cover =================
	}
	   
	if (prj.printTimeStep_min * 30 < prj.dtsec) {
		prj.dtsec = prj.printTimeStep_min * 30;
	}
	if (prj.lcDataType == fileOrConstant::File) {
		if (prj.fpnLC == "") {
			writeLog(fpnLog, "Land cover file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.fpnLCVat == "") {
			writeLog(fpnLog, "Land cover VAT file is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.lcs.size() ==0) {
			writeLog(fpnLog, "Land cover attributes from the files are invalid.\n", 1, 1);
			return -1;
		}
	}
	else if (prj.lcDataType == fileOrConstant::Constant) {
		if (prj.cnstRoughnessC == -1.0) {
			writeLog(fpnLog, "Land cover constant roughness is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstImperviousR == -1.0) {
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
	else if(prj.stDataType==fileOrConstant::Constant){
		if (prj.cnstSoilEffPorosity == -1.0) {
			writeLog(fpnLog, "Soil texture constant effective porosity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilHydraulicK == -1.0) {
			writeLog(fpnLog, "Soil texture constant hydraulic conductivity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilPorosity == -1.0) {
			writeLog(fpnLog, "Soil texture constant porosity is invalid.\n", 1, 1);
			return -1;
		}
		if (prj.cnstSoilWFSH == -1.0) {
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
		if (prj.cnstSoilDepth == -1.0) {
			writeLog(fpnLog, "Soil depth constant value is invalid.\n", 1, 1);
			return -1;
		}
	}

	if (prj.dtsec == -1) {
		if (prj.IsFixedTimeStep == 1) {
			writeLog(fpnLog, "In fixed dt, the calculation time step ["
				+ to_string(prj.dtsec) + "] is invalid.\n", 1, 1);
			return -1;
		}
		else if (prj.IsFixedTimeStep == -1) {
			writeLog(fpnLog, "In adaptive dt, the calculation time step ["
				+ to_string(prj.dtsec) + "] is changed to 1 minute.\n", 1, -1);
		}
	}

	return 1;
}

channelSettingInfo nullChannelSettingInfo()
{
	channelSettingInfo aci;
	return aci;
}


flowControlinfo nullFlowControlinfo()
{
	flowControlinfo afc;
	return afc;
}

swsParameters nullSwsParameters()
{
	swsParameters assp;
	return assp;
}

watchPointInfo nullWatchPointInfo()
{
	watchPointInfo awp;
	return awp;
}

soilTextureInfo nullSoilTextureInfo()
{
	soilTextureInfo ast;
	return ast;
}

soilDepthInfo nullSoilDepthInfo()
{
	soilDepthInfo asd;
	return asd;
}

landCoverInfo nullLandCoverInfo()
{
	landCoverInfo alc;
	return alc;
}

int isNormalChannelSettingInfo(channelSettingInfo aci)
{
	channelSettingInfo aci_ini; //여기서 생성된 초기값과 서로 비교
	if (aci.mdWsid == aci_ini.mdWsid) { return -1; }
	if (aci.csType == aci_ini.csType) { return -1; }
	if (aci.csWidthType == aci_ini.csWidthType) { return -1; }
	if (aci.csWidthType == channelWidthType::CWGeneration) {
		if (aci.cwEQc == aci_ini.cwEQc) { return -1; }
		if (aci.cwEQd == aci_ini.cwEQd) { return -1; }
		if (aci.cwEQe == aci_ini.cwEQe) { return -1; }
	}
	else if (aci.csWidthType == channelWidthType::CWGeneration) {
		if (aci.cwMostDownStream == aci_ini.cwMostDownStream) { return -1; }
	}
	if (aci.csType == crossSectionType::CSCompound) {
		if (aci.lowRegionHeight == aci_ini.lowRegionHeight) { return -1; }
		if (aci.lowRegionBaseWidth == aci_ini.lowRegionBaseWidth) { return -1; }
		if (aci.upRegionBaseWidth == aci_ini.upRegionBaseWidth) { return -1; }
		if (aci.compoundCSChannelWidthLimit == aci_ini.compoundCSChannelWidthLimit) { return -1; }
	}
	if (aci.bankSlopeRight == aci_ini.bankSlopeRight) { return -1; }
	if (aci.bankSlopeLeft == aci_ini.bankSlopeLeft) { return -1; }
	return 1;
}

int isNormalFlowControlinfo(flowControlinfo afc)
{
	flowControlinfo afc_ini;
	//여기서 생성된 초기값과 서로 비교
	if (afc.fcName == afc_ini.fcName) { return -1; }
	if (afc.fcColX == afc_ini.fcColX) { return -1; }
	if (afc.fcRowY == afc_ini.fcRowY) { return -1; }
	if (afc.fcType == afc_ini.fcType) { return -1; }
	if (afc.fcDT == afc_ini.fcDT) { return -1; }
	if (afc.fcType == flowControlType::ReservoirOperation) {
		if (afc.iniStorage == afc_ini.iniStorage) { return -1; }
		if (afc.maxStorage == afc_ini.maxStorage) { return -1; }
		if (afc.maxStorageR == afc_ini.maxStorageR) { return -1; }
		if (afc.roType == afc_ini.roType) { return -1; }
		if (afc.roConstQ == afc_ini.roConstQ) { return -1; }
		if (afc.roConstQDuration == afc_ini.roConstQDuration) { return -1; }
	} else 	if (afc.fcDataFile == afc_ini.fcDataFile){ return -1; }
	return 1;
}

int isNormalSwsParameter(swsParameters assp)
{
	swsParameters swsp_ini;//여기서 생성된 초기값과 서로 비교
	if (assp.wsid == swsp_ini.wsid) { return -1; }
	if (assp.iniSaturation == swsp_ini.iniSaturation) { return -1; }
	if (assp.minSlopeOF == swsp_ini.minSlopeOF) { return -1; }
	if (assp.unSatKType == swsp_ini.unSatKType) { return -1; }
	if (assp.coefUnsaturatedK == swsp_ini.coefUnsaturatedK) { return -1; }
	if (assp.minSlopeChBed == swsp_ini.minSlopeChBed) { return -1; }
	if (assp.minChBaseWidth == swsp_ini.minChBaseWidth) { return -1; }
	if (assp.chRoughness == swsp_ini.chRoughness) { return -1; }
	if (assp.dryStreamOrder == swsp_ini.dryStreamOrder) { return -1; }
	if (assp.iniFlow == swsp_ini.iniFlow) { return -1; }
	if (assp.ccLCRoughness == swsp_ini.ccLCRoughness) { return -1; }
	if (assp.ccPorosity == swsp_ini.ccPorosity) { return -1; }
	if (assp.ccWFSuctionHead == swsp_ini.ccWFSuctionHead) { return -1; }
	if (assp.ccHydraulicK == swsp_ini.ccHydraulicK) { return -1; }
	if (assp.ccSoilDepth == swsp_ini.ccSoilDepth) { return -1; }
	if (assp.userSet == swsp_ini.userSet) { return -1; }
	return 1;
}

int isNormalWatchPointInfo(watchPointInfo awp)
{
	watchPointInfo wpi_ini;//여기서 생성된 초기값과 서로 비교
	if (awp.wpName == wpi_ini.wpName){ return -1; };
	if (awp.wpColX == wpi_ini.wpColX) { return -1; };
	if (awp.wpRowY == wpi_ini.wpRowY) { return -1; };
	return 1;
}

int isNormalSoilTextureInfo(soilTextureInfo ast)
{
	soilTextureInfo sti;//여기서 생성된 초기값과 서로 비교
	if (ast.stGridValue == sti.stGridValue) { return -1; }
	if (ast.stName == sti.stName) { return -1; }
	if (ast.porosity == sti.porosity) { return -1; }
	if (ast.effectivePorosity == sti.effectivePorosity) { return -1; }
	if (ast.WFSuctionHead == sti.WFSuctionHead) { return -1; }
	if (ast.hydraulicK == sti.hydraulicK) { return -1; }
	return 1;
}

int isNormalSoilDepthInfo(soilDepthInfo asd)
{
	soilDepthInfo sdi;//여기서 생성된 초기값과 서로 비교
	if (asd.sdGridValue == sdi.sdGridValue) { return -1; }
	if (asd.sdName == sdi.sdName) { return -1; }
	if (asd.soilDepth == sdi.soilDepth ) { return -1; }
	return 1;
}

int isNormalLandCoverInfo(landCoverInfo alc)
{
	landCoverInfo lci;//여기서 생성된 초기값과 서로 비교하는 것으로 수정 필요?
	if(alc.lcGridValue == lci.lcGridValue) { return -1; }
	if (alc.lcName == lci.lcName) { return -1; }
	if (alc.RoughnessCoefficient == lci.RoughnessCoefficient) { return -1; }
	if (alc.ImperviousRatio == lci.ImperviousRatio) { return -1; }
	return 1;
}






