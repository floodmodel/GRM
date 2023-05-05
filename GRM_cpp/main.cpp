#include "stdafx.h"
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
cvAtt* cvs; // control volumes
cvAtt* cvsb; // t - dt �ð��� ����� cvs ������ �پ��� ���� ���
vector<weatherData> rfs; // ���췮 ���Ͽ��� ���� �ڷ�
vector<weatherData> tempMax; // �ִ��� ���Ͽ��� ���� �ڷ�
vector<weatherData> tempMin; // �ּұ�� ���Ͽ��� ���� �ڷ�
vector<weatherData> dayTimeLength; // �����ð� ���Ͽ��� ���� �ڷ�
vector<weatherData> snowpackTemp;// snow pack �µ� ���Ͽ��� ���� �ڷ�
double sunDurRatio[12]; // ���� �����ð� ���� ���Ͽ��� ���� �ڷ�
vector<weatherData> solarRad; // �ϻ緮 ���Ͽ��� ���� �ڷ�
map<int, double[12]> laiRatio; // <LCvalue, ���� laiRatio 12��>
double blaneyCriddleKData[12]; // Blaney Criddle K ���Ͽ��� ���� �ڷ�
map<int, int*> cvaisToFA; //fa�� cv array idex ���
vector<int> fas; // �帧������ ������ �ϳ��� ���� (0,1,2,4,6,100 ��), fa ���� �Ϸù�ȣ�� �ƴϹǷ�..
map<int, int> faCount; // �帧������(int)�� �� ����(int)
wpSimData wpSimValue; // wp���� ������ �پ��� ����.  prj.wps ���Ͽ��� ���� ������� 2�� ���� ����
flowControlCellAndData fccds; // gmp ���Ͽ��� ���� fc ������ �̿��ؼ� 2�� ���� ����
thisSimulation ts; // �̹� simulation ���μ����� ���õ� �پ��� ����

extern double** ssrAry;
extern double** rfAry;
extern double** rfaccAry;
extern double** QAry;

string msgFileProcess;

int main(int argc, char** args)
{
	string exeName = "GRM";
	version grmVersion = getCurrentFileVersion();
	string outString;
	outString = "GRM v." + to_string(grmVersion.pmajor) + "."
		+ to_string(grmVersion.pminor) + "."
		+ to_string(grmVersion.pbuild) + ". File version : "
		+ to_string(grmVersion.fmajor) + "."
		+ to_string(grmVersion.fminor) + "."
		+ to_string(grmVersion.fbuild) + ".\n";
	    //+"Modified in " + grmVersion.LastWrittenTime + ".\n";
	prj.cpusi = getCPUinfo();
	cout << outString;
	cout << prj.cpusi.infoString;
	outString = outString + prj.cpusi.infoString;
	ts.runByAnalyzer = -1;
	if (argc == 1) {
		printf("ERROR : GRM project file was not entered or invalid arguments.\n");
		grmHelp();
		return 1;
	}
	prj.deleteAllFilesExceptDischargeOut = -1;
	setlocale(LC_ALL, "korean");
	//prj.writeConsole = 1; // exe�� �����ϴ� ���� 1
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

			/// ����� ����-���� ��� �ǽð� ���ǰ� �����ϴ�. ������ ���Ǵ� �ǽð� �ڷ� ó�� ���� �ȵǾ� ����. 2023.03.06
			printf("WARNNING : The continuous simulation is not available for the real time simulation.\n");
			printf("WARNNING : In real time simulation, just the rainfall-runoff event simulation is possible.\n");

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
						printf("ERROR : There is no GRM project file in this directory.\n");
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
					printf("ERROR : Project folder is invalid!!\n");
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
					printf("ERROR : There is no GRM project file in this directory.\n");
					waitEnterKey();
					return -1;
				}
				startGMPsRun(gmpFiles, isP, outString);
				return 1;
			}
			else {
				printf("ERROR : Project folder is invalid!!\n");
				waitEnterKey();
				return -1;
			}
		}
	}
	disposeDynamicVars();
	return 1;
}

int startSingleRun(string fpnGMP, int enforceAutoROM, string outString)
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
		cout<<"ERROR : GRM project file["+
			fpnGMP+"] is invalid.\n";
		waitEnterKey();
		return -1;
	}
	else if (nResult == 0) {
		ppi = getProjectFileInfo(fpnGMP);
		ts.enforceFCautoROM = enforceAutoROM;
		// ������Ʈ ���� ���� ���� ��, fpnLog ���� ���� �� writeNewLog ����� �� �ִ�. 
		writeNewLog(fpnLog, outString, 1, -1); 
		if (setupAndStartSimulation() == -1) {
			waitEnterKey();
			return -1;
		}
	}
	return 1;
}

int startGMPsRun(vector<string> gmpFiles, int enforceAutoROM, string outString)
{
	clock_t  startT = clock();
	int nFiles = gmpFiles.size();
	ts.enforceFCautoROM = enforceAutoROM;
	for (int n = 0; n < nFiles; n++) {// /f Ȥ�� /fd �� ��� ���⼭ ����
		ppi = getProjectFileInfo(gmpFiles[n]);
		// ������Ʈ ���� ���� ���� ��, fpnLog ���� ���� �� writeNewLog ����� �� �ִ�. 
		writeNewLog(fpnLog, outString, 1, -1);
		string progF = to_string(n + 1) + '/' + to_string(gmpFiles.size());
		string progR = dtos(((n + 1) / double(nFiles) * 100.0), 2);
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
			+ to_string(ts_total.tm_hour) + "h "
			+ to_string(ts_total.tm_min) + "m "
			+ to_string(ts_total.tm_sec) + "s.\n";
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
	
	//============ �� �κ��� gmp ���Ϸ� �ν��Ͻ� ====================
	//string fpn = "C://GRM//SampleGHG//GHG500.gmp";
	//string fpn = "D://Github//zTestSet_GRM_SampleWC_cpp//SampleProject.gmp";
	//string fpn = "D://Github//zTestSetPyGRM//Prj_v2022_cont_CJDsmall_pyTest//CJD_Prj_cont_10year_small_pyTest.gmp";
	//string fpn = "D://Github//zTestSetPyGRM//SampleWC//SampleWiCheon.gmp";
	//grmWSinfo gws = grmWSinfo(fpn);
	//============ ������� gmp ���Ϸ� �ν��Ͻ� ====================

	//============ �� �κ��� �Է� ���� ���� ���� �ν��Ͻ� ====================
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
	//============ ������� �Է� ���� ���� ���� �ν��Ͻ� ====================

	//============= grmWSinfo���� ���� ���..
	//vector <int> mdwsidsv = gws.mostDownStreamWSIDs;
	//swsParameters swsp = gws.subwatershedPars(1);
	//int upWSCount = gws.upStreamWSCount(1);
	//===================================================

	
	if (openPrjAndSetupModel(-1) == -1) {
		writeLog(fpnLog, "ERROR : Model setup failed !!!\n", 1, 1);
		if (prj.forSimulation == 1) {// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
			return -1;
		}
	}
	writeLog(fpnLog, "Simulation was started.\n", 1, 1);
	if (startSimulation() == -1) {
		writeNewLog(fpnLog, "ERROR : An error was occurred while simulation...\n", 1, 1);
		return -1;
	}
	if (prj.deleteAllFilesExceptDischargeOut == 1) {
		if (deleteAllFilesExceptDischarge() == -1) {
			writeNewLog(fpnLog, "ERROR : An error was occurred while deleting all files except discharge.out.\n", 1, 1);
			return -1;
		}
	}
	return 1;
}

int openPrjAndSetupModel(int forceRealTime) // 1:true, -1:false
{	
	writeLog(fpnLog, "Checking the input files... \n", 1, 1);
	if (openProjectFile(forceRealTime) < 0)	{
		writeLog(fpnLog, "ERROR : Open "+ ppi.fpn_prj+" was failed.\n", 1, 1);
		if (prj.forSimulation == 1) {// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
			return -1;
		}
	}
	//cout << "completed. \n";
	writeLog(fpnLog, "Checking input files completed.\n", 1, 1);
	writeLog(fpnLog, ppi.fpn_prj+" project was opened.\n", 1, 1);
	//cout << "Setting up input data... ";
	writeLog(fpnLog, "Setting up input data...\n", 1, 1);
	if (setupModelAfterOpenProjectFile() == -1) {		
		if (prj.forSimulation == 1) { return -1; }// exe�� �����ϴ� ���� 1, dll�� �����ϴ� ���� -1
	}
	//cout << "completed. \n";
	writeLog(fpnLog, "Input data setup was completed.\n", 1, 1);
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
			writeLog(fpnLog, "ERROR : Initializing output files was failed.\n", 1, 1);
			return -1;
		}
	}
	writeLog(fpnLog, ppi.fpn_prj+" -> Model setup was completed.\n", 1, 1);
	writeLog(fpnLog, "The number of effecitve cells : " + to_string(di.cellNnotNull) + "\n", 1, 1);
	if (prj.mdp == 1) { isparallel = "false"; }
	writeLog(fpnLog, "Parallel : " + isparallel + ". Max. degree of parallelism : "
		+ to_string(prj.mdp) + ".\n", 1,1);
	return 1;
}


void grmHelp() // /r, /a ���� �߰�
{
	printf("\n");
	printf(" Usage : GRM [Current project file full path and name to simulate]\n");
	printf("\n");
	printf("** ���� (How to use in Korean)\n");
	printf("  1. GRM ������ project ����(.gmp)�� �Է��ڷ�(���������ڷ�, ����, flow control �ð迭 �ڷ�)�� �غ��Ѵ�. \n");
	printf("  2. �𵨸��� ������Ʈ �̸��� ����ġ(argument)�� �ְ� �����Ѵ�.\n");
	printf("      - Console���� grm.exe [argument] �� �����Ѵ�.\n");
	//printf("      ** ���ǻ��� : ��Ⱓ ������ ���, ��ǻ�� ������Ʈ�� ����, ����� �� ������, \n");
	//printf("                       ��Ʈ��ũ�� �����ϰ�, �ڵ�������Ʈ ���� �������� �����Ѵ�.\n");
	printf("  3. argument\n");
	printf("      - /?\n");
	printf("          ����\n");
	printf("      - ������Ʈ ���ϰ�ο� �̸�\n");
	printf("         -- GRM ���� ���ϴ����� �����Ų��.\n");
	printf("         -- �̶� full path, name�� �־�� ������, \n");
	printf("             ������Ʈ ������ GRM.exe ���ϰ� ������ ������ ���� ��쿡��,\n");
	printf("             file path�� �Է����� �ʾƵ� �ȴ�.\n");
	printf("         -- ������Ʈ �̸��� ��ο� ������ ���Ե� ��� ū����ǥ�� ��� �Է��Ѵ�.\n");
	printf("         -- ����(grm.exe�� d:/GRMrun�� ���� ���)\n");
	printf("           --- Case1. grm.exe�� �ٸ� ������ ������Ʈ ������ ���� ���\n");
	printf("               D:/GRMrun>grm D:/GRMTest/TestProject/test.gmp\n");
	printf("               D:/GRMrun>grm \"D:/GRMTest/Test Project/test.gmp\"\n");
	printf("           --- Case 2. grm.exe�� ���� ������ ������Ʈ ������ ���� ���\n");
	printf("               D:/GRMrun>grm test.gmp\n");
	printf("      - /f �������\n");
	printf("         -- grm�� ���� ������ �����Ų��.\n");
	printf("         -- ���� : D:/GRMrun>grm /f d:/GRMrun/TestProject\n");
	printf("      - /fd �������\n");
	printf("         -- grm�� ���� ������ �����Ų��.\n");
	printf("         -- ���� ���ǰ���� *discharge.out�� ������ ��� ������ �����(*.gmp, *Depth.out, ��)\n");
	printf("         -- ���� : D:/GRMrun>grm /fd d:/GRMrun/TestProject\n");
	printf("      - /r REF ���ϰ�ο� �̸�\n");
	printf("         -- �ǽð� �����ؼ��� �����Ѵ�.\n");
	printf("         -- ���� : D:/GRMrun>grm /r D:/GRMTest/TestProject/test.ref\n");
	printf("      - /a ������Ʈ ����(Ȥ�� REF ����) ��ο� �̸�\n");
	printf("         -- ������ ��� �Էµ� �ڷᰡ ���ų�, �Էµ� �ڷḦ ��� ����ϸ�, AutoROM�� �����Ѵ�.\n");
	printf("         -- ���� : D:/GRMrun>grm /a D:/GRMTest/TestProject/test.gmp\n");
	printf("         -- ���� : D:/GRMrun>grm /r /a D:/GRMTest/TestProject/test.ref\n");
	printf("\n");
	printf("** Usage\n");
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
	printf("         -- If there are spaces in the project file name or path, double quotation marks (\" \") have to be used to enclose it for input.\n");
	printf("         -- Example(when grm.exe is in 'd:/GRMrun' folder)\n");
	printf("           --- Case1. gmp file is in different folder with grm.exe\n");
	printf("               D:/GRMrun>grm D:/GRMTest/TestProject/test.gmp\n");
	printf("               D:/GRMrun>grm \"D:/GRMTest/Test Project/test.gmp\"\n");
	printf("           --- Case 2. gmp file is in the same folder with grm.exe\n");
	printf("               D:/GRMrun>grm test.gmp\n");
	printf("      - /f folder path\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- Example : D:/GRMrun>grm /f d:/GRMrun/TestProject\n");
	printf("      - /fd folder path\n");
	printf("         -- The GRM can run at once for all gmp files in the corresponding folder.\n");
	printf("         -- And all files, except for the discharge file (*discharge.out), are deleted (*.gmp, *Depth.out, etc.).\n");
	printf("         -- Example : D:/GRMrun>grm /fd d:/GRMrun/TestProject\n");
	printf("      - /r ref file path and name \n");
	printf("         -- Start real time simulation.\n");
	printf("         -- Example : D:/GRMrun>grm /r D:/GRMTest/TestProject/test.ref\n");
	printf("      - /a gmp file (or ref file) path and name\n");
	printf("         -- Apply AutoROM in case of no reservoir flow data or after using all the data entered for reservoir operation.\n");
	printf("         -- Example : D:/GRMrun>grm /a D:/GRMTest/TestProject/test.gmp\n");
	printf("         -- Example : D:/GRMrun>grm /r /a D:/GRMTest/TestProject/test.ref\n");
}
