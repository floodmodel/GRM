
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <io.h>
#include <omp.h>
//#include <thread>
#include <string>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

#include "gentle.h"
#include "grm.h"
#include "realTime.h"

#pragma comment(lib,"version.lib")

using namespace std;
namespace fs = std::filesystem;

projectfilePathInfo ppi;
fs::path fpnLog;
projectFile prj;
grmOutFiles ofs;

domaininfo di;
int** cvais; // 각셀의 cv array idex 
cvAtt* cvs;
cvAtt* cvsb;
vector<rainfallData> rfs;
map<int, int*> cvaisToFA; //fa별 cv array idex 목록
vector<int> fas;
map<int, int> faCount;
wpinfo wpis;
flowControlCellAndData fccds;

thisSimulation ts;


string msgFileProcess;

int main(int argc, char** args)
{
	string exeName = "GRM";
	version grmVersion = getCurrentFileVersion();
	char outString[200];
	sprintf_s(outString, "GRM v.%d.%d.%d. Built in %s.\n", 
		grmVersion.major, grmVersion.minor,
		grmVersion.build, grmVersion.LastWrittenTime);
	printf(outString);

	long elapseTime_Total_sec;
	clock_t  finish_Total, start_Total;
	start_Total = clock();
	ts.runByAnalyzer = -1;
	if (argc == 1) {
		printf("GRM project file was not entered or invalid arguments.\n");
		grmHelp();
		return -1;
	}
	prj.deleteAllFilesExceptDischargeOut = -1;
	if (argc == 2) {
		string arg1(args[1]);
		if (trim(arg1) == "/?" || lower(trim(arg1)) == "/help") {
			grmHelp();
			return -1;
		}
		int nResult = _access(args[1], 0);
		if (nResult == -1) {
			printf("GRM project file(%s) is invalid.\n", args[1]);
			waitEnterKey();
			return -1;
		}
		else if (nResult == 0) {
			ppi = getProjectFileInfo(arg1);
			writeNewLog(fpnLog, outString, 1, -1);
			if (simulateSingleEvent() == -1) { 
				waitEnterKey();
				return -1; 
			}
		}
	}

	if (argc == 3 || argc == 4) {
		vector<string> gmpFiles;
		if (argc == 3) {
			string arg1(args[1]);
			string arg2(args[2]);
			arg1 = lower(trim(arg1));
			arg2 = lower(trim(arg2));
			if (arg1 == "/" && (arg2 == "?" || arg2 == "help")) {
				grmHelp();
				return -1;
			}
			if (arg1 == "/f" || arg1 == "/fd") {
				struct stat finfo;
				if (stat(arg2.c_str(), &finfo) == 0) { //폴더가 있으면
					gmpFiles = getFileListInNaturalOrder(arg2, ".gmp");
					if (gmpFiles.size() == 0) {
						printf("There is no GRM project file in this directory.\n");
						waitEnterKey();
						return -1;
					}
					if (arg1 == "/fd") {
						prj.deleteAllFilesExceptDischargeOut = 1;
					}
				}
				else {
					printf("Project folder is invalid!!\n");
					waitEnterKey();
					return -1;
				}
			}
		}
		else if (argc == 4) {
			string arg1(args[1]);
			string arg2(args[2]);
			string arg3(args[3]);
			arg1 = lower(trim(arg1));
			arg2 = lower(trim(arg2));
			arg3 = lower(trim(arg3));
			if (arg1 == "/" && (arg2 == "f" || arg2 == "fd")) {
				struct stat finfo;
				if (stat(arg3.c_str(), &finfo) == 0) { //폴더가 있으면
					gmpFiles = getFileListInNaturalOrder(arg3, ".gmp");
					if (gmpFiles.size() == 0) {
						printf("There is no GRM project file in this directory.\n");
						waitEnterKey();
						return -1;
					}
					if (arg2 == "fd") {
						prj.deleteAllFilesExceptDischargeOut = 1;
					}
				}
				else {
					printf("Project folder is invalid!!\n");
					waitEnterKey();
					return -1;
				}
			}
		}
		int nFiles = gmpFiles.size();
		for (int n = 0; n < nFiles; n++) {
			ppi = getProjectFileInfo(gmpFiles[n]);
			writeNewLog(fpnLog, outString, 1, -1);
			string progF = to_string(n + 1) + '/' + to_string(gmpFiles.size());
			string progR = forString(((n + 1) / nFiles * 100), 2);
			msgFileProcess = "Total progress: " + progF + "(" + progR + "%%). ";
			if (simulateSingleEvent() == -1) {
				waitEnterKey();
				return -1;
			}
			if ((n + 1) % 100 == 0) {
				system("cls"); // On windows
				//system("clear");// On linux
			}
		}
		return 1;
	}

	finish_Total = clock();
	elapseTime_Total_sec = (long)(finish_Total - start_Total) / CLOCKS_PER_SEC;
	tm ts_total = secToHHMMSS(elapseTime_Total_sec);
	sprintf_s(outString, "Simulation was completed. Run time : %dhrs %dmin %dsec.\n",
		ts_total.tm_hour, ts_total.tm_min, ts_total.tm_sec);
	writeLog(fpnLog, outString, 1, 1);
	//waitEnterKey();
	disposeDynamicVars();
	return 1;
}



void disposeDynamicVars()
{
	if (cvais != NULL) {
		for (int i = 0; i < di.nCols; ++i) {
			if (cvais[i] != NULL) { delete[] cvais[i]; }
		}
	}
	if (cvs != NULL) { delete[] cvs; }
	if (cvsb != NULL) { delete[] cvsb; }

	if (cvaisToFA.size() > 0) {
		map<int, int*>::iterator iter;
		//map<int, int*> cvansTofa; //fa별 cvan 목록
		for (iter = cvaisToFA.begin(); iter != cvaisToFA.end(); ++iter) {
			if (cvaisToFA[iter->first] != NULL) {
				delete[] cvaisToFA[iter->first];
			}				
		}
	}

}


int simulateSingleEvent()
{
	if (openPrjAndSetupModel(-1) == -1) {
		writeLog(fpnLog, "Model setup failed !!!\n", 1, 1);
		return -1;
	}
	writeLog(fpnLog, "Simulation was started.\n", 1, 1);
	if (startSimulationSingleEvent() == -1) {
		writeNewLog(fpnLog, "An error was occurred while simulation...\n", 1, 1);
		return -1;
	}
	if (prj.deleteAllFilesExceptDischargeOut == 1) {
		if (deleteAllFilesExceptDischarge() == -1) {
			writeNewLog(fpnLog, "An error was occurred while deleting all files except discharge.out.\n", 1, 1);
			return -1;
		}
	}
	return 1;
}

int openPrjAndSetupModel(int forceRealTime) // 1:true, -1:false
{	
	prj.cpusi = getCPUinfo();
	writeLog(fpnLog, prj.cpusi.infoString, 1, 1);
	if (openProjectFile(forceRealTime) < 0)	{
		writeLog(fpnLog, "Open "+ ppi.fpn_prj+" was failed.\n", 1, 1);
		return -1;
	}
	//writeLog(fpnLog, "GRM was started.\n", 1, 1);
	writeLog(fpnLog, ppi.fpn_prj+" project was opened.\n", 1, 1);
	if (setupModelAfterOpenProjectFile() == -1) {
		return -1;
	}
	string isparallel = "true";
	omp_set_num_threads(prj.mdp);

	//todo : 여기에 셀 개수 조건으로 mdp 설정 추가 필요
	//위천의 경우, 11734인데, 병렬계산이 쪼매 더 느리다..
	// 금호강의 경우, 8418인데, 병렬계산이 쪼매 더 느리다..
	//if (di.cellNnotNull < 12000) {
	//	writeLog(fpnLog, "The number of effective cell [ "
	//		+ to_string(di.cellNnotNull) + "] is smaller than 12,000.\n"
	//		+ "It was converted to serial calculation. \n", 1, 1);
	//	prj.mdp = 1;
	//	isparallel = "false";
	//}

	if (prj.mdp == 1) { isparallel = "false"; }
	writeLog(fpnLog, "Parallel : "+ isparallel +". Max. degree of parallelism : "
		+ to_string(prj.mdp) +".\n", 1, 1);
	if (initOutputFiles() == -1) {
		writeLog(fpnLog, "Initializing output files was failed.\n", 1, 1);
		return -1;
	}
	writeLog(fpnLog, ppi.fpn_prj+"  -> Model setup was completed.\n", 1, 1);
	writeLog(fpnLog, "The number of effecitve cells : "+to_string(di.cellNnotNull)+"\n", 1, 1); 
	return 1;
}


void grmHelp()
{
	printf("\n");
	printf(" Usage : GRM [Current project file full path and name to simulate]\n");
	printf("\n");
	printf("** 사용법 (in Korean)\n");
	printf("  1. GRM 모형의 project 파일(.gmp)과 입력자료(지형공간자료, 강우, flow control 시계열 자료)를 준비한다. \n");
	printf("  2. 모델링할 프로젝트 이름을 스위치(argument)로 넣고 실행한다.\n");
	printf("      - Console에서 grm.exe [argument] 로 실행한다.\n");
	printf("      ** 주의사항 : 장기간 모의할 경우, 컴퓨터 업데이트로 인해, 종료될 수 있으니, \n");
	printf("                       네트워크를 차단하고, 자동업데이트 하지 않음으로 설정한다.\n");
	printf("  3. argument\n");
	printf("      - /?\n");
	printf("          도움말\n");
	printf("      - 프로젝트 파일경로와 이름\n");
	printf("         -- GRM 모델을 파일단위로 실행시킨다.\n");
	printf("         -- 이때 full path, name을 넣어야 하지만, \n");
	printf("             프로젝트 파일이 GRM.exe 파일과 동일한 폴더에 있을 경우에는,\n");
	printf("             path는 입력하지 않아도 된다.\n");
	printf("         -- 프로젝트 이름과 경로에 공백이 포함될 경우 큰따옴표로 묶어서 입력한다.\n\n");
	printf("         -- 예문(grm.exe가 d://GRMrun에 있을 경우)\n");
	printf("           --- Case1. grm.exe와 다른 폴더에 프로젝트 파일이 있을 경우\n");
	printf("               D://GRMrun>grm D://GRMTest//TestProject//test.gmp\n");
	printf("           --- Case 2. grm.exe와 같은 폴더에 프로젝트 파일이 있을 경우\n");
	printf("               D://GRMrun>grm test.gmp\n");
	printf("      - /f 폴더경로\n");
	printf("         -- grm을 폴더 단위로 실행시킨다.\n");
	printf("         -- 예문 : D://GRMrun>grm /f d://GRMrun//TestProject\n");
	printf("      - /fd 폴더경로\n");
	printf("         -- grm을 폴더 단위로 실행시킨다.\n");
	printf("         -- 유량 모의결과인 *discharge.out을 제외한 모든 파일을 지운다(*.gmp, *Depth.out, 등)\n");
	printf("         -- 예문 : D://GRMrun>grm /fd d://GRMrun//TestProject\n");
	printf("\n");
	printf("** Usage (in English)\n");
	printf("  1. Make a gmp file by using a text editor or the QGIS-GRM.\n");
	printf("  2. In the console window, the gmp file is entered as the argument to run GRM.exe.\n");
	printf("      - In Console : grm.exe [argument]\n");
	printf("  3. argument\n");
	printf("      - /?\n");
	printf("          Help\n");
	printf("      - Project file path and name\n");
	printf("         -- Run the GRM model for a single gmp file.\n");
	printf("         -- When the GRM.exe and gmp files are in the same folder,\n");
	printf("             the project file path does not have to be entered.\n");
	printf("         -- If there are spaces in the project file name or path, quotation marks “ ” are used to enclose it for input.\n\n");
	printf("         -- Example(when grm.exe is in 'd://GRMrun' folder)\n");
	printf("           --- Case1. gmp file is in different folder with grm.exe\n");
	printf("               D://GRMrun>grm D://GRMTest//TestProject//test.gmp\n");
	printf("           --- Case 2. gmp file is in the same folder with grm.exe\n");
	printf("               D://GRMrun>grm test.gmp\n");
	printf("      - /f [folder path]\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- Example : D://GRMrun>grm /f d://GRMrun//TestProject\n");
	printf("      - /fd 폴더경로\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- And all files, except for the discharge file (*discharge.out), are deleted (*.gmp, *Depth.out, etc.).\n");
	printf("         -- Example : D://GRMrun>grm /fd d://GRMrun//TestProject\n");
}
