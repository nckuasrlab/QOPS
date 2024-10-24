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

unsigned int total_gate;
gate *gateMap; // gate gateMap [MAX_QUBIT*max_depth];
// int **qubitTime; // int qubitTime [MAX_QUBIT][max_depth];
Type_t *real;
Type_t *imag;
void (*gate_func)(Type*);

void (*gate_func)(Type*);
void (*inner_loop_func)(ull, void*, int*, ull*);
ull loop_size;
ull _outer;
ull _half_outer;
ull _half_outer_size;

setStreamv2 *thread_settings;
int *fd_pair;
int *td_pair;

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
void single_gate (int targ, int ops, int density) {

    int t = omp_get_thread_num();
    setStreamv2 *s = &thread_settings[t];
    #pragma omp barrier
    /*----------------------------
    Setting up global variables.
    ----------------------------*/
    if (t == 0){
        gate_func = gate_ops[ops][density];

        loop_size = thread_state;
    
        if (isLocal(targ)){
            inner_loop_func = inner_loop;
            gate_move.half_targ = (1ULL << (N-targ-1));
            gate_size = chunk_state;
            // printf("innerloop_func = inner_loop\n");
            // printf("gate_size = chunk_state = %lld\n", chunk_state);
        }
        else{
            inner_loop_func = inner_loop2;
            gate_move.half_targ = chunk_state;
            gate_size = 2 * chunk_state;
            // printf("innerloop_func = inner_loop2\n");
            // printf("gate_size = 2*chunk_state = %lld\n", 2*chunk_state);
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
        else if(isThread(targ)){
            make_targ_pair(para_segment, targ-file_segment, td_pair);
        }
    }

    #pragma omp barrier

    /*------------------------------
    Applying gate
    ------------------------------*/
    if ((isGlobal(targ) || isThread(targ)) && t >= half_num_thread){
        return;
    }

    if (isGlobal(targ)){ //case 1 跨檔案
        // Drazermega
        // 注意這邊stream並不會走到下一對檔案進行處理 所以最多就是這一對固定的fd1 fd2
        // printf("isGlobal\n");
        int f = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        int fd1 = fd_pair[2*f];
        int fd2 = fd_pair[2*f+1];

        ull t_off = td * thread_state * sizeof(Type);

        s->fd[0] = fd_arr[fd1];
        s->fd[1] = fd_arr[fd2];
        s->fd_off[0] = t_off;
        s->fd_off[1] = t_off;
        _thread_CX(s);
        return;
    }
    
    if (isThread(targ)) {
        int fd = t/(num_thread_per_file/2);
        int td = t%(num_thread_per_file/2);

        int t1 = td_pair[2*td];
        int t2 = td_pair[2*td + 1];

        ull t1_off = t1 * thread_state * sizeof(Type);
        ull t2_off = t2 * thread_state * sizeof(Type);

        s->fd[0] = fd_arr[fd];
        s->fd[1] = fd_arr[fd];
        s->fd_off[0] = t1_off;
        s->fd_off[1] = t2_off;
        _thread_CX(s);
        return;
    } 

    if(isMiddle(targ)){
        int fd = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        ull t1_off = td * thread_state * sizeof(Type);
        ull t2_off = t1_off + half_targ_offset * sizeof(Type);
        
        s->fd[0] = fd_arr[fd];
        s->fd[1] = fd_arr[fd];
        s->fd_off[0] = t1_off;
        s->fd_off[1] = t2_off;
        _thread_CX2(s);
        return;
    }

    if (isLocal(targ)){
        int fd = t/num_thread_per_file;
        int td = t%num_thread_per_file;

        ull t_off = td * thread_state * sizeof(Type);

        s->fd[0] = fd_arr[fd];
        s->fd_off[0] = t_off;
        _thread_CX(s);
        return;
    }

    // shouldn't be here, previous states has been saved. 
    printf("error gate instruction: should not be here.\n");
    exit(0);
}

/*===================================================================
2 qubit gates
Control-Target type:
C_gate(ctrl, targ, single-gate)
single-gate should have 2 impl. type for basic 1 qubit and 2 qubit inside chunk.

General unitary type:

Swap type:
===================================================================*/

void control_gate(int ctrl, int targ, int ops, int density){

    /*----------------------------
    Setting up global variables.
    ----------------------------*/

    int t = omp_get_thread_num();
    setStreamv2 *s = &thread_settings[t];

    if(t == 0){
        assert(ctrl != targ);
        ctrl_offset = 1ULL << (N-ctrl);
        half_ctrl_offset = ctrl_offset >> 1;
        targ_offset = 1ULL << (N-targ);
        half_targ_offset = targ_offset >> 1;

        set_up_lo(ctrl, targ);

        /*  gate_func view
            ctrl/targ   global  thread  middle  local
            global      x       x       x       x
            thread      x       x       x       x
            middle      x       x       x       x
            local       x2      x2      x2      x2
        */

        if (isLocal(ctrl))
            gate_func = gate_ops[ops][density];
        else
            gate_func = gate_ops[ops-5][density];

        /*  inner loop view:
            ctrl/targ   global  thread  middle  local
            global      2       2       2       1
            thread      2       2       2       1
            middle      2       2       2       1
            local       2       2       2       1
        */
        if (isLocal(targ)){
            inner_loop_func = inner_loop;
            gate_size = chunk_state;
            // printf("innerloop_func = inner_loop\n");
            // printf("gate_size = chunk_state\n");
        }
        else{
            inner_loop_func = inner_loop2;
            gate_size = 2 * chunk_state;
            // printf("innerloop_func = inner_loop2\n");
            // printf("gate_size = 2 * chunk_state\n");
        }

        /*  loop-size view:
            ctrl/targ   global  thread  middle  local
            global      td_sz   td_sz   td_sz   td_sz
            thread      td_sz   td_sz   td_sz   td_sz
            middle      td_sz   td_sz   hl_of   td_sz
            local       td_sz   td_sz   td_sz   td_sz

            td_sz: thread_state
        */
        if(isMiddle(ctrl) && isMiddle(targ))
            loop_size = half_large_offset;
        else
            loop_size = thread_state;

        /*  _outer view:
            ctrl/targ   global  thread  middle  local
            global      td_sz   td_sz   s_s     td_sz
            thread      td_sz   td_sz   s_s     td_sz
            middle      s_s     s_s     s_s     s_l      
            local       td_sz   td_sz   s_l     td_sz 

            td_sz: thread_state
            s_s: _set_outer(small_offset)
            s_l: _set_outer(large_offset)
        */
        if(isMiddle(ctrl) || isMiddle(targ)){
            if(isLocal(ctrl) || isLocal(targ))
                set_outer(large_offset);
            else
                set_outer(small_offset);
        }
        else
            _outer = thread_state;

        /*  make_work_pair view:
            fd_pair
            ctrl/targ   global  thread  middle  local
            global      gctf    gc1f    gc1f    gc1f
            thread      g1tf
            middle      g1tf
            local       g1tf

            td_pair
            ctrl/targ   global  thread  middle  local
            global              s1tt
            thread      sc1t    sctt    sc1t    sc1t
            middle              s1tt
            local               s1tt

            gctf: file_segment, ctrl, targ, fd_pair
        // make_work_pair(file_segment_segment, ctrl, targ, fd_pair);

            gc1f: file_segment, ctrl, -1, fd_pair
        // make_work_pair(file_segment, ctrl, -1, fd_pair);

            g1tf: file_segment_segment, -1, targ, fd_pair
        // make_work_pair(file_segment, -1, targ, fd_pair);

            sctt: para_segment_segment, ctrl-file_segment, targ-file_segment, td_pair
        // make_work_pair(para_segment, ctrl-file_segment, targ-file_segment, td_pair);
 
            sc1t: para_segment, ctrl-file_segment, -1, td_pair
        // make_work_pair(para_segment, ctrl-file_segment, -1, td_pair);

            s1tt: para_segment, -1, targ-file_segment, td_pair
        // make_work_pair(para_segment, -1, targ-file_segment, td_pair);
        */
        if(isGlobal(ctrl) && isGlobal(targ)){
            make_work_pair(file_segment, ctrl, targ, fd_pair);
        }
        else if(isGlobal(ctrl)){
            make_work_pair(file_segment, ctrl, -1, fd_pair);
        }
        else if(isGlobal(targ)){
            make_work_pair(file_segment, -1, targ, fd_pair);
        }

        if(isThread(ctrl) && isThread(targ)){
            make_work_pair(para_segment, ctrl-file_segment, targ-file_segment, td_pair);
        }
        else if(isThread(ctrl)){
            make_work_pair(para_segment, ctrl-file_segment, -1, td_pair);
        }
        else if(isThread(targ)){
            make_work_pair(para_segment, -1, targ-file_segment, td_pair);
        }

        /*  gate_move view:
            ctrl/targ   global  thread  middle  local
            global      ht=c    ht=c    ht=c    ht=ht
            thread      ht=c    ht=c    ht=c    ht=ht
            middle      ht=c    ht=c    ht=c    ht=ht
            local       2Cs-ct  2Cs-ct  2Cs-ct  lsct

            ht=c: gate_move.half_targ = chunk_state;

            lsct:
            gate_move.large = large_offset;
            gate_move.small = small_offset;
            gate_move.half_ctrl = half_ctrl_offset;
            gate_move.half_targ = half_targ_offset;

            2Cs-ct:
            gate_move.large = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_ctrl = half_ctrl_offset;
            gate_move.half_targ = chunk_state;

            ht=hs
            gate_move.half_targ = half_targ_offset;
        */
        if(!isLocal(ctrl) && !isLocal(targ)){
            gate_move.half_targ = chunk_state;
        }
        else if(isLocal(ctrl) && isLocal(targ)){
            gate_move.large = large_offset;
            gate_move.small = small_offset;
            gate_move.half_ctrl = half_ctrl_offset;
            gate_move.half_targ = half_targ_offset;
        }
        else if(isLocal(targ)){
            gate_move.half_targ = half_targ_offset;
        }
        else{
            gate_move.large = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_ctrl = half_ctrl_offset;
            gate_move.half_targ = chunk_state;
        }

    }

    #pragma omp barrier
    /*------------------------------
    Applying gate
    ------------------------------*/


    /*  thread num view
        ctrl/targ   global  thread  middle  local
        global      Q       Q       H       H
        thread      Q       Q       H       H
        middle      H       H       N       N
        local       H       H       N       N

        N: num_thread
        H: half_num_thread
        Q: QuaterTD
    */
    if(isGlobal(ctrl) || isThread(ctrl)){
        if(isGlobal(targ) || isThread(targ)){
            if(t >= quarter_num_thread)
                return;
        }
        else if(t >= half_num_thread)
                return;
    }
    if(isGlobal(targ) || isThread(targ)){
        if(t >= half_num_thread)
            return;
    }

    if (isGlobal(ctrl)){
        if(isGlobal(targ)){
            // CX 在這個case 可以加速
            // for (int fd = 0; fd < NUMFD; fd += 2){
            //     int fd1 = fd_pair[fd];
            //     int fd2 = fd_pair[fd+1];
            //     int temp_fd;
            //     ull temp_off;
            //     temp_fd = fd_arr[fd1][0];
            //     fd_arr[fd1][0] = fd_arr[fd2][0];
            //     fd_arr[fd2][0] = temp_fd;
            //     temp_off = fd_arr[fd1][1];
            //     fd_arr[fd1][1] = fd_arr[fd2][1];
            //     fd_arr[fd2][1] = temp_off;
            // }
            
            //這邊是給更general的gate_func預備
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*fd];
            int fd2 = fd_pair[2*fd+1];
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off;
            s->fd_off[1] = fd_off;
            _thread_CX(s);
            return;
        }

        if(isThread(targ)){
            int f = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];
            ull fd_off1 = t1*thread_state*sizeof(Type);
            ull fd_off2 = t2*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off1;
            s->fd_off[1] = fd_off2;
            _thread_CX(s);
            return;
        }

        if(isMiddle(targ)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*f];
            ull fd_off1 = td*thread_state*sizeof(Type);
            ull fd_off2 = fd_off1 + half_small_offset*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd1];
            s->fd_off[0] = fd_off1;
            s->fd_off[1] = fd_off2;
            _thread_CX2(s);
            return;
        }

        if(isLocal(targ)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd = fd_pair[2*f];
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd_off[0] = fd_off;
            _thread_CX(s);
            return;
        }
    }

    if(isThread(ctrl)){ 
        if (isGlobal(targ)){
            int f = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];
            ull fd_off1 = t1*thread_state*sizeof(Type);
            ull fd_off2 = t2*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off1;
            s->fd_off[1] = fd_off2;
            _thread_CX(s);
            return;
        }

        if(isThread(targ)){
            int fd = t/(num_thread_per_file/4);
            int td = t%(num_thread_per_file/4);
            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];
            ull t1_off = (t1*thread_state)*sizeof(Type);
            ull t2_off = (t2*thread_state)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX(s);
            return;
        }

        if(isMiddle(targ)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);

            int t1 = td_pair[2*td];

            ull t1_off = (t1*thread_state) * sizeof(Type);
            ull t2_off = t1_off + half_targ_offset * sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX2(s);
            return;
        }

        if(isLocal(targ)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int t1 = td_pair[2*td];
            ull t_off = (t1*thread_state)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd_off[0] = t_off;
            _thread_CX(s);
            return;
        }
    }

    if(isMiddle(ctrl)){
        if(isGlobal(targ)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            ull fd_off = (td*thread_state + half_small_offset)*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off;
            s->fd_off[1] = fd_off;
            _thread_CX2(s);
            return;
        }

        if(isThread(targ)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);

            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];

            ull t1_off = (t1*thread_state+half_ctrl_offset)*sizeof(Type);
            ull t2_off = (t2*thread_state+half_ctrl_offset)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX2(s);
            return;
        }

        if(isMiddle(targ)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            ull t1_off = (td*thread_state + half_ctrl_offset)*sizeof(Type);
            ull t2_off = t1_off + half_targ_offset*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;

            for (ull i = 0; i < thread_state; i += large_offset){
                _thread_CX2(s);
                s->fd_off[0] += half_large_offset*sizeof(Type);
                s->fd_off[1] += half_large_offset*sizeof(Type);
            }

            return;
        }

        if(isLocal(targ)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            ull t_off = (td*thread_state + half_ctrl_offset)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd_off[0] = t_off;
            _thread_CX2(s);
            return;
        }
    }

    if (isLocal(ctrl)){
        if(isGlobal(targ)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off;
            s->fd_off[1] = fd_off;
            _thread_CX(s);
            return;
        }

        if(isThread(targ)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];
            ull t1_off = (t1*thread_state)*sizeof(Type);
            ull t2_off = (t2*thread_state)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX(s);
            return;
        }

        if(isMiddle(targ)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            ull t1_off = (td*thread_state) * sizeof(Type);
            ull t2_off = t1_off + half_targ_offset * sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX2(s);
            return;
        }

        if(isLocal(targ)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd_off[0] = fd_off;
            _thread_CX(s);
            return;
        }
    }

    // shouldn't be here, previous states has been saved.
    printf("error gate instruction: should not be here.\n");
    exit(0);
}

void unitary4x4(int q0, int q1, int density){
    // for (int i = 0; i < 4; i++){
    //     for(int j = 0; j < 4; j++){
    //         printf("%lf+",real[4*i+j]);
    //         printf("%lfi ",imag[4*i+j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    /*----------------------------
    Setting up global variables.
    ----------------------------*/

    int t = omp_get_thread_num();
    setStreamv2 *s = &thread_settings[t];

    if(t == 0){
        assert(q0 < q1);

        set_up_lo(q0, q1);

        /*  gate_func view
            q0/q1    global  thread  middle  local
            global   v2      v2      v2      v2
            thread           v2      v2      v2
            middle                   v2      v2
            local                            v2
        */

        gate_func = gate_ops[13][density];

        /*  inner loop view:
            q0/q1    global  thread  middle  local
            global   4       4       4       2
            thread           4       4       2
            middle                   4       2
            local                            1
        */
        
        if (isLocal(q0)){ // then q1 is local, too
            inner_loop_func = inner_loop;
            gate_size = chunk_state;
        }
        else if(isLocal(q1)){ // then q0 not local
            inner_loop_func = inner_loop2;
            gate_size = 2 * chunk_state;
        }
        else{ // both q0 q1 not local
            inner_loop_func = inner_loop4;
            gate_size = 4 * chunk_state;
        }
        
        
        // [TODO] check later
        /*  loop-size view:
            q0/q1    global  thread  middle  local
            global   td_sz   td_sz   td_sz   td_sz
            thread           td_sz   td_sz   td_sz
            middle                   hl_of   td_sz
            local                            td_sz

            td_sz: thread_state
        */
        if(isMiddle(q0) && isMiddle(q1))
            loop_size = half_large_offset;
        else
            loop_size = thread_state;

        /*  _outer view:
            q0/q1    global  thread  middle  local
            global   td_sz   td_sz   s_s     td_sz
            thread           td_sz   s_s     td_sz
            middle                   s_s     s_l
            local                            td_sz

            td_sz: thread_state
            s_s: _set_outer(small_offset)
            s_l: _set_outer(large_offset)
        */
        _outer = thread_state;
        if(isMiddle(q0))
            set_outer(large_offset);
        if(isMiddle(q1))
            set_outer(small_offset);

        /*  make_work_pair view:
            fd_pair
            q0/q1    global  thread  middle  local
            global   gctf    gcf     gcf     gcf
            thread
            middle
            local

            td_pair
            q0/q1    global  thread  middle  local
            global           stt
            thread           sctt    sct     sct
            middle
            local

            gctf: file_segment, ctrl, targ, fd_pair
        // make_work_group(file_segment, q0, q1, fd_pair);

            gcf: file_segment, ctrl, -1, fd_pair
        // make_targ_pair(file_segment, ctrl, fd_pair);

            sctt: para_segment, ctrl-file_segment, targ-file_segment, td_pair
        // make_work_group(para_segment, ctrl-file_segment, targ-file_segment, td_pair);
 
            sct: para_segment, ctrl-file_segment, -1, td_pair
        // make_targ_pair(para_segment, ctrl-file_segment, td_pair);

            s1tt: para_segment, -1, targ-file_segment, td_pair
        // make_targ_pair(para_segment, targ-file_segment, td_pair);
        */
        if(isGlobal(q0)){
            if(isGlobal(q1))
                make_work_group(file_segment, q0, q1, fd_pair);
            else
                make_targ_pair(file_segment, q0, fd_pair);
        } 

        if(isThread(q0) && isThread(q1)){
            make_work_group(para_segment, q0-file_segment, q1-file_segment, td_pair);
        }
        else if(isThread(q0)){
            make_targ_pair(para_segment, q0-file_segment, td_pair);
        }
        else if(isThread(q1)){
            make_targ_pair(para_segment, q1-file_segment, td_pair);
        }

        /*  gate_move view:
            q0/q1    global  thread  middle  local
            global   4c-2c   4c-2c   4c-2c   2c-s
            thread           4c-2c   4c-2c   2c-s
            middle                   4c-2c   2c-s
            local                            l-s

            4c-2c:
            gate_move.large = 4*chunk_state;
            gate_move.small = 2*chunk_state;
            gate_move.half_large = 2*chunk_state;
            gate_move.half_small = chunk_state;

            l-s:
            gate_move.large = large_offset;
            gate_move.small = small_offset;
            gate_move.half_large = half_large_offset;
            gate_move.half_small = half_small_offset;

            2c-s
            gate_move.large = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_large = chunk_state;
            gate_move.half_small = half_small_offset;
        */

        if(!isLocal(q0) && !isLocal(q1)){
            gate_move.large = 4*chunk_state;
            gate_move.small = 2*chunk_state;
            gate_move.half_large = 2*chunk_state;
            gate_move.half_small = chunk_state;
        }
        else if(isLocal(q0) && isLocal(q1)){
            gate_move.large = large_offset;
            gate_move.small = small_offset;
            gate_move.half_large = half_large_offset;
            gate_move.half_small = half_small_offset;
        }
        else if(isLocal(q1)){
            gate_move.large = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_large = chunk_state;
            gate_move.half_small = half_small_offset;
        }
    }

    #pragma omp barrier
    /*------------------------------
    Applying gate
    ------------------------------*/


    /*  thread num view
        q0/q1    global  thread  middle  local
        global   Q       Q       H       H
        thread           Q       H       H
        middle                   N       N
        local                            N

        N: num_thread
        H: half_num_thread
        Q: QuaterTD
    */
    if(isGlobal(q1) || isThread(q1)){
        if(t >= quarter_num_thread)
            return;
    }
    else if(isGlobal(q0) || isThread(q0)){
        if(t >= half_num_thread)
            return;
    }

    if (isGlobal(q0)){
        if(isGlobal(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            int fd1 = fd_pair[4*fd];
            int fd2 = fd_pair[4*fd+1];
            int fd3 = fd_pair[4*fd+2];
            int fd4 = fd_pair[4*fd+3];

            ull fd_off = td * thread_state * sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd[2] = fd_arr[fd3];
            s->fd[3] = fd_arr[fd4];
            s->fd_off[0] = fd_off;
            s->fd_off[1] = fd_off;
            s->fd_off[2] = fd_off;
            s->fd_off[3] = fd_off;
            _thread_CX(s);
            return;
        }

        if(isThread(q1)){
            int f = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];
            ull fd_off1 = t1*thread_state*sizeof(Type);
            ull fd_off2 = t2*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd1];
            s->fd[2] = fd_arr[fd2];
            s->fd[3] = fd_arr[fd2];
            s->fd_off[0] = fd_off1;
            s->fd_off[1] = fd_off2;
            s->fd_off[2] = fd_off1;
            s->fd_off[3] = fd_off2;
            _thread_CX(s);
            return;
        }

        if(isMiddle(q1)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];

            ull fd_off1 = td*thread_state*sizeof(Type);
            ull fd_off2 = fd_off1 + half_small_offset*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd1];
            s->fd[2] = fd_arr[fd2];
            s->fd[3] = fd_arr[fd2];
            s->fd_off[0] = fd_off1;
            s->fd_off[1] = fd_off2;
            s->fd_off[2] = fd_off1;
            s->fd_off[3] = fd_off2;
            _thread_CX4(s);
            return;
        }

        if(isLocal(q1)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off;
            s->fd_off[1] = fd_off;
            _thread_CX(s);
            return;
        }
    }

    if(isThread(q0)){ 
        if(isThread(q1)){
            int fd = t/(num_thread_per_file/4);
            int td = t%(num_thread_per_file/4);
            int t1 = td_pair[4*td];
            int t2 = td_pair[4*td+1];
            int t3 = td_pair[4*td+2];
            int t4 = td_pair[4*td+3];
            ull t1_off = (t1*thread_state)*sizeof(Type);
            ull t2_off = (t2*thread_state)*sizeof(Type);
            ull t3_off = (t3*thread_state)*sizeof(Type);
            ull t4_off = (t4*thread_state)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd[2] = fd_arr[fd];
            s->fd[3] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            s->fd_off[2] = t3_off;
            s->fd_off[3] = t4_off;
            _thread_CX(s);
            return;
        }

        if(isMiddle(q1)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);

            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];

            ull t1_off = (t1*thread_state) * sizeof(Type);
            ull t2_off = t1_off + half_small_offset * sizeof(Type);
            ull t3_off = (t2*thread_state) * sizeof(Type);
            ull t4_off = t3_off + half_small_offset * sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd[2] = fd_arr[fd];
            s->fd[3] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            s->fd_off[2] = t3_off;
            s->fd_off[3] = t4_off;
            _thread_CX4(s);
            return;
        }

        if(isLocal(q1)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];
            ull t1_off = (t1*thread_state)*sizeof(Type);
            ull t2_off = (t2*thread_state)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX(s);
            return;
        }
    }

    if(isMiddle(q0)){
        if(isMiddle(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            ull t1_off = td * thread_state * sizeof(Type);
            ull t2_off = (td*thread_state + half_small_offset)*sizeof(Type);
            ull t3_off = (td*thread_state + half_large_offset)*sizeof(Type);
            ull t4_off = t3_off + half_small_offset*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd[2] = fd_arr[fd];
            s->fd[3] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            s->fd_off[2] = t3_off;
            s->fd_off[3] = t4_off;

            for (ull i = 0; i < thread_state; i += large_offset){
                _thread_CX2(s);
                s->fd_off[0] += half_large_offset*sizeof(Type);
                s->fd_off[1] += half_large_offset*sizeof(Type);
                s->fd_off[2] += half_large_offset*sizeof(Type);
                s->fd_off[3] += half_large_offset*sizeof(Type);
            }

            return;
        }

        if(isLocal(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            ull t1_off = td * thread_state * sizeof(Type);
            ull t2_off = t1_off + half_large_offset*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX2(s);
            return;
        }
    }

    if (isLocal(q0)){
        if(isLocal(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd_off[0] = fd_off;
            _thread_CX(s);
            return;
        }
    }

    // shouldn't be here, previous states has been saved.
    printf("error gate instruction: should not be here.\n");
    exit(0);
}

void SWAP(int q0, int q1, int density){
    // for (int i = 0; i < 4; i++){
    //     for(int j = 0; j < 4; j++){
    //         printf("%lf+",real[4*i+j]);
    //         printf("%lfi ",imag[4*i+j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    /*----------------------------
    Setting up global variables.
    ----------------------------*/

    int t = omp_get_thread_num();
    setStreamv2 *s = &thread_settings[t];

    if(t == 0){
        assert(q0 < q1);

        set_up_lo(q0, q1);

        /*  gate_func view
            q0/q1    global  thread  middle  local
            global   N/A     N/A     N/A     v2
            thread           N/A     N/A     v2
            middle                   N/A     v2
            local                            v2
        */

        gate_func = gate_ops[14][0];

        /*  inner loop view:
            q0/q1    global  thread  middle  local
            global   2       2       2       2
            thread           2       2       2
            middle                   2       2
            local                            1
        */
        
        if (isLocal(q0)){ // then q1 is local, too
            inner_loop_func = inner_loop;
            gate_size = chunk_state;
        }
        else if (isLocal(q1)){ // then q0 not local
            inner_loop_func = inner_loop2;
            gate_size = 2 * chunk_state;
        }
        else{
            inner_loop_func = inner_loop2_swap;
            gate_size = 2 * chunk_state;
        }
        
        
        // [TODO] check later
        /*  loop-size view:
            q0/q1    global  thread  middle  local
            global   td_sz   td_sz   td_sz   td_sz
            thread           td_sz   td_sz   td_sz
            middle                   hl_of   td_sz
            local                            td_sz

            td_sz: thread_state
        */
        if(isMiddle(q0) && isMiddle(q1))
            loop_size = half_large_offset;
        else
            loop_size = thread_state;

        // [TODO] check later
        /*  _outer view:
            q0/q1    global  thread  middle  local
            global   td_sz   td_sz   s_s     td_sz
            thread           td_sz   s_s     td_sz
            middle                   s_s     s_l
            local                            td_sz

            td_sz: thread_state
            s_s: _set_outer(small_offset)
            s_l: _set_outer(large_offset)
        */
        _outer = thread_state;
        if(isMiddle(q0))
            set_outer(large_offset);
        if(isMiddle(q1))
            set_outer(small_offset);

        /*  make_work_pair view:
            fd_pair
            q0/q1    global  thread  middle  local
            global   gctf    gcf     gcf     gcf
            thread
            middle
            local

            td_pair
            q0/q1    global  thread  middle  local
            global           stt
            thread           sctt    sct     sct
            middle
            local

            gctf: file_segment, ctrl, targ, fd_pair
        // make_work_group(file_segment, q0, q1, fd_pair);

            gcf: file_segment, ctrl, -1, fd_pair
        // make_targ_pair(file_segment, ctrl, fd_pair);

            sctt: para_segment, ctrl-file_segment, targ-file_segment, td_pair
        // make_work_group(para_segment, ctrl-file_segment, targ-file_segment, td_pair);
 
            sct: para_segment, ctrl-file_segment, -1, td_pair
        // make_targ_pair(para_segment, ctrl-file_segment, td_pair);

            s1tt: para_segment, -1, targ-file_segment, td_pair
        // make_targ_pair(para_segment, targ-file_segment, td_pair);
        */
        if(isGlobal(q0)){
            if(isGlobal(q1))
                make_work_group(file_segment, q0, q1, fd_pair);
            else
                make_targ_pair(file_segment, q0, fd_pair);
        } 

        if(isThread(q0) && isThread(q1)){
            make_work_group(para_segment, q0-file_segment, q1-file_segment, td_pair);
        }
        else if(isThread(q0)){
            make_targ_pair(para_segment, q0-file_segment, td_pair);
        }
        else if(isThread(q1)){
            make_targ_pair(para_segment, q1-file_segment, td_pair);
        }

        /*  gate_move view:
            q0/q1    global  thread  middle  local
            global   N/A     N/A     N/A     2cs
            thread           N/A     N/A     2cs
            middle                   N/A     2cs
            local                            l-s

            N/A:
            swap by inner_loop2_swap

            l-s:
            gate_move.large = large_offset;
            gate_move.small = small_offset;
            gate_move.half_large = half_large_offset;
            gate_move.half_small = half_small_offset;

            2cs
            gate_move.large = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_large = chunk_state;
            gate_move.half_small = half_small_offset;
        */

        if(isLocal(q0)){
            gate_move.large = large_offset;
            gate_move.small = small_offset;
            gate_move.half_large = half_large_offset;
            gate_move.half_small = half_small_offset;
        }
        else if(isLocal(q1)){
            printf("[SWAP]: here\n");
            gate_move.large = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_large = chunk_state;
            gate_move.half_small = half_small_offset;
        }
    }

    #pragma omp barrier
    /*------------------------------
    Applying gate
    ------------------------------*/


    /*  thread num view
        q0/q1    global  thread  middle  local
        global   Q       Q       H       H
        thread           Q       H       H
        middle                   N       N
        local                            N

        N: num_thread
        H: half_num_thread
        Q: QuaterTD
    */
    if(isGlobal(q1) || isThread(q1)){
        if(t >= quarter_num_thread)
            return;
    }
    else if(isGlobal(q0) || isThread(q0)){
        if(t >= half_num_thread)
            return;
    }

    if (isGlobal(q0)){
        if(isGlobal(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            int fd1 = fd_pair[4*fd+1];
            int fd2 = fd_pair[4*fd+2];

            ull fd_off = td * thread_state * sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off;
            s->fd_off[1] = fd_off;
            _thread_CX(s);
            return;
        }

        if(isThread(q1)){
            int f = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            int t1 = td_pair[2*td];
            int t2 = td_pair[2*td+1];
            ull fd_off1 = t1*thread_state*sizeof(Type);
            ull fd_off2 = t2*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off2;
            s->fd_off[1] = fd_off1;
            _thread_CX(s);
            return;
        }

        if(isMiddle(q1)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];

            ull fd_off1 = td*thread_state*sizeof(Type);
            ull fd_off2 = fd_off1 + half_small_offset*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off2;
            s->fd_off[1] = fd_off1;
            _thread_CX2(s);
            return;
        }

        if(isLocal(q1)){
            int f = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            int fd1 = fd_pair[2*f];
            int fd2 = fd_pair[2*f+1];
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd1];
            s->fd[1] = fd_arr[fd2];
            s->fd_off[0] = fd_off;
            s->fd_off[1] = fd_off;
            _thread_CX(s);
            return;
        }
    }

    if(isThread(q0)){ 
        if(isThread(q1)){
            int fd = t/(num_thread_per_file/4);
            int td = t%(num_thread_per_file/4);
            int t1 = td_pair[4*td+1];
            int t2 = td_pair[4*td+2];
            ull t1_off = (t1*thread_state)*sizeof(Type);
            ull t2_off = (t2*thread_state)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX(s);
            return;
        }

        if(isMiddle(q1)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);

            int t0 = td_pair[2*td];
            int t2 = td_pair[2*td+1];

            ull t1_off = (t0*thread_state + half_small_offset) * sizeof(Type);
            ull t2_off = (t2*thread_state) * sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;
            _thread_CX2(s);
            return;
        }

        if(isLocal(q1)){
            int fd = t/(num_thread_per_file/2);
            int td = t%(num_thread_per_file/2);
            int t0 = td_pair[2*td];
            int t1 = td_pair[2*td+1];
            ull t0_off = (t0*thread_state)*sizeof(Type);
            ull t1_off = (t1*thread_state)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t0_off;
            s->fd_off[1] = t1_off;
            _thread_CX(s);
            return;
        }
    }

    if(isMiddle(q0)){
        if(isMiddle(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            ull t1_off = (td*thread_state + half_small_offset)*sizeof(Type);
            ull t2_off = (td*thread_state + half_large_offset)*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t1_off;
            s->fd_off[1] = t2_off;

            for (ull i = 0; i < thread_state; i += large_offset){
                _thread_CX2(s);
                s->fd_off[0] += half_large_offset*sizeof(Type);
                s->fd_off[1] += half_large_offset*sizeof(Type);
            }

            return;
        }

        if(isLocal(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;

            ull t0_off = td * thread_state * sizeof(Type);
            ull t1_off = t0_off + half_large_offset*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd[1] = fd_arr[fd];
            s->fd_off[0] = t0_off;
            s->fd_off[1] = t1_off;
            _thread_CX2(s);
            return;
        }
    }

    if (isLocal(q0)){
        if(isLocal(q1)){
            int fd = t/num_thread_per_file;
            int td = t%num_thread_per_file;
            ull fd_off = td*thread_state*sizeof(Type);

            s->fd[0] = fd_arr[fd];
            s->fd_off[0] = fd_off;
            _thread_CX(s);
            return;
        }
    }

    // shouldn't be here, previous states has been saved.
    printf("error gate instruction: should not be here.\n");
    exit(0);
}

void unitary8x8(int q0, int q1, int q2, int density){
    /*----------------------------
    Setting up global variables.
    ----------------------------*/

    int t = omp_get_thread_num();
    setStreamv2 *s = &thread_settings[t];

    if(t == 0){
        assert(q0 < q1);
        assert(q1 < q2);

        large_offset = 1ULL << (N-q0);
        middle_offset = 1ULL << (N-q1);
        small_offset = 1ULL << (N-q2);
        half_large_offset = large_offset >> 1;
        half_middle_offset = middle_offset >> 1;
        half_small_offset = small_offset >> 1;

        gate_func = gate_ops[15][density];

        /*  inner loop view:
            q0 = global
            q1/q2    global  thread  middle  local
            global   8       8       8       4
            thread           8       8       4
            middle                   8       4
            local                            2

            q0 = thread
            q1/q2    global  thread  middle  local
            global
            thread           8       8       4
            middle                   8       4
            local                            2

            q0 = middle
            q1/q2    global  thread  middle  local
            global
            thread
            middle                   8       4
            local                            2

            q0 = local
            q1/q2    global  thread  middle  local
            global
            thread
            middle
            local                            1
        */
        
        if (isLocal(q0)){ // then q1, q2 are local, too
            inner_loop_func = inner_loop;
            gate_size = chunk_state;
        }
        else if(isLocal(q1)){ // then q0 not local
            inner_loop_func = inner_loop2;
            gate_size = 2 * chunk_state;
        }
        else if(isLocal(q2)){ // then q0, q1 are not local
            inner_loop_func = inner_loop4;
            gate_size = 4 * chunk_state;
        }
        else{ // q0 q1 q3 all not local
            inner_loop_func = inner_loop8;
            gate_size = 8 * chunk_state;
        }

        /*  loop-size view:
            q0 = global
            q1/q2    global  thread  middle  local
            global   td_sz   td_sz   s_off   td_sz
            thread           td_sz   s_off   td_sz
            middle                   s_off   m_off
            local                            td_sz

            q0 = thread
            q1/q2    global  thread  middle  local
            global
            thread           td_sz   s_off   td_sz
            middle                   s_off   m_off
            local                            td_sz

            q0 = middle
            q1/q2    global  thread  middle  local
            global
            thread
            middle                   s_off   m_off
            local                            l_off

            q0 = local
            q1/q2    global  thread  middle  local
            global
            thread
            middle
            local                            td_sz

            td_sz: thread_state
            l_off: half_large_offset
            m_off: half_middle_offset
            s_off: half_small_offset
        */

        loop_size = thread_state;
        _outer = thread_state;
        if(isMiddle(q2)){
            loop_size = small_offset;
            set_outer(small_offset);
        }
        else if(isMiddle(q1)){
            loop_size = middle_offset;
            set_outer(middle_offset);
        }
        else if(isMiddle(q0)){
            loop_size = large_offset;
            set_outer(large_offset);
        }

        /*  make_work_pair view:
            fd_pair
            all  q0/q1/q2 global: gw012
            both q0/q1    global: gg01
            only q0       global: gt0

            td_pair
            all  q0/q1/q2 thread: sw012
            2 of q0/q1/q2 thread: sg01 sg02
            1 of q0/q1/q2 thread: st0 st1 st2

            gw012: file_segment, q0, q1, q2, fd_pair
        // make_work_warp(file_segment, q0, q1, q2, fd_pair);

            gg01: file_segment, q0, q1, fd_pair
        // make_work_group(file_segment, q0, q1, fd_pair);

            gt0: file_segment, q0, fd_pair
        // make_targ_pair(file_segment, q0, fd_pair);

            sw012: para_segment, q0-file_segment, q1-file_segment, q2-file_segment, td_pair
        // make_work_warp(para_segment, q0-file_segment, q1-file_segment, q2-file_segment, td_pair);

            sgxy: para_segment, qx-file_segment, qy-file_segment, td_pair
        // make_work_group(para_segment, qx-file_segment, qy-file_segment, td_pair);
 
            stx: para_segment, qx-file_segment, td_pair
        // make_targ_pair(para_segment, qx-file_segment, td_pair);
        */

        if(isGlobal(q2))
            make_work_warp(file_segment, q0, q1, q2, fd_pair);
        else if(isGlobal(q1)){
            make_work_group(file_segment, q0, q1, fd_pair);
            if(isThread(q2))
                make_targ_pair(para_segment, q2-file_segment, td_pair);
        }
        else if(isGlobal(q0)){ // only q0 global
            make_targ_pair(file_segment, q0, fd_pair);
            if(isThread(q2)){
                make_work_group(para_segment, q1-file_segment, q2-file_segment, td_pair);
            }
            else if(isThread(q1)){// only q1 thread
                make_targ_pair(para_segment, q1-file_segment, td_pair);
            }
        }
        else if(isThread(q2))
            make_work_warp(para_segment, q0-file_segment, q1-file_segment, q2-file_segment, td_pair);
        else if(isThread(q1)) // q0, q1 thread
            make_work_group(para_segment, q0-file_segment, q1-file_segment, td_pair);
        else if(isThread(q0)) // only q0 thread
            make_targ_pair(para_segment, q0-file_segment, td_pair);

        /*  gate_move view:
            q0 = global
            q1/q2    global  thread  middle  local
            global   842c    842c    842c    42cs
            thread           842c    842c    42cs
            middle                   842c    42cs
            local                            2cms

            q0 = thread
            q1/q2    global  thread  middle  local
            global
            thread           842c    842c    42cs
            middle                   842c    42cs
            local                            2cms

            q0 = middle
            q1/q2    global  thread  middle  local
            global
            thread
            middle                   842c    42cs
            local                            2cms

            q0 = local
            q1/q2    global  thread  middle  local
            global
            thread
            middle
            local                            lms

            842c:
            gate_move.large = 8*chunk_state;
            gate_move.middle = 4*chunk_state;
            gate_move.small = 2*chunk_state;
            gate_move.half_large = 4*chunk_state;
            gate_move.half_middle = 2*chunk_state;
            gate_move.half_small = chunk_state;

            42cs:
            gate_move.large = 4*chunk_state;
            gate_move.middle = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_large = 2*chunk_state;
            gate_move.half_middle = chunk_state;
            gate_move.half_small = half_small_offset;

            2cms:
            gate_move.large = 2*chunk_state;
            gate_move.middle = middle_offset;
            gate_move.small = small_offset;
            gate_move.half_large = chunk_state;
            gate_move.half_middle = half_middle_offset;
            gate_move.half_small = half_small_offset;

            lms:
            gate_move.large = large_offset;
            gate_move.middle = middle_offset;
            gate_move.small = small_offset;
            gate_move.half_large = half_large_offset;
            gate_move.half_middle = half_middle_offset;
            gate_move.half_small = half_small_offset;
        */

        if(isLocal(q0)){ // lms
            gate_move.large = large_offset;
            gate_move.middle = middle_offset;
            gate_move.small = small_offset;
            gate_move.half_large = half_large_offset;
            gate_move.half_middle = half_middle_offset;
            gate_move.half_small = half_small_offset;
        }
        else if(isLocal(q1)){ // 2cms
            gate_move.large = 2*chunk_state;
            gate_move.middle = middle_offset;
            gate_move.small = small_offset;
            gate_move.half_large = chunk_state;
            gate_move.half_middle = half_middle_offset;
            gate_move.half_small = half_small_offset;
        }
        else if(isLocal(q2)){ // 42cs
            gate_move.large = 4*chunk_state;
            gate_move.middle = 2*chunk_state;
            gate_move.small = small_offset;
            gate_move.half_large = 2*chunk_state;
            gate_move.half_middle = chunk_state;
            gate_move.half_small = half_small_offset;
        }
        else{ // 842c
            gate_move.large = 8*chunk_state;
            gate_move.middle = 4*chunk_state;
            gate_move.small = 2*chunk_state;
            gate_move.half_large = 4*chunk_state;
            gate_move.half_middle = 2*chunk_state;
            gate_move.half_small = chunk_state;
        }
    }

    #pragma omp barrier
    /*------------------------------
    Applying gate
    ------------------------------*/


    /*  thread num view
        q0 = global
        q1/q2    global  thread  middle  local
        global   E       E       Q       Q
        thread           E       Q       Q
        middle                   H       H
        local                            H

        q0 = thread
        q1/q2    global  thread  middle  local
        global
        thread           E       Q       Q
        middle                   H       H
        local                            H

        q0 = middle
        q1/q2    global  thread  middle  local
        global
        thread
        middle                   N       N
        local                            N

        q0 = local
        q1/q2    global  thread  middle  local
        global
        thread
        middle
        local                            N

        N: num_thread
        H: half_num_thread
        Q: quarter_num_thread
        E: eighth_num_thread
    */
    if(isGlobal(q0) || isThread(q0)){
        if(t >= half_num_thread) return;
        if(isGlobal(q1) || isThread(q1)){
            if(t >= quarter_num_thread) return;
            if(isGlobal(q2) || isThread(q2)){
                if(t >= eighth_num_thread) return;
            }
        }
    }

    if (isGlobal(q0)){
        if (isGlobal(q1)){
            if(isGlobal(q2)){
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                int fd0 = fd_pair[8*fd];
                int fd1 = fd_pair[8*fd+1];
                int fd2 = fd_pair[8*fd+2];
                int fd3 = fd_pair[8*fd+3];
                int fd4 = fd_pair[8*fd+4];
                int fd5 = fd_pair[8*fd+5];
                int fd6 = fd_pair[8*fd+6];
                int fd7 = fd_pair[8*fd+7];

                ull fd_off = td * thread_state * sizeof(Type);
                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = fd_off;
                s->fd[1] = fd_arr[fd1]; s->fd_off[1] = fd_off;
                s->fd[2] = fd_arr[fd2]; s->fd_off[2] = fd_off;
                s->fd[3] = fd_arr[fd3]; s->fd_off[3] = fd_off;
                s->fd[4] = fd_arr[fd4]; s->fd_off[4] = fd_off;
                s->fd[5] = fd_arr[fd5]; s->fd_off[5] = fd_off;
                s->fd[6] = fd_arr[fd6]; s->fd_off[6] = fd_off;
                s->fd[7] = fd_arr[fd7]; s->fd_off[7] = fd_off;
                _thread_CX(s);
                return;
            }

            if(isThread(q2)){
                int f = t/(num_thread_per_file/2);
                int td = t%(num_thread_per_file/2);
                int fd0 = fd_pair[4*f];
                int fd1 = fd_pair[4*f+1];
                int fd2 = fd_pair[4*f+2];
                int fd3 = fd_pair[4*f+3];
                int t1 = td_pair[2*td];
                int t2 = td_pair[2*td+1];
                ull fd_off1 = t1*thread_state*sizeof(Type);
                ull fd_off2 = t2*thread_state*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = fd_off1;
                s->fd[1] = fd_arr[fd0]; s->fd_off[1] = fd_off2;
                s->fd[2] = fd_arr[fd1]; s->fd_off[2] = fd_off1;
                s->fd[3] = fd_arr[fd1]; s->fd_off[3] = fd_off2;
                s->fd[4] = fd_arr[fd2]; s->fd_off[4] = fd_off1;
                s->fd[5] = fd_arr[fd2]; s->fd_off[5] = fd_off2;
                s->fd[6] = fd_arr[fd3]; s->fd_off[6] = fd_off1;
                s->fd[7] = fd_arr[fd3]; s->fd_off[7] = fd_off2;
                _thread_CX(s);
                return;
            }

            if(isMiddle(q2)){
                int f = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                int fd0 = fd_pair[4*f];
                int fd1 = fd_pair[4*f+1];
                int fd2 = fd_pair[4*f+2];
                int fd3 = fd_pair[4*f+3];

                ull fd_off1 = td*thread_state*sizeof(Type);
                ull fd_off2 = fd_off1 + half_small_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = fd_off1;
                s->fd[1] = fd_arr[fd0]; s->fd_off[1] = fd_off2;
                s->fd[2] = fd_arr[fd1]; s->fd_off[2] = fd_off1;
                s->fd[3] = fd_arr[fd1]; s->fd_off[3] = fd_off2;
                s->fd[4] = fd_arr[fd2]; s->fd_off[4] = fd_off1;
                s->fd[5] = fd_arr[fd2]; s->fd_off[5] = fd_off2;
                s->fd[6] = fd_arr[fd3]; s->fd_off[6] = fd_off1;
                s->fd[7] = fd_arr[fd3]; s->fd_off[7] = fd_off2;
                for (ull i = 0; i < thread_state; i += small_offset){
                    _thread_CX8(s);
                }
                return;
            }

            if(isLocal(q2)){
                int f = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                int fd0 = fd_pair[4*f];
                int fd1 = fd_pair[4*f+1];
                int fd2 = fd_pair[4*f+2];
                int fd3 = fd_pair[4*f+3];
                ull fd_off = td*thread_state*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = fd_off;
                s->fd[1] = fd_arr[fd1]; s->fd_off[1] = fd_off;
                s->fd[2] = fd_arr[fd2]; s->fd_off[2] = fd_off;
                s->fd[3] = fd_arr[fd3]; s->fd_off[3] = fd_off;
                _thread_CX(s);
                return;
            }
        }
    
        if(isThread(q1)){ 
            if(isThread(q2)){
                int fd = t/(num_thread_per_file/4);
                int td = t%(num_thread_per_file/4);
                int fd0 = fd_pair[2*fd];
                int fd1 = fd_pair[2*fd+1];
                int t0 = td_pair[4*td];
                int t1 = td_pair[4*td+1];
                int t2 = td_pair[4*td+2];
                int t3 = td_pair[4*td+3];
                ull t0_off = (t0*thread_state)*sizeof(Type);
                ull t1_off = (t1*thread_state)*sizeof(Type);
                ull t2_off = (t2*thread_state)*sizeof(Type);
                ull t3_off = (t3*thread_state)*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd0]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd0]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd0]; s->fd_off[3] = t3_off;
                s->fd[4] = fd_arr[fd1]; s->fd_off[4] = t0_off;
                s->fd[5] = fd_arr[fd1]; s->fd_off[5] = t1_off;
                s->fd[6] = fd_arr[fd1]; s->fd_off[6] = t2_off;
                s->fd[7] = fd_arr[fd1]; s->fd_off[7] = t3_off;
                _thread_CX(s);
                return;
            }

            if(isMiddle(q2)){
                int fd = t/(num_thread_per_file/2);
                int td = t%(num_thread_per_file/2);
                int fd0 = fd_pair[2*fd];
                int fd1 = fd_pair[2*fd+1];
                int t0 = td_pair[2*td];
                int t2 = td_pair[2*td+1];

                ull t0_off = t0*thread_state * sizeof(Type);
                ull t1_off = (t0*thread_state + half_small_offset) * sizeof(Type);
                ull t2_off = t2*thread_state * sizeof(Type);
                ull t3_off = (t2*thread_state + half_small_offset) * sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd0]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd0]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd0]; s->fd_off[3] = t3_off;
                s->fd[4] = fd_arr[fd1]; s->fd_off[4] = t0_off;
                s->fd[5] = fd_arr[fd1]; s->fd_off[5] = t1_off;
                s->fd[6] = fd_arr[fd1]; s->fd_off[6] = t2_off;
                s->fd[7] = fd_arr[fd1]; s->fd_off[7] = t3_off;
                for (ull i = 0; i < thread_state; i += small_offset){
                    _thread_CX8(s);
                }
                return;
            }

            if(isLocal(q2)){
                int fd = t/(num_thread_per_file/2);
                int td = t%(num_thread_per_file/2);
                int fd0 = fd_pair[2*fd];
                int fd1 = fd_pair[2*fd+1];
                int t0 = td_pair[2*td];
                int t1 = td_pair[2*td+1];
                ull t0_off = (t0*thread_state)*sizeof(Type);
                ull t1_off = (t1*thread_state)*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd0]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd1]; s->fd_off[2] = t0_off;
                s->fd[3] = fd_arr[fd1]; s->fd_off[3] = t1_off;
                _thread_CX(s);
                return;
            }
        }

        if(isMiddle(q1)){
            if(isMiddle(q2)){
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                int fd0 = fd_pair[2*fd];
                int fd1 = fd_pair[2*fd+1];
                ull t0_off = td*thread_state * sizeof(Type);
                ull t1_off = t0_off + half_small_offset*sizeof(Type);
                ull t2_off = t0_off + half_middle_offset*sizeof(Type);
                ull t3_off = t1_off + half_middle_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd0]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd0]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd0]; s->fd_off[3] = t3_off;
                s->fd[4] = fd_arr[fd1]; s->fd_off[4] = t0_off;
                s->fd[5] = fd_arr[fd1]; s->fd_off[5] = t1_off;
                s->fd[6] = fd_arr[fd1]; s->fd_off[6] = t2_off;
                s->fd[7] = fd_arr[fd1]; s->fd_off[7] = t3_off;

                for (ull i = 0; i < thread_state; i += middle_offset){
                    for (ull j = 0; j < half_middle_offset; j += small_offset){
                        _thread_CX8(s);
                    }
                    s->fd_off[0] += half_middle_offset*sizeof(Type);
                    s->fd_off[1] += half_middle_offset*sizeof(Type);
                    s->fd_off[2] += half_middle_offset*sizeof(Type);
                    s->fd_off[3] += half_middle_offset*sizeof(Type);
                    s->fd_off[4] += half_middle_offset*sizeof(Type);
                    s->fd_off[5] += half_middle_offset*sizeof(Type);
                    s->fd_off[6] += half_middle_offset*sizeof(Type);
                    s->fd_off[7] += half_middle_offset*sizeof(Type);
                }

                return;
            }

            if(isLocal(q2)){
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                int fd0 = fd_pair[2*fd];
                int fd1 = fd_pair[2*fd+1];

                ull t0_off = td * thread_state * sizeof(Type);
                ull t2_off = t0_off + half_middle_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd0]; s->fd_off[1] = t2_off;
                s->fd[2] = fd_arr[fd1]; s->fd_off[2] = t0_off;
                s->fd[3] = fd_arr[fd1]; s->fd_off[3] = t2_off;

                for (ull i = 0; i < thread_state; i += middle_offset){
                    _thread_CX4(s);
                }
                return;
            }
        }

        if (isLocal(q1)){
            if(isLocal(q2)){
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                int fd0 = fd_pair[2*fd];
                int fd1 = fd_pair[2*fd+1];
                ull fd_off = td*thread_state*sizeof(Type);

                s->fd[0] = fd_arr[fd0]; s->fd_off[0] = fd_off;
                s->fd[1] = fd_arr[fd1]; s->fd_off[1] = fd_off;
                _thread_CX(s);
                return;
            }
        }
    }

    if (isThread(q0)){
        if(isThread(q1)){ 
            if(isThread(q2)){
                int fd = t/(num_thread_per_file/8);
                int td = t%(num_thread_per_file/8);
                int t0 = td_pair[8*td];         ull t0_off = (t0*thread_state)*sizeof(Type);
                int t1 = td_pair[8*td + 1];     ull t1_off = (t1*thread_state)*sizeof(Type);
                int t2 = td_pair[8*td + 2];     ull t2_off = (t2*thread_state)*sizeof(Type);
                int t3 = td_pair[8*td + 3];     ull t3_off = (t3*thread_state)*sizeof(Type);
                int t4 = td_pair[8*td + 4];     ull t4_off = (t4*thread_state)*sizeof(Type);
                int t5 = td_pair[8*td + 5];     ull t5_off = (t5*thread_state)*sizeof(Type);
                int t6 = td_pair[8*td + 6];     ull t6_off = (t6*thread_state)*sizeof(Type);
                int t7 = td_pair[8*td + 7];     ull t7_off = (t7*thread_state)*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd]; s->fd_off[3] = t3_off;
                s->fd[4] = fd_arr[fd]; s->fd_off[4] = t4_off;
                s->fd[5] = fd_arr[fd]; s->fd_off[5] = t5_off;
                s->fd[6] = fd_arr[fd]; s->fd_off[6] = t6_off;
                s->fd[7] = fd_arr[fd]; s->fd_off[7] = t7_off;

                _thread_CX(s);
                return;
            }

            if(isMiddle(q2)){
                int fd = t/(num_thread_per_file/4);
                int td = t%(num_thread_per_file/4);
                int t0 = td_pair[4*td];
                int t1 = td_pair[4*td + 1];
                int t2 = td_pair[4*td + 2];
                int t3 = td_pair[4*td + 3];

                ull t0_off = t0*thread_state * sizeof(Type);
                ull t1_off = t0_off + half_small_offset * sizeof(Type);
                ull t2_off = t1*thread_state * sizeof(Type);
                ull t3_off = t2_off + half_small_offset * sizeof(Type);
                ull t4_off = t2*thread_state * sizeof(Type);
                ull t5_off = t4_off + half_small_offset * sizeof(Type);
                ull t6_off = t3*thread_state * sizeof(Type);
                ull t7_off = t6_off + half_small_offset * sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd]; s->fd_off[3] = t3_off;
                s->fd[4] = fd_arr[fd]; s->fd_off[4] = t4_off;
                s->fd[5] = fd_arr[fd]; s->fd_off[5] = t5_off;
                s->fd[6] = fd_arr[fd]; s->fd_off[6] = t6_off;
                s->fd[7] = fd_arr[fd]; s->fd_off[7] = t7_off;

                for (ull i = 0; i < thread_state; i += small_offset){
                    _thread_CX8(s);
                }
                return;
            }

            if(isLocal(q2)){
                int fd = t/(num_thread_per_file/4);
                int td = t%(num_thread_per_file/4);
                int t0 = td_pair[4*td];
                int t1 = td_pair[4*td + 1];
                int t2 = td_pair[4*td + 2];
                int t3 = td_pair[4*td + 3];
                ull t0_off = (t0*thread_state)*sizeof(Type);
                ull t1_off = (t1*thread_state)*sizeof(Type);
                ull t2_off = (t2*thread_state)*sizeof(Type);
                ull t3_off = (t3*thread_state)*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd]; s->fd_off[3] = t3_off;
                _thread_CX(s);
                return;
            }
        }

        if(isMiddle(q1)){
            if(isMiddle(q2)){
                int fd = t/(num_thread_per_file/2);
                int td = t%(num_thread_per_file/2);
                int t0 = td_pair[2*td];
                int t1 = td_pair[2*td + 1];
                ull t0_off = t0*thread_state * sizeof(Type);
                ull t1_off = t0_off + half_small_offset*sizeof(Type);
                ull t2_off = t0_off + half_middle_offset*sizeof(Type);
                ull t3_off = t2_off + half_small_offset*sizeof(Type);
                ull t4_off = t1*thread_state * sizeof(Type);
                ull t5_off = t4_off + half_small_offset*sizeof(Type);
                ull t6_off = t4_off + half_middle_offset*sizeof(Type);
                ull t7_off = t6_off + half_small_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd]; s->fd_off[3] = t3_off;
                s->fd[4] = fd_arr[fd]; s->fd_off[4] = t4_off;
                s->fd[5] = fd_arr[fd]; s->fd_off[5] = t5_off;
                s->fd[6] = fd_arr[fd]; s->fd_off[6] = t6_off;
                s->fd[7] = fd_arr[fd]; s->fd_off[7] = t7_off;

                for (ull i = 0; i < thread_state; i += middle_offset){
                    for (ull j = 0; j < half_middle_offset; j += small_offset){
                        _thread_CX8(s);
                    }
                    s->fd_off[0] += half_middle_offset*sizeof(Type);
                    s->fd_off[1] += half_middle_offset*sizeof(Type);
                    s->fd_off[2] += half_middle_offset*sizeof(Type);
                    s->fd_off[3] += half_middle_offset*sizeof(Type);
                    s->fd_off[4] += half_middle_offset*sizeof(Type);
                    s->fd_off[5] += half_middle_offset*sizeof(Type);
                    s->fd_off[6] += half_middle_offset*sizeof(Type);
                    s->fd_off[7] += half_middle_offset*sizeof(Type);
                }

                return;
            }

            if(isLocal(q2)){
                int fd = t/(num_thread_per_file/2);
                int td = t%(num_thread_per_file/2);
                int t0 = td_pair[2*td];
                int t1 = td_pair[2*td + 1];

                ull t0_off = t0 * thread_state * sizeof(Type);
                ull t1_off = t0_off + half_middle_offset*sizeof(Type);
                ull t2_off = t1 * thread_state * sizeof(Type);
                ull t3_off = t2_off + half_middle_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd]; s->fd_off[3] = t3_off;

                for (ull i = 0; i < thread_state; i += middle_offset){
                    _thread_CX4(s);
                }
                return;
            }
        }

        if (isLocal(q1)){
            if(isLocal(q2)){
                int fd = t/(num_thread_per_file/2);
                int td = t%(num_thread_per_file/2);
                int t0 = td_pair[2*td];
                int t1 = td_pair[2*td + 1];

                ull t0_off = t0*thread_state*sizeof(Type);
                ull t1_off = t1*thread_state*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                _thread_CX(s);
                return;
            }
        }
    }

    if (isMiddle(q0)){
        if(isMiddle(q1)){
            if(isMiddle(q2)){
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;

                ull t0_off = td*thread_state * sizeof(Type);
                ull t1_off = t0_off + half_small_offset*sizeof(Type);
                ull t2_off = t0_off + half_middle_offset*sizeof(Type);
                ull t3_off = t2_off + half_small_offset*sizeof(Type);
                ull t4_off = t0_off + half_large_offset * sizeof(Type);
                ull t5_off = t4_off + half_small_offset*sizeof(Type);
                ull t6_off = t4_off + half_middle_offset*sizeof(Type);
                ull t7_off = t6_off + half_small_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd]; s->fd_off[3] = t3_off;
                s->fd[4] = fd_arr[fd]; s->fd_off[4] = t4_off;
                s->fd[5] = fd_arr[fd]; s->fd_off[5] = t5_off;
                s->fd[6] = fd_arr[fd]; s->fd_off[6] = t6_off;
                s->fd[7] = fd_arr[fd]; s->fd_off[7] = t7_off;

                for (ull i = 0; i < thread_state; i += large_offset){
                    for (ull j = 0; j < half_large_offset; j += middle_offset){
                        for (ull k = 0; k < half_middle_offset; k += small_offset){
                            _thread_CX8(s);
                        }
                        s->fd_off[0] += half_middle_offset*sizeof(Type);
                        s->fd_off[1] += half_middle_offset*sizeof(Type);
                        s->fd_off[2] += half_middle_offset*sizeof(Type);
                        s->fd_off[3] += half_middle_offset*sizeof(Type);
                        s->fd_off[4] += half_middle_offset*sizeof(Type);
                        s->fd_off[5] += half_middle_offset*sizeof(Type);
                        s->fd_off[6] += half_middle_offset*sizeof(Type);
                        s->fd_off[7] += half_middle_offset*sizeof(Type);
                    }
                    s->fd_off[0] += half_large_offset*sizeof(Type);
                    s->fd_off[1] += half_large_offset*sizeof(Type);
                    s->fd_off[2] += half_large_offset*sizeof(Type);
                    s->fd_off[3] += half_large_offset*sizeof(Type);
                    s->fd_off[4] += half_large_offset*sizeof(Type);
                    s->fd_off[5] += half_large_offset*sizeof(Type);
                    s->fd_off[6] += half_large_offset*sizeof(Type);
                    s->fd_off[7] += half_large_offset*sizeof(Type);
                }

                return;
            }

            if(isLocal(q2)){
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                
                ull t0_off = td*thread_state * sizeof(Type);
                ull t1_off = t0_off + half_middle_offset*sizeof(Type);
                ull t2_off = t0_off + half_large_offset*sizeof(Type);
                ull t3_off = t2_off + half_middle_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;
                s->fd[2] = fd_arr[fd]; s->fd_off[2] = t2_off;
                s->fd[3] = fd_arr[fd]; s->fd_off[3] = t3_off;

                for (ull i = 0; i < thread_state; i += large_offset){
                    for (ull j = 0; j < half_large_offset; j += middle_offset){
                        _thread_CX4(s);
                    }
                    s->fd_off[0] += half_large_offset*sizeof(Type);
                    s->fd_off[1] += half_large_offset*sizeof(Type);
                    s->fd_off[2] += half_large_offset*sizeof(Type);
                    s->fd_off[3] += half_large_offset*sizeof(Type);
                }
                return;
            }
        }

        if (isLocal(q1)){
            if(isLocal(q2)){
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;

                ull t0_off = td*thread_state*sizeof(Type);
                ull t1_off = t0_off + half_large_offset*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = t0_off;
                s->fd[1] = fd_arr[fd]; s->fd_off[1] = t1_off;

                for (ull i = 0; i < thread_state; i += large_offset){
                    _thread_CX2(s);
                }
                return;
            }
        }
    }

    if (isLocal(q0)){
        if (isLocal(q1)){
            if (isLocal(q2)){
                // if(t == 0) printf("[U3]: inside LLL\n");
                fflush(stdout);
                int fd = t/num_thread_per_file;
                int td = t%num_thread_per_file;
                ull fd_off = td*thread_state*sizeof(Type);

                s->fd[0] = fd_arr[fd]; s->fd_off[0] = fd_off;
                _thread_CX(s);
                return;
            }
        }
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
