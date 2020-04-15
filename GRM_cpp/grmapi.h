#pragma once

#include "gentle.h"
#include "grm.h"

#ifdef GRM_EXPORTS // 전처리기 정의에서 이것 명시해 줘야함.
#define GRMDLL_API __declspec(dllexport)
#else
#define GRMDLL_API __declspec(dllimport)
#endif

extern "C" // for python //grmWSinfo의 내용을  재정의 한다. 
{
	GRMDLL_API grmWSinfo* grmWSinfo_new_inputFiles(string fdirType, 
		string fpnDomain,
		string slopeFPN, string fpnFdir, string fpnFac,
		string fpnStream = "", string fpnLandCover = "",
		string fpnSoilTexture = "", string fpnSoilDepth = "",
		string fpnIniSoilSaturationRatio = "",
		string pfnIniChannelFlow = "",
		string fpnChannelWidth = "")
	{
		return new grmWSinfo(fdirType, fpnDomain,
			slopeFPN, fpnFdir, fpnFac,
			fpnStream = "", fpnLandCover = "",
			fpnSoilTexture = "", fpnSoilDepth = "",
			fpnIniSoilSaturationRatio = "",
			pfnIniChannelFlow = "",
			fpnChannelWidth = "");
	}

	GRMDLL_API grmWSinfo* grmWSinfo_new_gmpFile(char * gmpFPN)
	{
		return new grmWSinfo(gmpFPN);
	}

	GRMDLL_API bool isInWatershedArea(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->isInWatershedArea(colXAryidx, rowYAryidx);
	}

	GRMDLL_API int* upStreamWSIDs(grmWSinfo* f,
		int currentWSID)
	{
		vector <int> r = f->upStreamWSIDs(currentWSID);
		int* rv = new int[r.size()];
		copy(r.begin(), r.end(), rv);
		return rv;
	}

	GRMDLL_API int upStreamWSCount(grmWSinfo* f,
		int currentWSID)
	{
		return f->upStreamWSCount(currentWSID);
	}

	GRMDLL_API int* downStreamWSIDs(grmWSinfo* f,
		int currentWSID)
	{
		vector <int> r = f->downStreamWSIDs(currentWSID);
		int* rv = new int[r.size()];
		copy(r.begin(), r.end(), rv);
		return rv;
	}

	GRMDLL_API int downStreamWSCount(grmWSinfo* f,
		int currentWSID)
	{
		return f->downStreamWSCount(currentWSID);
	}

	GRMDLL_API int watershedID(grmWSinfo* f,
		int colXAryidx, int rowYAryidx) // 배열 인덱스 사용
	{
		return f->watershedID(colXAryidx, rowYAryidx);
	}

	GRMDLL_API char * flowDirection(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		string r = f->flowDirection(colXAryidx, rowYAryidx);
		return stringToCharP(r);
	}

	GRMDLL_API int flowAccumulation(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->flowAccumulation(colXAryidx, rowYAryidx);
	}
	GRMDLL_API double slope(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->slope(colXAryidx, rowYAryidx);
	}
	GRMDLL_API int streamValue(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->streamValue(colXAryidx, rowYAryidx);
	}
	GRMDLL_API char * cellFlowTypeACell(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		string r = f->cellFlowType(colXAryidx, rowYAryidx);
		return stringToCharP(r);
	}
	GRMDLL_API int landCoverValue(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->landCoverValue(colXAryidx, rowYAryidx);
	}
	GRMDLL_API int soilTextureValue(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->soilTextureValue(colXAryidx, rowYAryidx);
	}

	GRMDLL_API int soilDepthValue(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->soilDepthValue(colXAryidx, rowYAryidx);
	}

	GRMDLL_API char ** allCellsInUpstreamArea(grmWSinfo* f,
		int colXAryidx, 	int rowYAryidx)//    Select all cells in upstream area of a input cell position. Return string list of cell positions - "xCol, yRow".
	{
		vector <string> rv = f->allCellsInUpstreamArea_Array(colXAryidx, rowYAryidx);
		char** r;
		r = new char* [rv.size()];
		for (int i = 0; i < rv.size(); ++i) {
			r[i] = stringToCharP(rv[i]);
		}
		return r;
	}

	GRMDLL_API int cellCountInUpstreamArea(grmWSinfo* f,  //  The number of all cells in upstream area of a input cell position. Return string list of cell positions - "column, row".
		int colXAryidx, int rowYAryidx)
	{
		return f->cellCountInUpstreamArea(colXAryidx, rowYAryidx);
	}

	// If this class was instanced by using gmp file --"grmWS(string gmpFPN)".		
	GRMDLL_API bool setOneSWSParsAndUpdateAllSWSUsingNetwork(grmWSinfo* f,
		int wsid, double iniSat,
		double minSlopeLandSurface, unSaturatedKType unSKType, double coefUnsK,
		double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
		int dryStreamOrder, double ccLCRoughness,
		double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
		double ccSoilHydraulicCond, double iniFlow = 0)
	{
		return f->setOneSWSParsAndUpdateAllSWSUsingNetwork(wsid, iniSat,
			minSlopeLandSurface, unSKType, coefUnsK,
			minSlopeChannel, minChannelBaseWidth, roughnessChannel,
			dryStreamOrder, ccLCRoughness,
			ccSoilDepth, ccPorosity, ccWFSuctionHead,
			ccSoilHydraulicCond, iniFlow = 0);
	}

	GRMDLL_API void updateAllSubWatershedParametersUsingNetwork(grmWSinfo* f)
	{
		return f->updateAllSubWatershedParametersUsingNetwork();
	}

	GRMDLL_API swsParameters subwatershedPars(grmWSinfo* f,
		int wsid)
	{
		return f->subwatershedPars(wsid);
	}

	GRMDLL_API bool removeUserParametersSetting(grmWSinfo* f,
		int wsid)
	{
		return f->removeUserParametersSetting(wsid);
	}

	// Properties
	GRMDLL_API int facMaxCellxCol(grmWSinfo* f)
	{
		return f->facMaxCellxCol;
	}
	GRMDLL_API int facMaxCellyRow(grmWSinfo* f)
	{
		return f->facMaxCellyRow;
	}
	GRMDLL_API int* WSIDsAll(grmWSinfo* f)
	{
		vector <int> r = f->WSIDsAll;
		int * rv = new int[r.size()];
		copy(r.begin(), r.end(), rv);
		return rv;
	}

	GRMDLL_API int WScount(grmWSinfo* f)
	{
		return f->WScount;
	}
		
	GRMDLL_API int* mostDownStreamWSIDs(grmWSinfo* f)
	{
		vector <int> r = f->mostDownStreamWSIDs;
		int* rv = new int[r.size()];
		copy(r.begin(), r.end(), rv);
		return rv;
	}

	GRMDLL_API int mostDownStreamWSCount(grmWSinfo* f)
	{
		return f->mostDownStreamWSCount;
	}


	GRMDLL_API int cellCountInWatershed(grmWSinfo* f)
	{
		return f->cellCountInWatershed;
	}

	GRMDLL_API double cellSize(grmWSinfo* f)
	{
		return f->cellSize;
	}
}
	
