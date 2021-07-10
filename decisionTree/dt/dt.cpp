#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <iostream>
#include <math.h>

using namespace std;

typedef class Node {
public:
    vector<pair<string, bool> > attributes;
    vector<vector<string> > datas;
    vector<Node* > childNodes;

    int keyAttributeIndex;
    string value;
    double info;
    string classificationResult;
    bool isLeaf;

    Node() {
        classificationResult = "ROOT";
        isLeaf = false;
    }

    void makeResult() {
        vector<pair<string, int> > counts;
        int max, index = 0;
        for (int i = 0; i < datas.size(); ++i) {
            bool isInList = false;
            for (int j = 0; j < counts.size(); ++j) {
                if (counts[j].first.compare(datas[i][attributes.size() - 1]) == 0) {
                    isInList = true;
                    counts[j].second++;
                }
            }
            if (!isInList) counts.push_back(make_pair(datas[i][attributes.size() - 1], 1));
        }

        max = counts[0].second;
        for (int i = 1; i < counts.size(); ++i) {
            if (max < counts[i].second) {
                max = counts[i].second;
                index = i;
            }
        }
        classificationResult = counts[index].first;
    }

    void makeInfo() {
        vector<pair<string, int> > count;
        double result = 0.0;
        for (int i = 0; i < datas.size(); ++i) {
            bool isInList = false;
            for (int j = 0; j < count.size(); ++j) {
                if (datas[i][attributes.size() - 1].compare(count[j].first) == 0) {
                    count[j].second++;
                    isInList = true;
                }
            }
            if (!isInList) {
                count.push_back(make_pair(datas[i][attributes.size() - 1], 1));
            }
        }
        for (int i = 0; i < count.size(); ++i) {
            double p = (double)count[i].second / datas.size();
            result += -1.0 * p * log2(p) / log(2);
        }
        info = result;
    }
}Node;

void readFile(string filename, vector<pair<string, bool> >& attributes, vector<vector<string> >& datas);
void chooseAttribute(Node** node);
vector< vector<pair<string, vector<vector<string> > > > > classify(Node* node);
double calculateInfo(vector<pair<string, vector<vector<string> > > > datas, int classAttribute);
string guess(vector<string> data, Node* rootNode);
void test(string testFileName, Node* rootNode, string outputFileName);


int main(int argc, const char* argv[]) {
    string trainingFileName = argv[1];
    string testFileName = argv[2];
    string outputFileName = argv[3];

    Node* rootNode = new Node();

    readFile(trainingFileName, rootNode->attributes, rootNode->datas);
    rootNode->makeInfo();
    chooseAttribute(&rootNode);
    test(testFileName, rootNode, outputFileName);

    return 0;
}

void test(string testFileName, Node* rootNode, string outputFileName) {
    Node* testNode = new Node();
    readFile(testFileName, testNode->attributes, testNode->datas);

    string str = "";
    ofstream writeFile;
    writeFile.open(outputFileName);

    for (int i = 0; i < rootNode->attributes.size() - 1; ++i) {
        str += rootNode->attributes[i].first + '\t';
    }
    str += rootNode->attributes[rootNode->attributes.size() - 1].first + '\n';
    writeFile.write(str.c_str(), str.size());
    str = "";

    for (int i = 0; i < testNode->datas.size(); ++i) {
        string result = guess(testNode->datas[i], rootNode);
        for (int j = 0; j < testNode->datas[i].size(); ++j) {
            str += testNode->datas[i][j] + '\t';
        }
        str += result + '\n';
        writeFile.write(str.c_str(), str.size());
        //cout << result << endl;
        str = "";
    }

    writeFile.close();
}

string guess(vector<string> data, Node* rootNode) {
    Node* cur = rootNode;
    string result = cur->classificationResult;
    while (!cur->isLeaf) {
        for (int i = 0; i < cur->childNodes.size(); ++i) {
            //cout << cur->childNodes[i]->value << ", " << data[cur->keyAttributeIndex] << endl;
            if (data[cur->keyAttributeIndex].compare(cur->childNodes[i]->value) == 0) {
                //cout << "checked" << endl;
                cur = cur->childNodes[i];
                result = cur->classificationResult;
                i = 0;
                if (cur->childNodes.size() == 0) return result;
            }
        }
        cur = cur->childNodes[0];
    }
    return result;
}

void readFile(string filename, vector<pair<string, bool> >& attributes, vector<vector<string> >& datas) {
    ifstream openFile(filename.data());

    if (!openFile.eof()) {
        string line;

        getline(openFile, line);
        //vector<string> attributesList = split(line, '\t');
        vector<string> attributesList;
        int pre = 0;
        for (int i = 0; i < line.size(); i++) {
            if (line[i] == '\t') {
                string word = line.substr(pre, i - pre);
                attributesList.push_back(word);
                pre = i + 1;
            }
        }
        string word = line.substr(pre, line.size() - pre);
        attributesList.push_back(word);

        for (int i = 0; i < attributesList.size(); ++i) {
            attributes.push_back(make_pair(attributesList[i], false));
        }

        while (getline(openFile, line)) {
            //vector<string> data = split(line, '\t');
            vector<string> data;
            int pre = 0;
            for (int i = 0; i < line.size(); i++) {
                if (line[i] == '\t') {
                    string word = line.substr(pre, i - pre);
                    data.push_back(word);
                    pre = i + 1;
                }
            }
            string word = line.substr(pre, line.size() - pre);
            data.push_back(word);

            datas.push_back(data);

        }
    }
}

vector< vector<pair<string, vector<vector<string> > > > > classify(Node* node) {
    //attribute 별로 값으로 분류된 벡터를 가지자. 
    vector< vector<pair<string, vector<vector<string> > > > > classify(node->attributes.size());

    for (int attributeIndex = 0; attributeIndex < node->attributes.size() - 1; ++attributeIndex) {
        if (node->attributes[attributeIndex].second == true) continue;
        for (int datasIndex = 0; datasIndex < node->datas.size(); ++datasIndex) {
            bool isInList = false;
            for (int attributeValueIndex = 0; attributeValueIndex < classify[attributeIndex].size(); ++attributeValueIndex) {
                if (node->datas[datasIndex][attributeIndex].compare(classify[attributeIndex][attributeValueIndex].first) == 0) {
                    isInList = true;
                    classify[attributeIndex][attributeValueIndex].second.push_back(node->datas[datasIndex]);
                }
            }
            if (!isInList) {
                vector<vector<string> > temp;
                temp.push_back(node->datas[datasIndex]);
                classify[attributeIndex].push_back(make_pair(node->datas[datasIndex][attributeIndex], temp));
            }
        }
    }

    return classify;
}

double calculateInfo(vector<pair<string, vector<vector<string> > > > datas, int classAttribute) {
    int all = 0;
    double info = 0;

    // 전체 갯수
    for (int i = 0; i < datas.size(); ++i) {
        vector<pair<string, int > > classify;
        all += datas[i].second.size();

        // class attribute value별로 갯수 세기
        for (int datasIndex = 0; datasIndex < datas[i].second.size(); ++datasIndex) {
            bool isInList = false;
            for (int j = 0; j < classify.size(); ++j) {
                if (classify[j].first.compare(datas[i].second[datasIndex][classAttribute]) == 0) {
                    isInList = true;
                    classify[j].second++;
                }
            }
            if (!isInList) {
                classify.push_back(make_pair(datas[i].second[datasIndex][classAttribute], 1));
            }
        }

        /*
        for(int a = 0 ; a < classify.size(); a++){
            cout << classify[a].first << " " << classify[a].second << endl;
        }*/

        double temp = 0;
        for (int k = 0; k < classify.size(); ++k) {
            double p = (double)classify[k].second / datas[i].second.size();
            temp += -1.0 * p * log(p) / log(2);
            //cout << "temp : " << temp << endl;
        }
        info += datas[i].second.size() * temp;
    }
    info /= all;

    return info;
}


void chooseAttribute(Node** node) {

    ///cout << endl << "chooseAttribute" << endl;

    /*for (int i = 0; i < (*node)->datas.size(); ++i) {
        for (int k = 0; k < (*node)->datas[i].size(); ++k) {
            cout << (*node)->datas[i][k] << " ";
        }
        cout << endl;
    }*/

    vector< vector<pair<string, vector<vector<string> > > > > classified = classify(*node);
    double maxGain = -1;
    double nowInfo = (*node)->info;
    int classifiedAttribute = 0;
    bool isAttributesAllChecked = true;

    //cout << "nowInfo : " << nowInfo << endl;

    /*for(int attributeIndex = 0 ; attributeIndex < classified.size(); ++attributeIndex){
        for(int attributeValueIndex = 0 ; attributeValueIndex < classified[attributeIndex].size(); ++attributeValueIndex){
            cout << classified[attributeIndex][attributeValueIndex].first << " : " << endl;
            for(int dataIndex = 0 ; dataIndex < classified[attributeIndex][attributeValueIndex].second.size(); ++dataIndex){
                for(int i = 0 ; i < classified[attributeIndex][attributeValueIndex].second[dataIndex].size(); ++i){
                    cout << classified[attributeIndex][attributeValueIndex].second[dataIndex][i] << " " ;
                }
                cout << endl;
            }
        }
        cout << endl;
    }*/

    for (int i = 0; i < (*node)->attributes.size(); ++i) {
        if ((*node)->attributes[i].second == false) isAttributesAllChecked = false;
    }

    if (isAttributesAllChecked) {
        //cout << "1" << endl;
        (*node)->makeResult();
        (*node)->isLeaf = true;
        //cout << (*node)->classificationResult << endl << endl;
        return;
    }
    if (nowInfo == 0) {
        //cout << "2" << endl;
        (*node)->makeResult();
        (*node)->isLeaf = true;
        //cout << (*node)->classificationResult << endl << endl;
        return;
    }
    if ((*node)->datas.size() == 0) {
        //cout << "3" << endl;
        (*node)->isLeaf = true;
        (*node)->makeResult();
        return;
    }

    for (int attributeIndex = 0; attributeIndex < classified.size(); ++attributeIndex) {
        double gain = nowInfo - calculateInfo(classified[attributeIndex], classified.size() - 1);
        if (gain > maxGain) {
            maxGain = gain;
            classifiedAttribute = attributeIndex;
        }
    }

    (*node)->keyAttributeIndex = classifiedAttribute;

    for (int i = 0; i < classified[classifiedAttribute].size(); ++i) {
        if (classified[classifiedAttribute][i].second.size() != 0) {
            //if(classified[classifiedAttribute][i].second.size() == 0 ) continue;
            Node* nNode = new Node();
            nNode->datas = classified[classifiedAttribute][i].second;
            nNode->attributes = (*node)->attributes;
            nNode->attributes[classifiedAttribute].second = true;
            nNode->value = classified[classifiedAttribute][i].first;
            nNode->makeInfo();
            nNode->makeResult();
            (*node)->childNodes.push_back(nNode);
        }
    }

    //cout << (*node)->attributes[(*node)->keyAttributeIndex].first << endl;

    for (int i = 0; i < (*node)->childNodes.size(); ++i) {
        /*cout << "chlid value : " << (*node)->childNodes[i]->value << endl;
        for (int j = 0; j < (*node)->childNodes[i]->datas.size(); ++j) {
            for (int k = 0; k < (*node)->childNodes[i]->datas[j].size(); ++k) {
                cout << (*node)->childNodes[i]->datas[j][k] << " ";
            }
            cout << endl;
        }*/
        chooseAttribute(&((*node)->childNodes[i]));
    }
}
