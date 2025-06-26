# ----------------------------------------------------------------------
# NWQBench: Northwest Quantum Proxy Application Suite
# ----------------------------------------------------------------------
# Ang Li, Samuel Stein, James Ang.
# Pacific Northwest National Laboratory(PNNL), U.S.
# BSD Lincese.
# Created 04/19/2021.
# ref: https://github.com/pnnl/nwqbench/blob/a7b829c/NWQ_Bench/vqc/vqc_raw.py
# ----------------------------------------------------------------------
# Updated 06/23/2024 for seed, no matplotlib, new qasm dumping, and IR format.
# ----------------------------------------------------------------------

import os
import sys

import numpy as np
import qiskit
from qiskit import qasm2

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from python.aer_utils import qasm_to_ir


class VQC:
    def __init__(self, qubit_count):
        self.qubit_count = qubit_count
        self.quantum_register = qiskit.circuit.QuantumRegister(self.qubit_count)
        self.classical_register = qiskit.circuit.ClassicalRegister(self.qubit_count)
        self.circuit = qiskit.circuit.QuantumCircuit(
            self.quantum_register, self.classical_register
        )
        self.layer_count = 4
        self.hadamard_circuit()
        self.phase_addition()
        self.learnable_layers()
        self.circuit.measure_all()

    def hadamard_circuit(self):
        for qubit in self.quantum_register:
            self.circuit.h(qubit)

    def phase_addition(self):
        for qubit in self.quantum_register:
            self.circuit.rz(np.random.rand() * np.pi, qubit)
        for cqubit, aqubit in zip(
            self.quantum_register[:-1], self.quantum_register[1:]
        ):
            self.circuit.cx(cqubit, aqubit)
            self.circuit.rz(np.random.rand() * np.pi, aqubit)
            self.circuit.cx(cqubit, aqubit)

    def learnable_layers(self):
        for _ in range(self.layer_count):
            for qubit in self.quantum_register:
                self.circuit.ry(np.random.rand() * np.pi, qubit)
                self.circuit.rz(np.random.rand() * np.pi, qubit)
            qbs = list(self.quantum_register)
            for i, qb in enumerate(qbs):
                for j in range(i + 1, self.qubit_count):
                    self.circuit.cz(qb, qbs[j])


if __name__=='__main__':
    if len(sys.argv) < 2:
        print("Use from CLI with python filename n_qubits")
        exit()

    k = int(sys.argv[1])
    np.random.seed(42)
    circuit = VQC(k)
    if not os.path.isdir("qasm"):
        os.mkdir("qasm")

    qasm_file = open(f"qasm/vqc{k}.qasm", "w")
    qasm_file.write(qasm2.dumps(circuit.circuit))
    qasm_file.close()

    if not os.path.isdir("ir"):
        os.mkdir("ir")
    qasm_to_ir(circuit.circuit, f"ir/vqc{k}.txt")
