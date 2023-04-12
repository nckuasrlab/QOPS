from qiskit import Aer, QuantumCircuit, transpile
from qiskit.extensions import UnitaryGate
from math import sqrt
import numpy as np
import sys
import os
import struct
import re
import time
import configparser

def print_header(N, NFQB, NLQB, isDensity):
    NUMFD = 1 << NFQB
    NUMTD = NUMFD
    CHUNKSIZE = 1 << NLQB

    if(isDensity):
        FILESIZE = 1 << (2*N-NFQB)
        print(f"N = {N}")
        print(f"#Thread = {NUMTD}")
        print(f"NFQB = {NFQB}, #FILE = {NUMFD}, FILESIZE = {FILESIZE}")
    else:
        FILESIZE = 1 << (N-NFQB)
        print(f"N = {N}")
        print(f"#Thread = {NUMTD}")
        print(f"NFQB = {NFQB}, #FILE = {NUMFD}, FILESIZE = {FILESIZE}")
        print(f"NLQB = {NLQB}, CHUNKSIZE = {CHUNKSIZE}")
        print(f"file = {[i for i in range(N, N-NFQB-1, -1)]}")
        print(f"middle = {[i for i in range(N-NFQB-1, NLQB-1, -1)]}")
        print(f"chunk  = {[i for i in range(NLQB-1, -1, -1)]}")

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

    data = simulator.run(circ).result().data(0)

    return data['save']

# [density_matrix] dump state with given circuit
def qiskit_init_density_matrix(circ):
    simulator = Aer.get_backend('aer_simulator_density_matrix')
    circ.save_density_matrix(label=f'save')
    circ = transpile(circ, simulator)

    data = simulator.run(circ).result().data(0)

    return data['save'].T.reshape(-1)

def read_state(path, N, NFQB):

    NUMFD = 1 << NFQB
    FILESIZE = 1<< (N-NFQB)
    config = configparser.ConfigParser()
    config.read(path)
    state_paths = config.get("system", "state_paths")
    state_paths = re.split(",", state_paths)
    state_paths = state_paths[0:NUMFD]

    fd_arr = [np.zeros(FILESIZE, dtype=np.complex128) for i in range(1 << NFQB)]

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
    circ.barrier()
    for op in ops:
        if op[0]==0:
            circ.h(int(op[4]))
    print(circ)
    return circ

def check(fd_state, qiskit_state, N, NFQB):
    NUMFD = 1 << NFQB
    FILESIZE = 1<< (N-NFQB)
    qiskit_state = np.array(qiskit_state)
    flag = True
    for i in range(NUMFD):
        if (not np.alltrue(np.abs(qiskit_state[i*FILESIZE:(i+1)*FILESIZE]-fd_state[i]) < 1e-9)):
            flag = False
            break
    return flag

def simple_test(name:str, isDensity:bool, circuit_path, ini_path, N, NFQB):
    flag = True
    if(isDensity):
        fd_state = read_state(ini_path, 2*N, NFQB)
        circ = set_circuit(circuit_path, N)
        qiskit_state = qiskit_init_density_matrix(circ)
        if(not check(fd_state, qiskit_state, 2*N, NFQB)):
            print("test not pass", flush=True)
            print()
            flag = False

    else:
        fd_state = read_state(ini_path, N, NFQB)
        circ = set_circuit(circuit_path, N)
        qiskit_state = qiskit_init_state_vector(circ)
        if(not check(fd_state, qiskit_state, N, NFQB)):
            print("test not pass", flush=True)
            print()
            flag = False

    if(flag):
        print("[PASS]", name, ": match with qiskit under 1e-9", flush=True)
    else:
        print("[x]", name, "not pass under 1e-9", flush=True)

    return flag