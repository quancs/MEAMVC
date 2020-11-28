#include "meavc.h"

#ifdef WIN32
clock_t start, finish;
#else
tms start, finish;
int startTime;
#endif

/*parameters of algorithm*/
long long maxSteps;
int cutoffTime;
int step;

/*structures about graph*/
int vNum;
int eNum;
Edge *edge;
int	**vEdges;
int	**vAdj;
int	*vDegree;

/* structures about solution */
int *cvSize;
bool **vInC;
int	**cVertexes;
int	**cVertexIndexes;
int	**dscore;
int **vTimestamp;
//uncovered edges of current solution
int	**uncovEdges;
int *ueSize;
int	**uncovEdgeIndexes;
int **eTimestamp;

//best solution found
int bestSize;
bool *vInBest;

double  bestTime;
int    bestStep;
long lastUpdate;

/*MEA structures*/
int selectAndDissolveInterval;
int removeSize;
int population;
int splitNum;
int *gainThrehold;
int **smallCovers;
int *smallSize;

inline int elder(int m, int v1, int v2) {
    if (vTimestamp[m][v1] < vTimestamp[m][v2])
        return v1;
    else {
        return v2;
    }
}

int selToAdd(int m, int p1, int p2) {
    int selToAddV = p1;
    if (dscore[m][p1] > dscore[m][p2]) {
        selToAddV = p1;
    } else if (dscore[m][p1] < dscore[m][p2]) {
        selToAddV = p2;
    } else {
        selToAddV = elder(m, p1, p2);
    }
    return selToAddV;
}

void cover_MEA() {
    for (int m = 0; m < population; m++)
        gainThrehold[m] = 0;

#ifdef WIN32
    start = clock();
#else
    times(&start);
    startTime = start.tms_utime + start.tms_stime;
#endif

    lastUpdate = time(nullptr);
    for (step = 0; true; step++) {
        split();

        if (population > 1)
            combine();

        if (step % 10 == 0 && time(nullptr) - lastUpdate > selectAndDissolveInterval)
            selectAndDissolve();

        fixPopulation();

        if (step % 10 == 0) {
#ifdef WIN32
            finish = clock();
            double elapsedTime = ((double) (finish - start)) / CLOCKS_PER_SEC;
#else
            times(&finish);
            double elapsedTime = double(finish.tms_utime + finish.tms_stime - startTime) / sysconf(_SC_CLK_TCK);
#endif
            if (elapsedTime > cutoffTime)
                break;
        }
    }

    printf("best= %d\ntime= %.2f\n", bestSize, bestTime);
}

void update_best_sol() {
    int bestOne = 0;
    for (int i = 1; i < population; i++) {
        if (cvSize[i] < cvSize[bestOne]) {
            bestOne = i;
        }
    }
    update_best_sol(bestOne);
}

int build_instance(char *filename) {
    char line[1024];
    char tempstr1[10];
    char tempstr2[10];
    int  v, e;

    char	tmp;
    int		v1, v2;

    ifstream infile(filename);
    if (!infile.is_open()) return 0;

    /*** build problem data structures of the instance ***/
    infile.getline(line, 1024);
    while (line[0] == '%') infile.getline(line, 1024);
    if (line[0] == 'p')
        sscanf(line, "%s %s %d %d", tempstr1, tempstr2, &vNum, &eNum);
    else
        sscanf(line, "%d %d %d", &v, &vNum, &eNum);

    edge = new Edge [eNum];
    vEdges = new int *[vNum];
    vAdj = new int *[vNum];
    vDegree = new int [vNum];
    memset(vDegree, 0, sizeof(int) * (vNum));
    vInBest = new bool [vNum];

    ueSize = new int[population];
    cvSize = new int[population];
    eTimestamp = new int *[population];
    uncovEdges = new int *[population];
    uncovEdgeIndexes = new int *[population];
    dscore = new int *[population];
    vTimestamp = new int *[population];
    vInC = new bool* [population];
    cVertexes = new int *[population];
    cVertexIndexes = new int *[population];
    //mea
    gainThrehold = new int[population];
    smallCovers = new int *[population];
    smallSize = new int[population];

    for (e = 0; e < eNum; e++) {
        infile.getline(line, 1024);
        stringstream ss;
        ss << line;
        if (line[0] == 'e')
            ss >> tmp >> v1 >> v2;
        else
            ss >> v1 >> v2;

        v1 = v1 - 1;
        v2 = v2 - 1;

        vDegree[v1]++;
        vDegree[v2]++;

        edge[e].v1 = v1;
        edge[e].v2 = v2;
    }
    infile.close();
    //init population
    for (int m = 0; m < population; m++) {
        eTimestamp[m] = new int[eNum];
        memset(eTimestamp[m], -1, sizeof (int)*eNum);
        uncovEdges[m] = new int [eNum];
        uncovEdgeIndexes[m] = new int [eNum];
        for (int j = 0; j < eNum; j++) {
            uncovEdges[m][j] = j;
            uncovEdgeIndexes[m][j] = j;
        }
        ueSize[m] = eNum;
        dscore[m] = new int [vNum];
        memcpy(dscore[m], vDegree, sizeof (int)*vNum);
        vTimestamp[m] = new int [vNum];
        memset(vTimestamp[m], -1, sizeof (int)*vNum);
        vInC[m] = new bool[vNum];
        memset(vInC[m], 0, sizeof (bool)*vNum);
        cVertexes[m] = new int [vNum];
        cVertexIndexes[m] = new int[vNum];
        memset(cVertexes[m], -1, sizeof(int)*vNum);
        memset(cVertexIndexes[m], -1, sizeof (int)*vNum);
        smallCovers[m] = new int[splitNum];
        memset(smallCovers[m], -1, sizeof(int)*splitNum);
        cvSize[m] = 0;
    }
    /* build v_adj and v_edges arrays */
    for (v = 0; v < vNum; v++) {
        vAdj[v] = new int[vDegree[v]];
        vEdges[v] = new int[vDegree[v]];
    }

    int *vDegreeTmp = new int [vNum];
    memset(vDegreeTmp, 0, sizeof(int) * vNum);
    for (e = 0; e < eNum; e++) {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        vEdges[v1][vDegreeTmp[v1]] = e;
        vEdges[v2][vDegreeTmp[v2]] = e;

        vAdj[v1][vDegreeTmp[v1]] = v2;
        vAdj[v2][vDegreeTmp[v2]] = v1;

        vDegreeTmp[v1]++;
        vDegreeTmp[v2]++;
    }
    delete[] vDegreeTmp;

    return 1;
}

void free_memory() {
    //delete graph
    delete[] edge;
    delete[] vDegree;
    for (int v = 0; v < vNum; v++) {
        delete[] vAdj[v];
        delete[] vEdges[v];
    }
    delete[] vAdj;
    delete[] vEdges;

    //delete membranes
    delete [] cvSize;
    for (int i = 0; i < population; i++) {
        delete [] vInC[i];
        delete [] cVertexes[i];
        delete [] cVertexIndexes[i];
        delete [] dscore[i];
        delete [] vTimestamp[i];
        delete [] uncovEdges[i];
        delete [] uncovEdgeIndexes[i];
        delete [] eTimestamp[i];
    }
    delete [] vInC;
    delete [] cVertexes;
    delete [] cVertexIndexes;
    delete [] dscore;
    delete [] vTimestamp;
    delete [] uncovEdges;
    delete [] ueSize;
    delete [] uncovEdgeIndexes;
    delete [] eTimestamp;

    //delete best membrane
    delete [] vInBest;

    //delete MEA
    delete [] gainThrehold;
    for (int i = 0; i < population; i++)
        delete [] smallCovers[i];
    delete [] smallCovers;
    delete [] smallSize;
}

int choose_remove_v(int m, int cand_count) {
    int i, v;

    int bestV = cVertexes[m][rand() % cvSize[m]];
    for (i = 1; i < cand_count; ++i) {
        v = cVertexes[m][rand() % cvSize[m]];

        if (dscore[m][v] < dscore[m][bestV])
            continue;
        else if (dscore[m][v] > dscore[m][bestV])
            bestV = v;
        else if (vTimestamp[m][v] < vTimestamp[m][bestV]) {
            bestV = v;
        }
    }

    return bestV;
}

void uncover(int m, int e) {
    uncovEdgeIndexes[m][e] = ueSize[m];
    uncovEdges[m][ueSize[m]++] = e;
    eTimestamp[m][e] = step;
}

void cover(int m, int e) {
    int index, last_uncov_edge;

    //since the edge is covered, its position can be reused to store the last_uncov_edge
    last_uncov_edge = uncovEdges[m][--ueSize[m]];
    index = uncovEdgeIndexes[m][e];
    uncovEdges[m][index] = last_uncov_edge;
    uncovEdgeIndexes[m][last_uncov_edge] = index;
    eTimestamp[m][e] = step;
}

void init_pop() {
    if (population <= 0)
        return;
#ifdef WIN32
    start = clock();
#else
    times(&start);
#endif

    bestSize = 0;

    int *addCount = new int[vNum];
    memset(addCount, 0, sizeof (int)*vNum);
    btree_multimap<GainAddKey, int, std::greater<GainAddKey>> gain2VsMap;
    vector<int> removedVers;
    removedVers.reserve(vNum);

    for (int m = 0; m < population; m++) {
        gain2VsMap.clear();
        for (int v = 0; v < vNum; v++) {
            gain2VsMap.insert(make_pair(GainAddKey(vDegree[v], addCount[v]), v));
        }
        for (; ueSize[m] > 0;) {
            removedVers.clear();
            int highestGain = gain2VsMap.begin()->first.gain;
            for (auto j = gain2VsMap.begin(); j != gain2VsMap.end() && ueSize[m] > 0; ) {
                int gainValue = j->first.gain, v = j->second, addV = j->first.add;
                if (highestGain == gainValue) {
                    if (gainValue != dscore[m][v] || addV != addCount[v]) {
                        j = gain2VsMap.erase(j);
                        if (dscore[m][v] > 0)
                            removedVers.push_back(v);
                    } else {
                        j = gain2VsMap.erase(j);
                        add(m, v);
                        addCount[v]++;
                    }
                } else
                    break;
            }
            //插入gain值变化了的点
            for (int v : removedVers)
                gain2VsMap.insert(make_pair(GainAddKey(dscore[m][v], addCount[v]), v));
        }

        for (int j = 0; j < cvSize[m]; j++) {
            int v = cVertexes[m][j];
            if (dscore[m][v] == 0) {
                remove(m, v);
                addCount[v]--;
                j--;
            }
        }
    }
    delete [] addCount;

    init_match_vc();
    init_edge_vc();

    update_best_sol();
#ifdef WIN32
    finish = clock();
    double initTime = ((double) (finish - start)) / CLOCKS_PER_SEC;
#else
    times(&finish);
    double initTime = double(finish.tms_utime - start.tms_utime + finish.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
#endif

    initTime = round(bestTime * 100) / 100.0;

    cout << "init best=" << bestSize << " time=" << initTime << endl;
}

void add(int m, int v) {
    vInC[m][v] = true;
    dscore[m][v] = -dscore[m][v];

    cVertexes[m][cvSize[m]] = v;
    cVertexIndexes[m][v] = cvSize[m];
    cvSize[m]++;

    vTimestamp[m][v] = step;

    int i, e, n;
    int edgeCount = vDegree[v];

    for (i = 0; i < edgeCount; ++i) {
        e = vEdges[v][i];// v's i'th edge
        n = vAdj[v][i];//v's i'th neighbor

        if (vInC[m][n] == 0) { //this adj isn't in cover set
            dscore[m][n]--;
            cover(m, e);
        } else {
            dscore[m][n]++;
        }
    }
}

void remove(int m, int v) {
    vInC[m][v] = 0;
    dscore[m][v] = -dscore[m][v];

    cvSize[m]--;

    int lastV = cVertexes[m][cvSize[m]];
    int vIndex = cVertexIndexes[m][v];
    cVertexes[m][vIndex] = lastV;
    cVertexIndexes[m][lastV] = vIndex;

    vTimestamp[m][v] = step;

    int i, e, n;

    int edge_count = vDegree[v];
    for (i = 0; i < edge_count; ++i) {
        e = vEdges[v][i];
        n = vAdj[v][i];

        if (vInC[m][n] == 0) { //this adj isn't in cover set
            dscore[m][n]++;
            uncover(m, e);
        } else {
            dscore[m][n]--;
        }
    }
}

void print_solution() {
    for (int i = 1; i <= vNum; i++) {
        if (vInBest[i] == 1) //output vertex cover
            cout << i << '\t';
    }
    cout << endl;
}

int check_solution() {
    for (int e = 0; e < eNum; ++e) {
        if (vInBest[edge[e].v1] != 1 && vInBest[edge[e].v2] != 1) {
            cout << "c error: uncovered edge " << e << endl;
            return 0;
        }
    }

    int verified_vc_size = 0;
    for (int i = 0; i < vNum; i++) {
        if (vInBest[i] == 1)
            verified_vc_size++;
    }

    if (bestSize == verified_vc_size) return 1;

    else {
        cout << "c error: claimed best found vc size!=verified vc size" << endl;
        cout << "c claimed best found vc size=" << bestSize << endl;
        cout << "c verified vc size=" << verified_vc_size << endl;
        return 0;
    }
}

int check_solution(int m) {
    for (int e = 0; e < eNum; ++e) {
        if (vInC[m][edge[e].v1] != 1 && vInC[m][edge[e].v2] != 1) {
            cout << "c error: uncovered edge " << e << " " << m << endl;
            return 0;
        }
    }

    int verified_vc_size = 0;
    for (int i = 0; i < vNum; i++) {
        if (vInC[m][i] == true)
            verified_vc_size++;
    }

    if (cvSize[m] == verified_vc_size)
        return 1;
    else {
        cout << "c error: claimed found vc size!=verified vc size" << endl;
        cout << "c " << m << " claimed found vc size=" << cvSize[m] << endl;
        cout << "c verified vc size=" << verified_vc_size << endl;
        return 0;
    }
}

void selectAndDissolve() {
    lastUpdate = time(nullptr);

    int worst = 0;
    int worstTargetSize = cvSize[worst] + ueSize[worst];

    for (int i = 1; i < population; i++) {
        int targetSize = cvSize[i] + ueSize[i];
        if (targetSize > worstTargetSize) {
            worstTargetSize = targetSize;
            worst = i;
        }
    }

    // replace worst with best
    for (int v = 0; v < vNum; v++) {
        if (vInBest[v] == true) {
            if (vInC[worst][v] == false)
                add(worst, v);
        } else {
            if (vInC[worst][v] == true)
                remove(worst, v);
        }
    }

    //remove removeSize vertices from worst
    for (int i = 0; i < removeSize && cvSize[worst] > 10; i++) {
        int v = cVertexes[worst][rand() % cvSize[worst]];
        remove(worst, v);
    }

    //    // remove similar membranes
    //    for (int m1 = 0; m1 < population; m1++) {
    //        for (int m2 = m1 + 1; m2 < population; m2++) {
    //            if (similar(m1, m2)) {
    //                for (int i = 0; i < 50 && cvSize[m2] > 10; i++) {
    //                    int v = cVertexes[m2][rand() % cvSize[m2]];
    //                    remove(m2, v);
    //                }
    //            }
    //        }
    //    }
}

void fixPopulation() {
    for (int m = 0; m < population; m++) {
        while (!isCover(m) && cvSize[m] < bestSize - 1) {
            int selVer = choose_add_v(m, 24);
            add(m, selVer);
        }
    }

    //0.判断是否存在覆盖
    for (int m = 0; m < population; m++) {
        //1.如果能替换最优解，则替换最优解
        //2.如果是覆盖：去掉损失最小的点，直到不是覆盖
        while (true) {
            if (cvSize[m] >= bestSize) {
                //int bestV = getMinLossVertex(m);
                int bestV = choose_remove_v(m, 100);
                remove(m, bestV);
            } else {
                if (ueSize[m] == 0) {
                    update_best_sol(m);
                    remove(m, getMinLossVertex(m));
                } else {
                    break;
                }
            }
        }
    }
}

void split() {
    for (int m = 0; m < population; m++) {
        smallSize[m] = 0;
        while (smallSize[m] < splitNum && cvSize[m] > 0) {
            int best = choose_remove_v(m, 50);
            remove(m, best);
            smallCovers[m][smallSize[m]++] = best;
        }

        int threhold = 0;
        for (int i = 0; i < smallSize[m]; i++) {
            int v = smallCovers[m][i];
            threhold += dscore[m][v];
        }

        gainThrehold[m] = threhold;
    }
}

void combine() {
    for (int m = 0; m < population; m++) {
        int bestGain = -eNum * 2;
        int bestIndex = -1;
        for (int test = population > 50 ? 50 : population; test >= 0 ; test--) { //最佳适配还是首次适配好？
            int index = rand() % population;
            int totalGain = 0;
            for (int i = 0; i < smallSize[m]; i++) {
                int v = smallCovers[m][i];
                if (vInC[m][v] == false)
                    totalGain += dscore[m][v];
            }
            if (totalGain > bestGain) {
                bestGain = totalGain;
                bestIndex = index;
            }
        }
        if (bestGain > gainThrehold[m]) {
            for (int i = 0; i < smallSize[bestIndex]; i++) {
                int v = smallCovers[bestIndex][i];
                if (vInC[m][v] == false)
                    add(m, v);
            }
        }
    }
}

int choose_add_v(int m, int cand_count) {
    int e = uncovEdges[m][rand() % ueSize[m]];
    int et = eTimestamp[m][e];
    for (int i = 0; i < 24; i++) {
        int e2 = uncovEdges[m][rand() % ueSize[m]];
        int et2 = eTimestamp[m][e2];
        if (et2 < et) {
            e = e2;
            et = et2;
        }
    }

    int v1 = edge[e].v1;
    int v2 = edge[e].v2;

    int addV;

    if (dscore[m][v1] > dscore[m][v2] || (dscore[m][v1] == dscore[m][v2] && vTimestamp[m][v1] < vTimestamp[m][v2]) )
        addV = v1;
    else
        addV = v2;

    return addV;
}

void update_best_sol(int m) {
    bestSize = cvSize[m];
    for (int i = 0; i < vNum; i++) {
        vInBest[i] = vInC[m][i];
    }
#ifdef WIN32
    finish = clock();
    bestTime = ((double) (finish - start)) / CLOCKS_PER_SEC;
#else
    times(&finish);
    bestTime = double(finish.tms_utime - start.tms_utime + finish.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
#endif
    bestTime = round(bestTime * 100) / 100.0;
    bestStep = step;
    lastUpdate = time(nullptr);
    //cout << "c Best vertex cover size = " << bestSize <<  ", SearchSteps = " << bestStep <<  ", SearchTime = " << bestTime << endl;
}

bool isCover(int m) {
    return ueSize[m] == 0;
}

bool similar(int m1, int m2) {
    if (ueSize[m1] == ueSize[m2] && cvSize[m1] == cvSize[m2]) {
        for (int i = 0, uesize = ueSize[m1]; i < uesize; i++) {
            int ueInM1 = uncovEdges[m1][i];

            if (uncovEdges[m2][uncovEdgeIndexes[m2][ueInM1]] != ueInM1)
                return false;
        }
        return true;
    } else
        return false;
}

int check_size(int m) {
    int verified_vc_size = 0;
    for (int i = 0; i < vNum; i++) {
        if (vInC[m][i] == true)
            verified_vc_size++;
    }

    if (cvSize[m] == verified_vc_size)
        return 1;
    else {
        cout << "c error: claimed found vc size!=verified vc size" << endl;
        cout << "c " << m << " claimed found vc size=" << cvSize[m] << endl;
        cout << "c verified vc size=" << verified_vc_size << endl;
        return 0;
    }
}

int getMinLossVertex(int m) {
    int v = cVertexes[m][0];
    int vGain = dscore[m][v];

    for (int i = 0; i < cvSize[m]; i++) {
        int vSel = cVertexes[m][i];
        int vSelGain = dscore[m][vSel];
        if (vSelGain > vGain) {
            v = vSel;
            vGain = vSelGain;
        } else if (vSelGain == vGain) {
            if (vDegree[vSel] > vDegree[v]) {
                v = vSel;
                vGain = vSelGain;
            } else if (vDegree[vSel] == vDegree[v]) {
                if (vTimestamp[m][vSel] < vTimestamp[m][v]) {
                    v = vSel;
                    vGain = vSelGain;
                }
            }
        }
    }
    return v;
}

void init_edge_vc() {
    int cSize2;
    int v, e;
    int v1, v2;

    int *dscore2 = new int[vNum];
    bool *vInC2 = new bool[vNum];

    memset(dscore2, 0, sizeof(int) * vNum);
    memset(vInC2, 0, sizeof(bool) * vNum);

    cSize2 = 0;
    for (e = 0; e < eNum; e++) {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        if (vInC2[v1] == 0 && vInC2[v2] == 0) { //if uncovered, choose the vertex with higher degree
            if (vDegree[v1] > vDegree[v2]) {
                vInC2[v1] = 1;
            } else {
                vInC2[v2] = 1;
            }
            cSize2++;
        }
    }

    //calculate dscores
    for (e = 0; e < eNum; e++) {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        if (vInC2[v1] == 1 && vInC2[v2] == 0) dscore2[v1]--;
        else if (vInC2[v2] == 1 && vInC2[v1] == 0) dscore2[v2]--;
    }

    //remove redundent vertices
    for (v = 0; v < vNum; v++) {
        if (vInC2[v] == 1 && dscore2[v] == 0) {
            vInC2[v] = 0;
            dscore2[v] = -1 * dscore2[v];
            int degree = vDegree[v];
            for (int i = 0; i < degree; i++) {
                int vadj = vAdj[v][i];
                if (vInC2[vadj] == 0) {
                    dscore2[vadj]++;
                } else
                    dscore2[vadj]--;
            }
            cSize2--;
        }
    }

    //replace the worst with this solution
    int worst = 0;
    for (int i = 1; i < population; i++) {
        if (cvSize[i] > cvSize[worst])
            worst = i;
    }

    //if (cvSize[worst] > cSize2) {
    //replace
    int *dstemp = dscore2;
    dscore2 = dscore[worst];
    dscore[worst] = dstemp;
    bool *vInCTemp = vInC2;
    vInC2 = vInC[worst];
    vInC[worst] = vInCTemp;
    cvSize[worst] = cSize2;

    //adjust other data
    for (int v = 0, index = 0; v < vNum; v++) {
        if (vInC[worst][v] == 1) {
            cVertexes[worst][index] = v;
            cVertexIndexes[worst][v] = index;
            index++;
        }
    }

    ueSize[worst] = 0;
    for (int e = 0; e < eNum; e++) {
        Edge &eg = edge[e];
        if (vInC[worst][eg.v1] == 0 && vInC[worst][eg.v2] == 0) {
            uncovEdges[worst][ueSize[worst]] = e;
            uncovEdgeIndexes[worst][e] = ueSize[worst];
            ueSize[worst]++;
        }
    }


    cout << "updated=" << worst << ", check EdgeVC" << endl;
    //check_size(worst);
    //check_solution(worst);
    //}

    delete [] vInC2;
    delete [] dscore2;
}

void init_match_vc() {
    int cSize3;
    int v, e;
    int v1, v2;

    int *dscore3 = new int[vNum];
    bool *vInC3 = new bool[vNum];

    memset(dscore3, 0, sizeof(int) * vNum);
    memset(vInC3, 0, sizeof(bool) * vNum);

    cSize3 = 0;
    for (e = 0; e < eNum; e++) {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        if (vInC3[v1] == 0 && vInC3[v2] == 0) {
            vInC3[v1] = 1;
            vInC3[v2] = 1;
            cSize3 = cSize3 + 2;
        }
    }

    //calculate dscores
    for (e = 0; e < eNum; e++) {
        v1 = edge[e].v1;
        v2 = edge[e].v2;

        if (vInC3[v1] == 1 && vInC3[v2] == 0) dscore3[v1]--;
        else if (vInC3[v2] == 1 && vInC3[v1] == 0) dscore3[v2]--;
    }

    //remove redundent vertices
    for (v = 0; v < vNum; v++) {
        if (vInC3[v] == 1 && dscore3[v] == 0) {
            vInC3[v] = 0;
            dscore3[v] = -1 * dscore3[v];
            int degree = vDegree[v];
            for (int i = 0; i < degree; i++) {
                int vadj = vAdj[v][i];
                if (vInC3[vadj] == 0) {
                    dscore3[vadj]++;
                } else
                    dscore3[vadj]--;
            }
            cSize3--;
        }
    }

    //replace the worst with this solution
    int worst = 0;
    for (int i = 1; i < population; i++) {
        if (cvSize[i] > cvSize[worst])
            worst = i;
    }

    if (cvSize[worst] > cSize3) {
        //replace
        int *dstemp = dscore3;
        dscore3 = dscore[worst];
        dscore[worst] = dstemp;
        bool *vInCTemp = vInC3;
        vInC3 = vInC[worst];
        vInC[worst] = vInCTemp;
        cvSize[worst] = cSize3;

        //adjust other data
        for (int v = 0, index = 0; v < vNum; v++) {
            if (vInC[worst][v] == 1) {
                cVertexes[worst][index] = v;
                cVertexIndexes[worst][v] = index;
                index++;
            }
        }

        ueSize[worst] = 0;
        for (int e = 0; e < eNum; e++) {
            Edge &eg = edge[e];
            if (vInC[worst][eg.v1] == 0 && vInC[worst][eg.v2] == 0) {
                uncovEdges[worst][ueSize[worst]] = e;
                uncovEdgeIndexes[worst][e] = ueSize[worst];
                ueSize[worst]++;
            }
        }
        cout << "updated=" << worst << ", check MatchVC" << endl;
        //check_size(worst);
        //check_solution(worst);
    }

    delete [] vInC3;
    delete [] dscore3;
}
