#include <math.h>
#include <omp.h>
#include "common.h"
#include "gate.h"
#include "gate_util.h"
#include "gate_chunk.h"

gate_args gate_move;

int gate_size;
int up_qubit;
int lo_qubit;

ull small_offset;
ull middle_offset;
ull large_offset;
ull half_small_offset;
ull half_middle_offset;
ull half_large_offset;

ull ctrl_offset;
ull targ_offset;
ull half_ctrl_offset;
ull half_targ_offset;

void (*gate_ops[16][2])(Type *) = {{H_gate, H_gate},
                                   {S_gate, Sc_gate},
                                   {T_gate, Tc_gate},
                                   {X_gate, X_gate},
                                   {Y_gate, Yc_gate},
                                   {Z_gate, Z_gate},
                                   {P_gate, Pc_gate},
                                   {U_gate, Uc_gate},

                                   {X_gate2, X_gate2},
                                   {Y_gate2, Yc_gate2},
                                   {Z_gate2, Z_gate2},
                                   {P_gate2, Pc_gate2},
                                   {U_gate2, Uc_gate2},
                                   {U2_gate, U2c_gate},
                                   {SWAP_gate, SWAP_gate},
                                   {U3_gate, U3c_gate}};
/*===================================================================
gate level (Type I)

prepare:
gate_move.half_targ
gate_size

void *_gate(Type *rd)
input: 指向一個存放gate_size這麼多state的buffer。
功能: 對buffer內的state操作，使他們變成作用完gate之後的樣子
===================================================================*/

/*
Hadamard gate

expressed in row-maj as:
  [[1/sqrt(2),  1/sqrt(2)]
   [1/sqrt(2), -1/sqrt(2)]]]
*/
void H_gate (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (ull i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (ull j = 0; j < gate_move.half_targ; j++){
            q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[up_off].real = 1./sqrt(2) * (q_0r + q_1r);
            q_rd[up_off].imag = 1./sqrt(2) * (q_0i + q_1i);
            q_rd[lo_off].real = 1./sqrt(2) * (q_0r - q_1r);
            q_rd[lo_off].imag = 1./sqrt(2) * (q_0i - q_1i);
            up_off += 1;
            lo_off += 1;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}

/*
S gate

expressed in row-maj as:
  [[1,  0]
   [0,  i]]
*/
void S_gate (Type *q_rd) {
    int lo_off = gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (ull i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (ull j = 0; j < gate_move.half_targ; j++){
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[lo_off].real = -q_1i;
            q_rd[lo_off].imag = q_1r;
            lo_off += 1;
        }
        lo_off += gate_move.half_targ;
    }
}

/*
Conjugated S gate

expressed in row-maj as:
  [[1,  0]
   [0, -i]]
*/
void Sc_gate (Type *q_rd) {
    int lo_off = gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (ull i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (ull j = 0; j < gate_move.half_targ; j++){
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            Type_t q_1r = q_rd[lo_off].real;
            Type_t q_1i = q_rd[lo_off].imag;
            q_rd[lo_off].real = -q_1i;
            q_rd[lo_off].imag = q_1r;
            lo_off += 1;
        }
        lo_off += gate_move.half_targ;
    }
}

/*
T gate

expressed in row-maj as:
  [[1,        0       ]
   [0,  exp(pi/4 * i)]]
which is equal to:
  [[1,        0                  ]
   [0,  cos(pi/4) + i * sin(pi/4)]]
Finally:
  [[1,        0                  ]
   [0,  1/sqrt(2) + i * 1/sqrt(2)]]
*/
void T_gate (Type *q_rd) {
    int lo_off = gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (ull i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (ull j = 0; j < gate_move.half_targ; j++){
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[lo_off].real = 1./sqrt(2) * (q_1r - q_1i);
            q_rd[lo_off].imag = 1./sqrt(2) * (q_1r + q_1i);
            lo_off += 1;
        }
        lo_off += gate_move.half_targ;
    }
}

/*
Conjugated T gate

expressed in row-maj as:
  [[1,        0                  ]
   [0,  1/sqrt(2) - i * 1/sqrt(2)]]
*/
void Tc_gate (Type *q_rd) {
    int lo_off = gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (ull i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (ull j = 0; j < gate_move.half_targ; j++){
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[lo_off].real = 1./sqrt(2) * (q_1r + q_1i);
            q_rd[lo_off].imag = 1./sqrt(2) * (q_1r - q_1i);
            lo_off += 1;
        }
        lo_off += gate_move.half_targ;
    }
}

/*
Pauli-X gate

expressed in row-maj as:
  [[0,  1]
   [1,  0]]
*/
void X_gate (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[up_off].real = q_1r;
            q_rd[up_off].imag = q_1i;
            q_rd[lo_off].real = q_0r;
            q_rd[lo_off].imag = q_0i;
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}

/*
Pauli-Y gate

expressed in row-maj as:
  [[0,  -i]
   [i,   0]]
*/
void Y_gate (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[up_off].real = q_1i;
            q_rd[up_off].imag = -q_1r;
            q_rd[lo_off].real = -q_0i;
            q_rd[lo_off].imag = q_0r;
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}

/*
Conjugated Pauli-Y gate

expressed in row-maj as:
  [[ 0, i]
   [-i, 0]]
*/
void Yc_gate (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[up_off].real = -q_1i;
            q_rd[up_off].imag = q_1r;
            q_rd[lo_off].real = q_0i;
            q_rd[lo_off].imag = -q_0r;
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}

/*
Pauli-Z gate

expressed in row-maj as:
  [[1,   0]
   [0,  -1]]
*/
void Z_gate (Type *q_rd) {
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[lo_off].real = -q_1r;
            q_rd[lo_off].imag = -q_1i;
            lo_off++;
        }
        lo_off += gate_move.half_targ;
    }
}

/*
Phase gate

expressed in row-maj as:
  [[1,   0]
   [0,  exp(theta)]]
where theta stored at real[0]
*/
void P_gate (Type *q_rd) {
    // printf("in P_gate\n");
    int lo_off = gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[lo_off].real = q_1r*cos(real[0]) - q_1i*sin(real[0]);
            q_rd[lo_off].imag = q_1r*sin(real[0]) + q_1i*cos(real[0]);
            lo_off++;
        }
        lo_off += gate_move.half_targ;
    }
}

/*
Conjugated Phase gate

expressed in row-maj as:
  [[1,   0]
   [0,  exp(-theta)]]
where theta stored at real[0]
*/
void Pc_gate (Type *q_rd) {
    // printf("in Pc_gate\n");
    int lo_off = gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[lo_off].real = q_1r*cos(real[0]) + q_1i*sin(real[0]);
            q_rd[lo_off].imag = -q_1r*sin(real[0]) + q_1i*cos(real[0]);
            lo_off++;
        }
        lo_off += gate_move.half_targ;
    }
}

/*
Unitary gate
pre:
在circuit.c會先設定好real, imag

1. 給使用者自訂1 qubit gate
2. 在利用scheduler合併之後可以使用這個

expressed in row-maj as:
  [[real[0]+imag[0]*i, real[1]+imag[1]*i]
   [real[2]+imag[2]*i, real[3]+imag[3]*i]]
*/
void U_gate (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[up_off].real = real[0]*q_0r + real[1]*q_1r - imag[0]*q_0i - imag[1]*q_1i;
            q_rd[up_off].imag = real[0]*q_0i + real[1]*q_1i + imag[0]*q_0r + imag[1]*q_1r;
            q_rd[lo_off].real = real[2]*q_0r + real[3]*q_1r - imag[2]*q_0i - imag[3]*q_1i;
            q_rd[lo_off].imag = real[2]*q_0i + real[3]*q_1i + imag[2]*q_0r + imag[3]*q_1r;
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}

/*
Conjugated Unitary gate
pre:
在circuit.c會先設定好real, imag

1. 給使用者自訂1 qubit gate
2. 在利用scheduler合併之後可以使用這個

expressed in row-maj as:
  [[real[0]-imag[0]*i, real[1]-imag[1]*i]
   [real[2]-imag[2]*i, real[3]-imag[3]*i]]
*/
void Uc_gate (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            q_rd[up_off].real =   real[0]*q_0r + real[1]*q_1r + imag[0]*q_0i + imag[1]*q_1i;
            q_rd[up_off].imag = -(real[0]*q_0i + real[1]*q_1i + imag[0]*q_0r + imag[1]*q_1r);
            q_rd[lo_off].real =   real[2]*q_0r + real[3]*q_1r + imag[2]*q_0i + imag[3]*q_1i;
            q_rd[lo_off].imag = -(real[2]*q_0i + real[3]*q_1i + imag[2]*q_0r + imag[3]*q_1r);
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}

/*===================================================================
gate level (Type II)
1) 2nd type of 1 qubit gate for control-target format
2) General 2 qubit gate

prepare:
gate_move.large
gate_move.small
gate_move.half_ctrl
gate_move.half_targ
gate_size

void *_gate(Type *rd)
input: 指向一個存放gate_size這麼多state的buffer。
功能: 對buffer內的state操作，使他們變成作用完gate之後的樣子
===================================================================*/

/*===================================================================
gate level (Type II)
2nd type of 1 qubit gate for control-target format
===================================================================*/
void X_gate2 (Type *q_rd){
    int up_off = gate_move.half_ctrl;
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;
    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[up_off].real = q_1r;
                q_rd[up_off].imag = q_1i;
                q_rd[lo_off].real = q_0r;
                q_rd[lo_off].imag = q_0i;
                up_off++;
                lo_off++;
            }
            up_off += (gate_move.small>>1);
            lo_off += (gate_move.small>>1);
        }
        up_off += (gate_move.large>>1);
        lo_off += (gate_move.large>>1);
    }
}

void Y_gate2 (Type *q_rd) {
    int up_off = gate_move.half_ctrl;
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[up_off].real = q_1i;
                q_rd[up_off].imag = -q_1r;
                q_rd[lo_off].real = -q_0i;
                q_rd[lo_off].imag = q_0r;
                up_off++;
                lo_off++;
            }
            up_off += (gate_move.small>>1);
            lo_off += (gate_move.small>>1);
        }
        up_off += (gate_move.large>>1);
        lo_off += (gate_move.large>>1);
    }
}

void Yc_gate2 (Type *q_rd) {
    int up_off = gate_move.half_ctrl;
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[up_off].real = -q_1i;
                q_rd[up_off].imag = q_1r;
                q_rd[lo_off].real = q_0i;
                q_rd[lo_off].imag = -q_0r;
                up_off++;
                lo_off++;
            }
            up_off += (gate_move.small>>1);
            lo_off += (gate_move.small>>1);
        }
        up_off += (gate_move.large>>1);
        lo_off += (gate_move.large>>1);
    }
}

void Z_gate2 (Type *q_rd) {
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[lo_off].real = -q_1r;
                q_rd[lo_off].imag = -q_1i;
                lo_off++;
            }
            lo_off += (gate_move.small>>1);
        }
        lo_off += (gate_move.large>>1);
    }
}

void P_gate2 (Type *q_rd) {
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[lo_off].real = q_1r*cos(real[0]) - q_1i*sin(real[0]);
                q_rd[lo_off].imag = q_1r*sin(real[0]) + q_1i*cos(real[0]);
                lo_off++;
            }
            lo_off += (gate_move.small>>1);
        }
        lo_off += (gate_move.large>>1);
    }
}

void Pc_gate2 (Type *q_rd) {
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[lo_off].real = q_1r*cos(real[0]) + q_1i*sin(real[0]);
                q_rd[lo_off].imag = -q_1r*sin(real[0]) + q_1i*cos(real[0]);
                lo_off++;
            }
            lo_off += (gate_move.small>>1);
        }
        lo_off += (gate_move.large>>1);
    }
}

void U_gate2 (Type *q_rd) {
    int up_off = gate_move.half_ctrl;
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[up_off].real = real[0]*q_0r + real[1]*q_1r - imag[0]*q_0i - imag[1]*q_1i;
                q_rd[up_off].imag = real[0]*q_0i + real[1]*q_1i + imag[0]*q_0r + imag[1]*q_1r;
                q_rd[lo_off].real = real[2]*q_0r + real[3]*q_1r - imag[2]*q_0i - imag[3]*q_1i;
                q_rd[lo_off].imag = real[2]*q_0i + real[3]*q_1i + imag[2]*q_0r + imag[3]*q_1r;
                up_off++;
                lo_off++;
            }
            up_off += (gate_move.small>>1);
            lo_off += (gate_move.small>>1);
        }
        up_off += (gate_move.large>>1);
        lo_off += (gate_move.large>>1);
    }
}

void Uc_gate2 (Type *q_rd) {
    int up_off = gate_move.half_ctrl;
    int lo_off = gate_move.half_ctrl + gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.large>>1; j += gate_move.small ){
            for (int k = 0; k < gate_move.small>>1; k++){
                q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
                q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
                q_rd[up_off].real = real[0]*q_0r + real[1]*q_1r + imag[0]*q_0i + imag[1]*q_1i;
                q_rd[up_off].imag = real[0]*q_0i + real[1]*q_1i - imag[0]*q_0r - imag[1]*q_1r;
                q_rd[lo_off].real = real[2]*q_0r + real[3]*q_1r + imag[2]*q_0i + imag[3]*q_1i;
                q_rd[lo_off].imag = real[2]*q_0i + real[3]*q_1i - imag[2]*q_0r - imag[3]*q_1r;
                up_off++;
                lo_off++;
            }
            up_off += (gate_move.small>>1);
            lo_off += (gate_move.small>>1);
        }
        up_off += (gate_move.large>>1);
        lo_off += (gate_move.large>>1);
    }
}

/*===================================================================
gate level (Type II)
General 2 qubit gate
===================================================================*/
void U2_gate (Type *q_rd) {
    int q_00_off = 0;
    int q_01_off = gate_move.half_small;
    int q_10_off = gate_move.half_large;
    int q_11_off = gate_move.half_large + gate_move.half_small;

    Type_t q_00r;   Type_t q_00i;
    Type_t q_01r;   Type_t q_01i;
    Type_t q_10r;   Type_t q_10i;
    Type_t q_11r;   Type_t q_11i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.half_large; j += gate_move.small){
            for (int k = 0; k < gate_move.half_small; k++){
                q_00r = q_rd[q_00_off].real;    q_00i = q_rd[q_00_off].imag;
                q_01r = q_rd[q_01_off].real;    q_01i = q_rd[q_01_off].imag;
                q_10r = q_rd[q_10_off].real;    q_10i = q_rd[q_10_off].imag;
                q_11r = q_rd[q_11_off].real;    q_11i = q_rd[q_11_off].imag;

                q_rd[q_00_off].real  = real[ 0]*q_00r + real[ 1]*q_01r + real[ 2]*q_10r + real[ 3]*q_11r;
                q_rd[q_00_off].real -= imag[ 0]*q_00i + imag[ 1]*q_01i + imag[ 2]*q_10i + imag[ 3]*q_11i;
                q_rd[q_01_off].real  = real[ 4]*q_00r + real[ 5]*q_01r + real[ 6]*q_10r + real[ 7]*q_11r;
                q_rd[q_01_off].real -= imag[ 4]*q_00i + imag[ 5]*q_01i + imag[ 6]*q_10i + imag[ 7]*q_11i;
                q_rd[q_10_off].real  = real[ 8]*q_00r + real[ 9]*q_01r + real[10]*q_10r + real[11]*q_11r;
                q_rd[q_10_off].real -= imag[ 8]*q_00i + imag[ 9]*q_01i + imag[10]*q_10i + imag[11]*q_11i;
                q_rd[q_11_off].real  = real[12]*q_00r + real[13]*q_01r + real[14]*q_10r + real[15]*q_11r;
                q_rd[q_11_off].real -= imag[12]*q_00i + imag[13]*q_01i + imag[14]*q_10i + imag[15]*q_11i;

                q_rd[q_00_off].imag  = real[ 0]*q_00i + real[ 1]*q_01i + real[ 2]*q_10i + real[ 3]*q_11i;
                q_rd[q_00_off].imag += imag[ 0]*q_00r + imag[ 1]*q_01r + imag[ 2]*q_10r + imag[ 3]*q_11r;
                q_rd[q_01_off].imag  = real[ 4]*q_00i + real[ 5]*q_01i + real[ 6]*q_10i + real[ 7]*q_11i;
                q_rd[q_01_off].imag += imag[ 4]*q_00r + imag[ 5]*q_01r + imag[ 6]*q_10r + imag[ 7]*q_11r;
                q_rd[q_10_off].imag  = real[ 8]*q_00i + real[ 9]*q_01i + real[10]*q_10i + real[11]*q_11i;
                q_rd[q_10_off].imag += imag[ 8]*q_00r + imag[ 9]*q_01r + imag[10]*q_10r + imag[11]*q_11r;
                q_rd[q_11_off].imag  = real[12]*q_00i + real[13]*q_01i + real[14]*q_10i + real[15]*q_11i;
                q_rd[q_11_off].imag += imag[12]*q_00r + imag[13]*q_01r + imag[14]*q_10r + imag[15]*q_11r;

                q_00_off++;     q_01_off++;
                q_10_off++;     q_11_off++;
            }
            q_00_off += gate_move.half_small;       q_01_off += gate_move.half_small;
            q_10_off += gate_move.half_small;       q_11_off += gate_move.half_small;
        }
        q_00_off += gate_move.half_large;       q_01_off += gate_move.half_large;
        q_10_off += gate_move.half_large;       q_11_off += gate_move.half_large;
    }
}

// General Conjugated 2-qubit Unitary gate.
void U2c_gate (Type *q_rd) {
    int q_00_off = 0;
    int q_01_off = gate_move.half_small;
    int q_10_off = gate_move.half_large;
    int q_11_off = gate_move.half_large + gate_move.half_small;

    Type_t q_00r;   Type_t q_00i;
    Type_t q_01r;   Type_t q_01i;
    Type_t q_10r;   Type_t q_10i;
    Type_t q_11r;   Type_t q_11i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.half_large; j += gate_move.small){
            for (int k = 0; k < gate_move.half_small; k++){
                q_00r = q_rd[q_00_off].real;    q_00i = q_rd[q_00_off].imag;
                q_01r = q_rd[q_01_off].real;    q_01i = q_rd[q_01_off].imag;
                q_10r = q_rd[q_10_off].real;    q_10i = q_rd[q_10_off].imag;
                q_11r = q_rd[q_11_off].real;    q_11i = q_rd[q_11_off].imag;

                q_rd[q_00_off].real  = real[ 0]*q_00r + real[ 1]*q_01r + real[ 2]*q_10r + real[ 3]*q_11r;
                q_rd[q_00_off].real += imag[ 0]*q_00i + imag[ 1]*q_01i + imag[ 2]*q_10i + imag[ 3]*q_11i;
                q_rd[q_01_off].real  = real[ 4]*q_00r + real[ 5]*q_01r + real[ 6]*q_10r + real[ 7]*q_11r;
                q_rd[q_01_off].real += imag[ 4]*q_00i + imag[ 5]*q_01i + imag[ 6]*q_10i + imag[ 7]*q_11i;
                q_rd[q_10_off].real  = real[ 8]*q_00r + real[ 9]*q_01r + real[10]*q_10r + real[11]*q_11r;
                q_rd[q_10_off].real += imag[ 8]*q_00i + imag[ 9]*q_01i + imag[10]*q_10i + imag[11]*q_11i;
                q_rd[q_11_off].real  = real[12]*q_00r + real[13]*q_01r + real[14]*q_10r + real[15]*q_11r;
                q_rd[q_11_off].real += imag[12]*q_00i + imag[13]*q_01i + imag[14]*q_10i + imag[15]*q_11i;

                q_rd[q_00_off].imag  = real[ 0]*q_00i + real[ 1]*q_01i + real[ 2]*q_10i + real[ 3]*q_11i;
                q_rd[q_00_off].imag -= imag[ 0]*q_00r + imag[ 1]*q_01r + imag[ 2]*q_10r + imag[ 3]*q_11r;
                q_rd[q_01_off].imag  = real[ 4]*q_00i + real[ 5]*q_01i + real[ 6]*q_10i + real[ 7]*q_11i;
                q_rd[q_01_off].imag -= imag[ 4]*q_00r + imag[ 5]*q_01r + imag[ 6]*q_10r + imag[ 7]*q_11r;
                q_rd[q_10_off].imag  = real[ 8]*q_00i + real[ 9]*q_01i + real[10]*q_10i + real[11]*q_11i;
                q_rd[q_10_off].imag -= imag[ 8]*q_00r + imag[ 9]*q_01r + imag[10]*q_10r + imag[11]*q_11r;
                q_rd[q_11_off].imag  = real[12]*q_00i + real[13]*q_01i + real[14]*q_10i + real[15]*q_11i;
                q_rd[q_11_off].imag -= imag[12]*q_00r + imag[13]*q_01r + imag[14]*q_10r + imag[15]*q_11r;

                q_00_off++;     q_01_off++;
                q_10_off++;     q_11_off++;
            }
            q_00_off += gate_move.half_small;       q_01_off += gate_move.half_small;
            q_10_off += gate_move.half_small;       q_11_off += gate_move.half_small;
        }
        q_00_off += gate_move.half_large;       q_01_off += gate_move.half_large;
        q_10_off += gate_move.half_large;       q_11_off += gate_move.half_large;
    }
}

// SWAP gate.
void SWAP_gate (Type *q_rd) {
    int q_01_off = gate_move.half_small;
    int q_10_off = gate_move.half_large;
    Type_t q_01r;    Type_t q_01i;
    Type_t q_10r;    Type_t q_10i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.half_large; j += gate_move.small){
            for (int k = 0; k < gate_move.half_small; k++){
                q_01r = q_rd[q_01_off].real;   q_01i = q_rd[q_01_off].imag;
                q_10r = q_rd[q_10_off].real;   q_10i = q_rd[q_10_off].imag;

                q_rd[q_01_off].real  = q_10r;
                q_rd[q_01_off].imag  = q_10i;

                q_rd[q_10_off].real  = q_01r;
                q_rd[q_10_off].imag  = q_01i;

                q_01_off++;
                q_10_off++;
            }
            q_01_off += gate_move.half_small;
            q_10_off += gate_move.half_small;
        }
        q_01_off += gate_move.half_large;
        q_10_off += gate_move.half_large;
    }
}

/*===================================================================
gate level (Type III)
General 3 qubit gate

prepare:
gate_move.large
gate_move.small
gate_move.half_ctrl
gate_move.half_targ
gate_size

void *_gate(Type *rd)
input: 指向一個存放gate_size這麼多state的buffer。
功能: 對buffer內的state操作，使他們變成作用完gate之後的樣子
===================================================================*/

void U3_gate (Type *q_rd){
    int q_000_off = 0;
    int q_001_off = gate_move.half_small;
    int q_010_off = gate_move.half_middle;
    int q_011_off = gate_move.half_middle + gate_move.half_small;
    int q_100_off = gate_move.half_large;
    int q_101_off = gate_move.half_large + gate_move.half_small;
    int q_110_off = gate_move.half_large + gate_move.half_middle;
    int q_111_off = gate_move.half_large + gate_move.half_middle + gate_move.half_small;
    Type_t q_000r;  Type_t q_000i;
    Type_t q_001r;  Type_t q_001i;
    Type_t q_010r;  Type_t q_010i;
    Type_t q_011r;  Type_t q_011i;
    Type_t q_100r;  Type_t q_100i;
    Type_t q_101r;  Type_t q_101i;
    Type_t q_110r;  Type_t q_110i;
    Type_t q_111r;  Type_t q_111i;

    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.half_large; j += gate_move.middle){
            for (int k = 0; k < gate_move.half_middle; k += gate_move.small){
                for (int l = 0; l < gate_move.half_small; l++){
                    q_000r = q_rd[q_000_off].real;  q_000i = q_rd[q_000_off].imag;
                    q_001r = q_rd[q_001_off].real;  q_001i = q_rd[q_001_off].imag;
                    q_010r = q_rd[q_010_off].real;  q_010i = q_rd[q_010_off].imag;
                    q_011r = q_rd[q_011_off].real;  q_011i = q_rd[q_011_off].imag;
                    q_100r = q_rd[q_100_off].real;  q_100i = q_rd[q_100_off].imag;
                    q_101r = q_rd[q_101_off].real;  q_101i = q_rd[q_101_off].imag;
                    q_110r = q_rd[q_110_off].real;  q_110i = q_rd[q_110_off].imag;
                    q_111r = q_rd[q_111_off].real;  q_111i = q_rd[q_111_off].imag;

                    q_rd[q_000_off].real  = real[ 0]*q_000r + real[ 1]*q_001r + real[ 2]*q_010r + real[ 3]*q_011r;
                    q_rd[q_000_off].real += real[ 4]*q_100r + real[ 5]*q_101r + real[ 6]*q_110r + real[ 7]*q_111r;
                    q_rd[q_000_off].real -= imag[ 0]*q_000i + imag[ 1]*q_001i + imag[ 2]*q_010i + imag[ 3]*q_011i;
                    q_rd[q_000_off].real -= imag[ 4]*q_100i + imag[ 5]*q_101i + imag[ 6]*q_110i + imag[ 7]*q_111i;

                    q_rd[q_001_off].real  = real[ 8]*q_000r + real[ 9]*q_001r + real[10]*q_010r + real[11]*q_011r;
                    q_rd[q_001_off].real += real[12]*q_100r + real[13]*q_101r + real[14]*q_110r + real[15]*q_111r;
                    q_rd[q_001_off].real -= imag[ 8]*q_000i + imag[ 9]*q_001i + imag[10]*q_010i + imag[11]*q_011i;
                    q_rd[q_001_off].real -= imag[12]*q_100i + imag[13]*q_101i + imag[14]*q_110i + imag[15]*q_111i;

                    q_rd[q_010_off].real  = real[16]*q_000r + real[17]*q_001r + real[18]*q_010r + real[19]*q_011r;
                    q_rd[q_010_off].real += real[20]*q_100r + real[21]*q_101r + real[22]*q_110r + real[23]*q_111r;
                    q_rd[q_010_off].real -= imag[16]*q_000i + imag[17]*q_001i + imag[18]*q_010i + imag[19]*q_011i;
                    q_rd[q_010_off].real -= imag[20]*q_100i + imag[21]*q_101i + imag[22]*q_110i + imag[23]*q_111i;

                    q_rd[q_011_off].real  = real[24]*q_000r + real[25]*q_001r + real[26]*q_010r + real[27]*q_011r;
                    q_rd[q_011_off].real += real[28]*q_100r + real[29]*q_101r + real[30]*q_110r + real[31]*q_111r;
                    q_rd[q_011_off].real -= imag[24]*q_000i + imag[25]*q_001i + imag[26]*q_010i + imag[27]*q_011i;
                    q_rd[q_011_off].real -= imag[28]*q_100i + imag[29]*q_101i + imag[30]*q_110i + imag[31]*q_111i;

                    q_rd[q_100_off].real  = real[32]*q_000r + real[33]*q_001r + real[34]*q_010r + real[35]*q_011r;
                    q_rd[q_100_off].real += real[36]*q_100r + real[37]*q_101r + real[38]*q_110r + real[39]*q_111r;
                    q_rd[q_100_off].real -= imag[32]*q_000i + imag[33]*q_001i + imag[34]*q_010i + imag[35]*q_011i;
                    q_rd[q_100_off].real -= imag[36]*q_100i + imag[37]*q_101i + imag[38]*q_110i + imag[39]*q_111i;

                    q_rd[q_101_off].real  = real[40]*q_000r + real[41]*q_001r + real[42]*q_010r + real[43]*q_011r;
                    q_rd[q_101_off].real += real[44]*q_100r + real[45]*q_101r + real[46]*q_110r + real[47]*q_111r;
                    q_rd[q_101_off].real -= imag[40]*q_000i + imag[41]*q_001i + imag[42]*q_010i + imag[43]*q_011i;
                    q_rd[q_101_off].real -= imag[44]*q_100i + imag[45]*q_101i + imag[46]*q_110i + imag[47]*q_111i;

                    q_rd[q_110_off].real  = real[48]*q_000r + real[49]*q_001r + real[50]*q_010r + real[51]*q_011r;
                    q_rd[q_110_off].real += real[52]*q_100r + real[53]*q_101r + real[54]*q_110r + real[55]*q_111r;
                    q_rd[q_110_off].real -= imag[48]*q_000i + imag[49]*q_001i + imag[50]*q_010i + imag[51]*q_011i;
                    q_rd[q_110_off].real -= imag[52]*q_100i + imag[53]*q_101i + imag[54]*q_110i + imag[55]*q_111i;

                    q_rd[q_111_off].real  = real[56]*q_000r + real[57]*q_001r + real[58]*q_010r + real[59]*q_011r;
                    q_rd[q_111_off].real += real[60]*q_100r + real[61]*q_101r + real[62]*q_110r + real[63]*q_111r;
                    q_rd[q_111_off].real -= imag[56]*q_000i + imag[57]*q_001i + imag[58]*q_010i + imag[59]*q_011i;
                    q_rd[q_111_off].real -= imag[60]*q_100i + imag[61]*q_101i + imag[62]*q_110i + imag[63]*q_111i;


                    q_rd[q_000_off].imag  = real[ 0]*q_000i + real[ 1]*q_001i + real[ 2]*q_010i + real[ 3]*q_011i;
                    q_rd[q_000_off].imag += real[ 4]*q_100i + real[ 5]*q_101i + real[ 6]*q_110i + real[ 7]*q_111i;
                    q_rd[q_000_off].imag += imag[ 0]*q_000r + imag[ 1]*q_001r + imag[ 2]*q_010r + imag[ 3]*q_011r;
                    q_rd[q_000_off].imag += imag[ 4]*q_100r + imag[ 5]*q_101r + imag[ 6]*q_110r + imag[ 7]*q_111r;

                    q_rd[q_001_off].imag  = real[ 8]*q_000i + real[ 9]*q_001i + real[10]*q_010i + real[11]*q_011i;
                    q_rd[q_001_off].imag += real[12]*q_100i + real[13]*q_101i + real[14]*q_110i + real[15]*q_111i;
                    q_rd[q_001_off].imag += imag[ 8]*q_000r + imag[ 9]*q_001r + imag[10]*q_010r + imag[11]*q_011r;
                    q_rd[q_001_off].imag += imag[12]*q_100r + imag[13]*q_101r + imag[14]*q_110r + imag[15]*q_111r;

                    q_rd[q_010_off].imag  = real[16]*q_000i + real[17]*q_001i + real[18]*q_010i + real[19]*q_011i;
                    q_rd[q_010_off].imag += real[20]*q_100i + real[21]*q_101i + real[22]*q_110i + real[23]*q_111i;
                    q_rd[q_010_off].imag += imag[16]*q_000r + imag[17]*q_001r + imag[18]*q_010r + imag[19]*q_011r;
                    q_rd[q_010_off].imag += imag[20]*q_100r + imag[21]*q_101r + imag[22]*q_110r + imag[23]*q_111r;

                    q_rd[q_011_off].imag  = real[24]*q_000i + real[25]*q_001i + real[26]*q_010i + real[27]*q_011i;
                    q_rd[q_011_off].imag += real[28]*q_100i + real[29]*q_101i + real[30]*q_110i + real[31]*q_111i;
                    q_rd[q_011_off].imag += imag[24]*q_000r + imag[25]*q_001r + imag[26]*q_010r + imag[27]*q_011r;
                    q_rd[q_011_off].imag += imag[28]*q_100r + imag[29]*q_101r + imag[30]*q_110r + imag[31]*q_111r;

                    q_rd[q_100_off].imag  = real[32]*q_000i + real[33]*q_001i + real[34]*q_010i + real[35]*q_011i;
                    q_rd[q_100_off].imag += real[36]*q_100i + real[37]*q_101i + real[38]*q_110i + real[39]*q_111i;
                    q_rd[q_100_off].imag += imag[32]*q_000r + imag[33]*q_001r + imag[34]*q_010r + imag[35]*q_011r;
                    q_rd[q_100_off].imag += imag[36]*q_100r + imag[37]*q_101r + imag[38]*q_110r + imag[39]*q_111r;

                    q_rd[q_101_off].imag  = real[40]*q_000i + real[41]*q_001i + real[42]*q_010i + real[43]*q_011i;
                    q_rd[q_101_off].imag += real[44]*q_100i + real[45]*q_101i + real[46]*q_110i + real[47]*q_111i;
                    q_rd[q_101_off].imag += imag[40]*q_000r + imag[41]*q_001r + imag[42]*q_010r + imag[43]*q_011r;
                    q_rd[q_101_off].imag += imag[44]*q_100r + imag[45]*q_101r + imag[46]*q_110r + imag[47]*q_111r;

                    q_rd[q_110_off].imag  = real[48]*q_000i + real[49]*q_001i + real[50]*q_010i + real[51]*q_011i;
                    q_rd[q_110_off].imag += real[52]*q_100i + real[53]*q_101i + real[54]*q_110i + real[55]*q_111i;
                    q_rd[q_110_off].imag += imag[48]*q_000r + imag[49]*q_001r + imag[50]*q_010r + imag[51]*q_011r;
                    q_rd[q_110_off].imag += imag[52]*q_100r + imag[53]*q_101r + imag[54]*q_110r + imag[55]*q_111r;

                    q_rd[q_111_off].imag  = real[56]*q_000i + real[57]*q_001i + real[58]*q_010i + real[59]*q_011i;
                    q_rd[q_111_off].imag += real[60]*q_100i + real[61]*q_101i + real[62]*q_110i + real[63]*q_111i;
                    q_rd[q_111_off].imag += imag[56]*q_000r + imag[57]*q_001r + imag[58]*q_010r + imag[59]*q_011r;
                    q_rd[q_111_off].imag += imag[60]*q_100r + imag[61]*q_101r + imag[62]*q_110r + imag[63]*q_111r;

                    q_000_off++;    q_001_off++;
                    q_010_off++;    q_011_off++;
                    q_100_off++;    q_101_off++;
                    q_110_off++;    q_111_off++;
                }
                q_000_off += gate_move.half_small;      q_001_off += gate_move.half_small;
                q_010_off += gate_move.half_small;      q_011_off += gate_move.half_small;
                q_100_off += gate_move.half_small;      q_101_off += gate_move.half_small;
                q_110_off += gate_move.half_small;      q_111_off += gate_move.half_small;
            }
            q_000_off += gate_move.half_middle;     q_001_off += gate_move.half_middle;
            q_010_off += gate_move.half_middle;     q_011_off += gate_move.half_middle;
            q_100_off += gate_move.half_middle;     q_101_off += gate_move.half_middle;
            q_110_off += gate_move.half_middle;     q_111_off += gate_move.half_middle;
        }
        q_000_off += gate_move.half_large;      q_001_off += gate_move.half_large;
        q_010_off += gate_move.half_large;      q_011_off += gate_move.half_large;
        q_100_off += gate_move.half_large;      q_101_off += gate_move.half_large;
        q_110_off += gate_move.half_large;      q_111_off += gate_move.half_large;
    }
};

void U3c_gate (Type *q_rd){
    int q_000_off = 0;
    int q_001_off = gate_move.half_small;
    int q_010_off = gate_move.half_middle;
    int q_011_off = gate_move.half_middle + gate_move.half_small;
    int q_100_off = gate_move.half_large;
    int q_101_off = gate_move.half_large + gate_move.half_small;
    int q_110_off = gate_move.half_large + gate_move.half_middle;
    int q_111_off = gate_move.half_large + gate_move.half_middle + gate_move.half_small;
    Type_t q_000r;  Type_t q_000i;
    Type_t q_001r;  Type_t q_001i;
    Type_t q_010r;  Type_t q_010i;
    Type_t q_011r;  Type_t q_011i;
    Type_t q_100r;  Type_t q_100i;
    Type_t q_101r;  Type_t q_101i;
    Type_t q_110r;  Type_t q_110i;
    Type_t q_111r;  Type_t q_111i;
    
    for (int i = 0; i < gate_size; i += gate_move.large) {
        for (int j = 0; j < gate_move.half_large; j += gate_move.middle){
            for (int k = 0; k < gate_move.half_middle; k += gate_move.small){
                for (int l = 0; l < gate_move.half_small; l++){
                    q_000r = q_rd[q_000_off].real;  q_000i = q_rd[q_000_off].imag;
                    q_001r = q_rd[q_001_off].real;  q_001i = q_rd[q_001_off].imag;
                    q_010r = q_rd[q_010_off].real;  q_010i = q_rd[q_010_off].imag;
                    q_011r = q_rd[q_011_off].real;  q_011i = q_rd[q_011_off].imag;
                    q_100r = q_rd[q_100_off].real;  q_100i = q_rd[q_100_off].imag;
                    q_101r = q_rd[q_101_off].real;  q_101i = q_rd[q_101_off].imag;
                    q_110r = q_rd[q_110_off].real;  q_110i = q_rd[q_110_off].imag;
                    q_111r = q_rd[q_111_off].real;  q_111i = q_rd[q_111_off].imag;

                    q_rd[q_000_off].real  = real[ 0]*q_000r + real[ 1]*q_001r + real[ 2]*q_010r + real[ 3]*q_011r;
                    q_rd[q_000_off].real += real[ 4]*q_100r + real[ 5]*q_101r + real[ 6]*q_110r + real[ 7]*q_111r;
                    q_rd[q_000_off].real += imag[ 0]*q_000i + imag[ 1]*q_001i + imag[ 2]*q_010i + imag[ 3]*q_011i;
                    q_rd[q_000_off].real += imag[ 4]*q_100i + imag[ 5]*q_101i + imag[ 6]*q_110i + imag[ 7]*q_111i;

                    q_rd[q_001_off].real  = real[ 8]*q_000r + real[ 9]*q_001r + real[10]*q_010r + real[11]*q_011r;
                    q_rd[q_001_off].real += real[12]*q_100r + real[13]*q_101r + real[14]*q_110r + real[15]*q_111r;
                    q_rd[q_001_off].real += imag[ 8]*q_000i + imag[ 9]*q_001i + imag[10]*q_010i + imag[11]*q_011i;
                    q_rd[q_001_off].real += imag[12]*q_100i + imag[13]*q_101i + imag[14]*q_110i + imag[15]*q_111i;

                    q_rd[q_010_off].real  = real[16]*q_000r + real[17]*q_001r + real[18]*q_010r + real[19]*q_011r;
                    q_rd[q_010_off].real += real[20]*q_100r + real[21]*q_101r + real[22]*q_110r + real[23]*q_111r;
                    q_rd[q_010_off].real += imag[16]*q_000i + imag[17]*q_001i + imag[18]*q_010i + imag[19]*q_011i;
                    q_rd[q_010_off].real += imag[20]*q_100i + imag[21]*q_101i + imag[22]*q_110i + imag[23]*q_111i;

                    q_rd[q_011_off].real  = real[24]*q_000r + real[25]*q_001r + real[26]*q_010r + real[27]*q_011r;
                    q_rd[q_011_off].real += real[28]*q_100r + real[29]*q_101r + real[30]*q_110r + real[31]*q_111r;
                    q_rd[q_011_off].real += imag[24]*q_000i + imag[25]*q_001i + imag[26]*q_010i + imag[27]*q_011i;
                    q_rd[q_011_off].real += imag[28]*q_100i + imag[29]*q_101i + imag[30]*q_110i + imag[31]*q_111i;

                    q_rd[q_100_off].real  = real[32]*q_000r + real[33]*q_001r + real[34]*q_010r + real[35]*q_011r;
                    q_rd[q_100_off].real += real[36]*q_100r + real[37]*q_101r + real[38]*q_110r + real[39]*q_111r;
                    q_rd[q_100_off].real += imag[32]*q_000i + imag[33]*q_001i + imag[34]*q_010i + imag[35]*q_011i;
                    q_rd[q_100_off].real += imag[36]*q_100i + imag[37]*q_101i + imag[38]*q_110i + imag[39]*q_111i;

                    q_rd[q_101_off].real  = real[40]*q_000r + real[41]*q_001r + real[42]*q_010r + real[43]*q_011r;
                    q_rd[q_101_off].real += real[44]*q_100r + real[45]*q_101r + real[46]*q_110r + real[47]*q_111r;
                    q_rd[q_101_off].real += imag[40]*q_000i + imag[41]*q_001i + imag[42]*q_010i + imag[43]*q_011i;
                    q_rd[q_101_off].real += imag[44]*q_100i + imag[45]*q_101i + imag[46]*q_110i + imag[47]*q_111i;

                    q_rd[q_110_off].real  = real[48]*q_000r + real[49]*q_001r + real[50]*q_010r + real[51]*q_011r;
                    q_rd[q_110_off].real += real[52]*q_100r + real[53]*q_101r + real[54]*q_110r + real[55]*q_111r;
                    q_rd[q_110_off].real += imag[48]*q_000i + imag[49]*q_001i + imag[50]*q_010i + imag[51]*q_011i;
                    q_rd[q_110_off].real += imag[52]*q_100i + imag[53]*q_101i + imag[54]*q_110i + imag[55]*q_111i;

                    q_rd[q_111_off].real  = real[56]*q_000r + real[57]*q_001r + real[58]*q_010r + real[59]*q_011r;
                    q_rd[q_111_off].real += real[60]*q_100r + real[61]*q_101r + real[62]*q_110r + real[63]*q_111r;
                    q_rd[q_111_off].real += imag[56]*q_000i + imag[57]*q_001i + imag[58]*q_010i + imag[59]*q_011i;
                    q_rd[q_111_off].real += imag[60]*q_100i + imag[61]*q_101i + imag[62]*q_110i + imag[63]*q_111i;


                    q_rd[q_000_off].imag  = real[ 0]*q_000i + real[ 1]*q_001i + real[ 2]*q_010i + real[ 3]*q_011i;
                    q_rd[q_000_off].imag += real[ 4]*q_100i + real[ 5]*q_101i + real[ 6]*q_110i + real[ 7]*q_111i;
                    q_rd[q_000_off].imag -= imag[ 0]*q_000r + imag[ 1]*q_001r + imag[ 2]*q_010r + imag[ 3]*q_011r;
                    q_rd[q_000_off].imag -= imag[ 4]*q_100r + imag[ 5]*q_101r + imag[ 6]*q_110r + imag[ 7]*q_111r;

                    q_rd[q_001_off].imag  = real[ 8]*q_000i + real[ 9]*q_001i + real[10]*q_010i + real[11]*q_011i;
                    q_rd[q_001_off].imag += real[12]*q_100i + real[13]*q_101i + real[14]*q_110i + real[15]*q_111i;
                    q_rd[q_001_off].imag -= imag[ 8]*q_000r + imag[ 9]*q_001r + imag[10]*q_010r + imag[11]*q_011r;
                    q_rd[q_001_off].imag -= imag[12]*q_100r + imag[13]*q_101r + imag[14]*q_110r + imag[15]*q_111r;

                    q_rd[q_010_off].imag  = real[16]*q_000i + real[17]*q_001i + real[18]*q_010i + real[19]*q_011i;
                    q_rd[q_010_off].imag += real[20]*q_100i + real[21]*q_101i + real[22]*q_110i + real[23]*q_111i;
                    q_rd[q_010_off].imag -= imag[16]*q_000r + imag[17]*q_001r + imag[18]*q_010r + imag[19]*q_011r;
                    q_rd[q_010_off].imag -= imag[20]*q_100r + imag[21]*q_101r + imag[22]*q_110r + imag[23]*q_111r;

                    q_rd[q_011_off].imag  = real[24]*q_000i + real[25]*q_001i + real[26]*q_010i + real[27]*q_011i;
                    q_rd[q_011_off].imag += real[28]*q_100i + real[29]*q_101i + real[30]*q_110i + real[31]*q_111i;
                    q_rd[q_011_off].imag -= imag[24]*q_000r + imag[25]*q_001r + imag[26]*q_010r + imag[27]*q_011r;
                    q_rd[q_011_off].imag -= imag[28]*q_100r + imag[29]*q_101r + imag[30]*q_110r + imag[31]*q_111r;

                    q_rd[q_100_off].imag  = real[32]*q_000i + real[33]*q_001i + real[34]*q_010i + real[35]*q_011i;
                    q_rd[q_100_off].imag += real[36]*q_100i + real[37]*q_101i + real[38]*q_110i + real[39]*q_111i;
                    q_rd[q_100_off].imag -= imag[32]*q_000r + imag[33]*q_001r + imag[34]*q_010r + imag[35]*q_011r;
                    q_rd[q_100_off].imag -= imag[36]*q_100r + imag[37]*q_101r + imag[38]*q_110r + imag[39]*q_111r;

                    q_rd[q_101_off].imag  = real[40]*q_000i + real[41]*q_001i + real[42]*q_010i + real[43]*q_011i;
                    q_rd[q_101_off].imag += real[44]*q_100i + real[45]*q_101i + real[46]*q_110i + real[47]*q_111i;
                    q_rd[q_101_off].imag -= imag[40]*q_000r + imag[41]*q_001r + imag[42]*q_010r + imag[43]*q_011r;
                    q_rd[q_101_off].imag -= imag[44]*q_100r + imag[45]*q_101r + imag[46]*q_110r + imag[47]*q_111r;

                    q_rd[q_110_off].imag  = real[48]*q_000i + real[49]*q_001i + real[50]*q_010i + real[51]*q_011i;
                    q_rd[q_110_off].imag += real[52]*q_100i + real[53]*q_101i + real[54]*q_110i + real[55]*q_111i;
                    q_rd[q_110_off].imag -= imag[48]*q_000r + imag[49]*q_001r + imag[50]*q_010r + imag[51]*q_011r;
                    q_rd[q_110_off].imag -= imag[52]*q_100r + imag[53]*q_101r + imag[54]*q_110r + imag[55]*q_111r;

                    q_rd[q_111_off].imag  = real[56]*q_000i + real[57]*q_001i + real[58]*q_010i + real[59]*q_011i;
                    q_rd[q_111_off].imag += real[60]*q_100i + real[61]*q_101i + real[62]*q_110i + real[63]*q_111i;
                    q_rd[q_111_off].imag -= imag[56]*q_000r + imag[57]*q_001r + imag[58]*q_010r + imag[59]*q_011r;
                    q_rd[q_111_off].imag -= imag[60]*q_100r + imag[61]*q_101r + imag[62]*q_110r + imag[63]*q_111r;

                    q_000_off++;    q_001_off++;
                    q_010_off++;    q_011_off++;
                    q_100_off++;    q_101_off++;
                    q_110_off++;    q_111_off++;
                }
                q_000_off += gate_move.half_small;      q_001_off += gate_move.half_small;
                q_010_off += gate_move.half_small;      q_011_off += gate_move.half_small;
                q_100_off += gate_move.half_small;      q_101_off += gate_move.half_small;
                q_110_off += gate_move.half_small;      q_111_off += gate_move.half_small;
            }
            q_000_off += gate_move.half_middle;     q_001_off += gate_move.half_middle;
            q_010_off += gate_move.half_middle;     q_011_off += gate_move.half_middle;
            q_100_off += gate_move.half_middle;     q_101_off += gate_move.half_middle;
            q_110_off += gate_move.half_middle;     q_111_off += gate_move.half_middle;
        }
        q_000_off += gate_move.half_large;      q_001_off += gate_move.half_large;
        q_010_off += gate_move.half_large;      q_011_off += gate_move.half_large;
        q_100_off += gate_move.half_large;      q_101_off += gate_move.half_large;
        q_110_off += gate_move.half_large;      q_111_off += gate_move.half_large;
    }
};

/*===================================================================
gate level (3rd version.)
For measure

prepare:
gate_move.half_targ
gate_size

void *_gate(Type *rd)
input: 指向一個存放gate_size這麼多state的buffer。
功能: 對buffer內的state操作，使他們變成作用完gate之後的樣子
===================================================================*/

void PreMeasure(Type *q_rd){
    int up_off = 0;
    int lo_off = gate_move.half_targ;
    Type_t q_0r;    Type_t q_0i;
    Type_t q_1r;    Type_t q_1i;

    double p0=0;
    double p1=0;
    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_0r = q_rd[up_off].real;   q_0i = q_rd[up_off].imag;
            q_1r = q_rd[lo_off].real;   q_1i = q_rd[lo_off].imag;
            p0 += q_0r*q_0r + q_0i*q_0i;
            p1 += q_1r*q_1r + q_1i*q_1i;
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
    #pragma omp critical
    {
        real[0] += p0;
        real[1] += p1;
    }
}

void Measure_0 (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_rd[up_off].real *= real[0];
            q_rd[up_off].imag *= real[0];
            q_rd[lo_off].real = 0;
            q_rd[lo_off].imag = 0;
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}

void Measure_1 (Type *q_rd) {
    int up_off = 0;
    int lo_off = gate_move.half_targ;

    for (int i = 0; i < gate_size; i += gate_move.half_targ*2) {
        for (int j = 0; j < gate_move.half_targ; j++){
            q_rd[up_off].real = 0;
            q_rd[up_off].imag = 0;
            q_rd[lo_off].real *= real[0];
            q_rd[lo_off].imag *= real[0];
            up_off++;
            lo_off++;
        }
        up_off += gate_move.half_targ;
        lo_off += gate_move.half_targ;
    }
}
