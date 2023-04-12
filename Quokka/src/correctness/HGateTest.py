from circuit_generator import *
from ini_generator import *
from test_util import *

# Test for H gates

# N    = 4
# NFQB =  1
# NLQB =  2

class HGateTest:
    def __init__(self):
        self.setting = {'total_qbit':'4',
                        'file_qbit':'1',
                        'thread_qbit':'2',
                        'local_qbit':'1',
                        'max_qbit':'38',
                        'state_paths':'./state/path1,./state/path2'}
        self.ini_path='test.ini'
        self.cir_path='cir.txt'

        self.N    = 4
        self.NFQB =  1
        self.NLQB =  2
        self.qubit_type = ["File", "Middle", "Chunk"]
        self.range_type = [reversed(range(self.N-self.NFQB, self.N)), reversed(range(self.NLQB, self.N-self.NFQB)), reversed(range(self.NLQB))]
    
    def _test_H(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(self.N):
                H(circuit, i)
            H(circuit, x)
            create_circuit(circuit, self.cir_path)
            flag = self.execute(name)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag
    
    def execute(self, name):
        # os.system(f"../quokka -i {self.ini_path} -c {self.cir_path} >> /dev/null")
        os.system(f"../quokka -i {self.ini_path} -c {self.cir_path}")
        return simple_test(name, False, self.cir_path, self.ini_path, self.N, self.NFQB)
    
    def test(self):
        create_ini(self.setting, self.ini_path)
        flag = True
        for i in range(3):
            q0_type = self.qubit_type[i]
            test_name = f"{q0_type}"
            flag = self._test_H(test_name, self.range_type[i])
            if not flag:    break
        if(flag):
            print("[PASS]", "H Test", ": match with qiskit under 1e-9", flush=True)
            print("===========================")
        else:
            print("[x]", "H Test", ": not pass under 1e-9", flush=True)
            print("===========================")
            return
        return flag
    