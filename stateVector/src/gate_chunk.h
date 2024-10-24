#ifndef GATE_CHUNK_H_
#define GATE_CHUNK_H_

void H_gate (Type *q_rd);
void S_gate (Type *q_rd); void Sc_gate (Type *q_rd);
void T_gate (Type *q_rd); void Tc_gate (Type *q_rd);
void X_gate (Type *q_rd);
void Y_gate (Type *q_rd); void Yc_gate (Type *q_rd);
void Z_gate (Type *q_rd);
void P_gate (Type *q_rd); void Pc_gate (Type *q_rd);
void U_gate (Type *q_rd); void Uc_gate (Type *q_rd);

void X_gate2 (Type *q_rd);
void Y_gate2 (Type *q_rd); void Yc_gate2 (Type *q_rd);
void Z_gate2 (Type *q_rd);
void P_gate2 (Type *q_rd); void Pc_gate2 (Type *q_rd);

void U_gate2 (Type *q_rd); void Uc_gate2 (Type *q_rd);
void U2_gate (Type *q_rd); void U2c_gate (Type *q_rd);
void SWAP_gate (Type *q_rd);
void U3_gate (Type *q_rd); void U3c_gate (Type *q_rd);

void PreMeasure (Type *q_rd);
void Measure_0 (Type *q_rd);
void Measure_1 (Type *q_rd);

extern void (*gate_ops[16][2])(Type *);

#endif
