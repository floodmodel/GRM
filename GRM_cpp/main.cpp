
#include <iostream>
#include <stdio.h>
//#include <stdlib.h>
//#include <conio.h>
#include <time.h>
#include <io.h>
//#include <thread>
#include <filesystem>

#include "gentle.h"
#include "grm.h"

#pragma comment(lib,"version.lib")

using namespace std;
namespace fs = std::filesystem;

fs::path fpn_prj;
fs::path fpn_log;
fs::path fp_prj;

projectFile prj;
//generalEnv ge;
//domaininfo di;
//domainCell** dmcells;
//cvatt* cvs;
//cvattAdd* cvsAA;
//vector<rainfallinfo> rf;
//bcCellinfo* bci;

int main(int argc, char** args)
{
	string exeName = "GRM";
	version grmVersion = getCurrentFileVersion();
	char outString[200];
	sprintf_s(outString, "GRM v.%d.%d.%d. Built in %s.\n", grmVersion.major, grmVersion.minor,
		grmVersion.build, grmVersion.LastWrittenTime);
	printf(outString);

	long elapseTime_Total_sec;
	clock_t  finish_Total, start_Total;
	start_Total = clock();

	if (argc == 1) {
		printf("GRM project file was not entered.");
		grmHelp();
		return -1;
	}
	if (args[1] == "/?" || args[1] == "/help"
		|| args[1] == "/ ?" || args[1] == "/ help") {
		grmHelp();
		return -1;
	}

	int nResult = _access(args[1], 0);

	if (nResult == -1) {
		printf("GRM project file(%s) is invalid.", args[1]);
		return -1;
	}
	else if (nResult == 0) {
		fpn_prj = fs::path(args[1]);
		fp_prj = fpn_prj.parent_path();
		fpn_log = fpn_prj;
		fpn_log = fpn_log.replace_extension(".log");
		writeNewLog(fpn_log, outString, 1, -1);
		if (openPrjAndSetupModel() == -1) {
			writeNewLog(fpn_log, "Model setup failed !!!\n", 1, 1);
			return -1;
		}
		if (runGRM() == -1) {
			writeNewLog(fpn_log, "An error was occurred while simulation...\n", 1, 1);
			return -1;
		}
	}

	finish_Total = clock();
	elapseTime_Total_sec = (long)(finish_Total - start_Total) / CLOCKS_PER_SEC;
	tm ts_total = secToHHMMSS(elapseTime_Total_sec);
	sprintf_s(outString, "Simulation was completed. Run time : %dhrs %dmin %dsec.\n",
		ts_total.tm_hour, ts_total.tm_min, ts_total.tm_sec);
	writeLog(fpn_log, outString, 1, 1);


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


int openPrjAndSetupModel()
{
//	char outString[200];
//	sprintf_s(outString, "G2D was started.\n");
//	writeLog(fpn_log, outString, 1, 1);
//
//	if (openProjectFile() < 0)
//	{
//		sprintf_s(outString, "Open %s was failed.\n", fpn_prj.string().c_str());
//		writeLog(fpn_log, outString, 1, 1);
//		return -1;
//	}
//
//	sprintf_s(outString, "%s project was opened.\n", fpn_prj.string().c_str());
//	writeLog(fpn_log, outString, 1, 1);
//
//	//if (prj.isParallel == 1)
//	//{
//	string usingGPU = "false";
//	if (prj.usingGPU == 1) { usingGPU = "true"; }
//	sprintf_s(outString, "Parallel : true. Max. degree of parallelism : %d. Using GPU : %s\n",
//		prj.maxDegreeOfParallelism, usingGPU.c_str());
//	writeLog(fpn_log, outString, 1, 1);
//	prj.cpusi = getCPUinfo();
//	writeLog(fpn_log, prj.cpusi.infoString, 1, 1);
//	if (prj.usingGPU == 1)
//	{
//		string gpuinfo = getGPUinfo();
//		writeLog(fpn_log, gpuinfo, 1, 1);
//		sprintf_s(outString, "Threshold number of effective cells to convert to GPU calculation : %d\n",
//			prj.effCellThresholdForGPU);
//		writeLog(fpn_log, outString, 1, 1);
//	}
//	//}
//	//else
//	//{
//	//	sprintf_s(outString, "Parallel : false. Using GPU : false\n");
//	//	writeLog(fpn_log, outString, 1, 1);
//	//}
//
//	if (setGenEnv() < 0) {
//		writeLog(fpn_log, "Setting general environment variables was failed.\n", 1, 1);
//		return -1;
//	}
//
//	sprintf_s(outString, "iGS(all cells) max : %d, iNR(a cell) max : %d, tolerance : %f\n",
//		prj.maxIterationAllCellsOnCPU, prj.maxIterationACellOnCPU, ge.convergenceConditionh);
//	writeLog(fpn_log, outString, 1, 1);
//
//	if (setupDomainAndCVinfo() < 0) {
//		writeLog(fpn_log, "Setting domain and control volume data were failed.\n", 1, 1);
//		return -1;
//	}
//	if (prj.isRainfallApplied == 1) {
//		if (setRainfallinfo() == -1) {
//			writeLog(fpn_log, "Setting rainfall data was failed.\n", 1, 1);
//			return -1;
//		}
//	}
//	if (prj.isbcApplied == 1) {
//		if (setBCinfo() == -1) {
//			writeLog(fpn_log, "Setting boundary condition data was failed.\n", 1, 1);
//			return -1;
//		}
//	}
//	if (deleteAlloutputFiles() == -1) {
//		writeLog(fpn_log, "Deleting previous output files was failed.\n", 1, 1);
//		return -1;
//	}
//
//	if (initializeOutputArray() == -1) {
//		writeLog(fpn_log, "Initialize output arrays was failed.\n", 1, 1);
//		return -1;
//	}
//
//	sprintf_s(outString, "%s  -> Model setup was completed.\n", fpn_prj.string().c_str());
//	writeLog(fpn_log, outString, 1, 1);
	return 1;
}

int runGRM()
{
//	writeLog(fpn_log, "Calculation using CPU was started.\n", 1, 1);
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
