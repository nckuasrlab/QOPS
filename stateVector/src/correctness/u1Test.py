from circuit_generator import *
from ini_generator import *
from test_util import *

# Test for Unitary 1-qubit gate

# N    = 4
# NGQB =  1
# NSQB =  2
# NLQB =  1

class u1Test:
    def __init__(self):
        self.setting = {'total_qbit':'4',
                        'global_qbit':'1',
                        'thread_qbit':'2',
                        'local_qbit':'1',
                        'max_qbit':'38',
                        'state_paths':'./state/path1,./state/path2,./state/path3,./state/path4,./state/path5,./state/path6,./state/path7,./state/path8,./state/path9,./state/path10,./state/path11,./state/path12,./state/path13,./state/path14,./state/path15,./state/path16'}
        self.ini_path='test.ini'
        self.cir_path='cir_test'
        
        self.N    = 4
        self.NGQB =  1
        self.NSQB =  2
        self.NLQB =  1
        self.qubit_type = ["Global", "Thread", "Middle", "Local"]
        self.range_type = [range(1), range(1, 2), range(2, 3), range(3, 4)]

    def _test(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(4):
                H(circuit, i)

            # for testing another randomized unitary:
            # from scipy.stats import unitary_group
            # x = unitary_group.rvs(2) # = real+imag*j
            real = [ 0.28632032, -0.47273926,
                    -0.84618156, -0.08260835]
            imag = [ 0.25736406, 0.79265503,
                    -0.36845784, 0.37602054]
            U1(circuit, x, real, imag)
            create_circuit(circuit, self.cir_path)

            os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path} >> /dev/null")
            # os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path}")
            flag = flag and simple_test(name, False, self.cir_path, "../path/set7.txt", self.N, self.NGQB, True)
            # flag = flag and simple_test(name, False, self.cir_path, "../path/set7.txt", self.N, self.NGQB)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def test(self):
        create_ini(self.setting, self.ini_path)
        flag = True
        for i in range(4):
            q0_type = self.qubit_type[i]
            test_name = f"{q0_type}"
            flag = self._test(test_name, self.range_type[i])
            if not flag:    break 

        if(flag):
            print("[PASS]", "u1Test", ": match with qiskit under 1e-9", flush=True)
        else:
            print("[x]", "u1Test", ": not pass under 1e-9", flush=True)
        print("===========================")
        return flag
