
#include "gentle.h"
#include "grm.h"
#include "realTime.h"

using namespace std;


extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern domaininfo di;
extern cvAtt** cellidx;
extern cvAtt* cvs;

int setupModelAfterOpenProjectFile()
{
	if (setBasicCVInfo() == -1) { return -1; }
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

int setBasicCVInfo()
{
	readDomainFileAndSetupCV();
	readSlopeFdirFacStreamCWiniSSRiniCF();
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


	//    if (GreenAmpt.mSoilTextureDataType.Equals(cGRM.FileOrConst.File) && row.IsSoilTextureFileNull() == false && File.Exists(row.SoilTextureFile))
	//    {
	//        if (!GreenAmpt.IsSet) { return false; }
	//        FPNst = row.SoilTextureFile;
	//        if (cReadGeoFileAndSetInfo.ReadSoilTextureFileAndSetVAT(row.SoilTextureFile, GreenAmpt,
	//            WSCells, watershed.colCount, watershed.rowCount,
	//            sThisSimulation.IsParallel) == false)
	//        {
	//            return false;
	//        }
	//    }
	//    else if (cReadGeoFileAndSetInfo.SetSoilTextureAttUsingConstant(GreenAmpt,
	//        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
	//    {
	//        return false;
	//    }
	//
	//    if (soilDepth.mSoilDepthDataType.Equals(cGRM.FileOrConst.File) && row.IsSoilDepthFileNull() == false && File.Exists(row.SoilDepthFile))
	//    {
	//        if (!soilDepth.IsSet)
	//        {
	//            return false;
	//        }
	//        FPNsd = row.SlopeFile;
	//        if (cReadGeoFileAndSetInfo.ReadSoilDepthFileAndSetVAT(row.SoilDepthFile, soilDepth,
	//            WSCells, watershed.colCount, watershed.rowCount,
	//            sThisSimulation.IsParallel) == false)
	//        {
	//            return false;
	//        }
	//    }
	//    else if (cReadGeoFileAndSetInfo.SetSoilDepthAttUsingConstant(soilDepth,
	//        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
	//    {
	//        return false;
	//    }
	//    SetGridNetworkFlowInformation();
	//    InitControlVolumeAttribute();
	//    return true;
	//}

	return 1;
}