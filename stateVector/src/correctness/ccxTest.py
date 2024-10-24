from circuit_generator import *
from ini_generator import *
from test_util import *

# Test for CCX gate

# N    = 12
# NGQB =  3
# NSQB =  6
# NLQB =  3

class ccxTest:
    def __init__(self):
        self.setting = {'total_qbit':'12',
                        'global_qbit':'3',
                        'thread_qbit':'6',
                        'local_qbit':'3',
                        'max_qbit':'38',
                        'state_paths':'./state/path1,./state/path2,./state/path3,./state/path4,./state/path5,./state/path6,./state/path7,./state/path8,./state/path9,./state/path10,./state/path11,./state/path12,./state/path13,./state/path14,./state/path15,./state/path16'}
        self.ini_path='test.ini'
        self.cir_path='cir_test'
        
        self.N    = 12
        self.NGQB =  3
        self.NSQB =  6
        self.NLQB =  3

        self.qubit_type = ["Global", "Thread", "Middle", "Local"]
        self.range_type = [range(3), range(3, 6), range(6, 9), range(9, 12)]

    def _test(self, name, x_range, y_range, z_range):
        flag = True
        for x in x_range:
            for y in y_range:
                if x==y: continue
                for z in z_range:
                    if z==x: continue
                    if z==y: continue

                    circuit=get_circuit()
                    for i in range(12):
                        if i == z: continue
                        H(circuit, i)
                    CCX(circuit, x, y, z)
                    create_circuit(circuit, self.cir_path)

                    os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path} >> /dev/null")
                    # os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path}")
                    # print("===========================", flush=True)

                    circuit=get_circuit()
                    for i in range(12):
                        if i == z: continue
                        H(circuit, i)
                    CCX_true(circuit, x, y, z)
                    create_circuit(circuit, self.cir_path)

                    flag = flag and simple_test(name, False, self.cir_path, "../path/set7.txt", self.N, self.NGQB, True)
                    if(not flag): break
                if(not flag): break
            if(not flag): break

        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def test(self):
        create_ini(self.setting, self.ini_path)
        flag = True
        for i in range(4):
            for j in range(4):
                for k in range(4):
                    q0_type = self.qubit_type[i]
                    q1_type = self.qubit_type[j]
                    q2_type = self.qubit_type[k]
                    test_name = f"{q0_type}-{q1_type}-{q2_type}"
                    flag = self._test(test_name, self.range_type[i], self.range_type[j], self.range_type[k])
                    if not flag:    break
                if not flag:    break
            if not flag:    break 

        if(flag):
            print("[PASS]", "ccxTest", ": match with qiskit under 1e-9", flush=True)
        else:
            print("[x]", "ccxTest", ": not pass under 1e-9", flush=True)
        print("===========================")
        return flag
