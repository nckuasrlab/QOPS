import cirq, time
from cirq.contrib.qasm_import import circuit_from_qasm
import qsimcirq


def main():
    # # ✅ This QASM works with cirq.contrib.qasm_import
    # qasm_str = """
    # OPENQASM 2.0;
    # include "qelib1.inc";
    # qreg q[3];
    # h q[0];
    # cx q[0], q[1];
    # """

    # Initialize qsim simulator
    opt = qsimcirq.QSimOptions(
        cpu_threads=64,
    )
    qsim = qsimcirq.QSimSimulator(qsim_options=opt, seed=0)
    # qsim = cirq.Simulator(seed=0)
    benchmarks = ["sc", "vc", "hs", "bv", "qv", "qft", "vqc", "ising", "qaoa"]

    for benchmark in benchmarks:
        qasm_file = f"circuit/qasm/{benchmark}32.qasm"
        qasm_str = ""
        with open(qasm_file, "r") as f:
            qasm_str = f.read()
        print(f"Running benchmark: {benchmark}")
        # run_benchmark(qasm_str, qsim)
        # Load circuit from QASM string
        circuit = circuit_from_qasm(qasm_str)

        # Run simulation (statevector)
        for _ in range(5):  # run multiple times
            start = time.perf_counter()
            result = qsim.simulate(circuit)  # exclude measurement for statevector
            end = time.perf_counter()
            # print("\nFinal statevector:")
            # print(result.final_state_vector)
            print(end - start, flush=True)


if __name__ == "__main__":
    main()
