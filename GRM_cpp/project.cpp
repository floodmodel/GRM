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
extern projectFileInfo pfi;
extern fs::path fpnLog;

int openProjectFile()
{
	ifstream prjfile(pfi.fpn_prj);
	if (prjfile.is_open() == false) { return -1; }
	string aline;
	projectFileFieldName fn;
	prj.isDateTimeFormat = -1;
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
		}

		if (aline.find(fn.DomainFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.DomainFile);
			prj.fpnDomain = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnDomain = vString;
			}
			else {
				writeLog(fpnLog, "DEM file " + vString + " is invalid.\n", 1, 1);
				return -1;
			}
		}

		if (aline.find(fn.SlopeFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.SlopeFile);
			prj.fpnSlope = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnSlope = vString;
			}
			else {
				writeLog(fpnLog, "Slope file " + vString + " is invalid.\n", 1, 1);
				return -1;
			}
		}

		if (aline.find(fn.FlowDirectionFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FlowDirectionFile);
			prj.fpnFD = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnFD = vString;
			}
			else {
				writeLog(fpnLog, "Flow direction file " + vString + " is invalid.\n", 1, 1);
				return -1;
			}
		}	

		if (aline.find(fn.FlowAccumFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.FlowAccumFile);
			prj.fpnFA = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnFA = vString;
			}
			else {
				writeLog(fpnLog, "Flow accumulation file " + vString + " is invalid.\n", 1, 1);
				return -1;
			}
		}

		if (aline.find(fn.StreamFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.StreamFile);
			prj.fpnStream = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnStream = vString;
			}
			else {
				writeLog(fpnLog, "Stream file " + vString + " was not set.\n", 1, -1);
			}
		}

		if (aline.find(fn.ChannelWidthFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ChannelWidthFile);
			prj.fpnChannelWidth = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnChannelWidth = vString;
			}
			else {
				writeLog(fpnLog, "Channel width file " + vString + " was not set.\n", 1, -1);
			}
		}

		if (aline.find(fn.InitialSoilSaturationRatioFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.InitialSoilSaturationRatioFile);
			prj.fpnIniSSR = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnIniSSR = vString;
			}
			else {
				writeLog(fpnLog, "Soil saturation ratio file " + vString + " was not set.\n", 1, -1);
			}
		}

		if (aline.find(fn.InitialChannelFlowFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.InitialChannelFlowFile);
			prj.fnpIniChannelFlow = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fnpIniChannelFlow = vString;
			}
			else {
				writeLog(fpnLog, "Soil saturation ratio file " + vString + " was not set.\n", 1, -1);
			}
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
		}
		
		if (aline.find(fn.LandCoverFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LandCoverFile);
			prj.fpnLC = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnLC = vString;
			}
			else {
				writeLog(fpnLog, "Land cover file " + vString + " was not set.\n", 1, -1);
			}
		}

		if (aline.find(fn.LandCoverVatFile) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.LandCoverVatFile);
			prj.fpnLCVat = "";
			if (vString != "" && _access(vString.c_str(), 0) == 0) {
				prj.fpnLCVat = vString;
			}
			else {
				writeLog(fpnLog, "Land cover VAT file " + vString + " was not set.\n", 1, -1);
			}
		}

		if (aline.find(fn.ConstantRoughnessCoeff) != string::npos) {
			vString = getValueStringFromXmlLine(aline, fn.ConstantRoughnessCoeff);
			prj.cnstRoughnessC = 0;
			if (vString != "") {
				prj.cnstRoughnessC = stod(vString);
			}
		}

}


	return 1;
}