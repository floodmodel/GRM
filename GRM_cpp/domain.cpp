#include <io.h>
#include <filesystem>

#include "gentle.h"
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern cellPosition* cvps;
extern domaininfo di;
extern cvAtt** cells;
extern cvAtt* cvs;


int readDomainFileAndSetupCV()
{
    if (prj.fpnDomain == "" || _access(prj.fpnDomain.c_str(), 0) != 0) {
        string outstr = "Domain file (" + prj.fpnDomain + ") is invalid.\n";
        writeLog(fpnLog, outstr, 1, 1);
        return -1;
    }
    ascRasterFile dmFile = ascRasterFile(prj.fpnDomain);
    di.nRows = dmFile.header.nRows;
    di.nCols = dmFile.header.nCols;
    di.cellSize = dmFile.header.cellsize;
    di.xll = dmFile.header.xllcorner;
    di.yll = dmFile.header.yllcorner;
    // dim ary(n) �ϸ�, vb.net������ 0~n���� n+1���� �迭��� ����. c#������ 0~(n-1) ���� n ���� ��� ����
    cells = new cvAtt * [di.nCols];
    for (int i = 0; i < di.nCols; ++i) {
        cells[i] = new cvAtt[di.nRows];
    }
    vector<cvAtt> cvsv;
    vector<cellPosition> cpv;
    di.cvidsInDM.clear();
    int cvid = 0;
    // cvid�� ���������� �ο��ϱ� ���ؼ�, �� ������ ���ķ� ���� �ʴ´�..
    for (int ry = 0; ry < di.nRows; ry++) {
        for (int cx = 0; cx < di.nCols; cx++) {
            int wsid = dmFile.valuesFromTL[cx][ry];
            if (wsid > 0) {
                cvAtt cv;
                cellPosition cp;
                cv.wsid = wsid;
                cvid += 1;
                cv.cvid = cvid; // mCVs.Count + 1.  CVid�� CV ����Ʈ(mCVs)�� �ε��� ��ȣ +1 �� ������ �Է� 
                cv.flowType = cellFlowType::OverlandFlow; // �켱 overland flow�� ����
                cp.xCol = cx;
                cp.yRow = ry;
                if (getVectorIndex(di.dmids, wsid) != -1) {
                    di.dmids.push_back(wsid);
                }
                if (di.cvidsInDM.count(wsid) == 0) {
                    vector<int> v;
                    v.push_back(cvid);
                    //di.cvidsInDM.insert(pair<int, vector<int>>(wsid, v));
                    di.cvidsInDM[wsid] = v;
                }
                else {
                    di.cvidsInDM[wsid].push_back(cvid);
                }
                cv.toBeSimulated = 1;
                //if (wsid != 3) { cv.toBeSimulated = -1; } //TODO:�ּ� 2018.12.11
                cells[cx][ry] = cv;
                cvsv.push_back(cv);
                cpv.push_back(cp);
            }
        }
    }
    initWatershedNetwork();
    //WSNetwork = new cWatershedNetwork(watershed.WSIDList);
    subWSPar.SetSubWSkeys(watershed.WSIDList);
    cvs = new cvAtt[cvsv.size()];
    copy(cvsv.begin(), cvsv.end(), cvs);
    cvps = new cellPosition[cpv.size()];
    copy(cpv.begin(), cpv.end(), cvps);
    return 1;
}

int initWatershedNetwork()
{
    di.wsn.wsidsNearbyUp.clear();
    di.wsn.wsidsNearbyDown.clear();
    di.wsn.wsidsAllUp.clear();
    di.wsn.wsidsAllDown.clear();
    di.wsn.mdWSIDs.clear();
    di.wsn.wsOutletCVids.clear();
    di.wsn.mdWSIDofCurrentWS.clear();
    for (int n = 0; n < di.dmids.size(); n++) {
        vector<int> v;
        int id = di.dmids[n];
        di.wsn.wsidsNearbyUp[id]=v;
        di.wsn.wsidsNearbyDown[id] = v;
        di.wsn.wsidsAllUp[id] = v;
        di.wsn.wsidsAllDown[id] = v;
        di.wsn.wsOutletCVids[id] = -1;
        di.wsn.mdWSIDofCurrentWS[id] = -1;
    }
}
