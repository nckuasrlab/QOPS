import argparse
import json
import os
import subprocess
import warnings
from pathlib import Path

from qiskit import QuantumCircuit
from qiskit.compiler import assemble, transpile

warnings.filterwarnings(
    "ignore", category=DeprecationWarning
)  # Ignore DeprecationWarning of qiskit.compiler.assembler.assemble()


def parse_circuit(circuit_filename, qubit):
    """
    The input format of circuits is provieded by Quokka.
    This function parses the circuit file and returns a QuantumCircuit object.
    """
    circuit = QuantumCircuit(qubit)

    f = open(circuit_filename, "r")
    lines = f.readlines()
    f.close()

    for line in lines:
        if len(line.strip()) == 0:
            continue
        line = line.split()
        gate = line[0]
        q1 = int(line[1])

        if gate == "H":
            circuit.h(q1)
        elif gate == "X":
            circuit.x(q1)
        elif gate == "RX":
            circuit.rx(float(line[2]), q1)
        elif gate == "RY":
            circuit.ry(float(line[2]), q1)
        elif gate == "RZ":
            circuit.rz(float(line[2]), q1)
        elif gate == "RZZ":
            circuit.rzz(float(line[3]), q1, int(line[2]))
        elif gate == "CX":
            circuit.cx(q1, int(line[2]))
        elif gate == "CZ":
            circuit.cz(q1, int(line[2]))
        elif gate == "CP":
            circuit.cp(float(line[3]), q1, int(line[2]))
        elif gate == "U1":
            circuit.u(float(line[2]), float(line[3]), float(line[4]), q1)
        else:
            raise ValueError(f"Unknown gate: {gate}")

    circuit.measure_all()
    return circuit


def get_args():
    parser = argparse.ArgumentParser(
        description="Qiskit Fusion Script",
        prog="python qiskit_fusion.py",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("circuit_filename", type=str, help="Path to the circuit file")
    parser.add_argument("num_qubits", type=int, help="Number of qubits")
    parser.add_argument(
        "fusion_mode",
        type=str,
        choices=["dynamic_qiskit", "static_qiskit"],
        help="Fusion mode",
    )
    parser.add_argument(
        "-m",
        "--fusion_max_qubit",
        type=int,
        metavar="NUM",
        help="Maximum number of qubits for fusion",
        default=3,
    )
    parser.add_argument(
        "-d",
        "--dynamic_cost_filename",
        type=str,
        metavar="PATH",
        help="Dynamic cost filename",
        default="../log/gate_exe_time.csv",
    )
    parser.add_argument(
        "-o",
        "--output_filename",
        type=str,
        metavar="PATH",
        required=True,
        help="Output filename for the fused circuit",
    )

    return parser.parse_args()


def main():
    args = get_args()
    print(args)
    current_dir = Path(__file__).parent
    circuit = parse_circuit(args.circuit_filename, args.num_qubits)
    qobj = assemble(transpile(circuit))
    with open(current_dir / "qobj.json", "wt") as fp:
        json.dump(qobj.to_dict(), fp)

    # Get a copy of the current environment and insert new environment variables
    modified_env = os.environ.copy()
    modified_env["FUSION_MAX_QUBIT"] = f"{args.fusion_max_qubit}"
    modified_env["FUSED_CIRCUIT_FILENAME"] = f"{args.output_filename}"
    if args.fusion_mode == "dynamic_qiskit":
        modified_env["DYNAMIC_COST_FILENAME"] = f"{args.dynamic_cost_filename}"

    result = subprocess.run(
        [
            (current_dir / f"{args.fusion_mode}.out"),
            (current_dir / "qobj.json"),
        ],
        env=modified_env,
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:
        print("ERROR:", result.returncode)
        print(result.stdout)
        print(result.stderr)
    else:
        fusion_time = result.stdout.split("\n")[-1]
        print(f"{args.fusion_mode} fusion time (s):\n{fusion_time}")


if __name__ == "__main__":
    main()
