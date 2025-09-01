import os
import sys

from python.aer_utils import load_circuit
from qiskit.visualization import circuit_drawer


def draw_circuit(input_path: str, filename: str):
    """Draws a quantum circuit (NTU format) and saves it to an SVG file."""
    circuit = load_circuit(
        input_path,
        24,
        "default_circuit",
        use_random_matrix=False,
        skip_measurement=True,
    )
    circuit_img = circuit_drawer(circuit, output="mpl", scale=2, fold=-1, style="bw")
    circuit_img.savefig(filename)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python circuit_drawer.py <input_path>")
        sys.exit(1)
    input_path = sys.argv[1]
    os.makedirs("./svg", exist_ok=True)
    draw_circuit(input_path, "./svg/" + os.path.basename(input_path).replace(".txt", ".svg"))
