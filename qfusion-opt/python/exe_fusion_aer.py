import os
import subprocess
import sys
from dataclasses import dataclass
from itertools import repeat
from math import pow

import numpy as np
from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit_aer import AerSimulator
from scipy.linalg import polar


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


def read_diagonal_matrix(num_qubits, matrix_1d):
    dim = int(pow(2, num_qubits))
    matrix = []
    for i in range(0, dim * 2, 2):
        matrix.append(float(matrix_1d[i]) + float(matrix_1d[i + 1]) * 1j)
    return matrix


def gen_qiskit_fusion(filename, total_qubit, fusion_method) -> float:
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
            f"./qiskitFusionCircuit/fused_{fusion_method}_{filename}",
        ],
        capture_output=True,
        text=True,
    )
    if fusion_process.returncode != 0:
        print("ERROR:", fusion_process.returncode)
        print(fusion_process.stdout)
        print(fusion_process.stderr)
        sys.exit(1)

    fusion_time = float(fusion_process.stdout.split("\n")[-1])
    return fusion_time


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
            gate_qubit = 1
            mat = read_diagonal_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                DiagonalGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0] == "D2":
            gate_qubit = 2
            mat = read_diagonal_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                DiagonalGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0] == "D3":
            gate_qubit = 3
            mat = read_diagonal_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                DiagonalGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0] == "D4":
            gate_qubit = 4
            mat = read_diagonal_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                DiagonalGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0] == "D5":
            gate_qubit = 5
            mat = read_diagonal_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                DiagonalGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
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
    return circuit


@dataclass
class ExecResult:
    simulation_time: float
    internal_fusio_applied: bool
    internal_fusion_time: float
    internal_fused_gate_num: int

    def __str__(self):
        return (
            f"{self.simulation_time}, "
            f"{self.internal_fusio_applied}, "
            f"{self.internal_fusion_time}, "
            f"{self.internal_fused_gate_num}"
        )


def exec_circuit(
    circuit: QuantumCircuit,
    fusion_method: str,
    max_fusion_qubits: int,
    open_fusion: bool,
    get_info: bool,
) -> ExecResult:
    simulator = AerSimulator(
        method="statevector",
        seed_simulator=0,
        fusion_enable=open_fusion,
        fusion_verbose=open_fusion,
        fusion_max_qubit=max_fusion_qubits,
    )
    res = simulator.run(circuit).result()

    # dump execution log
    log_filename = (
        f"./qiskitFusionCircuit/aer_{fusion_method}_{os.path.basename(filename)}.log"
    )
    with open(log_filename, "w") as f:
        f.write(str(res) + "\n")

    # dump fusion log
    fusion_result = res.results[0].metadata["fusion"]
    internal_fusio_applied = open_fusion and fusion_result["applied"]
    internal_fusion_time = 0
    internal_fused_gate_num = 0
    if open_fusion and get_info and internal_fusio_applied:
        internal_fusion_time = float(fusion_result["time_taken"])
        internal_fused_gate_num = len(fusion_result["output_ops"]) - total_qubit
        with open(log_filename, "a") as logfile:
            for gate in fusion_result["output_ops"]:
                if gate["name"] == "measure":
                    continue
                logfile.write(f"{gate['name']}-{len(gate['qubits'])} ")
                for qubit in gate["qubits"]:
                    logfile.write(str(qubit) + " ")
                logfile.write("\n")

    simulation_time = float(res.results[0].metadata["time_taken"])
    return ExecResult(
        simulation_time,
        internal_fusio_applied,
        internal_fusion_time,
        internal_fused_gate_num,
    )


def circuits_equivalent_by_samples(circ1, circ2, shots=1024, tol=0.01):
    """Compares two circuits by sampling random input states."""

    simulator = AerSimulator(
        method="statevector",
        seed_simulator=0,
        fusion_enable=False,
    )
    res1 = simulator.run(circ1, shots=shots).result().get_counts()
    res2 = simulator.run(circ2, shots=shots).result().get_counts()

    # Convert counts to probability distributions
    prob1 = {k: v / shots for k, v in res1.items()}
    prob2 = {k: v / shots for k, v in res2.items()}

    # Compute total variation distance (TVD)
    all_keys = set(prob1.keys()).union(set(prob2.keys()))
    tvd = sum(abs(prob1.get(k, 0) - prob2.get(k, 0)) for k in all_keys) / 2
    # print(f"TVD: {tvd:.4f}; {'' if tvd < tol else 'NOT '}Equivalent.")
    return tvd < tol


if __name__ == "__main__":
    mfq = 3  # max_fusion_qubits
    total_qubit = 24
    # benchmarks = ["sc", "vc", "hs", "qv", "bv", "qft", "qaoa", "ising"]
    benchmarks = ["sc", "vc"]

    os.makedirs("./qiskitFusionCircuit", exist_ok=True)

    for benchmark in benchmarks:
        filename = benchmark + str(total_qubit) + ".txt"
        print("==================================================================")
        print(filename)
        experiment_logfile = open(
            f"./qiskitFusionCircuit/experiment_{benchmark}{total_qubit}.log", "w"
        )

        # disable
        fm = "disable"
        print(f"\n{fm}: ")
        experiment_logfile.write(f"{fm}:\n")
        qc0 = load_circuit("./circuit/" + filename, total_qubit)
        for _ in repeat(None, 3):
            exec_result = exec_circuit(qc0, fm, mfq, False, True)  # warmup
        for _ in repeat(None, 10):
            exec_result = exec_circuit(qc0, fm, mfq, False, True)
            experiment_logfile.write(str(exec_result) + "\n")

        # origin
        fm = "origin"
        print(f"\n{fm}: ")
        experiment_logfile.write(f"{fm}:\n")
        qc1 = load_circuit("./circuit/" + filename, total_qubit)
        for _ in repeat(None, 3):
            exec_result = exec_circuit(qc1, fm, mfq, True, True)  # warmup
        for _ in repeat(None, 10):
            exec_result = exec_circuit(qc1, fm, mfq, True, True)
            experiment_logfile.write(str(exec_result) + "\n")

        # static Qiskit
        fm = "static_qiskit"
        print(f"\n{fm}: ")
        experiment_logfile.write(f"{fm}:\n")
        fusion_time = gen_qiskit_fusion(filename, total_qubit, fm)
        experiment_logfile.write(str(fusion_time) + "\n")
        qc2 = load_circuit(f"./qiskitFusionCircuit/fused_{fm}_{filename}", total_qubit)
        if not circuits_equivalent_by_samples(qc1, qc2):
            print(f"ERROR: {fm} circuits not equivalent")
        for _ in repeat(None, 3):
            exec_result = exec_circuit(qc2, fm, mfq, True, True)  # warmup
        for _ in repeat(None, 10):
            exec_result = exec_circuit(qc2, fm, mfq, True, True)
            experiment_logfile.write(str(exec_result) + "\n")

        # dynamic Qiskit
        fm = "dynamic_qiskit"
        print(f"\n{fm}: ")
        experiment_logfile.write(f"{fm}:\n")
        fusion_time = gen_qiskit_fusion(filename, total_qubit, fm)
        experiment_logfile.write(str(fusion_time) + "\n")
        qc3 = load_circuit(f"./qiskitFusionCircuit/fused_{fm}_{filename}", total_qubit)
        if not circuits_equivalent_by_samples(qc1, qc3):
            print(f"ERROR: {fm} circuits not equivalent")
        for _ in repeat(None, 3):
            exec_result = exec_circuit(qc3, fm, mfq, True, True)  # warmup
        for _ in repeat(None, 10):
            exec_result = exec_circuit(qc3, fm, mfq, True, True)
            experiment_logfile.write(str(exec_result) + "\n")

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
        #     exec_circuit(
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
        #     exec_circuit(
        #         "./fusionCircuit/fused_q_d_" + filename,
        #         total_qubit,
        #         True,
        #         mfq,
        #         False,
        #     )
        # )
        experiment_logfile.close()
