# !/usr/bin/python3
import sys
import os
import random
import subprocess

# MEAMVC的批量测试工具，给定图所在的目录和测试参数，批量测试完毕后将结果写入csv文件
# python testMAVC.py ./mavc ../../Data/ 10 1000 10 2

exefile = "./fastvc"
testDir = '../../Data/'
test = 10
tlimitation = 1000
npop=10
nsplit=2
if __name__ == "__main__":
    exefile = sys.argv[1]
    testDir = sys.argv[2]
    test = int(sys.argv[3])
    tlimitation = int(sys.argv[4])
    npop = int(sys.argv[5])
    nsplit = int(sys.argv[6])
outfile = exefile+"_result.csv"

f = open(outfile, "a+")
f.write("file,test,seed,time_limitation,size,time,step\n")
flist = os.listdir(testDir)
for j in range(1, test+1):
    random.shuffle(flist)
    for i in range(0, len(flist)):
        path = os.path.join(testDir, flist[i])
        seed = j
        cmd = exefile + " " + path + " " + str(seed) + " "+str(tlimitation)+ " "+str(npop)+" "+str(nsplit)
        if os.path.isfile(path):
            f.write(flist[i] + ","+str(j) + "," +
                    str(seed) + ","+str(tlimitation)+",")
            print("\n"+cmd)
            p = subprocess.Popen(
                cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            (stdoutdata, stderrdata) = p.communicate()
            bestSize = "-1"
            bestTime = "-1"
            bestStep = "-1"
            for line in str(stdoutdata).split("\n"):
                print(line)
                line = line.strip()
                if line:
                    index = line.find(",")
                    if index < 0:
                        if line.find("c Best found vertex cover size =") >= 0:
                            bestSize = line.replace(
                                "c Best found vertex cover size = ", "", -1)
                        if line.find("SearchSteps") >= 0:
                            bestStep = line.replace(
                                "c SearchSteps for best found vertex cover = ", "", -1)
                        if line.find("SearchTime") >= 0:
                            bestTime = line.replace(
                                "c SearchTime for best found vertex cover = ", "", -1)
            if bestSize != "-1":
                f.write(bestSize+","+bestTime+","+bestStep+"\n")
            else:
                f.write("No result\n")
        f.flush()
f.close()
