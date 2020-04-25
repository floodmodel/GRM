// �� ������ �ڵ�� �ְ� �ʾ� �ۼ�, �츣�޽ý����� ��������. 2020.04.23.
#include <stdio.h>
#include <iostream>
#include <string>
#include "grm.h"
#include "gentle.h"
#include "realTime.h"
using namespace std;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

thisSimulationRT tsrt;

// �ǽð� �����ڷ� �̿��� �ɼ��� /rt, ������ /p �̴�.
// args[0] : grm.exe,               args[1] : /rt Ȥ�� /p,   args[2] : /rt Ȥ�� /p,          
// args[3] : fpnRef,                 args[4] : strGUID,       args[5] : dtStartLine(���ǽ��� ? ? ), 
// args[6] : strRTStartDateTime, agrs[7] : strMODEL
int grmRTLauncher(int argc, char** args, int isPrediction)
{
    // �̺κ� �𸣰ڴ�.  ���̻�� ���� �ʿ�. 2020.04.23.��
    string delaySec = "1"; //  GRMLauncher2018.Properties.Settings.Default.DelaySecond_for_remote_debug.ToString(); 
    int delaymilSec = stod(delaySec) * 100;
    _sleep(delaymilSec);  

    string fpnRef;
    fpnRef = string(args[3]); 
    try {
        string strGUID = "";
        string strRTStartDateTime = "";
        string strMODEL = "";
        COleDateTime tnow = COleDateTime::GetCurrentTime();

        // 2020.04.23. ��
        // ���⼭ argument�� ������ �ð� ������ yyyymmddHHMMSS �� �����Ѵ�.
        // �������� ����Ϸ��� dateTimeFormat:: ���� �ٲ� �ش�.
        string strStartLine = timeToString(tnow, false, dateTimeFormat::yyyymmddHHMMSS);

        if (argc == 7) {   //2017.6.20 �� : monitor exe ���� call �� ���
            strGUID = args[4];
            strStartLine = args[5]; //new DateTime(Convert.ToInt64(args[4]));
            strRTStartDateTime = args[6];
        }
        else if (argc == 8) {   //2019.4.12 �� :  �ӽ� �ڵ�. LENS �ӻ�� ���
            //2019.10.30 �� : ��� line ���� ����
            strGUID = args[4];
            strStartLine = args[5];// new DateTime(Convert.ToInt64(args[4]));
            strRTStartDateTime = args[6];
            strMODEL = args[7];
        }
        else if (argc == 6) { //2017.6.20 �� : ����� ���� 1�� ���� call �� ���
            //strGUID = "????????-????-????-????-????????????";
            strGUID = "9998";   //2018.8.10
            //dtStartLine = DateTime.Now; //������ default�� ������. 2020.04.23.��
        }
        else {
            cout << "args.Length Error. Typical statement is below." << endl;
            cout << " grm.exe /rt fpnRef strGUID strStartLine strRTStartDateTime strMODEL" << endl;
            return;
        }
        //GRM oGRM = new GRM(fpnRef, strGUID, strStartLine, strRTStartDateTime, strMODEL);
        tsrt.isPrediction = isPrediction;
        grmRealTime oGRM = grmRealTime(fpnRef, strGUID, strStartLine,
            strRTStartDateTime, strMODEL);
        // ���⼭ ���ʹ� fpnLog ����� �� �ִ�.
        writeNewLog(fpnLog, "grmRTstarter was started.", 1, 1);
        oGRM.setUpAndStartGRMRT();
        writeLog(fpnLog, "oGRM.setUpAndStartGRMRT() - Completed", 1, 1);
        //Console.ReadLine();    // �ʼ�
        string s;
        getline(cin, s);
    }
    catch (int expn) {
        cout << expn << endl;
        string s;
        getline(cin, s);
        return -1;
    }
    return 1;
}