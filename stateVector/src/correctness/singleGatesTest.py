from circuit_generator import *
from ini_generator import *
from test_util import *

# Test for H, S, T, X, Y, Z gates

# N    = 4
# NGQB =  1
# NSQB =  2
# NLQB =  1

class singleGatesTest:
    def __init__(self):
        self.setting = {'total_qbit':'4',
                        'global_qbit':'1',
                        'thread_qbit':'2',
                        'local_qbit':'1',
                        'max_qbit':'38',
                        'state_paths':'./state/path1,./state/path2'}
        self.ini_path='test.ini'
        self.cir_path='cir_test'
        
        self.N    = 4
        self.NGQB =  1
        self.NSQB =  2
        self.NLQB =  1
        self.qubit_type = ["Global", "Thread", "Middle", "Local"]
        self.range_type = [range(1), range(1, 2), range(2, 3), range(3, 4)]

    def _test_H(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(4):
                H(circuit, i)
            H(circuit, x)
            create_circuit(circuit, self.cir_path)
            flag = self.execute(name)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def _test_S(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(4):
                H(circuit, i)
            S(circuit, x)
            create_circuit(circuit, self.cir_path)
            flag = self.execute(name)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def _test_T(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(4):
                H(circuit, i)
            T(circuit, x)
            create_circuit(circuit, self.cir_path)
            flag = self.execute(name)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def _test_X(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(4):
                if i == x: continue
                H(circuit, i)
            X(circuit, x)
            create_circuit(circuit, self.cir_path)
            flag = self.execute(name)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def _test_Y(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(4):
                H(circuit, i)
            Y(circuit, x)
            create_circuit(circuit, self.cir_path)
            flag = self.execute(name)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def _test_Z(self, name, x_range):
        flag = True
        for x in x_range:
            circuit=get_circuit()
            for i in range(4):
                H(circuit, i)
            Z(circuit, x)
            create_circuit(circuit, self.cir_path)
            flag = self.execute(name)
            if(not flag): break
        
        if(not flag):
            print("[X]", name, ": not pass under 1e-9", flush=True)
            print("===========================")
        return flag

    def execute(self, name):
        os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path} >> /dev/null")
        # os.system(f"../qSim.out -i {self.ini_path} -c {self.cir_path}")
        return simple_test(name, False, self.cir_path, "../path/set7.txt", self.N, self.NGQB, True)

    def test(self):
        create_ini(self.setting, self.ini_path)
        flag = True
        for i in range(4):
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
        
        for i in range(4):
            q0_type = self.qubit_type[i]
            test_name = f"{q0_type}"
            flag = self._test_S(test_name, self.range_type[i])
            if not flag:    break
        if(flag):
            print("[PASS]", "S Test", ": match with qiskit under 1e-9", flush=True)
            print("===========================")
        else:
            print("[x]", "S Test", ": not pass under 1e-9", flush=True)
            print("===========================")
            return

        for i in range(4):
            q0_type = self.qubit_type[i]
            test_name = f"{q0_type}"
            flag = self._test_T(test_name, self.range_type[i])
            if not flag:    break 
        if(flag):
            print("[PASS]", "T Test", ": match with qiskit under 1e-9", flush=True)
            print("===========================")
        else:
            print("[x]", "T Test", ": not pass under 1e-9", flush=True)
            print("===========================")
            return

        for i in range(4):
            q0_type = self.qubit_type[i]
            test_name = f"{q0_type}"
            flag = self._test_X(test_name, self.range_type[i])
            if not flag:    break 
        if(flag):
            print("[PASS]", "X Test", ": match with qiskit under 1e-9", flush=True)
            print("===========================")
        else:
            print("[x]", "X Test", ": not pass under 1e-9", flush=True)
            print("===========================")
            return

        for i in range(4):
            q0_type = self.qubit_type[i]
            test_name = f"{q0_type}"
            flag = self._test_Y(test_name, self.range_type[i])
            if not flag:    break 
        if(flag):
            print("[PASS]", "Y Test", ": match with qiskit under 1e-9", flush=True)
            print("===========================")
        else:
            print("[x]", "Y Test", ": not pass under 1e-9", flush=True)
            print("===========================")
            return

        for i in range(4):
            q0_type = self.qubit_type[i]
            test_name = f"{q0_type}"
            flag = self._test_Z(test_name, self.range_type[i])
            if not flag:    break 
        if(flag):
            print("[PASS]", "Z Test", ": match with qiskit under 1e-9", flush=True)
            print("===========================")
        else:
            print("[x]", "Z Test", ": not pass under 1e-9", flush=True)
            print("===========================")
            return

        if(flag):
            print("[PASS]", "singleGatesTest", ": match with qiskit under 1e-9", flush=True)
            print("===========================")
        return flag
