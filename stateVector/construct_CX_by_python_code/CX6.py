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
            lo = base+move[3]
            for k in range(move[1]>>1):
                # print(up, lo)
                swap(chunk, up, lo)
                up += 1
                lo += 1
            base += move[1]
        base += move[0]
    return chunk

# i 從左數第ctrl個bit是否為1
def isCtrl(i, ctrl, n):
    ctrl_mask = 1 << (n-ctrl-1)
    return True if i&ctrl_mask > 0 else False

# i 從左數第targ個bit對應的數字
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
def CX(ctrl, targ, fd_arr, one_gate):
    if ctrl == targ:
        print("illegal ops.")
        return

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


        fd_pair = make_work_pair(NGQB, ctrl, targ)
        
        # [Thread level parallel] here
        for [fd1, fd2] in fd_pair:
            for j in range(0, FILESIZE, CHUNKSIZE):
                work = []
                for k in range(CHUNKSIZE):
                    work.append(fd_arr[fd1][j+k])
                for k in range(CHUNKSIZE):
                    work.append(fd_arr[fd2][j+k])
                work = x_gate(work, CHUNKSIZE)
                
                for k in range(CHUNKSIZE):
                    fd_arr[fd1][j+k] = work[k]
                    fd_arr[fd2][j+k] = work[CHUNKSIZE+k]
        return

    # can parallel over all files, but doubled the work since the granularity.
    if isLocal(ctrl) and isLocal(targ):
        offset = [large_offset, small_offset, half_ctrl_offset, half_targ_offset]
        # [Thread level parallel] here
        for fd in fd_arr:
            for j in range(0, FILESIZE, CHUNKSIZE):
                work = []
                # read chunk
                for k in range(CHUNKSIZE):
                    work.append(fd[j+k])

                work = x2_gate(work, offset)
                
                #write chunk
                for k in range(CHUNKSIZE):
                    fd[j+k] = work[k]
        return

    # one-side global
    if isGlobal(up_qubit):
        # file pair construct
        def fdIsCtrl(num):
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
            if(targ < ctrl):
                pfd = pairFile(fd)
                file_checklist[pfd] = True
                fd_pair.append([fd, pfd])
            elif(fdIsCtrl(fd)): #ctrl is global
                fd_pair.append([fd, fd])
        # print(fd_pair)
        for fd in fd_pair:
            fd.sort() # 保證先0再1
        # print(fd_pair)

        if isLocal(lo_qubit):
            if (ctrl < targ): # ctrl is global, fd_pair[0][0] == fd_pair[0][1]:
                # [Thread level parallel] here
                for [fd1, fd2] in fd_pair:
                    for j in range(0, FILESIZE, CHUNKSIZE):
                        work = []
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd1][j+k])
                        work = x_gate(work, half_small_offset)
                        for k in range(CHUNKSIZE):
                            fd_arr[fd1][j+k] = work[k]
                return

            else:
                # [Thread level parallel] here
                for [fd1, fd2] in fd_pair:
                    # twice CHUNKSIZE
                    for j in range(0, FILESIZE, CHUNKSIZE):
                        work = []
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd1][j+k])
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd2][j+k])
                        work = x2_gate(work, [2*CHUNKSIZE, small_offset, half_small_offset, CHUNKSIZE])
                        
                        for k in range(CHUNKSIZE):
                            fd_arr[fd1][j+k] = work[k]
                            fd_arr[fd2][j+k] = work[k+CHUNKSIZE]
                return

        elif isThread(lo_qubit):# global-thread
            # only use half of all threads
            # thread pair construct
            threads_per_file = (1<<(NSQB-NGQB))
            def threadIsCtrl(num):
                ctrl_thread_mask = 1 << (NSQB-ctrl-1)
                return num&ctrl_thread_mask > 0

            def pairThread(num):
                thread_mask = 1 << (NSQB-lo_qubit-1)
                return num^thread_mask

            thread_pair = []
            thread_checklist = [ False for i in range(threads_per_file)]
            for t in range(threads_per_file):
                if(thread_checklist[t]):
                    continue
                thread_checklist[t] = True
                if(ctrl < targ): # 代表 ctrl-global targ-thread
                    thread_checklist[pairThread(t)] = True
                    # 這邊可以插一行code判斷target是否為1 方便後續gate不會搞混0,1順序
                    thread_pair.append([t, pairThread(t)])
                elif(threadIsCtrl(t)): # also targ < ctrl
                    thread_pair.append([t, t])

            # print(f"file_pair = {fd_pair}")
            # print(f"thread_pair = {thread_pair}")

            thread_size = 1 << (N-NSQB)
            # [Thread level parallel] here
            for [fd1, fd2] in fd_pair:
                for [t1, t2] in thread_pair:
                    t1_off = t1 * thread_size # equal to t1 << (N-NSQB)
                    t2_off = t2 * thread_size # equal to t2 << (N-NSQB)
                    for i in range(0, thread_size, CHUNKSIZE):
                        #這邊可能在其他gate要注意|0> |1>的順序
                        #x gate 沒差
                        work = []
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd1][t1_off+k])
                        for k in range(CHUNKSIZE):
                            work.append(fd_arr[fd2][t2_off+k])
                        
                        x_gate(work, CHUNKSIZE)

                        for k in range(CHUNKSIZE):
                            fd_arr[fd1][t1_off+k] = work[k]
                            fd_arr[fd2][t2_off+k] = work[k+CHUNKSIZE]
                        t1_off += CHUNKSIZE
                        t2_off += CHUNKSIZE
            return
            
        else: # global-middle
            # print(f"{ctrl}-{targ} global-middle")
            threads_per_file = (1<<(NSQB-NGQB))
            thread_size = 1 << (N-NSQB)

            if (ctrl < targ): # ctrl is global
                for [fd1, fd2] in fd_pair: # fd1 must equal to fd2
                    for t in range(threads_per_file):
                        t_off = t * thread_size # equal to t << (N-NSQB)
                        #這邊可能在其他gate要注意|0> |1>的順序
                        #x gate 沒差
                        for i in range(0, thread_size, small_offset):
                            for j in range (0, half_small_offset, CHUNKSIZE):
                                work = []
                                for k in range(CHUNKSIZE):
                                    work.append(fd_arr[fd1][t_off+k])
                                for k in range(CHUNKSIZE):
                                    work.append(fd_arr[fd2][t_off+k+half_small_offset]) # 此時ctrl 是global,  fd1==fd2
                                
                                x_gate(work, CHUNKSIZE)

                                for k in range(CHUNKSIZE):
                                    fd_arr[fd1][t_off+k] = work[k]
                                    fd_arr[fd2][t_off+k+half_small_offset] = work[k+CHUNKSIZE]

                                t_off += CHUNKSIZE
                            t_off += half_small_offset
                return
                        
            else: # targ is global
                for [fd1, fd2] in fd_pair: # fd1 must equal to fd2
                    for t in range(threads_per_file):
                        t_off = t * thread_size # equal to t << (N-NSQB)
                        #這邊可能在其他gate要注意|0> |1>的順序
                        #x gate 沒差
                        t_off += half_small_offset
                        for i in range(0, thread_size, small_offset):
                            for j in range (0, half_small_offset, CHUNKSIZE):
                                work = []
                                for k in range(CHUNKSIZE):
                                    work.append(fd_arr[fd1][t_off+k])
                                for k in range(CHUNKSIZE):
                                    work.append(fd_arr[fd2][t_off+k]) # 此時targ 是global,  fd1!=fd2
                                
                                x_gate(work, CHUNKSIZE)

                                for k in range(CHUNKSIZE):
                                    fd_arr[fd1][t_off+k] = work[k]
                                    fd_arr[fd2][t_off+k] = work[k+CHUNKSIZE]

                                t_off += CHUNKSIZE
                            t_off += half_small_offset
                return

    if isThread(ctrl) and isThread(targ):
        # only use half of all threads
        # thread pair construct
        threads_per_file = (1<<(NSQB-NGQB))
        def threadIsCtrl(num):
            ctrl_thread_mask = 1 << (NSQB-ctrl-1)
            return num&ctrl_thread_mask > 0

        def pairThread(num):
            thread_mask = 1 << (NSQB-lo_qubit-1)
            return num^thread_mask

        thread_pair = []
        thread_checklist = [ False for i in range(threads_per_file)]
        for t in range(threads_per_file):
            if(thread_checklist[t]):
                continue
            thread_checklist[t] = True
            if(ctrl < targ): # 代表 ctrl-global targ-thread
                thread_checklist[pairThread(t)] = True
                # 這邊可以插一行code判斷target是否為1 方便後續gate不會搞混0,1順序
                thread_pair.append([t, pairThread(t)])
            elif(threadIsCtrl(t)): # also targ < ctrl
                thread_pair.append([t, t])
        
        
        for fd in range(FILENUMBER):
            return
        return
    
    if isMiddle(up_qubit):
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
            # 應該有的state, 本模擬器的state
            # print(i, state[i], fd_arr[i//FILESIZE][i%FILESIZE])
            flag = False
    return flag
    # print(np.array_equal(qstate, state))



# state = np.zeros(2**N, dtype=np.complex128)
# print("|0> state:", state)
# init(state)
# print("H|0> state:", state)

def test(name:str, r1, r2, CTrev:bool, gate_func, h_list):
    flag = True
    for it1 in r1:
        for it2 in r2:
            if(it1 == it2): continue
            # print(f"ctrl={it1 if not CTrev else it2}, targ={it2 if not CTrev else it1}")
            circ = circ_init()
            for q in h_list:
                circ.h(reorder(q))
            state = qiskit_init(circ)
            fd_arr = init_state(state)

            
            # if(not check(fd_arr, state)):
            #     print(f"ctrl={it1 if not CTrev else it2}, targ={it2 if not CTrev else it1}")
            #     print("test not pass")
            #     flag = False
            
            # continue
            CX(it1 if not CTrev else it2, it2 if not CTrev else it1, fd_arr, gate_func)

            circ = circ_init()
            for q in h_list:
                circ.h(reorder(q))
            circ.cx(reorder(it1 if not CTrev else it2), reorder(it2 if not CTrev else it1))
            state = qiskit_init(circ)

            if(not check(fd_arr, state)):
                print(f"ctrl={it1 if not CTrev else it2}, targ={it2 if not CTrev else it1}")
                print("test not pass")
                print()
                flag = False

    if(flag):
        print(name, "pass: match with qiskit under 1e-9")
    else:
        # print_state_f(fd_arr)
        # print_state(state)
        print(name, "not pass")

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
    test("Global test", range(NGQB), range(NGQB), False, x_gate, h_list)
    test("Local test", range(N-NLQB, N), range(N-NLQB, N), False, x2_gate, h_list)
    test("Ctrl-Global Targ-Local test", range(NGQB), range(N-NLQB, N), False, x_gate, h_list)
    test("Targ-Global Ctrl-Local test", range(NGQB), range(N-NLQB, N), True, x2_gate, h_list)
    test("Ctrl-Global Targ-Thread test", range(NGQB), range(NGQB, NSQB), False, x_gate, h_list)
    test("Targ-Global Ctrl-Thread test", range(NGQB), range(NGQB, NSQB), True, x_gate, h_list)
    test("Ctrl-Global Targ-Middle test", range(NGQB), range(NSQB, N-NLQB), False, x_gate, h_list)
    test("Targ-Global Ctrl-Middle test", range(NGQB), range(NSQB, N-NLQB), True, x_gate, h_list)
    print("===========================")

