from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from typing import Dict, Iterable, Optional, Tuple

# CLI usage (backwards compatible defaults):
#   python3 estimate_cost_tmp.py [circuit_file] [dynamic_cost_file] [total_qubits] [chunk_size]


@dataclass(frozen=True)
class Config:
    circuit_path: str
    csv_path: str
    total_qubits: int
    chunk_size: int
    normalize_gate: bool = False  # Optional: normalize gate names to uppercase
    show_breakdown: bool = False  # Optional: print per-gate counts and cost


def parse_args(argv: Optional[Iterable[str]] = None) -> Config:
    """Parse CLI arguments with compatibility for positional argv order used previously."""
    parser = argparse.ArgumentParser(
        description="Estimate circuit runtime from microbenchmark CSV.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "circuit",
        nargs="?",
        default="xxx.txt",
        help="Circuit file with one gate per line (first token is gate name)",
    )
    parser.add_argument(
        "csv",
        nargs="?",
        default="log/microbenchmark_result_aer.csv",
        help="CSV file of microbenchmark results: gate, total_qubits, chunk_size, cost_ms",
    )
    parser.add_argument(
        "total_qubits",
        nargs="?",
        type=int,
        default=32,
        help="Total number of qubits used to filter CSV rows",
    )
    parser.add_argument(
        "chunk_size",
        nargs="?",
        type=int,
        default=20,
        help="Chunk size used to filter CSV rows",
    )
    parser.add_argument(
        "--normalize-gate",
        action="store_true",
        help="Normalize gate names to uppercase for matching between circuit and CSV",
    )
    parser.add_argument(
        "--breakdown",
        action="store_true",
        help="Print per-gate breakdown of counts and cost",
    )
    # If argv is None, parse from sys.argv (so -h works). Otherwise parse provided iterable.
    args = parser.parse_args(list(argv) if argv is not None else None)
    return Config(
        circuit_path=args.circuit,
        csv_path=args.csv,
        total_qubits=args.total_qubits,
        chunk_size=args.chunk_size,
        normalize_gate=args.normalize_gate,
        show_breakdown=args.breakdown,
    )


def _normalize_gate(name: str, enable: bool) -> str:
    return name.upper() if enable else name


def load_cost_table(
    csv_path: str, total_qubits: int, chunk_size: int, normalize_gate: bool
) -> Dict[str, float]:
    """Load a mapping of gate -> cost_ms filtered by total_qubits and chunk_size.

    CSV format (no header expected): gate, total_qubits, chunk_size, cost_ms
    Whitespace around fields is ignored.
    """
    cost: Dict[str, float] = {}
    with open(csv_path, "r", newline="") as f:
        reader = csv.reader(f)
        for row in reader:
            if not row or row[0].strip().startswith("#"):
                continue
            if len(row) < 4:
                # Skip malformed rows quietly
                continue
            try:
                gate = _normalize_gate(row[0].strip(), normalize_gate)
                tq = int(row[1].strip())
                cs = int(row[2].strip())
                cost_ms = float(row[3].strip())
            except ValueError:
                # Skip rows that can't be parsed
                continue
            if tq == total_qubits and cs == chunk_size:
                cost[gate] = cost_ms
    return cost


def iter_circuit_gates(circuit_path: str, normalize_gate: bool) -> Iterable[str]:
    """Yield gate names (first whitespace-separated token) from circuit file."""
    with open(circuit_path, "r") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            if not parts:
                continue
            yield _normalize_gate(parts[0], normalize_gate)


def estimate_cost(config: Config) -> Tuple[float, Dict[str, Tuple[int, float]]]:
    """Compute total estimated time (milliseconds) and per-gate breakdown.

    Returns: (total_ms, breakdown) where breakdown[gate] = (count, cost_per_gate_ms)
    """
    cost_table = load_cost_table(
        config.csv_path, config.total_qubits, config.chunk_size, config.normalize_gate
    )
    if not cost_table:
        raise SystemExit(
            f"No matching CSV rows in '{config.csv_path}' for total_qubits={config.total_qubits}, "
            f"chunk_size={config.chunk_size}."
        )

    # Count gates then multiply for efficiency
    counts: Dict[str, int] = {}
    for gate in iter_circuit_gates(config.circuit_path, config.normalize_gate):
        counts[gate] = counts.get(gate, 0) + 1

    missing = [g for g in counts.keys() if g not in cost_table]
    if missing:
        missing_str = ", ".join(sorted(missing))
        raise SystemExit(
            f"Missing cost entries for gates in circuit: {missing_str}. "
            f"Check CSV file or use --normalize-gate if casing differs."
        )

    breakdown: Dict[str, Tuple[int, float]] = {
        g: (cnt, float(cost_table[g])) for g, cnt in sorted(counts.items())
    }
    total_ms = sum(cnt * cost_ms for _, (cnt, cost_ms) in breakdown.items())
    return total_ms, breakdown


def main(argv: Iterable[str] | None = None) -> int:
    config = parse_args(argv)
    print(config)
    total_ms, breakdown = estimate_cost(config)
    total_s = total_ms / 1000.0
    print(f"Estimated time: {total_s:.2f} s ({total_ms:.0f} ms)")
    if config.show_breakdown:
        print("\nBreakdown (gate: count x cost_ms = subtotal_ms):")
        for gate, (cnt, cost_ms) in breakdown.items():
            print(f"  {gate}: {cnt} x {cost_ms:.3f} = {cnt * cost_ms:.3f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
