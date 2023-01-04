OPENQASM 3;
include "qelib1.inc";

qubit q;

h q;

bit c;
c = measure q;

print("bit result 1 = ", c);