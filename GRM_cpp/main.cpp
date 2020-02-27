
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <io.h>
//#include <thread>
#include <string>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

#include "gentle.h"
#include "grm.h"

#pragma comment(lib,"version.lib")

using namespace std;
namespace fs = std::filesystem;

projectfilePathInfo ppi;
fs::path fpnLog;

projectFile prj;
//generalEnv ge;
//domaininfo di;
//domainCell** dmcells;
//cvatt* cvs;
//cvattAdd* cvsAA;
//vector<rainfallinfo> rf;
//bcCellinfo* bci;

string msgToScreen="";

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

	if (argc == 1) {
		printf("GRM project file was not entered or invalid arguments.\n");
		grmHelp();
		return -1;
	}
	prj.deleteAllFilesExceptDischargeOut = -1;
	if (argc == 2) {
		string arg1(args[1]);
		if (trim(arg1) == "/?" || toLower(trim(arg1)) == "/help") {
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
			fpnLog = fs::path(ppi.fpn_prj.c_str()).replace_extension(".log");
			writeNewLog(fpnLog, outString, 1, -1);
			if (startSingleEventRun() == -1) { 
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
			arg1 = toLower(trim(arg1));
			arg2 = toLower(trim(arg2));
			if (arg1 == "/" && (arg2 == "?" || arg2 == "help")) {
				grmHelp();
				return -1;
			}
			if (arg1 == "/f" || arg1 == "/fd") {
				struct stat finfo;
				if (stat(arg2.c_str(), &finfo) == 0) { //������ ������
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
			arg1 = toLower(trim(arg1));
			arg2 = toLower(trim(arg2));
			arg3 = toLower(trim(arg3));
			if (arg1 == "/" && (arg2 == "f" || arg2 == "fd")) {
				struct stat finfo;
				if (stat(arg3.c_str(), &finfo) == 0) { //������ ������
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
			fpnLog = fs::path(ppi.fpn_prj.c_str()).replace_extension(".log");
			writeNewLog(fpnLog, outString, 1, -1);
			string progF = to_string(n + 1) + '/' + to_string(gmpFiles.size());
			string progR = forString(((n + 1) / nFiles * 100), 2);
			msgToScreen = "Total progress: " + progF + "(" + progR + "%%). ";
			if (startSingleEventRun() == -1) {
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
	waitEnterKey();
	disposeDynamicVars();
	return 1;
}



void disposeDynamicVars()
{
//	if (dmcells != NULL)
//	{
//		for (int i = 0; i < di.nCols; ++i)
//		{
//			if (dmcells[i] != NULL) { delete[] dmcells[i]; }
//		}
//	}
//	if (cvs != NULL) { delete[] cvs; }
//	if (cvsAA != NULL) { delete[] cvsAA; }
//	if (bci != NULL) { delete[] bci; }
}


int startSingleEventRun()
{
	if (openPrjAndSetupModel() == -1) {
		writeNewLog(fpnLog, "Model setup failed !!!\n", 1, 1);
		return -1;
	}
	if (runGRM() == -1) {
		writeNewLog(fpnLog, "An error was occurred while simulation...\n", 1, 1);
		return -1;
	}
}

int openPrjAndSetupModel()
{
	writeLog(fpnLog, "GRM was started.\n", 1, 1);
	if (openProjectFile() < 0)	{
		writeLog(fpnLog, "Open "+ ppi.fpn_prj+" was failed.\n", 1, 1);
		return -1;
	}
	writeLog(fpnLog, ppi.fpn_prj+" project was opened.\n", 1, 1);
	string isparallel = "true";
	if (prj.maxDegreeOfParallelism = 1) { isparallel = "false"; }
	writeLog(fpnLog, "Parallel : "+ isparallel +". Max. degree of parallelism : "
		+ to_string(prj.maxDegreeOfParallelism) +".\n", 1, 1);
	prj.cpusi = getCPUinfo();
	writeLog(fpnLog, prj.cpusi.infoString, 1, 1);
	//if (setGenEnv() < 0) {
	//	writeLog(fpnLog, "Setting general environment variables was failed.\n", 1, 1);
	//	return -1;
	//}

	//if (setupDomainAndCVinfo() < 0) {
	//	writeLog(fpnLog, "Setting domain and control volume data were failed.\n", 1, 1);
	//	return -1;
	//}
	//if (prj.isRainfallApplied == 1) {
	//	if (setRainfallinfo() == -1) {
	//		writeLog(fpnLog, "Setting rainfall data was failed.\n", 1, 1);
	//		return -1;
	//	}
	//}
	//if (prj.isbcApplied == 1) {
	//	if (setBCinfo() == -1) {
	//		writeLog(fpnLog, "Setting boundary condition data was failed.\n", 1, 1);
	//		return -1;
	//	}
	//}
	//if (deleteAlloutputFiles() == -1) {
	//	writeLog(fpnLog, "Deleting previous output files was failed.\n", 1, 1);
	//	return -1;
	//}

	//if (initializeOutputArray() == -1) {
	//	writeLog(fpnLog, "Initialize output arrays was failed.\n", 1, 1);
	//	return -1;
	//}

	writeLog(fpnLog, ppi.fpn_prj+"  -> Model setup was completed.\n", 1, 1);
	return 1;
}

int runGRM()
{
	writeLog(fpnLog, "Calculation was started.\n", 1, 1);
//	if (simulationControlUsingCPUnGPU() == -1) { return -1; }
	return 1;
}


void grmHelp()
{
	printf("\n");
	printf(" Usage : GRM [Current project file full path and name to simulate]\n");
	printf("\n");
	printf("** ���� (in Korean)\n");
	printf("  1. GRM ������ project ����(.gmp)�� �Է��ڷ�(���������ڷ�, ����, flow control �ð迭 �ڷ�)�� �غ��Ѵ�. \n");
	printf("  2. �𵨸��� ������Ʈ �̸��� ����ġ(argument)�� �ְ� �����Ѵ�.\n");
	printf("      - Console���� grm.exe [argument] �� �����Ѵ�.\n");
	printf("      ** ���ǻ��� : ��Ⱓ ������ ���, ��ǻ�� ������Ʈ�� ����, ����� �� ������, \n");
	printf("                       ��Ʈ��ũ�� �����ϰ�, �ڵ�������Ʈ ���� �������� �����Ѵ�.\n");
	printf("  3. argument\n");
	printf("      - /?\n");
	printf("          ����\n");
	printf("      - ������Ʈ ���ϰ�ο� �̸�\n");
	printf("         -- GRM ���� ���ϴ����� �����Ų��.\n");
	printf("         -- �̶� full path, name�� �־�� ������, \n");
	printf("             ������Ʈ ������ GRM.exe ���ϰ� ������ ������ ���� ��쿡��,\n");
	printf("             path�� �Է����� �ʾƵ� �ȴ�.\n");
	printf("         -- ������Ʈ �̸��� ��ο� ������ ���Ե� ��� ū����ǥ�� ��� �Է��Ѵ�.\n\n");
	printf("         -- ����(grm.exe�� d://GRMrun�� ���� ���)\n");
	printf("           --- Case1. grm.exe�� �ٸ� ������ ������Ʈ ������ ���� ���\n");
	printf("               D://GRMrun>grm D://GRMTest//TestProject//test.gmp\n");
	printf("           --- Case 2. grm.exe�� ���� ������ ������Ʈ ������ ���� ���\n");
	printf("               D://GRMrun>grm test.gmp\n");
	printf("      - /f �������\n");
	printf("         -- grm�� ���� ������ �����Ų��.\n");
	printf("         -- ���� : D://GRMrun>grm /f d://GRMrun//TestProject\n");
	printf("      - /fd �������\n");
	printf("         -- grm�� ���� ������ �����Ų��.\n");
	printf("         -- ���� ���ǰ���� *discharge.out�� ������ ��� ������ �����(*.gmp, *Depth.out, ��)\n");
	printf("         -- ���� : D://GRMrun>grm /fd d://GRMrun//TestProject\n");
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
	printf("         -- If there are spaces in the project file name or path, quotation marks �� �� are used to enclose it for input.\n\n");
	printf("         -- Example(when grm.exe is in 'd://GRMrun' folder)\n");
	printf("           --- Case1. gmp file is in different folder with grm.exe\n");
	printf("               D://GRMrun>grm D://GRMTest//TestProject//test.gmp\n");
	printf("           --- Case 2. gmp file is in the same folder with grm.exe\n");
	printf("               D://GRMrun>grm test.gmp\n");
	printf("      - /f [folder path]\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- Example : D://GRMrun>grm /f d://GRMrun//TestProject\n");
	printf("      - /fd �������\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- And all files, except for the discharge file (*discharge.out), are deleted (*.gmp, *Depth.out, etc.).\n");
	printf("         -- Example : D://GRMrun>grm /fd d://GRMrun//TestProject\n");
}
