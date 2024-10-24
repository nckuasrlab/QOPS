from circuit_generator import *
from ini_generator import *
import math
from test_util import *
from qiskit.circuit.library import QFT

N    = 10
NGQB =  0
NSQB =  0
NLQB =  10

def inverse_qft_phase_on_work_section(circuit,work_section):
    H(circuit,work_section[0])
    for id in range(1,len(work_section)):
        CPhase(circuit,work_section[id],work_section[0],math.pi/(2**id))

def qft_phase_on_work_section(circuit,work_section):
    for id in range(len(work_section)-1,0,-1):
        CPhase(circuit,work_section[id],work_section[0],math.pi/(2**id))
    H(circuit,work_section[0])

def our_qft(circuit,start_qubit,end_qubit):
    total_bit=end_qubit-start_qubit+1
    for swap_idx in range(int(total_bit/2)):
        SWAP(circuit,start_qubit+swap_idx,end_qubit-swap_idx)
    for i,times in enumerate(range(end_qubit-start_qubit+1)):
        work_section=list(range(end_qubit-i,end_qubit+1))
        qft_phase_on_work_section(circuit,work_section)
        
        

def our_inverse_qft(circuit,start_qubit,end_qubit):
    for i,times in enumerate(range(end_qubit-start_qubit+1)):
        work_section=list(range(start_qubit+i,end_qubit+1))
        
        inverse_qft_phase_on_work_section(circuit,work_section)

    total_bit=end_qubit-start_qubit+1
    for swap_idx in range(int(total_bit/2)):
        SWAP(circuit,start_qubit+swap_idx,end_qubit-swap_idx)


setting =   {'total_qbit':'10',
            'global_qbit':'0',
            'thread_qbit':'0',
            'local_qbit':'10',
            'max_qbit':'38',
            'max_path':'260',
            'max_depth':'1000',
            'is_density':'0',
            'skip_init_state':'0',
            'state_paths':'./state/path1,./state/path2,./state/path3,./state/path4,./state/path5,./state/path6,./state/path7,./state/path8'}
ini_path='test.ini'
cir_path='cir_test'

create_ini(setting,ini_path)

# circuit=get_circuit()

# start_qubit=0
# end_qubit=9
# inverse_qft(circuit,start_qubit,end_qubit)
# qft(circuit,start_qubit,end_qubit)
# create_circuit(circuit,cir_path)
# os.system(f"../qSim.out -i {ini_path} -c {cir_path}")

def qftTest(name, num_qubit):
    flag = True
    circuit=get_circuit()
    
    our_qft(circuit, 0, num_qubit-1)
    create_circuit(circuit,cir_path)

    # os.system(f"../qSim.out -i {ini_path} -c {cir_path} >> /dev/null")
    os.system(f"../qSim.out -i {ini_path} -c {cir_path}")
    print("===========================", flush=True)
    # circ = circ_init(N)
    circ = QFT(N)
    print(circ)
    fd_state = read_state("../path/set7.txt", N, NGQB)
    qiskit_state = qiskit_init_state_vector(circ)
    if(not check(fd_state, qiskit_state, N, NGQB)):
        print("test not pass")
        print()
        flag = False

    if(flag):
        print("[PASS]", name, ": match with qiskit under 1e-9", flush=True)
    else:
        print("[X]", name, ": not pass under 1e-9", flush=True)
    print("===========================")
    return flag

flag = True
for i in range(1,20):
    N = i
    NLQB = i
    set_ini(setting,'total_qbit',i)
    set_ini(setting,'local_qbit',i)
    create_ini(setting,ini_path)

    flag = flag and qftTest("qftTest", i)
    if not flag:    break 

if(flag):
    print("[pass]", "qftTest", ": match with qiskit under 1e-9", flush=True)
else:
    print("[x]", "qftTest", ": not pass under 1e-9", flush=True)
print("===========================")


