
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
	if (prj.lcDataType == fileOrConstant::File)
	{
		readLandCoverFileAndSetCvLcByVAT();
	}
	else if (prj.lcDataType == fileOrConstant::Constant) {
		setCvLcByConstant();
		}


	//readSlopeFile();
	//readFdirFile();
	//readFacFile();
	
	//    cReadGeoFileAndSetInfo.ReadLayerFdir(row.FlowDirectionFile, WSCells, watershed.colCount, watershed.rowCount, watershed.mFDType, sThisSimulation.IsParallel);
	//    cReadGeoFileAndSetInfo.ReadLayerFAcc(row.FlowAccumFile, WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel);
	//    string FPNstream = "";
	//    string FPNchannelWidth = "";
	//    string FPNiniSSR = "";
	//    string FPNiniChannelFlow = "";
	//    string FPNlc = "";
	//    string FPNst = "";
	//    string FPNsd = "";
	//    if (row.IsStreamFileNull() == false && File.Exists(row.StreamFile) == true)
	//    {
	//        FPNstream = row.StreamFile;
	//        if (cReadGeoFileAndSetInfo.ReadLayerStream(row.StreamFile,
	//            WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
	//        {
	//            cGRM.writelogAndConsole(string.Format("Some errors were occurred while reading stream file.. {0}", row.StreamFile.ToString()), true, true);
	//        }
	//    }
	//    if (row.IsChannelWidthFileNull() == false && File.Exists(row.ChannelWidthFile) == true)
	//    {
	//        FPNchannelWidth = row.ChannelWidthFile;
	//        if (cReadGeoFileAndSetInfo.ReadLayerChannelWidth(row.ChannelWidthFile,
	//            WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
	//        {
	//            Console.WriteLine(string.Format("Some errors were occurred while reading channel width file.. {0}", row.ChannelWidthFile.ToString()), true, true);
	//        }
	//    }
	//    if (row.IsInitialSoilSaturationRatioFileNull() == false && File.Exists(row.InitialSoilSaturationRatioFile) == true)
	//    {
	//        FPNiniSSR = row.InitialSoilSaturationRatioFile;
	//        if (cReadGeoFileAndSetInfo.ReadLayerInitialSoilSaturation(row.InitialSoilSaturationRatioFile,
	//            WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
	//        {
	//            cGRM.writelogAndConsole(string.Format("Some errors were occurred while reading initial soil saturation file.. {0}",
	//                row.InitialSoilSaturationRatioFile.ToString()), true, true);
	//        }
	//    }
	//    if (row.IsInitialChannelFlowFileNull() == false && File.Exists(row.InitialChannelFlowFile) == true)
	//    {
	//        FPNiniChannelFlow = row.InitialChannelFlowFile;
	//        if (cReadGeoFileAndSetInfo.ReadLayerInitialChannelFlow(row.InitialChannelFlowFile,
	//            WSCells, watershed.colCount, watershed.rowCount,
	//            sThisSimulation.IsParallel) == false)
	//        {
	//            cGRM.writelogAndConsole(string.Format("Some errors were occurred while reading initial channel flow file.. {0}",
	//                row.InitialChannelFlowFile.ToString()), true, true);
	//        }
	//    }
	//
	//    if (landcover.mLandCoverDataType.Equals(cGRM.FileOrConst.File) && row.IsLandCoverFileNull() == false && File.Exists(row.LandCoverFile))
	//    {
	//        if (!landcover.IsSet) { return false; }
	//        FPNlc = row.LandCoverFile;
	//        if (cReadGeoFileAndSetInfo.ReadLandCoverFileAndSetVAT(row.LandCoverFile, landcover,
	//            WSCells, watershed.colCount, watershed.rowCount,
	//            sThisSimulation.IsParallel) == false)
	//        {
	//            return false;
	//        }
	//    }
	//    else if (cReadGeoFileAndSetInfo.SetLandCoverAttUsingConstant(landcover,
	//        WSCells, watershed.colCount, watershed.rowCount, sThisSimulation.IsParallel) == false)
	//    {
	//        return false;
	//    }
	//
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