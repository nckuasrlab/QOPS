from qiskit import Aer, QuantumCircuit, transpile
from qiskit.extensions import UnitaryGate
from math import sqrt
import numpy as np
import sys
import os
import struct
import re
import time

def print_header(N, NGQB, NSQB, NLQB, isDensity):
    NUMFD = 1 << NGQB
    NUMTD = 1 << NSQB
    CHUNKSIZE = 1 << NLQB

    if(isDensity):
        FILESIZE = 1 << (2*N-NGQB)
        print(f"N = {N}")
        print(f"NGQB = {NGQB}, #FILE = {NUMFD}, FILESIZE = {FILESIZE}")
    else:
        FILESIZE = 1 << (N-NGQB)
        print(f"N = {N}")
        print(f"NGQB = {NGQB}, #FILE = {NUMFD}, FILESIZE = {FILESIZE}")
        print(f"NSQB = {NSQB}, #Thread = {NUMTD}")
        print(f"NLQB = {NLQB}, CHUNKSIZE = {CHUNKSIZE}")
        print(f"global = {[i for i in range(0, NGQB)]}")
        print(f"thread = {[i for i in range(NGQB, NSQB)]}")
        print(f"middle = {[i for i in range(NSQB, N-NLQB)]}")
        print(f"local  = {[i for i in range(N-NLQB, N)]}")

    print("===========================", flush=True)

# init to 0 state
def circ_init(N:int):
    circ = QuantumCircuit(N)
    initial_state = [1,0]   # Define initial_state as |0>
    circ.initialize(initial_state, 0)
    return circ

# [state_vector] dump state with given circuit
def qiskit_init_state_vector(circ):
    simulator = Aer.get_backend('aer_simulator_statevector')
    circ.save_statevector(label=f'save')
    circ = transpile(circ, simulator)

    start_time = time.perf_counter_ns()
    data = simulator.run(circ).result().data(0)
    end_time = time.perf_counter_ns()
    print(f"Time: {(end_time - start_time)/1000:.0f} (us)\n")

    return data['save']

# [density_matrix] dump state with given circuit
def qiskit_init_density_matrix(circ):
    simulator = Aer.get_backend('aer_simulator_density_matrix')
    circ.save_density_matrix(label=f'save')
    circ = transpile(circ, simulator)

    start_time = time.perf_counter_ns()
    data = simulator.run(circ).result().data(0)
    end_time = time.perf_counter_ns()
    print(f"Time: {(end_time - start_time)/1000:.0f} (us)\n")

    return data['save'].T.reshape(-1)

def read_state(path, N, NGQB):
    NUMFD = 1 << NGQB
    FILESIZE = 1<< (N-NGQB)
    with open(path, mode="r") as states_path:
        state_paths = states_path.read()
        state_paths = re.split("\n", state_paths)
        state_paths = state_paths[0:NUMFD]
    
    fd_arr = [np.zeros(FILESIZE, dtype=np.complex128) for i in range(1 << NGQB)]

    for fd, state_path in enumerate(state_paths):
        f = fd_arr[fd]
        with open(state_path, mode="rb") as state_file:
            try:
                state = state_file.read()
                k = 0
                for i in range(FILESIZE):
                    (real, imag) = struct.unpack("dd", state[k:k+16])
                    f[i] = real+imag*1j
                    k += 16
            except:
                print(f"read from {state_path}")
                print(f"[ERROR]: error at reading {k}th byte")
                exit()

    return fd_arr

# translate to Qiskit order or vice versa.
def reorder(qubit:int, N:int):
    return int(-qubit+N-1)

def set_circuit(path, N):
    circ=circ_init(N)
    ops_num=0
    ops=[]
    with open(path,'r') as f:
        for i, line in enumerate(f.readlines()):
            s=line.split()
            if i==0:
                ops_num=int(s[0])
            else:
                ops.append([float(x) for x in s])
    ops = ops[0:ops_num]
    # for op in ops:
    #     print(op)
    for op in ops:
        if op[0]==0:
            circ.h(reorder(op[4], N))
        if op[0]==1:
            circ.s(reorder(op[4], N))
        if op[0]==2:
            circ.t(reorder(op[4], N))
        if op[0]==3:
            circ.x(reorder(op[4], N))
        if op[0]==4:
            circ.y(reorder(op[4], N))
        if op[0]==5:
            circ.z(reorder(op[4], N))
        if op[0]==6:
            circ.p(float(op[5]), reorder(op[4], N))
        if op[0]==7: # UnitaryGate
            gate = UnitaryGate([[op[5]+op[9]*1j,  op[6]+op[10]*1j], 
                                [op[7]+op[11]*1j, op[8]+op[12]*1j]])
            circ.append(gate,[reorder(op[4], N)])

        if op[0]==8:
            circ.cx(reorder(op[4], N),reorder(op[5], N))
        if op[0]==9:
            circ.cy(reorder(op[4], N),reorder(op[5], N))
        if op[0]==10:
            circ.cz(reorder(op[4], N),reorder(op[5], N))
        if op[0]==11:
            circ.cp(float(op[6]),reorder(op[4], N),reorder(op[5], N))
        if op[0]==12:
            # control-unitary
            gate = UnitaryGate([[1, 0, 0, 0],
                                [0, 1, 0, 0],
                                [0, 0, op[6]+op[10]*1j, op[7]+op[11]*1j],
                                [0, 0, op[8]+op[12]*1j, op[9]+op[13]*1j]])
            circ.append(gate,[reorder(op[4], N), reorder(op[5], N)])
        if op[0]==13:
            circ.swap(reorder(op[4], N),reorder(op[5], N))
        if op[0]==14:
            # control1 control2 targert
            circ.toffoli(reorder(op[4], N),reorder(op[5], N),reorder(op[6], N))

        if op[0]==31: # 2 qubit UnitaryGate
            # gate = UnitaryGate([[op[6] +op[22]*i, op[7] +op[23]*i, op[8] +op[24]*i, op[9] +op[25]*i], 
            #                     [op[10]+op[26]*i, op[11]+op[27]*i, op[12]+op[28]*i, op[13]+op[29]*i], 
            #                     [op[14]+op[30]*i, op[15]+op[31]*i, op[16]+op[32]*i, op[17]+op[33]*i], 
            #                     [op[18]+op[34]*i, op[19]+op[35]*i, op[20]+op[36]*i, op[21]+op[37]*i]])
            gate = UnitaryGate([[op[ 6], op[ 7], op[ 8], op[ 9]], 
                                [op[10], op[11], op[12], op[13]], 
                                [op[14], op[15], op[16], op[17]], 
                                [op[18], op[19], op[20], op[21]]])
            # Qiskit treat the order in a reverse way.
            # WHAAAT?!!!WHYYYYYY?????
            # circ.append(gate,[reorder(op[4], N), reorder(op[5], N)])
            circ.append(gate,[reorder(op[5], N), reorder(op[4], N)])
        if op[0]==32:
            gate = UnitaryGate([[op[ 7], op[ 8], op[ 9], op[10], op[11], op[12], op[13], op[14]],
                                [op[15], op[16], op[17], op[18], op[19], op[20], op[21], op[22]],
                                [op[23], op[24], op[25], op[26], op[27], op[28], op[29], op[30]],
                                [op[31], op[32], op[33], op[34], op[35], op[36], op[37], op[38]],
                                [op[39], op[40], op[41], op[42], op[43], op[44], op[45], op[46]],
                                [op[47], op[48], op[49], op[50], op[51], op[52], op[53], op[54]],
                                [op[55], op[56], op[57], op[58], op[59], op[60], op[61], op[62]],
                                [op[63], op[64], op[65], op[66], op[67], op[68], op[69], op[70]]])
            circ.append(gate, [reorder(op[6], N), reorder(op[5], N),reorder(op[4], N)])
    
    print(circ)
    return circ

def check(fd_state, qiskit_state, N, NGQB):
    NUMFD = 1 << NGQB
    FILESIZE = 1<< (N-NGQB)
    qiskit_state = np.array(qiskit_state)
    flag = True
    # for i in range(2**N):
    #     if (np.abs(state[i]-fd_arr[i//FILESIZE][i%FILESIZE]) > 1e-9):
    for i in range(NUMFD):
        if (not np.alltrue(np.abs(qiskit_state[i*FILESIZE:(i+1)*FILESIZE]-fd_state[i]) < 1e-9)):
            # a = np.abs(qiskit_state[i*FILESIZE:(i+1)*FILESIZE]-fd_state[i]) < 1e-9
            # for k, x in enumerate(a):
            #     if(x==False):
            #         print(i, k, qiskit_state[i*FILESIZE+k], fd_state[i][k])

            # print order: 應該有的state, 本模擬器的state
            # print(i, state[i], fd_arr[i//FILESIZE][i%FILESIZE])
            flag = False
            break

    # for i in range(NUMFD):
    #     print(f"fd_arr[{i}][0] = {fd_state[i][0]}")
    #     # print(f"fd_arr[{i}][0] = \n{fd_state[i][0:16]}")
    # for i in range(NUMFD):
    #     print(f"qiskit[{i}][0] = {qiskit_state[i*FILESIZE]}")
    #     # print(f"qiskit[{i}][0] = \n{qiskit_state[i*FILESIZE:i*FILESIZE+16]}")
    return flag

def simple_test(name:str, isDensity:bool, circuit_path, state_path, N, NGQB):
    flag = True
    if(isDensity):
        fd_state = read_state(state_path, 2*N, NGQB)
        circ = set_circuit(circuit_path, N)
        qiskit_state = qiskit_init_density_matrix(circ)
        if(not check(fd_state, qiskit_state, 2*N, NGQB)):
            print("test not pass")
            print()
            flag = False
        

    else:
        fd_state = read_state(state_path, N, NGQB)
        circ = set_circuit(circuit_path, N)
        qiskit_state = qiskit_init_state_vector(circ)
        if(not check(fd_state, qiskit_state, N, NGQB)):
            print("test not pass")
            print()
            flag = False

    if(flag):
        print("[pass]", name, ": match with qiskit under 1e-9", flush=True)
    else:
        print("[x]", name, "not pass under 1e-9", flush=True)

    return flag