#pragma once
#include "grm.h"



//namespace GRM
//{
	//GRMDLL_API class grmWS {
	class grmWS {
	private:
		void setPublicVariables();
		bool byGMPfile = false;
	public:
		cellPosition mostDownStreamCell;
		vector<int> WSIDsAll;
		int WScount = 0;
		vector<int> mostDownStreamWSIDs;
		int cellCountInWatershed = 0;
		double cellSize = 0;

		grmWS(string fdirType, string fpnDomain,
			string slopeFPN, string fpnFdir, string fpnFac,
			string fpnStream = "", string fpnLandCover = "",
			string fpnSoilTexture = "", string fpnSoilDepth = "",
			string fpnIniSoilSaturationRatio = "",
			string pfnIniChannelFlow = "",
			string fpnChannelWidth = "");
		grmWS(string gmpFPN);
		~grmWS();

		bool isInWatershedArea(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		vector<int> upStreamWSIDs(int currentWSID);
		int upStreamWSCount(int currentWSID);
		vector<int> downStreamWSIDs(int currentWSID);
		int downStreamWSCount(int currentWSID);
		int watershedID(int colXAryidx, int rowYAryidx); // 배열 인덱스 사용
		string flowDirection(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		int flowAccumulation(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		double slope(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		int streamValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		string cellFlowType(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		int landCoverValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		int soilTextureValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		int soilDepthValue(int colXAryidx, int rowYAryidx);// 배열 인덱스 사용
		vector<string> allCellsInUpstreamArea(int colXAryidx, //    Select all cells in upstream area of a input cell position. Return string list of cell positions - "column, row".
			int rowYAryidx);

		// If this class was instanced by using gmp file --"grmWS(string gmpFPN)".		
		bool setOneSWSParsAndUpdateAllSWSUsingNetwork(int wsid, double iniSat,
			double minSlopeLandSurface, string unSKType, double coefUnsK,
			double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
			int dryStreamOrder, double ccLCRoughness,
			double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
			double ccSoilHydraulicCond, double iniFlow = 0);
		void updateAllSubWatershedParametersUsingNetwork();
		swsParameters subwatershedPars(int wsid);
		bool removeUserParametersSetting(int wsid);

		//extern "C" __declspec(dllexport)
		int grmPlus(int a, int b);
		int grmMultiple(int a, int b);

	};
//}

	class testClass {
		private:


		public:
			int ain;
			int bin;
			testClass(int a, int b);
			~testClass();
			int grmPlus();
			int grmMultiple();
	};



//#ifdef GRMDLL_EXPORTS
//#define GRMDLL_API __declspec(dllexport)
//#else
//#define GRMDLL_API __declspec(dllimport)
//#endif

//// extern C
//extern "C" GRMDLL_API class  grmWS;
	extern "C" __declspec(dllexport)
		int grmPlusFunction(int a, int b);
	extern "C" {
		__declspec(dllexport) grmWS* grmWS_new(string gmpFPN) {
			return new grmWS(gmpFPN);
		}
		__declspec(dllexport) int  grmWS_plus(grmWS* f, int a, int b) {
			return f->grmPlus(a, b);
		}
		__declspec(dllexport) int grmWS_multi(grmWS* f, int a, int b) {
			return f->grmMultiple(a, b);
		}
	}

	extern "C"
	{

		//testClass* __stdcall testClass_new(int a, int b) {
		//	return new testClass(a, b);
		//}
		//int __stdcall  testClass_plus(testClass* f) {
		//	return f->grmPlus();
		//}
		//int __stdcall  testClass_multi(testClass* f) {
		//	return f->grmMultiple();
		//}


		__declspec(dllexport) testClass* __stdcall testClass_new(int a, int b) {
			return new testClass(a, b);
		}
		__declspec(dllexport) int __stdcall testClass_plus(testClass* f) {
			return f->grmPlus();
		}
		__declspec(dllexport)   INT32 __stdcall testClass_multi(testClass* f) {
			return f->grmMultiple();
		}
		__declspec(dllexport)  int __stdcall ain(testClass* f) {
			return f->ain;
		}
		__declspec(dllexport)  int __stdcall  bin(testClass* f) {
			return f->bin;
		}
		
		//__declspec(dllexport) testClass* testClass_new(int a, int b) {
		//	return new testClass(a, b);
		//}
		//__declspec(dllexport) int testClass_plus(testClass* f) {
		//	return f->grmPlus();
		//}
		//__declspec(dllexport) int testClass_multi(testClass* f) {
		//	return f->grmMultiple();
		//}

		//__declspec(dllexport) int ain(testClass* f) {
		//	return f->ain;
		//}
		//__declspec(dllexport) int bin(testClass* f) {
		//	return f->bin;
		//}
	}