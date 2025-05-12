import math
import os
import subprocess
from dataclasses import dataclass


@dataclass
class ExecResult:
    simulation_time: float
    internal_fusio_applied: bool
    internal_fusion_time: float
    internal_fused_gate_num: int

    def __str__(self):
        return (
            f"{self.simulation_time}, "
            f"{self.internal_fusio_applied}, "
            f"{self.internal_fusion_time}, "
            f"{self.internal_fused_gate_num}"
        )


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


def get_num_file_qubit():
    cpu_info = subprocess.run(
        ["lscpu | grep -E '^Core|^Socket'"], shell=True, capture_output=True, text=True
    ).stdout.split("\n")

    core_number = int(cpu_info[0].split()[-1])
    socket_number = int(cpu_info[1].split()[-1])
    return int(math.log(core_number * socket_number, 2))


def exec_circuit(
    simulator_binary: str,
    optimizer_binary: str,
    fusion_method: str,
    circuit_name: str,
    circuit_path: str,
    total_qubit,
    chunk_size,
    dump_log: bool,
) -> ExecResult:
    tmp_circuit_dir = "./tmp/microbenchmark_queen"
    os.makedirs(tmp_circuit_dir, exist_ok=True)

    set_ini_file(total_qubit, get_num_file_qubit(), chunk_size, "MEM")
    opt_circuit_name = f"{tmp_circuit_dir}/" + circuit_name + "_opt.txt"

    opt_circuit = open(opt_circuit_name, "w")
    # run finder
    opt = subprocess.run(
        [
            os.path.expanduser(optimizer_binary),
            circuit_path,
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
        [
            os.path.expanduser(simulator_binary),
            "-i",
            "sub_cpu.ini",
            "-c",
            opt_circuit_name,
        ],
        capture_output=True,
        text=True,
    )

    # dump execution log
    if dump_log:
        log_filename = f"./queenFusionCircuit/queen_{fusion_method}_{circuit_name}.log"
        with open(log_filename, "w") as f:
            f.write(str(output.stdout) + "\n")

    simulation_time = float(output.stdout.split("\n")[-2].split(" ")[-2])
    return ExecResult(simulation_time, False, 0.0, 0)
