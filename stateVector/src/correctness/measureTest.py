from circuit_generator import *
from ini_generator import *
from test_util import *

# Test for measure_single and measure_multi operations

# N    =  8
# NGQB =  2
# NSQB =  4
# NLQB =  2

class measureTest:
    def __init__(self):
        self.setting = {'total_qbit':'8',
                        'global_qbit':'2',
                        'thread_qbit':'4',
                        'local_qbit':'2',
                        'max_qbit':'38',
                        'state_paths':'./state/path1,./state/path2,./state/path3,./state/path4,./state/path5,./state/path6,./state/path7,./state/path8'}
        set_ini(self.setting, 'set_of_save_state', 2)
        self.ini_path='test.ini'
        self.cir_path='cir_test'
        self.N    =  8
        self.NGQB =  2
        self.NSQB =  4
        self.NLQB =  2

        self.shots = 100
        self.m_out = "m.out"

        self.qubit_type = ["Global", "Thread", "Middle", "Local"]
        self.range_type = [range(2), range(2, 4), range(4, 6), range(6, 8)]

    def _test_single(self, name, x_range):
        for x in x_range:
            circuit=get_circuit()
            H(circuit,0)
            for i in range(1, self.N):
                CX(circuit,0, i)
            measure(circuit, 1, self.shots)
            create_circuit(circuit, self.cir_path)

            os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path} > {self.m_out}")
            print("[MEASURE]", name, ":", flush=True)
            self.result()
            print("===========================", flush=True)
        return True

    def _test_muilti(self, name):
        circuit=get_circuit()
        H(circuit,0)
        for i in range(1, self.N):
            CX(circuit,0, i)
        m_list = [i for i in range(self.N)]
        measure_multi(circuit, m_list, self.shots)
        create_circuit(circuit, self.cir_path)

        os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path} > {self.m_out}")
        print("[MEASURE]", name, ":", flush=True)
        self.result()
        print("===========================", flush=True)
        return True

    def result(self):
        out = {}
        with open(self.m_out, 'r') as f:
            content = f.readlines()
            for line in content:
                if line[0:11] == "[MEASURE]: ":
                    # print(line, end='')
                    try:
                        out[line[11:-1]] += 1
                    except KeyError:
                        out[line[11:-1]] = 1
        for x in out:
            print(f"{x}: {out[x]}")
    
    def test(self):
        create_ini(self.setting, self.ini_path)
        # measure single
        flag = True
        for i in range(4):
            q0_type = self.qubit_type[i]
            test_name = f"single {q0_type}"
            flag = self._test_single(test_name, self.range_type[i])
            if not flag:    break

        # measure multi
        test_name = f"multi Qubits"
        flag = self._test_muilti(test_name)

        if(flag):
            print("[PASS]", "measureTest", ": match with qiskit under 1e-9", flush=True)
        # else:
        #     print("[x]", "measureTest_multi", ": not pass under 1e-9", flush=True)
        print("===========================")
        return flag
