import argparse
import math
import os
import random
import subprocess
import sys
import time
from itertools import repeat

import numpy as np
from python.common import gate_list_queen as gate_list
from python.microbenchmark_suite.run_microbenchmark.random_param import (
    random_diagonal_gate,
    random_theta,
    random_u_gate_parameters,
    random_unitary_matrix,
)

# Random Seed for Reproducibility
random.seed(0)
np.random.seed(0)


def set_ini_file(total_qubit, num_file_qubit, chunk_size, simulation_type):
    f = open("sub_cpu.ini", "w")
    if simulation_type == "MEM":
        f.write(
            "[system]\n"
            + f"total_qbit={total_qubit}\n"
            + f"device_qbit=0\n"
            + f"chunk_qbit={chunk_size}\n"
            + f"buffer_qbit=26\n"
            + f"threads_bit={num_file_qubit}\n"
        )
    f.close()


# TODO
# - random diagonal and unitary matrix
# - chunk_size is used to reduce the number of VSWAP2-2, so the benchmark should rerun.
#       The test program could be applying gate from q0 to qN then run finder then simulate.
#       The result will show the larger the chunk_size, the smaller the number of VSWAP2-2 (faster simulation time).
def gen_microbenchmark(
    simulator_binary,
    optimizer_binary,
    tmp_circuit_dir,
    num_repeat_runs,
    total_qubit_min,
    total_qubit_max,
    chunk_min,
    chunk_max,
    simulation_type,
    num_file_qubit,
    gate_list,
    microbenchmark_result_file,
):
    f_log = open(microbenchmark_result_file, "a")
    for total_qubit in range(total_qubit_min, total_qubit_max + 1):
        gate_position_map = [
            random.sample(range(total_qubit), 5) for _ in range(num_repeat_runs)
        ]
        for gate in gate_list:
            ori_circuit_name = f"{tmp_circuit_dir}/" + gate + ".txt"
            ori_circuit = open(ori_circuit_name, "w")
            if gate in ["H", "X", "RX", "RY", "RZ"]:
                for i in range(num_repeat_runs):
                    target = gate_position_map[i]
                    ori_circuit.write(f"{gate} {target[0]}")
                    if gate[0] == "R":
                        ori_circuit.write(f" {random_theta()}")
                    ori_circuit.write("\n");
            elif gate in ["CX", "CZ", "CP", "RZZ"]:
                for i in range(num_repeat_runs):
                    target = gate_position_map[i]
                    ori_circuit.write(f"{gate} {target[0]} {target[1]}")
                    if gate == "CP" or gate == "RZZ":
                        ori_circuit.write(f" {random_theta()}")
                    ori_circuit.write("\n")
            elif gate[0] == "U":
                qubits = int(gate[1:])
                for idx in range(num_repeat_runs):
                    target = gate_position_map[idx]
                    ori_circuit.write(f"{gate}")
                    for i in range(qubits):
                        ori_circuit.write(f" {target[i]}")
                    m = random_unitary_matrix(qubits)
                    for i in range(2**qubits):
                        for j in range(2**qubits):
                            ori_circuit.write(f" {m[i][j].real} {m[i][j].imag}")
                    ori_circuit.write("\n")
            elif gate[0] == "D":
                qubits = int(gate[1:])
                for idx in range(num_repeat_runs):
                    target = gate_position_map[idx]
                    ori_circuit.write(f"{gate}")
                    for i in range(qubits):
                        ori_circuit.write(f" {target[i]}")
                    m = random_diagonal_gate(qubits)
                    for i in range(2**qubits):
                        ori_circuit.write(f" {m[i].real} {m[i].imag}")
                    ori_circuit.write("\n")
            ori_circuit.close()

            # continue
            for chunk_size in range(chunk_min, chunk_max + 1):
                set_ini_file(total_qubit, num_file_qubit, chunk_size, simulation_type)
                opt_circuit_name = f"{tmp_circuit_dir}/" + gate + "_opt.txt"
                # continue
                opt_circuit = open(opt_circuit_name, "w")
                # run finder
                opt = subprocess.run(
                    [
                        optimizer_binary,
                        ori_circuit_name,
                        str(chunk_size),
                        str(total_qubit),
                        str(total_qubit),
                        "1",
                        "0",
                        "5",
                        "0",
                    ],
                    stdout=opt_circuit,
                )
                opt_circuit.close()

                # run simulator
                output = subprocess.run(
                    [simulator_binary, "-i", "sub_cpu.ini", "-c", opt_circuit_name],
                    capture_output=True,
                    text=True,
                )
                # ms
                data = (
                    float(output.stdout.split("\n")[-2].split(" ")[-2])
                    / num_repeat_runs
                    * 1000
                )
                f_log.write(
                    gate
                    + ", "
                    + str(total_qubit)
                    + ", "
                    + str(chunk_size)
                    + ", "
                    + str(data)
                    + "\n"
                )
    f_log.close()


def get_args():
    parser = argparse.ArgumentParser(
        description="Run microbenchmark Queen",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--simulator_binary",
        type=str,
        help="Simulator binary",
        metavar="PATH",
        default="./Queen",
    )
    parser.add_argument(
        "--optimizer_binary",
        type=str,
        help="Optimizer binary",
        metavar="PATH",
        default="./finder",
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
        help="Total number of qubits minimum",
        metavar="NUM",
        default=24,
    )
    parser.add_argument(
        "--total_qubit_max",
        type=int,
        help="Total number of qubits maximum",
        metavar="NUM",
        default=32,
    )
    parser.add_argument(
        "--chunk_min",
        type=int,
        help="Chunk chunk_size minimum",
        metavar="NUM",
        default=10,
    )
    parser.add_argument(
        "--chunk_max",
        type=int,
        help="Chunk chunk_size maximum",
        metavar="NUM",
        default=20,
    )
    parser.add_argument(
        "--simulation_type",
        type=str,
        choices=["MEM"],
        help="Runner type",
        default="MEM",
    )
    parser.add_argument(
        "--microbenchmark_result_file",
        type=str,
        help="Output microbenchmark result file",
        metavar="CSV_FILENAME",
        default="./log/microbenchmark_result_queen.csv",
    )
    return parser.parse_args()


def main():
    args = get_args()
    print(args)

    t_start = time.perf_counter()
    os.system(f"rm -f {args.microbenchmark_result_file}")
    tmp_circuit_dir = "./tmp/microbenchmark_queen"
    os.makedirs(tmp_circuit_dir, exist_ok=True)
    os.system(f"rm -f {tmp_circuit_dir}/*")
    cpu_info = subprocess.run(
        ["lscpu | grep -E '^Core|^Socket'"], shell=True, capture_output=True, text=True
    ).stdout.split("\n")
    core_number = int(cpu_info[0].split()[-1])
    socket_number = int(cpu_info[1].split()[-1])
    num_file_qubit = int(math.log(core_number * socket_number, 2))

    gen_microbenchmark(
        args.simulator_binary,
        args.optimizer_binary,
        tmp_circuit_dir,
        args.num_repeat_runs,
        args.total_qubit_min,
        args.total_qubit_max,
        args.chunk_min,
        args.chunk_max,
        args.simulation_type,
        num_file_qubit,
        gate_list,
        args.microbenchmark_result_file,
    )
    t_end = time.perf_counter()
    print(f"Total time: {t_end - t_start}")


if __name__ == "__main__":
    main()
