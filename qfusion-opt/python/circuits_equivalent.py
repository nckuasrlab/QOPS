# Usage: python circuits_equivalent.py <total_qubit> <circuit1> <circuit2>
import sys
import time

from python.aer_utils import circuits_equivalent_by_samples, load_circuit


def main():
    args = sys.argv[1:]
    print(args)
    total_qubit = int(args[0])
    circuit1 = load_circuit(args[1], total_qubit, "c1")
    circuit2 = load_circuit(args[2], total_qubit, "c2")
    t_start = time.perf_counter()
    result = circuits_equivalent_by_samples(circuit1, circuit2)
    t_end = time.perf_counter()
    print(f"Total time: {t_end - t_start}")
    if result:
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == "__main__":
    main()