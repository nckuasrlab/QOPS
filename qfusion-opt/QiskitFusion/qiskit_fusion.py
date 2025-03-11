import json
import subprocess
import sys
import warnings
from pathlib import Path

from qiskit import QuantumCircuit
from qiskit.compiler import assemble, transpile

warnings.filterwarnings(
    "ignore", category=DeprecationWarning
)  # Ignore DeprecationWarning of qiskit.compiler.assembler.assemble()


def build_circuit(file_name, qubit):
    circuits = []
    circuit = QuantumCircuit(qubit)

    f = open(file_name, "r")
    lines = f.readlines()

    for line in lines:
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

    circuit.measure_all()
    circuits.append(circuit)
    return circuits


if __name__ == "__main__":
    mode = sys.argv[3]
    current_dir = Path(__file__).parent
    circuit = build_circuit(sys.argv[1], int(sys.argv[2]))
    qobj = assemble(transpile(circuit))
    with open(current_dir / "qobj.json", "wt") as fp:
        json.dump(qobj.to_dict(), fp)
    if mode == "dynamic_qiskit":
        result = subprocess.run(
            [(current_dir / "dynamic_qiskit.out"), (current_dir / "qobj.json")],
            capture_output=True,
            text=True,
        )
    elif mode == "static_qiskit":
        result = subprocess.run(
            [(current_dir / "static_qiskit.out"), (current_dir / "qobj.json")],
            capture_output=True,
            text=True,
        )
    if result.returncode != 0:
        print("ERROR:", result.returncode)
        print(result.stdout)
    print(f"{mode} fusion time: {result.stdout.split("\n")[-1]}")
