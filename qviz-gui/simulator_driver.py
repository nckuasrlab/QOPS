from circuit_generator import *
from ini_generator import *
from math import pi
import argparse
from qpu_function import * 

parser = argparse.ArgumentParser()
parser.add_argument("base")
args = parser.parse_args()

setting =   {'total_qbit':'4',
           'global_qbit':'1',
           'thread_qbit':'1',
           'local_qbit':'1',
           'max_qbit':'38',
           'max_path':'260',
           'max_depth':'1000',
           'is_density':'0',
           'skip_init_state':'0',
           # 'set_of_save_state':'0',
           'state_paths':'./state/path1,./state/path2,./state/path3,./state/path4,./state/path5,./state/path6,./state/path7,./state/path8,./state/path9,./state/path10,./state/path11,./state/path12,./state/path13,./state/path14,./state/path15,./state/path16'}
ini_path='test.ini'
cir_path='cir_test'

N=4
LQ=1

set_ini(setting,'total_qbit',N)
set_ini(setting,'local_qbit',LQ)

create_ini(setting,ini_path)

circuit=get_circuit()
circuit=qpu_function()

create_circuit(circuit,cir_path)
if args.base == "context" :
    os.system(f"../qSim_context.out -i {ini_path} -c {cir_path} >> ./result.txt")
elif args.base == "counter" :
    os.system(f"../qSim_counter.out -i {ini_path} -c {cir_path} >> ./result.txt")
elif args.base == "normal" :
    os.system(f"../qSim_normal.out -i {ini_path} -c {cir_path} >> ./result.txt")

# os.system(f"python3 ../test_by_qiskit.py cir_test ../path/set7.txt  4 1 1 1 >> ./result.txt")