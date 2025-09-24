import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from uuid import uuid4

from qiskit import QuantumCircuit

# Updated imports to support both legacy (<2.0) and Qiskit 2.1.0+
try:
    from qiskit.compiler import assemble, transpile  # Legacy path (pre 2.0)
except ImportError:
    from qiskit import transpile  # Qiskit 2.0+ (assemble removed)

    assemble = None

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


def write_qobj_or_instruction_json(circuit, out_path: Path):
    """
    Qiskit >=2.0 removed assemble/Qobj. For backward compatibility with external
    tools expecting a qobj-like JSON, we create a minimal instruction JSON when
    assemble is unavailable.
    """
    tcirc = transpile(circuit)
    if assemble is not None:
        # Legacy path: produce real Qobj dict
        qobj = assemble(tcirc)
        with open(out_path, "wt") as fp:
            json.dump(qobj.to_dict(), fp)
    else:
        # Fallback: emulate a minimal legacy Qobj dict expected by downstream C++ tool.
        # NOTE: This is NOT a full Qiskit Qobj; it only supplies the fields used by the fusion executables.
        qubit_index = {qb: i for i, qb in enumerate(tcirc.qubits)}
        clbit_index = {cb: i for i, cb in enumerate(tcirc.clbits)}

        try:  # Optional import for parameter expressions
            from qiskit.circuit import ParameterExpression
        except Exception:  # pragma: no cover
            ParameterExpression = tuple()  # type: ignore

        def serialize_param(p):
            if isinstance(p, (int, float)):
                return float(p)
            if isinstance(p, complex):
                return {"real": p.real, "imag": p.imag}
            if ParameterExpression and isinstance(p, ParameterExpression):
                try:
                    return float(p)
                except Exception:
                    return str(p)
            return str(p)

        instructions = []
        for item in tcirc.data:
            if hasattr(item, "operation"):
                inst = item.operation
                qargs = item.qubits
                cargs = item.clbits
            else:  # legacy tuple unpacking
                inst, qargs, cargs = item

            entry = {
                "name": inst.name,
                "qubits": [qubit_index[q] for q in qargs],
            }
            if cargs:
                entry["clbits"] = [clbit_index[c] for c in cargs]
            if getattr(inst, "params", None):
                entry["params"] = [serialize_param(p) for p in inst.params]
            instructions.append(entry)

        # Legacy headers (simplified)
        cl_labels = [["c", i] for i in range(tcirc.num_clbits)]
        q_labels = [["q", i] for i in range(tcirc.num_qubits)]

        qobj_dict = {
            "qobj_id": f"auto-{uuid4()}",
            "type": "QASM",
            "schema_version": "1.3.0",  # Chosen common legacy version
            "experiments": [
                {
                    "header": {
                        "n_qubits": tcirc.num_qubits,
                        "memory_slots": tcirc.num_clbits,
                        "clbit_labels": cl_labels,
                        "qubit_labels": q_labels,
                        "name": "circuit0",
                    },
                    "instructions": instructions,
                }
            ],
            "header": {"backend_name": "", "backend_version": ""},
            "config": {
                "shots": 1,
                "memory": True,
                "memory_slots": tcirc.num_clbits,
                "n_qubits": tcirc.num_qubits,
            },
        }

        with open(out_path, "wt") as fp:
            json.dump(qobj_dict, fp, indent=2)


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
        default="../log/gate_exe_time_aer.csv",
    )
    parser.add_argument(
        "-o",
        "--output_filename",
        type=str,
        metavar="PATH",
        required=True,
        help="Output filename for the fused circuit",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Verbose mode",
    )

    return parser.parse_args()


def main():
    args = get_args()
    print(args)
    current_dir = Path(__file__).parent
    circuit = parse_circuit(args.circuit_filename, args.num_qubits)

    # Write qobj (legacy) or fallback JSON
    write_qobj_or_instruction_json(circuit, current_dir / "qobj.json")

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
        sys.exit(1)
    else:
        if args.verbose:
            print(result.stdout)
            print(result.stderr)
        fusion_time = result.stdout.split("\n")[-1]
        print(f"{args.fusion_mode} fusion time (s):\n{fusion_time}", end="")


if __name__ == "__main__":
    main()
