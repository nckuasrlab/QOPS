import time
from dataclasses import dataclass
from math import pow

import numpy as np
import scipy.linalg
from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit.compiler import transpile
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
    assert len(matrix_1d) == dim * dim * 2, f"{len(matrix_1d)} != {dim*dim*2}"
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
    assert len(matrix_1d) == dim * 2, f"{len(matrix_1d)} != {dim*2}"
    matrix = []
    for i in range(0, dim * 2, 2):
        matrix.append(float(matrix_1d[i]) + float(matrix_1d[i + 1]) * 1j)
    return matrix


def random_unitary_matrix(num_qubits):
    """
    Generates a random unitary matrix of size 2^num_qubits x 2^num_qubits
    and returns it as a Python 2D list.
    """
    if not isinstance(num_qubits, int) or num_qubits < 1:
        raise ValueError("num_qubits must be a positive integer.")
    dimension = 2**num_qubits
    random_complex_matrix = np.random.randn(
        dimension, dimension
    ) + 1j * np.random.randn(dimension, dimension)
    Q, R = scipy.linalg.qr(random_complex_matrix)
    unitary_matrix_list = Q.tolist()
    return unitary_matrix_list


def random_diagonal_matrix(num_qubits):
    """
    Generates a random quantum diagonal gate for a given number of qubits.
    """
    dimension = 2**num_qubits
    return np.exp(2j * np.pi * np.random.rand(dimension))  # Random phases


def load_circuit(
    filename: str, total_qubit: int, circuit_name: str, use_random_matrix=False, skip_measurement=False
) -> QuantumCircuit:
    circuit = QuantumCircuit(total_qubit)
    circuit.name = circuit_name
    fusion_file = open(filename, "r")
    lines = fusion_file.readlines()
    fusion_file.close()
    line_count = 0
    for line in lines:
        line_count = line_count + 1
        if line.startswith("//") or line.strip() == "":
            continue
        line = line.split()
        if line[0] == "U1" and len(line) == 5:
            circuit.u(float(line[2]), float(line[3]), float(line[4]), int(line[1]))
        elif line[0].startswith("U") and len(line[0]) == 2:
            gate_qubit = int(line[0][1])
            if use_random_matrix:
                mat = random_unitary_matrix(gate_qubit)
            else:
                mat = read_unitary_matrix(gate_qubit, line[(gate_qubit + 1) :])
            circuit.append(
                UnitaryGate(mat), [int(line[q]) for q in range(1, gate_qubit + 1)]
            )
        elif line[0].startswith("D") and len(line[0]) == 2:
            gate_qubit = int(line[0][1])
            if use_random_matrix:
                mat = random_diagonal_matrix(gate_qubit)
            else:
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
        elif line[0] == "U1Gate":
            circuit.u(0, 0, float(line[2]), int(line[1]))
        elif line[0] == "U2Gate":
            circuit.u(np.pi/2, float(line[2]), float(line[3]), int(line[1]))
        elif line[0] == "U3Gate":
            circuit.u(float(line[2]), float(line[3]), float(line[4]), int(line[1]))
        else:
            raise Exception(f"{line[0]} is not supported ({line}), in {filename}:{line_count}")
    if not skip_measurement:
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
    dump_log: bool,
) -> ExecResult:
    simulator = AerSimulator(
        method="statevector",
        seed_simulator=0,
        fusion_enable=open_fusion,
        fusion_verbose=open_fusion,
        fusion_max_qubit=max_fusion_qubits,
    )
    # circuit is transpiled with default optimizations
    qc = transpile(circuit, simulator)
    res = simulator.run(qc).result()

    # dump execution log
    if dump_log:
        log_filename = f"./qiskitFusionCircuit/aer_{fusion_method}_{circuit.name}.log"
        with open(log_filename, "w") as f:
            f.write(str(res) + "\n")

    # dump fusion log
    fusion_result = res.results[0].metadata["fusion"]
    internal_fusio_applied = open_fusion and fusion_result["applied"]
    internal_fusion_time = 0
    internal_fused_gate_num = 0
    if open_fusion and internal_fusio_applied:
        internal_fusion_time = float(fusion_result["time_taken"])
        internal_fused_gate_num = len(fusion_result["output_ops"]) - circuit.num_qubits
        if dump_log:
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


def circuits_equivalent_by_samples(circ1, circ2, shots=1024, tol=0.001):
    """Compares two circuits by sampling random input states."""

    simulator = AerSimulator(
        method="statevector",
        seed_simulator=0,
        fusion_enable=True,
    )
    t_start1 = time.perf_counter()
    res1 = simulator.run(circ1, shots=shots).result().get_counts()
    t_end1 = time.perf_counter()
    t_start2 = time.perf_counter()
    res2 = simulator.run(circ2, shots=shots).result().get_counts()
    t_end2 = time.perf_counter()
    # Convert counts to probability distributions
    prob1 = {k: v / shots for k, v in res1.items()}
    prob2 = {k: v / shots for k, v in res2.items()}

    # Compute total variation distance (TVD)
    all_keys = set(prob1.keys()).union(set(prob2.keys()))
    tvd = sum(abs(prob1.get(k, 0) - prob2.get(k, 0)) for k in all_keys) / 2
    print(f"TVD: {tvd:.6f}; {'' if tvd < tol else 'NOT '}Equivalent.")
    return tvd < tol, t_end1 - t_start1, t_end2 - t_start2