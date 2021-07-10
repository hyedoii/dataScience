#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <math.h>
#include <sstream>

#define STRING(num) STR(num)
#define STR(num) #num

using namespace std;

class User {
private:
    int id;
    vector<pair<int, int> > items;

public:

    User(int id, int item, int rate) {
        this->id = id;
        this->addItem(item, rate);
    }

    void addItem(int item, int rate) {
        this->items.push_back(make_pair(item, rate));
    }

    int getId() {
        return id;
    }

    vector<pair<int, int> > getItemList() {
        return items;
    }

};

pair<vector<User*>, set<int> > readFile(string fileName) {
    ifstream openFile(fileName.data());
    vector<User*> users;
    set<int> items;

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

            int id = stoi(datas[0]);
            int item = stoi(datas[1]);
            int rate = stoi(datas[2]);

            items.insert(item);

            if (users.size() < id) {
                User* newUser = new User(id, item, rate);
                users.push_back(newUser);
            }
            else {
                if (users[id - 1]->getId() == id) {
                    users[id - 1]->addItem(item, rate);
                }
                else {
                    bool isInserted = false;
                    for (int i = 0; i < users.size(); ++i) {
                        if (users[i]->getId() == id) {
                            users[i]->addItem(item, rate);
                            isInserted = true;
                            break;
                        }
                    }
                    if (!isInserted) {
                        User* newUser = new User(id, item, rate);
                        users.push_back(newUser);
                    }
                }
            }
        }
    }
    openFile.close();

    return make_pair(users, items);
}

string makeOutputFileName(string filename) {
    return filename + "_prediction.txt";
}

void makeUserItemMatch(vector<User*> users, vector<vector<double> >& userItemMatch) {
    for (int i = 0; i < users.size(); ++i) {
        int userId = users[i]->getId();
        vector<pair<int, int> > itemList = users[i]->getItemList();
        for (int j = 0; j < itemList.size(); ++j) {
            userItemMatch[userId][itemList[j].first] = itemList[j].second;
        }
    }
}

void makeItemtoItem(int itemSize, int userSize, vector<vector<double> >& userItemMatch, vector<vector<double> >& itemToItem) {
    for (int i = 1; i <= itemSize; i++) {
        for (int j = 1; j <= itemSize; j++) {
            double top = 0, bleft = 0, bright = 0;

            int cnt = 0;
            for (int k = 1; k <= userSize; k++) {
                if (userItemMatch[k][i] == -2.0) continue;
                if (userItemMatch[k][j] == -2.0) continue;

                cnt++;
                top += userItemMatch[k][i] * userItemMatch[k][j];

                bleft += userItemMatch[k][i] * userItemMatch[k][i];
                bright += userItemMatch[k][j] * userItemMatch[k][j];
            }

            if (cnt < 1) {
                itemToItem[i][j] = 0;
            }
            else {
                itemToItem[i][j] = top / (sqrt(bleft) * sqrt(bright));
            }
        }
    }
}

int predict(int user, int item, int itemSize, vector<vector<double> >& userItemMatch, vector<vector<double> >& itemToItem) {
    double top = 0;
    double bottom = 0;

    if (item > itemSize) {
        return 3;
    }

    for (int i = 1; i <= itemSize; i++) {
        if (userItemMatch[user][i] == -2.0) continue;

        bottom += abs(itemToItem[item][i]);
        top += itemToItem[item][i] * userItemMatch[user][i];
    }

    if (bottom == 0) {
        return 3;
    }

    double rating = top / bottom;

    return rating + 0.5;
}

int main(int argc, const char* argv[]) {
    string trainingDataFile = argv[1];
    string testDataFile = argv[2];

    pair<vector<User*>, set<int> > trainingData = readFile(trainingDataFile);
    vector<User*> testData = readFile(testDataFile).first;

    vector<User*> users = trainingData.first;
    set<int> items = trainingData.second;
    vector<vector<double> > userItemMatch(users.size() + 1, vector<double>(items.size() + 1, -2.0));
    makeUserItemMatch(users, userItemMatch);

    vector<vector<double> > itemToItem(items.size() + 1, vector<double>(items.size() + 1));
    makeItemtoItem(items.size(), users.size(), userItemMatch, itemToItem);

    string outputFileName = makeOutputFileName(trainingDataFile);
    ofstream writeFile(outputFileName);

    for (int i = 0; i < testData.size(); ++i) {
        int userId = testData[i]->getId();
        vector<pair<int, int> > userItems = testData[i]->getItemList();

        for (int j = 0; j < userItems.size(); ++j) {
            int predictedRate = predict(userId, userItems[j].first, items.size(), userItemMatch, itemToItem);

            string str = to_string(userId) + '\t' + to_string(userItems[j].first) + '\t' + to_string(predictedRate) + '\n';
            writeFile.write(str.c_str(), str.size());
        }
    }

    writeFile.close();

    return 0;

}
