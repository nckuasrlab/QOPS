from typing import List

import numpy as np
import scipy.linalg


def random_unitary_matrix(num_qubits) -> List[List[complex]]:
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


def random_diagonal_gate(num_qubits) -> List[complex]:
    """
    Generates a random quantum diagonal gate for a given number of qubits.
    """
    dimension = 2**num_qubits
    angles = 2 * np.pi * np.random.rand(dimension)
    phases = np.exp(1j * angles)
    return phases.tolist()


def random_u_gate_parameters():
    """Generates random parameters (theta, phi, lambda) for a U gate."""
    theta = np.random.uniform(0, np.pi)  # Theta ranges from 0 to pi
    phi = np.random.uniform(0, 2 * np.pi)  # Phi ranges from 0 to 2pi
    lambda_ = np.random.uniform(0, 2 * np.pi)  # Lambda ranges from 0 to 2pi
    return theta, phi, lambda_


def random_theta():
    theta = np.random.uniform(0, 2 * np.pi)
    return theta

if __name__ == "__main__":
    print(random_unitary_matrix(2))
    print(random_diagonal_gate(2))
    print(random_u_gate_parameters())
    print(random_theta())