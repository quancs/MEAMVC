#ifndef MEAVC_H
#define MEAVC_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <vector>
#include <cmath>
#include "btree/btree_map.h"
#include <utility>
#include <time.h>

using std::make_pair;
using btree::btree_multimap;
using namespace std;

#ifdef WIN32
extern clock_t start, finish;
#else
#include <unistd.h>
#include <sys/times.h>
extern tms start, finish;
extern int startTime;
#endif

struct Edge {
    int v1;
    int v2;
};
struct GainAddKey  {
    GainAddKey(int gain, int add) {
        this->gain = gain;
        this->add = add;
    }
    GainAddKey() {
        this->gain = 0;
        this->add = 0;
    }

    int gain;
    int add;
    bool operator <(const GainAddKey &key)const {
        if (gain != key.gain) {
            return gain < key.gain;
        } else {
            return add > key.add;
        }
    }
    bool operator ==(const GainAddKey &key)const {
        return gain == key.gain && add == key.add;
    }
    bool operator >(const GainAddKey &key) const {
        if (gain != key.gain) {
            return gain > key.gain;
        } else {
            return add < key.add;
        }
    }
};

/*parameters of algorithm*/
extern long long maxSteps;//step limit
extern int cutoffTime;//time limit
extern int step;//current step

/*structures about graph*/
extern int vNum;//|V|
extern int eNum;//|E|
extern Edge *edge;
extern int **vEdges;//edges of v, vEdges[i][k] means vertex v_i's k_th edge
extern int **vAdj;//vAdj[v_i][k] = v_j(actually, that is v_i's k_th neighbor)
extern int *vDegree;//amount of edges (neighbors) related to v

/* structures about solution */
//C: current candidate solutions
extern int *cvSize;//sizes of the vertex set of C
extern bool **vInC;//a flag indicates whether a vertex is in C
extern int **cVertexes;//vertexes of C
extern int **cVertexIndexes;//the positions of vertexes, eg. cVertexIndexes[i][v] means the location of v in the i-th C (cVertexes[i]) that means cVertexes[i]=v or v is not in C[i]
extern int **dscore;//danymic score of v, stores the gain value or loss value
extern int **vTimestamp;//ages of vertexes
//uncovered edges of current solution
extern int **uncovEdges;//stores the uncov edge id
extern int *ueSize;//sizes of uncovered edge sets
extern int **uncovEdgeIndexes;//the position of uncovered edges in uncovEdges
extern int **eTimestamp;//ages of edges

//best solution found
extern int bestSize;
extern bool *vInBest;//if v in best
extern double bestTime;
extern int bestStep;

/*MEA structures*/
extern int selectAndDissolveInterval;
extern int removeSize;
extern int population;
extern int splitNum;
extern int *gainThrehold;
extern int **smallCovers;
extern int *smallSize;

//mea functions
void cover_MEA();
void split();
void combine();
void selectAndDissolve();
void fixPopulation();


/* functions declaration */
void update_best_sol();
void update_best_sol(int m);

int build_instance(char *filename);
void free_memory();

//update the best vertex in C
int choose_remove_v(int m, int cand_count);
inline int choose_add_v(int m, int cand_count);

inline void uncover(int m, int e);
inline void cover(int m, int e);

void init_pop();
void init_edge_vc();
void init_match_vc();
void add(int m, int v);
void remove(int m, int v);

/*On solution*/
void print_solution();
int check_solution();
int check_solution(int m);
int check_size(int m);
inline bool isCover(int m);
bool similar(int m1, int m2);
int getMinLossVertex(int m);
#endif // MEAVC_H