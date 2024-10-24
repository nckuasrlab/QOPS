from circuit_generator import *
from ini_generator import *
from test_util import *

# Test for Unitary 3-qubit gate

# N    = 12
# NGQB =  3
# NSQB =  6
# NLQB =  3

class u3Test:
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
                        H(circuit, i)
                    real = [0.031703890, 0.546264264, 0.606622689, 0.157559501, -0.489839459, -0.130269192, 0.225524119, 0.000497503,
                            0.336453272, 0.194472042, -0.010234904, 0.379841454, 0.078144359, 0.626291867, -0.225809712, 0.505230114,
                            -0.042591163, -0.638293120, 0.047380086, 0.022477337, -0.711385751, 0.047626132, -0.109284987, 0.260262636,
                            0.648273427, -0.156774036, 0.245046606, -0.544822507, 0.155249384, -0.115105068, 0.280183730, 0.287105540,
                            0.534060505, -0.265819672, -0.026228826, 0.524108087, -0.018330603, 0.015750946, 0.243690314, -0.555673437,
                            0.306821215, 0.068454057, 0.175635896, -0.071019654, -0.029700506, -0.287238077, -0.861745829, -0.198215333,
                            -0.015578235, 0.056470711, 0.091709997, -0.480167498, -0.186309267, 0.688456735, -0.051712912, -0.496224530,
                            -0.290032849, -0.391464911, 0.727780740, 0.152527946, 0.433416004, 0.136178297, -0.055114850, -0.016582608]
                    imag = [0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0]
                    U3(circuit, x, y, z, real, imag)
                    create_circuit(circuit, self.cir_path)

                    os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path} >> /dev/null")
                    # os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path}")
                    flag = flag and simple_test(name, False, self.cir_path, "../path/set7.txt", self.N, self.NGQB, True)
                    # print("===========================")
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
            print("[PASS]", "u3Test", ": match with qiskit under 1e-9", flush=True)
        else:
            print("[x]", "u3Test", ": not pass under 1e-9", flush=True)
        print("===========================")
        return flag
