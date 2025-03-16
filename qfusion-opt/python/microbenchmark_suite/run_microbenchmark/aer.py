import argparse
import os
import random

from python.common import gate_list_aer as gate_list
from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit_aer import AerSimulator

# Random Seed for Reproducibility
random.seed(0)


def unitary_matrix(number):
    matrix = []
    for i in range(int(pow(2, number))):
        row = []
        for j in range(int(pow(2, number))):
            if i == j:
                row.append(1)
            else:
                row.append(0)
        matrix.append(row)
    return matrix


def diagonal_matrix(number):
    matrix = []
    for i in range(int(pow(2, number))):
        matrix.append(1)
    return matrix


U3 = UnitaryGate(unitary_matrix(3))
U2 = UnitaryGate(unitary_matrix(2))
D1 = DiagonalGate(diagonal_matrix(1))
D2 = DiagonalGate(diagonal_matrix(2))
D3 = DiagonalGate(diagonal_matrix(3))


def gen_microbenchmark(
    num_repeat_runs,
    total_qubit_min,
    total_qubit_max,
    gate_list,
    microbenchmark_result_file,
):
    f_log = open(microbenchmark_result_file, "a")
    for gate in gate_list:
        for total_qubit in range(total_qubit_min, total_qubit_max + 1):
            for target_qubit_1 in range(total_qubit):
                circuit = QuantumCircuit(total_qubit)
                target_qubit = []
                while target_qubit_1 not in target_qubit:
                    target_qubit = random.sample(range(total_qubit), 3)
                    target_qubit.sort()
                if target_qubit.index(target_qubit_1) == 0:
                    target_qubit_2 = target_qubit[1]
                    target_qubit_3 = target_qubit[2]
                elif target_qubit.index(target_qubit_1) == 1:
                    target_qubit_2 = target_qubit[0]
                    target_qubit_3 = target_qubit[2]
                elif target_qubit.index(target_qubit_1) == 2:
                    target_qubit_2 = target_qubit[0]
                    target_qubit_3 = target_qubit[1]
                for i in range(num_repeat_runs):
                    if gate == "H":
                        circuit.h(target_qubit_1)
                    elif gate == "X":
                        circuit.x(target_qubit_1)
                    elif gate == "RX":
                        circuit.rx(3.141592653589793, target_qubit_1)
                    elif gate == "RY":
                        circuit.ry(3.141592653589793, target_qubit_1)
                    elif gate == "RZ":
                        circuit.rz(3.141592653589793, target_qubit_1)
                    elif gate == "U1":
                        circuit.u(
                            3.141592653589793,
                            3.141592653589793,
                            3.141592653589793,
                            target_qubit_1,
                        )
                    elif gate == "CX":
                        circuit.cx(target_qubit_1, target_qubit_2)
                    elif gate == "CZ":
                        circuit.cz(target_qubit_1, target_qubit_2)
                    elif gate == "CP":
                        circuit.cp(3.141592653589793, target_qubit_1, target_qubit_2)
                    elif gate == "RZZ":
                        circuit.rzz(3.141592653589793, target_qubit_1, target_qubit_2)
                    elif gate == "U2":
                        circuit.append(U2, [target_qubit_1, target_qubit_2])
                    elif gate == "U3":
                        circuit.append(
                            U3, [target_qubit_1, target_qubit_2, target_qubit_3]
                        )
                    elif gate == "D1":
                        circuit.append(D1, [target_qubit_1])
                    elif gate == "D2":
                        circuit.append(D2, [target_qubit_1, target_qubit_2])
                    elif gate == "D3":
                        circuit.append(
                            D3, [target_qubit_1, target_qubit_2, target_qubit_3]
                        )
                circuit.measure_all()
                simulator = AerSimulator(
                    method="statevector",
                    fusion_enable=False,
                )
                job = simulator.run(circuit)
                res = job.result()
                # ms
                data = (
                    float(res.metadata["time_taken_execute"]) / num_repeat_runs * 1000
                )
                f_log.write(
                    gate
                    + ", "
                    + str(total_qubit)
                    + ", "
                    + str(target_qubit_1)
                    + ", "
                    + str(data)
                    + "\n"
                )
                f_log.flush()
    f_log.close()


def get_args():
    parser = argparse.ArgumentParser(
        description="Run microbenchmark Aer",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--num_repeat_runs",
        type=int,
        help="Number of times to run the benchmark",
        metavar="NUM",
        default=100,
    )
    parser.add_argument(
        "--total_qubit_min",
        type=int,
        help="Total number of qubits",
        metavar="NUM",
        default=24,
    )
    parser.add_argument(
        "--total_qubit_max",
        type=int,
        help="Test size",
        metavar="NUM",
        default=32,
    )
    parser.add_argument(
        "--microbenchmark_result_file",
        type=str,
        help="Output microbenchmark result file",
        metavar="CSV_FILENAME",
        default="./log/microbenchmark_result_aer.csv",
    )

    return parser.parse_args()


def main():
    args = get_args()
    print(args)

    os.system(f"rm -f {args.microbenchmark_result_file}")
    gen_microbenchmark(
        args.num_repeat_runs,
        args.total_qubit_min,
        args.total_qubit_max,
        gate_list,
        args.microbenchmark_result_file,
    )


if __name__ == "__main__":
    main()
