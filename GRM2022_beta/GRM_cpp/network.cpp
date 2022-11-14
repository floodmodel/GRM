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
    initWatershedNetwork(); // 여기서 유역 네트워크 정보 초기화
#pragma omp parallel for// schedule(guided)
    //여기서 셀별 네트워크 정보 초기화
    for (int i = 0; i < di.cellNnotNull; ++i) {
        cvs[i].neighborCVidxFlowintoMe.clear();
        cvs[i].downWPCVidx.clear();
        if (di.facMax == di.cellNnotNull) {
            cvs[i].fac = cvs[i].fac - 1;
        }
    }
    if (di.facMax == di.cellNnotNull) {// 이경우는 최상류 셀 fac=1인 경우, 이것을 fac=0으로 만든다.
        di.facMax = di.facMax - 1;
        di.facMin = di.facMin - 1;
    }

    double halfDX_Diag_m;
    double halfDXperp_m;
    halfDX_Diag_m = di.cellSize * sqrt(2) / 2.0;
    halfDXperp_m = di.cellSize / 2.0;
    for (int i = 0; i < di.cellNnotNull; ++i) {
        double dxe = 0.0;
        int tCx; // 하류방향 대상 셀의 x array index
        int tRy; // 하류방향 대상 셀의 y array index
        // 좌상단이 0,0 이다... 즉, 북쪽이면, row-1, 동쪽이면 col +1
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
            //하류 셀이 전체 raster 영역 내부이면,
            if (cvais[tCx][tRy] == -1) {// 하류 셀이 effect 셀 영역 외부에 있으면,
                int wsidKey = cvps[i].wsid; // 이건 현재셀이 포함된 ws의 id
                //di.wsn.wsOutletCVids 는 readDomainFileAndSetupCV() 에서 초기화 되어 있다.
				di.wsn.wsidNearbyDown[cvps[i].wsid] = -1;// 하류 셀이 eff 영역 외부이면, tidx가 cvs의 영역을 벗어난다..
                if (di.wsn.wsOutletidxs.find(wsidKey) == di.wsn.wsOutletidxs.end() ||
                    cvs[i].fac > cvs[di.wsn.wsOutletidxs[wsidKey]].fac) {
                    // 현재 ws에 대한 outlet셀이 지정되지 않았거나, 
                    //이미 지정되어 있는 셀의 fac 보다 현재 셀의 fac가 크면
                    di.wsn.wsOutletidxs[wsidKey] = i;
                    //di.wsn.wsidNearbyDown[cvps[i].wsid] = -1;// 하류 셀이 eff 영역 외부이면, tidx가 cvs의 영역을 벗어난다..
                }
            }
            else {
                int tidx = cvais[tCx][tRy]; // target cell index
                cvs[tidx].neighborCVidxFlowintoMe.push_back(i);
                cvs[tidx].dxWSum = cvs[tidx].dxWSum + dxe;
                cvs[i].downCellidxToFlow = tidx;// 흘러갈 방향의 cellid를 현재 셀의 정보에 기록
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
        else {// 하류셀이 전체 raster 파일 영역 외부이면, 혹은 현재 셀의 Fdir 값이 null 이면
            cvs[i].downCellidxToFlow = -1;
            cvs[i].dxDownHalf_m = dxe;
            int wsidKey = cvps[i].wsid; // 이건 현재셀이 포함된 ws의 id
			di.wsn.wsidNearbyDown[cvps[i].wsid] = -1;
            if (di.wsn.wsOutletidxs.find(wsidKey) == di.wsn.wsOutletidxs.end() ||
                cvs[i].fac > cvs[di.wsn.wsOutletidxs[wsidKey]].fac) {
                // 현재 ws에 대한 outlet셀이 지정되지 않았거나, 
                //이미 지정되어 있는 셀의 fac 보다 현재 셀의 fac가 크면
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
    for (int wsid_cur : di.dmids) { //우선 인접한 유역 id를 추가하고
        for (int wsid_nu : di.wsn.wsidsNearbyUp[wsid_cur]) {
            di.wsn.wsidsAllUp[wsid_cur].push_back(wsid_nu);
        }
        int wsid_nd = di.wsn.wsidNearbyDown[wsid_cur];
        if (wsid_nd > 0) {
            di.wsn.wsidsAllDown[wsid_cur].push_back(wsid_nd);
        }
    }

    for (int wsid_cur : di.dmids) {// 상하류 wsid를 추가한다.
        vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];//여기서 wsid_cur로 없는 key가 들어가면, key가 추가되고, size 0 이 설정된다.
        if (upIDs.size() == 0) {
            di.wsn.wsidsAllUp[wsid_cur].push_back(-1); // 그래서 -1을 강제로 추가한다.
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

	// ID 검증 부분, 현재 유역의 upID 와 downID 에 중복 값이 있으면, 애러 메세지 
	for (int wsid_cur : di.dmids) {// 상하류 wsid를 추가한다.
		vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];//여기서 wsid_cur로 없는 key가 들어가면, key가 추가되고, size 0 이 설정된다.
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
	for (int wsid_cur : di.dmids) {// 상하류 wsid를 추가한다.
		vector<int> upIDs = di.wsn.wsidsAllUp[wsid_cur];//여기서 wsid_cur로 없는 key가 들어가면, key가 추가되고, size 0 이 설정된다.
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


    for (int wsid_cur : di.dmids) {//최하류 wsid 목록을 만들고
        if (di.wsn.wsidNearbyDown[wsid_cur] == -1) {
            di.wsn.mdWSIDs.push_back(wsid_cur);
        }
    }

    for (int wsid_cur : di.dmids) {//특정 wsid에 대한 최하류 wsid를 설정한다.
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