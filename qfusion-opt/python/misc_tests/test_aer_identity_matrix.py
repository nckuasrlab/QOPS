import random
from itertools import repeat

import numpy as np
import scipy.linalg
from qiskit import QuantumCircuit
from qiskit.circuit.library import UnitaryGate
from qiskit_aer import AerSimulator


def random_unitary_matrix(num_qubits):
    """
    Generates a random unitary matrix of size 2^num_qubits x 2^num_qubits
    and returns it as a Python 2D list.
    """
    if not isinstance(num_qubits, int) or num_qubits < 1:
        raise ValueError("num_qubits must be a positive integer.")

    dimension = 2**num_qubits

    # Generate a random complex matrix using numpy
    random_complex_matrix = np.random.randn(
        dimension, dimension
    ) + 1j * np.random.randn(dimension, dimension)

    # Perform QR decomposition to get a unitary matrix using scipy
    Q, R = scipy.linalg.qr(random_complex_matrix)

    # Convert the numpy array to a Python list of lists
    unitary_matrix_list = Q.tolist()

    return unitary_matrix_list


def identity_unitary_matrix(num_qubits):
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


def test(num_qubits: int, num_gates: int, func, verbose=True):
    circuit = QuantumCircuit(num_qubits)
    for _ in repeat(None, num_gates):
        target_qubit = random.sample(range(num_qubits), 3)
        circuit.append(UnitaryGate(func(3)), target_qubit)
    circuit.measure_all()
    simulator = AerSimulator(method="statevector", fusion_enable=False)
    result = simulator.run(circuit).result()
    if verbose:
        print(func.__name__, result.results[0].metadata["time_taken"])


def main():
    num_qubits = 28
    num_gates = 100
    print(f"{num_qubits} qubits, {num_gates} gates")
    # warmup
    test(num_qubits, num_gates, identity_unitary_matrix, verbose=False)
    test(num_qubits, num_gates, identity_unitary_matrix)

    # warmup
    test(num_qubits, num_gates, random_unitary_matrix, verbose=False)
    test(num_qubits, num_gates, random_unitary_matrix)


if __name__ == "__main__":
    main()
