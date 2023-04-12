#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>
#include "common.h"
#include "gate.h"

unsigned int total_gate;
gate *gateMap; // gate gateMap [MAX_QUBIT*max_depth];
setStreamv2 *thread_settings;

/*===================================================================
Qubit type guide

Qubit的順序從左邊開始為q0，根據Qubit在不同位置翻轉之後，
對應的另一個狀態跟自己的相對位置，可以有下面四種可能。

1. Global
這一段的qubit翻轉之後會被放到另一個檔案

2. Thread
這一段的qubit翻轉之後會跑到另一段thread_state

3. Local
這一段的qubit翻轉之後會在同一個CHUNK內

4. Middle
除上述的位置之外都會是Middle qubit

按照由左至右的排列
Global -> Thread -> Middle -> Local
===================================================================*/
/*===================================================================
1 qubit gates guide

single_gate(int targ, int ops)
根據ops決定等等採用的gate函數
根據targ所在位置決定gate_move.half_targ這個共用變數。
利用omp將工作分配給不同thread處理

分配方式:
將states盡可能平均分配給各個thread
thread_state = 全部state數量/全部thread數量 = 2^N / 2^thread_segment = 1 << (N-thread_segment)
每一條thread再將thread_state打包成一個個chunk_state大小的CHUNK
這些CHUNK再進入*_gate去處理，可能一次處理一包也可能一次處理兩包(inner_loop, inner_loop2的差別)
處理的過程會丟給*_gate，遵循各gate的定義實作
https://en.wikipedia.org/wiki/Quantum_logic_gate

===================================================================*/
void swap(unsigned int *a, unsigned int *b){
    unsigned int temp = *a;
    *a = *b;
    *b = temp;
}

void find_base(const int segment, const int targ, const int tIdx, unsigned int *base, unsigned int *correspond){
    unsigned int targShift = segment-(N-targ);
    unsigned int targSegment = 1 << targShift; // 區間位置
    *base = tIdx + ((tIdx >> targShift) << targShift);
    *correspond = (*base)^targSegment;
    return;
}

void H_gate (int targ) {

    int t = omp_get_thread_num();
    setStreamv2 *s = &thread_settings[t];

    ull half_targ_offset = 1ULL << targ;
    ull targ_offset = half_targ_offset << 1;
    double sqrt2 = 1./sqrt(2);

    /*------------------------------
    Applying gate
    ------------------------------*/
    if (isFile(targ)){
        Type* rd = (Type *)s->rd;
        if(thread_state==chunk_state)
        {    
            if (t >= half_num_thread)
                return;
            unsigned int fd1 = 0, fd2 = 0;
            find_base(file_segment, targ, t, &fd1, &fd2);
            ull t_off = 0;
            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = t_off;
            s->fd_off[1] = t_off;
            for (ull i = 0; i < thread_state; i += chunk_state)
            {
                if(pread (s->fd[0], (void*)rd           , chunk_size, s->fd_off[0]));
                if(pread (s->fd[1], (void*)rd+chunk_size, chunk_size, s->fd_off[1]));
                int up_off = 0;
                int lo_off = chunk_state;
                Type_t q_0r;    Type_t q_0i;
                Type_t q_1r;    Type_t q_1i;
                for (ull j = 0; j < chunk_state; j++)
                {
                    q_0r = rd[up_off].real;   q_0i = rd[up_off].imag;
                    q_1r = rd[lo_off].real;   q_1i = rd[lo_off].imag;
                    rd[up_off].real = sqrt2 * (q_0r + q_1r);
                    rd[up_off].imag = sqrt2 * (q_0i + q_1i);
                    rd[lo_off].real = sqrt2 * (q_0r - q_1r);
                    rd[lo_off].imag = sqrt2 * (q_0i - q_1i);
                    up_off += 1;
                    lo_off += 1;
                }
                if(pwrite(s->fd[0], (void*)rd           , chunk_size, s->fd_off[0]));
                if(pwrite(s->fd[1], (void*)rd+chunk_size, chunk_size, s->fd_off[1]));
                s->fd_off[0] += chunk_size;
                s->fd_off[1] += chunk_size;
            }
        }
        else{
            unsigned int td = t % 2;
            unsigned int fd1 = 0, fd2 = 0;
            find_base(file_segment, targ, t/2, &fd1, &fd2);
            unsigned int thread_half_state = thread_state >> 1;
            ull t_off = td * thread_half_state * sizeof(Type);            
            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = t_off;
            s->fd_off[1] = t_off;

            for (ull i = 0; i < thread_half_state; i += chunk_state)
            {
                if(pread (s->fd[0], (void*)rd           , chunk_size, s->fd_off[0]));
                if(pread (s->fd[1], (void*)rd+chunk_size, chunk_size, s->fd_off[1]));
                int up_off = 0;
                int lo_off = chunk_state;
                Type_t q_0r;    Type_t q_0i;
                Type_t q_1r;    Type_t q_1i;
                for (ull j = 0; j < chunk_state; j++)
                {
                    q_0r = rd[up_off].real;   q_0i = rd[up_off].imag;
                    q_1r = rd[lo_off].real;   q_1i = rd[lo_off].imag;
                    rd[up_off].real = sqrt2 * (q_0r + q_1r);
                    rd[up_off].imag = sqrt2 * (q_0i + q_1i);
                    rd[lo_off].real = sqrt2 * (q_0r - q_1r);
                    rd[lo_off].imag = sqrt2 * (q_0i - q_1i);
                    up_off += 1;
                    lo_off += 1;
                }
                if(pwrite(s->fd[0], (void*)rd           , chunk_size, s->fd_off[0]));
                if(pwrite(s->fd[1], (void*)rd+chunk_size, chunk_size, s->fd_off[1]));
                s->fd_off[0] += chunk_size;
                s->fd_off[1] += chunk_size;
            }


        }
        
        return;
    }
    
    if(isMiddle(targ)){
        Type* rd = (Type *)(s->rd);
        int fd = t;
        int td = 0;
        
        ull t1_off = 0;
        ull t2_off = half_targ_offset * sizeof(Type);
        s->fd[0] = fd_arr[fd];
        s->fd[1] = fd_arr[fd];
        s->fd_off[0] = t1_off;
        s->fd_off[1] = t2_off;

        Type_t q_0r, q_0i, q_1r, q_1i;
        for(ull k = 0; k < thread_state; k += targ_offset){
            for (ull c = 0; c < half_targ_offset; c += chunk_state){
                if(pread (s->fd[0], (void *)rd,           chunk_size, s->fd_off[0]));
                if(pread (s->fd[1], (void *)rd+chunk_size, chunk_size, s->fd_off[1]));
                for (int up = 0, lo = chunk_state; up < chunk_state; up++, lo++) {
                    q_0r = rd[up].real;   q_0i = rd[up].imag;
                    q_1r = rd[lo].real;   q_1i = rd[lo].imag;
                    rd[up].real = sqrt2 * (q_0r + q_1r);
                    rd[up].imag = sqrt2 * (q_0i + q_1i);
                    rd[lo].real = sqrt2 * (q_0r - q_1r);
                    rd[lo].imag = sqrt2 * (q_0i - q_1i);
                }
                if(pwrite(s->fd[0], (void *)rd,           chunk_size, s->fd_off[0]));
                if(pwrite(s->fd[1], (void *)rd+chunk_size, chunk_size, s->fd_off[1]));
                s->fd_off[0] += chunk_size;
                s->fd_off[1] += chunk_size;
            }
            s->fd_off[0] += half_targ_offset * sizeof(Type);
            s->fd_off[1] += half_targ_offset * sizeof(Type);
        }
        return;
    }

    if (isChunk(targ)){
        Type* rd = (Type *) s->rd;
        // 目前的 thread 去找到要作用的 file 的 起始位址
        int fd = t;
        int td = 0;
        ull t_off = 0;
        s->fd[0] = fd_arr[fd]; // 拿到負責 file 的 id 
        s->fd_off[0] = t_off;  // 拿起該 file 的起始位置
        Type_t q_0r, q_0i, q_1r, q_1i;
        for (ull c = 0; c < thread_state; c += chunk_state){
            if(pread(s->fd[0], (void *) rd, chunk_size, s->fd_off[0]));
            int up = 0, lo = 0;
            for (int i = 0; i < chunk_state; i += targ_offset) {
                for (int j = 0; j < half_targ_offset; j++) {
                    up = i + j;
                    lo = up + half_targ_offset;
                    q_0r = rd[up].real;   q_0i = rd[up].imag;
                    q_1r = rd[lo].real;   q_1i = rd[lo].imag;
                    rd[up].real = sqrt2 * (q_0r + q_1r);
                    rd[up].imag = sqrt2 * (q_0i + q_1i);
                    rd[lo].real = sqrt2 * (q_0r - q_1r);
                    rd[lo].imag = sqrt2 * (q_0i - q_1i);
                }
            }
            if(pwrite(s->fd[0], (void *) rd, chunk_size, s->fd_off[0]));
            s->fd_off[0] += chunk_size;
        }
        return;
    }

    // shouldn't be here, previous states has been saved. 
    printf("error gate instruction: should not be here.\n");
    exit(0);
}

inline void print_gate(gate* g) {
    printf("%2d ",
        g->gate_ops);
    for(int i = 0; i < g->numCtrls; i++)
        printf("c%02d ", g->ctrls[i]);
    for(int i = 0; i < g->numTargs; i++)
        printf("t%02d ", g->targs[i]);
    printf("\n");
}
