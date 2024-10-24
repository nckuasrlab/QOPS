from circuit_generator import *
from ini_generator import *


setting =   {'total_qbit':'25',
            'global_qbit':'3',
            'thread_qbit':'6',
            'local_qbit':'12',
            'max_qbit':'38',
            'max_path':'260',
            'max_depth':'1000',
            'is_density':'0',
            'skip_init_state':'0',
            'state_paths':'./state/path1,./state/path2,./state/path3,./state/path4,./state/path5,./state/path6,./state/path7,./state/path8'}
ini_path='test.ini'
cir_path='cir_test'
set_ini(setting,'total_qbit',20)
create_ini(setting,ini_path)

circuit=get_circuit()
H(circuit,5)
H(circuit,6)
H(circuit,7)
create_circuit(circuit,cir_path)
os.system(f"../qSim.out -i {ini_path} -c {cir_path}")