#pragma once
#include "grm.h"

#ifdef GRM_EXPORTS // 전처리기 정의에서 이것 명시해 줘야함.
#define GRMDLL_API __declspec(dllexport)
#else
#define GRMDLL_API __declspec(dllimport)
#endif

extern "C" // for python //grmWSinfo의 내용을  로 재정의 한다. 
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
		return f->upStreamWSIDs(currentWSID);
	}

	GRMDLL_API int upStreamWSCount(grmWSinfo* f,
		int currentWSID)
	{
		return f->upStreamWSCount(currentWSID);
	}

	GRMDLL_API int* downStreamWSIDs(grmWSinfo* f,
		int currentWSID)
	{
		return f->downStreamWSIDs(currentWSID);
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
	GRMDLL_API string flowDirection(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->flowDirection(colXAryidx, rowYAryidx);
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
	GRMDLL_API string cellFlowTypeOfaCell(grmWSinfo* f,
		int colXAryidx, int rowYAryidx)// 배열 인덱스 사용
	{
		return f->cellFlowType(colXAryidx, rowYAryidx);
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

	GRMDLL_API string* allCellsInUpstreamArea(grmWSinfo* f,
		int colXAryidx, 	int rowYAryidx)//    Select all cells in upstream area of a input cell position. Return string list of cell positions - "xCol, yRow".
	{
		return f->allCellsInUpstreamArea(colXAryidx, rowYAryidx);
	}

	// If this class was instanced by using gmp file --"grmWS(string gmpFPN)".		
	GRMDLL_API bool setOneSWSParsAndUpdateAllSWSUsingNetwork(grmWSinfo* f,
		int wsid, double iniSat,
		double minSlopeLandSurface, string unSKType, double coefUnsK,
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
		return f->WSIDsAll;
	}

	GRMDLL_API int WScount(grmWSinfo* f)
	{
		return f->WScount;
	}
		
	GRMDLL_API int* mostDownStreamWSIDs(grmWSinfo* f)
	{
		return f->mostDownStreamWSIDs;
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
	

//// extern "C" sample code
//	class testClass {
//	private:
//	public:
//		int ain;
//		int bin;
//		testClass(int a, int b);
//		~testClass();
//		int grmPlus();
//		int grmMultiple();
//	};
//
//  extern "C" GRMDLL_API int grmPlusFunction(int a, int b);
//	extern "C" // for python
//	{
//		// __stdcall을 꼭 써야 하는 것은 아니다..
//		// __declspec(dllexport) 은 꼭 써야 한다.
//		//__declspec(dllexport) testClass* __stdcall testClass_new(int a, int b) {
//		//	return new testClass(a, b);
//		//}
//		GRMDLL_API testClass* testClass_new(int a, int b) {
//			return new testClass(a, b);
//		}
//		GRMDLL_API int  testClass_plus(testClass* f) {
//			return f->grmPlus();
//		}
//		GRMDLL_API   int testClass_multi(testClass* f) {
//			return f->grmMultiple();
//		}
//		GRMDLL_API  int  ain(testClass* f) {
//			return f->ain;
//		}
//		GRMDLL_API  int   bin(testClass* f) {
//			return f->bin;
//		}
//	}
//
//int grmPlusFunction(int a, int b)	{
//	return a + b;
//}
//testClass::testClass(int a, int b)	{
//	ain = a;
//	bin = b;
//	cout << "a : " << ain << "  b : " << bin << endl;
//}
//testClass::~testClass()	{
//	cout << "end" << endl;
//}
//int  testClass::grmPlus() {
//	cout << "grmPlus" << endl;
//	cout << "a : " << ain << "  b : " << bin << endl;
//	return ain + bin;
//}
//
//int  testClass::grmMultiple() {
//	cout << "grmMultiple" << endl;
//	return (int)(ain * bin);
//}
//// extern "C" sample code