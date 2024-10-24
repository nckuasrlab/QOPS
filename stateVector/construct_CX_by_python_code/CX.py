from qiskit import Aer, QuantumCircuit, transpile
from math import sqrt
import numpy as np

N = 20

def init(state):
    for x in range(len(state)):
        state[x] = 0.0
    state[0] = 1/sqrt(2)+0j
    state[1<<(N-1)] = 1/sqrt(2)+0j

def swap(state, a, b):
    temp = state[a]
    state[a] = state[b]
    state[b] = temp

def CX(ctrl, targ, state):
    if ctrl == targ:
        print("illegal ops.")
        return

    ctrl_offset = 1 << (N-ctrl)
    targ_offset = 1 << (N-targ)
    half_ctrl_offset = ctrl_offset >> 1
    half_targ_offset = targ_offset >> 1

    if ctrl < targ:
        targ_on_ctrl = 1 << (targ-ctrl-1)
        offset = half_ctrl_offset
        for i in range(1 << ctrl):
            for j in range(targ_on_ctrl):
                for k in range(half_targ_offset):
                    swap(state, offset+k, offset+k+half_targ_offset)
                offset += targ_offset
            offset += half_ctrl_offset
        return
    
    #targ < ctrl
    ctrl_on_targ = 1 << (ctrl-targ-1)
    offset = half_ctrl_offset
    for i in range(1 << targ):
        for j in range(ctrl_on_targ):
            for k in range(half_ctrl_offset):
                swap(state, offset+k, offset+k+half_targ_offset)
            offset += ctrl_offset
        offset += half_targ_offset
    return

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
        if (np.abs(qstate[i]-state[i]) > 1e-9):
            print(i, qstate[i], state[i])
            flag = False
    return flag
    # print(np.array_equal(qstate, state))



state = np.zeros(2**N, dtype=np.complex128)
print(state)
init(state)
print(state)

flag = True
for ctrl in range(N):
    for targ in range(N):
        if(ctrl == targ): continue
        init(state)
        CX(ctrl, targ, state)
        if(not check(ctrl, targ, state)):
            print(f"ctrl={ctrl}, targ={targ}")
            print("test not pass")
            flag = False
if(flag):
    print("test pass: match with qiskit under 1e-9")

