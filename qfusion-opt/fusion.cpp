#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <ostream>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// global variables
size_t gMaxFusionSize;
int gQubits;
int gDFSCounter = 0;
int gMethod = 0;
double gCostFactor = 1.8;
std::map<std::string, std::vector<double>> gGateTime; // dynamic cost

double cost(const std::string &gateType, const int fusionSize,
            const std::set<int> &sortedQubits);

// gate size-index for fusion
struct Finfo {
    int size; // fusiom size
    int id;   // fusion gate index
};

int targetQubitCounter(const std::string &gateType) {
    if (gateType == "H" || gateType == "X" || gateType == "RY" ||
        gateType == "RX" || gateType == "U1" || gateType == "RZ" ||
        gateType == "D1")
        return 1;
    else if (gateType == "CX" || gateType == "CP" || gateType == "CZ" ||
             gateType == "RZZ" || gateType == "D2")
        return 2;
    else if (gateType == "D3")
        return 3;
    else if (gateType == "D4")
        return 4;
    else if (gateType == "D5")
        return 5;
    return 0;
}

class Gate {
  public:
    std::string gateType;
    std::vector<int> qubits;
    std::set<int> sortedQubits;
    std::vector<double> params; // non-qubit parameters

    // for fusion
    Finfo finfo;
    std::vector<Gate> subGateList;

    // Gate() = default;
    Gate(Finfo finfo) : finfo(finfo) {}
    Gate(Finfo finfo, const std::string &gateType,
         const std::set<int> &sortedQubits)
        : finfo(finfo), gateType(gateType), sortedQubits(sortedQubits) {}
    Gate(Finfo finfo, const Gate &gate)
        : finfo(finfo), gateType(gate.gateType), qubits(gate.qubits),
          sortedQubits(gate.sortedQubits), params(gate.params) {}
    Gate(const std::string &gateType, const std::vector<int> &qubits)
        : gateType(gateType), qubits(qubits),
          sortedQubits(qubits.begin(), qubits.end()) {}

    Gate(const std::string &line, int gateIndex) : finfo{1, gateIndex} {
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
        int i = 1;
        for (; i <= targetQubitCounter(gateType); ++i) {
            qubits.push_back(stoi(gateInfo[i]));
        }
        sortedQubits = std::set<int>(qubits.begin(), qubits.end());
        for (; i < gateInfo.size(); ++i) {
            params.push_back(stod(gateInfo[i]));
        }
    }

    std::string str() const {
        std::stringstream ss;
        ss << gateType;
        for (int qubit : qubits) {
            ss << " " << qubit;
        }
        for (double param : params) {
            ss << " " << std::fixed << std::setprecision(16) << param;
        }
        return ss.str();
    }

    void dump() const {
        // To dump only the current gate, use the command provided. To dump all
        // gates, iterate through the gate.subGateList using a loop such as: for
        // (const auto &subGate : gate.subGateList) { subGate.dump(); }.
        std::cout << finfo.size << "-" << finfo.id;
        std::cout << " (" << cost(gateType, finfo.size, sortedQubits) << ") ";
        if (subGateList.empty())
            return;
        double sum_cost = 0;
        for (const auto &subGate : subGateList) {
            sum_cost += cost(subGate.gateType, subGate.finfo.size,
                             subGate.sortedQubits);
        }
        std::cout << "sum_cost = " << sum_cost << "\n";
    }
};

using Matrix = std::vector<std::vector<std::complex<double>>>;

void printMat(const Matrix &mat) {
    for (const auto &row : mat) {
        for (const auto &val : row) {
            std::cout << /*std::setw(10) << "(" << */ val.real() << "+"
                      << val.imag() << "i, ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/* Return the matrix of two-qubit gate */
Matrix twoQubitsGateMat(const Gate &gate, const std::set<int> &fusedQubits) {
    int sizeAfterExpand = pow(2, fusedQubits.size());
    std::vector<int> reorderQubits;

    for (auto i : gate.qubits) {
        auto it = fusedQubits.find(i);
        if (it != fusedQubits.end())
            reorderQubits.push_back(std::distance(fusedQubits.begin(), it));
    }
    Matrix resMat(sizeAfterExpand,
                  std::vector<std::complex<double>>(sizeAfterExpand,
                                                    std::complex<double>()));
    if (gate.gateType == "CX") {
        for (int i = 0; i < sizeAfterExpand; i++)
            resMat[i][i] = 1;
        uint32_t controlBit = 1 << reorderQubits[0];
        uint32_t targetBit = 1 << reorderQubits[1];
        for (int i = 0; i < resMat.size(); i++) {
            if (i & controlBit) {
                resMat[i][i] = 0;
                resMat[i][i ^ targetBit] = 1;
            }
        }
    } else if (gate.gateType == "CP") {
        for (int i = 0; i < sizeAfterExpand; i++)
            resMat[i][i] = 1;
        int higherQubit = std::max(reorderQubits[0], reorderQubits[1]);
        int lowerQubit = std::min(reorderQubits[0], reorderQubits[1]);
        uint32_t controlledBit = 1 << higherQubit;
        uint32_t targetBit = 1 << lowerQubit;
        uint32_t changeTarget = controlledBit | targetBit;
        for (int i = 0; i < resMat.size(); i++) {
            if ((i & changeTarget) == changeTarget)
                resMat[i][i] = {cos(gate.params[0]), sin(gate.params[0])};
        }
    } else if (gate.gateType == "CZ") {
        for (int i = 0; i < sizeAfterExpand; i++)
            resMat[i][i] = 1;
        int higherQubit = std::max(reorderQubits[0], reorderQubits[1]);
        int lowerQubit = std::min(reorderQubits[0], reorderQubits[1]);
        uint32_t controlledBit = 1 << higherQubit;
        uint32_t targetBit = 1 << lowerQubit;
        uint32_t changeTarget = controlledBit | targetBit;
        for (int i = 0; i < resMat.size(); i++) {
            if ((i & changeTarget) == changeTarget)
                resMat[i][i] = -1;
        }
    } else if (gate.gateType == "RZZ") {
        Matrix rzzTmp(4, std::vector<std::complex<double>>(4));
        rzzTmp[0][0] = {cos(gate.params[0] / 2), -sin(gate.params[0] / 2)};
        rzzTmp[1][1] = {cos(gate.params[0] / 2), sin(gate.params[0] / 2)};
        rzzTmp[2][2] = rzzTmp[1][1];
        rzzTmp[3][3] = rzzTmp[0][0];
        uint32_t firstQubitMask = 1 << reorderQubits[0];
        uint32_t secondQubitMask = 1 << reorderQubits[1];
        for (int i = 0; i < resMat.size(); i++) {
            uint32_t target =
                ((i & secondQubitMask) >> (reorderQubits[1]) - 1) |
                ((i & firstQubitMask) >> (reorderQubits[0]));
            resMat[i][i] = rzzTmp[target][target];
        }
    } else if (gate.gateType[0] == 'D') {
        int matSize = 1 << fusedQubits.size();
        for (int i = 0; i < resMat.size(); i++)
            resMat[i][i] = 1;
        uint32_t targetMask = 0;

        for (auto qubit : reorderQubits)
            targetMask |= (1 << qubit);

        for (int i = 0; i < resMat.size(); i++) {
            uint32_t target = i & targetMask;
            /* sortTarget is used while the gate.targetQubit would cross a
             * qubit. For example, if gate.targetQubit = {0 ,2}, and the
             * fusedQubits = {0, 1, 2}, the targetMask = 101b, which means it
             * will get the first and third qubit. If i = 5 target = 5 & "101b",
             * which is 5. But the original target should be "11b". So here is
             * how we handle it.*/
            uint32_t sortTarget = 0;
            for (int idx = 0; idx < reorderQubits.size(); idx++)
                sortTarget |= ((target >> reorderQubits[idx]) & 1) << idx;
            std::complex<double> elem = {gate.params[sortTarget * 2],
                                         gate.params[sortTarget * 2 + 1]};
            resMat[i][i] = elem;
        }
    }
    return resMat;
}

/* Return the matrix of input gate */
// TODO const & params causes wrong results for vc24
Matrix gateMatrix(const std::string &gateType, std::vector<double> params) {
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
        mat[0].assign({{cos(params[0] / 2), 0}, {0, -sin(params[0] / 2)}});
        mat[1].assign({{0, -sin(params[0] / 2)}, {cos(params[0] / 2), 0}});
    } else if (gateType == "RY") {
        mat[0].assign({{cos(params[0] / 2), 0}, {-sin(params[0] / 2), 0}});
        mat[1].assign({{sin(params[0] / 2), 0}, {cos(params[0] / 2), 0}});
    } else if (gateType == "RZ") {
        mat[0].assign({{cos(params[0] / 2), -sin(params[0] / 2)}, {0, 0}});
        mat[1].assign({{0, 0}, {cos(params[0] / 2), sin(params[0] / 2)}});
    } else if (gateType == "I2") {
        mat[0].assign({{1, 0}, {0, 0}});
        mat[1].assign({{0, 0}, {1, 0}});
    } else if (gateType == "U1") {
        double theta = params[0];
        double phi = params[1];
        double lamda = params[2];
        mat[0].assign(
            {{cos(theta / 2), 0},
             {-sin(theta / 2) * cos(lamda), -sin(theta / 2) * sin(lamda)}});
        mat[1].assign({{sin(theta / 2) * cos(phi), sin(theta / 2) * sin(phi)},
                       {cos(theta / 2) * cos(phi + lamda),
                        cos(theta / 2) * sin(phi + lamda)}});
    } else {
        std::cerr << gateType << " gate doesn't support!\n";
        exit(1);
    }
    return mat;
}

Matrix tensorProduct(const Matrix &matA, const Matrix &matB) {
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
Matrix gateExpansion(const Gate &targetGate,
                     const std::set<int> &sortedQubits) {
    Matrix resMat, rhsMat;
    std::vector<std::string> expandedGateList;
    /* If targetGate two-qubit gate like CX, CZ and CP, get the expanded matrix
     * from twoQubitsGateMat().(Because they can't be simply expanded from
     * tensor product.) */
    if (targetGate.gateType[0] == 'C' || targetGate.gateType == "RZZ" ||
        targetGate.gateType[0] == 'D')
        return twoQubitsGateMat(targetGate, sortedQubits);

    for (auto &qubit : sortedQubits) {
        if (!targetGate.sortedQubits.contains(qubit))
            expandedGateList.push_back("I2");
        else
            expandedGateList.push_back(targetGate.gateType);
    }
    /* Use tensor product to expand a gate. Notice that we have to calculate it
     * from MSB to LSB */
    for (int i = expandedGateList.size() - 1; i >= 0; i--) {
        rhsMat = gateMatrix(expandedGateList[i], targetGate.params);
        if (i == expandedGateList.size() - 1)
            resMat = rhsMat;
        else {
            resMat = tensorProduct(resMat, rhsMat);
        }
    }
    return resMat;
}

/* Implement the matrix multiplication */
Matrix matrixMul(const Matrix &matA, const Matrix &matB) {
    Matrix resMat(matA.size(),
                  std::vector<std::complex<double>>(matB[0].size(), {0, 0}));
    if (matA[0].size() != matB.size()) {
        std::cerr << "The size of first matrix row is not equal to second "
                     "matrix column, matA[0] = "
                  << matA[0].size() << " ,matB.size = " << matB.size()
                  << std::endl;
        exit(1);
    }
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
Matrix calculateFusionGate(const std::vector<Gate> &subGateList,
                           const std::set<int> &sortedQubits) {
    Matrix resGate;
    Matrix expandedGate;
    /* We have to multiply these gates reversly. For example, if the subGateList
     * is H, X, Y, we should reverse the multiplication, it would be Y * X * H.
     */
    for (int i = subGateList.size() - 1; i >= 0; i--) {
        expandedGate = gateExpansion(subGateList[i], sortedQubits);
        if (i == subGateList.size() - 1) {
            resGate = expandedGate;
            continue;
        }
        resGate = matrixMul(resGate, expandedGate);
    }
    return resGate;
}

void showFusionGateList(const std::vector<std::vector<Gate>> &fusionGateList,
                        const int start, const int end) {
    for (int i = start - 1; i < end; ++i) {
        std::cout << "Number of fusion qubits: " << i + 1 << "\n";
        for (const auto &gate : fusionGateList[i]) {
            std::cout << "=============================================\n";
            gate.dump();
            for (const auto &subGate : gate.subGateList) {
                subGate.dump();
                std::cout << subGate.str() << "\n";
            }
        }
        std::cout << "\n";
    }
}

void showDependencyList(const std::vector<std::set<int>> &dependencyList) {
    std::cout << "=============================================\n";
    for (size_t i = 0; i < dependencyList.size(); ++i) {
        std::cout << i << " || ";
        for (auto j : dependencyList[i]) {
            std::cout << j << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

/* Check if the gate has dependency, scheduled gates are excluded */
// note: dependencyList is needed to copy
inline bool hasDependency(const std::set<int> &gateIndex,
                          std::vector<std::set<int>> dependencyList,
                          const std::set<int> &scheduledGates) {
    for (int i : gateIndex) {
        for (int gate : scheduledGates)
            dependencyList[i].erase(gate);
        if (!dependencyList[i].empty())
            return true;
    }
    return false;
}

// note: fusionGateList and dependencyList is modedified and wrapper is
// needed to copy
void deleteRelatedNode(std::vector<std::vector<Gate>> &fusionGateList,
                       std::vector<std::set<int>> &dependencyList,
                       const Gate wrapper) {
    // delete node in dependency list
    for (const auto &subgate : wrapper.subGateList)
        for (auto &dependencyList_i : dependencyList) {
            dependencyList_i.erase(subgate.finfo.id);
        }

    // delete node in fusionGate list
    for (auto &fusionList_i : fusionGateList) {
        for (size_t index = 0; index < fusionList_i.size(); ++index) {
            int flag = 0;
            for (const auto &subgate_i : fusionList_i[index].subGateList) {
                for (const auto &subgate_j : wrapper.subGateList)
                    if (subgate_i.finfo.id == subgate_j.finfo.id) {
                        flag = 1;
                        break;
                    }
                if (flag)
                    break;
            }
            if (flag) {
                fusionList_i.erase(fusionList_i.begin() + index);
                index--;
            }
        }
    }
}

/* Construct dependency gate set for each gate */
inline std::vector<std::set<int>>
constructDependencyList(const std::vector<Gate> &gates) {
    std::vector<int> gateOnQubit(gQubits, -1); // record the last gate on qubit
    int maxIndex = 0;
    for (const auto &gate : gates)
        maxIndex = std::max(maxIndex, gate.finfo.id);
    std::vector<std::set<int>> dependencyList(maxIndex + 1);
    for (const auto &gate : gates) {
        for (int qubit : gate.sortedQubits) {
            if (gateOnQubit[qubit] != -1) // the first gate on qubit is ignored
                dependencyList[gate.finfo.id].insert(gateOnQubit[qubit]);
            gateOnQubit[qubit] = gate.finfo.id;
        }
    }
    return dependencyList;
}

double cost(const std::string &gateType, const int fusionSize,
            const std::set<int> &sortedQubits) {
    if (gMethod < 4 || gMethod == 5) { // mode 4,6,7,8 need dynamic cost
        return pow(gCostFactor, (double)std::max(fusionSize - 1, 1));
    }
    int targetQubit = *sortedQubits.begin();
    if (gMethod != 7 and gMethod != 8) {
        // In Aer simulator, the second dimension represents the
        // target qubit. However, this is unnecessary in Quokka or
        // Queen simulators, so a value of 0 is used.
        targetQubit = 0;
    }
    if (fusionSize >= 2) {
        if (fusionSize > 5) {
            throw std::runtime_error("Current design does not support "
                                     "fusion size larger than 5: " +
                                     fusionSize);
        }
        if (gateType.starts_with("D")) { // for fused diagonal gates
            return gGateTime[gateType][targetQubit];
        } else { // for fused unitary gates
            std::string tempGateType = "U" + std::to_string(fusionSize);
            return gGateTime[tempGateType][targetQubit];
        }
    } else if (gGateTime.contains(gateType)) {
        return gGateTime[gateType][targetQubit];
    } else { // gate type not in gGateTime, we use the number of target qubits
             // to estimate
        std::string tempGateType = "U" + std::to_string(sortedQubits.size());
        return gGateTime[tempGateType][targetQubit];
    }
    throw std::runtime_error("Should not reach here.");
    return 0;
}

class Node {
  public:
    Node *parent = nullptr;
    Finfo finfo = {0, 0};
    double weight = 0;
    Node() = default; // root node
    Node(Node *parent, const Finfo &finfo, std::string gateType,
         const std::set<int> &sortedQubits)
        : parent(parent), finfo(finfo),
          weight(cost(gateType, this->finfo.size, sortedQubits)) {}

    void getSmallWeight(std::vector<std::vector<Gate>> fusionGateList,
                        std::vector<std::set<int>> dependencyList,
                        double nowWeight, double &smallWeight,
                        std::vector<Finfo> &nowGateList,
                        std::vector<Finfo> &finalGateList) {
        gDFSCounter++;
        nowWeight += weight;
        nowGateList.push_back(finfo);
        if (parent != nullptr) {
            auto &currentFusionList = fusionGateList[finfo.size - 1];
            auto it = find_if(
                currentFusionList.begin(), currentFusionList.end(),
                [this](const Gate &gate) { return gate.finfo.id == finfo.id; });
            if (it != currentFusionList.end()) {
                deleteRelatedNode(fusionGateList, dependencyList, *it);
            }
        }
        if (fusionGateList[0].empty()) {
            if (nowWeight < smallWeight) {
                smallWeight = nowWeight;
                finalGateList = nowGateList;
            }
            nowGateList.pop_back();
            return;
        }

        for (int fSize = gMaxFusionSize - 1; fSize >= 0; --fSize) {
            for (const auto &fusionGate : fusionGateList[fSize]) {
                if (finfo.size == fSize + 1 && finfo.id > fusionGate.finfo.id &&
                    (gMethod != 0 && gMethod != 2))
                    break;
                // collect gate indexes of fusion subgates
                std::set<int> gateIndexes;
                for (const auto &subgate : fusionGate.subGateList) {
                    gateIndexes.insert(subgate.finfo.id);
                }
                if (!hasDependency(gateIndexes, dependencyList, gateIndexes)) {
                    Node(this, fusionGate.finfo,
                         fusionGate.subGateList[0].gateType,
                         fusionGate.subGateList[0].sortedQubits)
                        .getSmallWeight(fusionGateList, dependencyList,
                                        nowWeight, smallWeight, nowGateList,
                                        finalGateList);
                }
            }
        }
        nowGateList.pop_back();
        return;
    }
};

class Circuit {
  public:
    std::vector<Gate> gates;

    Circuit() = default;
    Circuit(const std::vector<Gate> &gates) : gates(gates) {}
    Circuit(const std::string &fileName) {
        std::ifstream tmpInputFile(fileName);
        std::string line;
        for (size_t gateIndex = 0; getline(tmpInputFile, line); ++gateIndex) {
            gates.emplace_back(line, gateIndex);
        }
        tmpInputFile.close();
    }

    Gate &operator[](size_t index) { return gates[index]; }
    int size() const { return gates.size(); }

    Circuit schedule() const {
        Circuit reorderedCircuit;
        std::vector<Gate> waitingGates;
        std::set<int> scheduledGates;
        int currentQubit = 0;
        std::vector<std::set<int>> dependencyList =
            constructDependencyList(gates);
        std::vector<std::queue<int>> qubitGateQueues = buildQubitGateQueues();

        while (true) {
            // Skip empty qubit queues
            while (currentQubit < gQubits &&
                   qubitGateQueues[currentQubit].empty()) {
                ++currentQubit;
            }

            if (currentQubit == gQubits)
                break; // All gates processed

            // Extract gate index and its partner qubit
            int gateIdx = qubitGateQueues[currentQubit].front();
            qubitGateQueues[currentQubit].pop();

            int partnerQubit = qubitGateQueues[currentQubit].front();
            qubitGateQueues[currentQubit].pop();

            if (currentQubit >= partnerQubit) {
                if (!hasDependency({gates[gateIdx].finfo.id}, dependencyList,
                                   scheduledGates)) {
                    reorderedCircuit.gates.push_back(gates[gateIdx]);
                    scheduledGates.insert(gates[gateIdx].finfo.id);
                } else {
                    waitingGates.push_back(gates[gateIdx]);
                }
            }

            // Try to schedule waiting gates whose dependencies are now resolved
            for (auto it = waitingGates.begin(); it != waitingGates.end();) {
                int pendingIdx = (*it).finfo.id;
                if (!hasDependency({pendingIdx}, dependencyList,
                                   scheduledGates)) {
                    reorderedCircuit.gates.push_back(gates[pendingIdx]);
                    scheduledGates.insert(pendingIdx);
                    it = waitingGates.erase(it); // Remove and advance
                } else {
                    ++it;
                }
            }
            currentQubit = partnerQubit; // Continue with the next related qubit
        }
        return reorderedCircuit;
    }

    std::string str() const {
        std::stringstream ss;
        for (const auto &gate : gates) {
            ss << gate.str() << "\n";
        }
        return ss.str();
    }

  private:
    // Build a queue of gate indices for each qubit
    // for each gate, a qubit is pushed as a pair (gate, qubit)
    std::vector<std::queue<int>> buildQubitGateQueues() const {
        std::vector<std::queue<int>> qubitGateQueues(gQubits);
        for (size_t gateIdx = 0; gateIdx < gates.size(); ++gateIdx) {
            const auto &qubits = gates[gateIdx].sortedQubits;
            int prevQubit = -1;
            int firstQubit = *qubits.begin();
            // Add the gate to each involved qubit's queue in reverse order
            for (auto it = qubits.rbegin(); it != qubits.rend(); ++it) {
                int qubit = *it;
                qubitGateQueues[qubit].push(gateIdx);
                if (prevQubit == -1) {
                    qubitGateQueues[qubit].push(firstQubit);
                } else {
                    qubitGateQueues[qubit].push(prevQubit);
                }
                prevQubit = qubit;
            }
        }
        return qubitGateQueues;
    }
};

void outputFusionCircuit(const std::string &outputFileName,
                         const std::vector<std::vector<Gate>> &fusionGateList,
                         const std::vector<Finfo> &finalGateList) {
    std::ofstream outputFile(outputFileName, std::ios_base::app);
    for (size_t index = 1; index < finalGateList.size(); ++index) {
        const Finfo &finfo = finalGateList[index];
        if (finfo.size > 1) {
            const auto &wrapper = fusionGateList[finfo.size - 1][finfo.id];
            bool isDiagonal = true;
            for (const auto &subGate : wrapper.subGateList) {
                if (subGate.gateType[0] != 'D') {
                    isDiagonal = false;
                    break;
                }
            }
            char gateT = isDiagonal ? 'D' : 'U';
            outputFile << gateT << wrapper.sortedQubits.size() << " ";
            for (int qubit : wrapper.sortedQubits)
                outputFile << qubit << " ";

            Matrix fusedGateMat =
                calculateFusionGate(wrapper.subGateList, wrapper.sortedQubits);
            if (isDiagonal) {
                for (size_t i = 0; i < fusedGateMat.size(); i++)
                    outputFile << std::fixed << std::setprecision(16)
                               << fusedGateMat[i][i].real() << " "
                               << fusedGateMat[i][i].imag() << " ";
            } else {
                for (size_t matRow = 0; matRow < fusedGateMat.size();
                     matRow++) {
                    for (const auto elem : fusedGateMat[matRow])
                        outputFile << std::fixed << std::setprecision(16)
                                   << elem.real() << " " << std::fixed
                                   << std::setprecision(16) << elem.imag()
                                   << " ";
                }
            }
            outputFile << "\n";
        } else {
            const auto &subGate = fusionGateList[0][finfo.id].subGateList[0];
            outputFile << subGate.str() << "\n";
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
    void constructDAG(const std::vector<Gate> &gateList) {
        for (size_t i = 0; i < gateList.size(); ++i) {
            // one qubit fusion edge
            addEdge(i, i + 1,
                    cost(gateList[i].subGateList[0].gateType, 1,
                         gateList[i].subGateList[0].sortedQubits));
            // muti qubit fusion edge
            for (size_t fusionSize = 2; fusionSize <= gMaxFusionSize;
                 ++fusionSize) {
                std::set<int> qubit(gateList[i].subGateList[0].sortedQubits);
                size_t nowIndex = i;
                while (nowIndex < gateList.size() - 1) {
                    qubit.insert(gateList[nowIndex + 1]
                                     .subGateList[0]
                                     .sortedQubits.begin(),
                                 gateList[nowIndex + 1]
                                     .subGateList[0]
                                     .sortedQubits.end());
                    if (qubit.size() <= fusionSize)
                        nowIndex++;
                    else
                        break;
                }
                if (nowIndex > i) {
                    addEdge(i, nowIndex + 1, cost("", fusionSize, qubit));
                } else {
                    addEdge(i, -1, DBL_MAX);
                }
            }
        }
    }
    void shortestPath(const std::vector<Gate> &gateList,
                      const std::string &outputFileName) {
        struct DAGNode {
            int predecessor = -1;
            double distance = DBL_MAX;
            int fusionSize = 0;
        };

        // find shortestPath
        std::vector<DAGNode> nodeList(graphSize);
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

        for (int i = 0; i < graphSize; ++i) {
            std::cout << i << " " << nodeList[i].predecessor << " "
                      << nodeList[i].distance << "\n";
        }

        // output fusion result
        std::ofstream outputFile(outputFileName, std::ios_base::app);
        std::vector<std::string> outputStr;
        int nowIndex = graphSize - 1;

        while (nowIndex > 0) {
            if (nodeList[nowIndex].fusionSize > 1) {
                // bool isDiagonal = true;
                // for (int i = nodeList[nowIndex].predecessor; i < nowIndex;
                //      ++i) {
                //     auto subGate = gateList[i].subGateList[0];
                //     if (subGate.gateType[0] != 'D') {
                //         isDiagonal = false;
                //         break;
                //     }
                // }
                // char gateT = 'U';
                // if (isDiagonal)
                //     gateT = 'D';
                // std::cout << gateT;
                std::set<int> qubit;
                for (int i = nodeList[nowIndex].predecessor; i < nowIndex;
                     ++i) {
                    for (int qubitIndex :
                         gateList[i].subGateList[0].sortedQubits) {
                        qubit.insert(qubitIndex);
                    }
                }
                std::string tmpStr = "U" + std::to_string(qubit.size());
                for (int qubitIndex : qubit)
                    tmpStr += " " + std::to_string(qubitIndex);
                // std::cout << tmpStr << " " << cost("", qubit.size(), qubit)
                //           << "\n";
                Matrix fusedGateMat;
                std::vector<Gate> gateListForUGate;

                for (int i = nodeList[nowIndex].predecessor; i < nowIndex;
                     ++i) {
                    gateListForUGate.push_back(gateList[i].subGateList[0]);
                }
                fusedGateMat = calculateFusionGate(gateListForUGate, qubit);

                for (size_t matRow = 0; matRow < fusedGateMat.size();
                     matRow++) {
                    for (const auto elem : fusedGateMat[matRow]) {
                        std::ostringstream realStr, imagStr;
                        realStr << std::fixed << std::setprecision(16) << " "
                                << elem.real();
                        imagStr << std::fixed << std::setprecision(16) << " "
                                << elem.imag();
                        tmpStr += realStr.str() + imagStr.str();
                    }
                }
                outputStr.push_back(tmpStr);
            } else {
                int predecessorIndex = nodeList[nowIndex].predecessor;
                std::string tmpStr =
                    gateList[predecessorIndex].subGateList[0].gateType;
                for (int qubitIndex :
                     gateList[predecessorIndex].subGateList[0].sortedQubits)
                    tmpStr += " " + std::to_string(qubitIndex);
                if (gateList[predecessorIndex].subGateList[0].gateType[0] ==
                    'D') {
                    for (int i = 0; i * 2 < gateList[predecessorIndex]
                                                .subGateList[0]
                                                .params.size();
                         i++) {
                        std::ostringstream rotateStr;
                        rotateStr << std::fixed << std::setprecision(16) << " "
                                  << gateList[predecessorIndex]
                                         .subGateList[0]
                                         .params[i * 2]
                                  << " "
                                  << gateList[predecessorIndex]
                                         .subGateList[0]
                                         .params[i * 2 + 1];
                        tmpStr += rotateStr.str();
                    }
                } else {
                    for (double params :
                         gateList[predecessorIndex].subGateList[0].params) {
                        std::ostringstream rotateStr;
                        rotateStr << std::fixed << std::setprecision(16) << " "
                                  << params;
                        tmpStr += rotateStr.str();
                    }
                }
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

    fusionList(const std::vector<Gate> &gateList) {
        for (const auto &gate : gateList) {
            info tmpInfo;
            tmpInfo.gateIndex = gate.finfo.id;
            for (int qubit : gate.subGateList[0].sortedQubits) {
                tmpInfo.targetQubit.insert(qubit);
            }
            infoList.push_back(tmpInfo);
        }
    }

    void showInfoList() const {
        for (const auto &inf : infoList) {
            std::cout << inf.gateIndex << "|";
            if (!inf.targetQubit.empty()) {
                std::cout << " t{";
                for (auto it = inf.targetQubit.begin();
                     it != inf.targetQubit.end(); ++it) {
                    if (it != inf.targetQubit.begin())
                        std::cout << ", ";
                    std::cout << *it;
                }
                std::cout << "} ";
            }
            std::cout << "|";
            if (!inf.relatedQubit.empty()) {
                std::cout << " r{";
                for (auto it = inf.relatedQubit.begin();
                     it != inf.relatedQubit.end(); ++it) {
                    if (it != inf.relatedQubit.begin())
                        std::cout << ", ";
                    std::cout << *it;
                }
                std::cout << "}";
            }
            std::cout << " | " << inf.maxGateNumber << "\n";
        }
    }

    void reNewList() {
        std::vector<std::vector<int>> qubitDependency(gQubits);
        std::vector<std::set<int>> preGate(gQubits);
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

    std::vector<int> getFusedGate(size_t fusionSize) {
        std::vector<int> gateToFused;
        std::set<int> fusionQubit;

        if (infoList[0].targetQubit.size() > fusionSize) {
            gateToFused.push_back(infoList[0].gateIndex);
            infoList.erase(infoList.begin());
            return gateToFused;
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
        return gateToFused;
    }
};

void DoDiagonalFusion(Circuit &circuit) {
    std::vector<int> targetQubitList;
    // use hash table to record the gates fused into a diagonal gate
    std::map<Gate *, std::vector<Gate>> fusedGatesRecord;
    int addFlag = 0;
    std::vector<Gate> subGateList;
    for (size_t i = 0; i < circuit.size(); ++i) {
        Gate subGate(circuit[i].gateType, circuit[i].qubits);
        double params = 0.0;
        // Parsing the input string to fusionGate for the usage of
        // calculating the fused result of diagonal gate.
        if (circuit[i].gateType == "RZ" || circuit[i].gateType == "CP" ||
            circuit[i].gateType == "RZZ")
            params = circuit[i].params[0];
        subGate.params.push_back(params);

        if (circuit[i].gateType == "RZ") {
            auto firstInset =
                find(targetQubitList.begin(), targetQubitList.end(),
                     circuit[i].qubits[0]) != targetQubitList.end();
            if (firstInset && targetQubitList.size() <= gMaxFusionSize) {
                subGateList.push_back(subGate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else if (!firstInset &&
                       targetQubitList.size() <= gMaxFusionSize - 1) {
                targetQubitList.push_back(circuit[i].qubits[0]);
                subGateList.push_back(subGate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else {
                addFlag++;
            }
        } else if (circuit[i].gateType == "CZ" || circuit[i].gateType == "CP" ||
                   circuit[i].gateType == "RZZ") {
            auto firstInset =
                find(targetQubitList.begin(), targetQubitList.end(),
                     circuit[i].qubits[0]) != targetQubitList.end();
            auto secondInset =
                find(targetQubitList.begin(), targetQubitList.end(),
                     circuit[i].qubits[1]) != targetQubitList.end();
            if (!firstInset && !secondInset &&
                targetQubitList.size() <= gMaxFusionSize - 2) {
                targetQubitList.push_back(circuit[i].qubits[0]);
                targetQubitList.push_back(circuit[i].qubits[1]);
                subGateList.push_back(subGate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else if (firstInset && !secondInset &&
                       targetQubitList.size() <= gMaxFusionSize - 1) {
                targetQubitList.push_back(circuit[i].qubits[1]);
                subGateList.push_back(subGate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else if ((firstInset ^ secondInset) &&
                       targetQubitList.size() <= gMaxFusionSize - 1) {
                targetQubitList.push_back(
                    circuit[i].qubits[firstInset ? 1 : 0]);
                subGateList.push_back(subGate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else {
                addFlag++;
            }
        } else if (targetQubitList.size() > 0)
            addFlag++;
        if (addFlag && targetQubitList.size() > 0) {
            sort(targetQubitList.begin(), targetQubitList.end());
            Gate newGate("D", targetQubitList);
            circuit.gates.insert(circuit.gates.begin() + i, newGate);
            fusedGatesRecord[&*(circuit.gates.begin() + i)] = subGateList;
            subGateList.clear();
            addFlag = 0;
            targetQubitList.clear();
        }
    }

    if (!targetQubitList.empty()) {
        sort(targetQubitList.begin(), targetQubitList.end());
        Gate newGate("D", targetQubitList);
        circuit.gates.push_back(newGate);
        fusedGatesRecord[&*(circuit.gates.begin() + circuit.size() - 1)] =
            subGateList;
    }
    for (size_t i = 0; i < circuit.size(); ++i) {
        if (circuit[i].gateType == "D") {
            circuit[i].gateType += std::to_string(circuit[i].qubits.size());
            Matrix fusedDGate = calculateFusionGate(
                fusedGatesRecord[&circuit[i]], circuit[i].sortedQubits);
            for (int j = 0; j < fusedDGate.size(); j++) {
                circuit[i].params.push_back(fusedDGate[j][j].real());
                circuit[i].params.push_back(fusedDGate[j][j].imag());
            }
        }
    }
}

std::vector<std::vector<Gate>> GetPGFS(const Circuit &circuit) {
    std::vector<std::vector<Gate>> fusionGateList;
    // construct 1-qubit fusion list
    std::vector<Gate> NQubitFusionList;
    for (size_t gateIndex = 0; gateIndex < circuit.size(); gateIndex++) {
        const Gate &gate = circuit.gates[gateIndex];
        Gate wrapper({1, gateIndex}, gate.gateType, gate.sortedQubits);
        Gate innerGate({1, gateIndex}, gate);
        wrapper.subGateList.push_back(innerGate);
        NQubitFusionList.push_back(wrapper);
    }
    fusionGateList.push_back(NQubitFusionList);

    // muti qubit fusionList
    for (size_t fusionQubits = 2; fusionQubits <= gMaxFusionSize;
         ++fusionQubits) {
        std::vector<Gate> NQubitFusionList;
        fusionList NowInfoList(fusionGateList[0]); // BuildGateInfoList
        NowInfoList.reNewList();
        // NowInfoList.showInfoList();
        // std::vector<std::set<int>> dependencyList =
        //     constructDependencyList(fusionGateList[0]);
        // showDependencyList(dependencyList);
        int gateIndex = 0;
        while (!NowInfoList.infoList.empty()) {
            Gate wrapper({fusionQubits, gateIndex++});
            std::vector<int> gateToFused =
                NowInfoList.getFusedGate(fusionQubits);
            for (int index : gateToFused) {
                auto &subGate = fusionGateList[0][index].subGateList[0];
                wrapper.subGateList.push_back(subGate);
                // note: wrapper.qubits is not updated since it is not used
                wrapper.sortedQubits.insert(subGate.sortedQubits.begin(),
                                            subGate.sortedQubits.end());
            }
            NQubitFusionList.push_back(wrapper);
            NowInfoList.reNewList();
        }
        fusionGateList.push_back(NQubitFusionList);
    }
    return fusionGateList;
}

void GetOptimalGFS(std::string &outputFileName,
                   const std::vector<std::vector<Gate>> &fusionGateList) {
    // showFusionGateList(fusionGateList, 1, fusionGateList.size());
    //  find best fusion conbination
    //  execute small gate block one by one to reduce execution time
    const std::string cmd = "rm " + outputFileName + " > /dev/null 2>&1";
    [[maybe_unused]] auto sysinfo = system(cmd.c_str());

    [[unlikely]] if (gMethod <= 1) { // legacy; not tested
        std::vector<std::set<int>> dependencyList =
            constructDependencyList(fusionGateList[0]);
        double smallWeight = DBL_MAX;
        std::vector<Finfo> finalGateList, nowGateList;
        Node().getSmallWeight(fusionGateList, dependencyList, 0, smallWeight,
                              nowGateList, finalGateList);
        outputFusionCircuit(outputFileName, fusionGateList, finalGateList);
        return;
    }
    std::vector<std::vector<Gate>> subFusionGateList(gMaxFusionSize);
    std::vector<std::vector<int>> recordIndex(gMaxFusionSize);
    for (size_t index = 0; index < fusionGateList[gMaxFusionSize - 1].size();
         ++index) {
        for (const auto &subGate :
             fusionGateList[gMaxFusionSize - 1][index].subGateList) {
            subFusionGateList[0].push_back(fusionGateList[0][subGate.finfo.id]);
            for (size_t i = 0; i < gMaxFusionSize; ++i)
                recordIndex[i].push_back(subGate.finfo.id);
        }
        for (size_t fusionQubits = 1; fusionQubits < gMaxFusionSize - 1;
             ++fusionQubits) {
            for (size_t subIndex = 0;
                 subIndex < fusionGateList[fusionQubits].size(); ++subIndex) {
                int flag = 1;
                std::vector<int> reIndex;
                for (const auto &subGate :
                     fusionGateList[fusionQubits][subIndex].subGateList) {
                    // Check if all fusion indices of the sub-gates are a
                    // subset of the recordIndex for this fusion size.
                    if (find(recordIndex[fusionQubits].begin(),
                             recordIndex[fusionQubits].end(),
                             subGate.finfo.id) ==
                        recordIndex[fusionQubits].end()) {
                        flag = 0;
                        recordIndex[fusionQubits].insert(
                            recordIndex[fusionQubits].end(), reIndex.begin(),
                            reIndex.end());
                        break;
                    }
                    reIndex.push_back(subGate.finfo.id);
                    std::erase(recordIndex[fusionQubits], subGate.finfo.id);
                }
                if (flag)
                    subFusionGateList[fusionQubits].push_back(
                        fusionGateList[fusionQubits][subIndex]);
            }
        }
        subFusionGateList[gMaxFusionSize - 1].push_back(
            fusionGateList[gMaxFusionSize - 1][index]);

        int flag = 1;
        for (size_t fusionQubits = 1; fusionQubits < gMaxFusionSize - 1;
             ++fusionQubits)
            if (!recordIndex[fusionQubits].empty())
                flag = 0;
        if (flag) {
            // execute find weight
            size_t smallCircuitSize = subFusionGateList[0].size();
            // choose different strategies with different circuit size
            std::cout << smallCircuitSize << std::endl;
            if (smallCircuitSize < 26) {
                double smallWeight = DBL_MAX;
                std::vector<std::set<int>> dependencyList =
                    constructDependencyList(subFusionGateList[0]);
                std::cout << "get small weight" << std::endl;
                std::vector<Finfo> finalGateList, nowGateList;
                Node().getSmallWeight(subFusionGateList, dependencyList, 0,
                                      smallWeight, nowGateList, finalGateList);
                outputFusionCircuit(outputFileName, fusionGateList,
                                    finalGateList);
            } else {
                std::cout << "get shortest path" << std::endl;
                DAG subDAG(smallCircuitSize);
                subDAG.constructDAG(subFusionGateList[0]);
                subDAG.showInfo();
                subDAG.shortestPath(subFusionGateList[0], outputFileName);
            }
            for (size_t i = 0; i < gMaxFusionSize; ++i) {
                subFusionGateList[i].clear();
                recordIndex[i].clear();
            }
        }
    }
}

std::string get_env_variable(const std::string &var_name) {
    const char *var_value_cstr = std::getenv(var_name.c_str());
    if (var_value_cstr == nullptr) {
        throw std::runtime_error("Environment variable '" + var_name +
                                 "' not found.");
    }
    return std::string(var_value_cstr);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0]
                  << " [Input File] [Output File] [Max Fusion Qubits] "
                     "[Total Qubits] "
                     "[Mode]"
                  << "\n";
        return 1;
    }
    auto total_time_start = std::chrono::steady_clock::now();
    std::unordered_map<std::string, double> timers;

    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];
    gMaxFusionSize = atoi(argv[3]);
    gQubits = atoi(argv[4]);
    if (argc == 6)
        gMethod = atoi(argv[5]);

    if (gMethod == 4 || gMethod == 6 || gMethod == 7 || gMethod == 8) {
        std::string gateTimeFilenamme =
            get_env_variable("DYNAMIC_COST_FILENAME");
        std::ifstream gateTimeFile(gateTimeFilenamme);
        if (!gateTimeFile) {
            std::cerr << "Error: Could not open the file " << gateTimeFilenamme
                      << "\nPlease set environment variable "
                         "DYNAMIC_COST_FILENAME correctly\n";
            std::cerr
                << "Please run python/performance_model_{SIMULATOR}.py first\n";
            return 1;
        }
        // the cost table format: gate, target_qubit (or chunk_size), time
        for (std::string line; getline(gateTimeFile, line);) {
            std::stringstream ss(line);
            std::string gateType, target_qubit, time;
            getline(ss, gateType, ',');
            getline(ss, target_qubit, ',');
            getline(ss, time, ',');
            gGateTime[gateType].push_back(stod(time));
        }
    }

    // reorder
    auto time_start = std::chrono::steady_clock::now();
    Circuit circuit(inputFileName);

    Circuit newCircuit = circuit.schedule();

    auto time_end = std::chrono::steady_clock::now();
    timers["reorder"] =
        std::chrono::duration<double>(time_end - time_start).count();

    // diagonal fusion
    time_start = std::chrono::steady_clock::now();
    if (gMethod > 4 && gMethod != 7) {
        DoDiagonalFusion(newCircuit);
    }

    time_end = std::chrono::steady_clock::now();
    timers["diagonal"] =
        std::chrono::duration<double>(time_end - time_start).count();

    time_start = std::chrono::steady_clock::now();
    std::vector<std::vector<Gate>> fusionGateList = GetPGFS(newCircuit);
    time_end = std::chrono::steady_clock::now();
    timers["GetPGFS"] =
        std::chrono::duration<double>(time_end - time_start).count();
    showFusionGateList(fusionGateList, 1, gMaxFusionSize);
    // do reorder
    time_start = std::chrono::steady_clock::now();
    for (size_t fusionQubits = 1; fusionQubits < gMaxFusionSize;
         ++fusionQubits) {
        Circuit cc(fusionGateList[fusionQubits]);
        fusionGateList[fusionQubits] = cc.schedule().gates;
        for (size_t index = 0; index < fusionGateList[fusionQubits].size();
             ++index)
            fusionGateList[fusionQubits][index].finfo.id = index;
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
    std::vector<std::string> time_keys{"reorder", "diagonal", "reorder2",
                                       "GetPGFS", "GetOptimalGFS"};
    for (const auto &key : time_keys) {
        std::cout << timers[key] << ", ";
    }
    std::cout << timers["total"] << "\n";
    return 0;
}
