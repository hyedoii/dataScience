#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <set>

using namespace std;

class Object {
private:
    int id;
    double x;
    double y;
    int clusterNum;
    bool isCoreObject;
    vector<int> NEps;

public:
    Object(string iId, string iX, string iY) {
        id = stoi(iId);
        x = stod(iX);
        y = stod(iY);
        clusterNum = -1;
        isCoreObject = false;
    }

    int getId() {
        return id;
    }

    double getX() {
        return x;
    }

    double getY() {
        return y;
    }

    int getClusterNum() {
        return clusterNum;
    }

    void setClusterNum(int num) {
        clusterNum = num;
    }

    void addNEps(int num) {
        NEps.push_back(num);
    }

    void makeDisicionIfisCore(int minPts) {
        isCoreObject = NEps.size() >= minPts ? true : false;
    }

    bool isCore() {
        return isCoreObject;
    }

    int getNEpsSize() {
        return NEps.size();
    }

    vector<int> getNEps() {
        return NEps;
    }

    void printInfo(vector<Object*> objects) {
        for (int i = 0; i < NEps.size(); ++i) {
            Object* object = objects[NEps[i]];
            cout << object->getId() << " ";
            cout << (object->getX() - x) * (object->getX() - x) + (object->getY() - y) * (object->getY() - y) << endl;
        }
        cout << endl;
    }

};

vector<Object*> readFile(string fileName) {
    ifstream openFile(fileName.data());
    vector<Object*> objects;

    if (!openFile.eof()) {
        string line;
        while (getline(openFile, line)) {
            vector<string> datas;

            int pre = 0;
            for (int i = 0; i < line.size(); i++) {
                if (line[i] == '\t') {
                    string data = line.substr(pre, i - pre);
                    datas.push_back(data);
                    pre = i + 1;
                }
            }
            string word = line.substr(pre, line.size() - pre);
            datas.push_back(word);

            Object* newObject = new Object(datas[0], datas[1], datas[2]);
            objects.push_back(newObject);
        }
    }
    openFile.close();

    return objects;
}

void dfs(vector<Object*> objects, int now, int cluster) {
    objects[now]->setClusterNum(cluster);
    if (!objects[now]->isCore()) return;

    for (int i = 0; i < objects[now]->getNEpsSize(); ++i) {
        int next = objects[now]->getNEps()[i];
        if (objects[next]->getClusterNum() == -1) {
            dfs(objects, next, cluster);
        }
    }
}

bool compare(vector<int> a, vector<int> b) {
    return a.size() > b.size();
}

string makeOutputFileName(string filename, int clusterNum) {
    string inputFileNum = filename.substr(0, 6);
    return inputFileNum + "_cluster_" + to_string(clusterNum) + ".txt";
}

void writeFile(vector<vector<int> > clusters, string filename) {
    for (int i = 0; i < clusters.size(); ++i) {
        string outputFileName = makeOutputFileName(filename, i);
        ofstream writeFile(outputFileName);

        for (int j = 0; j < clusters[i].size(); ++j) {
            string str = to_string(clusters[i][j]) + '\n';
            writeFile.write(str.c_str(), str.size());
        }

        writeFile.close();
    }
}


int main(int argc, const char* argv[]) {
    string fileName = argv[1];
    int n = stoi(argv[2]);
    double eps = stod(argv[3]);
    int minPts = stoi(argv[4]);

    vector<Object*> objects = readFile(fileName);
    vector<vector<int> > clusters;
    int clusterIndex = -1;

    for (int i = 0; i < objects.size(); ++i) {
        for (int j = 0; j < objects.size(); ++j) {
            if (i == j) continue;

            double iX = objects[i]->getX(), jX = objects[j]->getX();
            double iY = objects[i]->getY(), jY = objects[j]->getY();
            double dist = (iX - jX) * (iX - jX) + (iY - jY) * (iY - jY);

            if (dist < eps * eps) {
                objects[i]->addNEps(j);
            }
        }
        objects[i]->makeDisicionIfisCore(minPts);
    }

    for (int i = 0; i < objects.size(); i++) {
        if (objects[i]->getClusterNum() == -1) {
            if (objects[i]->isCore()) {
                dfs(objects, i, ++clusterIndex);
            }
            else {
                objects[i]->setClusterNum(-2);
            }
        }
    }

    clusters.resize(clusterIndex + 1);
    for (int i = 0; i < objects.size(); i++) {
        if (objects[i]->getClusterNum() != -2) {
            clusters[objects[i]->getClusterNum()].push_back(i);
        }
    }

    sort(clusters.begin(), clusters.end(), compare);
    while (clusters.size() > n) {
        clusters.pop_back();
    }

    writeFile(clusters, fileName);

    return 0;
}