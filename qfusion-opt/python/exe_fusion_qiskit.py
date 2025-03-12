import os
import subprocess
from math import pow

from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit_aer import AerSimulator


def unitary_matrix(num_qubits):
    matrix = []
    for i in range(int(pow(2, num_qubits))):
        row = []
        for j in range(int(pow(2, num_qubits))):
            if i == j:
                row.append(1 + 0.0j)
            else:
                row.append(0)
        matrix.append(row)
    return matrix


def diagonal_matrix(num_qubits):
    matrix = []
    for i in range(int(pow(2, num_qubits))):
        matrix.append(1)
    return matrix


U5 = UnitaryGate(unitary_matrix(5))
U4 = UnitaryGate(unitary_matrix(4))
U3 = UnitaryGate(unitary_matrix(3))
U2 = UnitaryGate(unitary_matrix(2))
U1 = UnitaryGate(unitary_matrix(1))
D5 = DiagonalGate(diagonal_matrix(5))
D4 = DiagonalGate(diagonal_matrix(4))
D3 = DiagonalGate(diagonal_matrix(3))
D2 = DiagonalGate(diagonal_matrix(2))
D1 = DiagonalGate(diagonal_matrix(1))


def gen_qiskit_fusion(file_name, total_qubit, mode):
    time_overhead = subprocess.run(
        [
            "python3",
            "./QiskitFusion/QiskitFusion.py",
            "./circuit/" + file_name,
            str(total_qubit),
            mode,
        ],
        capture_output=True,
        text=True,
    )
    out = os.system("mv ./1.txt ./qiskitFusionCircuit/GBG_tmp.txt >/dev/null 2>&1")
    if mode == "dynamic_qiskit":
        output_qiskit_file = open("./qiskitFusionCircuit/GBG_" + file_name, "w")
    elif mode == "static_qiskit":
        output_qiskit_file = open("./qiskitFusionCircuit/c_GBG_" + file_name, "w")
    if out == 0:
        tmp_file = open("./qiskitFusionCircuit/GBG_tmp.txt", "r")
        lines = tmp_file.readlines()
        tmp_file.close()
        for line in lines:
            line = line.split()
            if line[0] == "unitary-5":
                output_qiskit_file.write(
                    "U4 "
                    + line[1]
                    + " "
                    + line[2]
                    + " "
                    + line[3]
                    + " "
                    + line[4]
                    + " "
                    + line[5]
                )
                for i in range(int(pow(2, 5) * pow(2, 5))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "unitary-4":
                output_qiskit_file.write(
                    "U4 " + line[1] + " " + line[2] + " " + line[3] + " " + line[4]
                )
                for i in range(int(pow(2, 4) * pow(2, 4))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "unitary-3":
                output_qiskit_file.write(
                    "U3 " + line[1] + " " + line[2] + " " + line[3]
                )
                for i in range(int(pow(2, 3) * pow(2, 3))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "unitary-2":
                output_qiskit_file.write("U2 " + line[1] + " " + line[2])
                for i in range(int(pow(2, 2) * pow(2, 2))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "diagonal-5":
                output_qiskit_file.write(
                    "D5 "
                    + line[1]
                    + " "
                    + line[2]
                    + " "
                    + line[3]
                    + " "
                    + line[4]
                    + " "
                    + line[5]
                )
                for i in range(int(pow(2, 5))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "diagonal-4":
                output_qiskit_file.write(
                    "D4 " + line[1] + " " + line[2] + " " + line[3] + " " + line[4]
                )
                for i in range(int(pow(2, 4))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "diagonal-3":
                output_qiskit_file.write(
                    "D3 " + line[1] + " " + line[2] + " " + line[3]
                )
                for i in range(int(pow(2, 3))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "diagonal-2":
                output_qiskit_file.write("D2 " + line[1] + " " + line[2])
                for i in range(int(pow(2, 2))):
                    output_qiskit_file.write(" 3.141596")
                output_qiskit_file.write("\n")
            elif line[0] == "cz-2":
                output_qiskit_file.write("CZ " + line[1] + " " + line[2] + "\n")
                output_qiskit_file.write("")
            elif line[0] == "rzz-2":
                output_qiskit_file.write(
                    "RZZ " + line[1] + " " + line[2] + " 3.141596\n"
                )
            elif line[0] == "h-1":
                output_qiskit_file.write("H " + line[1] + " \n")
            else:
                raise Exception(f"{line[0]} is not supported ({line})")
    else:
        tmp_file = open("./qiskitFusionCircuit/tmp.txt", "r")
        lines = tmp_file.readlines()
        gate_number_qiskit = gate_number_qiskit + len(lines)
        for line in lines:
            output_qiskit_file.write(line)
        tmp_file.close()
    output_qiskit_file.close()
    return time_overhead.stdout


def exe_circuit(file_name: str, total_qubit, open_fusion, max_fusion_qubits, get_info):
    circuit = QuantumCircuit(total_qubit)
    fusion_file = open(file_name, "r")
    lines = fusion_file.readlines()
    fusion_file.close()
    line_count = 0
    for line in lines:
        line_count = line_count + 1
        line = line.split()
        if line[0] == "U1":
            if len(line) == 6:
                circuit.append(U1, [int(line[1])])
            else:
                circuit.u(float(line[2]), float(line[3]), float(line[4]), int(line[1]))
        elif line[0] == "U2":
            circuit.append(U2, [int(line[1]), int(line[2])])
        elif line[0] == "U3":
            circuit.append(U3, [int(line[1]), int(line[2]), int(line[3])])
        elif line[0] == "U4":
            circuit.append(U4, [int(line[1]), int(line[2]), int(line[3]), int(line[4])])
        elif line[0] == "U5":
            circuit.append(
                U5,
                [int(line[1]), int(line[2]), int(line[3]), int(line[4]), int(line[5])],
            )
        elif line[0] == "D1":
            circuit.append(D1, [int(line[1])])
        elif line[0] == "D2":
            circuit.append(D2, [int(line[1]), int(line[2])])
        elif line[0] == "D3":
            circuit.append(D3, [int(line[1]), int(line[2]), int(line[3])])
        elif line[0] == "D4":
            circuit.append(
                D4,
                [int(line[1]), int(line[2]), int(line[3]), int(line[4]), int(line[5])],
            )
        elif line[0] == "D5":
            circuit.append(
                D5,
                [
                    int(line[1]),
                    int(line[2]),
                    int(line[3]),
                    int(line[4]),
                    int(line[5]),
                    int(line[6]),
                ],
            )
        elif line[0] == "CP":
            circuit.cp(float(line[3]), int(line[1]), int(line[2]))
        elif line[0] == "H":
            circuit.h(int(line[1]))
        elif line[0] == "X":
            circuit.x(int(line[1]))
        elif line[0] == "CX":
            circuit.cx(int(line[1]), int(line[2]))
        elif line[0] == "CZ":
            circuit.cz(int(line[1]), int(line[2]))
        elif line[0] == "RX":
            circuit.rx(float(line[2]), int(line[1]))
        elif line[0] == "RY":
            circuit.ry(float(line[2]), int(line[1]))
        elif line[0] == "RZ":
            circuit.rz(float(line[2]), int(line[1]))
        elif line[0] == "RZZ":
            circuit.rzz(float(line[3]), int(line[1]), int(line[2]))
        else:
            raise Exception(f"{line[0]} is not supported ({line})")

    circuit.measure_all()
    print("gate number:", line_count)

    simulator = AerSimulator(
        method="statevector",
        fusion_enable=True,
        fusion_verbose=True,
        fusion_max_qubit=max_fusion_qubits,
    )

    # t1 = time.perf_counter_ns()
    res = simulator.run(circuit).result()
    # t2 = time.perf_counter_ns()
    # print(f"Elapsed time: {(t2-t1)/1e9} s")
    # sample_time = res.results[0].metadata["sample_measure_time"]
    # print(f"Sample time: {sample_time} s")
    simulation_time = res.results[0].metadata["time_taken"]
    # print(f"simulation time: {simulation_time} s")

    qiskit_result = res.results[0].metadata["fusion"]
    if open_fusion == True and get_info == True:
        print(
            "qiskit fusion with diagonal fusion, fusion time: "
            + str(qiskit_result["time_taken"])
        )
        print(
            "qiskit fusion with diagonal fusion, gate number: "
            + str(len(qiskit_result["output_ops"]) - total_qubit)
        )
        with open("./qiskitFusionCircuit/L_GBG_" + file_name.split("/")[-1], "w") as f:
            for gate in qiskit_result["output_ops"]:
                if gate["name"] == "measure":
                    continue
                f.write(gate["name"] + str(len(gate["qubits"])) + " ")
                for qubit in gate["qubits"]:
                    f.write(str(qubit) + " ")
                f.write("\n")
    return simulation_time


if __name__ == "__main__":
    max_fusion_qubits = 3
    total_qubit = 32
    file_name_list = ["sc", "vc", "hs", "qv", "bv", "qft", "qaoa", "ising"]

    for i, file_name in enumerate(file_name_list):
        file_name = file_name + str(total_qubit) + ".txt"
        print("==================================================================")
        print(file_name)

        # origin
        print(
            exe_circuit(
                "./circuit/" + file_name, total_qubit, True, max_fusion_qubits, False
            )
        )

        # static Qiskit
        print(
            "static Qiskit: ",
            gen_qiskit_fusion(file_name, total_qubit, "static_qiskit"),
            end="",
        )
        print(
            exe_circuit(
                "./qiskitFusionCircuit/c_GBG_" + file_name,
                total_qubit,
                True,
                max_fusion_qubits,
                False,
            )
        )

        # dynamic Qiskit
        print(
            "dynamic Qiskit: ",
            gen_qiskit_fusion(file_name, total_qubit, "dynamic_qiskit"),
            end="",
        )
        print(
            exe_circuit(
                "./qiskitFusionCircuit/GBG_" + file_name,
                total_qubit,
                True,
                max_fusion_qubits,
                False,
            )
        )

        # static DFGC
        result = subprocess.run(
            [
                "./fusion",
                "./circuit/" + file_name,
                "./fusionCircuit/c_GBG_" + file_name,
                str(max_fusion_qubits),
                str(total_qubit),
                "5",
            ],
            capture_output=True,
            text=True,
        )
        print("static DFGC time: ", result.stdout.split()[-1])
        print(
            exe_circuit(
                "./fusionCircuit/c_GBG_" + file_name,
                total_qubit,
                True,
                max_fusion_qubits,
                False,
            )
        )

        # dynamic DFGC
        result = subprocess.run(
            [
                "./fusion",
                "./circuit/" + file_name,
                "./fusionCircuit/GBG_" + file_name,
                str(max_fusion_qubits),
                str(total_qubit),
                "8",
            ],
            capture_output=True,
            text=True,
        )
        print("dynamic DFGC time: ", result.stdout.split()[-1])
        print(
            exe_circuit(
                "./fusionCircuit/GBG_" + file_name,
                total_qubit,
                True,
                max_fusion_qubits,
                False,
            )
        )
