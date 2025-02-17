#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <float.h>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <time.h>
#include <vector>

using namespace std;

int maxFusionQuibits;
int Qubits;
int counter = 0;
int method = 0;
double cost_factor = 1.8;
vector<double> gateTime;

// gate size-index
struct gateSI {
    int fusionSize;
    int fusionIndex;
};

int targetQubitCounter(string gateType) {
    if (gateType == "H" || gateType == "X" || gateType == "RY" ||
        gateType == "RX" || gateType == "U1" || gateType == "RZ" ||
        gateType == "DONE")
        return 1;
    else if (gateType == "CX" || gateType == "CP" || gateType == "CZ" ||
             gateType == "RZZ" || gateType == "DTWO")
        return 2;
    else if (gateType == "DTHREE")
        return 3;
    else if (gateType == "DFOUR")
        return 4;
    else if (gateType == "DFIVE")
        return 5;
    return 0;
}

class fusionGate {
  public:
    gateSI fusionGateSI;
    string gateType;
    vector<int> targetQubit;
    vector<float> rotation;
    vector<fusionGate> subGateList;

    fusionGate() = default;
    fusionGate(int fusionSize, int fusionIndex)
        : fusionGateSI{fusionSize, fusionIndex} {}

    void init() {
        fusionGateSI = {0, 0};
        gateType.clear();
        targetQubit.clear();
        rotation.clear();
        subGateList.clear();
    }

    void dump() const {
        cout << fusionGateSI.fusionSize << "-" << fusionGateSI.fusionIndex
             << endl;
        for (const auto &subGate : subGateList)
            cout << subGate.fusionGateSI.fusionSize << "-"
                 << subGate.fusionGateSI.fusionIndex << " ";
        cout << endl;
    }

    void showGateInfo() const {
        if (fusionGateSI.fusionSize > 1) {
            cout << "fusion size != 1" << endl;
            return;
        }
        cout << gateType << " ";
        for (const auto &qubit : targetQubit)
            cout << qubit << " ";
        for (const auto &rot : rotation)
            cout << rot << " ";
        cout << endl;
    }

    void sortTargetQubit() { sort(targetQubit.begin(), targetQubit.end()); }
};

void showFusionGateList(const vector<vector<fusionGate>> &fusionGateList,
                        int start, int end) {
    for (int i = start - 1; i < end; ++i) {
        cout << "Number of fusion qubits: " << i + 1 << endl;
        for (const auto &gate : fusionGateList[i]) {
            cout << "=========================================================="
                    "======"
                    "======="
                 << endl;
            gate.dump();
            for (const auto &subGate : gate.subGateList)
                subGate.showGateInfo();
        }
        cout << endl;
    }
}

void showDependencyList(const vector<vector<int>> &dependencyList) {
    for (int i = 0; i < dependencyList.size(); ++i) {
        cout << i << " || ";
        for (int j = 0; j < dependencyList[i].size(); ++j) {
            if (dependencyList[i][j] > -1)
                cout << dependencyList[i][j] << " ";
            else
                cout << "NULL";
        }
        cout << endl;
    }
    cout << endl;
}

// check dependency 1:can execute 0:have dependency -1:other
int checkDependency(fusionGate &bigFusionGate,
                    vector<vector<int>> dependencyList) {
    for (int i = 0; i < bigFusionGate.subGateList.size(); ++i) {
        if (dependencyList[bigFusionGate.subGateList[i]
                               .fusionGateSI.fusionIndex][0] == -1)
            for (int j = 0; j < dependencyList.size(); ++j) {
                dependencyList[j].erase(
                    remove(
                        dependencyList[j].begin(), dependencyList[j].end(),
                        bigFusionGate.subGateList[i].fusionGateSI.fusionIndex),
                    dependencyList[j].end());
                if (dependencyList[j].size() == 0)
                    dependencyList[j].push_back(-1);
            }
        else
            return 0;
    }
    return 1;
}

void deleteRelatedNode(vector<vector<fusionGate>> &fusionGateList,
                       vector<vector<int>> &dependencyList,
                       fusionGate bigFusionGate) {
    // delete node in dependency list
    for (int i = 0; i < bigFusionGate.subGateList.size(); ++i)
        for (int index = 0; index < dependencyList.size(); ++index) {
            dependencyList[index].erase(
                remove(dependencyList[index].begin(),
                       dependencyList[index].end(),
                       bigFusionGate.subGateList[i].fusionGateSI.fusionIndex),
                dependencyList[index].end());
            if (dependencyList[index].empty())
                dependencyList[index].push_back(-1);
        }

    // delete node in fusionGate list
    for (int nowFusionSize = 0; nowFusionSize < maxFusionQuibits;
         ++nowFusionSize)
        for (int index = 0; index < fusionGateList[nowFusionSize].size();
             ++index) {
            int flag = 0;
            for (int i = 0;
                 i < fusionGateList[nowFusionSize][index].subGateList.size();
                 ++i) {
                for (int j = 0; j < bigFusionGate.subGateList.size(); ++j)
                    if (fusionGateList[nowFusionSize][index]
                            .subGateList[i]
                            .fusionGateSI.fusionIndex ==
                        bigFusionGate.subGateList[j].fusionGateSI.fusionIndex) {
                        flag = 1;
                        break;
                    }
                if (flag)
                    break;
            }
            if (flag) {
                fusionGateList[nowFusionSize].erase(
                    fusionGateList[nowFusionSize].begin() + index);
                index--;
            }
        }
}

void constructDependencyList(vector<vector<fusionGate>> &fusionGateList,
                             vector<vector<int>> &dependencyList) {
    int maxIndex = 0;
    vector<int> gateOnQubit(Qubits, -1);
    for (const auto &gate : fusionGateList[0])
        maxIndex = max(maxIndex, gate.subGateList[0].fusionGateSI.fusionIndex);
    for (int i = 0; i <= maxIndex; ++i)
        dependencyList.push_back(vector<int>{-1});
    for (const auto &gate : fusionGateList[0]) {
        vector<int> dependency;
        for (int qubit : gate.subGateList[0].targetQubit) {
            dependency.push_back(gateOnQubit[qubit]);
            gateOnQubit[qubit] = gate.subGateList[0].fusionGateSI.fusionIndex;
        }
        if (dependency.size() > 1)
            dependency.erase(remove(dependency.begin(), dependency.end(), -1),
                             dependency.end());
        if (dependency.empty())
            dependency.push_back(-1);
        dependencyList[gate.subGateList[0].fusionGateSI.fusionIndex] =
            dependency;
    }
}

double cost(string gateType, int fusionSize, int targetQubit) {
    if (method < 4 || method == 5)
        return pow(cost_factor, (double)max(fusionSize - 1, 1));
    else if (method == 7 || method == 8) {
        if (fusionSize == 2)
            return gateTime[10 * Qubits + targetQubit];
        else if (fusionSize == 3)
            return gateTime[11 * Qubits + targetQubit];
        else if (gateType == "H")
            return gateTime[0 * Qubits + targetQubit];
        else if (gateType == "X")
            return gateTime[1 * Qubits + targetQubit];
        else if (gateType == "RX")
            return gateTime[2 * Qubits + targetQubit];
        else if (gateType == "RY")
            return gateTime[3 * Qubits + targetQubit];
        else if (gateType == "RZ")
            return gateTime[4 * Qubits + targetQubit];
        else if (gateType == "U1")
            return gateTime[5 * Qubits + targetQubit];
        else if (gateType == "CX")
            return gateTime[6 * Qubits + targetQubit];
        else if (gateType == "CZ")
            return gateTime[7 * Qubits + targetQubit];
        else if (gateType == "CP")
            return gateTime[8 * Qubits + targetQubit];
        else if (gateType == "RZZ")
            return gateTime[9 * Qubits + targetQubit];
        else if (gateType == "DONE")
            return gateTime[12 * Qubits + targetQubit];
        else if (gateType == "DTWO")
            return gateTime[13 * Qubits + targetQubit];
        else if (gateType == "DTHREE")
            return gateTime[14 * Qubits + targetQubit];
        else if (gateType == "DFOUR")
            return gateTime[15 * Qubits + targetQubit];
        else if (gateType == "DFIVE")
            return gateTime[16 * Qubits + targetQubit];
    } else {
        if (fusionSize == 2)
            return gateTime[10];
        else if (fusionSize == 3)
            return gateTime[11];
        else if (gateType == "H")
            return gateTime[0];
        else if (gateType == "X")
            return gateTime[1];
        else if (gateType == "RX")
            return gateTime[2];
        else if (gateType == "RY")
            return gateTime[3];
        else if (gateType == "RZ")
            return gateTime[4];
        else if (gateType == "U1")
            return gateTime[5];
        else if (gateType == "CX")
            return gateTime[6];
        else if (gateType == "CZ")
            return gateTime[7];
        else if (gateType == "CP")
            return gateTime[8];
        else if (gateType == "RZZ")
            return gateTime[9];
    }
    return 0;
}

class treeNode {
  public:
    treeNode *PNode;
    gateSI nodeSI;
    double executionTime;
    treeNode() : PNode(nullptr), nodeSI{0, 0}, executionTime(0) {}
    void setValue(treeNode *PNode, gateSI &gateSI, string gateType,
                  int targetQubit) {
        this->PNode = PNode;
        this->nodeSI = gateSI;
        // set weight
        this->executionTime =
            cost(gateType, this->nodeSI.fusionSize, targetQubit);
    }
    void getSmallWeight(vector<vector<fusionGate>> fusionGateList,
                        vector<vector<int>> dependencyList, double nowWeight,
                        double &smallWeight, vector<gateSI> nowGateList,
                        vector<gateSI> &executionGateList) {
        counter++;
        nowWeight += executionTime;
        nowGateList.push_back(nodeSI);
        if (PNode != nullptr) {
            auto &currentFusionList = fusionGateList[nodeSI.fusionSize - 1];
            auto it = find_if(
                currentFusionList.begin(), currentFusionList.end(),
                [this](const fusionGate &gate) {
                    return gate.fusionGateSI.fusionIndex == nodeSI.fusionIndex;
                });
            if (it != currentFusionList.end()) {
                deleteRelatedNode(fusionGateList, dependencyList, *it);
            }
        }
        if (fusionGateList[0].empty()) {
            if (nowWeight < smallWeight) {
                smallWeight = nowWeight;
                executionGateList = nowGateList;
            }
            return;
        }

        for (int nowFusionSize = maxFusionQuibits - 1; nowFusionSize >= 0;
             --nowFusionSize) {
            for (int i = 0; i < fusionGateList[nowFusionSize].size(); ++i) {
                if (nodeSI.fusionSize == nowFusionSize + 1 &&
                    nodeSI.fusionIndex > fusionGateList[nowFusionSize][i]
                                             .fusionGateSI.fusionIndex &&
                    (method != 0 && method != 2))
                    break;
                if (checkDependency(fusionGateList[nowFusionSize][i],
                                    dependencyList)) {
                    treeNode nextNode;
                    nextNode.setValue(
                        this, fusionGateList[nowFusionSize][i].fusionGateSI,
                        fusionGateList[nowFusionSize][i]
                            .subGateList[0]
                            .gateType,
                        fusionGateList[nowFusionSize][i]
                            .subGateList[0]
                            .targetQubit[0]);
                    nextNode.getSmallWeight(fusionGateList, dependencyList,
                                            nowWeight, smallWeight, nowGateList,
                                            executionGateList);
                }
            }
        }
        return;
    }
};

class gateLine {
  public:
    string line;
    string gateType;
    vector<int> targetQubit;

    gateLine(const string &line) : line(line) {
        vector<string> gateInfo;
        size_t pos = 0;
        string token;
        string tempLine = line;

        while ((pos = tempLine.find(' ')) != string::npos) {
            token = tempLine.substr(0, pos);
            gateInfo.push_back(token);
            tempLine.erase(0, pos + 1);
        }
        if (!tempLine.empty() && (isdigit(tempLine[0]) || tempLine[0] == '-')) {
            gateInfo.push_back(tempLine);
        }

        gateType = gateInfo[0];
        for (int i = 1; i <= targetQubitCounter(gateType); ++i) {
            targetQubit.push_back(stoi(gateInfo[i]));
        }
        sort(targetQubit.begin(), targetQubit.end());
    }

    gateLine(const string &gateType, const vector<int> &targetQubit)
        : gateType(gateType), targetQubit(targetQubit) {}

    void showInfo() const {
        cout << gateType;
        for (int qubit : targetQubit) {
            cout << " " << qubit;
        }
        cout << endl;
    }
};

void outputFusionCircuit(const string &outputFileName,
                         const vector<vector<fusionGate>> &fusionGateList,
                         const vector<gateSI> &executionGateList) {
    ofstream outputFile(outputFileName, ios_base::app);
    for (int index = 1; index < executionGateList.size(); ++index) {
        const auto &execGate = executionGateList[index];
        if (execGate.fusionSize > 1) {
            vector<int> targetQubits;
            const auto &pGate =
                fusionGateList[execGate.fusionSize - 1][execGate.fusionIndex];
            for (const auto &subGate : pGate.subGateList)
                for (int qubit : subGate.targetQubit)
                    if (find(targetQubits.begin(), targetQubits.end(), qubit) ==
                        targetQubits.end())
                        targetQubits.push_back(qubit);
            sort(targetQubits.begin(), targetQubits.end());
            outputFile << "U" << targetQubits.size() << " ";
            for (int qubit : targetQubits)
                outputFile << qubit << " ";
            for (size_t i = 0; i < pow(4, targetQubits.size()); ++i)
                outputFile << "3.141596 ";
            outputFile << endl;
        } else {
            const auto &subGate =
                fusionGateList[0][execGate.fusionIndex].subGateList[0];
            outputFile << subGate.gateType << " ";
            for (int qubit : subGate.targetQubit)
                outputFile << qubit << " ";
            for (float rotation : subGate.rotation)
                outputFile << rotation << " ";
            outputFile << endl;
        }
    }
    outputFile.close();
}

class DAG {
  public:
    int graphSize;
    vector<vector<pair<int, double>>> edge; // <destination, weight>
    DAG(int graphSize) : graphSize(graphSize + 1) {
        for (int i = 0; i < this->graphSize - 1; ++i)
            edge.push_back(vector<pair<int, double>>());
    }
    void addEdge(int source, int destination, double weight) {
        edge[source].push_back(make_pair(destination, weight));
    }
    void constructDAG(const vector<fusionGate> gateList) {
        for (int i = 0; i < gateList.size(); ++i) {
            // one qubit fusion edge
            addEdge(i, i + 1,
                    cost(gateList[i].subGateList[0].gateType, 1,
                         gateList[i].subGateList[0].targetQubit[0]));
            // muti qubit fusion edge
            for (int fusionSize = 2; fusionSize <= maxFusionQuibits;
                 ++fusionSize) {
                vector<int> qubit(gateList[i].subGateList[0].targetQubit);
                int nowIndex = i;
                while (nowIndex < gateList.size() - 1) {
                    for (int qubitIndex :
                         gateList[nowIndex + 1].subGateList[0].targetQubit)
                        if (find(qubit.begin(), qubit.end(), qubitIndex) ==
                            qubit.end())
                            qubit.push_back(qubitIndex);
                    if (qubit.size() <= fusionSize)
                        nowIndex++;
                    else
                        break;
                }
                if (nowIndex > i) {
                    sort(qubit.begin(), qubit.end());
                    addEdge(i, nowIndex + 1, cost("", fusionSize, qubit[0]));
                } else {
                    addEdge(i, -1, DBL_MAX);
                }
            }
        }
    }
    void shortestPath(const vector<fusionGate> &gateList,
                      const string &outputFileName) {
        struct DAGNode {
            int predecessor = -1;
            double distance = DBL_MAX;
            int fusionSize = 0;
        };

        // find shortestPath
        vector<DAGNode> nodeList(graphSize, DAGNode());
        nodeList[0].distance = 0;
        for (int i = 0; i < graphSize - 1; ++i) {
            for (int j = 0; j < edge[i].size(); ++j) {
                if (edge[i][j].first == -1)
                    continue;
                if (edge[i][j].second + nodeList[i].distance <
                    nodeList[edge[i][j].first].distance) {
                    nodeList[edge[i][j].first].distance =
                        edge[i][j].second + nodeList[i].distance;
                    nodeList[edge[i][j].first].fusionSize = j + 1;
                    nodeList[edge[i][j].first].predecessor = i;
                }
            }
        }

        // output fusion result
        ofstream outputFile(outputFileName, ios_base::app);
        vector<string> outputStr;
        int nowIndex = graphSize - 1;

        while (nowIndex > 0) {
            if (nodeList[nowIndex].fusionSize > 1) {
                vector<int> qubit;
                for (int i = nodeList[nowIndex].predecessor; i < nowIndex; ++i)
                    for (int qubitIndex :
                         gateList[i].subGateList[0].targetQubit)
                        if (find(qubit.begin(), qubit.end(), qubitIndex) ==
                            qubit.end())
                            qubit.push_back(qubitIndex);
                sort(qubit.begin(), qubit.end());
                string tmpStr = "U" + to_string(qubit.size());
                for (int qubitIndex : qubit)
                    tmpStr += " " + to_string(qubitIndex);
                for (int i = 0; i < pow(4, qubit.size()); ++i)
                    tmpStr += " 3.141596";
                outputStr.push_back(tmpStr);
            } else {
                int predecessorIndex = nodeList[nowIndex].predecessor;
                string tmpStr =
                    gateList[predecessorIndex].subGateList[0].gateType;
                for (int qubitIndex :
                     gateList[predecessorIndex].subGateList[0].targetQubit)
                    tmpStr += " " + to_string(qubitIndex);
                for (float rotation :
                     gateList[predecessorIndex].subGateList[0].rotation)
                    tmpStr += " " + to_string(rotation);
                outputStr.push_back(tmpStr);
            }
            nowIndex = nodeList[nowIndex].predecessor;
        }
        for (auto it = outputStr.rbegin(); it != outputStr.rend(); ++it)
            outputFile << *it << endl;
        outputFile.close();
    }
    void showInfo() const {
        cout << "--------------------------------------------------------"
             << endl;
        cout << "Gate number: " << graphSize - 1 << endl;
        for (int i = 0; i < edge.size(); ++i) {
            cout << "Source: " << i << endl;
            for (const auto &[destination, weight] : edge[i]) {
                cout << "    D: " << destination << "  W: " << weight << endl;
            }
        }
    }
};

class fusionList {
  public:
    struct info {
        int gateIndex;
        set<int> targetQubit;
        set<int> relatedQubit;
        int maxGateNumber = 0;
    };

    vector<info> infoList;

    fusionList(const vector<fusionGate> &gateList) {
        for (const auto &gate : gateList) {
            info tmpInfo;
            tmpInfo.gateIndex = gate.fusionGateSI.fusionIndex;
            for (int qubit : gate.subGateList[0].targetQubit) {
                tmpInfo.targetQubit.insert(qubit);
            }
            infoList.push_back(tmpInfo);
        }
    }

    void showInfoList() const {
        for (const auto &inf : infoList) {
            cout << inf.gateIndex << " |";
            for (int element : inf.targetQubit)
                cout << " " << element;
            cout << " |";
            for (int element : inf.relatedQubit)
                cout << " " << element;
            cout << " | " << inf.maxGateNumber << endl;
        }
    }

    void reNewList() {
        vector<vector<int>> qubitDependency(Qubits);
        vector<set<int>> preGate(Qubits);
        for (auto &inf : infoList) {
            inf.relatedQubit = inf.targetQubit;
            for (int element : inf.targetQubit) {
                inf.relatedQubit.insert(qubitDependency[element].begin(),
                                        qubitDependency[element].end());
            }
            set<int> allPreGate;
            for (int element : inf.relatedQubit) {
                qubitDependency[element].assign(inf.relatedQubit.begin(),
                                                inf.relatedQubit.end());
                preGate[element].insert(&inf - &infoList[0]);
                allPreGate.insert(preGate[element].begin(),
                                  preGate[element].end());
            }
            inf.maxGateNumber = allPreGate.size();
        }
    }

    void getFusedGate(vector<int> &gateToFused, int fusionSize) {
        set<int> fusionQubit;

        if (infoList[0].targetQubit.size() > fusionSize) {
            gateToFused.push_back(infoList[0].gateIndex);
            infoList.erase(infoList.begin());
            return;
        }

        while (fusionSize) {
            int nowMaxGateNumber = 0;
            set<int> tmpFusionQubit;
            for (const auto &inf : infoList) {
                if (inf.maxGateNumber > nowMaxGateNumber &&
                    inf.relatedQubit.size() <= fusionSize) {
                    nowMaxGateNumber = inf.maxGateNumber;
                    tmpFusionQubit = inf.relatedQubit;
                }
            }
            fusionSize -= tmpFusionQubit.size();
            fusionQubit.insert(tmpFusionQubit.begin(), tmpFusionQubit.end());

            for (auto it = infoList.begin(); it != infoList.end();) {
                if (includes(tmpFusionQubit.begin(), tmpFusionQubit.end(),
                             it->relatedQubit.begin(),
                             it->relatedQubit.end())) {
                    gateToFused.push_back(it->gateIndex);
                    it = infoList.erase(it);
                } else {
                    ++it;
                }
            }
            if (tmpFusionQubit.empty())
                break;
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc < 5) {
        cerr
            << "Usage: " << argv[0]
            << " [Input File] [Output File] [Max Fusion Qubits] [Total Qubits] "
               "[Mode]"
            << endl;
        return 1;
    }
    double constructFusionListTime = 0;
    clock_t tStart = clock();

    string inputFileName = argv[1];
    string outputFileName = argv[2];
    maxFusionQuibits = atoi(argv[3]);
    Qubits = atoi(argv[4]);
    if (argc == 6)
        method = atoi(argv[5]);
    vector<vector<fusionGate>> fusionGateList;

    if (method == 4 || method == 6 || method == 7 || method == 8) {
        ifstream gateTimeFile("./log/gate_exe_time.csv");
        for (string line; getline(gateTimeFile, line);)
            gateTime.push_back(stod(line));
    }

    // reorder
    ifstream inputFile(inputFileName);
    vector<gateLine> circuit;
    for (string line; getline(inputFile, line);) {
        gateLine tmpGateline(line);
        circuit.push_back(tmpGateline);
    }
    inputFile.close();

    string newCircuit;
    vector<queue<int>> qubitReorder(Qubits, queue<int>());
    for (int index = 0; index < circuit.size(); ++index) {
        int nextIndex = -1;
        for (int i = circuit[index].targetQubit.size() - 1; i >= 0; --i) {
            qubitReorder[circuit[index].targetQubit[i]].push(index);
            if (nextIndex != -1)
                qubitReorder[circuit[index].targetQubit[i]].push(nextIndex);
            nextIndex = circuit[index].targetQubit[i];
        }
        qubitReorder[circuit[index].targetQubit.back()].push(
            circuit[index].targetQubit.front());
    }

    int nowQubit = 0;
    while (1) {
        while (qubitReorder[nowQubit].size() == 0 && nowQubit < Qubits)
            nowQubit++;
        if (nowQubit == Qubits)
            break;
        int gateIndex = qubitReorder[nowQubit].front();
        qubitReorder[nowQubit].pop();
        int qubitIndex = qubitReorder[nowQubit].front();
        qubitReorder[nowQubit].pop();
        if (nowQubit >= qubitIndex)
            newCircuit += (circuit[gateIndex].line + "\n");
        nowQubit = qubitIndex;
    }
    ofstream outputFile("diagonal.txt");
    outputFile << newCircuit;
    outputFile.close();

    // diagonal fusion
    if (method > 4 && method != 7) {
        ifstream tmpInputFile("diagonal.txt");
        circuit.clear();
        for (string line; getline(tmpInputFile, line);) {
            gateLine tmpGateline(line);
            circuit.push_back(tmpGateline);
        }
        tmpInputFile.close();
        vector<int> targetQubitList;
        int addFlag = 0;
        for (int i = 0; i < circuit.size(); ++i) {
            if (circuit[i].gateType == "RZ") {
                auto firstInset =
                    find(targetQubitList.begin(), targetQubitList.end(),
                         circuit[i].targetQubit[0]) != targetQubitList.end();
                if (firstInset && targetQubitList.size() <= maxFusionQuibits) {
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else if (!firstInset &&
                           targetQubitList.size() <= maxFusionQuibits - 1) {
                    targetQubitList.push_back(circuit[i].targetQubit[0]);
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else {
                    addFlag++;
                }
            } else if (circuit[i].gateType == "CZ" ||
                       circuit[i].gateType == "CP" ||
                       circuit[i].gateType == "RZZ") {
                auto firstInset =
                    find(targetQubitList.begin(), targetQubitList.end(),
                         circuit[i].targetQubit[0]) != targetQubitList.end();
                auto secondInset =
                    find(targetQubitList.begin(), targetQubitList.end(),
                         circuit[i].targetQubit[1]) != targetQubitList.end();
                if (!firstInset && !secondInset &&
                    targetQubitList.size() <= maxFusionQuibits - 2) {
                    targetQubitList.push_back(circuit[i].targetQubit[0]);
                    targetQubitList.push_back(circuit[i].targetQubit[1]);
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else if (firstInset && !secondInset &&
                           targetQubitList.size() <= maxFusionQuibits - 1) {
                    targetQubitList.push_back(circuit[i].targetQubit[1]);
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else if ((firstInset ^ secondInset) &&
                           targetQubitList.size() <= maxFusionQuibits - 1) {
                    targetQubitList.push_back(
                        circuit[i].targetQubit[firstInset ? 1 : 0]);
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else {
                    addFlag++;
                }
            } else if (targetQubitList.size() > 0)
                addFlag++;
            if (addFlag && targetQubitList.size() > 0) {
                sort(targetQubitList.begin(), targetQubitList.end());
                gateLine newGate("D", targetQubitList);
                circuit.insert(circuit.begin() + i, newGate);
                addFlag = 0;
                targetQubitList.clear();
            }
        }

        if (!targetQubitList.empty()) {
            sort(targetQubitList.begin(), targetQubitList.end());
            gateLine newGate("D", targetQubitList);
            circuit.push_back(newGate);
        }
        ofstream tmpOutputFile("diagonal.txt");
        // add more if else if max qubit is larger than 5
        for (int i = 0; i < circuit.size(); ++i) {
            if (circuit[i].line != "")
                tmpOutputFile << circuit[i].line << endl;
            else if (circuit[i].gateType == "D") {
                if (circuit[i].targetQubit.size() == 1)
                    tmpOutputFile << "DONE";
                else if (circuit[i].targetQubit.size() == 2)
                    tmpOutputFile << "DTWO";
                else if (circuit[i].targetQubit.size() == 3)
                    tmpOutputFile << "DTHREE";
                else if (circuit[i].targetQubit.size() == 4)
                    tmpOutputFile << "DFOUR";
                else if (circuit[i].targetQubit.size() == 5)
                    tmpOutputFile << "DFIVE";
                for (int j = 0; j < circuit[i].targetQubit.size(); ++j)
                    tmpOutputFile << " "
                                  << to_string(circuit[i].targetQubit[j]);
                for (int j = 0; j < pow(2, circuit[i].targetQubit.size()); ++j)
                    tmpOutputFile << " 3.141596";
                tmpOutputFile << endl;
            }
        }
        tmpOutputFile.close();
    }

    // construct fusion list
    inputFile.open("diagonal.txt");
    for (int fusionQubits = 1; fusionQubits <= maxFusionQuibits;
         ++fusionQubits) {
        vector<fusionGate> NQubitFusionList;

        // one qubit fusionList
        if (fusionQubits == 1) {
            int gateIndex = 0;
            for (string line; getline(inputFile, line);) {
                vector<string> gateInfo;
                fusionGate bigFusionGate;
                fusionGate nowFusionGate(1, gateIndex);
                // read gate info
                size_t pos = 0;
                string token;
                while ((pos = line.find(' ')) != std::string::npos) {
                    token = line.substr(0, pos);
                    gateInfo.push_back(token);
                    line.erase(0, pos + 1);
                }
                if (line[0] > 47 || line[0] == 45)
                    gateInfo.push_back(line);
                // fill info to gateList
                bigFusionGate.fusionGateSI.fusionSize = 1;
                bigFusionGate.fusionGateSI.fusionIndex = gateIndex;
                nowFusionGate.gateType = gateInfo[0];
                nowFusionGate.fusionGateSI.fusionIndex = gateIndex;
                for (int i = 1;
                     i < targetQubitCounter(nowFusionGate.gateType) + 1; ++i)
                    nowFusionGate.targetQubit.push_back(stoi(gateInfo[i]));
                nowFusionGate.sortTargetQubit();
                for (int i = targetQubitCounter(nowFusionGate.gateType) + 1;
                     i < gateInfo.size(); ++i)
                    nowFusionGate.rotation.push_back(stof(gateInfo[i]));
                bigFusionGate.subGateList.push_back(nowFusionGate);
                NQubitFusionList.push_back(bigFusionGate);
                gateIndex++;
            }
            fusionGateList.push_back(NQubitFusionList);
            continue;
        }
        // muti qubit fusionList
        fusionList NowInfoList(fusionGateList[0]);
        NowInfoList.reNewList();
        int gateIndex = 0;
        while (!NowInfoList.infoList.empty()) {
            fusionGate bigFusionGate;
            bigFusionGate.fusionGateSI.fusionSize = fusionQubits;
            bigFusionGate.fusionGateSI.fusionIndex = gateIndex;
            vector<int> gateToFused;
            NowInfoList.getFusedGate(gateToFused, fusionQubits);
            for (int index : gateToFused) {
                bigFusionGate.subGateList.push_back(
                    fusionGateList[0][index].subGateList[0]);
            }
            NQubitFusionList.push_back(bigFusionGate);
            NowInfoList.reNewList();
            gateIndex++;
        }
        fusionGateList.push_back(NQubitFusionList);
    }
    inputFile.close();

    constructFusionListTime =
        static_cast<double>(clock() - tStart) / CLOCKS_PER_SEC;
    cout << "constructFusionListTime: " << constructFusionListTime << endl;
    tStart = clock();

    // do reorder
    for (int fusionQubits = 1; fusionQubits < maxFusionQuibits;
         ++fusionQubits) {
        vector<fusionGate> NQubitFusionList;
        vector<queue<int>> qubitReorder(Qubits, queue<int>());
        for (int index = 0; index < fusionGateList[fusionQubits].size();
             ++index) {
            vector<int> targetQubit;
            for (const auto &subGate :
                 fusionGateList[fusionQubits][index].subGateList)
                for (int qubit : subGate.targetQubit)
                    if (find(targetQubit.begin(), targetQubit.end(), qubit) ==
                        targetQubit.end())
                        targetQubit.push_back(qubit);
            sort(targetQubit.begin(), targetQubit.end());
            int nextIndex = -1;
            for (int i = targetQubit.size() - 1; i >= 0; --i) {
                qubitReorder[targetQubit[i]].push(
                    fusionGateList[fusionQubits][index]
                        .fusionGateSI.fusionIndex);
                if (nextIndex != -1)
                    qubitReorder[targetQubit[i]].push(nextIndex);
                nextIndex = targetQubit[i];
            }
            qubitReorder[targetQubit.back()].push(targetQubit.front());
        }

        int nowQubit = 0;
        while (1) {
            while (qubitReorder[nowQubit].size() == 0 && nowQubit < Qubits)
                nowQubit++;
            if (nowQubit == Qubits)
                break;
            int gateIndex = qubitReorder[nowQubit].front();
            qubitReorder[nowQubit].pop();
            int qubitIndex = qubitReorder[nowQubit].front();
            qubitReorder[nowQubit].pop();
            if (nowQubit >= qubitIndex)
                NQubitFusionList.push_back(
                    fusionGateList[fusionQubits][gateIndex]);
            nowQubit = qubitIndex;
        }
        for (int index = 0; index < NQubitFusionList.size(); ++index)
            NQubitFusionList[index].fusionGateSI.fusionIndex = index;
        fusionGateList[fusionQubits] = NQubitFusionList;
    }

    // find best fusion conbination
    // execute small gate block one by one to reduce execution time
    const string cmd = "rm " + outputFileName + " > /dev/null 2>&1";
    auto sysinfo = system(cmd.c_str());

    if (method > 1) {
        vector<vector<fusionGate>> subFusionGateList(maxFusionQuibits,
                                                     vector<fusionGate>());
        vector<vector<int>> recordIndex(maxFusionQuibits, vector<int>());
        for (int index = 0; index < fusionGateList[maxFusionQuibits - 1].size();
             ++index) {
            for (const auto &subGate :
                 fusionGateList[maxFusionQuibits - 1][index].subGateList) {
                subFusionGateList[0].push_back(
                    fusionGateList[0][subGate.fusionGateSI.fusionIndex]);
                for (int i = 0; i < maxFusionQuibits; ++i)
                    recordIndex[i].push_back(subGate.fusionGateSI.fusionIndex);
            }
            for (int fusionQubits = 1; fusionQubits < maxFusionQuibits - 1;
                 ++fusionQubits) {
                for (int subIndex = 0;
                     subIndex < fusionGateList[fusionQubits].size();
                     ++subIndex) {
                    int flag = 1;
                    vector<int> reIndex;
                    for (const auto &subGate :
                         fusionGateList[fusionQubits][subIndex].subGateList) {
                        auto it = find(recordIndex[fusionQubits].begin(),
                                       recordIndex[fusionQubits].end(),
                                       subGate.fusionGateSI.fusionIndex);
                        if (it == recordIndex[fusionQubits].end()) {
                            flag = 0;
                            recordIndex[fusionQubits].insert(
                                recordIndex[fusionQubits].end(),
                                reIndex.begin(), reIndex.end());
                            break;
                        }
                        reIndex.push_back(subGate.fusionGateSI.fusionIndex);
                        recordIndex[fusionQubits].erase(
                            remove(recordIndex[fusionQubits].begin(),
                                   recordIndex[fusionQubits].end(),
                                   subGate.fusionGateSI.fusionIndex),
                            recordIndex[fusionQubits].end());
                    }
                    if (flag)
                        subFusionGateList[fusionQubits].push_back(
                            fusionGateList[fusionQubits][subIndex]);
                }
            }
            subFusionGateList[maxFusionQuibits - 1].push_back(
                fusionGateList[maxFusionQuibits - 1][index]);

            int flag = 1;
            for (int fusionQubits = 1; fusionQubits < maxFusionQuibits - 1;
                 ++fusionQubits)
                if (!recordIndex[fusionQubits].empty())
                    flag = 0;
            if (flag) {
                // execute find weight
                treeNode nowNode;
                vector<gateSI> executionGateList;
                vector<vector<int>> dependencyList;
                constructDependencyList(subFusionGateList, dependencyList);
                double smallWeight = DBL_MAX;
                int smallCircuitSize = subFusionGateList[0].size();
                // choose different strategies with different circuit size
                if (smallCircuitSize < 26) {
                    nowNode.getSmallWeight(subFusionGateList, dependencyList, 0,
                                           smallWeight, executionGateList,
                                           executionGateList);
                    outputFusionCircuit(outputFileName, fusionGateList,
                                        executionGateList);
                } else {
                    DAG subDAG(smallCircuitSize);
                    subDAG.constructDAG(subFusionGateList[0]);
                    subDAG.shortestPath(subFusionGateList[0], outputFileName);
                }
                for (int i = 0; i < maxFusionQuibits; ++i) {
                    subFusionGateList[i].clear();
                    recordIndex[i].clear();
                }
            }
        }
    } else {
        treeNode nowNode;
        vector<vector<int>> dependencyList;
        constructDependencyList(fusionGateList, dependencyList);
        double smallWeight = DBL_MAX;
        vector<gateSI> executionGateList;
        nowNode.getSmallWeight(fusionGateList, dependencyList, 0, smallWeight,
                               executionGateList, executionGateList);
    }

    // some result
    double otherTime = static_cast<double>(clock() - tStart) / CLOCKS_PER_SEC;
    cout << "otherTime: " << otherTime << endl;
    cout << "total fusion time: " << otherTime + constructFusionListTime
         << endl;
}
