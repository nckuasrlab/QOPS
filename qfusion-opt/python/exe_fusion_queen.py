import datetime
import os
import subprocess
import sys
from dataclasses import dataclass

import numpy as np
from python.aer_utils import circuits_equivalent_by_samples, load_circuit
from python.microbenchmark_suite.gen_cost_table.queen import get_best_chunk_size
from python.queen_utils import exec_circuit
from qiskit import QuantumCircuit

SIMULATOR_BINARY = "~/Queen_CPU/Queen_MPI_gateBlock/src/simulator/Queen"
OPTIMIZER_BINARY = "~/Queen_CPU/Queen_MPI_gateBlock/src/optimizer/finder"
MICROBENCHMARK_RESULT_FILE = "./log/microbenchmark_result_queen.csv"
COST_TABLE_FILE = "./log/gate_exe_time_queen.csv"


def gen_qiskit_fusion(
    filename: str,
    fused_filename: str,
    total_qubit: int,
    fusion_method: str,
    fusion_max_qubit: int,
    verbose: bool,
) -> float:
    print(f"{'--verbose' if verbose else ''}", end="", flush=True)
    cmd = [
        "python",
        "./QiskitFusion/qiskit_fusion.py",
        "./circuit/" + filename,
        str(total_qubit),
        fusion_method,
        "--fusion_max_qubit",
        f"{fusion_max_qubit}",
        "--dynamic_cost_filename",
        COST_TABLE_FILE,
        "--output_filename",
        fused_filename,
    ]
    if verbose:
        cmd.append("--verbose")
    fusion_process = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
    )
    if fusion_process.returncode != 0:
        print("ERROR:", fusion_process.returncode)
        print(fusion_process.stdout)
        print(fusion_process.stderr)
        sys.exit(1)

    if verbose:
        print(fusion_process.stdout)
        print(fusion_process.stderr)
    fusion_time = float(fusion_process.stdout.split("\n")[-1])
    return fusion_time


@dataclass
class FusionConfig:
    fusion_method: str
    fusion_max_qubit: int
    verbose: bool = False


@dataclass
class ExecConfig:
    fusion_enable: bool
    fusion_max_qubit: int
    chunk_size: int = 12
    skip_exec: bool = False


def gen_dfgc_fusion(
    filename: str,
    fused_filename: str,
    total_qubit: int,
    fusion_method: str,
    fusion_max_qubit: int,
    verbose: bool,
) -> float:
    cmd = [
        "./fusion",
        "./circuit/" + filename,
        fused_filename,
        str(fusion_max_qubit),
        str(total_qubit),
        "3" if fusion_method == "static_dfgc" else "4",
    ]
    modified_env = os.environ.copy()
    if fusion_method == "dynamic_dfgc":
        modified_env["DYNAMIC_COST_FILENAME"] = COST_TABLE_FILE

    print(f"{'--verbose' if verbose else ''}", end="", flush=True)
    fusion_process = subprocess.run(
        cmd,
        env=modified_env,
        capture_output=True,
        text=True,
    )
    if fusion_process.returncode != 0:
        print("ERROR:", fusion_process.returncode, flush=True)
        print(fusion_process.stdout, flush=True)
        print(fusion_process.stderr, flush=True)
        sys.exit(1)

    if verbose:
        print(fusion_process.stdout, flush=True)
        print(fusion_process.stderr, flush=True)
    fusion_time = float(fusion_process.stdout.split("\n")[0].split(",")[-1])
    return fusion_time


def run_benchmark(
    circuit_name: str,
    total_qubit: int,
    fusion_config: FusionConfig,
    exec_config: ExecConfig,
    logfile,
    compare_circuit: QuantumCircuit = None,
    repeat_num: int = 5,
):
    fusion_method = fusion_config.fusion_method

    print(f"{fusion_method}: ", end="", flush=True)
    logfile.write(f"{fusion_method}:\n")

    fused_filename = f"./queenFusionCircuit/fused_{fusion_method}_{circuit_name}.txt"
    if fusion_method in ["static_qiskit", "dynamic_qiskit"]:
        fusion_time = gen_qiskit_fusion(
            f"{circuit_name}.txt",
            fused_filename,
            total_qubit,
            fusion_method,
            fusion_config.fusion_max_qubit,
            fusion_config.verbose,
        )

        qc = load_circuit(fused_filename, total_qubit, circuit_name)
        # total gate count =
        #   original gate count - all measure gates - one barrier gate before measurement
        logfile.write(
            f"{fusion_time}, {len(qc.data) - 1 - qc.num_qubits}, "
            f"{exec_config.chunk_size}, {fusion_config.fusion_max_qubit}\n"
        )

    elif fusion_method in ["static_dfgc", "dynamic_dfgc"]:
        fusion_time = gen_dfgc_fusion(
            f"{circuit_name}.txt",
            fused_filename,
            total_qubit,
            fusion_method,
            fusion_config.fusion_max_qubit,
            fusion_config.verbose,
        )
        qc = load_circuit(
            fused_filename, total_qubit, circuit_name, use_random_matrix=False
        )
        # total gate count =
        #   original gate count - all measure gates - one barrier gate before measurement
        logfile.write(
            f"{fusion_time}, {len(qc.data) - 1 - qc.num_qubits}, "
            f"{exec_config.chunk_size}, {fusion_config.fusion_max_qubit}\n"
        )

    else:
        fused_filename = f"./circuit/{circuit_name}.txt"
        qc = load_circuit(fused_filename, total_qubit, circuit_name)
        logfile.write(
            f"0, {len(qc.data) - 1 - qc.num_qubits}, {exec_config.chunk_size}\n"
        )

    if exec_config.skip_exec:
        print("", flush=True)
        return
    # Verify circuit fusion
    if compare_circuit is not None and not circuits_equivalent_by_samples(
        compare_circuit, qc
    ):
        print(f"ERROR: {fusion_method} circuits not equivalent", flush=True)

    if compare_circuit is None:  # warmup if no compare circuit
        exec_result = exec_circuit(
            SIMULATOR_BINARY,
            OPTIMIZER_BINARY,
            fusion_method,
            circuit_name,
            circuit_path=fused_filename,
            total_qubit=total_qubit,
            chunk_size=exec_config.chunk_size,
            dump_log=False,
        )
    for i in range(repeat_num):
        print(f".", end="", flush=True)
        exec_result = exec_circuit(
            SIMULATOR_BINARY,
            OPTIMIZER_BINARY,
            fusion_method,
            circuit_name,
            circuit_path=fused_filename,
            total_qubit=total_qubit,
            chunk_size=exec_config.chunk_size,
            dump_log=(
                True if i == (repeat_num // 2) else False
            ),  # we only dump log at the middle
        )
        logfile.write(str(exec_result.simulation_time) + "\n")
        logfile.flush()
        os.fsync(logfile)
    print("", flush=True)


if __name__ == "__main__":
    print(f"running {__file__ }")
    fusion_max_qubit = 5  # max_fusion_qubits
    total_qubit = 32
    best_chunk_size = get_best_chunk_size(MICROBENCHMARK_RESULT_FILE, total_qubit)
    benchmarks = ["sc", "vc", "hs", "bv", "qv", "qft", "vqc", "ising", "qaoa"]

    os.makedirs("./queenFusionCircuit", exist_ok=True)

    for benchmark in benchmarks:
        circuit_name = benchmark + str(total_qubit)
        filename = circuit_name + ".txt"
        print("==================================================================")
        print(filename)
        logfile = open(
            f"./queenFusionCircuit/experiment_{benchmark}{total_qubit}.log", "a"
        )
        logfile.write(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S") + "\n")

        # disable
        fusion_method = "disable"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit),
            logfile,
        )

        qc1 = load_circuit("./circuit/" + filename, total_qubit, circuit_name)
        # qc1 = None

        # static Qiskit
        fusion_method = "static_qiskit"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit),
            logfile,
            qc1,
        )

        # static Qiskit with best_chunk_size
        fusion_method = "static_qiskit"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit, best_chunk_size),
            logfile,
            qc1,
        )

        # dynamic Qiskit
        fusion_method = "dynamic_qiskit"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit),
            logfile,
            qc1,
        )

        # dynamic Qiskit with best_chunk_size
        fusion_method = "dynamic_qiskit"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit, best_chunk_size),
            logfile,
            qc1,
        )

        # static DFGC
        fusion_method = "static_dfgc"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit, best_chunk_size),
            logfile,
            qc1,
        )

        # dynamic DFGC
        fusion_method = "dynamic_dfgc"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit, best_chunk_size),
            logfile,
            qc1,
        )
        logfile.close()
