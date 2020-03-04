#include <io.h>
#include <filesystem>

#include "gentle.h"
#include "grm.h"

using namespace std;
namespace fs = std::filesystem;

extern projectfilePathInfo ppi;
extern fs::path fpnLog;
extern projectFile prj;

extern domainCell* cvxys;
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
    //cCVAttribute[,] wsCells = new cCVAttribute[watershed.colCount, watershed.rowCount];
    cells = new cvAtt * [di.nCols];
    for (int i = 0; i < di.nCols; ++i) {
        cells[i] = new cvAtt[di.nRows];
    } 
    vector<cvAtt> cvsv;
    vector<domainCell> dmv;
    int cvid = 0;
    // cvid�� ���������� �ο��ϱ� ���ؼ�, �� ������ ���ķ� ���� �ʴ´�..
    for (int ry = 0; ry < di.nRows; ry++)    {
        for (int cx = 0; cx < di.nCols; cx++)        {
            int wsid = dmFile.valuesFromTL[cx][ry];
            if (wsid > 0)            {
                cvAtt cv;
                domainCell dm;
                cv.wsid = wsid;
                cvid += 1;
                cv.cvid = cvid; // mCVs.Count + 1.  CVid�� CV ����Ʈ(mCVs)�� �ε��� ��ȣ +1 �� ������ �Է� 
                cv.flowType = cellFlowType::OverlandFlow; // �켱 overland flow�� ����
                dm.xCol = cx;
                dm.yRow = ry;
                if (getVectorIndex(di.dmids,cv.wsid)!=-1)                {
                    di.dmids.push_back(cv.wsid);
                }
                if (watershed.mCVidListForEachWS.ContainsKey(wsid) == false)
                {
                    watershed.mCVidListForEachWS.Add(wsid, new List<int>());
                }
                watershed.mCVidListForEachWS[wsid].Add(cv.cvid);
                cv.toBeSimulated = 1;
                if (wsid != 3) { cv.toBeSimulated = -1; }//TODO:�ּ� 2018.12.11
                wsCells[cx, ry] = cv;
                lstCV.Add(cv);
                lstDM.Add(dm);
            }
        }
    }
    WSNetwork = new cWatershedNetwork(watershed.WSIDList);
    subWSPar.SetSubWSkeys(watershed.WSIDList);
    //CVs = new cCVAttribute[lstCV.Count];

    cvs = new cvAtt[cvsv.size()];
    copy(cvsv.begin(), cvsv.end(), cvs);
    cvxys = new domainCell[dmv.size()];
    copy(dmv.begin(), dmv.end(), cvxys);

    CVs = lstCV.ToArray();
    dmInfo = lstDM.ToArray();
    return true;

}