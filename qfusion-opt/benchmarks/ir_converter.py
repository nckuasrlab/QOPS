# ref: https://github.com/wang2346581/Quokka/blob/SC24/utils/circuit.py

import sys

import numpy as np
import openqasm3
from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit_qasm3_import import parse


def qasm_to_ir(qc: QuantumCircuit, verbose=False) -> str:

    ss = []
    # the id of gate
    # id = 0

    for gate in qc.data:

        op = gate.operation.name
        if verbose:
            print(op)
        if op not in [
            "h",
            "rx",
            "rzz",
            "cp",
            "u",
            "cu1",
            "swap",
            "x",
            "cx",
            "rz",
            "cz",
            "ry",
            "u1",
        ]:
            print(f"Unknown gate: {op}")
            exit(-1)

        if op == "cu1":
            op = "cp"
        elif op == "u1":
            op == "p"

        if op == "u1":
            mat = gate.operation.to_matrix()
            mat = np.column_stack([mat.reshape(-1).real, mat.reshape(-1).imag]).reshape(
                -1
            )
            ss.append(
                f"""\
{op.upper()} \
{' '.join((str(qc.find_bit(gate.qubits[i]).index) for i in range(gate.operation.num_qubits)))} \
{' '.join(map(str, mat.tolist()))}"""
            )
        else:
            ss.append(
                f"""\
{op.upper()} \
{' '.join((str(qc.find_bit(gate.qubits[i]).index) for i in range(gate.operation.num_qubits)))} \
{' '.join((str(n) for n in gate.operation.params)) if len(gate.operation.params) else ''}"""
            )

    # append id to the tail for debug
    # ss[-1] += f" {id}"
    # id += 1

    return "\n".join(ss)


def circuit_load(circuit_path, total_qubit, reverse_qubits=True) -> QuantumCircuit:

    circ = QuantumCircuit(total_qubit)

    qubit_mapping = [q for q in range(total_qubit)]

    def diag_gate(N, circ: QuantumCircuit, op):
        mat = []
        for i in range(1 << N):
            idx = 1 + N + 2 * i
            mat.append(float(op[idx]) + float(op[idx + 1]) * 1j)
        gate = DiagonalGate(mat)
        targs = [int(op[i]) for i in range(1, N + 1)]
        circ.append(gate, targs)

    with open(circuit_path, "r") as f:
        for line in f.readlines():
            op = line.split()
            if op[0].isdigit():
                pass
            elif op[0] == "H":
                circ.h(int(op[1]))
            elif op[0] == "X":
                circ.x(int(op[1]))
            elif op[0] == "Z":
                circ.z(int(op[1]))
            elif op[0] == "RX":
                circ.rx(float(op[2]), int(op[1]))
            elif op[0] == "RY":
                circ.ry(float(op[2]), int(op[1]))
            elif op[0] == "RZ":
                circ.rz(float(op[2]), int(op[1]))
            elif op[0] == "P":
                circ.p(float(op[2]), int(op[1]))
            elif op[0] == "U":
                circ.u(float(op[2]), float(op[3]), float(op[4]), int(op[1]))
            elif op[0] == "CX":
                circ.cx(int(op[1]), int(op[2]))
            elif op[0] == "CZ":
                circ.cz(int(op[1]), int(op[2]))
            elif op[0] == "CP":
                circ.cp(float(op[3]), int(op[1]), int(op[2]))
            elif op[0] == "CRX":
                circ.crx(float(op[3]), int(op[1]), int(op[2]))
            elif op[0] == "SWAP":
                circ.swap(int(op[1]), int(op[2]))
            elif op[0] == "RZZ":
                circ.rzz(float(op[3]), int(op[1]), int(op[2]))
            elif op[0] == "D1":
                diag_gate(1, circ, op)
            elif op[0] == "D2":
                diag_gate(2, circ, op)
            elif op[0] == "D3":
                diag_gate(3, circ, op)
            elif op[0] == "D4":
                diag_gate(4, circ, op)
            elif op[0] == "D5":
                diag_gate(5, circ, op)
            elif op[0] == "U1":  # UnitaryGate
                for i in range(2, 10):
                    op[i] = float(op[i])
                gate = UnitaryGate(
                    [
                        [op[2] + op[3] * 1j, op[4] + op[5] * 1j],
                        [op[6] + op[7] * 1j, op[8] + op[9] * 1j],
                    ]
                )
                circ.append(gate, [int(op[1])])
            elif op[0] == "U2":  # 2 qubit UnitaryGate
                for i in range(3, 35):
                    op[i] = float(op[i])
                gate = UnitaryGate(
                    [
                        [
                            op[3] + op[4] * 1j,
                            op[5] + op[6] * 1j,
                            op[7] + op[8] * 1j,
                            op[9] + op[10] * 1j,
                        ],
                        [
                            op[11] + op[12] * 1j,
                            op[13] + op[14] * 1j,
                            op[15] + op[16] * 1j,
                            op[17] + op[18] * 1j,
                        ],
                        [
                            op[19] + op[20] * 1j,
                            op[21] + op[22] * 1j,
                            op[23] + op[24] * 1j,
                            op[25] + op[26] * 1j,
                        ],
                        [
                            op[27] + op[28] * 1j,
                            op[29] + op[30] * 1j,
                            op[31] + op[32] * 1j,
                            op[33] + op[34] * 1j,
                        ],
                    ]
                )
                circ.append(gate, [int(op[2]), int(op[1])])
            elif op[0] == "U3":  # 3 qubit UnitaryGate
                for i in range(4, 132):
                    op[i] = float(op[i])
                gate = UnitaryGate(
                    [
                        [
                            op[4] + op[5] * 1j,
                            op[6] + op[7] * 1j,
                            op[8] + op[9] * 1j,
                            op[10] + op[11] * 1j,
                            op[12] + op[13] * 1j,
                            op[14] + op[15] * 1j,
                            op[16] + op[17] * 1j,
                            op[18] + op[19] * 1j,
                        ],
                        [
                            op[20] + op[21] * 1j,
                            op[22] + op[23] * 1j,
                            op[24] + op[25] * 1j,
                            op[26] + op[27] * 1j,
                            op[28] + op[29] * 1j,
                            op[30] + op[31] * 1j,
                            op[32] + op[33] * 1j,
                            op[34] + op[35] * 1j,
                        ],
                        [
                            op[36] + op[37] * 1j,
                            op[38] + op[39] * 1j,
                            op[40] + op[41] * 1j,
                            op[42] + op[43] * 1j,
                            op[44] + op[45] * 1j,
                            op[46] + op[47] * 1j,
                            op[48] + op[49] * 1j,
                            op[50] + op[51] * 1j,
                        ],
                        [
                            op[52] + op[53] * 1j,
                            op[54] + op[55] * 1j,
                            op[56] + op[57] * 1j,
                            op[58] + op[59] * 1j,
                            op[60] + op[61] * 1j,
                            op[62] + op[63] * 1j,
                            op[64] + op[65] * 1j,
                            op[66] + op[67] * 1j,
                        ],
                        [
                            op[68] + op[69] * 1j,
                            op[70] + op[71] * 1j,
                            op[72] + op[73] * 1j,
                            op[74] + op[75] * 1j,
                            op[76] + op[77] * 1j,
                            op[78] + op[79] * 1j,
                            op[80] + op[81] * 1j,
                            op[82] + op[83] * 1j,
                        ],
                        [
                            op[84] + op[85] * 1j,
                            op[86] + op[87] * 1j,
                            op[88] + op[89] * 1j,
                            op[90] + op[91] * 1j,
                            op[92] + op[93] * 1j,
                            op[94] + op[95] * 1j,
                            op[96] + op[97] * 1j,
                            op[98] + op[99] * 1j,
                        ],
                        [
                            op[100] + op[101] * 1j,
                            op[102] + op[103] * 1j,
                            op[104] + op[105] * 1j,
                            op[106] + op[107] * 1j,
                            op[108] + op[109] * 1j,
                            op[110] + op[111] * 1j,
                            op[112] + op[113] * 1j,
                            op[114] + op[115] * 1j,
                        ],
                        [
                            op[116] + op[117] * 1j,
                            op[118] + op[119] * 1j,
                            op[120] + op[121] * 1j,
                            op[122] + op[123] * 1j,
                            op[124] + op[125] * 1j,
                            op[126] + op[127] * 1j,
                            op[128] + op[129] * 1j,
                            op[130] + op[131] * 1j,
                        ],
                    ]
                )
                circ.append(gate, [int(op[3]), int(op[2]), int(op[1])])
            elif op[0] == "SQS" or op[0] == "CSQS":
                numSwap = int(op[1])
                targs = list(map(int, op[2:]))
                for i, j in zip(targs[:numSwap], targs[numSwap:]):
                    circ.swap(i, j)
                    qubit_mapping[i], qubit_mapping[j] = (
                        qubit_mapping[j],
                        qubit_mapping[i],
                    )
            else:
                print(f"Unknown gate: {line}")
                exit()

        if reverse_qubits:
            for i, val in enumerate(qubit_mapping):
                if i != val:
                    j = qubit_mapping.index(i)
                    circ.swap(i, j)
                    qubit_mapping[i], qubit_mapping[j] = (
                        qubit_mapping[j],
                        qubit_mapping[i],
                    )

        return circ


if __name__ == "__main__":
    """
    Usage: python ir_converter.py <circuit.qasm> <output.ir>

    Converts a QASM2 or QASM3 circuit file to an IR representation and saves it to the specified output file.
    """
    circuit_path = sys.argv[1]
    ir_path = sys.argv[2]

    circuit_file = ""
    with open(circuit_path, "r") as f:
        circuit_file = f.read()

    try:
        qc = openqasm3.parse(circuit_file)
    except:
        print("Error: OpenQASM parsing failed.")
        sys.exit(1)
    version = qc.version

    if version is not None and float(version) < 3.0:  # QASM 2
        try:
            qc = QuantumCircuit.from_qasm_str(circuit_file)
        except:
            print("Error: QASM2 parsing succeeded, but the loading process failed.")
            sys.exit(1)
    else:  # QASM 3
        try:
            # from qbraid import transpile
            # qc = transpile(circuit_file, "qiskit")

            # from qiskit import qasm3
            # qc = qasm3.loads_experimental(circuit_file)

            qc = parse(circuit_file)
            # print(qc)
        except:
            print("Error: QASM3 parsing succeeded, but the loading process failed.")
            sys.exit(1)

    # print(qc)
    ir = qasm_to_ir(qc)
    # print("==== IR ====")
    # print(ir)
    with open(ir_path, "w") as f:
        f.write(ir)
