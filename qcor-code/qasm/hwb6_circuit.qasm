OPENQASM 2.0;
include "qelib1.inc";
qreg qrg_nWlrB[7];
creg qrg_nWlrB_c[7];
x qrg_nWlrB[0];
CX qrg_nWlrB[2], qrg_nWlrB[0];
h qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[0], qrg_nWlrB[2];
t qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[0], qrg_nWlrB[2];
t qrg_nWlrB[1];
t qrg_nWlrB[2];
h qrg_nWlrB[2];
CX qrg_nWlrB[0], qrg_nWlrB[1];
t qrg_nWlrB[0];
tdg qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
CX qrg_nWlrB[5], qrg_nWlrB[3];
CX qrg_nWlrB[4], qrg_nWlrB[5];
h qrg_nWlrB[1];
CX qrg_nWlrB[2], qrg_nWlrB[1];
tdg qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
t qrg_nWlrB[1];
CX qrg_nWlrB[2], qrg_nWlrB[1];
tdg qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
t qrg_nWlrB[2];
t qrg_nWlrB[1];
h qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[2];
t qrg_nWlrB[0];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[0], qrg_nWlrB[2];
h qrg_nWlrB[5];
CX qrg_nWlrB[2], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[0], qrg_nWlrB[5];
t qrg_nWlrB[5];
CX qrg_nWlrB[2], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[0], qrg_nWlrB[5];
t qrg_nWlrB[2];
t qrg_nWlrB[5];
CX qrg_nWlrB[0], qrg_nWlrB[2];
t qrg_nWlrB[0];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[0], qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[3];
x qrg_nWlrB[1];
CX qrg_nWlrB[3], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[1], qrg_nWlrB[5];
t qrg_nWlrB[5];
CX qrg_nWlrB[3], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[1], qrg_nWlrB[5];
t qrg_nWlrB[3];
t qrg_nWlrB[5];
h qrg_nWlrB[5];
CX qrg_nWlrB[1], qrg_nWlrB[3];
t qrg_nWlrB[1];
tdg qrg_nWlrB[3];
CX qrg_nWlrB[1], qrg_nWlrB[3];
CX qrg_nWlrB[5], qrg_nWlrB[4];
CX qrg_nWlrB[0], qrg_nWlrB[4];
CX qrg_nWlrB[0], qrg_nWlrB[1];
h qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
tdg qrg_nWlrB[3];
CX qrg_nWlrB[1], qrg_nWlrB[3];
t qrg_nWlrB[3];
CX qrg_nWlrB[2], qrg_nWlrB[3];
tdg qrg_nWlrB[3];
CX qrg_nWlrB[1], qrg_nWlrB[3];
t qrg_nWlrB[2];
t qrg_nWlrB[3];
h qrg_nWlrB[3];
CX qrg_nWlrB[1], qrg_nWlrB[2];
t qrg_nWlrB[1];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[5];
x qrg_nWlrB[5];
h qrg_nWlrB[2];
CX qrg_nWlrB[5], qrg_nWlrB[2];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[4], qrg_nWlrB[2];
t qrg_nWlrB[2];
CX qrg_nWlrB[5], qrg_nWlrB[2];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[4], qrg_nWlrB[2];
t qrg_nWlrB[5];
t qrg_nWlrB[2];
h qrg_nWlrB[2];
CX qrg_nWlrB[4], qrg_nWlrB[5];
t qrg_nWlrB[4];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
h qrg_nWlrB[6];
CX qrg_nWlrB[4], qrg_nWlrB[6];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[2], qrg_nWlrB[6];
t qrg_nWlrB[6];
CX qrg_nWlrB[4], qrg_nWlrB[6];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[2], qrg_nWlrB[6];
t qrg_nWlrB[4];
t qrg_nWlrB[6];
h qrg_nWlrB[6];
CX qrg_nWlrB[2], qrg_nWlrB[4];
t qrg_nWlrB[2];
tdg qrg_nWlrB[4];
CX qrg_nWlrB[2], qrg_nWlrB[4];
h qrg_nWlrB[1];
CX qrg_nWlrB[5], qrg_nWlrB[1];
tdg qrg_nWlrB[1];
CX qrg_nWlrB[6], qrg_nWlrB[1];
t qrg_nWlrB[1];
CX qrg_nWlrB[5], qrg_nWlrB[1];
tdg qrg_nWlrB[1];
CX qrg_nWlrB[6], qrg_nWlrB[1];
t qrg_nWlrB[5];
t qrg_nWlrB[1];
h qrg_nWlrB[1];
CX qrg_nWlrB[6], qrg_nWlrB[5];
t qrg_nWlrB[6];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[6], qrg_nWlrB[5];
h qrg_nWlrB[6];
CX qrg_nWlrB[4], qrg_nWlrB[6];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[2], qrg_nWlrB[6];
t qrg_nWlrB[6];
CX qrg_nWlrB[4], qrg_nWlrB[6];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[2], qrg_nWlrB[6];
t qrg_nWlrB[4];
t qrg_nWlrB[6];
h qrg_nWlrB[6];
CX qrg_nWlrB[2], qrg_nWlrB[4];
t qrg_nWlrB[2];
tdg qrg_nWlrB[4];
CX qrg_nWlrB[2], qrg_nWlrB[4];
CX qrg_nWlrB[5], qrg_nWlrB[0];
CX qrg_nWlrB[0], qrg_nWlrB[3];
CX qrg_nWlrB[5], qrg_nWlrB[2];
h qrg_nWlrB[5];
CX qrg_nWlrB[2], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[1], qrg_nWlrB[5];
t qrg_nWlrB[5];
CX qrg_nWlrB[2], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[1], qrg_nWlrB[5];
t qrg_nWlrB[2];
t qrg_nWlrB[5];
CX qrg_nWlrB[1], qrg_nWlrB[2];
t qrg_nWlrB[1];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
CX qrg_nWlrB[3], qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[1];
CX qrg_nWlrB[4], qrg_nWlrB[0];
CX qrg_nWlrB[4], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[2], qrg_nWlrB[5];
t qrg_nWlrB[5];
CX qrg_nWlrB[4], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[2], qrg_nWlrB[5];
t qrg_nWlrB[4];
t qrg_nWlrB[5];
h qrg_nWlrB[5];
CX qrg_nWlrB[2], qrg_nWlrB[4];
t qrg_nWlrB[2];
tdg qrg_nWlrB[4];
CX qrg_nWlrB[2], qrg_nWlrB[4];
CX qrg_nWlrB[5], qrg_nWlrB[2];
CX qrg_nWlrB[4], qrg_nWlrB[5];
h qrg_nWlrB[3];
CX qrg_nWlrB[4], qrg_nWlrB[3];
tdg qrg_nWlrB[3];
CX qrg_nWlrB[1], qrg_nWlrB[3];
t qrg_nWlrB[3];
CX qrg_nWlrB[4], qrg_nWlrB[3];
tdg qrg_nWlrB[3];
CX qrg_nWlrB[1], qrg_nWlrB[3];
t qrg_nWlrB[4];
t qrg_nWlrB[3];
h qrg_nWlrB[3];
CX qrg_nWlrB[1], qrg_nWlrB[4];
t qrg_nWlrB[1];
tdg qrg_nWlrB[4];
CX qrg_nWlrB[1], qrg_nWlrB[4];
h qrg_nWlrB[5];
CX qrg_nWlrB[3], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[0], qrg_nWlrB[5];
t qrg_nWlrB[5];
CX qrg_nWlrB[3], qrg_nWlrB[5];
tdg qrg_nWlrB[5];
CX qrg_nWlrB[0], qrg_nWlrB[5];
t qrg_nWlrB[3];
t qrg_nWlrB[5];
h qrg_nWlrB[5];
CX qrg_nWlrB[0], qrg_nWlrB[3];
t qrg_nWlrB[0];
tdg qrg_nWlrB[3];
CX qrg_nWlrB[0], qrg_nWlrB[3];
x qrg_nWlrB[0];
CX qrg_nWlrB[0], qrg_nWlrB[2];
h qrg_nWlrB[0];
CX qrg_nWlrB[4], qrg_nWlrB[0];
tdg qrg_nWlrB[0];
CX qrg_nWlrB[1], qrg_nWlrB[0];
t qrg_nWlrB[0];
CX qrg_nWlrB[4], qrg_nWlrB[0];
tdg qrg_nWlrB[0];
CX qrg_nWlrB[1], qrg_nWlrB[0];
t qrg_nWlrB[4];
t qrg_nWlrB[0];
h qrg_nWlrB[0];
CX qrg_nWlrB[1], qrg_nWlrB[4];
t qrg_nWlrB[1];
tdg qrg_nWlrB[4];
CX qrg_nWlrB[1], qrg_nWlrB[4];
CX qrg_nWlrB[3], qrg_nWlrB[2];
h qrg_nWlrB[1];
CX qrg_nWlrB[2], qrg_nWlrB[1];
tdg qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
t qrg_nWlrB[1];
CX qrg_nWlrB[2], qrg_nWlrB[1];
tdg qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[1];
t qrg_nWlrB[2];
t qrg_nWlrB[1];
h qrg_nWlrB[1];
CX qrg_nWlrB[0], qrg_nWlrB[2];
t qrg_nWlrB[0];
tdg qrg_nWlrB[2];
CX qrg_nWlrB[0], qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[0];
CX qrg_nWlrB[2], qrg_nWlrB[4];
x qrg_nWlrB[2];
CX qrg_nWlrB[1], qrg_nWlrB[2];
CX qrg_nWlrB[0], qrg_nWlrB[1];
CX qrg_nWlrB[3], qrg_nWlrB[4];
CX qrg_nWlrB[5], qrg_nWlrB[3];
x qrg_nWlrB[5];
