// 이 문서의 코드는 최가 초안 작성, 헤르메시스에서 유지관리. 2020.04.23.
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

// 실시간 수신자료 이용은 옵션은 /rt, 예측은 /p 이다.
// args[0] : grm.exe,               args[1] : /rt 혹은 /p,   args[2] : /rt 혹은 /p,          
// args[3] : fpnRef,                 args[4] : strGUID,       args[5] : dtStartLine(모의시점 ? ? ), 
// args[6] : strRTStartDateTime, agrs[7] : strMODEL
int grmRTLauncher(int argc, char** args, int isPrediction)
{
    // 이부분 모르겠다.  원이사님 검토 필요. 2020.04.23.최
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

        // 2020.04.23. 최
        // 여기서 argument로 들어오는 시간 포맷은 yyyymmddHHMMSS 을 가정한다.
        // 포맷으로 사용하려면 dateTimeFormat:: 에서 바꿔 준다.
        string strStartLine = timeToString(tnow, false, dateTimeFormat::yyyymmddHHMMSS);

        if (argc == 7) {   //2017.6.20 원 : monitor exe 에서 call 한 경우
            strGUID = args[4];
            strStartLine = args[5]; //new DateTime(Convert.ToInt64(args[4]));
            strRTStartDateTime = args[6];
        }
        else if (argc == 8) {   //2019.4.12 원 :  임시 코드. LENS 앙상블 고려
            //2019.10.30 원 : 상기 line 으로 실험
            strGUID = args[4];
            strStartLine = args[5];// new DateTime(Convert.ToInt64(args[4]));
            strRTStartDateTime = args[6];
            strMODEL = args[7];
        }
        else if (argc == 6) { //2017.6.20 원 : 디버깅 위해 1개 유역 call 한 경우
            //strGUID = "????????-????-????-????-????????????";
            strGUID = "9998";   //2018.8.10
            //dtStartLine = DateTime.Now; //위에서 default로 지정됨. 2020.04.23.최
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
        // 여기서 부터는 fpnLog 사용할 수 있다.
        writeNewLog(fpnLog, "grmRTstarter was started.", 1, 1);
        oGRM.setUpAndStartGRMRT();
        writeLog(fpnLog, "oGRM.setUpAndStartGRMRT() - Completed", 1, 1);
        //Console.ReadLine();    // 필수
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