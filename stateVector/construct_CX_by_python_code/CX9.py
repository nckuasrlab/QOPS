from qiskit import Aer, QuantumCircuit, transpile
from math import sqrt
import numpy as np

N = 8
NGQB = 2
NSQB = 4
NLQB = 2
# NSQB >= NGQB 確保一個file至少有一個thread處理
# NSQB + NLQB <= N 確保不會有多個thread處理到同一個chunk

POW2N = 1 << N
CHUNKSIZE = 1 << NLQB
HALF_CHUNKSIZE = CHUNKSIZE >> 1
FILENUMBER = 1 << NGQB
FILESIZE = 1 << (N-NGQB)

fd_arr = []

# 0 < target < NGQB
def isGlobal(target):
    return target < NGQB

# NGQB <= target < NSQB
def isThread(target):
    return target >= NGQB and target < (NSQB)

# NSQB <= target < N-NLQB
def isMiddle(target):
    return target >= NSQB and target < (N-NLQB)

# N-NLQB <= target < N
def isLocal(target):
    return target >= (N-NLQB)

def print_state(state):
    print("state")
    for i in range(POW2N):
        if(state[i] > 1e-9):
            print(i, state[i])
    print()

def print_state_f(fd_arr):
    print("fd_arr")
    for i in range(POW2N):
        if(fd_arr[i//FILESIZE][i%FILESIZE] > 1e-9):
            print(i, fd_arr[i//FILESIZE][i%FILESIZE])
    print()

# init to 0 state
def circ_init():
    circ = QuantumCircuit(N)
    initial_state = [1,0]   # Define initial_state as |0>
    circ.initialize(initial_state, 0)
    return circ

# dump state with given circuit
def qiskit_init(circ):
    simulator = Aer.get_backend('aer_simulator_statevector')
    circ.save_statevector(label=f'save')
    circ = transpile(circ, simulator)
    data = simulator.run(circ).result().data(0)
    return data['save']

#init to the state after apply H on qubit 0 over |0>
def init_state(state):
    fd_arr = [np.zeros(FILESIZE, dtype=np.complex128) for i in range(1 << NGQB)]
    for fd in range(FILENUMBER):
        fd_off = fd*FILESIZE
        for i in range(FILESIZE):
            fd_arr[fd][i] = state[fd_off+i]
    return fd_arr

#swap(state[a], state[b])
def swap(state, a, b):
    temp = state[a]
    state[a] = state[b]
    state[b] = temp

# x_gate on chunk
def x_gate(chunk:list, move:int):
    for i in range(0, len(chunk), move*2):
        up = i
        lo = i+move
        for j in range(move):
            swap(chunk, up+j, lo+j)
    return chunk

def x2_gate(chunk:list, move:list):
    # outer_move:int, inner_move:int, half_ctrl_offset inside chunk:int, half_targ_offset inside chunk:int
    # print(move)
    base = move[2]
    for i in range(0, len(chunk), move[0]):
        for j in range(0, move[0]>>1, move[1]):
            up = base
            lo = base + move[3]
            for k in range(move[1]>>1):
                # print(up, lo)
                swap(chunk, up, lo)
                up += 1
                lo += 1
            base += move[1]
        base += move[0] >> 1
    return chunk

# i從左數第ctrl個bit是否為1 全長為n
def isCtrl(i, ctrl, n):
    ctrl_mask = 1 << (n-ctrl-1)
    return True if i&ctrl_mask > 0 else False

# i從左數第targ個bit對應的數字 全長為n
def iPair(i, targ, n):
    targ_mask = 1 << (n-targ-1)
    return i^targ_mask

def make_work_pair(size:int, ctrl:int, targ:int)->list:
    pair = []
    
    checklist = [False for i in range(1<<size)]
    for i in range(1<<size):
        if(checklist[i] == True):
            continue
        checklist[i] = True
        if (ctrl < 0 and targ >= 0):
            checklist[iPair(i, targ, size)] = True
            pair.append([i,iPair(i, targ, size)])
        elif (isCtrl(i, ctrl, size)):
            if targ < 0:
                pair.append([i, i])
            else:
                checklist[iPair(i, targ, size)] = True
                pair.append([i, iPair(i, targ, size)])
    for p in pair:
        p.sort()
    # print(pair)
    return pair

    
#CX gate with state as a state vector and 0~N-1 endian
def CX(ctrl, targ, fd_arr, gate_func):
    if ctrl == targ:
        print("illegal ops.")
        return

    def read_from_fd_to_work(work:list, fd:int, fd_off:int):
        for k in range(CHUNKSIZE):
            work.append(fd_arr[fd][fd_off+k])
        return work

    def write_from_work_to_fd(work:list, fd:int, fd_off:int, w_off:int = 0):
        for k in range(CHUNKSIZE):
            fd_arr[fd][fd_off+k] = work[w_off+k]

    def inner_loop(size, fd, fd_off, func_arg):
        for j in range(0, size, CHUNKSIZE):
                work = []
                work = read_from_fd_to_work(work, fd, fd_off)
                work = gate_func(work, func_arg)
                write_from_work_to_fd(work, fd, fd_off, 0)
                fd_off += CHUNKSIZE

    def inner_loop2(size, fd1, fd1_off, fd2, fd2_off, func_arg):
        for j in range(0, size, CHUNKSIZE):
                work = []
                work = read_from_fd_to_work(work, fd1, fd1_off)
                work = read_from_fd_to_work(work, fd2, fd2_off)
                work = gate_func(work, func_arg)
                write_from_work_to_fd(work, fd1, fd1_off, 0)
                write_from_work_to_fd(work, fd2, fd2_off, CHUNKSIZE)
                fd1_off += CHUNKSIZE
                fd2_off += CHUNKSIZE
    
    ctrl_offset = 1 << (N-ctrl)
    half_ctrl_offset = ctrl_offset >> 1
    targ_offset = 1 << (N-targ)
    half_targ_offset = targ_offset >> 1

    up_qubit = ctrl if ctrl < targ else targ    
    lo_qubit = ctrl if ctrl > targ else targ

    small_offset = 1 << (N-lo_qubit)
    half_small_offset = small_offset >> 1
    large_offset = 1 << (N-up_qubit)
    half_large_offset = large_offset >> 1

    threads_per_file = (1<<(NSQB-NGQB))
    thread_size = 1 << (N-NSQB)

    # file-wise
    # half of files doesn't work since ctrl == 0
    if isGlobal(ctrl) and isGlobal(targ): # file-wise
        fd_pair = make_work_pair(NGQB, ctrl, targ)
        
        # [Thread level parallel] here
        for [fd1, fd2] in fd_pair:
            inner_loop2(FILESIZE, fd1, 0, fd2, 0, CHUNKSIZE)
        return

    # can parallel over all files, but doubled the work since the granularity.
    if isLocal(ctrl) and isLocal(targ):
        # [Thread level parallel] here
        for fd in range(FILENUMBER):
            inner_loop(FILESIZE, fd, 0, [large_offset, small_offset, half_ctrl_offset, half_targ_offset])
        return

    # one-side global
    if isGlobal(up_qubit):
        # file pair construct
        if (ctrl < targ):
            fd_pair = make_work_pair(NGQB, ctrl, -1)
        else:
            fd_pair = make_work_pair(NGQB, -1, targ)

        if isLocal(lo_qubit):
            # ctrl-Global targ-Local, fd_pair[0][0] == fd_pair[0][1]:
            if (ctrl < targ): 
                # [Thread level parallel] here
                for [fd, _] in fd_pair:
                    inner_loop(FILESIZE, fd, 0, half_small_offset)
                return

            # targ-Global ctrl-Local
            # [Thread level parallel] here
            for [fd1, fd2] in fd_pair:
                inner_loop2(FILESIZE, fd1, 0, fd2, 0, [2*CHUNKSIZE, small_offset, half_small_offset, CHUNKSIZE])
            return

        if isThread(lo_qubit):# Global-Thread
            # thread pair construct
            if(ctrl < targ):
                thread_pair = make_work_pair(NSQB-NGQB, -1, targ-NGQB)
            else:   
                thread_pair = make_work_pair(NSQB-NGQB, ctrl-NGQB, -1)

            # only use half of all threads
            threads_per_file = (1<<(NSQB-NGQB))
            thread_size = 1 << (N-NSQB)
            # [Thread level parallel] here
            for [fd1, fd2] in fd_pair:
                for [t1, t2] in thread_pair:
                    t1_off = t1 * thread_size # equal to t1 << (N-NSQB)
                    t2_off = t2 * thread_size # equal to t2 << (N-NSQB)
                    inner_loop2(thread_size, fd1, t1_off, fd2, t2_off, CHUNKSIZE)
            return
            
        # Global-Middle
        if (ctrl < targ): # ctrl-Global targ-Middle
            for [fd1, fd2] in fd_pair: # fd1 must equal to fd2
                for t in range(threads_per_file):
                    t1_off = t * thread_size # equal to t << (N-NSQB)
                    t2_off = t1_off + half_small_offset
                    #這邊可能在其他gate要注意|0> |1>的順序
                    #x gate 沒差
                    for i in range(0, thread_size, small_offset):
                        inner_loop2(half_small_offset, fd1, t1_off, fd2, t2_off, CHUNKSIZE)
                        t1_off += small_offset
                        t2_off += small_offset
            return
                        
        # targ-Global ctrl-Middle
        for [fd1, fd2] in fd_pair: # fd1 != fd2
            for t in range(threads_per_file):
                t_off = t * thread_size # equal to t << (N-NSQB)
                #這邊可能在其他gate要注意|0> |1>的順序
                #x gate 沒差
                t_off += half_small_offset
                for i in range(0, thread_size, small_offset):
                    inner_loop2(half_small_offset, fd1, t_off, fd2, t_off, CHUNKSIZE)
                    t_off += small_offset
        return

    if isThread(up_qubit):
        if isThread(lo_qubit):
            # only use half of all threads
            # thread pair construct
            thread_pair = make_work_pair(NSQB-NGQB, ctrl-NGQB, targ-NGQB)
            
            for fd in range(FILENUMBER):
                for [t1, t2] in thread_pair:
                    t1_off = t1 * thread_size
                    t2_off = t2 * thread_size
                    inner_loop2(thread_size, fd, t1_off, fd, t2_off, CHUNKSIZE)
            return

        if (ctrl < targ):
            thread_pair = make_work_pair(NSQB-NGQB, ctrl-NGQB, -1)
        else:
            thread_pair = make_work_pair(NSQB-NGQB, -1, targ-NGQB)

        if isLocal(lo_qubit):
            if (ctrl < targ): # ctrl-Thread targ-Local 
                for fd in range(FILENUMBER):
                    for [t1, t2] in thread_pair: # t1==t2
                        t_off = t1 * thread_size
                        inner_loop(thread_size, fd, t_off, half_small_offset)
                return

            # targ-Thread ctrl-Local
            for fd in range(FILENUMBER):
                for [t1, t2] in thread_pair:
                    t1_off = t1 * thread_size
                    t2_off = t2 * thread_size
                    inner_loop2(thread_size, fd, t1_off, fd, t2_off, [2*CHUNKSIZE, small_offset, half_ctrl_offset, CHUNKSIZE])
            return

        # isMiddle(lo_qubit) == True
        # ctrl-thread targ-middle
        if ctrl < targ: 
            for fd in range(FILENUMBER):
                for [t1, t2] in thread_pair: # t1 == t2
                    t1_off = t1 * thread_size
                    t2_off = t1 * thread_size + half_targ_offset
                    for i in range(0, thread_size, small_offset):
                        inner_loop2(half_small_offset, fd, t1_off, fd, t2_off, CHUNKSIZE)
                        t1_off += small_offset
                        t2_off += small_offset
            return

        # targ-thread ctrl-middle
        for fd in range(FILENUMBER):
            for [t1, t2] in thread_pair: # t1 != t2
                t1_off = t1 * thread_size + half_ctrl_offset
                t2_off = t2 * thread_size + half_ctrl_offset
                
                for i in range(0, thread_size, small_offset):
                    inner_loop2(half_small_offset, fd, t1_off, fd, t2_off, CHUNKSIZE)
                    t1_off += small_offset
                    t2_off += small_offset
        return

    if isMiddle(up_qubit):
        if isMiddle(lo_qubit): # Middle-Middle
            
            for fd in range(FILENUMBER):
                for t in range(threads_per_file):
                    t1_off = t * thread_size + half_ctrl_offset
                    t2_off = t1_off + half_targ_offset
                    for i in range(0, thread_size, large_offset):
                        for j in range(0, half_large_offset, small_offset):
                            inner_loop2(half_small_offset, fd, t1_off, fd, t2_off, CHUNKSIZE)
                            t1_off += small_offset
                            t2_off += small_offset
                        t1_off += half_large_offset
                        t2_off += half_large_offset
            return
        # Middle-Local
        if ctrl < targ: #ctrl-Middle targ-Local
            for fd in range(FILENUMBER):
                for t in range(threads_per_file):
                    t_off = t * thread_size + half_ctrl_offset
                    for i in range(0, thread_size, large_offset):
                        inner_loop(half_large_offset, fd, t_off, half_targ_offset)
                        t_off += large_offset
            return
        #targ-Middle ctrl-Local
        for fd in range(FILENUMBER):
            for t in range(threads_per_file):
                m1_off = t * thread_size
                m2_off = m1_off + half_targ_offset
                for i in range(0, thread_size, large_offset):
                    inner_loop2(half_large_offset, fd, m1_off, fd, m2_off, [2*CHUNKSIZE, ctrl_offset, half_ctrl_offset, CHUNKSIZE])
                    m1_off += large_offset
                    m2_off += large_offset
        return 

# translate to Qiskit order or vice versa.
def reorder(qubit):
    return -qubit+N-1

def check(fd_arr, state):
    # qstate = data[f'CX_{ctrl:02d}-{targ:02d}']
    # print(qstate[0], state[0])
    state = np.array(state)

    flag = True
    # for i in range(2**N):
    #     if (np.abs(state[i]-fd_arr[i//FILESIZE][i%FILESIZE]) > 1e-9):
    for i in range(FILENUMBER):
        if (not np.alltrue(np.abs(state[i*FILESIZE:(i+1)*FILESIZE]-fd_arr[i]) < 1e-9)):

            # print order: 應該有的state, 本模擬器的state
            # print(i, state[i], fd_arr[i//FILESIZE][i%FILESIZE])
            flag = False
    return flag
    # print(np.array_equal(qstate, state))



# state = np.zeros(2**N, dtype=np.complex128)
# print("|0> state:", state)
# init(state)
# print("H|0> state:", state)

def test(name:str, c_range, t_range, gate_func, h_list):
    flag = True
    for ctrl in c_range:
        for targ in t_range:
            if(ctrl == targ): continue
            # print(f"ctrl={ctrl}, targ={targ}")
            circ = circ_init()
            for q in h_list:
                circ.h(reorder(q))
            state = qiskit_init(circ)
            fd_arr = init_state(state)

            
            # if(not check(fd_arr, state)):
            #     print(f"ctrl={ctrl}, targ={targ}")
            #     print("test not pass")
            #     flag = False
            
            # continue
            CX(ctrl, targ, fd_arr, gate_func)

            circ = circ_init()
            for q in h_list:
                circ.h(reorder(q))
            circ.cx(reorder(ctrl), reorder(targ))
            state = qiskit_init(circ)

            if(not check(fd_arr, state)):
                print(f"ctrl={ctrl}, targ={targ}")
                print("test not pass")
                print()
                flag = False

    if(flag):
        print("[pass]", name, ": match with qiskit under 1e-9", flush=True)
    else:
        # print_state_f(fd_arr)
        # print_state(state)
        print("[x]", name, "not pass", flush=True)

print(f"NGQB = {NGQB}, #FILE = {FILENUMBER}, FILESIZE = {FILESIZE}")
print(f"NSQB = {NSQB}, #Thread = {1<<NSQB}")
print(f"middle = {[i for i in range(NSQB, N-NLQB)]}")
print(f"NLQB = {NLQB}, CHUNKSIZE = {CHUNKSIZE}")


for h in range(N):
    print("===========================")
    h_list = []
    for q in range(N):
        if q!=h :
            h_list.append(q)
    print(f"h skip on {h}, h_list = {h_list}")
    test("Global test", range(NGQB), range(NGQB), x_gate, h_list)
    test("Local test", range(N-NLQB, N), range(N-NLQB, N), x2_gate, h_list)
    
    test("Ctrl-Global Targ-Thread test", range(NGQB), range(NGQB, NSQB), x_gate, h_list)
    test("Targ-Global Ctrl-Thread test", range(NGQB, NSQB), range(NGQB), x_gate, h_list)
    test("Ctrl-Global Targ-Middle test", range(NGQB), range(NSQB, N-NLQB), x_gate, h_list)
    test("Targ-Global Ctrl-Middle test", range(NSQB, N-NLQB), range(NGQB), x_gate, h_list)
    test("Ctrl-Global Targ-Local test", range(NGQB), range(N-NLQB, N), x_gate, h_list)
    test("Targ-Global Ctrl-Local test", range(N-NLQB, N), range(NGQB), x2_gate, h_list)
    
    test("Thread test", range(NGQB, NSQB), range(NGQB, NSQB), x_gate, h_list)
    test("Ctrl-Thread Targ-Middle test", range(NGQB, NSQB), range(NSQB, N-NLQB), x_gate, h_list)
    test("Targ-Thread Ctrl-Middle test", range(NSQB, N-NLQB), range(NGQB, NSQB), x_gate, h_list)
    test("Ctrl-Thread Targ-Local test", range(NGQB, NSQB), range(N-NLQB, N), x_gate, h_list)
    test("Targ-Thread Ctrl-Local test", range(N-NLQB, N), range(NGQB, NSQB), x2_gate, h_list)

    test("Middle test", range(NSQB, N-NLQB), range(NSQB, N-NLQB), x_gate, h_list)
    test("Ctrl-Middle Targ-Local test", range(NSQB, N-NLQB), range(N-NLQB, N), x_gate, h_list)
    test("Targ-Middle Ctrl-Local test", range(N-NLQB, N), range(NSQB, N-NLQB), x2_gate, h_list)
    print("===========================")
    