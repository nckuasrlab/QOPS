import argparse
import os
import sys

from python.aer_utils import load_circuit
from qiskit.visualization import circuit_drawer


def draw_circuit(input_path: str, filename: str, total_qubit: int = 24):
    """Draw a quantum circuit (NTU format) and save it to an SVG file.

    Args:
        input_path: Path to NTU circuit text file.
        filename: Output SVG path.
        total_qubit: Number of qubits to allocate when loading circuit.
    """
    circuit = load_circuit(
        input_path,
        total_qubit=total_qubit,
        circuit_name="default_circuit",
        use_random_matrix=False,
        skip_measurement=True,
    )
    circuit_img = circuit_drawer(circuit, output="mpl", scale=2, fold=-1, style="bw")
    circuit_img.savefig(filename)


def _parse_args(argv=None):
    parser = argparse.ArgumentParser(description="Draw an NTU-format quantum circuit to SVG.")
    parser.add_argument("input_path", help="Path to NTU circuit .txt file")
    parser.add_argument(
        "-q",
        "--qubits",
        type=int,
        default=24,
        help="Total qubit count (default: 24)",
    )
    parser.add_argument(
        "-o",
        "--output",
        help="Output SVG path (default: ./svg/<input_basename>.svg)",
    )
    return parser.parse_args(argv)


if __name__ == "__main__":
    args = _parse_args()
    input_path = args.input_path
    os.makedirs("./svg", exist_ok=True)
    output_path = args.output or ("./svg/" + os.path.basename(input_path).replace(".txt", ".svg"))
    draw_circuit(input_path, output_path, total_qubit=args.qubits)
