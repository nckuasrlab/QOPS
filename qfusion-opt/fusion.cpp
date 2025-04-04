#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <vector>
#include <map>

size_t maxFusionQuibits;
int Qubits;
int counter = 0;
int method = 0;
double cost_factor = 1.8;
std::vector<double> gateTime;

// gate size-index
struct gateSI {
    int fusionSize;
    int fusionIndex;
};

using Matrix = std::vector<std::vector<std::complex<double>>>;

int targetQubitCounter(std::string gateType) {
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
    std::string gateType;
    std::vector<int> targetQubit;
    std::vector<int> originalOrderQubit;
    std::vector<float> rotation;
    std::vector<fusionGate> subGateList;

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
        std::cout << fusionGateSI.fusionSize << "-" << fusionGateSI.fusionIndex
                  << "\n";
        for (const auto &subGate : subGateList)
            std::cout << subGate.fusionGateSI.fusionSize << "-"
                      << subGate.fusionGateSI.fusionIndex << " ";
        std::cout << "\n";
    }

    void showGateInfo() const {
        if (fusionGateSI.fusionSize > 1) {
            std::cout << "fusion size != 1"
                      << "\n";
            return;
        }
        std::cout << gateType << " ";
        for (const auto &qubit : targetQubit)
            std::cout << qubit << " ";
        for (const auto &rot : rotation)
            std::cout << rot << " ";
        std::cout << "\n";
    }

    void sortTargetQubit() { sort(targetQubit.begin(), targetQubit.end()); }
};

void printMat(Matrix mat) {
    for (const auto &row : mat) {
        for (const auto &val : row) {
            std::cout << /*std::setw(10) << "(" << */ val.real() << "+"
                      << val.imag() << "i, ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/* Return the matrix of gate which has the control qubit */
Matrix twoQubitsGateMat(fusionGate gate, std::vector<int> fusedQubits) {
    int sizeAfterExpand = pow(2, fusedQubits.size());
    std::vector<int> reorderQubits;
    for (auto i : gate.originalOrderQubit) {
        auto it = std::find(fusedQubits.begin(), fusedQubits.end(), i);
        if (it != fusedQubits.end())
            reorderQubits.push_back(std::distance(fusedQubits.begin(), it));
    }
    Matrix resMat(sizeAfterExpand,
                  std::vector<std::complex<double>>(sizeAfterExpand,
                                                    std::complex<double>()));
    if (gate.gateType == "CX") {
        for (int i = 0; i < sizeAfterExpand; i++)
            resMat[i][i] = 1;
        if (reorderQubits[0] >
            reorderQubits[1]) { // high qubit control low qubit
            uint32_t controlBit = 1 << reorderQubits[0];
            for (; controlBit < resMat.size(); controlBit += 2) {
                resMat[controlBit][controlBit] = 0;
                resMat[controlBit][controlBit + 1] = 1;
                resMat[controlBit + 1][controlBit + 1] = 0;
                resMat[controlBit + 1][controlBit] = 1;
            }
        } else { // low qubit control high qubit
            uint32_t controlledBit = 1 << reorderQubits[1];
            for (int i = 1; i < resMat.size(); i += 2) {
                resMat[i][i] = 0;
                resMat[i][i ^ controlledBit] = 1;
            }
        }
    } else if (gate.gateType == "CP") {
        for (int i = 0; i < sizeAfterExpand; i++)
            resMat[i][i] = 1;
        int higherQubit = std::max(reorderQubits[0], reorderQubits[1]);
        int lowerQubit = std::min(reorderQubits[0], reorderQubits[1]);
        uint32_t controlledBit = 1 << higherQubit;
        uint32_t targetBit = 1 << lowerQubit;
        uint32_t change_target = controlledBit | targetBit;
        for (int i = 0; i < resMat.size(); i++) {
            if ((i & change_target) == change_target)
                resMat[i][i] = {cos(gate.rotation[0]), sin(gate.rotation[0])};
        }
    } else if (gate.gateType == "CZ") {
        for (int i = 0; i < sizeAfterExpand; i++)
            resMat[i][i] = 1;
        int higherQubit = std::max(reorderQubits[0], reorderQubits[1]);
        int lowerQubit = std::min(reorderQubits[0], reorderQubits[1]);
        uint32_t controlledBit = 1 << higherQubit;
        uint32_t targetBit = 1 << lowerQubit;
        uint32_t change_target = controlledBit | targetBit;
        for (int i = 0; i < resMat.size(); i++) {
            if ((i & change_target) == change_target)
                resMat[i][i] = -1;
        }
    }
    return resMat;
}

/* Return the matrix of input gate */
Matrix gateMatrix(std::string gateType, float rotation, size_t fusedQubitNum) {
    Matrix mat(2, std::vector<std::complex<double>>());
    if (gateType == "X") {
        mat[0].assign({{0, 0}, {1, 0}});
        mat[1].assign({{1, 0}, {0, 0}});
    } else if (gateType == "Y") {
        mat[0].assign({{0, 0}, {0, -1}});
        mat[1].assign({{0, 1}, {0, 0}});
    } else if (gateType == "Z") {
        mat[0].assign({{1, 0}, {0, 0}});
        mat[1].assign({{0, 0}, {-1, 0}});
    } else if (gateType == "H") {
        mat[0].assign({{1 / sqrt(2), 0}, {1 / sqrt(2), 0}});
        mat[1].assign({{1 / sqrt(2), 0}, {-1 / sqrt(2), 0}});
    } else if (gateType == "RX") {
        mat[0].assign({{cos(rotation / 2), 0}, {0, -sin(rotation / 2)}});
        mat[1].assign({{0, -sin(rotation / 2)}, {cos(rotation / 2), 0}});
    } else if (gateType == "RY") {
        mat[0].assign({{cos(rotation / 2), 0}, {-sin(rotation / 2), 0}});
        mat[1].assign({{sin(rotation / 2), 0}, {cos(rotation / 2), 0}});
    } else if (gateType == "RZ") {
        mat[0].assign({{cos(-rotation / 2), sin(-rotation / 2)}, {0, 0}});
        mat[1].assign({{0, 0}, {cos(rotation / 2), sin(rotation / 2)}});
    } else if (gateType == "RZZ") {
        mat.push_back(std::vector<std::complex<double>>());
        mat.push_back(std::vector<std::complex<double>>());
        mat[0].assign(
            {{cos(-rotation / 2), sin(-rotation / 2)}, {0, 0}, {0, 0}, {0, 0}});
        mat[1].assign(
            {{0, 0}, {cos(rotation / 2), sin(rotation / 2)}, {0, 0}, {0, 0}});
        mat[2].assign(
            {{0, 0}, {0, 0}, {cos(rotation / 2), sin(rotation / 2)}, {0, 0}});
        mat[3].assign(
            {{0, 0}, {0, 0}, {0, 0}, {cos(-rotation / 2), sin(-rotation / 2)}});
    } else if (gateType == "I2") {
        mat[0].assign({{1, 0}, {0, 0}});
        mat[1].assign({{0, 0}, {1, 0}});
    }
    return mat;
}

Matrix tensorProduct(Matrix matA, Matrix matB) {
    int resRow = matA.size() * matB.size();
    int resCol = matA[0].size() * matB[0].size();
    Matrix resMat(resRow, std::vector<std::complex<double>>(resCol, {0, 0}));

    for (int rowA = 0; rowA < matA.size(); rowA++) {
        for (int colA = 0; colA < matA[0].size(); colA++) {
            std::complex<double> elemA = matA[rowA][colA];
            for (int rowB = 0; rowB < matB.size(); rowB++) {
                for (int colB = 0; colB < matB[0].size(); colB++) {
                    std::complex<double> elemB = matB[rowB][colB];
                    int newRow = rowA * matB.size() + rowB;
                    int newCol = colA * matB[0].size() + colB;
                    resMat[newRow][newCol] = elemA * elemB;
                }
            }
        }
    }
    return resMat;
}

/* Expand the size of gate to specific size */
Matrix gateExpansion(const fusionGate targetGate,
                     std::vector<int> targetQubits) {
    // deal with the gate before the qubits which is not occupied by the
    // targetGate
    Matrix A, B;
    std::vector<std::string> expandedGateList;
    float rotation = targetGate.rotation.empty() ? 0 : targetGate.rotation[0];
#ifdef DEBUG
    std::cout << targetGate.gateType << " ";
    for (auto i : targetGate.targetQubit)
        std::cout << i << " ";
    std::cout << std::endl;
#endif
    /* If targetGate is control gate like CX, CZ and CP, get the expanded matrix
     * from twoQubitsGateMat().(Because they can't be expanded from tensor
     * product.) */
    if (targetGate.gateType[0] == 'C')
        return twoQubitsGateMat(targetGate, targetQubits);
    /* Use tensor product to expand a gate. Notice that we have to calculate it
     * from MSB to LSB */
    for (int i = 0; i < targetQubits.size(); i++) {
        if (find(targetGate.targetQubit.begin(), targetGate.targetQubit.end(),
                 targetQubits[i]) == targetGate.targetQubit.end())
            expandedGateList.push_back("I2");
        else
            expandedGateList.push_back(targetGate.gateType);
    }
    for (int i = expandedGateList.size() - 1; i >= 0; i--) {
#ifdef DEBUG
        std::cout << expandedGateList[i] << " ";
#endif

        B = gateMatrix(expandedGateList[i], rotation, targetQubits.size());
        if (i == expandedGateList.size() - 1)
            A = B;
        else {
            A = tensorProduct(A, B);
        }
    }
#ifdef DEBUG
    std::cout << std::endl;
    printMat(A);
    std::cout << "=======gateExpansion result========\n";
#endif
    return A;
}

/* Implement the matrix multiplication */
Matrix matrixMul(Matrix matA, Matrix matB) {
    Matrix resMat(matA.size(),
                  std::vector<std::complex<double>>(matB[0].size(), {0, 0}));
    for (int i = 0; i < matA.size(); i++) {
        for (int j = 0; j < matB[0].size(); j++) {
            std::complex<double> tmp(0, 0);
            for (int k = 0; k < matB.size(); k++) {
                tmp += (matA[i][k] * matB[k][j]);
            }
            resMat[i][j] = tmp;
        }
    }
    return resMat;
}

/* Calculate the result matrix of gate fusion */
Matrix calculateFusionGate(std::vector<fusionGate> subGateList,
                           std::vector<int> targetQubits) {
    Matrix resGate;
    Matrix expandedGate;
    std::sort(subGateList.begin(), subGateList.end(),
              [](const fusionGate &a, const fusionGate &b) {
                  return a.fusionGateSI.fusionIndex <
                         b.fusionGateSI.fusionIndex;
              });
#ifdef DEBUG_FUSION
    for (int i = 0; i < subGateList.size(); i++) {
        std::cout << subGateList[i].gateType << " ";
        for (auto x : subGateList[i].targetQubit)
            std::cout << x << " ";
        std::cout << std::endl;
    }
#endif
    /* MUL_REVERSE is for testing the order of matrix multiplication. For
     * example, if the subGateList is H, X, Y. In the original method, it would
     * be H * X * Y. But MUL_REVERSE will reverse the multiplication, it would
     * be Y * X * H. */
#ifdef MUL_REVERSE
    for (int i = subGateList.size() - 1; i >= 0; i--) {
#endif
#ifndef MUL_REVERSE
        for (int i = 0; i < subGateList.size(); i++) {
#endif
            expandedGate = gateExpansion(subGateList[i], targetQubits);
#ifdef MUL_REVERSE
            if (i == subGateList.size() - 1) {
#endif
#ifndef MUL_REVERSE
            if (i == 0) {
#endif
                resGate = expandedGate;
                continue;
            }
#ifdef DEBUG
            std::cout << subGateList[i + 1].gateType << ":\n";
            printMat(resGate);
            std::cout << subGateList[i].gateType << ":\n";
            printMat(expandedGate);
            std::cout << "-----------matrixMul----------\n";
#endif
            resGate = matrixMul(resGate, expandedGate);
#ifdef DEBUG
            printMat(resGate);
            std::cout << "\n====================================\n";
#endif
        }
#ifdef DEBUG_FUSION
        std::cout << "===========================\n";
#endif
#ifdef DEBUG
        std::cout << "============final UGate matrix============\n";
        printMat(resGate);
        std::cout << "============final UGate matrix============\n";
#endif
        return resGate;
}

void showFusionGateList(
    const std::vector<std::vector<fusionGate>> &fusionGateList, int start,
    int end) {
    for (int i = start - 1; i < end; ++i) {
        std::cout << "Number of fusion qubits: " << i + 1 << "\n";
        for (const auto &gate : fusionGateList[i]) {
            std::cout << "============================================="
                         "============="
                         "======"
                         "======="
                      << "\n";
            gate.dump();
            for (const auto &subGate : gate.subGateList)
                subGate.showGateInfo();
        }
        std::cout << "\n";
    }
}

void showDependencyList(const std::vector<std::vector<int>> &dependencyList) {
    for (size_t i = 0; i < dependencyList.size(); ++i) {
        std::cout << i << " || ";
        for (size_t j = 0; j < dependencyList[i].size(); ++j) {
            if (dependencyList[i][j] > -1)
                std::cout << dependencyList[i][j] << " ";
            else
                std::cout << "NULL";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// check dependency 1:can execute 0:have dependency -1:other
int checkDependency(fusionGate &bigFusionGate,
                    std::vector<std::vector<int>> dependencyList) {
    for (size_t i = 0; i < bigFusionGate.subGateList.size(); ++i) {
        if (dependencyList[bigFusionGate.subGateList[i]
                               .fusionGateSI.fusionIndex][0] == -1)
            for (size_t j = 0; j < dependencyList.size(); ++j) {
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

void deleteRelatedNode(std::vector<std::vector<fusionGate>> &fusionGateList,
                       std::vector<std::vector<int>> &dependencyList,
                       fusionGate bigFusionGate) {
    // delete node in dependency list
    for (size_t i = 0; i < bigFusionGate.subGateList.size(); ++i)
        for (size_t index = 0; index < dependencyList.size(); ++index) {
            dependencyList[index].erase(
                remove(dependencyList[index].begin(),
                       dependencyList[index].end(),
                       bigFusionGate.subGateList[i].fusionGateSI.fusionIndex),
                dependencyList[index].end());
            if (dependencyList[index].empty())
                dependencyList[index].push_back(-1);
        }

    // delete node in fusionGate list
    for (size_t nowFusionSize = 0; nowFusionSize < maxFusionQuibits;
         ++nowFusionSize)
        for (size_t index = 0; index < fusionGateList[nowFusionSize].size();
             ++index) {
            int flag = 0;
            for (size_t i = 0;
                 i < fusionGateList[nowFusionSize][index].subGateList.size();
                 ++i) {
                for (size_t j = 0; j < bigFusionGate.subGateList.size(); ++j)
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

void constructDependencyList(
    std::vector<std::vector<fusionGate>> &fusionGateList,
    std::vector<std::vector<int>> &dependencyList) {
    int maxIndex = 0;
    std::vector<int> gateOnQubit(Qubits, -1);
    for (const auto &gate : fusionGateList[0])
        maxIndex =
            std::max(maxIndex, gate.subGateList[0].fusionGateSI.fusionIndex);
    for (int i = 0; i <= maxIndex; ++i)
        dependencyList.push_back(std::vector<int>{-1});
    for (const auto &gate : fusionGateList[0]) {
        std::vector<int> dependency;
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

double cost(std::string gateType, int fusionSize, int targetQubit) {
    if (method < 4 || method == 5)
        return pow(cost_factor, (double)std::max(fusionSize - 1, 1));
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
    void setValue(treeNode *PNode, gateSI &gateSI, std::string gateType,
                  int targetQubit) {
        this->PNode = PNode;
        this->nodeSI = gateSI;
        // set weight
        this->executionTime =
            cost(gateType, this->nodeSI.fusionSize, targetQubit);
    }
    void getSmallWeight(std::vector<std::vector<fusionGate>> fusionGateList,
                        std::vector<std::vector<int>> dependencyList,
                        double nowWeight, double &smallWeight,
                        std::vector<gateSI> nowGateList,
                        std::vector<gateSI> &executionGateList) {
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
            for (size_t i = 0; i < fusionGateList[nowFusionSize].size(); ++i) {
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
    std::string line;
    std::string gateType;
    std::vector<int> targetQubit;

    gateLine(const std::string &line) : line(line) {
        std::vector<std::string> gateInfo;
        size_t pos = 0;
        std::string token;
        std::string tempLine = line;

        while ((pos = tempLine.find(' ')) != std::string::npos) {
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

    gateLine(const std::string &gateType, const std::vector<int> &targetQubit)
        : gateType(gateType), targetQubit(targetQubit) {}

    void showInfo() const {
        std::cout << gateType;
        for (int qubit : targetQubit) {
            std::cout << " " << qubit;
        }
        std::cout << "\n";
    }
};

void outputFusionCircuit(
    const std::string &outputFileName,
    const std::vector<std::vector<fusionGate>> &fusionGateList,
    const std::vector<gateSI> &executionGateList) {
    std::ofstream outputFile(outputFileName, std::ios_base::app);
    for (size_t index = 1; index < executionGateList.size(); ++index) {
        const auto &execGate = executionGateList[index];
        if (execGate.fusionSize > 1) {
            std::vector<int> targetQubits;
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
            Matrix fusedGateMat =
                calculateFusionGate(pGate.subGateList, targetQubits);
            for (size_t matRow = 0; matRow < fusedGateMat.size(); matRow++) {
                for (const auto elem : fusedGateMat[matRow])
                    outputFile << elem.real() << " " << elem.imag() << " ";
            }
            outputFile << "\n";
        } else {
            const auto &subGate =
                fusionGateList[0][execGate.fusionIndex].subGateList[0];
            outputFile << subGate.gateType << " ";
            for (int qubit : subGate.targetQubit)
                outputFile << qubit << " ";
            for (float rotation : subGate.rotation)
                outputFile << rotation << " ";
            outputFile << "\n";
        }
    }
    outputFile.close();
}
class DAG {
  public:
    int graphSize;
    std::vector<std::vector<std::pair<int, double>>>
        edge; // <destination, weight>
    DAG(int graphSize) : graphSize(graphSize + 1) {
        for (int i = 0; i < this->graphSize - 1; ++i)
            edge.push_back(std::vector<std::pair<int, double>>());
    }
    void addEdge(int source, int destination, double weight) {
        edge[source].push_back(std::make_pair(destination, weight));
    }
    void constructDAG(const std::vector<fusionGate> gateList) {
        for (size_t i = 0; i < gateList.size(); ++i) {
            // one qubit fusion edge
            addEdge(i, i + 1,
                    cost(gateList[i].subGateList[0].gateType, 1,
                         gateList[i].subGateList[0].targetQubit[0]));
            // muti qubit fusion edge
            for (size_t fusionSize = 2; fusionSize <= maxFusionQuibits;
                 ++fusionSize) {
                std::vector<int> qubit(gateList[i].subGateList[0].targetQubit);
                size_t nowIndex = i;
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
    void shortestPath(const std::vector<fusionGate> &gateList,
                      const std::string &outputFileName) {
        struct DAGNode {
            int predecessor = -1;
            double distance = DBL_MAX;
            int fusionSize = 0;
        };

        // find shortestPath
        std::vector<DAGNode> nodeList(graphSize, DAGNode());
        nodeList[0].distance = 0;
        for (int i = 0; i < graphSize - 1; ++i) {
            for (size_t j = 0; j < edge[i].size(); ++j) {
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
        std::ofstream outputFile(outputFileName, std::ios_base::app);
        std::vector<std::string> outputStr;
        int nowIndex = graphSize - 1;

        while (nowIndex > 0) {
            if (nodeList[nowIndex].fusionSize > 1) {
                std::vector<int> qubit;
                for (int i = nodeList[nowIndex].predecessor; i < nowIndex; ++i)
                    for (int qubitIndex :
                         gateList[i].subGateList[0].targetQubit)
                        if (find(qubit.begin(), qubit.end(), qubitIndex) ==
                            qubit.end())
                            qubit.push_back(qubitIndex);
                sort(qubit.begin(), qubit.end());
                std::string tmpStr = "U" + std::to_string(qubit.size());
                for (int qubitIndex : qubit)
                    tmpStr += " " + std::to_string(qubitIndex);

                Matrix fusedGateMat;
                std::vector<fusionGate> gateListForUGate;
                for (int i = nodeList[nowIndex].predecessor; i < nowIndex; ++i)
                    // for (auto subGate : gateList[i].subGateList)
                    gateListForUGate.push_back(gateList[i].subGateList[0]);
                fusedGateMat = calculateFusionGate(gateListForUGate, qubit);
                for (size_t matRow = 0; matRow < fusedGateMat.size(); matRow++)
                    for (const auto elem : fusedGateMat[matRow])
                        tmpStr += " " + std::to_string(elem.real()) + " " +
                                  std::to_string(elem.imag());
                outputStr.push_back(tmpStr);
            } else {
                int predecessorIndex = nodeList[nowIndex].predecessor;
                std::string tmpStr =
                    gateList[predecessorIndex].subGateList[0].gateType;
                for (int qubitIndex :
                     gateList[predecessorIndex].subGateList[0].targetQubit)
                    tmpStr += " " + std::to_string(qubitIndex);
                for (float rotation :
                     gateList[predecessorIndex].subGateList[0].rotation)
                    tmpStr += " " + std::to_string(rotation);
                outputStr.push_back(tmpStr);
            }
            nowIndex = nodeList[nowIndex].predecessor;
        }
        for (auto it = outputStr.rbegin(); it != outputStr.rend(); ++it)
            outputFile << *it << "\n";
        outputFile.close();
    }
    void showInfo() const {
        std::cout << "-------------------------------------------------"
                     "-------"
                  << "\n";
        std::cout << "Gate number: " << graphSize - 1 << "\n";
        for (size_t i = 0; i < edge.size(); ++i) {
            std::cout << "Source: " << i << "\n";
            for (const auto &[destination, weight] : edge[i]) {
                std::cout << "    D: " << destination << "  W: " << weight
                          << "\n";
            }
        }
    }
};

class fusionList {
  public:
    struct info {
        int gateIndex;
        std::set<int> targetQubit;
        std::set<int> relatedQubit;
        int maxGateNumber = 0;
    };

    std::vector<info> infoList;

    fusionList(const std::vector<fusionGate> &gateList) {
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
            std::cout << inf.gateIndex << " |";
            for (int element : inf.targetQubit)
                std::cout << " " << element;
            std::cout << " |";
            for (int element : inf.relatedQubit)
                std::cout << " " << element;
            std::cout << " | " << inf.maxGateNumber << "\n";
        }
    }

    void reNewList() {
        std::vector<std::vector<int>> qubitDependency(Qubits);
        std::vector<std::set<int>> preGate(Qubits);
        for (auto &inf : infoList) {
            inf.relatedQubit = inf.targetQubit;
            for (int element : inf.targetQubit) {
                inf.relatedQubit.insert(qubitDependency[element].begin(),
                                        qubitDependency[element].end());
            }
            std::set<int> allPreGate;
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

    void getFusedGate(std::vector<int> &gateToFused, size_t fusionSize) {
        std::set<int> fusionQubit;

        if (infoList[0].targetQubit.size() > fusionSize) {
            gateToFused.push_back(infoList[0].gateIndex);
            infoList.erase(infoList.begin());
            return;
        }

        while (fusionSize) {
            int nowMaxGateNumber = 0;
            std::set<int> tmpFusionQubit;
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

inline void GetPGFS(std::vector<std::vector<fusionGate>> &fusionGateList) {
    // construct fusion list
    std::vector<fusionGate> NQubitFusionList;
    int gateIndex = 0;
    // one qubit fusionList
    std::ifstream inputFile("diagonal.txt");
    for (std::string line; std::getline(inputFile, line);) {
        std::vector<std::string> gateInfo;
        fusionGate bigFusionGate;
        fusionGate nowFusionGate(1, gateIndex);
        // read gate info
        size_t pos = 0;
        std::string token;
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
        for (int i = 1; i < targetQubitCounter(nowFusionGate.gateType) + 1;
             ++i) {
            nowFusionGate.targetQubit.push_back(stoi(gateInfo[i]));
            nowFusionGate.originalOrderQubit.push_back(stoi(gateInfo[i]));
        }
        nowFusionGate.sortTargetQubit();
        for (size_t i = targetQubitCounter(nowFusionGate.gateType) + 1;
             i < gateInfo.size(); ++i)
            nowFusionGate.rotation.push_back(stof(gateInfo[i]));
        bigFusionGate.subGateList.push_back(nowFusionGate);
        NQubitFusionList.push_back(bigFusionGate);
        gateIndex++;
    }
    inputFile.close();
    fusionGateList.push_back(NQubitFusionList);

    // muti qubit fusionList
    for (size_t fusionQubits = 2; fusionQubits <= maxFusionQuibits;
         ++fusionQubits) {
        std::vector<fusionGate> NQubitFusionList;
        fusionList NowInfoList(fusionGateList[0]); // BuildGateInfoList
        NowInfoList.reNewList();
        int gateIndex = 0;
        while (!NowInfoList.infoList.empty()) {
            fusionGate bigFusionGate;
            bigFusionGate.fusionGateSI.fusionSize = fusionQubits;
            bigFusionGate.fusionGateSI.fusionIndex = gateIndex;
            std::vector<int> gateToFused;
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
}

inline void
GetOptimalGFS(std::string &outputFileName,
              std::vector<std::vector<fusionGate>> &fusionGateList) {
    // find best fusion conbination
    // execute small gate block one by one to reduce execution time
    const std::string cmd = "rm " + outputFileName + " > /dev/null 2>&1";
    [[maybe_unused]] auto sysinfo = system(cmd.c_str());

    if (method > 1) {
        std::vector<std::vector<fusionGate>> subFusionGateList(
            maxFusionQuibits, std::vector<fusionGate>());
        std::vector<std::vector<int>> recordIndex(maxFusionQuibits,
                                                  std::vector<int>());
        for (size_t index = 0;
             index < fusionGateList[maxFusionQuibits - 1].size(); ++index) {
            for (const auto &subGate :
                 fusionGateList[maxFusionQuibits - 1][index].subGateList) {
                subFusionGateList[0].push_back(
                    fusionGateList[0][subGate.fusionGateSI.fusionIndex]);
                for (size_t i = 0; i < maxFusionQuibits; ++i)
                    recordIndex[i].push_back(subGate.fusionGateSI.fusionIndex);
            }
            for (size_t fusionQubits = 1; fusionQubits < maxFusionQuibits - 1;
                 ++fusionQubits) {
                for (size_t subIndex = 0;
                     subIndex < fusionGateList[fusionQubits].size();
                     ++subIndex) {
                    int flag = 1;
                    std::vector<int> reIndex;
                    for (const auto &subGate :
                         fusionGateList[fusionQubits][subIndex].subGateList) {
                        // Check if all fusion indices of the sub-gates are a 
                        // subset of the recordIndex for this fusion size.
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
            for (size_t fusionQubits = 1; fusionQubits < maxFusionQuibits - 1;
                 ++fusionQubits)
                if (!recordIndex[fusionQubits].empty())
                    flag = 0;
            if (flag) {
                // execute find weight
                treeNode nowNode;
                std::vector<gateSI> executionGateList;
                std::vector<std::vector<int>> dependencyList;
                constructDependencyList(subFusionGateList, dependencyList);
                double smallWeight = DBL_MAX;
                size_t smallCircuitSize = subFusionGateList[0].size();
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
                for (size_t i = 0; i < maxFusionQuibits; ++i) {
                    subFusionGateList[i].clear();
                    recordIndex[i].clear();
                }
            }
        }
    } else {
        treeNode nowNode;
        std::vector<std::vector<int>> dependencyList;
        constructDependencyList(fusionGateList, dependencyList);
        double smallWeight = DBL_MAX;
        std::vector<gateSI> executionGateList;
        nowNode.getSmallWeight(fusionGateList, dependencyList, 0, smallWeight,
                               executionGateList, executionGateList);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr
            << "Usage: " << argv[0]
            << " [Input File] [Output File] [Max Fusion Qubits] " "[Total Qubits] "
               "[Mode]"
            << "\n";
        return 1;
    }
    auto total_time_start = std::chrono::steady_clock::now();
    std::unordered_map<std::string, double> timers;

    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];
    maxFusionQuibits = atoi(argv[3]);
    Qubits = atoi(argv[4]);
    if (argc == 6)
        method = atoi(argv[5]);

    if (method == 4 || method == 6 || method == 7 || method == 8) {
        std::ifstream gateTimeFile("./log/gate_exe_time.csv");
        if (!gateTimeFile) {
            std::cerr << "Error: Could not open the file "
                         "(./log/gate_exe_time.csv)!\n";
            std::cerr << "Please run python/performance_model_{SIMULATOR}.py first\n";
            return 1;
        }
        for (std::string line; getline(gateTimeFile, line);)
            gateTime.push_back(stod(line));
    }

    // reorder
    auto time_start = std::chrono::steady_clock::now();
    std::ifstream inputFile(inputFileName);
    std::vector<gateLine> circuit;
    for (std::string line; getline(inputFile, line);) {
        gateLine tmpGateline(line);
        circuit.push_back(tmpGateline);
    }
    inputFile.close();

    std::string newCircuit;
    std::vector<std::queue<int>> qubitReorder(Qubits, std::queue<int>());
    for (size_t index = 0; index < circuit.size(); ++index) {
        int nextIndex = -1;
        for (auto it = circuit[index].targetQubit.rbegin();
             it != circuit[index].targetQubit.rend(); ++it) {
            qubitReorder[*it].push(index);
            if (nextIndex != -1)
                qubitReorder[*it].push(nextIndex);
            nextIndex = *it;
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
    std::ofstream outputFile("diagonal.txt");
    outputFile << newCircuit;
    outputFile.close();
    auto time_end = std::chrono::steady_clock::now();
    timers["reorder"] =
        std::chrono::duration<double>(time_end - time_start).count();

    // diagonal fusion
    time_start = std::chrono::steady_clock::now();
    if (method > 4 && method != 7) {
        std::ifstream tmpInputFile("diagonal.txt");
        circuit.clear();
        for (std::string line; getline(tmpInputFile, line);) {
            gateLine tmpGateline(line);
            circuit.push_back(tmpGateline);
        }
        tmpInputFile.close();
        std::vector<int> targetQubitList;
        // use hash table to record the gates fused into a diagonal gate
        std::map<gateLine *, std::vector<fusionGate>> fusedGatesRecord;
        int addFlag = 0;
        std::vector<fusionGate> subGateList;
        for (size_t i = 0; i < circuit.size(); ++i) {
            fusionGate subGate;
            int ignoredValue;
            float rotation = 0.0;
            std::istringstream gateParser(circuit[i].line);
            // Parsing the input string to fusionGate for the usage of
            // calculating the fused result of diagonal gate.
            if (circuit[i].gateType == "RZ")
                gateParser >> subGate.gateType >> ignoredValue >> rotation;
            else if (circuit[i].gateType == "CZ")
                gateParser >> subGate.gateType;
            else if (circuit[i].gateType == "CP" ||
                     circuit[i].gateType == "RZZ")
                gateParser >> subGate.gateType >> ignoredValue >>
                    ignoredValue >> rotation;
            subGate.rotation.push_back(rotation);

            if (circuit[i].gateType == "RZ") {
                auto firstInset =
                    find(targetQubitList.begin(), targetQubitList.end(),
                         circuit[i].targetQubit[0]) != targetQubitList.end();
                if (firstInset && targetQubitList.size() <= maxFusionQuibits) {
                    subGateList.push_back(subGate);
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else if (!firstInset &&
                           targetQubitList.size() <= maxFusionQuibits - 1) {
                    targetQubitList.push_back(circuit[i].targetQubit[0]);
                    subGateList.push_back(subGate);
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
                    subGateList.push_back(subGate);
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else if (firstInset && !secondInset &&
                           targetQubitList.size() <= maxFusionQuibits - 1) {
                    targetQubitList.push_back(circuit[i].targetQubit[1]);
                    subGateList.push_back(subGate);
                    circuit.erase(circuit.begin() + i);
                    i--;
                } else if ((firstInset ^ secondInset) &&
                           targetQubitList.size() <= maxFusionQuibits - 1) {
                    targetQubitList.push_back(
                        circuit[i].targetQubit[firstInset ? 1 : 0]);
                    subGateList.push_back(subGate);
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
                fusedGatesRecord[&*(circuit.begin() + i)] = subGateList;
                subGateList.clear();
                addFlag = 0;
                targetQubitList.clear();
            }
        }

        if (!targetQubitList.empty()) {
            sort(targetQubitList.begin(), targetQubitList.end());
            gateLine newGate("D", targetQubitList);
            circuit.push_back(newGate);
        }
        std::ofstream tmpOutputFile("diagonal.txt");
        // add more if else if max qubit is larger than 5
        for (size_t i = 0; i < circuit.size(); ++i) {
            if (circuit[i].line != "")
                tmpOutputFile << circuit[i].line << "\n";
            else if (circuit[i].gateType == "D") {
                if (circuit[i].targetQubit.size() == 1)
                    tmpOutputFile << "D1";
                else if (circuit[i].targetQubit.size() == 2)
                    tmpOutputFile << "D2";
                else if (circuit[i].targetQubit.size() == 3)
                    tmpOutputFile << "D3";
                else if (circuit[i].targetQubit.size() == 4)
                    tmpOutputFile << "D4";
                else if (circuit[i].targetQubit.size() == 5)
                    tmpOutputFile << "D5";
                for (size_t j = 0; j < circuit[i].targetQubit.size(); ++j)
                    tmpOutputFile << " "
                                  << std::to_string(circuit[i].targetQubit[j]);
                Matrix fusedDGate = calculateFusionGate(
                    fusedGatesRecord[&circuit[i]], circuit[i].targetQubit);
                for (size_t matRow = 0; matRow < fusedDGate.size(); matRow++) {
                    for (const auto elem : fusedDGate[matRow])
                        tmpOutputFile << " " << elem.real() << " "
                                      << elem.imag();
                }
                tmpOutputFile << "\n";
            }
        }
        tmpOutputFile.close();
    }
    time_end = std::chrono::steady_clock::now();
    timers["diagonal"] =
        std::chrono::duration<double>(time_end - time_start).count();

    std::vector<std::vector<fusionGate>> fusionGateList;
    time_start = std::chrono::steady_clock::now();
    GetPGFS(fusionGateList);
    time_end = std::chrono::steady_clock::now();
    timers["GetPGFS"] =
        std::chrono::duration<double>(time_end - time_start).count();

    // do reorder
    time_start = std::chrono::steady_clock::now();
    for (size_t fusionQubits = 1; fusionQubits < maxFusionQuibits;
         ++fusionQubits) {
        std::vector<fusionGate> NQubitFusionList;
        std::vector<std::queue<int>> qubitReorder(Qubits, std::queue<int>());
        for (size_t index = 0; index < fusionGateList[fusionQubits].size();
             ++index) {
            std::vector<int> targetQubit;
            for (const auto &subGate :
                 fusionGateList[fusionQubits][index].subGateList)
                for (int qubit : subGate.targetQubit)
                    if (find(targetQubit.begin(), targetQubit.end(), qubit) ==
                        targetQubit.end())
                        targetQubit.push_back(qubit);
            sort(targetQubit.begin(), targetQubit.end());
            int nextIndex = -1;

            for (auto it = targetQubit.rbegin(); it != targetQubit.rend();
                 ++it) {
                qubitReorder[*it].push(fusionGateList[fusionQubits][index]
                                           .fusionGateSI.fusionIndex);
                if (nextIndex != -1)
                    qubitReorder[*it].push(nextIndex);
                nextIndex = *it;
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
        for (size_t index = 0; index < NQubitFusionList.size(); ++index)
            NQubitFusionList[index].fusionGateSI.fusionIndex = index;
        fusionGateList[fusionQubits] = NQubitFusionList;
    }
    time_end = std::chrono::steady_clock::now();
    timers["reorder2"] =
        std::chrono::duration<double>(time_end - time_start).count();

    time_start = std::chrono::steady_clock::now();
    GetOptimalGFS(outputFileName, fusionGateList);
    time_end = std::chrono::steady_clock::now();
    timers["GetOptimalGFS"] =
        std::chrono::duration<double>(time_end - time_start).count();
    auto total_time_end = std::chrono::steady_clock::now();
    timers["total"] =
        std::chrono::duration<double>(total_time_end - total_time_start)
            .count();

    // some result
    std::vector time_keys{"reorder", "diagonal", "reorder2", "GetPGFS",
                          "GetOptimalGFS"};
    for (const auto &key : time_keys) {
        std::cout << timers[key] << ", ";
    }
    std::cout << timers["total"] << "\n";
    return 0;
}
