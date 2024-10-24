#ifndef GATE_UTIL_H_
#define GATE_UTIL_H_

// typedef

/* gate_args guide
    主結構為for-loop
    
    Gate in Type I:
    for i in range(0, gate_size, "targ"):
        for k in range(0, "half_targ"):
    
    Gate in Type II:
    for i in range(0, gate_size, "large"):
        for j in range(0, "half_large", "small"):
*/
typedef struct {
    int large;      // 外層大迴圈
    int small;      // 內層小迴圈
    int middle;
    int half_large;
    int half_small;
    int half_middle;
    int half_ctrl;  // "buffer內" 要找到 ctrl 這個bit反轉之後另一個state 的距離
    int half_targ;  // "buffer內" 要找到 targ 這個bit反轉之後另一個state 的距離
} gate_args;
extern gate_args gate_move;

// global variable
extern Type_t *real;
extern Type_t *imag;

extern int gate_size;
extern void (*gate_func)(Type *);
extern void (*inner_loop_func)(unsigned long long, void*, int*, unsigned long long*);
extern unsigned long long loop_size;
extern unsigned long long _outer;
extern unsigned long long _half_outer;
extern unsigned long long _half_outer_size;

extern int up_qubit;
extern int lo_qubit;

extern unsigned long long small_offset;
extern unsigned long long middle_offset;
extern unsigned long long large_offset;
extern unsigned long long half_small_offset;
extern unsigned long long half_middle_offset;
extern unsigned long long half_large_offset;

extern unsigned long long ctrl_offset;
extern unsigned long long targ_offset;
extern unsigned long long half_ctrl_offset;
extern unsigned long long half_targ_offset;

extern int *fd_pair;
extern int *td_pair;

void _thread_CX(setStreamv2 *s);
void _thread_CX2(setStreamv2 *s);
void _thread_CX4(setStreamv2 *s);
void _thread_CX8(setStreamv2 *s);
void set_up_lo(int ctrl, int targ);

void inner_loop(ull size, void *rd, int fd[1], ull fd_off[1]);
void inner_loop_read(ull size, void *rd, int fd[1], ull fd_off[1]);

void inner_loop2(ull size, void *rd, int fd[2], ull fd_off[2]);
void inner_loop2_read(ull size, void *rd, int fd[2], ull fd_off[2]);
void inner_loop2_swap(ull size, void *rd, int fd[2], ull fd_off[2]);

void inner_loop4(ull size, void *rd, int fd[4], ull fd_off[4]);
void inner_loop8(ull size, void *rd, int fd[8], ull fd_off[8]);

void make_targ_pair(const int n, const int targ, int *pair_list);
void make_work_pair(const int n, const int ctrl, const int targ, int *pair_list);
void make_work_group(const int n, const int q0, const int q1, int *group_list);
void make_work_warp(const int n, const int q0, const int q1, const int q2, int *warp_list);

void set_outer(ull outer);

#endif
