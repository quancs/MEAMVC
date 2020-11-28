#include <iostream>
#include "meavc.h"

using namespace std;

//  ./meavc D:/Data/bio-dmela--2630.mtx.mis 1 10 5 6 10 50

int main(int argc, char *argv[]) {
#ifdef WIN32
    cout << "WIN32 on" << endl;
#else
    cout << "WIN32 off" << endl;
#endif

    char *filename = argv[1];//测试文件
    int seed;//随机数种子
    //int cutoff;//测试时长
    //int population;//种群大小
    //int splitNum;//分裂数量

    sscanf(argv[2], "%d", &seed);
    sscanf(argv[3], "%d", &cutoffTime);
    sscanf(argv[4], "%d", &population);
    sscanf(argv[5], "%d", &splitNum);
    sscanf(argv[6], "%d", &selectAndDissolveInterval);
    sscanf(argv[7], "%d", &removeSize);
    srand(seed);

    if (build_instance(argv[1]) != 1) {
        cout << "can't open instance file" << endl;
        return -1;
    }

    cout << "c This is EAVC, solving instance " << argv[1] << " " << seed << " " << cutoffTime << " " << population << " " << splitNum << " " << selectAndDissolveInterval << " " << removeSize << endl;
    init_pop();

    cover_MEA();
    if (check_solution() == 1) {
        cout << "c Best found vertex cover size = " << bestSize << endl;
        cout << "c SearchSteps for best found vertex cover = " << bestStep << endl;
        cout << "c SearchTime for best found vertex cover = " << bestTime << endl;
    }
    free_memory();
}
