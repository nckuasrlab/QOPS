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
    # print(move)
    base = move[2]
    for i in range(0, CHUNKSIZE, move[0]):
        for j in range(0, move[0]>>1, move[1]):
            up = base+j
            lo = base+j+move[3]
            for k in range(move[1]>>1):
                # print(up, lo)
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
        # print(fd_pair)

        if isLocal(lo_qubit):
            for [fd1, fd2] in fd_pair:
                if fd1 == fd2:
                    for j in range(0, FILESIZE, CHUNKSIZE):
                        work = []
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd1][j+k])
                        work = x_gate(work, half_small_offset)
                        for k in range(CHUNKSIZE):
                            fd_arr[fd1][j+k] = work[k]
                    return

                else:
                    # twice CHUNKSIZE
                    for j in range(0, FILESIZE, CHUNKSIZE):
                        work = []
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd1][j+k])
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd2][j+k])
                        work = x2_gate(work, [CHUNKSIZE, small_offset, half_ctrl_offset, CHUNKSIZE])
                        
                        for k in range(CHUNKSIZE):
                            fd_arr[fd1][j+k] = work[k]
                            fd_arr[fd2][j+k] = work[k+CHUNKSIZE]
                    return

            

            
        # elif 
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

def check(fd_arr, state):
    # qstate = data[f'CX_{ctrl:02d}-{targ:02d}']
    # print(qstate[0], state[0])

    flag = True
    for i in range(2**N):
        if (np.abs(state[i]-fd_arr[i//FILESIZE][i%FILESIZE]) > 1e-9):
            print(i, state[i], fd_arr[i//FILESIZE][i%FILESIZE])
            flag = False
    return flag
    # print(np.array_equal(qstate, state))



# state = np.zeros(2**N, dtype=np.complex128)
# print("|0> state:", state)
# init(state)
# print("H|0> state:", state)

def test(name:str, r1, r2, rev:bool, gate_func, h1):
    flag = True
    for it1 in r1:
        for it2 in r2:
            if(it1 == it2): continue
            # print(f"ctrl={it1 if not rev else it2}, targ={it2 if not rev else it1}")
            circ = circ_init()
            circ.h(reorder(h1))
            state = qiskit_init(circ)
            fd_arr = init_state(state)
            CX(it1 if not rev else it2, it2 if not rev else it1, fd_arr, gate_func)

            circ = circ_init()
            circ.h(reorder(h1))
            circ.cx(reorder(it1 if not rev else it2), reorder(it2 if not rev else it1))
            state = qiskit_init(circ)

            if(not check(fd_arr, state)):
                print(f"ctrl={it1 if not rev else it2}, targ={it2 if not rev else it1}")
                print("test not pass")
                flag = False

    if(flag):
        print(name, "pass: match with qiskit under 1e-9")
    else:
        print_state_f(fd_arr)
        print_state(state)
        print(name, "not pass")

print(CHUNKSIZE)
for h in range(N):
    print(f"h on {h}")
    test("Global test", range(NGQB), range(NGQB), False, x_gate, h)
    test("Local test", range(N-NLQB, N), range(N-NLQB, N), False, x2_gate, h)
    test("Ctrl-Global test", range(NGQB), range(N-NLQB, N), False, x_gate, h)
    test("Targ-Global test", range(NGQB), range(N-NLQB, N), True, x2_gate, h)
