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

int openProjectFile()
{
	ifstream prjfile(ppi.fpn_prj);
	if (prjfile.is_open() == false) { return -1; }
	string aline;
	projectFileFieldName fn;
	swsParameters assp;
	channelSettingInfo aci;
	watchPointInfo awp;


	while (getline(prjfile, aline))
	{
		string vString = "";
		if (aline.find(fn.GRMSimulationType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.GRMSimulationType);
			prj.simType = simulationType::None;
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
			prj.lcDataType = fileOrConstant::None;
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
			else {
				writeLog(fpnLog, "Land cover file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.LandCoverVatFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LandCoverVatFile);
			prj.fpnLCVat = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnLCVat = vString;
			}
			else {
				writeLog(fpnLog, "Land cover VAT file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.ConstantRoughnessCoeff) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantRoughnessCoeff);
			prj.cnstRoughnessC = 0;
			if (vString != "") {
				prj.cnstRoughnessC = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.ConstantImperviousRatio) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantImperviousRatio);
			prj.cnstImperviousR = 0;
			if (vString != "") {
				prj.cnstImperviousR = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.SoilTextureDataType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilTextureDataType);
			prj.stDataType = fileOrConstant::None;
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
			else {
				writeLog(fpnLog, "Soil texture file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.SoilTextureVATFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilTextureVATFile);
			prj.fpnSTVat = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnSTVat = vString;
			}
			else {
				writeLog(fpnLog, "Soil texture VAT file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}

		if (aline.find(fn.ConstantSoilPorosity) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilPorosity);
			prj.cnstSoilPorosity = 0;
			if (vString != "") {
				prj.cnstSoilPorosity = stod(vString);
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilEffPorosity) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilEffPorosity);
			prj.cnstSoilEffPorosity = 0;
			if (vString != "") {
				prj.cnstSoilEffPorosity = stod(vString);
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilWettingFrontSuctionHead) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilWettingFrontSuctionHead);
			prj.cnstSoilWFSH = 0;
			if (vString != "") {
				prj.cnstSoilWFSH = stod(vString);
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilHydraulicConductivity) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilHydraulicConductivity);
			prj.cnstSoilHydraulicK = 0;
			if (vString != "") {
				prj.cnstSoilHydraulicK = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.SoilDepthDataType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilDepthDataType);
			prj.sdDataType = fileOrConstant::None;
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
			else {
				writeLog(fpnLog, "Soil depth file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}
		if (aline.find(fn.SoilDepthVATFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SoilDepthVATFile);
			prj.fpnSDVat = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnSDVat = vString;
			}
			else {
				writeLog(fpnLog, "Soil depth VAT file [" + vString + "] was not set.\n", 1, -1);
			}
			continue;
		}
		if (aline.find(fn.ConstantSoilDepth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantSoilDepth);
			prj.cnstSoilDepth = 0;
			if (vString != "") {
				prj.cnstSoilDepth = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.RainfallDataType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.RainfallDataType);
			prj.rfDataType = rainfallDataType::NoneRF;
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
			prj.RainfallDataFile = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.RainfallDataFile = vString;
			}
			else if (prj.simType == simulationType::SingleEvent) {
				writeLog(fpnLog, "Rainfall data file [" + vString + "] was not set.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.RainfallInterval) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.RainfallInterval);
			prj.rfinterval_min = 0;
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
			prj.fdType = flowDirectionType::None;
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
			prj.maxDegreeOfParallelism = -1;
			if (vString != "") {
				prj.maxDegreeOfParallelism = stoi(vString);
			}
			continue;
		}

		if (aline.find(fn.SimulStartingTime) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulStartingTime);
			prj.SimulStartingTime = "0";
			prj.isDateTimeFormat = -1;
			if (vString != "") {
				prj.SimulStartingTime = vString;
			}
			if (prj.SimulStartingTime != "0") {
				prj.isDateTimeFormat = 1;
			}
			continue;
		}

		if (aline.find(fn.SimulationDuration) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SimulationDuration);
			prj.simDuration_hr = 1;
			if (vString != "") {
				prj.simDuration_hr = stod(vString);
			}
			continue;
		}

		if (aline.find(fn.ComputationalTimeStep) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ComputationalTimeStep);
			prj.dtsec = 1;
			if (vString != "") {
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
			prj.printTimeStep_min = 1.0;
			if (vString != "") {
				prj.printTimeStep_min = stod(vString);
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
			prj.printOption = GRMPrintType::None;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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
			continue;
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

		if (isNormalSwsParameters(assp) == 1) {
			prj.swps.push_back(assp);
			assp = nullSwsParameters();
			continue;
		}
		//subwatershed parameters ===========

		// channel setting info ===============
		if (aline.find(fn.MDWSID) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.MDWSID);
			if (vString != "" && stoi(vString) > 0) {
				aci.mdWsid = stoi(vString);
			}
			else {
				writeLog(fpnLog, "Most downstream watershed ID for channel setting is invalid.\n", 1, 1);
				return -1;
			}
			continue;
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
						+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else {
				writeLog(fpnLog, "Channel cross section type in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			aci.csType = acst;
			continue;
		}
		if (aline.find(fn.SingleCSChannelWidthType) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SingleCSChannelWidthType);
			channelWidthType  acw = channelWidthType::None;
			if (vString != "") {
				if (toLower(vString) == "cwequation") {
					acw = channelWidthType::CWEquation;
				}
				else if (toLower(vString) == "cwgeneration") {
					acw = channelWidthType::CWGeneration;
				}
				else {
					writeLog(fpnLog, "Channel width type in the watershed ["
						+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
					return -1;
				}
			}
			else {
				writeLog(fpnLog, "Channel width type in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			aci.csWidthType = acw;
			continue;
		}
		if (aline.find(fn.ChannelWidthEQc) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthEQc);
			if (vString != "" && stod(vString) > 0) {
				aci.cwEQc = stod(vString);
			}
			else if (aci.csWidthType==channelWidthType::CWEquation){
				writeLog(fpnLog, "EQc parameter for channel width eq. in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.ChannelWidthEQd) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthEQd);
			if (vString != "" && stod(vString) > 0) {
				aci.cwEQd = stod(vString);
			}
			else if (aci.csWidthType == channelWidthType::CWEquation) {
				writeLog(fpnLog, "EQd parameter for channel width eq. in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.ChannelWidthEQe) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthEQe);
			if (vString != "" && stod(vString) > 0) {
				aci.cwEQe = stod(vString);
			}
			else if (aci.csWidthType == channelWidthType::CWEquation) {
				writeLog(fpnLog, "EQe parameter for channel width eq. in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.ChannelWidthMostDownStream) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthMostDownStream);
			if (vString != "" && stod(vString) > 0) {
				aci.cwMostDownStream = stod(vString);
			}
			else if (aci.csWidthType == channelWidthType::CWGeneration) {
				writeLog(fpnLog, "The channel width at the most down stream in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.LowerRegionHeight) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LowerRegionHeight);
			if (vString != "" && stod(vString) > 0) {
				aci.lowRegionHeight = stod(vString);
			}
			else if (aci.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Lower region height parameter in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.LowerRegionBaseWidth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LowerRegionBaseWidth);
			if (vString != "" && stod(vString) > 0) {
				aci.lowRegionBaseWidth = stod(vString);
			}
			else if (aci.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Lower region base width parameter in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.UpperRegionBaseWidth) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.UpperRegionBaseWidth);
			if (vString != "" && stod(vString) > 0) {
				aci.upRegionBaseWidth = stod(vString);
			}
			else if (aci.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Upper region base width parameter in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.CompoundCSChannelWidthLimit) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.CompoundCSChannelWidthLimit);
			if (vString != "" && stod(vString) > 0) {
				aci.compoundCSChannelWidthLimit = stod(vString);
			}
			else if (aci.csType == crossSectionType::CSCompound) {
				writeLog(fpnLog, "Compound cross section width limit parameter in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.BankSideSlopeRight) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.BankSideSlopeRight);
			if (vString != "" && stod(vString) > 0) {
				aci.bankSlopeRight = stod(vString);
			}
			else {
				writeLog(fpnLog, "Right bank side slope parameter in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
			continue;
		}
		if (aline.find(fn.BankSideSlopeLeft) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.BankSideSlopeLeft);
			if (vString != "" && stod(vString) > 0) {
				aci.bankSlopeLeft = stod(vString);
			}
			else {
				writeLog(fpnLog, "Left bank side slope parameter in the watershed ["
					+ to_string(aci.mdWsid) + "] is invalid.\n", 1, 1);
				return -1;
			}
		}
		if (isNormalChannelSettingInfo(aci) == 1) {
			prj.css.push_back(aci);
			aci = nullChannelSettingInfo();
			continue;
		}


				// flow control info ================
		//const string FCName = "FCName";
		//const string FCColX = "FCColX";
		//const string FCRowY = "FCRowY";
		//const string ControlType = "ControlType";
		//const string FCDT = "FCDT";
		//const string FlowDataFile = "FlowDataFile";
		//const string IniStorage = "IniStorage";
		//const string MaxStorage = "MaxStorage";
		//const string MaxStorageR = "MaxStorageR";
		//const string ROType = "ROType";
		//const string ROConstQ = "ROConstQ";
		//const string ROConstQDuration = "ROConstQDuration";


		// watch point ====================
		if (aline.find(fn.WPName) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.WPName);
			if (vString != "") {
				awp.wpName = vString;
			}
			continue;
		}

		if (aline.find(fn.WPColX) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.WPColX);
			if (vString != "") {
				awp.wpColX = stoi(vString);
			}
			continue;
		}

		if (aline.find(fn.WPRowY) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.WPRowY);
			if (vString != "") {
				awp.wpRowY = stoi(vString);
			}
		}

		if (isNormalWatchPointInfo(awp) == 1) {
			prj.wps.push_back(awp);
			awp = nullWatchPointInfo();
			continue;
		}
		// watch point =====================





	}



	if (prj.printTimeStep_min * 30 < prj.dtsec) {
		prj.dtsec = prj.printTimeStep_min * 30;
	}

	return 1;
}

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
	return pfi;
}



channelSettingInfo nullChannelSettingInfo()
{
	channelSettingInfo aci;
	aci.mdWsid = -1;
	aci.csType = crossSectionType::None;
	aci.csWidthType = channelWidthType::None;
	aci.cwEQc = -1.0;
	aci.cwEQd = -1.0;
	aci.cwEQe = -1.0;
	aci.cwMostDownStream = -1.0;
	aci.lowRegionHeight = -1.0;
	aci.lowRegionBaseWidth = -1.0;
	aci.upRegionBaseWidth = -1.0;
	aci.compoundCSChannelWidthLimit = -1.0;
	aci.bankSlopeRight = -1.0;
	aci.bankSlopeLeft = -1.0;
	return aci;
}


swsParameters nullSwsParameters()
{
	swsParameters assp;
	assp.wsid = -1;
	assp.iniSaturation = -1.0;
	assp.minSlopeOF = -1.0;
	assp.unSatKType = unSaturatedKType::None;
	assp.coefUnsaturatedK = -1.0;
	assp.minSlopeChBed = -1.0;
	assp.minChBaseWidth = -1.0;
	assp.chRoughness = -1.0;
	assp.dryStreamOrder = -1;
	assp.iniFlow = -1.0;
	assp.ccLCRoughness = -1.0;
	assp.ccPorosity = -1.0;
	assp.ccWFSuctionHead = -1.0;
	assp.ccHydraulicK = -1.0;
	assp.ccSoilDepth = -1.0;
	assp.userSet = 0;
	return assp;
}

int isNormalChannelSettingInfo(channelSettingInfo aci)
{
	if (aci.mdWsid == -1.0) { return -1; }
	if (aci.csType == crossSectionType::None) { return -1; }
	if (aci.csWidthType == channelWidthType::None) { return -1; }
	if (aci.cwEQc == -1.0) { return -1; }
	if (aci.cwEQd == -1.0) { return -1; }
	if (aci.cwEQe == -1.0) { return -1; }
	if (aci.cwMostDownStream == -1.0) { return -1; }
	if (aci.lowRegionHeight == -1.0) { return -1; }
	if (aci.lowRegionBaseWidth == -1.0) { return -1; }
	if (aci.upRegionBaseWidth == -1.0) { return -1; }
	if (aci.compoundCSChannelWidthLimit = -1.0) { return -1; }
	if (aci.bankSlopeRight == -1.0) { return -1; }
	if (aci.bankSlopeLeft == -1.0) { return -1; }
	return 1;
}

int isNormalSwsParameters(swsParameters assp)
{
	if (assp.wsid == -1.0) { return -1; }
	if (assp.iniSaturation == -1.0) { return -1; }
	if (assp.minSlopeOF == -1.0) { return -1; }
	if (assp.unSatKType == unSaturatedKType::None) { return -1; }
	if (assp.coefUnsaturatedK == -1.0) { return -1; }
	if (assp.minSlopeChBed == -1.0) { return -1; }
	if (assp.minChBaseWidth == -1.0) { return -1; }
	if (assp.chRoughness == -1.0) { return -1; }
	if (assp.dryStreamOrder == -1) { return -1; }
	if (assp.iniFlow == -1.0) { return -1; }
	if (assp.ccLCRoughness == -1.0) { return -1; }
	if (assp.ccPorosity == -1.0) { return -1; }
	if (assp.ccWFSuctionHead == -1.0) { return -1; }
	if (assp.ccHydraulicK == -1.0) { return -1; }
	if (assp.ccSoilDepth == -1.0) { return -1; }
	if (assp.userSet == 0) { return -1; }
	return 1;
}

watchPointInfo nullWatchPointInfo()
{
	watchPointInfo awp;
	awp.wpName = "";
	awp.wpColX = -1;
	awp.wpRowY = -1;
	return awp;
}


int isNormalWatchPointInfo(watchPointInfo awp)
{
	if (awp.wpName == "") { return -1; };
	if (awp.wpColX == -1) { return -1; };
	if (awp.wpRowY == -1) { return -1; };
	return 1;
}




//int nSWS = prj.wsid.size();
//if (prj.iniSaturation.size() != nSWS) {
//	writeLog(fpnLog, "The number of [IniSaturation] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.minSlopeOF.size() != nSWS) {
//	writeLog(fpnLog, "The number of [MinSlopeOF] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.unSatKType.size() != nSWS) {
//	writeLog(fpnLog, "The number of [UnsaturatedKType] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.coefUnsaturatedK.size() != nSWS) {
//	writeLog(fpnLog, "The number of [CoefUnsaturatedK] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.minSlopeChBed.size() != nSWS) {
//	writeLog(fpnLog, "The number of [MinSlopeChBed] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.minChBaseWidth.size() != nSWS) {
//	writeLog(fpnLog, "The number of [MinChBaseWidth] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.chRoughness.size() != nSWS) {
//	writeLog(fpnLog, "The number of [ChRoughness] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.dryStreamOrder.size() != nSWS) {
//	writeLog(fpnLog, "The number of [DryStreamOrder] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.iniFlow.size() != nSWS) {
//	writeLog(fpnLog, "The number of [IniFlow] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.ccLCRoughness.size() != nSWS) {
//	writeLog(fpnLog, "The number of [CalCoefLCRoughness] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.ccPorosity.size() != nSWS) {
//	writeLog(fpnLog, "The number of [CalCoefPorosity] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.ccWFSuctionHead.size() != nSWS) {
//	writeLog(fpnLog, "The number of [CalCoefWFSuctionHead] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.ccHydraulicK.size() != nSWS) {
//	writeLog(fpnLog, "The number of [CalCoefHydraulicK] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.ccSoilDepth.size() != nSWS) {
//	writeLog(fpnLog, "The number of [CalCoefSoilDepth] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}
//if (prj.userSet.size() != nSWS) {
//	writeLog(fpnLog, "The number of [UserSet] element is not equal to the number of subwatershed.\n", 1, 1);
//	return -1;
//}