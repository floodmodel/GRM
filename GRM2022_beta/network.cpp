#include "stdafx.h"
#include "gentle.h"
#include "grm.h"

using namespace std;

extern cvAtt* cvs;
extern domaininfo di;
extern int** cvais;
extern cvpos* cvps;
extern fs::path fpnLog;

int setFlowNetwork()
{
    initWatershedNetwork(); // ���⼭ ���� ��Ʈ��ũ ���� �ʱ�ȭ
#pragma omp parallel for// schedule(guided)
    //���⼭ ���� ��Ʈ��ũ ���� �ʱ�ȭ
    for (int i = 0; i < di.cellNnotNull; ++i) {
        cvs[i].neighborCVidxFlowintoMe.clear();
        cvs[i].downWPCVidx.clear();
        if (di.facMax == di.cellNnotNull) {
            cvs[i].fac = cvs[i].fac - 1;
        }
    }
    if (di.facMax == di.cellNnotNull) {// �̰��� �ֻ�� �� fac=1�� ���, �̰��� fac=0���� �����.
        di.facMax = di.facMax - 1;
        di.facMin = di.facMin - 1;
    }

    double halfDX_Diag_m;
    double halfDXperp_m;
    halfDX_Diag_m = di.cellSize * sqrt(2) / 2.0;
    halfDXperp_m = di.cellSize / 2.0;
    for (int i = 0; i < di.cellNnotNull; ++i) {
        double dxe = 0.0;
        int tCx; // �Ϸ����� ��� ���� x array index
        int tRy; // �Ϸ����� ��� ���� y array index
        // �»���� 0,0 �̴�... ��, �����̸�, row-1, �����̸� col +1
        int cx = cvps[i].xCol;
        int ry = cvps[i].yRow;
        switch (cvs[i].fdir) {
        case flowDirection8::NE8: {
            tCx = cx + 1;
            tRy = ry - 1;
            dxe = halfDX_Diag_m;
            break;
        }
        case flowDirection8::E8: {
            tCx = cx + 1;
            tRy = ry;
            dxe = halfDXperp_m;
            break;
        }
        case flowDirection8::SE8: {
            tCx = cx + 1;
            tRy = ry + 1;
            dxe = halfDX_Diag_m;
            break;
        }
        case flowDirection8::S8: {
            tCx = cx;
            tRy = ry + 1;
            dxe = halfDXperp_m;
            break;
        }
        case flowDirection8::SW8: {
            tCx = cx - 1;
            tRy = ry + 1;
            dxe = halfDX_Diag_m;
            break;
        }
        case flowDirection8::W8: {
            tCx = cx - 1;
            tRy = ry;
            dxe = halfDXperp_m;
            break;
        }
        case flowDirection8::NW8: {
            tCx = cx - 1;
            tRy = ry - 1;
            dxe = halfDX_Diag_m;
            break;
        }
        case flowDirection8::N8: {
            tCx = cx;
            tRy = ry - 1;
            dxe = halfDXperp_m;
            break;
        }
        case flowDirection8::None8: {
            tCx = -1;
            tRy = -1;
            dxe = halfDXperp_m;
            break;
        }
        }
        if (tCx >= 0 && tCx < di.nCols && tRy >= 0 && tRy < di.nRows) {
            //�Ϸ� ���� ��ü raster ���� �����̸�,
            if (cvais[tCx][tRy] == -1) {// �Ϸ� ���� effect �� ���� �ܺο� ������,
                int wsidKey = cvps[i].wsid; // �̰� ���缿�� ���Ե� ws�� id
                //di.wsn.wsOutletCVids �� readDomainFileAndSetupCV() ���� �ʱ�ȭ �Ǿ� �ִ�.
				di.wsn.wsidNearbyDown[cvps[i].wsid] = -1;// �Ϸ� ���� eff ���� �ܺ��̸�, tidx�� cvs�� ������ �����..
                if (di.wsn.wsOutletidxs.find(wsidKey) == di.wsn.wsOutletidxs.end() ||
                    cvs[i].fac > cvs[di.wsn.wsOutletidxs[wsidKey]].fac) {
                    // ���� ws�� ���� outlet���� �������� �ʾҰų�, 
                    //�̹� �����Ǿ� �ִ� ���� fac ���� ���� ���� fac�� ũ��
                    di.wsn.wsOutletidxs[wsidKey] = i;
                    //di.wsn.wsidNearbyDown[cvps[i].wsid] = -1;// �Ϸ� ���� eff ���� �ܺ��̸�, tidx�� cvs�� ������ �����..
                }
            }
            else {
                int tidx = cvais[tCx][tRy]; // target cell index
                cvs[tidx].neighborCVidxFlowintoMe.push_back(i);
                cvs[tidx].dxWSum = cvs[tidx].dxWSum + dxe;
                cvs[i].downCellidxToFlow = tidx;// �귯�� ������ cellid�� ���� ���� ������ ���
                if (cvps[i].wsid != cvps[tidx].wsid) {
                    if (di.wsn.wsidNearbyDown[cvps[i].wsid] != cvps[tidx].wsid) {
                        di.wsn.wsidNearbyDown[cvps[i].wsid] = cvps[tidx].wsid;
                        di.wsn.wsOutletidxs[cvps[i].wsid] = i;
                    }
                    vector<int> v = di.wsn.wsidsNearbyUp[cvps[tidx].wsid];
                    if (std::find(v.begin(), v.end(), cvps[i].wsid) == v.end()) {
                        di.wsn.wsidsNearbyUp[cvps[tidx].wsid].push_back(cvps[i].wsid);
                    }
                }
            }
            cvs[i].dxDownHalf_m = dxe;
        }
        else {// �Ϸ����� ��ü raster ���� ���� �ܺ��̸�, Ȥ�� ���� ���� Fdir ���� null �̸�
            cvs[i].downCellidxToFlow = -1;
            cvs[i].dxDownHalf_m = dxe;
            int wsidKey = cvps[i].wsid; // �̰� ���缿�� ���Ե� ws�� id
			di.wsn.wsidNearbyDown[cvps[i].wsid] = -1;
            if (di.wsn.wsOutletidxs.find(wsidKey) == di.wsn.wsOutletidxs.end() ||
                cvs[i].fac > cvs[di.wsn.wsOutletidxs[wsidKey]].fac) {
                // ���� ws�� ���� outlet���� �������� �ʾҰų�, 
                //�̹� �����Ǿ� �ִ� ���� fac ���� ���� ���� fac�� ũ��
                di.wsn.wsOutletidxs[wsidKey] = i;
				//di.wsn.wsidNearbyDown[cvps[i].wsid] = -1;
            }
        }
    }
    if (updateWatershedNetwork() == -1) {
        return -1;
    }
    return 1;
}



int initWatershedNetwork()
{
    di.wsn.wsidsNearbyUp.clear();
    di.wsn.wsidNearbyDown.clear();
    di.wsn.wsidsAllUp.clear();
    di.wsn.wsidsAllDown.clear();
    di.wsn.mdWSIDs.clear();
    di.wsn.wsOutletidxs.clear();
    di.wsn.mdWSIDofCurrentWS.clear();
    return 1;
}




int updateWatershedNetwork()
{
    for (int wsid_cur : di.dmids) { //�켱 ������ ���� id�� �߰��ϰ�
        for (int wsid_nu : di.wsn.wsidsNearbyUp[wsid_cur]) {
            di.wsn.wsidsAllUp[wsid_cur].push_back(wsid_nu);
        }
        int wsid_nd = di.wsn.wsidNearbyDown[wsid_cur];
        if (wsid_nd > 0) {
            di.wsn.wsidsAllDown[wsid_cur].push_back(wsid_nd);
        }
    }

    for (int wsid_cur : di.dmids) {// ���Ϸ� wsid�� �߰��Ѵ�.
        vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];//���⼭ wsid_cur�� ���� key�� ����, key�� �߰��ǰ�, size 0 �� �����ȴ�.
        if (upIDs.size() == 0) {
            di.wsn.wsidsAllUp[wsid_cur].push_back(-1); // �׷��� -1�� ������ �߰��Ѵ�.
        }
        vector<int> downIDs = di.wsn.wsidsAllDown[wsid_cur];
        if (downIDs.size() == 0) {
            di.wsn.wsidsAllDown[wsid_cur].push_back(-1);
        }
        for (int uid : upIDs) {
            for (int did : downIDs) {
                if (getVectorIndex(di.wsn.wsidsAllUp[did], uid) == -1) {
                    di.wsn.wsidsAllUp[did].push_back(uid);
                }
                if (getVectorIndex(di.wsn.wsidsAllDown[uid], did) == -1) {
                    di.wsn.wsidsAllDown[uid].push_back(did);
                }
            }
        }
    }					

	// ID ���� �κ�, ���� ������ upID �� downID �� �ߺ� ���� ������, �ַ� �޼��� 
	for (int wsid_cur : di.dmids) {// ���Ϸ� wsid�� �߰��Ѵ�.
		vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];//���⼭ wsid_cur�� ���� key�� ����, key�� �߰��ǰ�, size 0 �� �����ȴ�.
		vector<int> downIDs = di.wsn.wsidsAllDown[wsid_cur];
		for (int uid : upIDs) {
			if (uid != wsid_cur && uid>0) {
				for (int did : downIDs) {
					if (uid == did) {
						writeLog(fpnLog, "ERROR : The flow directions of watershed IDs " + to_string(uid) + " and " + to_string(wsid_cur) + " are recursive.\n", 1, 1);
						return -1;
					}
				}
			}
		}
	}
	for (int wsid_cur : di.dmids) {// ���Ϸ� wsid�� �߰��Ѵ�.
		vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];//���⼭ wsid_cur�� ���� key�� ����, key�� �߰��ǰ�, size 0 �� �����ȴ�.
		vector<int> downIDs = di.wsn.wsidsAllDown[wsid_cur];
		for (int  did : downIDs) {
			if (did != wsid_cur && did > 0) {
				for (int uid : upIDs) {
					if (uid == did) {
						writeLog(fpnLog, "ERROR : The flow directions of watershed IDs " + to_string(did) + " and " + to_string(wsid_cur) + " are recursive.\n", 1, 1);
						return -1;
					}
				}
			}
		}
	}


    for (int wsid_cur : di.dmids) {//���Ϸ� wsid ����� �����
        if (di.wsn.wsidNearbyDown[wsid_cur] == -1) {
            di.wsn.mdWSIDs.push_back(wsid_cur);
        }
    }

    for (int wsid_cur : di.dmids) {//Ư�� wsid�� ���� ���Ϸ� wsid�� �����Ѵ�.
        for (int wsid_md : di.wsn.mdWSIDs) {
            if (wsid_cur == wsid_md
                || getVectorIndex(di.wsn.wsidsAllUp[wsid_md], wsid_cur) != -1) {
                di.wsn.mdWSIDofCurrentWS[wsid_cur] = wsid_md;
            }
        }
    }
    return 1;
}

vector<int> getAllUpstreamCells(int startingBaseidx)
{
    bool bAllisEnded = false;
    vector<int> cvidxsOfAllUpstreamCells;// = new List<int>();
    vector<int> baseidxs;
    //vector<int> currentUpstreamCells;
    cvidxsOfAllUpstreamCells.push_back(startingBaseidx);
    baseidxs.push_back(startingBaseidx);
    while (!bAllisEnded == true)    {
        int intCountAL = baseidxs.size();
        vector<int> currentUpstreamCells;
        bAllisEnded = true;
        for (int bidx : baseidxs) {
            int curCVidx = bidx;
            int countUpCells = cvs[curCVidx].neighborCVidxFlowintoMe.size();
            if (countUpCells > 0) {
                bAllisEnded = false;
                for (int i : cvs[curCVidx].neighborCVidxFlowintoMe) {
                    int upArrayNum = i;
                    //int colX = cvps[i].xCol;
                    //int rowY = cvps[i].yRow;
                    currentUpstreamCells.push_back(i); 
                    cvidxsOfAllUpstreamCells.push_back(i);
                }
            }
        }
        baseidxs.clear();
        baseidxs = currentUpstreamCells;
    }
    return cvidxsOfAllUpstreamCells;
}