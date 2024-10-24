from circuit_generator import *
from ini_generator import *
from test_util import *

# Test for Unitary 2-qubit gate

# N    = 8
# NGQB =  2
# NSQB =  4
# NLQB =  2

class u2Test:
    def __init__(self):
        self.setting = {'total_qbit':'8',
                        'global_qbit':'2',
                        'thread_qbit':'4',
                        'local_qbit':'2',
                        'max_qbit':'38',
                        'state_paths':'./state/path1,./state/path2,./state/path3,./state/path4,./state/path5,./state/path6,./state/path7,./state/path8,./state/path9,./state/path10,./state/path11,./state/path12,./state/path13,./state/path14,./state/path15,./state/path16'}
        self.ini_path='test.ini'
        self.cir_path='cir_test'
        
        self.N    = 8
        self.NGQB =  2
        self.NSQB =  4
        self.NLQB =  2
        self.qubit_type = ["Global", "Thread", "Middle", "Local"]
        self.range_type = [range(2), range(2, 4), range(4, 6), range(6, 8)]

    def _test(self, name, x_range, y_range):
        flag = True
        for x in x_range:
            for y in y_range:
                if x==y: continue
                circuit=get_circuit()
                for i in range(8):
                    H(circuit, i)
                real = [ 0.65396611,  0.04869761, -0.02532728, -0.75452992,
                         0.38302748, -0.80093799, -0.35552298,  0.29221858,
                        -0.55736732, -0.57963226,  0.27007049, -0.52955646,
                         0.33905744, -0.14196239,  0.89444053,  0.25468188]
                imag = [0, 0, 0, 0,
                        0, 0, 0, 0,
                        0, 0, 0, 0,
                        0, 0, 0, 0]
                U2(circuit, x, y, real, imag)
                create_circuit(circuit, self.cir_path)

                os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path} >> /dev/null")
                # os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path}")
                flag = flag and simple_test(name, False, self.cir_path, "../path/set7.txt", self.N, self.NGQB, True)
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
                q0_type = self.qubit_type[i]
                q1_type = self.qubit_type[j]
                test_name = f"{q0_type}-{q1_type}"
                flag = self._test(test_name, self.range_type[i], self.range_type[j])
                if not flag:    break
            if not flag:    break 

        if(flag):
            print("[PASS]", "u2Test", ": match with qiskit under 1e-9", flush=True)
        else:
            print("[x]", "u2Test", ": not pass under 1e-9", flush=True)
        print("===========================")
        return flag
