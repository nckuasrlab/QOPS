#include "ThreadPool.h"
#include <algorithm>
#include <atomic>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <complex>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <ostream>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#define DEBUG_getSmallWeight 0
#define DEBUG_getSmallWeightSingleThread 0
#define DEBUG_shortestPath 0
#define DEBUG_SECTION(x, y)                                                    \
    if constexpr (x)                                                           \
        do {                                                                   \
            y                                                                  \
    } while (0)

using gate_size_t = unsigned short;  // 65535 gates
using qubit_size_t = unsigned short; // 65535 qubits

// global variables
qubit_size_t gMaxFusionSize;
qubit_size_t gQubits;
std::atomic<unsigned long long> gDFSCounter = 0, gEarlyStopCounter = 0;
std::atomic<unsigned int> gSmallCircuitCounter = 0, gShortestPathCounter = 0;
int gMethod = 0;
double gCostFactor = 1.8;
std::map<std::string, std::vector<double>> gGateTime; // dynamic cost
// Global mutex to protect shared resources (e.g., smallWeight and
// finalGateList)
std::mutex gMutex;
ThreadPool gPool(std::thread::hardware_concurrency());

bool isDiagonalGate(const std::string &gateType) {
    static const std::set<std::string> kDiagonalGates = {"S",  "Z",  "T",  "RZ",
                                                         "CZ", "CP", "RZZ"};
    if (gateType[0] == 'D') {
        return true;
    } else {
        return kDiagonalGates.contains(gateType);
    }
}

double cost(const std::string &gateType,
            const std::set<qubit_size_t> &sortedQubits);

// gate size-index for fusion
struct Finfo {
    qubit_size_t size; // fusiom size
    gate_size_t fid;   // fusion gate index
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
    std::vector<qubit_size_t> qubits;
    std::set<qubit_size_t> sortedQubits;
    std::vector<double> params; // non-qubit parameters

    // for fusion
    Finfo finfo;
    std::vector<gate_size_t> gids; // index to the original circuit
    std::vector<Gate> subGateList;

    Gate(Finfo finfo) : finfo(finfo) {}
    Gate(const std::string &gateType,
         const std::set<qubit_size_t> &sortedQubits, Finfo finfo,
         const std::vector<gate_size_t> &gids)
        : gateType(gateType), sortedQubits(sortedQubits), finfo(finfo),
          gids(gids) {}
    Gate(Finfo finfo, const Gate &gate)
        : gateType(gate.gateType), qubits(gate.qubits),
          sortedQubits(gate.sortedQubits), params(gate.params), finfo(finfo),
          gids(gate.gids) {}
    Gate(const std::string &gateType,
         const std::set<qubit_size_t> &sortedQubits,
         const std::vector<Gate> &subGateList)
        : gateType(gateType), qubits(sortedQubits.begin(), sortedQubits.end()),
          sortedQubits(sortedQubits), subGateList(subGateList) {}
    Gate(const std::string &line, gate_size_t gateIndex)
        : finfo{1, gateIndex}, gids{gateIndex} {
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
        sortedQubits = std::set<qubit_size_t>(qubits.begin(), qubits.end());
        params.reserve(gateInfo.size() - i);
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
        // (const auto &subgate : gate.subGateList) { subgate.dump(); }.
        std::cout << finfo.size << "-" << finfo.fid;
        std::cout << " (" << cost(gateType, sortedQubits) << ") ";
        if (subGateList.empty())
            return;
        double sum_cost = 0;
        for (const auto &subgate : subGateList) {
            sum_cost += cost(subgate.gateType, subgate.sortedQubits);
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
Matrix twoQubitsGateMat(const Gate &gate,
                        const std::set<qubit_size_t> &fusedQubits) {
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
        for (size_t i = 0; i < resMat.size(); i++) {
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
Matrix gateMatrix(const std::string &gateType,
                  const std::vector<double> &params) {
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
                     const std::set<qubit_size_t> &sortedQubits) {
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
                           const std::set<qubit_size_t> &sortedQubits) {
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
    std::cout << "showFusionGateList\n";
    for (int i = start - 1; i < end; ++i) {
        std::cout << "Number of fusion qubits: " << i + 1 << "\n";
        for (const auto &gate : fusionGateList[i]) {
            std::cout << std::string(40, '=') << "\n";
            gate.dump();
            for (const auto &subgate : gate.subGateList) {
                subgate.dump();
                std::cout << subgate.str() << "\n";
            }
        }
        std::cout << "\n";
    }
}

void showDependencyList(const std::vector<std::set<int>> &dependencyList) {
    std::cout << "showDependencyList\n";
    std::cout << std::string(40, '=') << "\n";
    for (size_t i = 0; i < dependencyList.size(); ++i) {
        std::cout << i << " || ";
        for (auto j : dependencyList[i]) {
            std::cout << j << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void showFinfoList(const std::vector<Finfo> &finfoList) {
    for (const auto &info : finfoList) {
        std::cout << info.size << "-" << info.fid << " ";
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
    // Build a set of fids from wrapper for O(1) lookup
    std::unordered_set<int> wrapperFids;
    for (const auto &subgate : wrapper.subGateList) {
        wrapperFids.insert(subgate.finfo.fid);
    }

    // delete node in dependency list
    for (auto &dependencyList_i : dependencyList) {
        for (int fid : wrapperFids) {
            dependencyList_i.erase(fid);
        }
    }

    // delete node in fusionGate list
    for (auto &fusionList : fusionGateList) {
        fusionList.erase(
            std::remove_if(fusionList.begin(), fusionList.end(),
                           [&](const Gate &gate) {
                               for (const auto &subgate : gate.subGateList) {
                                   if (wrapperFids.count(subgate.finfo.fid)) {
                                       return true; // Remove this gate
                                   }
                               }
                               return false;
                           }),
            fusionList.end());
    }
}

/* Construct dependency gate set for each gate */
inline std::vector<std::set<int>>
constructDependencyList(const std::vector<Gate> &gates) {
    std::vector<int> gateOnQubit(gQubits, -1); // record the last gate on qubit
    unsigned short maxIndex = 0;
    for (const auto &gate : gates)
        maxIndex = std::max(maxIndex, gate.finfo.fid);
    std::vector<std::set<int>> dependencyList(maxIndex + 1);
    for (const auto &gate : gates) {
        for (int qubit : gate.sortedQubits) {
            if (gateOnQubit[qubit] != -1) // the first gate on qubit is ignored
                dependencyList[gate.finfo.fid].insert(gateOnQubit[qubit]);
            gateOnQubit[qubit] = gate.finfo.fid;
        }
    }
    return dependencyList;
}

double cost(const std::string &gateType,
            const std::set<qubit_size_t> &sortedQubits) {
    const int qSize = sortedQubits.size();
    if (gMethod < 4 || gMethod == 5) { // mode 4,6,7,8 need dynamic cost
        return pow(gCostFactor, (double)std::max(qSize - 1, 1));
    }
    int targetQubit = *sortedQubits.begin();
    if (gMethod != 7 and gMethod != 8) {
        // In Aer simulator, the second dimension represents the
        // target qubit. However, this is unnecessary in Quokka or
        // Queen simulators, so a value of 0 is used.
        targetQubit = 0;
    }
    if (gGateTime.contains(gateType)) {
        return gGateTime[gateType][targetQubit];
    } else { // gate type not in gGateTime, we use the number of target qubits
             // to estimate
        return gGateTime["U1"][targetQubit] * qSize;
    }
    return 0;
}

class SmallWeightNode {
  public:
    SmallWeightNode *parent = nullptr;
    Finfo finfo = {0, 0};
    double weight = 0;
    SmallWeightNode() = default; // root node
    SmallWeightNode(SmallWeightNode *parent, const Finfo &finfo,
                    std::string gateType,
                    const std::set<qubit_size_t> &sortedQubits)
        : parent(parent), finfo(finfo), weight(cost(gateType, sortedQubits)) {}

    void getSmallWeight(std::vector<std::vector<Gate>> fusionGateList,
                        std::vector<std::set<int>> dependencyList,
                        double nowWeight, std::atomic<double> &smallWeight,
                        std::vector<Finfo> nowGateList,
                        std::vector<Finfo> &finalGateList, ThreadPool &pool,
                        std::mutex &mutex, int depth = 0) {

        gDFSCounter.fetch_add(1, std::memory_order_relaxed);
        nowWeight += weight;
        // early stop if nowWeight > smallWeight
        if (nowWeight > smallWeight.load(std::memory_order_relaxed)) {
            gEarlyStopCounter.fetch_add(1, std::memory_order_relaxed);
            return;
        }

        nowGateList.push_back(finfo);
        if (parent != nullptr)
            [[likely]] {
                auto &currentFusionList = fusionGateList[finfo.size - 1];
                auto it = std::find_if(currentFusionList.begin(),
                                       currentFusionList.end(),
                                       [this](const Gate &gate) {
                                           return gate.finfo.fid == finfo.fid;
                                       });
                if (it != currentFusionList.end()) {
                    deleteRelatedNode(fusionGateList, dependencyList, *it);
                }
            }
        else
            [[unlikely]] {
                DEBUG_SECTION(
                    DEBUG_getSmallWeightSingleThread,
                    std::cout << "getSmallWeight\n"
                              << std::string(40, 'O') << "\n";
                    showFusionGateList(fusionGateList, 1, gMaxFusionSize););
            }

        if (fusionGateList[0].empty()) {
            DEBUG_SECTION(DEBUG_getSmallWeightSingleThread,
                          showFinfoList(nowGateList););
            std::unique_lock<std::mutex> lock(mutex);
            if (nowWeight < smallWeight) {
                smallWeight = nowWeight;
                finalGateList = nowGateList;
                DEBUG_SECTION(DEBUG_getSmallWeightSingleThread,
                              std::cout << "update\n";);
            }
            return;
        }

        std::vector<std::future<void>> futures;

        for (int fSize = gMaxFusionSize - 1; fSize >= 0; --fSize) {
            for (const auto &fusionGate : fusionGateList[fSize]) {
                if (finfo.size == fSize + 1 &&
                    finfo.fid > fusionGate.finfo.fid &&
                    (gMethod != 0 && gMethod != 2))
                    break;

                std::set<int> gateIndexes;
                for (const auto &subgate : fusionGate.subGateList)
                    gateIndexes.insert(subgate.finfo.fid);

                if (!hasDependency(gateIndexes, dependencyList,
                                   gateIndexes)) { // pruning
                    SmallWeightNode nextNode(
                        this, fusionGate.finfo,
                        fusionGate.subGateList[0].gateType,
                        fusionGate.subGateList[0].sortedQubits);

                    auto task = [&, nextNode, nowGateList,
                                 nowWeight]() mutable {
                        nextNode.getSmallWeight(fusionGateList, dependencyList,
                                                nowWeight, smallWeight,
                                                nowGateList, finalGateList,
                                                pool, mutex, depth + 1);
                    };

                    if (depth < 2 && !DEBUG_getSmallWeightSingleThread) {
                        futures.emplace_back(pool.enqueue(task));
                    } else {
                        task(); // sequential
                    }
                }
            }
        }

        for (auto &f : futures)
            f.get();
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
    gate_size_t size() const { return gates.size(); }

    Gate fusedToGate() const {
        Gate fusedGate(Finfo{0, 0}); // dummy
        bool isDiagonal = true;
        for (const auto &subgate : gates) {
            fusedGate.sortedQubits.insert(subgate.sortedQubits.begin(),
                                          subgate.sortedQubits.end());
            if (!isDiagonalGate(subgate.gateType))
                isDiagonal = false;
        }
        fusedGate.qubits = std::vector(fusedGate.sortedQubits.begin(),
                                       fusedGate.sortedQubits.end());
        fusedGate.gateType = (isDiagonal ? "D" : "U") +
                             std::to_string(fusedGate.sortedQubits.size());

        Matrix fusedGateMat =
            calculateFusionGate(gates, fusedGate.sortedQubits);
        if (isDiagonal) {
            fusedGate.params.reserve(fusedGateMat.size() * 2);
            for (size_t i = 0; i < fusedGateMat.size(); i++) {
                fusedGate.params.push_back(fusedGateMat[i][i].real());
                fusedGate.params.push_back(fusedGateMat[i][i].imag());
            }
        } else {
            fusedGate.params.reserve(fusedGateMat.size() *
                                     fusedGateMat[0].size() * 2);
            for (size_t matRow = 0; matRow < fusedGateMat.size(); matRow++) {
                for (const auto elem : fusedGateMat[matRow]) {
                    fusedGate.params.push_back(elem.real());
                    fusedGate.params.push_back(elem.imag());
                }
            }
        }
        return fusedGate;
    }

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
                if (!hasDependency({gates[gateIdx].finfo.fid}, dependencyList,
                                   scheduledGates)) {
                    reorderedCircuit.gates.push_back(gates[gateIdx]);
                    scheduledGates.insert(gates[gateIdx].finfo.fid);
                } else {
                    waitingGates.push_back(gates[gateIdx]);
                }
            }

            // Try to schedule waiting gates whose dependencies are now resolved
            for (auto it = waitingGates.begin(); it != waitingGates.end();) {
                int pendingIdx = (*it).finfo.fid;
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

    void dump() const {
        for (const auto &gate : gates) {
            std::cout << "gids: ";
            for (int gid : gate.gids)
                std::cout << gid << " ";
            std::cout << ", fid: " << gate.finfo.fid << " ";
            std::cout << gate.str() << "\n";
        }
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
    for (const auto &finfo : finalGateList) {
        if (finfo.size == 0) { // root node
            continue;
        } else {
            const auto &wrapper = fusionGateList[finfo.size - 1][finfo.fid];
            if (wrapper.subGateList.size() == 1) { // no fusion
                outputFile << wrapper.subGateList[0].str() << "\n";
                continue;
            }
            outputFile << Circuit(wrapper.subGateList).fusedToGate().str()
                       << "\n";
        }
    }
    outputFile.close();
}

class DAG {
  public:
    int graphSize;
    gate_size_t fidOffset;
    std::vector<std::vector<std::pair<int, double>>>
        edge; // <destination, weight>
    std::vector<int> finalShortestPath;

    DAG(int graphSize) : graphSize(graphSize + 1) {
        for (int i = 0; i < this->graphSize - 1; ++i)
            edge.push_back(std::vector<std::pair<int, double>>());
    }
    bool addEdge(int source, int destination, double weight) {
        // prevent duplicate edge; return true if added else false (duplicate)
        for (auto it = edge[source].begin(); it != edge[source].end(); ++it) {
            if (it->first == destination && it->second == weight)
                return false;
        }
        edge[source].push_back(std::make_pair(destination, weight));
        return true;
    }
    void constructDAG(const std::vector<Gate> &gateList) {
        DEBUG_SECTION(
            DEBUG_shortestPath, std::cout << "constructDAG\n";
            for (auto gate
                 : gateList) {
                std::cout << gate.subGateList[0].str() << "\n";
            });
        fidOffset = gateList[0].finfo.fid;
        for (size_t i = 0; i < gateList.size(); ++i) {
            // one qubit fusion edge
            addEdge(i, i + 1,
                    cost(gateList[i].subGateList[0].gateType,
                         gateList[i].subGateList[0].sortedQubits));
            // muti qubit fusion edge
            for (size_t fSize = 2; fSize <= gMaxFusionSize; ++fSize) {
                std::set<qubit_size_t> qubit;
                size_t j = i;
                // note: We cannot use fusedToGate() here because we want to
                // reuse the testQubit. This avoids calling fusedToGate(),
                // which is expensive and involves redundant loops.
                bool isDiagonal = true;
                while (j < gateList.size()) {
                    const auto &subgate = gateList[j].subGateList[0];
                    std::set<qubit_size_t> testQubit = qubit;
                    // note: diagonal gate's subgate has sorted qubits
                    testQubit.insert(subgate.sortedQubits.begin(),
                                     subgate.sortedQubits.end());
                    if (testQubit.size() <=
                        fSize) { // test if the fusion is valid
                        if (!isDiagonalGate(subgate.gateType))
                            isDiagonal = false;
                        qubit = testQubit; // update fused qubits
                        j++;
                    } else
                        break;
                }
                if (j > i + 1) { // means more than one gate to be fused
                    std::string gateType =
                        (isDiagonal ? "D" : "U") + std::to_string(qubit.size());
                    bool isAdded = addEdge(i, j, cost(gateType, qubit));
                    DEBUG_SECTION(
                        DEBUG_shortestPath, if (isAdded) {
                            std::cout << "Add edge: " << i << " -> " << j
                                      << " : " << gateType << " "
                                      << cost(gateType, qubit) << " (";
                            for (int k = i; k < j; k++)
                                std::cout
                                    << (k == i ? "" : " ")
                                    << gateList[k].subGateList[0].gateType;
                            std::cout << ")\n";
                        });
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
            int fSize = 0;
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
                    nodeList[edge[i][j].first].fSize = j + 1;
                    nodeList[edge[i][j].first].predecessor = i;
                }
            }
        }

        // output fusion result
        std::ofstream outputFile(outputFileName, std::ios_base::app);
        std::vector<std::string> outputStr;
        int idx = graphSize - 1; // start from the end of the graph

        DEBUG_SECTION(DEBUG_shortestPath,
                      std::cout << "fid: " << idx + fidOffset << " \n";);
        while (idx > 0) {
            if (nodeList[idx].fSize >
                1) { // means more than one gate to be fused
                Circuit gatesToFused;
                for (int i = nodeList[idx].predecessor; i < idx; ++i) {
                    DEBUG_SECTION(DEBUG_shortestPath,
                                  std::cout << "↓ "
                                            << gateList[i].subGateList[0].str()
                                            << "\n";);
                    gatesToFused.gates.push_back(gateList[i].subGateList[0]);
                }
                std::string tmpStr = gatesToFused.fusedToGate().str();
                DEBUG_SECTION(DEBUG_shortestPath,
                              std::cout << "Merge to -> " << tmpStr << "\n";);
                outputStr.push_back(tmpStr);
            } else {
                int predecessorIndex = nodeList[idx].predecessor;
                std::string tmpStr =
                    gateList[predecessorIndex].subGateList[0].str();
                DEBUG_SECTION(DEBUG_shortestPath,
                              std::cout << "Single -> " << tmpStr << "\n";);
                outputStr.push_back(tmpStr);
            }
            finalShortestPath.push_back(idx);
            DEBUG_SECTION(DEBUG_shortestPath,
                          std::cout << "fid: "
                                    << nodeList[idx].predecessor + fidOffset
                                    << " \n";);
            idx = nodeList[idx].predecessor;
        }
        finalShortestPath.push_back(idx);
        for (auto it = outputStr.rbegin(); it != outputStr.rend(); ++it)
            outputFile << *it << "\n";
        outputFile.close();
    }

    void dump() const {
        std::cout << "--------------------------------------------\n";
        std::cout << "Gate number: " << graphSize - 1 << "\n";
        for (size_t i = 0; i < edge.size(); ++i) {
            std::cout << "Source: " << i << " (fid: " << i + fidOffset
                      << " )\n";
            for (const auto &[destination, weight] : edge[i]) {
                std::cout << "    D: " << destination
                          << " (fid: " << destination + fidOffset
                          << " )  W: " << weight << "\n";
            }
        }
        std::cout << "shortest path: \n";
        for (auto it = finalShortestPath.rbegin();
             it != finalShortestPath.rend(); ++it)
            std::cout << *it << " ";
        std::cout << "\n";
    }

    void dumpDot(const std::string &outputFileName = "graph.dot") const {
        // generate dot file; use `dot -Tsvg graph.dot > graph.svg` to get svg
        // TODO: if there are more than one shortest path
        std::ofstream outputFile(outputFileName);
        outputFile << "strict digraph {\n";
        for (size_t i = 0; i < edge.size(); ++i) {
            for (const auto &[destination, weight] : edge[i]) {
                if (destination == -1)
                    continue;
                outputFile << i + fidOffset << " -> " << destination + fidOffset
                           << " [label=\"" << weight << "\"]\n";
            }
        }
        for (size_t i = finalShortestPath.size() - 1; i > 0; --i) {
            outputFile << finalShortestPath[i] + fidOffset
                       << "[color=\"red\", penwidth=4]\n";
            outputFile << finalShortestPath[i] + fidOffset << " -> "
                       << finalShortestPath[i - 1] + fidOffset
                       << " [color=\"red\", fontcolor=\"red\", penwidth=4]\n";
        }
        outputFile << finalShortestPath[0] + fidOffset
                   << "[color=\"red\", penwidth=4]\n";
        outputFile << "}\n";
    }
};

bool isSubset(const std::unordered_set<qubit_size_t> &a,
              const std::unordered_set<qubit_size_t> &b) {
    for (const auto &elem : b) {
        if (a.find(elem) == a.end()) {
            return false; // element in b not found in a
        }
    }
    return true; // all elements in b are in a
}

class FusionList {
  public:
    struct Info {
        Info(gate_size_t gateIndex, const std::set<qubit_size_t> &sortedQubits)
            : gateIndex(gateIndex), sortedQubits(sortedQubits) {}

        gate_size_t gateIndex;
        int maxGateNumber = 0;
        std::set<qubit_size_t> sortedQubits;
        std::unordered_set<qubit_size_t> relatedQubits;
    };

    std::vector<Info> infoList;

    FusionList(const std::vector<Gate> &gateList) {
        for (const auto &gate : gateList) {
            infoList.emplace_back(gate.finfo.fid,
                                  gate.subGateList[0].sortedQubits);
        }
    }
    // copy assignment operator
    FusionList &operator=(const FusionList &rhs) {
        infoList = rhs.infoList;
        return *this;
    }
    // copy constructor
    FusionList(const FusionList &rhs) { infoList = rhs.infoList; }

    void reNewList() {
        std::vector<std::unordered_set<qubit_size_t>> qubitDependency(gQubits);
        std::vector<std::unordered_set<size_t>> preGate(gQubits);
        for (size_t i = 0; i < infoList.size(); ++i) {
            auto &info = infoList[i];
            std::unordered_set<qubit_size_t> relatedQubits(
                info.sortedQubits.begin(), info.sortedQubits.end());
            for (int q : info.sortedQubits) {
                relatedQubits.insert(qubitDependency[q].begin(),
                                     qubitDependency[q].end());
            }

            info.relatedQubits = std::move(relatedQubits);
            std::unordered_set<int> allPreGate;
            for (int element : info.relatedQubits) {
                qubitDependency[element] = info.relatedQubits;
                preGate[element].insert(i);
                allPreGate.insert(preGate[element].begin(),
                                  preGate[element].end());
            }
            info.maxGateNumber = static_cast<int>(allPreGate.size());
        }
    }

    std::vector<int> getFusedGate(size_t fSize) {
        std::vector<int> gateToFused;

        if (infoList[0].sortedQubits.size() > fSize) {
            gateToFused.push_back(infoList[0].gateIndex);
            infoList.erase(infoList.begin());
            return gateToFused;
        }

        while (fSize) {
            int nowMaxGateNumber = 0;
            std::unordered_set<qubit_size_t> tmpFusionQubit;
            for (const auto &info : infoList) {
                if (info.maxGateNumber > nowMaxGateNumber &&
                    info.relatedQubits.size() <= fSize) {
                    nowMaxGateNumber = info.maxGateNumber;
                    tmpFusionQubit = info.relatedQubits;
                }
            }
            fSize -= tmpFusionQubit.size();

            for (auto it = infoList.begin(); it != infoList.end();) {
                // if (includes(tmpFusionQubit.begin(), tmpFusionQubit.end(),
                //              it->relatedQubits.begin(),
                //              it->relatedQubits.end())) {
                if (isSubset(tmpFusionQubit, it->relatedQubits)) {
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

    void dump() const {
        for (const auto &inf : infoList) {
            std::cout << inf.gateIndex << "|";
            if (!inf.sortedQubits.empty()) {
                std::cout << " t{";
                for (auto it = inf.sortedQubits.begin();
                     it != inf.sortedQubits.end(); ++it) {
                    if (it != inf.sortedQubits.begin())
                        std::cout << ", ";
                    std::cout << *it;
                }
                std::cout << "} ";
            }
            std::cout << "|";
            if (!inf.relatedQubits.empty()) {
                std::cout << " r{";
                for (auto it = inf.relatedQubits.begin();
                     it != inf.relatedQubits.end(); ++it) {
                    if (it != inf.relatedQubits.begin())
                        std::cout << ", ";
                    std::cout << *it;
                }
                std::cout << "}";
            }
            std::cout << " | " << inf.maxGateNumber << "\n";
        }
    }
};

void DoDiagonalFusion(Circuit &circuit) {
    std::set<qubit_size_t> targetQubits;
    int addFlag = 0;
    std::vector<Gate> subGateList;
    for (size_t i = 0; i < circuit.size(); ++i) {
        const auto &subgate = circuit[i];
        if (subgate.gateType == "RZ") {
            bool firstInset = targetQubits.contains(subgate.qubits[0]);
            if (firstInset && targetQubits.size() <= gMaxFusionSize) {
                subGateList.push_back(subgate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else if (!firstInset &&
                       targetQubits.size() <= gMaxFusionSize - 1) {
                targetQubits.insert(subgate.qubits[0]);
                subGateList.push_back(subgate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else {
                addFlag++;
            }
        } else if (subgate.gateType == "CZ" || subgate.gateType == "CP" ||
                   subgate.gateType == "RZZ") {
            bool firstInset = targetQubits.contains(subgate.qubits[0]);
            bool secondInset = targetQubits.contains(subgate.qubits[1]);
            if (!firstInset && !secondInset &&
                targetQubits.size() <= gMaxFusionSize - 2) {
                targetQubits.insert(subgate.qubits[0]);
                targetQubits.insert(subgate.qubits[1]);
                subGateList.push_back(subgate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else if (firstInset && !secondInset &&
                       targetQubits.size() <= gMaxFusionSize - 1) {
                targetQubits.insert(subgate.qubits[1]);
                subGateList.push_back(subgate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else if ((firstInset ^ secondInset) &&
                       targetQubits.size() <= gMaxFusionSize - 1) {
                targetQubits.insert(subgate.qubits[firstInset ? 1 : 0]);
                subGateList.push_back(subgate);
                circuit.gates.erase(circuit.gates.begin() + i);
                i--;
            } else {
                addFlag++;
            }
        } else if (targetQubits.size() > 0)
            addFlag++;
        if (addFlag && targetQubits.size() > 0) {
            Gate newGate("D", targetQubits, subGateList);
            circuit.gates.insert(circuit.gates.begin() + i, newGate);
            subGateList.clear();
            addFlag = 0;
            targetQubits.clear();
        }
    }
    if (!targetQubits.empty()) {
        Gate newGate("D", targetQubits, subGateList);
        circuit.gates.push_back(newGate);
    }
    for (auto &gate : circuit.gates) {
        if (gate.gateType == "D") {
            if (gate.subGateList.size() == 1) {
                // revert the conversion due to only one gate
                gate = gate.subGateList[0];
                continue;
            }

            gate.finfo.fid = gate.subGateList[0].gids[0];
            gate.gids.reserve(gate.subGateList.size());
            for (const auto &subgate : gate.subGateList) { // collect gids
                gate.gids.push_back(subgate.gids[0]);
            }

            Gate tmpGate = Circuit(gate.subGateList).fusedToGate();
            gate.gateType = std::move(tmpGate.gateType);
            gate.qubits = std::move(tmpGate.qubits);
            gate.sortedQubits = std::move(tmpGate.sortedQubits);
            gate.params = std::move(tmpGate.params);
            gate.subGateList.clear();
        }
    }
}

std::vector<std::vector<Gate>> GetPGFS(const Circuit &circuit) {
    std::vector<std::vector<Gate>> fusionGateList;
    // Step 1: construct 1-qubit fusion list (sequential, fast)
    std::vector<Gate> NQubitFusionList;
    for (gate_size_t gateIndex = 0; gateIndex < circuit.size(); gateIndex++) {
        const Gate &gate = circuit.gates[gateIndex];
        Gate wrapper(gate.gateType, gate.sortedQubits, {1, gateIndex},
                     gate.gids);
        Gate innerGate({1, gateIndex}, gate);
        wrapper.subGateList.push_back(innerGate);
        NQubitFusionList.push_back(wrapper);
    }
    fusionGateList.push_back(NQubitFusionList);

    // Step 2: parallel generation of multi-qubit fusion lists
    std::vector<std::future<std::vector<Gate>>> futures;
    FusionList fusionList(fusionGateList[0]);
    const auto &fusionGateList0 = fusionGateList[0];
    for (qubit_size_t fSize = 2; fSize <= gMaxFusionSize; ++fSize) {
        futures.push_back(std::async(std::launch::async, [fSize, fusionList,
                                                          fusionGateList0]() {
            std::vector<Gate> nQubitFusionList;
            FusionList nowFusionList = fusionList; // Assume copy is cheap
            nowFusionList.reNewList();
            gate_size_t gateIndex = 0;
            while (!nowFusionList.infoList.empty()) {
                Gate wrapper({fSize, gateIndex++});
                std::vector<int> gateToFused =
                    nowFusionList.getFusedGate(fSize);
                // note: we don't need to check if the fused gate is diagonal
                // and set the gateType of warpper here
                wrapper.subGateList.reserve(gateToFused.size());
                for (int index : gateToFused) {
                    auto &subgate = fusionGateList0[index].subGateList[0];
                    wrapper.subGateList.push_back(subgate);
                    wrapper.sortedQubits.insert(subgate.sortedQubits.begin(),
                                                subgate.sortedQubits.end());
                }
                nQubitFusionList.push_back(wrapper);
                nowFusionList.reNewList();
            }
            return nQubitFusionList;
        }));
    }

    // Step 3: collect results in order
    for (auto &future : futures) {
        fusionGateList.push_back(future.get());
    }
    return fusionGateList;
}

void searchAndOutputFusionCircuit(
    const std::string &outputFileName,
    const std::vector<std::vector<Gate>> &subFusionGateList,
    const std::vector<std::vector<Gate>> &fusionGateList) {
    gSmallCircuitCounter.fetch_add(1, std::memory_order_relaxed);
    std::atomic<double> smallWeight = DBL_MAX;
    std::vector<std::set<int>> dependencyList =
        constructDependencyList(subFusionGateList[0]);
    std::vector<Finfo> finalGateList, nowGateList;
    SmallWeightNode().getSmallWeight(subFusionGateList, dependencyList, 0,
                                     smallWeight, nowGateList, finalGateList,
                                     gPool, gMutex, 0);
    outputFusionCircuit(outputFileName, fusionGateList, finalGateList);
}

void GetOptimalGFS(const std::string &outputFileName,
                   const std::vector<std::vector<Gate>> &fusionGateList) {
    // find best fusion conbination
    // execute small gate block one by one to reduce execution time
    // because the getSmallWeight and shortestPath use different file output
    // system, the output logic cannot be decoupled
    const std::string cmd = "rm " + outputFileName + " > /dev/null 2>&1";
    [[maybe_unused]] auto sysinfo = system(cmd.c_str());

    [[unlikely]] if (gMethod <= 1) { // legacy; not tested
        searchAndOutputFusionCircuit(outputFileName, fusionGateList,
                                     fusionGateList);
        return;
    }
    std::vector<std::vector<Gate>> subFusionGateList(gMaxFusionSize);
    std::vector<std::vector<int>> recordIndex(gMaxFusionSize);
    constexpr int SMALL_CIRCUIT_SIZE = 8, BIG_CIRCUIT_SIZE = 15;
    for (const auto &maxFGate : fusionGateList[gMaxFusionSize - 1]) {
        for (const auto &subgate : maxFGate.subGateList) {
            subFusionGateList[0].push_back(
                fusionGateList[0][subgate.finfo.fid]);
            for (size_t i = 0; i < gMaxFusionSize; ++i)
                recordIndex[i].push_back(subgate.finfo.fid);
        }
        for (qubit_size_t fSize = 1; fSize < gMaxFusionSize - 1; ++fSize) {
            for (const auto &wrapper : fusionGateList[fSize]) {
                int flag = 1;
                std::vector<int> reIndex;
                reIndex.reserve(wrapper.subGateList.size());
                for (const auto &subgate : wrapper.subGateList) {
                    // Check if all fusion indices of the sub-gates are a
                    // subset of the recordIndex for this fusion size.
                    if (find(recordIndex[fSize].begin(),
                             recordIndex[fSize].end(),
                             subgate.finfo.fid) == recordIndex[fSize].end()) {
                        flag = 0;
                        recordIndex[fSize].insert(recordIndex[fSize].end(),
                                                  reIndex.begin(),
                                                  reIndex.end());
                        break;
                    }
                    reIndex.push_back(subgate.finfo.fid);
                    std::erase(recordIndex[fSize], subgate.finfo.fid);
                }
                if (flag)
                    subFusionGateList[fSize].push_back(wrapper);
            }
        }
        subFusionGateList[gMaxFusionSize - 1].push_back(maxFGate);

        int flag = 1;
        for (qubit_size_t fSize = 1; fSize < gMaxFusionSize - 1; ++fSize)
            if (!recordIndex[fSize].empty())
                flag = 0;
        size_t smallCircuitSize = subFusionGateList[0].size();

        if (flag && smallCircuitSize >= SMALL_CIRCUIT_SIZE) {
            // choose different strategies with different circuit size
            DEBUG_SECTION(
                DEBUG_shortestPath || DEBUG_getSmallWeight, int allsize = 0;
                for (const auto &s
                     : subFusionGateList) { allsize += s.size(); } std::cout
                << "smallCircuitSize: " << smallCircuitSize
                << ", allsize: " << allsize << std::endl;);
            if (smallCircuitSize < BIG_CIRCUIT_SIZE) {
                searchAndOutputFusionCircuit(outputFileName, subFusionGateList,
                                             fusionGateList);
            } else {
                gShortestPathCounter.fetch_add(1, std::memory_order_relaxed);
                DAG subDAG(smallCircuitSize);
                subDAG.constructDAG(subFusionGateList[0]);
                subDAG.shortestPath(subFusionGateList[0], outputFileName);
                DEBUG_SECTION(DEBUG_shortestPath,
                              Circuit(subFusionGateList[0]).dump(););
                DEBUG_SECTION(DEBUG_shortestPath, subDAG.dump(););
                DEBUG_SECTION(
                    DEBUG_shortestPath,
                    subDAG.dumpDot("graph_" +
                                   std::to_string(gShortestPathCounter) +
                                   ".dot"););
            }
            for (size_t i = 0; i < gMaxFusionSize; ++i) {
                subFusionGateList[i].clear();
                recordIndex[i].clear();
            }
        }
    }
    if (subFusionGateList[0].size() >= BIG_CIRCUIT_SIZE)
        throw std::runtime_error(
            "subFusionGateList[0].size() should not >= 10");
    if (std::any_of(subFusionGateList.begin(), subFusionGateList.end(),
                    [](const std::vector<Gate> &v) { return !v.empty(); })) {
        // The subFusionGateList is not empty; however, it is being ignored
        // because smallCircuitSize is less than 6.
        searchAndOutputFusionCircuit(outputFileName, subFusionGateList,
                                     fusionGateList);
    }
}

std::string getEnvVariable(const std::string &var_name) {
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

    if (gMethod == 3) {
        gCostFactor = 4.0; // calibraction: 1.8 / (1287/1277) * (390/174)
    }
    if (gMethod == 4 || gMethod == 6 || gMethod == 7 || gMethod == 8) {
        std::string gateTimeFilenamme = getEnvVariable("DYNAMIC_COST_FILENAME");
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

    // do reorder
    time_start = std::chrono::steady_clock::now();
    for (size_t fSize = 1; fSize < gMaxFusionSize; ++fSize) {
        Circuit cc(fusionGateList[fSize]); // build circuit of fSize
        fusionGateList[fSize] = cc.schedule().gates;
        gate_size_t index = 0;
        for (auto &wrapper : fusionGateList[fSize])
            wrapper.finfo.fid = index++;
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
    std::cout << "Small: " << gSmallCircuitCounter << " (DFS: " << gDFSCounter
              << ", "
              << "Early stop: " << gEarlyStopCounter
              << ")"
                 "; "
              << "Shortest: " << gShortestPathCounter << "\n";
    return 0;
}
