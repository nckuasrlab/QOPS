OPENQASM 2.0;
include "qelib1.inc";
qreg qrg_nWlrB[24];
creg qrg_nWlrB_c[24];
swap qrg_nWlrB[4], qrg_nWlrB[18];
swap qrg_nWlrB[3], qrg_nWlrB[11];
swap qrg_nWlrB[1], qrg_nWlrB[20];
CX qrg_nWlrB[11], qrg_nWlrB[2];
CX qrg_nWlrB[8], qrg_nWlrB[7];
CX qrg_nWlrB[14], qrg_nWlrB[13];
CX qrg_nWlrB[21], qrg_nWlrB[1];
CX qrg_nWlrB[11], qrg_nWlrB[18];
CX qrg_nWlrB[8], qrg_nWlrB[9];
CX qrg_nWlrB[14], qrg_nWlrB[15];
CX qrg_nWlrB[21], qrg_nWlrB[22];
h qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[20];
t qrg_nWlrB[11];
h qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[20];
t qrg_nWlrB[0];
tdg qrg_nWlrB[20];
CX qrg_nWlrB[0], qrg_nWlrB[20];
h qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[6];
t qrg_nWlrB[8];
h qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[6];
t qrg_nWlrB[5];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
h qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[12];
t qrg_nWlrB[14];
h qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[12];
t qrg_nWlrB[3];
tdg qrg_nWlrB[12];
CX qrg_nWlrB[3], qrg_nWlrB[12];
h qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[19];
t qrg_nWlrB[21];
h qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[19];
t qrg_nWlrB[4];
tdg qrg_nWlrB[19];
CX qrg_nWlrB[4], qrg_nWlrB[19];
h qrg_nWlrB[18];
CX qrg_nWlrB[11], qrg_nWlrB[18];
tdg qrg_nWlrB[18];
CX qrg_nWlrB[2], qrg_nWlrB[18];
t qrg_nWlrB[18];
CX qrg_nWlrB[11], qrg_nWlrB[18];
tdg qrg_nWlrB[18];
CX qrg_nWlrB[2], qrg_nWlrB[18];
t qrg_nWlrB[11];
t qrg_nWlrB[18];
h qrg_nWlrB[18];
CX qrg_nWlrB[2], qrg_nWlrB[11];
t qrg_nWlrB[2];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[2], qrg_nWlrB[11];
h qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[7], qrg_nWlrB[9];
t qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[7], qrg_nWlrB[9];
t qrg_nWlrB[8];
t qrg_nWlrB[9];
h qrg_nWlrB[9];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[15];
CX qrg_nWlrB[14], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[13], qrg_nWlrB[15];
t qrg_nWlrB[15];
CX qrg_nWlrB[14], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[13], qrg_nWlrB[15];
t qrg_nWlrB[14];
t qrg_nWlrB[15];
h qrg_nWlrB[15];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
h qrg_nWlrB[22];
CX qrg_nWlrB[21], qrg_nWlrB[22];
tdg qrg_nWlrB[22];
CX qrg_nWlrB[1], qrg_nWlrB[22];
t qrg_nWlrB[22];
CX qrg_nWlrB[21], qrg_nWlrB[22];
tdg qrg_nWlrB[22];
CX qrg_nWlrB[1], qrg_nWlrB[22];
t qrg_nWlrB[21];
t qrg_nWlrB[22];
h qrg_nWlrB[22];
CX qrg_nWlrB[1], qrg_nWlrB[21];
t qrg_nWlrB[1];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[1], qrg_nWlrB[21];
h qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[21];
t qrg_nWlrB[23];
h qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[21];
t qrg_nWlrB[1];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[1], qrg_nWlrB[21];
CX qrg_nWlrB[6], qrg_nWlrB[5];
CX qrg_nWlrB[12], qrg_nWlrB[3];
CX qrg_nWlrB[19], qrg_nWlrB[4];
CX qrg_nWlrB[5], qrg_nWlrB[8];
CX qrg_nWlrB[3], qrg_nWlrB[14];
CX qrg_nWlrB[4], qrg_nWlrB[21];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
h qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[21];
t qrg_nWlrB[23];
h qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[21];
t qrg_nWlrB[1];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[1], qrg_nWlrB[21];
h qrg_nWlrB[17];
CX qrg_nWlrB[23], qrg_nWlrB[17];
tdg qrg_nWlrB[17];
CX qrg_nWlrB[16], qrg_nWlrB[17];
t qrg_nWlrB[17];
CX qrg_nWlrB[23], qrg_nWlrB[17];
tdg qrg_nWlrB[17];
CX qrg_nWlrB[16], qrg_nWlrB[17];
t qrg_nWlrB[23];
t qrg_nWlrB[17];
h qrg_nWlrB[17];
CX qrg_nWlrB[16], qrg_nWlrB[23];
t qrg_nWlrB[16];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[16], qrg_nWlrB[23];
h qrg_nWlrB[22];
CX qrg_nWlrB[23], qrg_nWlrB[22];
tdg qrg_nWlrB[22];
CX qrg_nWlrB[15], qrg_nWlrB[22];
t qrg_nWlrB[22];
CX qrg_nWlrB[23], qrg_nWlrB[22];
tdg qrg_nWlrB[22];
CX qrg_nWlrB[15], qrg_nWlrB[22];
t qrg_nWlrB[23];
t qrg_nWlrB[22];
h qrg_nWlrB[22];
CX qrg_nWlrB[15], qrg_nWlrB[23];
t qrg_nWlrB[15];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[15], qrg_nWlrB[23];
h qrg_nWlrB[9];
CX qrg_nWlrB[10], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[18], qrg_nWlrB[9];
t qrg_nWlrB[9];
CX qrg_nWlrB[10], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[18], qrg_nWlrB[9];
t qrg_nWlrB[10];
t qrg_nWlrB[9];
h qrg_nWlrB[9];
CX qrg_nWlrB[18], qrg_nWlrB[10];
t qrg_nWlrB[18];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[18], qrg_nWlrB[10];
h qrg_nWlrB[22];
CX qrg_nWlrB[17], qrg_nWlrB[22];
tdg qrg_nWlrB[22];
CX qrg_nWlrB[9], qrg_nWlrB[22];
t qrg_nWlrB[22];
CX qrg_nWlrB[17], qrg_nWlrB[22];
tdg qrg_nWlrB[22];
CX qrg_nWlrB[9], qrg_nWlrB[22];
t qrg_nWlrB[17];
t qrg_nWlrB[22];
h qrg_nWlrB[22];
CX qrg_nWlrB[9], qrg_nWlrB[17];
t qrg_nWlrB[9];
tdg qrg_nWlrB[17];
CX qrg_nWlrB[9], qrg_nWlrB[17];
h qrg_nWlrB[15];
CX qrg_nWlrB[16], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[9], qrg_nWlrB[15];
t qrg_nWlrB[15];
CX qrg_nWlrB[16], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[9], qrg_nWlrB[15];
t qrg_nWlrB[16];
t qrg_nWlrB[15];
h qrg_nWlrB[15];
CX qrg_nWlrB[9], qrg_nWlrB[16];
t qrg_nWlrB[9];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[9], qrg_nWlrB[16];
h qrg_nWlrB[17];
CX qrg_nWlrB[23], qrg_nWlrB[17];
tdg qrg_nWlrB[17];
CX qrg_nWlrB[16], qrg_nWlrB[17];
t qrg_nWlrB[17];
CX qrg_nWlrB[23], qrg_nWlrB[17];
tdg qrg_nWlrB[17];
CX qrg_nWlrB[16], qrg_nWlrB[17];
t qrg_nWlrB[23];
t qrg_nWlrB[17];
h qrg_nWlrB[17];
CX qrg_nWlrB[16], qrg_nWlrB[23];
t qrg_nWlrB[16];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[16], qrg_nWlrB[23];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
h qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[21];
t qrg_nWlrB[23];
h qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[21];
t qrg_nWlrB[1];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[1], qrg_nWlrB[21];
CX qrg_nWlrB[5], qrg_nWlrB[8];
CX qrg_nWlrB[3], qrg_nWlrB[14];
CX qrg_nWlrB[4], qrg_nWlrB[21];
CX qrg_nWlrB[6], qrg_nWlrB[5];
CX qrg_nWlrB[12], qrg_nWlrB[3];
CX qrg_nWlrB[19], qrg_nWlrB[4];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
h qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[23];
CX qrg_nWlrB[21], qrg_nWlrB[23];
tdg qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[23];
t qrg_nWlrB[21];
t qrg_nWlrB[23];
h qrg_nWlrB[23];
CX qrg_nWlrB[1], qrg_nWlrB[21];
t qrg_nWlrB[1];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[1], qrg_nWlrB[21];
h qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[20];
t qrg_nWlrB[11];
h qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[20];
t qrg_nWlrB[0];
tdg qrg_nWlrB[20];
CX qrg_nWlrB[0], qrg_nWlrB[20];
h qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[6];
t qrg_nWlrB[8];
h qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[6];
t qrg_nWlrB[5];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
h qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[12];
t qrg_nWlrB[14];
h qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[12];
t qrg_nWlrB[3];
tdg qrg_nWlrB[12];
CX qrg_nWlrB[3], qrg_nWlrB[12];
h qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[19];
t qrg_nWlrB[21];
h qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[19];
t qrg_nWlrB[4];
tdg qrg_nWlrB[19];
CX qrg_nWlrB[4], qrg_nWlrB[19];
CX qrg_nWlrB[11], qrg_nWlrB[2];
CX qrg_nWlrB[8], qrg_nWlrB[7];
CX qrg_nWlrB[14], qrg_nWlrB[13];
CX qrg_nWlrB[21], qrg_nWlrB[1];
CX qrg_nWlrB[6], qrg_nWlrB[5];
CX qrg_nWlrB[12], qrg_nWlrB[3];
CX qrg_nWlrB[19], qrg_nWlrB[4];
CX qrg_nWlrB[6], qrg_nWlrB[8];
CX qrg_nWlrB[12], qrg_nWlrB[14];
CX qrg_nWlrB[19], qrg_nWlrB[21];
CX qrg_nWlrB[18], qrg_nWlrB[6];
CX qrg_nWlrB[9], qrg_nWlrB[12];
CX qrg_nWlrB[15], qrg_nWlrB[19];
h qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[20];
t qrg_nWlrB[11];
h qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[20];
t qrg_nWlrB[0];
tdg qrg_nWlrB[20];
CX qrg_nWlrB[0], qrg_nWlrB[20];
h qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[6];
t qrg_nWlrB[8];
h qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[6];
t qrg_nWlrB[5];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
h qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[12];
t qrg_nWlrB[14];
h qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[12];
t qrg_nWlrB[3];
tdg qrg_nWlrB[12];
CX qrg_nWlrB[3], qrg_nWlrB[12];
h qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[19];
t qrg_nWlrB[21];
h qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[19];
t qrg_nWlrB[4];
tdg qrg_nWlrB[19];
CX qrg_nWlrB[4], qrg_nWlrB[19];
CX qrg_nWlrB[11], qrg_nWlrB[2];
CX qrg_nWlrB[8], qrg_nWlrB[7];
CX qrg_nWlrB[14], qrg_nWlrB[13];
CX qrg_nWlrB[21], qrg_nWlrB[1];
h qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[20];
t qrg_nWlrB[11];
h qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[20];
t qrg_nWlrB[0];
tdg qrg_nWlrB[20];
CX qrg_nWlrB[0], qrg_nWlrB[20];
h qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[6];
t qrg_nWlrB[8];
h qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[6];
t qrg_nWlrB[5];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
h qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[12];
t qrg_nWlrB[14];
h qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[12];
t qrg_nWlrB[3];
tdg qrg_nWlrB[12];
CX qrg_nWlrB[3], qrg_nWlrB[12];
h qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[21];
CX qrg_nWlrB[19], qrg_nWlrB[21];
tdg qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[21];
t qrg_nWlrB[19];
t qrg_nWlrB[21];
h qrg_nWlrB[21];
CX qrg_nWlrB[4], qrg_nWlrB[19];
t qrg_nWlrB[4];
tdg qrg_nWlrB[19];
CX qrg_nWlrB[4], qrg_nWlrB[19];
CX qrg_nWlrB[6], qrg_nWlrB[5];
CX qrg_nWlrB[12], qrg_nWlrB[3];
CX qrg_nWlrB[19], qrg_nWlrB[4];
CX qrg_nWlrB[18], qrg_nWlrB[6];
CX qrg_nWlrB[9], qrg_nWlrB[12];
CX qrg_nWlrB[15], qrg_nWlrB[19];
CX qrg_nWlrB[6], qrg_nWlrB[8];
CX qrg_nWlrB[12], qrg_nWlrB[14];
CX qrg_nWlrB[19], qrg_nWlrB[21];
CX qrg_nWlrB[20], qrg_nWlrB[0];
CX qrg_nWlrB[6], qrg_nWlrB[5];
CX qrg_nWlrB[12], qrg_nWlrB[3];
CX qrg_nWlrB[19], qrg_nWlrB[4];
x qrg_nWlrB[0];
x qrg_nWlrB[2];
x qrg_nWlrB[5];
x qrg_nWlrB[7];
x qrg_nWlrB[3];
x qrg_nWlrB[13];
CX qrg_nWlrB[11], qrg_nWlrB[2];
CX qrg_nWlrB[8], qrg_nWlrB[7];
CX qrg_nWlrB[14], qrg_nWlrB[13];
h qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[20];
t qrg_nWlrB[11];
h qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[20];
t qrg_nWlrB[0];
tdg qrg_nWlrB[20];
CX qrg_nWlrB[0], qrg_nWlrB[20];
h qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[6];
t qrg_nWlrB[8];
h qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[6];
t qrg_nWlrB[5];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
h qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[12];
t qrg_nWlrB[14];
h qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[12];
t qrg_nWlrB[3];
tdg qrg_nWlrB[12];
CX qrg_nWlrB[3], qrg_nWlrB[12];
CX qrg_nWlrB[6], qrg_nWlrB[5];
CX qrg_nWlrB[12], qrg_nWlrB[3];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
CX qrg_nWlrB[5], qrg_nWlrB[8];
CX qrg_nWlrB[3], qrg_nWlrB[14];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
h qrg_nWlrB[15];
CX qrg_nWlrB[16], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[9], qrg_nWlrB[15];
t qrg_nWlrB[15];
CX qrg_nWlrB[16], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[9], qrg_nWlrB[15];
t qrg_nWlrB[16];
t qrg_nWlrB[15];
h qrg_nWlrB[15];
CX qrg_nWlrB[9], qrg_nWlrB[16];
t qrg_nWlrB[9];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[9], qrg_nWlrB[16];
h qrg_nWlrB[9];
CX qrg_nWlrB[10], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[18], qrg_nWlrB[9];
t qrg_nWlrB[9];
CX qrg_nWlrB[10], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[18], qrg_nWlrB[9];
t qrg_nWlrB[10];
t qrg_nWlrB[9];
h qrg_nWlrB[9];
CX qrg_nWlrB[18], qrg_nWlrB[10];
t qrg_nWlrB[18];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[18], qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
CX qrg_nWlrB[5], qrg_nWlrB[8];
CX qrg_nWlrB[3], qrg_nWlrB[14];
h qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[10];
CX qrg_nWlrB[8], qrg_nWlrB[10];
tdg qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[10];
t qrg_nWlrB[8];
t qrg_nWlrB[10];
h qrg_nWlrB[10];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[16];
CX qrg_nWlrB[14], qrg_nWlrB[16];
tdg qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[16];
t qrg_nWlrB[14];
t qrg_nWlrB[16];
h qrg_nWlrB[16];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
CX qrg_nWlrB[6], qrg_nWlrB[5];
CX qrg_nWlrB[12], qrg_nWlrB[3];
h qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[7], qrg_nWlrB[9];
t qrg_nWlrB[9];
CX qrg_nWlrB[8], qrg_nWlrB[9];
tdg qrg_nWlrB[9];
CX qrg_nWlrB[7], qrg_nWlrB[9];
t qrg_nWlrB[8];
t qrg_nWlrB[9];
h qrg_nWlrB[9];
CX qrg_nWlrB[7], qrg_nWlrB[8];
t qrg_nWlrB[7];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[7], qrg_nWlrB[8];
h qrg_nWlrB[15];
CX qrg_nWlrB[14], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[13], qrg_nWlrB[15];
t qrg_nWlrB[15];
CX qrg_nWlrB[14], qrg_nWlrB[15];
tdg qrg_nWlrB[15];
CX qrg_nWlrB[13], qrg_nWlrB[15];
t qrg_nWlrB[14];
t qrg_nWlrB[15];
h qrg_nWlrB[15];
CX qrg_nWlrB[13], qrg_nWlrB[14];
t qrg_nWlrB[13];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[13], qrg_nWlrB[14];
h qrg_nWlrB[18];
CX qrg_nWlrB[11], qrg_nWlrB[18];
tdg qrg_nWlrB[18];
CX qrg_nWlrB[2], qrg_nWlrB[18];
t qrg_nWlrB[18];
CX qrg_nWlrB[11], qrg_nWlrB[18];
tdg qrg_nWlrB[18];
CX qrg_nWlrB[2], qrg_nWlrB[18];
t qrg_nWlrB[11];
t qrg_nWlrB[18];
h qrg_nWlrB[18];
CX qrg_nWlrB[2], qrg_nWlrB[11];
t qrg_nWlrB[2];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[2], qrg_nWlrB[11];
h qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[11];
CX qrg_nWlrB[20], qrg_nWlrB[11];
tdg qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[11];
t qrg_nWlrB[20];
t qrg_nWlrB[11];
h qrg_nWlrB[11];
CX qrg_nWlrB[0], qrg_nWlrB[20];
t qrg_nWlrB[0];
tdg qrg_nWlrB[20];
CX qrg_nWlrB[0], qrg_nWlrB[20];
h qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[8];
CX qrg_nWlrB[6], qrg_nWlrB[8];
tdg qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[8];
t qrg_nWlrB[6];
t qrg_nWlrB[8];
h qrg_nWlrB[8];
CX qrg_nWlrB[5], qrg_nWlrB[6];
t qrg_nWlrB[5];
tdg qrg_nWlrB[6];
CX qrg_nWlrB[5], qrg_nWlrB[6];
h qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[14];
CX qrg_nWlrB[12], qrg_nWlrB[14];
tdg qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[14];
t qrg_nWlrB[12];
t qrg_nWlrB[14];
h qrg_nWlrB[14];
CX qrg_nWlrB[3], qrg_nWlrB[12];
t qrg_nWlrB[3];
tdg qrg_nWlrB[12];
CX qrg_nWlrB[3], qrg_nWlrB[12];
CX qrg_nWlrB[11], qrg_nWlrB[18];
CX qrg_nWlrB[8], qrg_nWlrB[9];
CX qrg_nWlrB[14], qrg_nWlrB[15];
CX qrg_nWlrB[11], qrg_nWlrB[2];
CX qrg_nWlrB[8], qrg_nWlrB[7];
CX qrg_nWlrB[14], qrg_nWlrB[13];
x qrg_nWlrB[0];
x qrg_nWlrB[2];
x qrg_nWlrB[5];
x qrg_nWlrB[7];
x qrg_nWlrB[3];
x qrg_nWlrB[13];
swap qrg_nWlrB[1], qrg_nWlrB[20];
swap qrg_nWlrB[3], qrg_nWlrB[11];
swap qrg_nWlrB[4], qrg_nWlrB[18];
