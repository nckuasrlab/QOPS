#ifndef GATE_H_
#define GATE_H_

// typedef
typedef struct gate {
    // action should only be used in scheduler.
    // for normal use it is unnecessary
    int action; // wheter this gate is combine

    int gate_ops; // gate's opcode
    int numCtrls; // #control
    int numTargs; // #targets
    int val_num; // #variable does the gate has
    int ctrls [3]; // at most three
    int targs [3]; // at most three
    Type_t *real_matrix; // angle (real) also put in here 可能(?)
    Type_t *imag_matrix; // row-maj
} gate;

typedef struct setStreamv2 {
    // int id;             // thread id
    int fd[2];          // 要處理的file對應的file descriptor
    unsigned long long fd_off[2];      // 要處理的file內的offset
    void *rd;           // 指向buffer的位置
} setStreamv2;
extern setStreamv2 *thread_settings;

// global variable
extern unsigned int total_gate;
extern gate *gateMap; // gate gateMap [MAX_QUBIT*max_depth];

extern Type_t *real;
extern Type_t *imag;

void H_gate(int targ);
void print_gate(gate* g);

#endif
