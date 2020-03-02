
#include "grm.h"


extern grmOutFiles ofs;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

int initOutputFiles()
{

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

	return 1;

}