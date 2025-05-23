import subprocess
import os
from time import perf_counter

max_fusion_qubits = 3
chunk_size = 18

def gen_qiskit_fusion(file_name, total_qubit, mode):
    if mode == "static_qiskit":
        output_qiskit_file = open("./qiskitFusionCircuit/cnf_" + file_name, 'w')
    else:
        output_qiskit_file = open("./qiskitFusionCircuit/nf_" + file_name, 'w')
    gate_number = 0

    out = subprocess.run(["python3", "./QiskitFusion/qiskit_fusion.py",  "./circuit/" + file_name, str(total_qubit), mode], capture_output=True, text=True)
    time_overhead = float(out.stdout)
    os.system("rm ./qiskitFusionCircuit/tmp.txt")
    out = os.system("mv ./1.txt ./qiskitFusionCircuit/tmp.txt >/dev/null 2>&1")
    if out == 0:
        tmp_file = open("./qiskitFusionCircuit/tmp.txt", "r")
        lines = tmp_file.readlines()
        gate_number = len(lines)
        tmp_file.close()
        for line in lines:
            line = line.split()
            if line[0] == "unitary-3":
                output_qiskit_file.write("U3 " + line[1] + " " + line[2] + " " + line[3])
                for i in range(0, 64):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "unitary-2":
                output_qiskit_file.write("U2 " + line[1] + " " + line[2])
                for i in range(0, 16):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "cz-2":
                output_qiskit_file.write("CZ " + line[1] + " " + line[2] + "\n")
                output_qiskit_file.write("")
            elif line[0] == "rzz-2":
                output_qiskit_file.write("RZZ " + line[1] + " " + line[2] + " 3.141596\n")
            elif line[0] == "cp-2":
                output_qiskit_file.write("CP " + line[1] + " " + line[2] + " 3.141596\n")
            elif line[0] == "ry-1":
                output_qiskit_file.write("RY " + line[1] + " 3.141596\n")
            elif line[0] == "cx-2":
                output_qiskit_file.write("CX " + line[1] + " " + line[2] + "\n")
            else:
                print("other gate")
                print(line[0])
    else:
        tmp_file = open("./qiskitFusionCircuit/tmp.txt", 'r')
        lines = tmp_file.readlines()
        output_qiskit_file.write(str(len(lines)) + "\n")
        gate_number = gate_number + len(lines)
        for line in lines:
            output_qiskit_file.write(line)
        tmp_file.close()
    output_qiskit_file.close()
    return [gate_number, time_overhead]


if __name__ == '__main__':
    times = 1
    total_qubit = 32
    file_name_list = ["sc", "vc", "hs", "qv", "bv", "qft", "qaoa", "ising"]

    for file_name in file_name_list:
        file_name = file_name + str(total_qubit) + ".txt"
        print(file_name)
        
        # gen circuit
        # origin
        subprocess.run(["./finder/finder", "./circuit/" + file_name, "./subCircuit/" + file_name, str(chunk_size)], capture_output=True, text=True)
        # static Qiskit
        result = gen_qiskit_fusion(file_name, total_qubit, "static_qiskit")
        print("static Qiskit")
        print("gate number:", result[0])
        print("fusion time:", result[1])
        subprocess.run(["./finder/finder", "./qiskitFusionCircuit/cnf_" + file_name, "./qiskitFusionCircuit/cnf_" + file_name, str(chunk_size)], capture_output=True)
        
        # dynamic Qiskit
        result = gen_qiskit_fusion(file_name, total_qubit, "dynamic_qiskit")
        print("dynamic Qiskit")
        print("gate number:", result[0])
        print("fusion time:", result[1])
        subprocess.run(["./finder/finder", "./qiskitFusionCircuit/nf_" + file_name, "./qiskitFusionCircuit/nf_" + file_name, str(chunk_size)], capture_output=True)
        # static DFGC
        output = subprocess.run(["./fusion", "./circuit/" + file_name, "./fusionCircuit/cnf_" + file_name, str(max_fusion_qubits), str(total_qubit), "3"], capture_output=True, text=True)
        print("static DFGC")
        with open("./fusionCircuit/cnf_" + file_name, 'r') as file:
            line_count = sum(1 for line in file)
            print("gate number:", line_count)
        print("fusion time:", output.stdout.split()[-1])
        subprocess.run(["./finder/finder", "./fusionCircuit/cnf_" + file_name, "./fusionCircuit/cnf_" + file_name, str(chunk_size)], capture_output=True)
        
        # dynamic DFGC
        output = subprocess.run(["./fusion", "./circuit/" + file_name, "./fusionCircuit/nf_" + file_name, str(max_fusion_qubits), str(total_qubit), "4"], capture_output=True, text=True)
        print("dynamic DFGC")
        with open("./fusionCircuit/nf_" + file_name, 'r') as file:
            line_count = sum(1 for line in file)
            print("gate number:", line_count)
        print("fusion time:", output.stdout.split()[-1])
        subprocess.run(["./finder/finder", "./fusionCircuit/nf_" + file_name, "./fusionCircuit/nf_" + file_name, str(chunk_size)], capture_output=True)

        # set ini file
        f = open("sub_cpu.ini", "w")
        f.write("[system]\ntotal_qbit=" + str(total_qubit) + "\nfile_qbit=6\nchunk_qbit=" + str(chunk_size) + "\nrunner_type=MEM\nis_subcircuit=1")
        f.close()
        
        # origin
        time = 0
        for i in range(times):
            output = subprocess.run(["./cpu/Quokka", "-i", "sub_cpu.ini", "-c", "./subCircuit/" + file_name], capture_output=True, text=True)
            time = time + float(output.stdout.split()[-1].split("s")[0])
        print("origin: ", time / times)
        # static Qiskit
        time = 0
        for i in range(times):
            output = subprocess.run(["./cpu/Quokka", "-i", "sub_cpu.ini", "-c", "./qiskitFusionCircuit/cnf_" + file_name], capture_output=True, text=True)
            time = time + float(output.stdout.split()[-1].split("s")[0])
        print("static Qiskit: ", time / times)
        
        # dynamic Qiskit
        time = 0
        for i in range(times):
            output = subprocess.run(["./cpu/Quokka", "-i", "sub_cpu.ini", "-c", "./qiskitFusionCircuit/nf_" + file_name], capture_output=True, text=True)
            time = time + float(output.stdout.split()[-1].split("s")[0])
        print("dynamic Qiskit: ", time / times)
        # static DFGC
        time = 0
        for i in range(times):
            output = subprocess.run(["./cpu/Quokka", "-i", "sub_cpu.ini", "-c", "./fusionCircuit/cnf_" + file_name], capture_output=True, text=True)
            time = time + float(output.stdout.split()[-1].split("s")[0])
        print("static DFGC: ", time / times)
        
        # dynamic DFGC
        time = 0
        for i in range(times):
            output = subprocess.run(["./cpu/Quokka", "-i", "sub_cpu.ini", "-c", "./fusionCircuit/nf_" + file_name], capture_output=True, text=True)
            time = time + float(output.stdout.split()[-1].split("s")[0])
        print("DFGC dynamic: ", time / times)

        print("========================================================")
