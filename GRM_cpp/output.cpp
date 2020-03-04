
#include "grm.h"
#include "realTime.h"


extern projectFile prj;
extern grmOutFiles ofs;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

int initOutputFiles()
{
	int isnormal = -1;
	ofs.ofnpDischarge= ppi.fp_prj +"\\"+ ppi.fn_withoutExt_prj+CONST_TAG_DISCHARGE;
	ofs.ofpnDepth = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_DEPTH;
	ofs.ofpnRFGrid = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RFGRID;
	ofs.ofpnRFMean = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_RFMEAN;
	//ofs.OFNPSwsPars = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_SWSPARSTEXTFILE;
	//ofs.OFNPFCData = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FCAPP;
	ofs.OFNPFCStorage = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + CONST_TAG_FCSTORAGE;
	ofs.OFPSSRDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj+"_" + CONST_DIST_SSR_DIRECTORY_TAG;
	ofs.OFPRFDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "_" + CONST_DIST_RF_DIRECTORY_TAG;
	ofs.OFPRFAccDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "_" + CONST_DIST_RFACC_DIRECTORY_TAG;
	ofs.OFPFlowDistribution = ppi.fp_prj + "\\" + ppi.fn_withoutExt_prj + "_" + CONST_DIST_FLOW_DIRECTORY_TAG;
	isnormal = 1;
	return isnormal;
}

int changeOutputFileDisk(char targetDisk)
{
	int isnormal = -1;
	ofs.ofnpDischarge = IO_Path_ChangeDrive(targetDisk, ofs.ofnpDischarge);
	ofs.ofpnDepth = IO_Path_ChangeDrive(targetDisk, ofs.ofpnDepth);
	ofs.ofpnRFGrid = IO_Path_ChangeDrive(targetDisk, ofs.ofpnRFGrid);
	ofs.ofpnRFMean = IO_Path_ChangeDrive(targetDisk, ofs.ofpnRFMean);
	//ofs.OFNPSwsPars = IO_Path_ChangeDrive(targetDisk, ofs.OFNPSwsPars);
	//ofs.OFNPFCData = IO_Path_ChangeDrive(targetDisk, ofs.OFNPFCData);
	ofs.OFNPFCStorage = IO_Path_ChangeDrive(targetDisk, ofs.OFNPFCStorage);
	ofs.OFPSSRDistribution = IO_Path_ChangeDrive(targetDisk, ofs.OFPSSRDistribution);
	ofs.OFPRFDistribution = IO_Path_ChangeDrive(targetDisk, ofs.OFPRFDistribution);
	ofs.OFPRFAccDistribution = IO_Path_ChangeDrive(targetDisk, ofs.OFPRFAccDistribution);
	ofs.OFPFlowDistribution = IO_Path_ChangeDrive(targetDisk, ofs.OFPFlowDistribution);
	isnormal = 1;
	return isnormal;
}