#ifndef GATE_H_
#define GATE_H_
// typedef
typedef enum gate_ops {
  GATE_H = 0,
  GATE_S = 1,
  GATE_T = 2,
  GATE_X = 3,
  GATE_Y = 4,
  GATE_Z = 5,
  GATE_Phase = 6,
  GATE_U1 = 7,
  GATE_CX = 8,
  GATE_CY = 9,
  GATE_CZ = 10,
  GATE_CPhase = 11,
  GATE_CU1 = 12,
  GATE_SWAP = 13,
  GATE_TOFFOLI = 14,
  OPS_MEASURE = 20,
  OPS_MEASURE_MULTI = 21,
  OPS_COPY = 22,
  GATE_U2 = 31,
  GATE_U3 = 32
} GATE_OPS;

typedef double Type_t;

typedef struct gate {
  // action should only be used in scheduler.
  // for normal use it is unnecessary
  int active; // wheter this gate is combine

  GATE_OPS gate_ops;   // gate's opcode
  int numCtrls;        // #control
  int numTargs;        // #targets
  int val_num;         // #variable does the gate has
  int ctrls[3];        // at most three
  int targs[3];        // at most three
  Type_t *real_matrix; // angle (real) also put in here 可能(?)
  Type_t *imag_matrix; // row-maj
} gate;

extern unsigned int total_gate;
extern gate *gateMap; // gate gateMap [MAX_QUBIT*max_depth];

void single_gate(int targ, int ops, int density);
void control_gate(int ctrl, int targ, int ops, int density);
void SWAP(int q0, int q1, int density);

#endif /* GATE_H_ */
