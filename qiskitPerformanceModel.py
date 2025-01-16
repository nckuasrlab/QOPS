import os
import pandas as pd
import pickle
import math
import random
import sys
from qiskit import QuantumCircuit
from qiskit.circuit.library import UnitaryGate, DiagonalGate
from qiskit_aer import AerSimulator
from dotenv import load_dotenv
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_percentage_error

def unitary_matrix(number):
    matrix = []
    for i in range(int(pow(2, number))):
        row = []
        for j in range(int(pow(2, number))):
            if i == j:
                row.append(1)
            else:
                row.append(0)
        matrix.append(row)
    return matrix

def diagonal_matrix(number):
    matrix = []
    for i in range(int(pow(2, number))):
        matrix.append(1)
    return matrix

U3 = UnitaryGate(unitary_matrix(3))
U2 = UnitaryGate(unitary_matrix(2))
D1 = DiagonalGate(diagonal_matrix(1))
D2 = DiagonalGate(diagonal_matrix(2))
D3 = DiagonalGate(diagonal_matrix(3))

def gen_microbenchmark(TIMES, TOTAL_QUBIT, TEST_SIZE, gate_list):
    f_log = open("./log/qiskit_microbenchmark_result.csv", "a")
    for gate in gate_list:
        for total_qubit in range(TOTAL_QUBIT, TOTAL_QUBIT+TEST_SIZE):
            for target_qubit_1 in range (total_qubit):
                circuit = QuantumCircuit(total_qubit)
                target_qubit = []
                while target_qubit_1 not in target_qubit:
                    target_qubit = random.sample(range(total_qubit), 3)
                    target_qubit.sort()
                if target_qubit.index(target_qubit_1) == 0:
                    target_qubit_2 = target_qubit[1]
                    target_qubit_3 = target_qubit[2]
                elif target_qubit.index(target_qubit_1) == 1:
                    target_qubit_2 = target_qubit[0]
                    target_qubit_3 = target_qubit[2]
                elif target_qubit.index(target_qubit_1) == 2:
                    target_qubit_2 = target_qubit[0]
                    target_qubit_3 = target_qubit[1]
                for i in range(TIMES):
                    if gate == "H":
                        circuit.h(target_qubit_1)
                    elif gate == "X":
                        circuit.x(target_qubit_1)
                    elif gate == "RX":
                        circuit.rx(3.141592653589793, target_qubit_1)
                    elif gate == "RY":
                        circuit.ry(3.141592653589793, target_qubit_1)
                    elif gate == "RZ":
                        circuit.rz(3.141592653589793, target_qubit_1)
                    elif gate == "U1":
                        circuit.u(3.141592653589793, 3.141592653589793, 3.141592653589793, target_qubit_1)
                    elif gate == "CX":
                        circuit.cx(target_qubit_1, target_qubit_2)
                    elif gate == "CZ":
                        circuit.cz(target_qubit_1, target_qubit_2)
                    elif gate == "CP":
                        circuit.cp(3.141592653589793, target_qubit_1, target_qubit_2)
                    elif gate == "RZZ":
                        circuit.rzz(3.141592653589793, target_qubit_1, target_qubit_2)
                    elif gate == "U2":
                        circuit.append(U2, [target_qubit_1, target_qubit_2])
                    elif gate == "U3":
                        circuit.append(U3, [target_qubit_1, target_qubit_2, target_qubit_3])
                    elif gate == "D1":
                        circuit.append(D1, [target_qubit_1])
                    elif gate == "D2":
                        circuit.append(D2, [target_qubit_1, target_qubit_2])
                    elif gate == "D3":
                        circuit.append(D3, [target_qubit_1, target_qubit_2, target_qubit_3])
                circuit.measure_all()    
                simulator = AerSimulator(
                    method = 'statevector',
                    fusion_enable = False,
                )
                job = simulator.run(circuit)
                res = job.result()
                # ms
                data = float(res.metadata['time_taken_execute'])/TIMES*1000
                f_log.write(gate + ", " + str(total_qubit) + ", " + str(target_qubit_1) + ", " + str(data) +"\n")
                f_log.flush()
    f_log.close()

if __name__ == '__main__':
    load_dotenv()
    TIMES = int(os.getenv("TIMES"))
    TOTAL_QUBIT = int(os.getenv("TOTAL_QUBIT"))
    TEST_SIZE = int(os.getenv("TEST_SIZE"))
    RUN_MICROBENCHMARK = os.getenv("RUN_MICROBENCHMARK")
    RE_TRAIN = os.getenv("RE_TRAIN")
    MODE = os.getenv("MODE")
    gate_list = ["H", "X", "RX", "RY", "RZ", "U1", "CX", "CZ", "CP", "RZZ", "U2", "U3", "D1", "D2", "D3"]

    if RUN_MICROBENCHMARK == "1":
        os.system("rm ./log/qiskit_microbenchmark_result.csv")
        gen_microbenchmark(TIMES, TOTAL_QUBIT, TEST_SIZE, gate_list)

    if MODE == "0":
        # input data
        input_gate_type = sys.argv[1]
        input_total_qubit = sys.argv[2]
        input_target_qubit = sys.argv[3]
        df_input = pd.DataFrame([[input_gate_type, input_total_qubit, input_target_qubit]], columns=['gate_type', 'total_qubit', 'target_qubit'])
        df_input['gate_type'] = pd.Categorical(df_input['gate_type'], categories=gate_list)
        df_input = pd.get_dummies(df_input, columns=['gate_type'])
    elif MODE == "1":
        input_total_qubit = sys.argv[1]
    
    if RE_TRAIN == "1":
        # load microbrnchmark result
        column_names = ['gate_type', 'total_qubit', 'target_qubit', 'execution_time']
        df_microbenchmark = pd.read_csv('./log/qiskit_microbenchmark_result.csv', names=column_names)

        # data preprocessing
        df_microbenchmark['gate_type'] = pd.Categorical(df_microbenchmark['gate_type'], categories=gate_list)
        df_microbenchmark = pd.get_dummies(df_microbenchmark, columns=['gate_type'])
        x = df_microbenchmark.drop(labels=['execution_time'], axis=1)
        y = df_microbenchmark['execution_time']
        x_train, x_test, y_train, y_test = train_test_split(x, y, test_size = 0.2)
        sc = StandardScaler()
        x_train = sc.fit_transform(x_train)
        x_test = sc.transform(x_test)

        # model
        model = RandomForestRegressor(n_estimators=500)
        model.fit(x_train, y_train)
        y_test_predict = model.predict(x_test)
        print(mean_absolute_percentage_error(y_test, y_test_predict))

        # store model and scaler
        with open("./model/gate_model.pkl", "wb") as f:
            pickle.dump(model, f)
        with open("./model/scaler.pkl", "wb") as f:
            pickle.dump(sc, f)
    else:
        with open("./model/gate_model.pkl", "rb") as f:
            model = pickle.load(f)
        with open("./model/scaler.pkl", "rb") as f:
            sc = pickle.load(f)
        
    if MODE == "0":
        df_input = sc.transform(df_input)
        print(model.predict(df_input)[0])
    elif MODE == "1":
        os.system("rm ./log/gate_exe_time.csv")
        f_exe = open('./log/gate_exe_time.csv', 'w')
        for gate in gate_list:
            for target_qubit in range(int(input_total_qubit)):
                df_input = pd.DataFrame([[gate, input_total_qubit, target_qubit]], columns=['gate_type', 'total_qubit', 'target_qubit'])
                df_input['gate_type'] = pd.Categorical(df_input['gate_type'], categories=gate_list)
                df_input = pd.get_dummies(df_input, columns=['gate_type'])
                df_input = sc.transform(df_input)
                f_exe.write(str(model.predict(df_input)[0])+"\n")
        f_exe.close()