import json
import os
import sys

import networkx as nx
import numpy as np
import qibojit_circ_template as qibojit
import qiskit
from ir_converter import qasm_to_ir
from qiskit import qasm2

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from python.circuit_drawer import draw_circuit


def gen_qaoa_file(graph):
    filename = f"/tmp/qaoa_graph.json"
    with open(filename, "w") as f:
        f.write(json.dumps(nx.node_link_data(graph)))
    return filename


if __name__ == "__main__":
    cirs = {
        "hs": lambda q: qibojit.HiddenShift(q),
        "bv": lambda q: qibojit.BernsteinVazirani(q),
        "qaoa": lambda q: qibojit.QAOA(q, 5, graph=gen_qaoa_file(nx.complete_graph(q))),
        "qft": lambda q: qibojit.QFT(q, swaps=True),
        "qv": lambda q: qibojit.QuantumVolume(q, depth=3),
        "sc": lambda q: qibojit.SupremacyCircuit(q, depth=10),
        "vc": lambda q: qibojit.VariationalCircuit(q, nlayers=3),
    }

    for q in [32]:
        for cir_name, cir_class in cirs.items():
            np.random.seed(123)
            qasms = cir_class(q).to_qasm()
            qc = qiskit.QuantumCircuit.from_qasm_str(qasms)
            qc.remove_final_measurements(True)  # affect gate's ordering

            ir_output_dir = "ir"
            os.makedirs(f"{ir_output_dir}/", exist_ok=True)
            with open(f"{ir_output_dir}/{cir_name}{q}.txt", "w") as f:
                print(qasm_to_ir(qc), end="", file=f, flush=True)

            qasm_output_dir = "qasm"
            os.makedirs(f"{qasm_output_dir}/", exist_ok=True)
            qasm2.dump(qc, f"{qasm_output_dir}/{cir_name}{q}.qasm")

            # draw_circuit(
            #     f"{qasm_output_dir}/{cir_name}{q}.qasm",
            #     filename=f"{qasm_output_dir}/{cir_name}{q}.png",
            # )
