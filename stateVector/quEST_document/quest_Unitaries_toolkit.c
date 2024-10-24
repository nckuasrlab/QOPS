我們有的：

unitaries: 
unitary
void unitary (Qureg qureg, int targetQubit, ComplexMatrix2 u)
two_unitary
void twoQubitUnitary (Qureg qureg, int targetQubit1, int targetQubit2, ComplexMatrix4 u)
three_unitary
void multiQubitUnitary (Qureg qureg, int *targs, int numTargs, ComplexMatrixN u)
// void compactUnitary (Qureg qureg, int targetQubit, Complex alpha, Complex beta)

CX 
void controlledNot (Qureg qureg, int controlQubit, int targetQubit)
CZ 
void controlledPhaseFlip (Qureg qureg, int idQubit1, int idQubit2)


CS  (他們好像沒有特別寫這個，都用這個做替代)
CT  (和這個相符) 
void controlledPhaseShift (Qureg qureg, int idQubit1, int idQubit2, qreal angle)
// (可以藉由使用者自行填入的 theta 完成此 gate) 
// void rotateAroundAxis (Qureg qureg, int rotQubit, qreal angle, Vector axis)


SWAP
void swapGate (Qureg qureg, int qubit1, int qubit2)

H
void hadamard (Qureg qureg, int targetQubit)

S
void sGate (Qureg qureg, int targetQubit)

T
void tGate (Qureg qureg, int targetQubit)

X
void pauliX (Qureg qureg, int targetQubit)

Y
void pauliY (Qureg qureg, int targetQubit)

Z 
void pauliZ (Qureg qureg, int targetQubit)


Toffoli
void multiControlledMultiQubitNot (Qureg qureg, int *ctrls, int numCtrls, int *targs, int numTargs)
這部份他們做的比較完整，我的最多只能吃到 兩個 control, 一個 target

* 下面有四個 gate 是我覺得值得建立的。
----------------------------------------------------------------------------------------------------------------------------------

QuEST 有的：

// void compactUnitary (Qureg qureg, int targetQubit, Complex alpha, Complex beta)

(我覺得可以建一下) void controlledCompactUnitary (Qureg qureg, int controlQubit, int targetQubit, Complex alpha, Complex beta)

void controlledMultiQubitUnitary (Qureg qureg, int ctrl, int *targs, int numTargs, ComplexMatrixN u)

// void controlledNot (Qureg qureg, int controlQubit, int targetQubit)

void controlledPauliY (Qureg qureg, int controlQubit, int targetQubit)

// void controlledPhaseFlip (Qureg qureg, int idQubit1, int idQubit2)

// void controlledPhaseShift (Qureg qureg, int idQubit1, int idQubit2, qreal angle)

void controlledRotateAroundAxis (Qureg qureg, int controlQubit, int targetQubit, qreal angle, Vector axis) 

void controlledRotateX (Qureg qureg, int controlQubit, int targetQubit, qreal angle)

void controlledRotateY (Qureg qureg, int controlQubit, int targetQubit, qreal angle)

void controlledRotateZ (Qureg qureg, int controlQubit, int targetQubit, qreal angle)

void controlledTwoQubitUnitary (Qureg qureg, int controlQubit, int targetQubit1, int targetQubit2, ComplexMatrix4 u)

(我覺得可以建一下) void controlledUnitary (Qureg qureg, int controlQubit, int targetQubit, ComplexMatrix2 u)

// void hadamard (Qureg qureg, int targetQubit)

// void multiControlledMultiQubitNot (Qureg qureg, int *ctrls, int numCtrls, int *targs, int numTargs)

void multiControlledMultiQubitUnitary (Qureg qureg, int *ctrls, int numCtrls, int *targs, int numTargs, ComplexMatrixN u)

void multiControlledMultiRotatePauli (Qureg qureg, int *controlQubits, int numControls, int *targetQubits, enum pauliOpType *targetPaulis, int numTargets, qreal angle)

void multiControlledMultiRotateZ (Qureg qureg, int *controlQubits, int numControls, int *targetQubits, int numTargets, qreal angle)

void multiControlledPhaseFlip (Qureg qureg, int *controlQubits, int numControlQubits)

void multiControlledPhaseShift (Qureg qureg, int *controlQubits, int numControlQubits, qreal angle)
 
void multiControlledTwoQubitUnitary (Qureg qureg, int *controlQubits, int numControlQubits, int targetQubit1, int targetQubit2, ComplexMatrix4 u)

void multiControlledUnitary (Qureg qureg, int *controlQubits, int numControlQubits, int targetQubit, ComplexMatrix2 u)

void multiQubitNot (Qureg qureg, int *targs, int numTargs)
 
void multiQubitUnitary (Qureg qureg, int *targs, int numTargs, ComplexMatrixN u)

void multiRotatePauli (Qureg qureg, int *targetQubits, enum pauliOpType *targetPaulis, int numTargets, qreal angle)

void multiRotateZ (Qureg qureg, int *qubits, int numQubits, qreal angle)

void multiStateControlledUnitary (Qureg qureg, int *controlQubits, int *controlState, int numControlQubits, int targetQubit, ComplexMatrix2 u)

// void pauliX (Qureg qureg, int targetQubit)

// void pauliY (Qureg qureg, int targetQubit)

// void pauliZ (Qureg qureg, int targetQubit)

(這可以從 S or T gate 那邊改過來, single qubit gate) void phaseShift (Qureg qureg, int targetQubit, qreal angle)

// void rotateAroundAxis (Qureg qureg, int rotQubit, qreal angle, Vector axis)
 

/*
以下三個會被 single unitary 完成
void rotateX (Qureg qureg, int rotQubit, qreal angle)
 
void rotateY (Qureg qureg, int rotQubit, qreal angle)
 
void rotateZ (Qureg qureg, int rotQubit, qreal angle)
*/
 
// void sGate (Qureg qureg, int targetQubit)

(這可以從 SWAP gate 那邊改過來, two qubit gate) void sqrtSwapGate (Qureg qureg, int qb1, int qb2)
 
// void swapGate (Qureg qureg, int qubit1, int qubit2)
 
// void tGate (Qureg qureg, int targetQubit)

// void twoQubitUnitary (Qureg qureg, int targetQubit1, int targetQubit2, ComplexMatrix4 u)

// void unitary (Qureg qureg, int targetQubit, ComplexMatrix2 u)
