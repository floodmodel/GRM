// �� ������ �ڵ�� �ְ� �ʾ� �ۼ�, �츣�޽ý����� ��������. 2020.04.23.
#include <stdio.h>
#include <iostream>
#include <string>
#include <io.h>
#include "grm.h"
#include "gentle.h"
#include "realTime.h"
using namespace std;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

thisSimulationRT tsrt;

// �ǽð� �����ڷ� �̿��� �ɼ��� /r, ������ /p �̴�.
int grmRTLauncher(int argc, char** args, int isPrediction)
{
    // �̺κ� �𸣰ڴ�.  ���̻�� ���� �ʿ�. 2020.04.23.��
    string delaySec = "1"; //  GRMLauncher2018.Properties.Settings.Default.DelaySecond_for_remote_debug.ToString(); 
    int delaymilSec = stod(delaySec) * 100;
    _sleep(delaymilSec);  
    string strGUID = "";
    string startCommandTime = ""; //�̰� ���� ���� ��ư�� ���� �ð�
    string rtStartDataTime = ""; // �̰� ���� ���� ������ �ð�
    string nameLensModel = "";
    COleDateTime tnow = COleDateTime::GetCurrentTime();
    // 2020.04.23. ��
    // ���⼭ argument�� ������ �ð� ������ yyyymmddHHMMSS �� �����Ѵ�.
    // �������� ����Ϸ��� dateTimeFormat:: ���� �ٲ� �ش�.
    startCommandTime = timeToString(tnow, false, dateTimeFormat::yyyymmddHHMMSS);

    tsrt.isPrediction = isPrediction; // isPrediction==1 �ΰ�쿡�� /r, /p �ɼ��� ��� ����� ����̴�.
    string fpnRef;

    if (tsrt.isPrediction == 1) {
        fpnRef = string(args[3]);
    }
    else {
        fpnRef = string(args[2]);
    }
    fs::path fpn_ref = fs::path(fpnRef.c_str());
    int nResult = _access(fpnRef.c_str(), 0);
    if (nResult == -1
        || lower(fpn_ref.extension().string()) != ".ref") {
        cout << "GRM real time simulation environment file ("
            + fpn_ref.string() + ") is not exist.\n";
        waitEnterKey();
        return -1;
    }
    try {
        if (tsrt.isPrediction == 1) {// �̰��� argc �ִ� 8��
        // args[0] : grm.exe,               args[1] : /r Ȥ�� /p,   args[2] : /r Ȥ�� /p,          
        // args[3] : fpnRef,                 args[4] : strGUID,       args[5] : startCommandTime,
        // args[6] : rtStartDataTime, agrs[7] : strMODEL        
            if (argc == 7) {   //2017.6.20 �� : monitor exe ���� call �� ���
                strGUID = args[4];
                startCommandTime = args[5]; //new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[6];
            }
            else if (argc == 8) {   //2019.4.12 �� :  �ӽ� �ڵ�. LENS �ӻ�� ���
                //2019.10.30 �� : ��� line ���� ����
                strGUID = args[4];
                startCommandTime = args[5];// new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[6];
                nameLensModel = args[7];
            }
            else if (argc == 6) { //2017.6.20 �� : ����� ���� 1�� ���� call �� ���
                //strGUID = "????????-????-????-????-????????????";
                strGUID = "9998";   //2018.8.10
                // startCommandTime ������ default�� ������. 2020.04.23.��
                // rtStartDataTime �� ref ���Ͽ� ����
            }
            else if (argc == 5) { 
                strGUID = args[4];
                // startCommandTime ������ default�� ������. 2020.04.23.��
                // rtStartDataTime �� ref ���Ͽ� ����
            }
            else if (argc == 4) {
                strGUID = "1";
                // startCommandTime ������ default�� ������. 2020.04.23.��
                // rtStartDataTime �� ref ���Ͽ� ����
            }
            else {
                cout << "args.Length Error. Typical statements are below." << endl;
                cout << " grm.exe /r /p fpnRef " << endl;
                cout << " grm.exe /r /p fpnRef strGUID" << endl;
                cout << " grm.exe /r /p fpnRef strGUID startCommandTime " << endl;
                cout << " grm.exe /r /p fpnRef strGUID startCommandTime rtStartDataTime" << endl;
                cout << " grm.exe /r /p fpnRef strGUID startCommandTime rtStartDataTime strMODEL" << endl;
                return -1 ;
            }
        }
        else {// �̰��� argc �ִ� 7��
        // args[0] : grm.exe,               args[1] : /r 
        // args[2] : fpnRef,                 args[3] : strGUID,       args[4] : startCommandTime, 
        // args[5] : rtStartDataTime,     agrs[6] : strMODEL
            if (argc == 6) {   //2017.6.20 �� : monitor exe ���� call �� ���
                strGUID = args[3];
                startCommandTime = args[4]; //new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[5];
            }
            else if (argc == 7) {   //2019.4.12 �� :  �ӽ� �ڵ�. LENS �ӻ�� ���
                //2019.10.30 �� : ��� line ���� ����
                strGUID = args[3];
                startCommandTime = args[4];// new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[5];
                nameLensModel = args[7];
            }
            else if (argc == 5) { //2017.6.20 �� : ����� ���� 1�� ���� call �� ���
                //strGUID = "????????-????-????-????-????????????";
                strGUID = "9998";   //2018.8.10
                // startCommandTime= //������ default�� ������. 2020.04.23.��
                // rtStartDataTime �� ref ���Ͽ� ����
            }
            else if (argc == 4) {
                strGUID = args[3];
                // startCommandTime ������ default�� ������. 2020.04.23.��
                // rtStartDataTime �� ref ���Ͽ� ����
            }
            else if (argc == 3) {
                strGUID = "1";
                // startCommandTime ������ default�� ������. 2020.04.23.��
                // rtStartDataTime �� ref ���Ͽ� ����
            }
            else {
                cout << "args.Length Error. Typical statements are below." << endl;
                cout << " grm.exe /r fpnRef " << endl;
                cout << " grm.exe /r fpnRef strGUID " << endl;
                cout << " grm.exe /r fpnRef strGUID startCommandTime" << endl;
                cout << " grm.exe /r fpnRef strGUID startCommandTime rtStartDataTime" << endl;
                cout << " grm.exe /r fpnRef strGUID startCommandTime rtStartDataTime strMODEL" << endl;
                return -1;
            }
        }

        //GRM oGRM = new GRM(fpnRef, strGUID, strStartLine, strRTStartDateTime, strMODEL);
        grmRealTime oGRM = grmRealTime(fpnRef, strGUID, startCommandTime,
            rtStartDataTime, nameLensModel);
        // ���⼭ ���ʹ� fpnLog ����� �� �ִ�.
        writeNewLog(fpnLog, "GRM RT was started.\n", 1, 1);
        oGRM.setUpAndStartGRMRT();
        writeLog(fpnLog, "GRM RT was ended.\n", 1, 1);
        //Console.ReadLine();    // �ʼ�
        //string s;
        //getline(cin, s);
        waitEnterKey();
    }
    catch (int expn) {
        cout << expn << endl;
        //string s;
        //getline(cin, s);
        waitEnterKey();
        return -1;
    }
}