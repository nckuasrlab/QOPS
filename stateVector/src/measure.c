#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>
#include "common.h"
#include "gate.h"
#include "gate_util.h"
#include "gate_chunk.h"
#include "measure.h"

int* measure_fd_arr;

void measure(int targ, int fd_set){
    int t = omp_get_thread_num();
    setStreamv2 *s = &thread_settings[t];

    /*----------------------------
    [1st phase] Setting up global variables.
    ----------------------------*/
    if (t == 0){
        measure_fd_arr = fd_arr_set[fd_set];

        gate_func = PreMeasure;

        loop_size = thread_state;
    
        if (isLocal(targ)){
            inner_loop_func = inner_loop_read;
            gate_move.half_targ = (1ULL << (N-targ-1));
            gate_size = chunk_state;
        }
        else{
            inner_loop_func = inner_loop2_read;
            gate_move.half_targ = chunk_state;
            gate_size = 2 * chunk_state;
        }
        if(isMiddle(targ)){
            targ_offset = 1 << (N-targ);
            half_targ_offset = targ_offset >> 1;
            set_outer(targ_offset);
        }
        else{
            _outer = thread_state;
        }

        if(isGlobal(targ)){
            make_targ_pair(file_segment, targ, fd_pair);
        }
        if(isThread(targ)){
            make_targ_pair(para_segment, targ-file_segment, td_pair);
        }

        real = (double*)malloc(2*sizeof(double));
        real[0] = 0.0;
        real[1] = 0.0;
        // printf("Thread rank: %d pass barrier, targ= %d\n", t, targ);
    }

    #pragma omp barrier

    /*------------------------------
    [2nd phase] Applying measure
    ------------------------------*/
    if ((isGlobal(targ) || isThread(targ)) && t >= half_num_thread){
        #pragma omp barrier
        #pragma omp barrier
        return;
    }

    if (isGlobal(targ)){
        int f = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        int fd0 = fd_pair[2*f];
        int fd1 = fd_pair[2*f+1];

        ull t_off = td * thread_state * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd0]; s->fd_off[0] = t_off;
        s->fd[1] = measure_fd_arr[fd1]; s->fd_off[1] = t_off;
        _thread_CX(s);
    }
    
    if (isThread(targ)) {
        int fd = t/(num_thread_per_file/2);
        int td = t%(num_thread_per_file/2);

        int t0 = td_pair[2*td];
        int t1 = td_pair[2*td + 1];

        ull t0_off = t0 * thread_state * sizeof(Type);
        ull t1_off = t1 * thread_state * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd]; s->fd_off[0] = t0_off;
        s->fd[1] = measure_fd_arr[fd]; s->fd_off[1] = t1_off;
        _thread_CX(s);
    } 

    if(isMiddle(targ)){
        int fd = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        ull t0_off = td * thread_state * sizeof(Type);
        ull t1_off = t0_off + half_targ_offset * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd]; s->fd_off[0] = t0_off;
        s->fd[1] = measure_fd_arr[fd]; s->fd_off[1] = t1_off;
        _thread_CX2(s);
    }

    if (isLocal(targ)){
        int fd = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        ull t_off = td * thread_state * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd]; s->fd_off[0] = t_off;
        _thread_CX(s);
    }

    #pragma omp barrier
    
    /*----------------------------
    [3rd phase] Setting up global variables.
    ----------------------------*/
    if (t == 0){
        // printf("target %d = 0, with probability %lf\n", targ, real[0]);
        // printf("target %d = 1, with probability %lf\n\n", targ, real[1]);
        // fflush(stdout);

        double d;
        d = (double) rand() / ((double) RAND_MAX + 1);
        d *= (real[0]+real[1]);

        // if(real[0] > 0.99999){
        //     printf("[1]");
        //     // printf("[0]target %d = 0", targ);
        //     // fflush(stdout);
        //     gate_func = Measure_0;
        //     real[0] = 1;
        //     real[1] = 0;
        // }
        // else if(real[1] > 0.99999){
        //     printf("[2]");
        //     // printf("[1]target %d = 1", targ);
        //     // fflush(stdout);
        //     gate_func = Measure_1;
        //     real[0] = 1;
        //     real[1] = 1;
        // }
        if(d < real[0]){
            // printf("[3]");
            // printf("[0]target %d = 0", targ);
            // fflush(stdout);
            gate_func = Measure_0;
            real[0] = 1/sqrt(real[0]);
            real[1] = 0;
        }
        else{
            // printf("[4]");
            // printf("[1]target %d = 1", targ);
            // fflush(stdout);
            gate_func = Measure_1;
            real[0] = 1/sqrt(real[1]);
            real[1] = 1;
        }

        if (isLocal(targ)){
            inner_loop_func = inner_loop;
        }
        else{
            inner_loop_func = inner_loop2;
        }
    }

    #pragma omp barrier

    /*------------------------------
    [4th phase] Applying measure
    ------------------------------*/
    if (isGlobal(targ)){
        int f = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        int fd1 = fd_pair[2*f];
        int fd2 = fd_pair[2*f+1];

        ull t_off = td * thread_state * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd1]; s->fd_off[0] = t_off;
        s->fd[1] = measure_fd_arr[fd2]; s->fd_off[1] = t_off;
        _thread_CX(s);
        return;
    }
    
    if (isThread(targ)) {
        int fd = t/(num_thread_per_file/2);
        int td = t%(num_thread_per_file/2);

        int t0 = td_pair[2*td];
        int t1 = td_pair[2*td + 1];

        ull t0_off = t0 * thread_state * sizeof(Type);
        ull t1_off = t1 * thread_state * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd]; s->fd_off[0] = t0_off;
        s->fd[1] = measure_fd_arr[fd]; s->fd_off[1] = t1_off;
        _thread_CX(s);
        return;
    }

    if(isMiddle(targ)){
        int fd = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        ull t0_off = td * thread_state * sizeof(Type);
        ull t1_off = t1_off + half_targ_offset * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd]; s->fd_off[0] = t0_off;
        s->fd[1] = measure_fd_arr[fd]; s->fd_off[1] = t1_off;
        _thread_CX2(s);
        return;
    }

    if (isLocal(targ)){
        int fd = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        ull t_off = td * thread_state * sizeof(Type);

        s->fd[0] = measure_fd_arr[fd]; s->fd_off[0] = t_off;
        _thread_CX(s);
        return;
    }

    // shouldn't be here, previous states has been saved. 
    printf("error gate instruction: should not be here.\n");
    exit(0);
}

void save_state(int fd_set_src, int fd_set_dst){
    int t = omp_get_thread_num();
    int fd = t/num_thread_per_file;
    int td = t%num_thread_per_file;

    int fd_src = fd_arr_set[fd_set_src][fd];
    int fd_dst = fd_arr_set[fd_set_dst][fd];

    ull t_off = td * thread_state * sizeof(Type);

    void* rd = (void *) q_read + t * chunk_size;
    for(ull i = 0; i < thread_state; i += chunk_state){
        if(pread  (fd_src, rd, chunk_size, t_off));
        if(pwrite (fd_dst, rd, chunk_size, t_off));
        t_off += chunk_size;
    }
    return;
}