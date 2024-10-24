opt_input_format: 
gate_num numCtrls numTargs val_num ctrls [3] targs [3] real_matrix[8][8] imag_matrix[8][8]

0. unitary_single
void unitary (Qureg qureg, int targetQubit, ComplexMatrix2 u) -> 是一個 2*2 matrix 
opt_input_format: 
0 0 1 4 X targetQubit real_matrix[2][2] imag_matrix[2][2] 

1. two_unitary
void twoQubitUnitary (Qureg qureg int targetQubit1 int targetQubit2 ComplexMatrix4 u) -> 是一個 4*4 matrix 
opt_input_format: 
1 0 2 16 X targetQubit real_matrix[4][4] imag_matrix[4][4]

2. three_unitary
void multiQubitUnitary (Qureg qureg int *targs int numTargs ComplexMatrixN u) -> 是一個 8*8 matrix 
opt_input_format: 
2 0 3 64 X targetQubit real_matrix[8][8] imag_matrix[8][8]

3. H
void hadamard (Qureg qureg int targetQubit)
opt_input_format: 
3 0 1 4 X targetQubit real_matrix[2][2] imag_matrix[2][2] 

4. S
void sGate (Qureg qureg int targetQubit)
opt_input_format: 
4 0 1 4 X targetQubit real_matrix[2][2] imag_matrix[2][2] 

5. T
void tGate (Qureg qureg int targetQubit)
opt_input_format: 
5 0 1 4 X targetQubit real_matrix[2][2] imag_matrix[2][2] 

6. X
void pauliX (Qureg qureg int targetQubit)
opt_input_format: 
6 0 1 4 X targetQubit real_matrix[2][2] imag_matrix[2][2] 

7. Y
void pauliY (Qureg qureg int targetQubit)
opt_input_format: 
7 0 1 4 X targetQubit real_matrix[2][2] imag_matrix[2][2] 

8. Z 
void pauliZ (Qureg qureg int targetQubit)
opt_input_format: 
8 0 1 4 X targetQubit real_matrix[2][2] imag_matrix[2][2] 

9. CX 
void controlledNot (Qureg qureg int controlQubit int targetQubit)
opt_input_format: 
9 1 1 16 controlQubit targetQubit real_matrix[4][4] imag_matrix[4][4]

10. CZ 
void controlledPhaseFlip (Qureg qureg int idQubit1 int idQubit2)
opt_input_format: 
10 1 1 16 controlQubit targetQubit real_matrix[4][4] imag_matrix[4][4]

11. CT (改名：CP)
void controlledPhaseShift (Qureg qureg int idQubit1 int idQubit2 qreal angle)
opt_input_format: 
11 1 1 16 controlQubit targetQubit real_matrix[4][4] imag_matrix[4][4]

12. SWAP
void swapGate (Qureg qureg int qubit1 int qubit2)
opt_input_format: 
12 1 1 16 controlQubit targetQubit real_matrix[4][4] imag_matrix[4][4]

13. Toffoli
void multiControlledMultiQubitNot (Qureg qureg int *ctrls int numCtrls int *targs int numTargs)
opt_input_format: 
13 2 1 64 ctrls [2] targs [1] real_matrix[8][8] imag_matrix[8][8]

