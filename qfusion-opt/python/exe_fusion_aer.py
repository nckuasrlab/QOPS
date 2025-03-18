import os
import subprocess
import sys
from math import pow

import numpy as np
from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit_aer import AerSimulator
from scipy.linalg import polar


def diagonal_matrix(num_qubits):
    matrix = []
    for i in range(int(pow(2, num_qubits))):
        matrix.append(1)
    return matrix


def print_matrix(matrix):
    """Prints a square NumPy matrix with 3 decimal places."""
    rows, cols = matrix.shape
    if rows != cols:
        raise ValueError("Matrix must be square.")

    for row in matrix:
        formatted_row = ["{:.4f}".format(val) for val in row]
        print("[", " ".join(formatted_row), "]")


def read_unitary_matrix(num_qubits, matrix_1d):
    dim = int(pow(2, num_qubits))
    matrix = []
    for i in range(dim):
        row = []
        for j in range(0, dim * 2, 2):
            row.append(
                float(matrix_1d[i * dim * 2 + j])
                + float(matrix_1d[i * dim * 2 + j + 1]) * 1j
            )
        matrix.append(row)
    # Projecting onto the nearest unitary matrix (Polar Decomposition)
    matrix = np.array(matrix, dtype=complex)
    U, P = polar(matrix)
    return U


D5 = DiagonalGate(diagonal_matrix(5))
D4 = DiagonalGate(diagonal_matrix(4))
D3 = DiagonalGate(diagonal_matrix(3))
D2 = DiagonalGate(diagonal_matrix(2))
D1 = DiagonalGate(diagonal_matrix(1))


def gen_qiskit_fusion(filename, total_qubit, fusion_method):
    fusion_process = subprocess.run(
        [
            "python",
            "./QiskitFusion/qiskit_fusion.py",
            "./circuit/" + filename,
            str(total_qubit),
            fusion_method,
            "-d",
            "./log/gate_exe_time.csv",
            "-o",
            "./qiskitFusionCircuit/fused_tmp.txt",
        ],
        capture_output=True,
        text=True,
    )
    output_qiskit_file = open(
        f"./qiskitFusionCircuit/fused_{fusion_method}_{filename}", "w"
    )
    if fusion_process.returncode != 0:
        print("ERROR:", fusion_process.returncode)
        print(fusion_process.stdout)
        print(fusion_process.stderr)
        sys.exit(1)
    else:
        tmp_file = open("./qiskitFusionCircuit/fused_tmp.txt", "r")
        lines = tmp_file.readlines()
        tmp_file.close()
        for line in lines:
            line = line.split()
            if line[0] == "unitary-5":
                assert len(line[(5 + 1) :]) == int(pow(2, 5) * pow(2, 5)) * 2
                output_qiskit_file.write(f"U5 {" ".join(line[1:])}\n")
            elif line[0] == "unitary-4":
                assert len(line[(4 + 1) :]) == int(pow(2, 4) * pow(2, 4)) * 2
                output_qiskit_file.write(f"U4 {" ".join(line[1:])}\n")
            elif line[0] == "unitary-3":
                assert len(line[(3 + 1) :]) == int(pow(2, 3) * pow(2, 3)) * 2
                output_qiskit_file.write(f"U3 {" ".join(line[1:])}\n")
            elif line[0] == "unitary-2":
                assert len(line[(2 + 1) :]) == int(pow(2, 2) * pow(2, 2)) * 2
                output_qiskit_file.write(f"U2 {" ".join(line[1:])}\n")
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

    output_qiskit_file.close()
    return fusion_process.stdout


def load_circuit(filename: str, total_qubit: int) -> QuantumCircuit:
    circuit = QuantumCircuit(total_qubit)
    fusion_file = open(filename, "r")
    lines = fusion_file.readlines()
    fusion_file.close()
    line_count = 0
    for line in lines:
        line_count = line_count + 1
        line = line.split()
        if line[0] == "U1":
            if len(line) == 5:
                circuit.u(float(line[2]), float(line[3]), float(line[4]), int(line[1]))
            else:
                gate_qubit = 1
                mat = read_unitary_matrix(gate_qubit, line[(gate_qubit + 1) :])
                circuit.append(
                    UnitaryGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
                )
        elif line[0] == "U2":
            gate_qubit = 2
            mat = read_unitary_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                UnitaryGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0] == "U3":
            gate_qubit = 3
            mat = read_unitary_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                UnitaryGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0] == "U4":
            gate_qubit = 4
            mat = read_unitary_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                UnitaryGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0] == "U5":
            gate_qubit = 5
            mat = read_unitary_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                UnitaryGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
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

    return circuit


def exe_circuit(
    circuit: QuantumCircuit,
    fusion_method: str,
    max_fusion_qubits: int,
    open_fusion: bool,
    get_info: bool,
):
    simulator = AerSimulator(
        method="statevector",
        seed_simulator=0,
        fusion_enable=open_fusion,
        fusion_verbose=open_fusion,
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
    log_filename = (
        f"./qiskitFusionCircuit/aer_{fusion_method}_{os.path.basename(filename)}.log"
    )
    with open(log_filename, "w") as f:
        f.write(str(res) + "\n")
    if open_fusion and get_info and qiskit_result["applied"]:
        print(
            "qiskit fusion with diagonal fusion, fusion time: "
            + str(qiskit_result["time_taken"])
        )
        print(
            "qiskit fusion with diagonal fusion, gate number: "
            + str(len(qiskit_result["output_ops"]) - total_qubit)
        )
        with open(log_filename, "a") as f:
            for gate in qiskit_result["output_ops"]:
                if gate["name"] == "measure":
                    continue
                f.write(gate["name"] + str(len(gate["qubits"])) + " ")
                for qubit in gate["qubits"]:
                    f.write(str(qubit) + " ")
                f.write("\n")
    return simulation_time


def circuits_equivalent_by_samples(circ1, circ2, shots=1024, tol=0.01):
    """Compares two circuits by sampling random input states."""

    simulator = AerSimulator(
        method="statevector",
        seed_simulator=0,
    )
    res1 = simulator.run(circ1, shots=shots).result().get_counts()
    res2 = simulator.run(circ2, shots=shots).result().get_counts()

    # Convert counts to probability distributions
    prob1 = {k: v / shots for k, v in res1.items()}
    prob2 = {k: v / shots for k, v in res2.items()}

    # Compute total variation distance (TVD)
    all_keys = set(prob1.keys()).union(set(prob2.keys()))
    tvd = sum(abs(prob1.get(k, 0) - prob2.get(k, 0)) for k in all_keys) / 2
    print(f"TVD: {tvd:.4f}; {'' if tvd < tol else 'NOT '}Equivalent.")
    return tvd < tol


if __name__ == "__main__":
    mfq = 3  # max_fusion_qubits
    total_qubit = 24
    filename_list = ["sc", "vc", "hs", "qv", "bv", "qft", "qaoa", "ising"]
    # filename_list = ["qft"]

    os.makedirs("./qiskitFusionCircuit", exist_ok=True)

    for i, filename in enumerate(filename_list):
        filename = filename + str(total_qubit) + ".txt"
        print("==================================================================")
        print(filename)

        # disable
        qc0 = load_circuit("./circuit/" + filename, total_qubit)
        print(exe_circuit(qc0, "disable", mfq, False, True))
        # origin
        qc1 = load_circuit("./circuit/" + filename, total_qubit)
        print(exe_circuit(qc1, "origin", mfq, True, True))

        # static Qiskit
        fm = "static_qiskit"
        print(f"\n{fm}: ", gen_qiskit_fusion(filename, total_qubit, fm), end="")
        qc2 = load_circuit(f"./qiskitFusionCircuit/fused_{fm}_{filename}", total_qubit)
        print(exe_circuit(qc2, fm, mfq, True, True))
        circuits_equivalent_by_samples(qc1, qc2)

        # dynamic Qiskit
        fm = "dynamic_qiskit"
        print(f"\n{fm}: ", gen_qiskit_fusion(filename, total_qubit, fm), end="")
        qc3 = load_circuit(f"./qiskitFusionCircuit/fused_{fm}_{filename}", total_qubit)
        print(exe_circuit(qc3, fm, mfq, True, True))
        circuits_equivalent_by_samples(qc1, qc3)

        # static DFGC
        # result = subprocess.run(
        #     [
        #         "./fusion",
        #         "./circuit/" + filename,
        #         "./fusionCircuit/fused_q_s_" + filename,
        #         str(mfq),
        #         str(total_qubit),
        #         "5",
        #     ],
        #     capture_output=True,
        #     text=True,
        # )
        # print("static DFGC time: ", result.stdout.split()[-1])
        # print(
        #     exe_circuit(
        #         "./fusionCircuit/fused_q_s_" + filename,
        #         total_qubit,
        #         True,
        #         mfq,
        #         False,
        #     )
        # )

        # # dynamic DFGC
        # result = subprocess.run(
        #     [
        #         "./fusion",
        #         "./circuit/" + filename,
        #         "./fusionCircuit/fused_q_d_" + filename,
        #         str(mfq),
        #         str(total_qubit),
        #         "8",
        #     ],
        #     capture_output=True,
        #     text=True,
        # )
        # print("dynamic DFGC time: ", result.stdout.split()[-1])
        # print(
        #     exe_circuit(
        #         "./fusionCircuit/fused_q_d_" + filename,
        #         total_qubit,
        #         True,
        #         mfq,
        #         False,
        #     )
        # )
