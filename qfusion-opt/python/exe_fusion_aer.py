import os
import subprocess
import sys
from dataclasses import dataclass

from python.aer_utils import circuits_equivalent_by_samples, exec_circuit, load_circuit
from qiskit import QuantumCircuit


def gen_qiskit_fusion(
    filename: str, total_qubit: int, fusion_method: str, fusion_max_qubit: int
) -> float:
    fusion_process = subprocess.run(
        [
            "python",
            "./QiskitFusion/qiskit_fusion.py",
            "./circuit/" + filename,
            str(total_qubit),
            fusion_method,
            "--fusion_max_qubit",
            f"{fusion_max_qubit}",
            "--dynamic_cost_filename",
            "./log/gate_exe_time.csv",
            "--output_filename",
            f"./qiskitFusionCircuit/fused_{fusion_method}_{filename}",
        ],
        capture_output=True,
        text=True,
    )
    if fusion_process.returncode != 0:
        print("ERROR:", fusion_process.returncode)
        print(fusion_process.stdout)
        print(fusion_process.stderr)
        sys.exit(1)

    fusion_time = float(fusion_process.stdout.split("\n")[-1])
    return fusion_time


@dataclass
class FusionConfig:
    fusion_method: str
    fusion_max_qubit: int


@dataclass
class ExecConfig:
    fusion_enable: bool
    fusion_max_qubit: int


def run_benchmark(
    circuit_name: str,
    total_qubit: int,
    fusion_config: FusionConfig,
    exec_config: ExecConfig,
    logfile,
    compare_circuit: QuantumCircuit = None,
    repeat_num: int = 10,
):
    fusion_method = fusion_config.fusion_method

    print(f"{fusion_method}: ", flush=True)
    logfile.write(f"{fusion_method}:\n")
    if fusion_method in ["static_qiskit", "dynamic_qiskit"]:
        fusion_time = gen_qiskit_fusion(
            f"{circuit_name}.txt",
            total_qubit,
            fusion_method,
            fusion_config.fusion_max_qubit,
        )

        qc = load_circuit(
            f"./qiskitFusionCircuit/fused_{fusion_method}_{circuit_name}.txt",
            total_qubit,
            circuit_name,
        )
        logfile.write(f"{fusion_time}, {len(qc.data) - qc.num_qubits}\n")
    else:
        qc = load_circuit(f"./circuit/{circuit_name}.txt", total_qubit, circuit_name)
    if compare_circuit is not None and not circuits_equivalent_by_samples(
        compare_circuit, qc
    ):
        print(f"ERROR: {fusion_method} circuits not equivalent", flush=True)

    exec_result = exec_circuit(
        qc,
        fusion_method,
        exec_config.fusion_max_qubit,
        exec_config.fusion_enable,
        False,
    )  # warmup
    for i in range(repeat_num):
        exec_result = exec_circuit(
            qc,
            fusion_method,
            exec_config.fusion_max_qubit,
            exec_config.fusion_enable,
            True if i == (repeat_num // 2) else False,
        )
        logfile.write(str(exec_result) + "\n")


if __name__ == "__main__":
    fusion_max_qubit = 3  # max_fusion_qubits
    total_qubit = 24
    benchmarks = ["sc", "vc", "hs", "qv", "bv", "qft", "qaoa", "ising"]
    # benchmarks = ["sc", "vc"]

    os.makedirs("./qiskitFusionCircuit", exist_ok=True)

    for benchmark in benchmarks:
        circuit_name = benchmark + str(total_qubit)
        filename = circuit_name + ".txt"
        print("==================================================================")
        print(filename)
        logfile = open(
            f"./qiskitFusionCircuit/experiment_{benchmark}{total_qubit}.log", "w"
        )

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
        fusion_method = "origin"
        run_benchmark(
            circuit_name,
            total_qubit,
            FusionConfig(fusion_method, fusion_max_qubit),
            ExecConfig(True, fusion_max_qubit),
            logfile,
        )
        qc1 = load_circuit("./circuit/" + filename, total_qubit, circuit_name)

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
        # result = subprocess.run(
        #     [
        #         "./fusion",
        #         "./circuit/" + filename,
        #         "./fusionCircuit/fused_q_s_" + filename,
        #         str(mfq),
        #         str(total_qubit),
        #         "5",
        #     ],
        #     capture_output=True,
        #     text=True,
        # )
        # print("static DFGC time: ", result.stdout.split()[-1])
        # print(
        #     exec_circuit(
        #         "./fusionCircuit/fused_q_s_" + filename,
        #         total_qubit,
        #         True,
        #         mfq,
        #         False,
        #     )
        # )

        # # dynamic DFGC
        # result = subprocess.run(
        #     [
        #         "./fusion",
        #         "./circuit/" + filename,
        #         "./fusionCircuit/fused_q_d_" + filename,
        #         str(mfq),
        #         str(total_qubit),
        #         "8",
        #     ],
        #     capture_output=True,
        #     text=True,
        # )
        # print("dynamic DFGC time: ", result.stdout.split()[-1])
        # print(
        #     exec_circuit(
        #         "./fusionCircuit/fused_q_d_" + filename,
        #         total_qubit,
        #         True,
        #         mfq,
        #         False,
        #     )
        # )
        logfile.close()
