import argparse
import os
import random
import time

import numpy as np
from python.common import gate_list_aer as gate_list
from python.microbenchmark_suite.run_microbenchmark.random_param import (
    random_diagonal_gate,
    random_theta,
    random_u_gate_parameters,
    random_unitary_matrix,
)
from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit_aer import AerSimulator

# Random Seed for Reproducibility
random.seed(0)
np.random.seed(0)


def gen_microbenchmark(
    num_repeat_runs,
    total_qubit_min,
    total_qubit_max,
    gate_list,
    microbenchmark_result_file,
):
    f_log = open(microbenchmark_result_file, "a")
    for gate in gate_list:
        for total_qubit in range(total_qubit_min, total_qubit_max + 1, 2):
            for target_qubit_1 in range(0, total_qubit, 4):
                circuit = QuantumCircuit(total_qubit)
                sample_pool = [i for i in range(total_qubit) if i != target_qubit_1]
                other_qubit = random.sample(sample_pool, 4)
                for i in range(num_repeat_runs):
                    if gate == "H":
                        circuit.h(target_qubit_1)
                    elif gate == "X":
                        circuit.x(target_qubit_1)
                    elif gate == "RX":
                        circuit.rx(random_theta(), target_qubit_1)
                    elif gate == "RY":
                        circuit.ry(random_theta(), target_qubit_1)
                    elif gate == "RZ":
                        circuit.rz(random_theta(), target_qubit_1)
                    elif gate == "U1":
                        circuit.u(
                            *random_u_gate_parameters(),
                            target_qubit_1,
                        )
                    elif gate == "CX":
                        circuit.cx(target_qubit_1, *other_qubit[:1])
                    elif gate == "CZ":
                        circuit.cz(target_qubit_1, *other_qubit[:1])
                    elif gate == "CP":
                        circuit.cp(random_theta(), target_qubit_1, *other_qubit[:1])
                    elif gate == "RZZ":
                        circuit.rzz(
                            random_theta(), target_qubit_1, *other_qubit[:1]
                        )
                    elif gate == "U2":
                        circuit.append(
                            UnitaryGate(random_unitary_matrix(2)),
                            [target_qubit_1, *other_qubit[:1]],
                        )
                    elif gate == "U3":
                        circuit.append(
                            UnitaryGate(random_unitary_matrix(3)),
                            [target_qubit_1, *other_qubit[:2]],
                        )
                    elif gate == "U4":
                        circuit.append(
                            UnitaryGate(random_unitary_matrix(4)),
                            [target_qubit_1, *other_qubit[:3]],
                        )
                    elif gate == "U5":
                        circuit.append(
                            UnitaryGate(random_unitary_matrix(5)),
                            [target_qubit_1, *other_qubit[:4]],
                        )
                    elif gate == "D1":
                        circuit.append(
                            DiagonalGate(random_diagonal_gate(1)), [target_qubit_1]
                        )
                    elif gate == "D2":
                        circuit.append(
                            DiagonalGate(random_diagonal_gate(2)),
                            [target_qubit_1, *other_qubit[:1]],
                        )
                    elif gate == "D3":
                        circuit.append(
                            DiagonalGate(random_diagonal_gate(3)),
                            [target_qubit_1, *other_qubit[:2]],
                        )
                    elif gate == "D4":
                        circuit.append(
                            DiagonalGate(random_diagonal_gate(4)),
                            [target_qubit_1, *other_qubit[:3]],
                        )
                    elif gate == "D5":
                        circuit.append(
                            DiagonalGate(random_diagonal_gate(5)),
                            [target_qubit_1, *other_qubit[:4]],
                        )
                circuit.measure_all()
                simulator = AerSimulator(
                    method="statevector",
                    fusion_enable=False,
                    seed_simulator=0,
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
        help="Min total number of qubits",
        metavar="NUM",
        default=24,
    )
    parser.add_argument(
        "--total_qubit_max",
        type=int,
        help="Max total number of qubits",
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

    t_start = time.perf_counter()
    os.system(f"rm -f {args.microbenchmark_result_file}")
    gen_microbenchmark(
        args.num_repeat_runs,
        args.total_qubit_min,
        args.total_qubit_max,
        gate_list,
        args.microbenchmark_result_file,
    )
    t_end = time.perf_counter()
    print(f"Total time: {t_end - t_start}")


if __name__ == "__main__":
    main()
