import numpy as np


def random_diagonal_gate(num_qubits):
    """
    Generates a random quantum diagonal gate for a given number of qubits.

    Args:
        num_qubits (int): The number of qubits.

    Returns:
        numpy.ndarray: A random diagonal unitary matrix representing the gate.
    """
    dimension = 2**num_qubits
    phases = np.exp(2j * np.pi * np.random.rand(dimension))  # Random phases
    diagonal_matrix = np.diag(phases)

    return diagonal_matrix


def fuse_diagonal_gates(gate1, gate2):
    """Fuses two diagonal gates into a single diagonal gate."""
    if gate1.shape != gate2.shape:
        raise ValueError("Gates must have the same dimensions.")
    if not (
        np.all(np.isclose(gate1, np.diag(np.diag(gate1))))
        and np.all(np.isclose(gate2, np.diag(np.diag(gate2))))
    ):
        raise ValueError("Both matrices must be diagonal")

    return np.diag(np.diag(gate1) * np.diag(gate2))


def print_square_matrix(matrix):
    """Prints a square NumPy matrix with 3 decimal places."""
    rows, cols = matrix.shape
    if rows != cols:
        raise ValueError("Matrix must be square.")

    for row in matrix:
        formatted_row = ["{:.3f}".format(val) for val in row]
        print("[", " ".join(formatted_row), "]")


# Example usage:
num_qubits = 2
gate1 = random_diagonal_gate(num_qubits)
gate2 = random_diagonal_gate(num_qubits)

fused_gate = fuse_diagonal_gates(gate1, gate2)

print("Gate 1:")
print_square_matrix(gate1)
print("\nGate 2:")
print_square_matrix(gate2)
print("\nFused Gate:")
print_square_matrix(fused_gate)

# Verify the result.
print("\nVerify fusion result:")
print(np.allclose(fused_gate, gate1 @ gate2))
