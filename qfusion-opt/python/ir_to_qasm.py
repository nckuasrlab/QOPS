"""Convert a fusion IR (*.txt) circuit description into OpenQASM 2 output.

Usage:
  python ir_to_qasm.py <circuit_file.txt> [--out output.qasm] [--no-measure]

Behavior:
  * total qubits inferred from digits before the .txt suffix (e.g. qaoa32.txt -> 32)
  * circuit name is the stem (filename without extension)
  * by default appends measurements to all qubits (consistent with load_circuit)
    pass --no-measure to skip adding measurements
  * output file defaults to replacing .txt with .qasm in same directory unless --out provided
"""

from __future__ import annotations

import argparse
import os
import re
import sys

try:  # Allow running from project root or within python/ directory
    from python.aer_utils import load_circuit  # type: ignore
except ImportError:  # pragma: no cover
    from aer_utils import load_circuit  # type: ignore

from qiskit import qasm2


def infer_total_qubits(path: str) -> int:
    """Extract trailing digits before .txt as qubit count."""
    basename = os.path.basename(path)
    m = re.search(r"(\d+)(?=\.txt$)", basename)
    if not m:
        raise ValueError(
            f"Cannot infer total qubits from filename '{basename}'. Expected pattern like name32.txt"
        )
    return int(m.group(1))


def parse_args(argv: list[str]):
    p = argparse.ArgumentParser(description="Convert IR txt circuit to QASM 2")
    p.add_argument("circuit_file", help="Input IR circuit .txt file")
    p.add_argument(
        "--out",
        "-o",
        dest="out_file",
        help="Output QASM file (default: replace .txt with .qasm)",
    )
    p.add_argument(
        "--use-measure",
        action="store_false",
        help="Do not append measure-all (overrides load_circuit default)",
    )  # UniQ's input format do not have measurement operations
    return p.parse_args(argv)


def main(argv: list[str] | None = None):
    args = parse_args(sys.argv[1:] if argv is None else argv)
    circuit_path = args.circuit_file
    if not os.path.isfile(circuit_path):
        print(f"Input file not found: {circuit_path}", file=sys.stderr)
        return 2

    try:
        total_qubit = infer_total_qubits(circuit_path)
    except ValueError as e:
        print(str(e), file=sys.stderr)
        return 2

    stem = os.path.splitext(os.path.basename(circuit_path))[0]
    out_file = args.out_file or os.path.join(
        os.path.dirname(circuit_path), stem + ".qasm"
    )

    # load_circuit will append measurements unless we ask it not to.
    qc = load_circuit(
        circuit_path, total_qubit, stem, skip_measurement=args.use_measure
    )

    # Dump OpenQASM 2 (qasm2.dump writes file path; ensures parent dir exists)
    os.makedirs(os.path.dirname(out_file) or ".", exist_ok=True)
    qasm2.dump(qc, out_file)
    print(f"Wrote QASM to {out_file}")
    return 0


if __name__ == "__main__":  # pragma: no cover
    sys.exit(main())
