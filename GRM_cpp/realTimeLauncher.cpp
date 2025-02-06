// 이 문서의 코드는 최가 초안 작성, 실시간 시스템 구축시 유지관리. 2020.04.23. 최
#include "stdafx.h"
#include "grm.h"
#include "gentle.h"
#include "realTime.h"
using namespace std;

extern projectFile prj;
extern projectfilePathInfo ppi;
extern fs::path fpnLog;

thisSimulationRT tsrt;

// 실시간 수신자료 이용은 옵션은 /r 
int grmRTLauncher(int argc, char** args, int isEnforceAutoROM)
{
    // 이부분 모르겠다.  원이사님 검토 필요. 2020.04.23.최
    string delaySec = "1"; //  GRMLauncher2018.Properties.Settings.Default.DelaySecond_for_remote_debug.ToString(); 
    int delaymilSec = stoi(delaySec) * 100;
    Sleep(delaymilSec);  
    string strGUID = "";
    string startCommandTime = ""; //이건 모의 시작 버튼을 누른 시간
    string rtStartDataTime = ""; // 이게 모의 시작 데이터 시간
    string nameLensModel = "";
    //COleDateTime tnow = COleDateTime::GetCurrentTime();
    tm tnow = getCurrentTimeAsLocal_tm(); //MP 수정
    // 2020.04.23. 최
    // 여기서 argument로 들어오는 시간 포맷은 yyyymmddHHMMSS 을 가정한다.
    // 포맷으로 사용하려면 dateTimeFormat:: 에서 바꿔 준다.
    startCommandTime = timeToString(tnow, timeUnitToShow::toM, dateTimeFormat::yyyymmddHHMMSS);

    tsrt.enforceAutoROM = isEnforceAutoROM; // isPrediction==1 인경우에는 /r, /a 옵션이 모두 적용된 경우이다.
    string fpnRef;

    if (tsrt.enforceAutoROM == 1) {
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
        if (tsrt.enforceAutoROM == 1) {// 이경우는 argc 최대 8개
        // args[0] : grm.exe,               args[1] : /r 혹은 /a,   args[2] : /r 혹은 /a,          
        // args[3] : fpnRef,                 args[4] : strGUID,       args[5] : startCommandTime,
        // args[6] : rtStartDataTime, agrs[7] : strMODEL        
            if (argc == 7) {   //2017.6.20 원 : monitor exe 에서 call 한 경우
                strGUID = args[4];
                startCommandTime = args[5]; //new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[6];
            }
            else if (argc == 8) {   //2019.4.12 원 :  임시 코드. LENS 앙상블 고려
                //2019.10.30 원 : 상기 line 으로 실험
                strGUID = args[4];
                startCommandTime = args[5];// new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[6];
                nameLensModel = args[7];
            }
            else if (argc == 6) { //2017.6.20 원 : 디버깅 위해 1개 유역 call 한 경우
                //strGUID = "????????-????-????-????-????????????";
                strGUID = "9998";   //2018.8.10
                // startCommandTime 위에서 default로 지정됨. 2020.04.23.최
                // rtStartDataTime 는 ref 파일에 있음
            }
            else if (argc == 5) { 
                strGUID = args[4];
                // startCommandTime 위에서 default로 지정됨. 2020.04.23.최
                // rtStartDataTime 는 ref 파일에 있음
            }
            else if (argc == 4) {
                strGUID = "1";
                // startCommandTime 위에서 default로 지정됨. 2020.04.23.최
                // rtStartDataTime 는 ref 파일에 있음
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
        else {// 이경우는 argc 최대 7개
        // args[0] : grm.exe,               args[1] : /r 
        // args[2] : fpnRef,                 args[3] : strGUID,       args[4] : startCommandTime, 
        // args[5] : rtStartDataTime,     agrs[6] : strMODEL
            if (argc == 6) {   //2017.6.20 원 : monitor exe 에서 call 한 경우
                strGUID = args[3];
                startCommandTime = args[4]; //new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[5];
            }
            else if (argc == 7) {   //2019.4.12 원 :  임시 코드. LENS 앙상블 고려
                //2019.10.30 원 : 상기 line 으로 실험
                strGUID = args[3];
                startCommandTime = args[4];// new DateTime(Convert.ToInt64(args[4]));
                rtStartDataTime = args[5];
                nameLensModel = args[7];
            }
            else if (argc == 5) { //2017.6.20 원 : 디버깅 위해 1개 유역 call 한 경우
                //strGUID = "????????-????-????-????-????????????";
                strGUID = "9998";   //2018.8.10
                // startCommandTime= //위에서 default로 지정됨. 2020.04.23.최
                // rtStartDataTime 는 ref 파일에 있음
            }
            else if (argc == 4) {
                strGUID = args[3];
                // startCommandTime 위에서 default로 지정됨. 2020.04.23.최
                // rtStartDataTime 는 ref 파일에 있음
            }
            else if (argc == 3) {
                strGUID = "1";
                // startCommandTime 위에서 default로 지정됨. 2020.04.23.최
                // rtStartDataTime 는 ref 파일에 있음
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
        // 여기서 부터는 fpnLog 사용할 수 있다.
        writeLogString(fpnLog, "GRM RT was started.\n", 1, 1);
        oGRM.setUpAndStartGRMRT();
        writeLogString(fpnLog, "GRM RT was ended.\n", 1, 1);
        //Console.ReadLine();    // 필수
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
	return 1;
}