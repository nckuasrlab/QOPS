import os
import subprocess
import sys
import json
from circuit import circuit_load

from python.aer_utils import circuits_equivalent_by_samples, exec_circuit, load_circuit
from qiskit import QuantumCircuit
from pathlib import Path
from qiskit_aer import AerSimulator
from qiskit.compiler import transpile


def qiskit_validation(circuit_name, total_qubit):
    fusion_max_qubit = 3  # max_fusion_qubits
    circuit = load_circuit(f"./{circuit_name}", total_qubit, circuit_name)

    simulator = AerSimulator(
        method="statevector",
        seed_simulator=0,
        fusion_enable=0,
        fusion_verbose=0,
        fusion_max_qubit=fusion_max_qubit,
    )
    qc = transpile(circuit, simulator)
    res = simulator.run(qc).result()
    return res.get_counts()


circuits = list(Path("./circuit").glob("*.txt"))
modes = [3, 5]

for circuit_path in circuits:
    origin_path = str(circuit_path)
    total_qubit = int(circuit_path.name[-6:-4])
    tmpPath = circuit_path.name[:-4]
    fused_path = f"{tmpPath}.out"

    # Only validate the circuits less than 31 qubits. Can be changed as needed.
    if total_qubit >= 31:
        continue
    # qaoa takes quite long time, skip while developing
    if tmpPath[:-2] == "qaoa":
        continue

    for mode in modes:
        print(f"Running fusion for {origin_path} with mode = {mode}...")
        subprocess.run(
            ["./fusion", origin_path, fused_path, "3", str(total_qubit), str(mode)],
            stdout=subprocess.DEVNULL,
        )

        print(f"Simulating original circuit: {origin_path}")
        res = qiskit_validation(origin_path, total_qubit)
        res = json.dumps(res)
        with open("origin.out", "w") as origin_out:
            origin_out.write(res)

        print(f"Simulating fused circuit: {fused_path}")
        res = qiskit_validation(fused_path, total_qubit)
        res = json.dumps(res)
        with open("fusion.out", "w") as fusion_out:
            fusion_out.write(res)

        diff_output_file = f"diff_{tmpPath}.txt"
        result = subprocess.run(
            ["diff", "origin.out", "fusion.out"], stdout=subprocess.PIPE
        )

        if result.returncode == 0:
            print(f"\033[32mNo differences for {origin_path}\033[0m")
            if os.path.exists(diff_output_file):
                os.remove(diff_output_file)
        else:
            with open(diff_output_file, "w") as diff_out:
                diff_out.write(result.stdout.decode())
            print(
                f"\033[31mDifferences found for {origin_path}. See {diff_output_file}\033[0m"
            )
        print("---------------------------------------")
