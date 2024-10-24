#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.h"
#include "gate.h"
#include "gate_util.h"

void set_outer(ull outer){
    _outer = outer;
    _half_outer = _outer/2;
    _half_outer_size = _half_outer * sizeof(Type);
}

inline void _thread_CX(setStreamv2 *s){
    for(ull i = 0; i < loop_size; i += _outer){
        inner_loop_func(_outer, s->rd, s->fd, s->fd_off);
    }
}

inline void _thread_CX2(setStreamv2 *s){
    for(ull i = 0; i < loop_size; i += _outer){
        inner_loop_func(_half_outer, s->rd, s->fd, s->fd_off);
        s->fd_off[0] += _half_outer_size;
        s->fd_off[1] += _half_outer_size;
    }
}

inline void _thread_CX4(setStreamv2 *s){
    for(ull i = 0; i < loop_size; i += _outer){
        inner_loop_func(_half_outer, s->rd, s->fd, s->fd_off);
        s->fd_off[0] += _half_outer_size;
        s->fd_off[1] += _half_outer_size;
        s->fd_off[2] += _half_outer_size;
        s->fd_off[3] += _half_outer_size;
    }
}

inline void _thread_CX8(setStreamv2 *s){
    for(ull i = 0; i < loop_size; i += _outer){
        inner_loop_func(_half_outer, s->rd, s->fd, s->fd_off);
        s->fd_off[0] += _half_outer_size;
        s->fd_off[1] += _half_outer_size;
        s->fd_off[2] += _half_outer_size;
        s->fd_off[3] += _half_outer_size;
        s->fd_off[4] += _half_outer_size;
        s->fd_off[5] += _half_outer_size;
        s->fd_off[6] += _half_outer_size;
        s->fd_off[7] += _half_outer_size;
    }
}

inline void set_up_lo(int ctrl, int targ){
    if(ctrl < targ){
        up_qubit = ctrl;
        lo_qubit = targ;
    }
    else{
        up_qubit = targ;
        lo_qubit = ctrl;
    }

    small_offset = 1ULL << (N-lo_qubit);
    half_small_offset = small_offset >> 1;
    large_offset = 1ULL << (N-up_qubit);
    half_large_offset = large_offset >> 1;
}

// fd_off[0] += size * sizeof(Type) afterward
void inner_loop(ull size, void *rd, int fd[1], ull fd_off[1]){
    for (ull i = 0; i < size; i += chunk_state){
        if(pread (fd[0], rd, chunk_size, fd_off[0]));
        gate_func((Type *)rd);
        if(pwrite(fd[0], rd, chunk_size, fd_off[0]));
        fd_off[0] += chunk_size;
    }
}

void inner_loop_read(ull size, void *rd, int fd[1], ull fd_off[1]){
    for (ull i = 0; i < size; i += chunk_state){
        if(pread (fd[0], rd, chunk_size, fd_off[0]));
        gate_func((Type *)rd);
        fd_off[0] += chunk_size;
    }
}

// fd_off[0] += size * sizeof(Type) afterward
// fd_off[1] += size * sizeof(Type) afterward
void inner_loop2(ull size, void *rd, int fd[2], ull fd_off[2]){
    for (ull i = 0; i < size; i += chunk_state){
        if(pread (fd[0], rd,           chunk_size, fd_off[0]));
        if(pread (fd[1], rd+chunk_size, chunk_size, fd_off[1]));

        gate_func((Type *)rd);

        if(pwrite(fd[0], rd,           chunk_size, fd_off[0]));
        if(pwrite(fd[1], rd+chunk_size, chunk_size, fd_off[1]));
        fd_off[0] += chunk_size;
        fd_off[1] += chunk_size;
    }
}

void inner_loop2_read(ull size, void *rd, int fd[2], ull fd_off[2]){
    for (ull i = 0; i < size; i += chunk_state){
        if(pread (fd[0], rd,           chunk_size, fd_off[0]));
        if(pread (fd[1], rd+chunk_size, chunk_size, fd_off[1]));
        gate_func((Type *)rd);
        fd_off[0] += chunk_size;
        fd_off[1] += chunk_size;
    }
}

void inner_loop2_swap(ull size, void *rd, int fd[2], ull fd_off[2]){
    for (ull i = 0; i < size; i += chunk_state){
        if(pread (fd[0], rd + chunk_size, chunk_size, fd_off[0]));
        if(pread (fd[1], rd,             chunk_size, fd_off[1]));

        if(pwrite(fd[0], rd,             chunk_size, fd_off[0]));
        if(pwrite(fd[1], rd + chunk_size, chunk_size, fd_off[1]));
        fd_off[0] += chunk_size;
        fd_off[1] += chunk_size;
    }
}

void inner_loop4(ull size, void *rd, int fd[4], ull fd_off[4]){
    for (ull i = 0; i < size; i += chunk_state){
        if(pread (fd[0], rd,               chunk_size, fd_off[0]));
        if(pread (fd[1], rd +   chunk_size, chunk_size, fd_off[1]));
        if(pread (fd[2], rd + 2*chunk_size, chunk_size, fd_off[2]));
        if(pread (fd[3], rd + 3*chunk_size, chunk_size, fd_off[3]));

        gate_func((Type *)rd);

        if(pwrite(fd[0], rd,               chunk_size, fd_off[0]));
        if(pwrite(fd[1], rd +   chunk_size, chunk_size, fd_off[1]));
        if(pwrite(fd[2], rd + 2*chunk_size, chunk_size, fd_off[2]));
        if(pwrite(fd[3], rd + 3*chunk_size, chunk_size, fd_off[3]));
        fd_off[0] += chunk_size;
        fd_off[1] += chunk_size;
        fd_off[2] += chunk_size;
        fd_off[3] += chunk_size;
    }
}

void inner_loop8(ull size, void *rd, int fd[8], ull fd_off[8]){
    void *rd0 = rd;
    void *rd1 = rd + 1*chunk_size;
    void *rd2 = rd + 2*chunk_size;
    void *rd3 = rd + 3*chunk_size;
    void *rd4 = rd + 4*chunk_size;
    void *rd5 = rd + 5*chunk_size;
    void *rd6 = rd + 6*chunk_size;
    void *rd7 = rd + 7*chunk_size;

    for (ull i = 0; i < size; i += chunk_state){
        if(pread (fd[0], rd0, chunk_size, fd_off[0]));
        if(pread (fd[1], rd1, chunk_size, fd_off[1]));
        if(pread (fd[2], rd2, chunk_size, fd_off[2]));
        if(pread (fd[3], rd3, chunk_size, fd_off[3]));
        if(pread (fd[4], rd4, chunk_size, fd_off[4]));
        if(pread (fd[5], rd5, chunk_size, fd_off[5]));
        if(pread (fd[6], rd6, chunk_size, fd_off[6]));
        if(pread (fd[7], rd7, chunk_size, fd_off[7]));

        gate_func((Type *)rd);

        if(pwrite(fd[0], rd0, chunk_size, fd_off[0]));
        if(pwrite(fd[1], rd1, chunk_size, fd_off[1]));
        if(pwrite(fd[2], rd2, chunk_size, fd_off[2]));
        if(pwrite(fd[3], rd3, chunk_size, fd_off[3]));
        if(pwrite(fd[4], rd4, chunk_size, fd_off[4]));
        if(pwrite(fd[5], rd5, chunk_size, fd_off[5]));
        if(pwrite(fd[6], rd6, chunk_size, fd_off[6]));
        if(pwrite(fd[7], rd7, chunk_size, fd_off[7]));
        
        fd_off[0] += chunk_size;
        fd_off[1] += chunk_size;
        fd_off[2] += chunk_size;
        fd_off[3] += chunk_size;
        fd_off[4] += chunk_size;
        fd_off[5] += chunk_size;
        fd_off[6] += chunk_size;
        fd_off[7] += chunk_size;
    }
}

int isCtrl(int i , int ctrl, int n){
    int ctrl_mask = 1 << (n-ctrl-1);
    return i&ctrl_mask;
}

int iPair(const int i, const int targ, const int n){
    int targ_mask = 1 << (n-targ-1);
    return i^targ_mask;
}

void make_targ_pair(const int n, const int targ, int *pair_list){
    int idx = 0;
    int *checklist = (int *)malloc((1 << n) * sizeof(int));
    for (int i = 0; i < 1<<n; i += 1){
        checklist[i] = 0;
    }

    for (int i = 0; i < 1<<n; i += 1){
        if(checklist[i])
            continue;
        checklist[i] = 1;
        int p = iPair(i, targ, n);
        checklist[p] = 1;
        pair_list[idx++] = i;
        pair_list[idx++] = p;
    }

    // for (int i = 0; i < 1<<n; i += 1){
    //     printf("%d ", pair_list[i]);
    // }
    // printf("\n");
    // printf("in make_targ_pair\n");
    free(checklist);
}

void make_work_pair(const int n, const int ctrl, const int targ, int *pair_list){
    int idx = 0;
    int *checklist = (int *)malloc((1 << n) * sizeof(int));
    for (int i = 0; i < 1<<n; i += 1){
        checklist[i] = 0;
    }

    for (int i = 0; i < 1<<n; i += 1){
        if(checklist[i])
            continue;
        checklist[i] = 1;
        if (ctrl < 0 && targ >= 0){
            int p = iPair(i, targ, n);
            checklist[p] = 1;
            pair_list[idx++] = i < p ? i : p;
            pair_list[idx++] = i < p ? p : i;
        }
        else if (isCtrl(i, ctrl, n)){
            if(targ < 0){
                pair_list[idx++] = i;
                pair_list[idx++] = i;
            }
            else{
                int p = iPair(i, targ, n);
                checklist[p] = 1;
                pair_list[idx++] = i < p ? i : p;
                pair_list[idx++] = i < p ? p : i;
            }
        }
    }
    // for (int i = 0; i < 1<<n; i += 1){
    //     printf("%d ", pair_list[i]);
    // }
    // printf("\n");

    free(checklist);
}

void make_work_group(const int n, const int q0, const int q1, int *group_list){
    int idx = 0;
    int *checklist = (int *)malloc((1 << n) * sizeof(int));
    for (int i = 0; i < 1<<n; i += 1){
        checklist[i] = 0;
    }

    for (int i = 0; i < 1<<n; i += 1){
        if(checklist[i])
            continue;
        int m00 = i;
        int m01 = iPair(i, q1, n);
        int m10 = iPair(i, q0, n);
        int m11 = iPair(m10, q1, n);
        checklist[m00] = 1;
        checklist[m01] = 1;
        checklist[m10] = 1;
        checklist[m11] = 1;
        group_list[idx++] = m00;
        group_list[idx++] = m01;
        group_list[idx++] = m10;
        group_list[idx++] = m11;
    }

    // for (int i = 0; i < 1<<n; i += 1){
    //     printf("%d ", group_list[i]);
    // }
    // printf("\n");
    // fflush(stdout);

    free(checklist);
}

void make_work_warp(const int n, const int q0, const int q1, const int q2, int *warp_list){
    int idx = 0;
    int *checklist = (int *)malloc((1 << n) * sizeof(int));
    for (int i = 0; i < 1<<n; i += 1){
        checklist[i] = 0;
    }

    for (int i = 0; i < 1<<n; i += 1){
        if(checklist[i])
            continue;
        int m000 = i;
        int m001 = iPair(i, q2, n);
        int m010 = iPair(i, q1, n);
        int m011 = iPair(m010, q2, n);
        int m100 = iPair(i, q0, n);
        int m101 = iPair(m100, q2, n);
        int m110 = iPair(m100, q1, n);
        int m111 = iPair(m110, q2, n);

        checklist[m000] = 1;
        checklist[m001] = 1;
        checklist[m010] = 1;
        checklist[m011] = 1;
        checklist[m100] = 1;
        checklist[m101] = 1;
        checklist[m110] = 1;
        checklist[m111] = 1;
        warp_list[idx++] = m000;
        warp_list[idx++] = m001;
        warp_list[idx++] = m010;
        warp_list[idx++] = m011;
        warp_list[idx++] = m100;
        warp_list[idx++] = m101;
        warp_list[idx++] = m110;
        warp_list[idx++] = m111;

    }

    // for (int i = 0; i < 1<<n; i += 1){
    //     printf("%d ", warp_list[i]);
    // }
    // printf("\n");
    // fflush(stdout);

    free(checklist);
}