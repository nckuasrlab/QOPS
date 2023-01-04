OPENQASM 3;
include "qelib1.inc";

const n = 10;

qubit q[n];

for i in [0:n]{
    h q[i];
}

bit c[n];
c = measure q;

for i in [0:n]{
    print("bit result", i, "=", c[i]);
}