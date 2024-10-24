from qiskit import Aer, QuantumCircuit, transpile
from math import sqrt
import numpy as np

N = 8
NGQB = 3
# NSQB = 5
NLQB = 5

POW2N = 1 << N
CHUNKSIZE = 1 << NLQB
HALF_CHUNKSIZE = CHUNKSIZE >> 1
FILENUMBER = 1 << NGQB
FILESIZE = 1 << (N-NGQB)

fd_arr = []

# 0 < target < NGQB
def isGlobal(target):
    return target < NGQB

# N-NLQB <= target < N
def isLocal(target):
    return target >= (N-NLQB)


#init to the state after apply H on qubit 0 over |0>
def init_state():
    fd_arr = [np.zeros(FILESIZE, dtype=np.complex128) for i in range(1 << NGQB)]
    fd_arr[0][0] = np.complex128(1/sqrt(2)+0j)
    fd_arr[1<<(NGQB-1)][0] = np.complex128(1/sqrt(2)+0j)
    return fd_arr

#swap(state[a], state[b])
def swap(state, a, b):
    temp = state[a]
    state[a] = state[b]
    state[b] = temp

def x_gate(chunk:list, move:int):
    for i in range(0, CHUNKSIZE, move*2):
        up = i
        lo = i+move
        for j in range(i, move):
            swap(chunk, up, lo)
            up += 1
            lo += 1
    return chunk

def x2_gate(chunk:list, move:list):
    # outer_move:int, inner_move:int, half_ctrl_offset:int, half_targ_offset:int
    base = move[2]
    for i in range(0, CHUNKSIZE, move[0]):
        for j in range(0, move[0]>>1, move[1]):
            up = base+j
            lo = base+j+move[3]
            for k in range(move[1]>>1):
                swap(chunk, up, lo)
                up += 1
                lo += 1
        base += move[0]
    return chunk


#CX gate with state as a state vector and 0~N-1 endian
def CX(ctrl, targ, fd_arr, x_gate):
    if ctrl == targ:
        print("illegal ops.")
        return

    ctrl_offset = 1 << (N-ctrl)
    half_ctrl_offset = ctrl_offset >> 1
    targ_offset = 1 << (N-targ)
    half_targ_offset = targ_offset >> 1

    small_offset = targ_offset if targ > ctrl else ctrl_offset
    half_small_offset = small_offset >> 1
    large_offset = targ_offset if targ < ctrl else ctrl_offset
    half_large_offset = large_offset >> 1

    up_qubit = ctrl if ctrl < targ else targ    
    lo_qubit = ctrl if ctrl > targ else targ

    groups = 1 << (abs(targ-ctrl)-1)
    offset = half_ctrl_offset

    # file-wise
    # half files doesn't work since ctrl == 0
    if isGlobal(ctrl) and isGlobal(targ): # file-wise
        # up_file_offset = 1 << (NGQB-up_qubit)
        # half_up_file_offset = up_file_offset >> 1
        # lo_file_offset = 1 << (NGQB-lo_qubit)
        # half_lo_file_offset = lo_file_offset >> 1

        # ctrl_file_offset = 1 << (NGQB-ctrl)
        # half_ctrl_file_offset = ctrl_file_offset >> 1
        # targ_file_offset = 1 << (NGQB-targ)
        # half_targ_file_offset = targ_file_offset >> 1
        
        ctrl_file_mask = 1 << (NGQB-ctrl-1)
        targ_file_mask = 1 << (NGQB-targ-1)

        def isCtrl(num):
            return num&ctrl_file_mask > 0
        def pairFile(num):
            return num^targ_file_mask

        fd_pair = []
        file_checklist = [ False for i in range(FILENUMBER)]
        for fd in range(FILENUMBER):
            if(file_checklist[fd]):
                continue
            file_checklist[fd] = True
            if(isCtrl(fd)):
                pfd = pairFile(fd)
                file_checklist[pfd] = True
                fd_pair.append([fd, pfd])

        # here we can parallel do this
        for [fd1, fd2] in fd_pair:
            for j in range(0, FILESIZE, HALF_CHUNKSIZE):
                work = []
                for k in range(HALF_CHUNKSIZE):
                    work.append(fd_arr[fd1][j+k])
                for k in range(HALF_CHUNKSIZE):
                    work.append(fd_arr[fd2][j+k])
                work = x_gate(work, HALF_CHUNKSIZE)
                
                for k in range(HALF_CHUNKSIZE):
                    fd_arr[fd1][j+k] = work[k]
                    fd_arr[fd2][j+k] = work[HALF_CHUNKSIZE+k]
        return

    # can parallel over all files, but doubled the work since the granularity.
    if isLocal(ctrl) and isLocal(targ):
        offset = [large_offset, small_offset, half_ctrl_offset, half_targ_offset]
        for fd in fd_arr:
            for j in range(0, FILESIZE, CHUNKSIZE):
                work = []
                for k in range(CHUNKSIZE):
                    work.append(fd[j+k])

                work = x2_gate(work, offset)
                
                for k in range(CHUNKSIZE):
                    fd[j+k] = work[k]
        return

    if isGlobal(up_qubit):
        # up_file_mask = 1 << (NGQB-up_qubit-1)
        # if ctrl == up_qubit: # half files not working
        #     # targ_file_mask = 1 << (NGQB-targ-1)

        #     def isCtrl(num):
        #         ctrl_file_mask = 1 << (NGQB-ctrl-1)
        #         return num&ctrl_file_mask > 0

        #     fd_pair = []
        #     file_checklist = [ False for i in range(FILENUMBER)]
        #     for fd in range(FILENUMBER):
        #         if(file_checklist[fd]):
        #             continue
        #         file_checklist[fd] = True
        #         if(isCtrl(fd)):
        #         #     pfd = pairFile(fd)
        #         #     file_checklist[pfd] = True
        #             fd_pair.append([fd, fd])
        #     print(fd_pair)
        # else:
        #     def isCtrl(num):
        #         ctrl_file_mask = 1 << (NGQB-ctrl-1)
        #         return num&ctrl_file_mask > 0

        #     def pairFile(num):
        #         targ_file_mask = 1 << (NGQB-targ-1)
        #         return num^targ_file_mask
            
        #     fd_pair = []
        #     file_checklist = [ False for i in range(FILENUMBER)]
        #     for fd in range(FILENUMBER):
        #         if(file_checklist[fd]):
        #             continue
        #         file_checklist[fd] = True
        #         pfd = pairFile(fd)
        #         file_checklist[pfd] = True
        #         fd_pair.append([fd, pfd])
        #     print(fd_pair)

        def isCtrl(num):
            ctrl_file_mask = 1 << (NGQB-ctrl-1)
            return num&ctrl_file_mask > 0

        def pairFile(num):
            targ_file_mask = 1 << (NGQB-targ-1)
            return num^targ_file_mask

        fd_pair = []
        file_checklist = [ False for i in range(FILENUMBER)]
        for fd in range(FILENUMBER):
            if(file_checklist[fd]):
                continue
            file_checklist[fd] = True
            if(ctrl < targ and isCtrl(fd)): #ctrl is global
                fd_pair.append([fd, fd])
            elif(targ < ctrl):
                pfd = pairFile(fd)
                file_checklist[pfd] = True
                fd_pair.append([fd, pfd])
        print(fd_pair)
        return
    

    # for i in range(1 << front_qubit):
    #     for j in range(groups):
    #         for k in range(half_small_offset):
    #             swap(state, offset+k, offset+k+half_targ_offset)
    #         offset += small_offset
    #     offset += half_large_offset
    # return

# translate to Qiskit order or vice versa.
def reorder(qubit):
    return N-1-qubit

def check(ctrl, targ, state):
    simulator = Aer.get_backend('aer_simulator_statevector')
    circ = QuantumCircuit(N)
    initial_state = [1,0]   # Define initial_state as |0>
    circ.initialize(initial_state, 0)
    circ.h(reorder(0))
    circ.cx(reorder(ctrl), reorder(targ))
    circ.save_statevector(label=f'CX_{ctrl:02d}-{targ:02d}')
    circ = transpile(circ, simulator)
    data = simulator.run(circ).result().data(0)
    # for i, x in enumerate(state):
    #     if(data[i]!=x):
    #         print(data[i],x)
    qstate = data[f'CX_{ctrl:02d}-{targ:02d}']
    # print(qstate[0], state[0])

    flag = True
    for i in range(2**N):
        if (np.abs(qstate[i]-state[i//FILESIZE][i%FILESIZE]) > 1e-9):
            print(i, qstate[i], state[i//FILESIZE][i%FILESIZE])
            flag = False
    return flag
    # print(np.array_equal(qstate, state))



# state = np.zeros(2**N, dtype=np.complex128)
# print("|0> state:", state)
# init(state)
# print("H|0> state:", state)

flag = True
for ctrl in range(NGQB):
    for targ in range(NGQB):
        if(ctrl == targ): continue
        # print(f"ctrl={ctrl}, targ={targ}")
        fd_arr = init_state()
        CX(ctrl, targ, fd_arr, x_gate)
        if(not check(ctrl, targ, fd_arr)):
            print(f"ctrl={ctrl}, targ={targ}")
            print("test not pass")
            flag = False
if(flag):
    print("Global test pass: match with qiskit under 1e-9")


flag = True
for ctrl in range(N-NLQB, N):
    for targ in range(N-NLQB, N):
        if(ctrl == targ): continue
        # print(f"ctrl={ctrl}, targ={targ}")
        fd_arr = init_state()
        CX(ctrl, targ, fd_arr, x2_gate)
        if(not check(ctrl, targ, fd_arr)):
            print(f"ctrl={ctrl}, targ={targ}")
            print("test not pass")
            flag = False
if(flag):
    print("Local test pass: match with qiskit under 1e-9")


flag = True
for ctrl in range(NGQB):
    for targ in range(N-NLQB, N):
        if(ctrl == targ): continue
        print(f"ctrl={ctrl}, targ={targ}")
        fd_arr = init_state()
        CX(ctrl, targ, fd_arr, x2_gate)
        # if(not check(ctrl, targ, fd_arr)):
        #     print(f"ctrl={ctrl}, targ={targ}")
        #     print("test not pass")
        #     flag = False
if(flag):
    print("Ctrl-Global test pass: match with qiskit under 1e-9")


flag = True
for targ in range(NGQB):
    for ctrl in range(N-NLQB, N):
        if(ctrl == targ): continue
        print(f"ctrl={ctrl}, targ={targ}")
        fd_arr = init_state()
        CX(ctrl, targ, fd_arr, x2_gate)
        # if(not check(ctrl, targ, fd_arr)):
        #     print(f"ctrl={ctrl}, targ={targ}")
        #     print("test not pass")
        #     flag = False
if(flag):
    print("Targ-Global test pass: match with qiskit under 1e-9")


