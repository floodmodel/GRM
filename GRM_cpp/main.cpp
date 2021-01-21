
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <io.h>
#include <omp.h>
#include <string>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale>

#include "gentle.h"
#include "grm.h"
#include "realTime.h"
#include "grmapi.h"

#pragma comment(lib,"version.lib")

using namespace std;
namespace fs = std::filesystem;

projectfilePathInfo ppi;
fs::path fpnLog;
projectFile prj;
grmOutFiles ofs;

domaininfo di;
int** cvais; // ����(x,y)�� cv array idex 
cvpos* cvps;//�� cv�� x, y ��ġ, ws ��ġ
cvAtt* cvs;
cvAtt* cvsb;
vector<rainfallData> rfs;
map<int, int*> cvaisToFA; //fa�� cv array idex ���
vector<int> fas;
map<int, int> faCount;
wpinfo wpis;
flowControlCellAndData fccds;

thisSimulation ts;


extern double** ssrAry;
extern double** rfAry;
extern double** rfaccAry;
extern double** QAry;
//extern double** ssrAryL;
//extern double** rfAryL;
//extern double** rfaccAryL;
//extern double** QAryL;

string msgFileProcess;

int main(int argc, char** args)
{
	string exeName = "GRM";
	version grmVersion = getCurrentFileVersion();
	string outString;
	outString = "GRM v." + to_string(grmVersion.major) + "."
		+ to_string(grmVersion.minor) + "."
		+ to_string(grmVersion.build) + ". Modified in "
		+ grmVersion.LastWrittenTime + ".\n";
	prj.cpusi = getCPUinfo();
	cout << outString;
	cout << prj.cpusi.infoString;
	outString = outString + prj.cpusi.infoString;
	ts.runByAnalyzer = -1;
	if (argc == 1) {
		printf("GRM project file was not entered or invalid arguments.\n");
		grmHelp();
		return 1;
	}
	prj.deleteAllFilesExceptDischargeOut = -1;
	setlocale(LC_ALL, "korean");
	prj.writeConsole = 1; // exe�� �����ϴ� ���� 1
	prj.forSimulation = 1;// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
	if (argc == 2) {
		string arg1(args[1]);
		string arg1L = lower(trim(arg1));
		if (arg1L == "/?" || arg1L == "/help") {
			grmHelp();
			return 1;
		}
		// �̰��� grm.exe  fpn_gmp �� ���
		startSingleRun(arg1, -1, outString);
	}
	else {
		string rtOption1(args[1]);
		string rtOption2(args[2]);
		rtOption1 = lower(trim(rtOption1));
		rtOption2 = lower(trim(rtOption2));
		int isRealTime = -1;
		if (rtOption1 == "/r" || rtOption2 == "/r") {
			// �ǽð� �����ڷ� ���� �ɼ��� /r 
			// �ִ� �ɼ��� �Ʒ��� ����. 
			// args[0] : grm.exe,               args[1] : /r Ȥ�� /a,   args[2] : /r Ȥ�� /a,          
			// args[3] : fpnRef,                 args[4] : strGUID,       args[5] : startCommandTime, 
			// args[6] : rtStartDataTime, agrs[7] : strMODEL
			isRealTime = 1;
			string arg1(args[1]); // /r Ȥ�� /a
			string arg2(args[2]); // /r Ȥ�� /a
			//string arg3(args[3]); // ref ���� ���, �̸�
			arg1 = lower(trim(arg1));
			arg2 = lower(trim(arg2));
			//arg3 = lower(trim(arg3));
			int isForceAutoROM = -1;
			if (arg1 == "/a" || arg2 == "/a") {
				isForceAutoROM = 1;
			}
			if (grmRTLauncher(argc, args, isForceAutoROM) == -1) {
				return 1;
			}
		}

		if (isRealTime == -1 && argc == 3) {
			// �̰��� /a, /f, /fd �� �ϳ��� �ɼ��� ���� ��� 
			string arg1(args[1]); // �̰� �ɼ�
			string arg2(args[2]); // �̰� gmp ����, Ȥ�� gmps ����
			arg1 = lower(trim(arg1));
			arg2 = trim(arg2);
			if (arg1 == "/a") {
				startSingleRun(arg2, 1, outString);
			}
			if (arg1 == "/f" || arg1 == "/fd") {
				struct stat finfo;
				if (stat(arg2.c_str(), &finfo) == 0) { //������ ������
					vector<string> gmpFiles;
					gmpFiles = getFileListInNaturalOrder(arg2, ".gmp");
					if (gmpFiles.size() == 0) {
						printf("There is no GRM project file in this directory.\n");
						waitEnterKey();
						return -1;
					}
					prj.deleteAllFilesExceptDischargeOut = -1;
					if (arg1 == "/fd") {
						prj.deleteAllFilesExceptDischargeOut = 1;
					}
					startGMPsRun(gmpFiles, -1, outString);
					return 1;
				}
				else {
					printf("Project folder is invalid!!\n");
					waitEnterKey();
					return -1;
				}
			}
		}

		if (isRealTime == -1 && argc == 4) {
			// �̰��� /a, /f Ȥ�� /a, /fd �ɼ��� ���� ��� 
			string arg1(args[1]); // /a, /f , /fd �ɼ�
			string arg2(args[2]);// /a, /f , /fd �ɼ�
			string arg3(args[3]);// gmps ����
			arg1 = lower(trim(arg1));
			arg2 = lower(trim(arg2));
			arg3 = trim(arg3);
			int isP = -1;
			if (arg1 == "/a" || arg2 == "/a") {
				isP = 1;
			}
			prj.deleteAllFilesExceptDischargeOut = -1;
			if (arg1 == "/fd" || arg2 == "/fd") {
				prj.deleteAllFilesExceptDischargeOut = 1;
			}
			struct stat finfo;
			if (stat(arg3.c_str(), &finfo) == 0) { //������ ������
				vector<string> gmpFiles;
				gmpFiles = getFileListInNaturalOrder(arg3, ".gmp");
				if (gmpFiles.size() == 0) {
					printf("There is no GRM project file in this directory.\n");
					waitEnterKey();
					return -1;
				}
				startGMPsRun(gmpFiles, isP, outString);
				return 1;
			}
			else {
				printf("Project folder is invalid!!\n");
				waitEnterKey();
				return -1;
			}
		}
	}
	disposeDynamicVars();
	return 1;
}

int startSingleRun(string fpnGMP, int isPrediction, string outString)
{
	fs::path in_arg = fs::path(fpnGMP.c_str());
	string fp = in_arg.parent_path().string();
	if (trim(fp) == "") {
		string fpn_exe = getCurrentExeFilePathName();
		fs::path grmexef = fs::path(fpn_exe.c_str());
		string fp_exe = grmexef.parent_path().string();
		fpnGMP = fp_exe + "\\" + fpnGMP;
	}
	int nResult = _access(fpnGMP.c_str(), 0);
	if (nResult == -1
		|| lower(in_arg.extension().string()) != ".gmp") {
		cout<<"GRM project file["+
			fpnGMP+"] is invalid.\n";
		waitEnterKey();
		return -1;
	}
	else if (nResult == 0) {
		ppi = getProjectFileInfo(fpnGMP);
		ts.enforceFCautoROM = isPrediction;
		writeNewLog(fpnLog, outString, 1, -1);
		if (setupAndStartSimulation() == -1) {
			waitEnterKey();
			return -1;
		}
	}
}

int startGMPsRun(vector<string> gmpFiles, int isPrediction, string outString)
{
	clock_t  startT = clock();
	int nFiles = gmpFiles.size();
	ts.enforceFCautoROM = isPrediction;
	for (int n = 0; n < nFiles; n++) {// /f Ȥ�� /fd �� ��� ���⼭ ����
		ppi = getProjectFileInfo(gmpFiles[n]);
		writeNewLog(fpnLog, outString, 1, -1);
		string progF = to_string(n + 1) + '/' + to_string(gmpFiles.size());
		string progR = dtos(((n + 1) / nFiles * 100), 2);
		msgFileProcess = "Total progress: " + progF + "(" + progR + "%). ";
		if (setupAndStartSimulation() == -1) {
			waitEnterKey();
			return -1;
		}
		if ((n + 1) % 100 == 0) {
			system("cls");
		}
		disposeDynamicVars();
	}
	if (prj.deleteAllFilesExceptDischargeOut != 1) {
		clock_t finishT = clock();
		long elapseT_sec = (long)(finishT - startT) / CLOCKS_PER_SEC;
		tm ts_total = secToHHMMSS(elapseT_sec);
		string endingStr = "Total simulation was completed. Run time : "
			+ to_string(ts_total.tm_hour) + "hrs "
			+ to_string(ts_total.tm_min) + "min "
			+ to_string(ts_total.tm_sec) + "sec.\n";
		writeLog(fpnLog, endingStr, 1, 1);
	}
	return 1;
}

void disposeDynamicVars()
{
	if (cvais != NULL) {
		for (int i = 0; i < di.nCols; ++i) {
			if (cvais[i] != NULL) { delete[] cvais[i]; }
		}
		delete[] cvais;
	}
	if (cvps != NULL) { delete[] cvps; }
	if (cvs != NULL) { delete[] cvs; }
	if (cvsb != NULL) { delete[] cvsb; }

	if (cvaisToFA.size() > 0) {
		map<int, int*>::iterator iter;
		//map<int, int*> cvansTofa; //fa�� cvan ���
		for (iter = cvaisToFA.begin(); iter != cvaisToFA.end(); ++iter) {
			if (cvaisToFA[iter->first] != NULL) {
				delete[] cvaisToFA[iter->first];
			}				
		}
	}

	if (ssrAry != NULL) {
		for (int i = 0; i < di.nCols; ++i) {
			if (ssrAry[i] != NULL) { delete[] ssrAry[i]; }
		}
		delete[] ssrAry;
	}
	if (rfAry != NULL) {
		for (int i = 0; i < di.nCols; ++i) {
			if (rfAry[i] != NULL) { delete[] rfAry[i]; }
		}
		delete[] rfAry;
	}
	if (rfaccAry != NULL) {
		for (int i = 0; i < di.nCols; ++i) {
			if (rfaccAry[i] != NULL) { delete[] rfaccAry[i]; }
		}
		delete[] rfaccAry;
	}
	if (QAry != NULL) {
		for (int i = 0; i < di.nCols; ++i) {
			if (QAry[i] != NULL) { delete[] QAry[i]; }
		}
		delete[] QAry;
	}
	prj.swps.clear();
	prj.css.clear();
	prj.fcs.clear();
	prj.wps.clear();
	prj.sts.clear();
	prj.sds.clear();
	prj.lcs.clear();

}

int setupAndStartSimulation()
{
	//===== ���⼭ grmWSinfo class  test ===============
	////string fpn = "C://GRM//SampleGHG//GHG500.gmp";
	////string fpn = "D://Github//zTestSet_GRM_SampleWC_cpp//QGIS_GRM_test.gmp";
	////grmWSinfo gws = grmWSinfo(fpn);
	//string fdType = "StartsFromE_TauDEM";
	//string fpn_domain = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_Watershed_c.asc";
	//string fpn_slope = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_Slope_ST.asc";
	//string fpn_fd = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_FDir.asc";
	//string fpn_fa = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_FAc.asc";
	//string fpn_stream = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_Stream.asc";
	//string fpn_lc = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_lc.asc";
	//string fpn_st = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_SoilTexture.asc";
	//string fpn_sd = "D:/Github/zTestSet_GRM_SampleGHG_cpp/watershed/GHG_SoilDepth.asc";

	////string fpn_domain = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiWatershed.asc";
	////string fpn_slope = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/Wi_Slope_ST.ASC";
	////string fpn_fd = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiFDir.ASC";
	////string fpn_fa = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiFAc.asc";
	////string fpn_stream = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiStream6.asc";
	////string fpn_lc = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/wilc200.asc";
	////string fpn_st = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/wistext200.asc";
	////string fpn_sd = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/wisdepth200.asc";

	//string fdType = "StartsFromE_TauDEM";
	//string fpn_domain = "D:/GRM_ex/Nakdong/watershed/ND_Watershed.asc";
	//string fpn_slope = "D:/GRM_ex/Nakdong/watershed/ND_Slope.asc";
	//string fpn_fd = "D:/GRM_ex/Nakdong/watershed/ND_Fdr.asc";
	//string fpn_fa = "D:/GRM_ex/Nakdong/watershed/ND_Fac.asc";
	//string fpn_stream = "D:/GRM_ex/Nakdong/watershed/ND_Stream.asc";
	//string fpn_lc = "D:/GRM_ex/Nakdong/watershed/ND_LC.asc";
	//string fpn_st = "D:/GRM_ex/Nakdong/watershed/ND_STexture.asc";
	//string fpn_sd = "D:/GRM_ex/Nakdong/watershed/ND_SDepth.asc";

	//grmWSinfo	gws = grmWSinfo(fdType, fpn_domain,	fpn_slope, fpn_fd,
	//	fpn_fa, fpn_stream, fpn_lc, fpn_st, fpn_sd);
	//vector <int> mdwsidsv = gws.mostDownStreamWSIDs;
	////swsParameters swsp = gws.subwatershedPars(2022);
	//===================================================

	
	if (openPrjAndSetupModel(-1) == -1) {
		writeLog(fpnLog, "Model setup failed !!!\n", 1, prj.writeConsole);
		if (prj.forSimulation == 1) {// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
			return -1;
		}
	}
	writeLog(fpnLog, "Simulation was started.\n", 1, 1);
	if (startSimulation() == -1) {
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
	if (openProjectFile(forceRealTime) < 0)	{
		writeLog(fpnLog, "Open "+ ppi.fpn_prj+" was failed.\n", 1, prj.writeConsole);
		if (prj.forSimulation == 1) {// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
			return -1;
		}
	}
	writeLog(fpnLog, ppi.fpn_prj+" project was opened.\n", 1, prj.writeConsole);
	if (setupModelAfterOpenProjectFile() == -1) {		
		if (prj.forSimulation == 1) { return -1; }// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
	}
	string isparallel = "true";
	omp_set_num_threads(prj.mdp);

	//todo : ���⿡ �� ���� �������� mdp ���� �߰� ����
	//if (di.cellNnotNull < 12000) {
	//	writeLog(fpnLog, "The number of effective cell [ "
	//		+ to_string(di.cellNnotNull) + "] is smaller than 12,000.\n"
	//		+ "It was converted to serial calculation. \n", 1, 1);
	//	prj.mdp = 1;
	//	isparallel = "false";
	//}
	if (prj.forSimulation == 1) {// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
		if (initOutputFiles() == -1) {
			writeLog(fpnLog, "Initializing output files was failed.\n", 1, 1);
			return -1;
		}
	}
	writeLog(fpnLog, ppi.fpn_prj+" -> Model setup was completed.\n", 1, prj.writeConsole);
	writeLog(fpnLog, "The number of effecitve cells : " + to_string(di.cellNnotNull) + "\n", 1, prj.writeConsole);
	if (prj.mdp == 1) { isparallel = "false"; }
	writeLog(fpnLog, "Parallel : " + isparallel + ". Max. degree of parallelism : "
		+ to_string(prj.mdp) + ".\n", 1, prj.writeConsole);
	return 1;
}


void grmHelp() // /r, /a ���� �߰�
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
	printf("             file path�� �Է����� �ʾƵ� �ȴ�.\n");
	printf("         -- ������Ʈ �̸��� ��ο� ������ ���Ե� ��� ū����ǥ�� ��� �Է��Ѵ�.\n");
	printf("         -- ����(grm.exe�� d://GRMrun�� ���� ���)\n");
	printf("           --- Case1. grm.exe�� �ٸ� ������ ������Ʈ ������ ���� ���\n");
	printf("               D://GRMrun>grm D://GRMTest//TestProject//test.gmp\n");
	printf("               D://GRMrun>grm \"D://GRMTest//Test Project//test.gmp\"\n");
	printf("           --- Case 2. grm.exe�� ���� ������ ������Ʈ ������ ���� ���\n");
	printf("               D://GRMrun>grm test.gmp\n");
	printf("      - /f �������\n");
	printf("         -- grm�� ���� ������ �����Ų��.\n");
	printf("         -- ���� : D://GRMrun>grm /f d://GRMrun//TestProject\n");
	printf("      - /fd �������\n");
	printf("         -- grm�� ���� ������ �����Ų��.\n");
	printf("         -- ���� ���ǰ���� *discharge.out�� ������ ��� ������ �����(*.gmp, *Depth.out, ��)\n");
	printf("         -- ���� : D://GRMrun>grm /fd d://GRMrun//TestProject\n");
	printf("      - /r REF ���ϰ�ο� �̸�\n");
	printf("         -- �ǽð� �����ؼ��� �����Ѵ�.\n");
	printf("         -- ���� : D://GRMrun>grm /r D://GRMTest//TestProject//test.ref\n");
	printf("      - /a ������Ʈ ����(Ȥ�� REF ����) ��ο� �̸�\n");
	printf("         -- ������ ��� �Էµ� �ڷḦ ��� ����ϸ�, AutoROM�� �����Ѵ�.\n");
	printf("         -- ���� : D://GRMrun>grm /a D://GRMTest//TestProject//test.gmp\n");
	printf("         -- ���� : D://GRMrun>grm /r /a D://GRMTest//TestProject//test.ref\n");
	printf("\n");
	printf("** Usage (in English)\n");
	printf("  1. Make a gmp file by using a text editor or the QGIS-GRM.\n");
	printf("  2. In the console window, the gmp file is entered as the argument to run GRM.exe.\n");
	printf("      - In Console : grm.exe [argument]\n");
	printf("  3. argument\n");
	printf("      - /?\n");
	printf("          Help\n");
	printf("      - gmp file path and name\n");
	printf("         -- Run the GRM model for a single gmp file.\n");
	printf("         -- When the GRM.exe and gmp files are in the same folder,\n");
	printf("             the project file path does not have to be entered.\n");
	printf("         -- If there are spaces in the project file name or path, quotation marks �� �� are used to enclose it for input.\n");
	printf("         -- Example(when grm.exe is in 'd://GRMrun' folder)\n");
	printf("           --- Case1. gmp file is in different folder with grm.exe\n");
	printf("               D://GRMrun>grm D://GRMTest//TestProject//test.gmp\n");
	printf("               D://GRMrun>grm \"D://GRMTest//Test Project//test.gmp\"\n");
	printf("           --- Case 2. gmp file is in the same folder with grm.exe\n");
	printf("               D://GRMrun>grm test.gmp\n");
	printf("      - /f folder path\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- Example : D://GRMrun>grm /f d://GRMrun//TestProject\n");
	printf("      - /fd folder path\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- And all files, except for the discharge file (*discharge.out), are deleted (*.gmp, *Depth.out, etc.).\n");
	printf("         -- Example : D://GRMrun>grm /fd d://GRMrun//TestProject\n");
	printf("      - /r ref file path and name \n");
	printf("         -- Start real time simulation.\n");
	printf("         -- Example : D://GRMrun>grm /r D://GRMTest//TestProject//test.ref\n");
	printf("      - /a gmp file (or ref file) path and name\n");
	printf("         -- Apply AutoROM after using all the data entered for reservoir operation.\n");
	printf("         -- Example : D://GRMrun>grm /a D://GRMTest//TestProject//test.gmp\n");
	printf("         -- Example : D://GRMrun>grm /r /a D://GRMTest//TestProject//test.ref\n");
}
