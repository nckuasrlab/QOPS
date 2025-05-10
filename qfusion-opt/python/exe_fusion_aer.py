import datetime
import os
import subprocess
import sys
from dataclasses import dataclass

from python.aer_utils import circuits_equivalent_by_samples, exec_circuit, load_circuit
from qiskit import QuantumCircuit


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
        "./log/gate_exe_time_aer.csv",
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
        "5" if fusion_method == "static_dfgc" else "8",
    ]
    modified_env = os.environ.copy()
    if fusion_method == "dynamic_dfgc":
        modified_env["DYNAMIC_COST_FILENAME"] = "./log/gate_exe_time_aer.csv"

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
    repeat_num: int = 3,
):
    fusion_method = fusion_config.fusion_method

    print(f"{fusion_method}: ", end="", flush=True)
    logfile.write(f"{fusion_method}:\n")

    fused_filename = f"./qiskitFusionCircuit/fused_{fusion_method}_{circuit_name}.txt"
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
        logfile.write(f"{fusion_time}, {len(qc.data) - 1 - qc.num_qubits}\n")

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
        logfile.write(f"{fusion_time}, {len(qc.data) - 1 - qc.num_qubits}\n")

    else:
        qc = load_circuit(f"./circuit/{circuit_name}.txt", total_qubit, circuit_name)

    if exec_config.skip_exec:
        print("", flush=True)
        return
    # Verify circuit fusion
    if compare_circuit is not None and not circuits_equivalent_by_samples(
        compare_circuit, qc
    ):
        print(f"ERROR: {fusion_method} circuits not equivalent", flush=True)
    
    if compare_circuit is None: # warmup if no compare circuit
        exec_result = exec_circuit(
            qc,
            fusion_method,
            exec_config.fusion_max_qubit,
            exec_config.fusion_enable,
            False,
        )
    for i in range(repeat_num):
        print(f".", end="", flush=True)
        exec_result = exec_circuit(
            qc,
            fusion_method,
            exec_config.fusion_max_qubit,
            exec_config.fusion_enable,
            True if i == (repeat_num // 2) else False,  # we only dump log at the middle
        )
        logfile.write(str(exec_result) + "\n")
        logfile.flush()
        os.fsync(logfile)
    print("", flush=True)


if __name__ == "__main__":
    fusion_max_qubit = 3  # max_fusion_qubits
    total_qubit = 32
    benchmarks = ["sc", "vc", "hs", "bv", "qv", "qft", "qaoa"]  # , "ising"

    os.makedirs("./qiskitFusionCircuit", exist_ok=True)

    for benchmark in benchmarks:
        circuit_name = benchmark + str(total_qubit)
        filename = circuit_name + ".txt"
        print("==================================================================")
        print(filename)
        logfile = open(
            f"./qiskitFusionCircuit/experiment_{benchmark}{total_qubit}.log", "a"
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

        # origin
        # fusion_method = "origin"
        # run_benchmark(
        #     circuit_name,
        #     total_qubit,
        #     FusionConfig(fusion_method, fusion_max_qubit),
        #     ExecConfig(True, fusion_max_qubit),
        #     logfile,
        # )
        # qc1 = load_circuit("./circuit/" + filename, total_qubit, circuit_name)
        qc1 = None

        # static Qiskit = origin
        fusion_method = "static_qiskit"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit),
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

        # static DFGC
        fusion_method = "static_dfgc"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit),
            logfile,
            qc1,
        )

        # dynamic DFGC
        fusion_method = "dynamic_dfgc"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(False, fusion_max_qubit),
            logfile,
            qc1,
        )
        logfile.close()
