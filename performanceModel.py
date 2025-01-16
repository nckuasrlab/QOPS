import os
import subprocess
import pandas as pd
import pickle
import math
import random
import sys
from dotenv import load_dotenv
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_percentage_error

def set_ini_file(total_qbit, FILE_QUBIT, size, RUNNER_TYPE):
    f = open("sub_cpu.ini", "w")
    if RUNNER_TYPE == "MEM":
        f.write("[system]\ntotal_qbit=" + str(total_qbit) + "\nfile_qbit=" + str(FILE_QUBIT) + "\nchunk_qbit=" + str(size) + "\nrunner_type=MEM\nis_subcircuit=1")
    elif RUNNER_TYPE == "IO":
        f.write("[system]\ntotal_qbit=" + str(total_qbit) + "\nfile_qbit=" + str(FILE_QUBIT) + "\nchunk_qbit=" + str(size) + "\nrunner_type=IO\nis_subcircuit=1\nstate_paths=/mnt/nvme/card0/0/path0,/mnt/nvme/card0/0/path1,/mnt/nvme/card0/0/path2,/mnt/nvme/card0/0/path3,/mnt/nvme/card0/0/path4,/mnt/nvme/card0/0/path5,/mnt/nvme/card0/0/path6,/mnt/nvme/card0/0/path7,/mnt/nvme/card0/1/path0,/mnt/nvme/card0/1/path1,/mnt/nvme/card0/1/path2,/mnt/nvme/card0/1/path3,/mnt/nvme/card0/1/path4,/mnt/nvme/card0/1/path5,/mnt/nvme/card0/1/path6,/mnt/nvme/card0/1/path7,/mnt/nvme/card0/2/path0,/mnt/nvme/card0/2/path1,/mnt/nvme/card0/2/path2,/mnt/nvme/card0/2/path3,/mnt/nvme/card0/2/path4,/mnt/nvme/card0/2/path5,/mnt/nvme/card0/2/path6,/mnt/nvme/card0/2/path7,/mnt/nvme/card0/3/path0,/mnt/nvme/card0/3/path1,/mnt/nvme/card0/3/path2,/mnt/nvme/card0/3/path3,/mnt/nvme/card0/3/path4,/mnt/nvme/card0/3/path5,/mnt/nvme/card0/3/path6,/mnt/nvme/card0/3/path7,/mnt/nvme/card1/0/path0,/mnt/nvme/card1/0/path1,/mnt/nvme/card1/0/path2,/mnt/nvme/card1/0/path3,/mnt/nvme/card1/0/path4,/mnt/nvme/card1/0/path5,/mnt/nvme/card1/0/path6,/mnt/nvme/card1/0/path7,/mnt/nvme/card1/1/path0,/mnt/nvme/card1/1/path1,/mnt/nvme/card1/1/path2,/mnt/nvme/card1/1/path3,/mnt/nvme/card1/1/path4,/mnt/nvme/card1/1/path5,/mnt/nvme/card1/1/path6,/mnt/nvme/card1/1/path7,/mnt/nvme/card1/2/path0,/mnt/nvme/card1/2/path1,/mnt/nvme/card1/2/path2,/mnt/nvme/card1/2/path3,/mnt/nvme/card1/2/path4,/mnt/nvme/card1/2/path5,/mnt/nvme/card1/2/path6,/mnt/nvme/card1/2/path7,/mnt/nvme/card1/3/path0,/mnt/nvme/card1/3/path1,/mnt/nvme/card1/3/path2,/mnt/nvme/card1/3/path3,/mnt/nvme/card1/3/path4,/mnt/nvme/card1/3/path5,/mnt/nvme/card1/3/path6,/mnt/nvme/card1/3/path7\nis_directIO=0")
    elif RUNNER_TYPE == "DirectIO":
        f.write("[system]\ntotal_qbit=" + str(total_qbit) + "\nfile_qbit=" + str(FILE_QUBIT) + "\nchunk_qbit=" + str(size) + "\nrunner_type=DirectIO\nis_subcircuit=1\nstate_paths=/mnt/nvme/card0/0/path0,/mnt/nvme/card0/0/path1,/mnt/nvme/card0/0/path2,/mnt/nvme/card0/0/path3,/mnt/nvme/card0/0/path4,/mnt/nvme/card0/0/path5,/mnt/nvme/card0/0/path6,/mnt/nvme/card0/0/path7,/mnt/nvme/card0/1/path0,/mnt/nvme/card0/1/path1,/mnt/nvme/card0/1/path2,/mnt/nvme/card0/1/path3,/mnt/nvme/card0/1/path4,/mnt/nvme/card0/1/path5,/mnt/nvme/card0/1/path6,/mnt/nvme/card0/1/path7,/mnt/nvme/card0/2/path0,/mnt/nvme/card0/2/path1,/mnt/nvme/card0/2/path2,/mnt/nvme/card0/2/path3,/mnt/nvme/card0/2/path4,/mnt/nvme/card0/2/path5,/mnt/nvme/card0/2/path6,/mnt/nvme/card0/2/path7,/mnt/nvme/card0/3/path0,/mnt/nvme/card0/3/path1,/mnt/nvme/card0/3/path2,/mnt/nvme/card0/3/path3,/mnt/nvme/card0/3/path4,/mnt/nvme/card0/3/path5,/mnt/nvme/card0/3/path6,/mnt/nvme/card0/3/path7,/mnt/nvme/card1/0/path0,/mnt/nvme/card1/0/path1,/mnt/nvme/card1/0/path2,/mnt/nvme/card1/0/path3,/mnt/nvme/card1/0/path4,/mnt/nvme/card1/0/path5,/mnt/nvme/card1/0/path6,/mnt/nvme/card1/0/path7,/mnt/nvme/card1/1/path0,/mnt/nvme/card1/1/path1,/mnt/nvme/card1/1/path2,/mnt/nvme/card1/1/path3,/mnt/nvme/card1/1/path4,/mnt/nvme/card1/1/path5,/mnt/nvme/card1/1/path6,/mnt/nvme/card1/1/path7,/mnt/nvme/card1/2/path0,/mnt/nvme/card1/2/path1,/mnt/nvme/card1/2/path2,/mnt/nvme/card1/2/path3,/mnt/nvme/card1/2/path4,/mnt/nvme/card1/2/path5,/mnt/nvme/card1/2/path6,/mnt/nvme/card1/2/path7,/mnt/nvme/card1/3/path0,/mnt/nvme/card1/3/path1,/mnt/nvme/card1/3/path2,/mnt/nvme/card1/3/path3,/mnt/nvme/card1/3/path4,/mnt/nvme/card1/3/path5,/mnt/nvme/card1/3/path6,/mnt/nvme/card1/3/path7\nis_directIO=1")
    f.close()

def gen_microbenchmark(TIMES, TOTAL_QUBIT, CHUNK_SIZE, CHUNK_START, TEST_SIZE, RUNNER_TYPE, FILE_QUBIT, gate_list):
    f_log = open("./log/microbenchmark_result.csv", "a")
    for gate in gate_list:
        for total_qbit in range(TOTAL_QUBIT, TOTAL_QUBIT+TEST_SIZE):
            for size in range(CHUNK_START, CHUNK_SIZE+1):
                circuit_name = "./txt/" + gate + ".txt"
                f = open(circuit_name, "w")
                f.write(str(TIMES)+"\n")
                target = random.sample(range(size), 3)
                target.sort()
                if gate == "H" or gate == "X":
                    for i in range(TIMES):
                        f.write(gate + " " + str(target[0]) + "\n")
                elif gate == "RX" or gate == "RY" or gate == "RZ":
                    for i in range(TIMES):
                        f.write(gate + " " + str(target[0]) + " 3.141592653589793\n")
                elif gate == "U1":
                    for i in range(TIMES):
                        f.write(gate + " " + str(target[0]) + " 3.141592653589793 3.141592653589793 3.141592653589793\n")
                elif gate == "CX" or gate == "CZ":
                    for i in range(TIMES):
                        f.write(gate + " " + str(target[0]) + " " + str(target[1]) + "\n")
                elif gate == "CP" or gate == "RZZ":
                    for i in range(TIMES):
                        f.write(gate + " " + str(target[0]) + " " + str(target[1]) + " 3.141592653589793\n")
                elif gate == "U2":
                    for i in range(TIMES):
                        f.write(gate + " " + str(target[0]) + " " + str(target[1]))
                        for j in range(16):
                            f.write(" 3.141592653589793")
                        f.write("\n")
                elif gate == "U3":
                    for i in range(TIMES):
                        f.write(gate + " " + str(target[0]) + " " + str(target[1]) + " " + str(target[2]))
                        for j in range(64):
                            f.write(" 3.141592653589793")
                        f.write("\n")
                f.close()

                set_ini_file(total_qbit, FILE_QUBIT, size, RUNNER_TYPE)
                output = subprocess.run(["../cpu/Quokka", "-i", "sub_cpu.ini", "-c", circuit_name], capture_output=True, text=True)
                # ms
                data = float(output.stdout.split("\n")[-2].split("s")[0])/TIMES*1000
                f_log.write(gate + ", " + str(total_qbit) + ", " + str(size) + ", " + str(data) +"\n")
    f_log.close()

if __name__ == '__main__':
    # variable
    load_dotenv()
    TIMES = int(os.getenv("TIMES"))
    TOTAL_QUBIT = int(os.getenv("TOTAL_QUBIT"))
    CHUNK_SIZE = int(os.getenv("CHUNK_SIZE"))
    CHUNK_START = int(os.getenv("CHUNK_START"))
    TEST_SIZE = int(os.getenv("TEST_SIZE"))
    RUNNER_TYPE = os.getenv("RUNNER_TYPE")
    RUN_MICROBENCHMARK = os.getenv("RUN_MICROBENCHMARK")
    RE_TRAIN = os.getenv("RE_TRAIN")
    MODE = os.getenv("MODE")
    gate_list = ["H", "X", "RX", "RY", "RZ", "U1", "CX", "CZ", "CP", "RZZ", "U2", "U3"]

    cpu_info = subprocess.run(["lscpu | grep -E '^Core|^Socket'"], shell=True, capture_output=True, text=True).stdout.split("\n")
    core_number = int(cpu_info[0].split(" ")[-1])
    socket_number = int(cpu_info[1].split(" ")[-1])
    FILE_QUBIT = int(math.log(core_number*socket_number, 2))

    if RUN_MICROBENCHMARK == "1":
        os.system("rm ./log/microbenchmark_result.csv")
        os.system("rm ./txt/*")
        gen_microbenchmark(TIMES, TOTAL_QUBIT, CHUNK_SIZE, CHUNK_START, TEST_SIZE, RUNNER_TYPE, FILE_QUBIT, gate_list)

    if MODE == "0":
        # input data
        input_gate_type = sys.argv[1]
        input_total_qubit = sys.argv[2]
        input_chunk_size = sys.argv[3]
        df_input = pd.DataFrame([[input_gate_type, input_total_qubit, input_chunk_size]], columns=['gate_type', 'total_qubit', 'chunk_size'])
        df_input['gate_type'] = pd.Categorical(df_input['gate_type'], categories=gate_list)
        df_input = pd.get_dummies(df_input, columns=['gate_type'])
    elif MODE == "1":
        input_total_qubit = sys.argv[1]
        input_chunk_size = sys.argv[2]
    
    if RE_TRAIN == "1":
        # load microbrnchmark result
        column_names = ['gate_type', 'total_qubit', 'chunk_size', 'execution_time']
        df_microbenchmark = pd.read_csv('./log/microbenchmark_result.csv', names=column_names)

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
            df_input = pd.DataFrame([[gate, input_total_qubit, input_chunk_size]], columns=['gate_type', 'total_qubit', 'chunk_size'])
            df_input['gate_type'] = pd.Categorical(df_input['gate_type'], categories=gate_list)
            df_input = pd.get_dummies(df_input, columns=['gate_type'])
            df_input = sc.transform(df_input)
            f_exe.write(str(model.predict(df_input)[0])+"\n")
        f_exe.close()