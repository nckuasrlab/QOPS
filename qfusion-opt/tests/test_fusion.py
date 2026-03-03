"""
Comprehensive test suite for qfusion-opt gate fusion correctness.

Tests verify that the fusion process preserves the quantum state by comparing
the state vectors of the original and fused circuits using Qiskit's Statevector.

Test categories:
1. Single Gate Tests - each individual gate type
2. 1-Qubit Multi-Gate Tests - sequences on a single qubit
3. 2-Qubit Unitary Fusion Tests - gates fusing to U2
4. 3-Qubit Unitary Fusion Tests - gates fusing to U3
5. 4-Qubit Unitary Fusion Tests - gates fusing to U4
6. 5-Qubit Unitary Fusion Tests - gates fusing to U5

pytest tests/test_fusion.py -v --tb=short 2>&1
python -m pytest tests/test_fusion.py::TestLargeCase::test_large_case_21q -v --tb=long 2>&1
"""

import math
import os
import subprocess
import tempfile

import numpy as np
import pytest
from qiskit import QuantumCircuit
from qiskit.circuit.library import DiagonalGate, UnitaryGate
from qiskit.quantum_info import Statevector

# ---------------------------------------------------------------------------
# Path constants
# ---------------------------------------------------------------------------
FUSION_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FUSION_EXE = os.path.join(FUSION_DIR, "fusion")
COST_FILE = os.path.join(FUSION_DIR, "log", "gate_exe_time_queen.csv")

# Tolerance for state vector comparison
ATOL = 1e-6


# ---------------------------------------------------------------------------
# Circuit loader (extended from benchmarks/ir_converter.py)
# Supports all gate types including U4, U5
# ---------------------------------------------------------------------------
def circuit_load(circuit_path: str, total_qubit: int) -> QuantumCircuit:
    """Load an IR file into a Qiskit QuantumCircuit.

    Extended to handle U4 and U5 unitary gates produced by the fusion process.
    """
    circ = QuantumCircuit(total_qubit)
    qubit_mapping = list(range(total_qubit))

    def _diag_gate(n: int, circ: QuantumCircuit, op: list[str]):
        mat = []
        for i in range(1 << n):
            idx = 1 + n + 2 * i
            mat.append(float(op[idx]) + float(op[idx + 1]) * 1j)
        gate = DiagonalGate(mat)
        targs = [int(op[i]) for i in range(1, n + 1)]
        circ.append(gate, targs)

    def _unitary_gate(n: int, circ: QuantumCircuit, op: list[str]):
        """Load an N-qubit UnitaryGate from the IR line tokens."""
        dim = 1 << n  # 2^n
        num_reals = dim * dim * 2
        start_idx = n + 1
        for i in range(start_idx, start_idx + num_reals):
            op[i] = float(op[i])

        # Build the dim x dim complex matrix
        matrix = []
        idx = start_idx
        for row in range(dim):
            row_data = []
            for col in range(dim):
                row_data.append(op[idx] + op[idx + 1] * 1j)
                idx += 2
            matrix.append(row_data)

        gate = UnitaryGate(matrix)
        # Qubits in the same order as the IR (fusion executable uses the same
        # LSB convention as Qiskit)
        qubit_indices = [int(op[i]) for i in range(1, n + 1)]
        circ.append(gate, qubit_indices)

    with open(circuit_path, "r") as f:
        for line in f.readlines():
            line = line.strip()
            if not line:
                continue
            op = line.split()
            gate_name = op[0]

            if gate_name.isdigit():
                continue  # skip lines starting with a number
            elif gate_name == "H":
                circ.h(int(op[1]))
            elif gate_name == "X":
                circ.x(int(op[1]))
            elif gate_name == "Y":
                circ.y(int(op[1]))
            elif gate_name == "Z":
                circ.z(int(op[1]))
            elif gate_name == "S":
                circ.s(int(op[1]))
            elif gate_name == "SDG":
                circ.sdg(int(op[1]))
            elif gate_name == "T":
                circ.t(int(op[1]))
            elif gate_name == "TDG":
                circ.tdg(int(op[1]))
            elif gate_name == "SX":
                circ.sx(int(op[1]))
            elif gate_name == "SXDG":
                circ.sxdg(int(op[1]))
            elif gate_name == "RX":
                circ.rx(float(op[2]), int(op[1]))
            elif gate_name == "RY":
                circ.ry(float(op[2]), int(op[1]))
            elif gate_name == "RZ":
                circ.rz(float(op[2]), int(op[1]))
            elif gate_name == "CX":
                circ.cx(int(op[1]), int(op[2]))
            elif gate_name == "CZ":
                circ.cz(int(op[1]), int(op[2]))
            elif gate_name == "CP":
                circ.cp(float(op[3]), int(op[1]), int(op[2]))
            elif gate_name == "SWAP":
                circ.swap(int(op[1]), int(op[2]))
            elif gate_name == "RZZ":
                circ.rzz(float(op[3]), int(op[1]), int(op[2]))
            elif gate_name in ("D1", "D2", "D3", "D4", "D5"):
                n = int(gate_name[1])
                _diag_gate(n, circ, op)
            elif gate_name in ("U1", "U2", "U3", "U4", "U5"):
                n = int(gate_name[1])
                _unitary_gate(n, circ, op)
            elif gate_name in ("SQS", "CSQS"):
                num_swap = int(op[1])
                targs = list(map(int, op[2:]))
                for a, b in zip(targs[:num_swap], targs[num_swap:]):
                    circ.swap(a, b)
                    qubit_mapping[a], qubit_mapping[b] = (
                        qubit_mapping[b],
                        qubit_mapping[a],
                    )
            else:
                raise ValueError(f"Unknown gate in IR: {line}")

    # Undo qubit mapping if needed
    for i, val in enumerate(qubit_mapping):
        if i != val:
            j = qubit_mapping.index(i)
            circ.swap(i, j)
            qubit_mapping[i], qubit_mapping[j] = (
                qubit_mapping[j],
                qubit_mapping[i],
            )
    return circ


# ---------------------------------------------------------------------------
# Test helpers
# ---------------------------------------------------------------------------
def run_fusion(
    ir_text: str,
    num_qubits: int,
    max_fusion_size: int = 5,
    method: int = 4,
) -> tuple[str, str]:
    """Write *ir_text* to a temp file, run the fusion executable and return
    (original_path, fused_path) pointing to the temp files."""
    tmpdir = tempfile.mkdtemp(prefix="qfusion_test_")
    orig_path = os.path.join(tmpdir, "original.txt")
    fused_path = os.path.join(tmpdir, "fused.txt")

    with open(orig_path, "w") as f:
        f.write(ir_text)

    env = os.environ.copy()
    env["DYNAMIC_COST_FILENAME"] = COST_FILE
    result = subprocess.run(
        [
            FUSION_EXE,
            orig_path,
            fused_path,
            str(max_fusion_size),
            str(num_qubits),
            str(method),
        ],
        env=env,
        capture_output=True,
        text=True,
    )
    assert result.returncode == 0, (
        f"Fusion executable failed (rc={result.returncode}):\n"
        f"stdout: {result.stdout}\nstderr: {result.stderr}"
    )
    return orig_path, fused_path


def assert_fusion_preserves_statevector(
    ir_text: str,
    num_qubits: int,
    max_fusion_size: int = 5,
    method: int = 4,
    atol: float = ATOL,
):
    """Core assertion: original and fused circuits produce the same state vector."""
    orig_path, fused_path = run_fusion(ir_text, num_qubits, max_fusion_size, method)

    circ_orig = circuit_load(orig_path, num_qubits)
    circ_fused = circuit_load(fused_path, num_qubits)

    sv_orig = Statevector.from_instruction(circ_orig)
    sv_fused = Statevector.from_instruction(circ_fused)

    assert sv_orig.equiv(sv_fused, atol=atol), (
        f"State vectors differ!\n"
        f"Original IR:\n{ir_text}\n"
        f"Fused circuit file: {fused_path}\n"
        f"SV original: {sv_orig}\n"
        f"SV fused:    {sv_fused}"
    )


def run_fusion_from_file(
    ir_file_path: str,
    num_qubits: int,
    max_fusion_size: int = 5,
    method: int = 4,
) -> tuple[str, str]:
    """Run the fusion executable on an existing IR file and return
    (original_path, fused_path)."""
    tmpdir = tempfile.mkdtemp(prefix="qfusion_test_")
    fused_path = os.path.join(tmpdir, "fused.txt")

    env = os.environ.copy()
    env["DYNAMIC_COST_FILENAME"] = COST_FILE
    result = subprocess.run(
        [
            FUSION_EXE,
            ir_file_path,
            fused_path,
            str(max_fusion_size),
            str(num_qubits),
            str(method),
        ],
        env=env,
        capture_output=True,
        text=True,
    )
    assert result.returncode == 0, (
        f"Fusion executable failed (rc={result.returncode}):\n"
        f"stdout: {result.stdout}\nstderr: {result.stderr}"
    )
    return ir_file_path, fused_path


def assert_fusion_file_preserves_statevector(
    ir_file_path: str,
    num_qubits: int,
    max_fusion_size: int = 5,
    method: int = 4,
    atol: float = ATOL,
):
    """Assert that fusion of an IR file preserves the state vector."""
    orig_path, fused_path = run_fusion_from_file(
        ir_file_path, num_qubits, max_fusion_size, method
    )

    circ_orig = circuit_load(orig_path, num_qubits)
    circ_fused = circuit_load(fused_path, num_qubits)

    sv_orig = Statevector.from_instruction(circ_orig)
    sv_fused = Statevector.from_instruction(circ_fused)

    assert sv_orig.equiv(sv_fused, atol=atol), (
        f"State vectors differ!\n"
        f"Original IR file: {orig_path}\n"
        f"Fused circuit file: {fused_path}\n"
        f"SV original: {sv_orig}\n"
        f"SV fused:    {sv_fused}"
    )


TESTS_DIR = os.path.dirname(os.path.abspath(__file__))


# ===================================================================
# 1. Single Gate Tests
# ===================================================================
class TestSingleGate:
    """Verify each individual gate type passes through fusion correctly."""

    def test_h_gate(self):
        assert_fusion_preserves_statevector("H 0", num_qubits=1)

    def test_x_gate(self):
        assert_fusion_preserves_statevector("X 0", num_qubits=1)

    def test_y_gate(self):
        assert_fusion_preserves_statevector("Y 0", num_qubits=1)

    def test_z_gate(self):
        assert_fusion_preserves_statevector("Z 0", num_qubits=1)

    def test_s_gate(self):
        assert_fusion_preserves_statevector("S 0", num_qubits=1)

    def test_sdg_gate(self):
        assert_fusion_preserves_statevector("SDG 0", num_qubits=1)

    def test_t_gate(self):
        assert_fusion_preserves_statevector("T 0", num_qubits=1)

    def test_tdg_gate(self):
        assert_fusion_preserves_statevector("TDG 0", num_qubits=1)

    def test_sx_gate(self):
        assert_fusion_preserves_statevector("SX 0", num_qubits=1)

    def test_sxdg_gate(self):
        assert_fusion_preserves_statevector("SXDG 0", num_qubits=1)

    def test_rx_gate(self):
        assert_fusion_preserves_statevector("RX 0 1.2", num_qubits=1)

    def test_ry_gate(self):
        assert_fusion_preserves_statevector("RY 0 0.7", num_qubits=1)

    def test_rz_gate(self):
        assert_fusion_preserves_statevector("RZ 0 2.1", num_qubits=1)

    def test_rx_negative_angle(self):
        assert_fusion_preserves_statevector("RX 0 -0.5", num_qubits=1)

    def test_ry_pi(self):
        assert_fusion_preserves_statevector(
            f"RY 0 {math.pi}", num_qubits=1
        )

    def test_rz_zero(self):
        assert_fusion_preserves_statevector("RZ 0 0.0", num_qubits=1)

    def test_cx_gate(self):
        assert_fusion_preserves_statevector("CX 0 1", num_qubits=2)

    def test_cz_gate(self):
        assert_fusion_preserves_statevector("CZ 0 1", num_qubits=2)

    def test_cp_gate(self):
        assert_fusion_preserves_statevector("CP 0 1 0.8", num_qubits=2)

    def test_swap_gate(self):
        assert_fusion_preserves_statevector("SWAP 0 1", num_qubits=2)

    def test_rzz_gate(self):
        assert_fusion_preserves_statevector("RZZ 0 1 1.5", num_qubits=2)

    def test_h_on_higher_qubit(self):
        assert_fusion_preserves_statevector("H 1", num_qubits=2)

    def test_cx_reversed(self):
        assert_fusion_preserves_statevector("CX 1 0", num_qubits=2)

    def test_cp_pi(self):
        assert_fusion_preserves_statevector(
            f"CP 0 1 {math.pi}", num_qubits=2
        )


# ===================================================================
# 2. 1-Qubit Multi-Gate Tests
# ===================================================================
class TestOneQubitMultiGate:
    """Sequences of gates on a single qubit that fuse into U1."""

    def test_h_x(self):
        ir = "H 0\nX 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_h_z_h(self):
        ir = "H 0\nZ 0\nH 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_rx_ry_rz(self):
        ir = "RX 0 0.3\nRY 0 0.5\nRZ 0 0.7"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_t_h_t_h(self):
        ir = "T 0\nH 0\nT 0\nH 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_s_t_combination(self):
        ir = "S 0\nT 0\nS 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_h_s_h(self):
        """H S H should equal RX(pi/2) up to global phase."""
        ir = "H 0\nS 0\nH 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_multiple_rz(self):
        """Multiple diagonal gates should fuse to D1."""
        ir = "RZ 0 0.5\nRZ 0 0.3\nRZ 0 0.7"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_sdg_sdg_equals_z(self):
        ir = "SDG 0\nSDG 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_h_rz_h(self):
        ir = "H 0\nRZ 0 1.0\nH 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_sx_sx(self):
        """SX * SX = X."""
        ir = "SX 0\nSX 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_sxdg_sx(self):
        """SX^dag * SX = I."""
        ir = "SXDG 0\nSX 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_long_single_qubit_chain(self):
        """Long sequence of single-qubit gates."""
        ir = "\n".join([
            "H 0", "RX 0 0.3", "RY 0 0.5", "RZ 0 0.7",
            "T 0", "S 0", "H 0", "X 0",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_identity_sequence(self):
        """X X = I."""
        ir = "X 0\nX 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_z_s_t(self):
        ir = "Z 0\nS 0\nT 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_tdg_tdg_equals_sdg(self):
        ir = "TDG 0\nTDG 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_parallel_single_qubit_gates(self):
        """Independent single-qubit gates on different qubits."""
        ir = "H 0\nX 1\nRZ 0 0.5\nRY 1 0.3"
        assert_fusion_preserves_statevector(ir, num_qubits=2)


# ===================================================================
# 3. Multiple Gates fuse to 2-Qubit Unitary Gate(s)
# ===================================================================
class TestTwoQubitFusion:
    """Gates on 2 qubits that fuse into U2 gate(s)."""

    def test_h_h_cx(self):
        ir = "H 0\nH 1\nCX 0 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_h_cx_rz(self):
        ir = "H 0\nCX 0 1\nRZ 0 0.5"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_h_h_cx_rz_rz(self):
        ir = "H 0\nH 1\nCX 0 1\nRZ 0 0.5\nRZ 1 0.3"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_cx_cx(self):
        """Two CNOT gates (same qubits) = Identity on computational basis."""
        ir = "CX 0 1\nCX 0 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_cx_reversed_cx(self):
        ir = "CX 0 1\nCX 1 0"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_bell_state(self):
        """H on q0 then CNOT creates Bell state."""
        ir = "H 0\nCX 0 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_swap_decomposition(self):
        """SWAP = CX CX CX pattern."""
        ir = "CX 0 1\nCX 1 0\nCX 0 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_h_cz_h(self):
        ir = "H 0\nH 1\nCZ 0 1\nH 0\nH 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_cp_with_rotations(self):
        ir = "RX 0 0.5\nRY 1 0.3\nCP 0 1 1.2\nRZ 0 0.7"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_rzz_with_hadamards(self):
        ir = "H 0\nH 1\nRZZ 0 1 0.8\nH 0\nH 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_cx_swap_pattern_with_rotations(self):
        """SWAP decomposed as CX CX CX, with rotations."""
        ir = "H 0\nCX 0 1\nCX 1 0\nCX 0 1\nRZ 1 0.5"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_multiple_cx_with_rotations(self):
        ir = "H 0\nCX 0 1\nRZ 0 0.3\nCX 0 1\nH 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_cz_with_single_qubit_gates(self):
        ir = "H 0\nT 1\nCZ 0 1\nS 0\nH 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_two_qubit_diagonal_fusion(self):
        """CZ and RZZ are diagonal gates, should fuse to D2."""
        ir = "CZ 0 1\nRZZ 0 1 0.5"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_cp_cp_accumulation(self):
        ir = "CP 0 1 0.3\nCP 0 1 0.7"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_dense_two_qubit_circuit(self):
        ir = "\n".join([
            "H 0", "H 1",
            "CX 0 1", "RZ 0 0.3", "RZ 1 0.5",
            "CX 1 0", "H 0", "T 1",
            "CZ 0 1", "S 0", "SDG 1",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_non_adjacent_qubits(self):
        """2-qubit gate on non-adjacent qubits in a larger circuit."""
        ir = "H 0\nH 2\nCX 0 2\nRZ 0 0.5"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_multiple_independent_2q_fusions(self):
        """Two independent 2-qubit clusters on 4 qubits."""
        ir = "\n".join([
            "H 0", "CX 0 1", "RZ 0 0.5",
            "H 2", "CX 2 3", "RZ 2 0.3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)


# ===================================================================
# 4. Multiple Gates fuse to 3-Qubit Unitary Gate(s)
# ===================================================================
class TestThreeQubitFusion:
    """Gates on 3 qubits that fuse into U3 gate(s)."""

    def test_h_cx_cx_chain(self):
        """H then chain of CX gates across 3 qubits."""
        ir = "H 0\nCX 0 1\nCX 1 2"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_ghz_state_3q(self):
        """GHZ state: H + CX chain."""
        ir = "H 0\nCX 0 1\nCX 0 2"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_h_cx_rz_cx(self):
        ir = "H 0\nCX 0 1\nRZ 1 0.5\nCX 1 2"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_rotations_cx(self):
        ir = "\n".join([
            "RX 0 0.3", "RY 1 0.5", "RZ 2 0.7",
            "CX 0 1", "CX 1 2",
            "H 0", "H 1", "H 2",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_cz_chain(self):
        ir = "H 0\nH 1\nH 2\nCZ 0 1\nCZ 1 2"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_toffoli_like_pattern(self):
        """A pattern that mimics Toffoli gate."""
        ir = "\n".join([
            "H 2",
            "CX 1 2", "TDG 2", "CX 0 2", "T 2",
            "CX 1 2", "TDG 2", "CX 0 2",
            "T 1", "T 2", "H 2",
            "CX 0 1", "T 0", "TDG 1",
            "CX 0 1",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_swap_decomposed_and_cx(self):
        """SWAP decomposed as CX chain, then CX on remaining qubit."""
        ir = "H 0\nCX 0 1\nCX 1 0\nCX 0 1\nCX 1 2\nH 2"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_rzz_chain(self):
        ir = "H 0\nH 1\nH 2\nRZZ 0 1 0.5\nRZZ 1 2 0.3"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_diagonal_only(self):
        """Only diagonal gates across 3 qubits."""
        ir = "RZ 0 0.3\nRZ 1 0.5\nCZ 0 1\nRZ 2 0.7\nCZ 1 2"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_dense(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2",
            "CX 0 1", "CX 1 2",
            "RZ 0 0.3", "RY 1 0.5", "RX 2 0.7",
            "CZ 0 2",
            "T 0", "S 1", "SDG 2",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_cp_chain(self):
        ir = "H 0\nH 1\nH 2\nCP 0 1 0.5\nCP 1 2 0.7\nCP 0 2 1.1"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_three_qubit_interleaved_single_two(self):
        ir = "\n".join([
            "H 0", "CX 0 1", "T 0",
            "H 2", "CX 1 2", "S 1",
            "CX 0 2", "H 0",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_multiple_3q_fusions(self):
        """Two independent 3-qubit clusters on 6 qubits."""
        ir = "\n".join([
            "H 0", "CX 0 1", "CX 1 2", "RZ 0 0.5",
            "H 3", "CX 3 4", "CX 4 5", "RZ 3 0.3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=6)

    def test_3q_mixed_fusions(self):
        """Mix of 2-qubit and 3-qubit fusions."""
        ir = "\n".join([
            "H 0", "H 1", "CX 0 1",          # 2q cluster
            "H 2", "CX 1 2", "RZ 2 0.5",     # extends to 3q
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=3)


# ===================================================================
# 5. Multiple Gates fuse to 4-Qubit Unitary Gate(s)
# ===================================================================
class TestFourQubitFusion:
    """Gates on 4 qubits that fuse into U4 gate(s)."""

    def test_four_qubit_cx_chain(self):
        ir = "H 0\nCX 0 1\nCX 1 2\nCX 2 3"
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_ghz_state_4q(self):
        ir = "H 0\nCX 0 1\nCX 0 2\nCX 0 3"
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_rotations_cx(self):
        ir = "\n".join([
            "RX 0 0.3", "RY 1 0.5", "RZ 2 0.7", "H 3",
            "CX 0 1", "CX 1 2", "CX 2 3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_interleaved(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3",
            "CX 0 1", "CX 2 3",
            "CX 1 2",
            "RZ 0 0.3", "RZ 3 0.7",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_cz_ring(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3",
            "CZ 0 1", "CZ 1 2", "CZ 2 3", "CZ 0 3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_dense_rotations(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3",
            "CX 0 1", "RZ 0 0.3", "RY 1 0.5",
            "CX 2 3", "RZ 2 0.7", "RX 3 0.9",
            "CX 1 2",
            "T 0", "S 1", "SDG 2", "TDG 3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_cx_swap_chain(self):
        """Chain of SWAP gates decomposed into CX."""
        ir = "\n".join([
            "H 0",
            "CX 0 1", "CX 1 0", "CX 0 1",
            "CX 1 2", "CX 2 1", "CX 1 2",
            "CX 2 3", "CX 3 2", "CX 2 3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_cp_all_pairs(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3",
            "CP 0 1 0.3", "CP 0 2 0.5", "CP 0 3 0.7",
            "CP 1 2 0.9", "CP 1 3 1.1", "CP 2 3 1.3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_rzz_chain(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3",
            "RZZ 0 1 0.5", "RZZ 1 2 0.3", "RZZ 2 3 0.7",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_four_qubit_mixed_two_body(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3",
            "CX 0 1", "CZ 1 2", "CP 2 3 0.8",
            "RZZ 0 3 0.5",
            "H 0", "H 1", "H 2", "H 3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_qft_like_4q(self):
        """QFT-like pattern on 4 qubits."""
        ir = "\n".join([
            "H 0",
            "CP 0 1 1.5707963267948966",
            "CP 0 2 0.7853981633974483",
            "CP 0 3 0.39269908169872414",
            "H 1",
            "CP 1 2 1.5707963267948966",
            "CP 1 3 0.7853981633974483",
            "H 2",
            "CP 2 3 1.5707963267948966",
            "H 3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_multiple_4q_fusions(self):
        """Two independent 4-qubit clusters on 8 qubits."""
        ir = "\n".join([
            "H 0", "CX 0 1", "CX 1 2", "CX 2 3",
            "H 4", "CX 4 5", "CX 5 6", "CX 6 7",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=8)

    def test_4q_with_partial_fusions(self):
        """Mix of different fusion sizes within a 4-qubit circuit."""
        ir = "\n".join([
            "H 0", "T 0",                     # 1q
            "H 1", "CX 0 1",                  # 2q
            "H 2", "CX 1 2", "RZ 2 0.5",     # 3q
            "H 3", "CX 2 3",                  # 4q
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)


# ===================================================================
# 6. Multiple Gates fuse to 5-Qubit Unitary Gate(s)
# ===================================================================
class TestFiveQubitFusion:
    """Gates on 5 qubits that fuse into U5 gate(s)."""

    def test_five_qubit_cx_chain(self):
        ir = "H 0\nCX 0 1\nCX 1 2\nCX 2 3\nCX 3 4"
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_ghz_state_5q(self):
        ir = "H 0\nCX 0 1\nCX 0 2\nCX 0 3\nCX 0 4"
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_all_hadamards_cx_chain(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "CX 0 1", "CX 1 2", "CX 2 3", "CX 3 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_rotations_cx(self):
        ir = "\n".join([
            "RX 0 0.3", "RY 1 0.5", "RZ 2 0.7", "H 3", "T 4",
            "CX 0 1", "CX 1 2", "CX 2 3", "CX 3 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_cz_chain(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "CZ 0 1", "CZ 1 2", "CZ 2 3", "CZ 3 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_interleaved_gates(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "CX 0 1", "CX 2 3",
            "CX 1 2", "CX 3 4",
            "CX 0 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_dense(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "CX 0 1", "RZ 0 0.3",
            "CX 1 2", "RY 1 0.5",
            "CX 2 3", "RX 2 0.7",
            "CX 3 4", "T 3",
            "CZ 0 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_cp_chain(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "CP 0 1 0.3", "CP 1 2 0.5", "CP 2 3 0.7", "CP 3 4 0.9",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_rzz_chain(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "RZZ 0 1 0.5", "RZZ 1 2 0.3", "RZZ 2 3 0.7", "RZZ 3 4 0.9",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_mixed_gates(self):
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "CX 0 1", "CZ 1 2", "CP 2 3 0.8", "RZZ 3 4 0.5",
            "CX 0 4", "CX 4 0", "CX 0 4",
            "H 0", "H 1", "H 2", "H 3", "H 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_qft_like_5q(self):
        """QFT-like pattern on 5 qubits."""
        ir = "\n".join([
            "H 0",
            f"CP 0 1 {math.pi / 2}",
            f"CP 0 2 {math.pi / 4}",
            f"CP 0 3 {math.pi / 8}",
            f"CP 0 4 {math.pi / 16}",
            "H 1",
            f"CP 1 2 {math.pi / 2}",
            f"CP 1 3 {math.pi / 4}",
            f"CP 1 4 {math.pi / 8}",
            "H 2",
            f"CP 2 3 {math.pi / 2}",
            f"CP 2 4 {math.pi / 4}",
            "H 3",
            f"CP 3 4 {math.pi / 2}",
            "H 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_five_qubit_with_partial_fusions(self):
        """Circuit with mixed fusion sizes up to 5 qubits."""
        ir = "\n".join([
            "H 0", "T 0",                      # 1q
            "H 1", "CX 0 1",                   # 2q
            "H 2", "CX 1 2",                   # 3q
            "H 3", "CX 2 3", "RZ 3 0.5",      # 4q
            "H 4", "CX 3 4",                   # 5q
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_grover_like_5q(self):
        """Grover-like oracle + diffusion on 5 qubits."""
        ir = "\n".join([
            # Initial superposition
            "H 0", "H 1", "H 2", "H 3", "H 4",
            # Oracle (phase flip on |11111⟩ using CZ chain)
            "CZ 0 1", "CZ 2 3",
            "CX 1 2", "CZ 0 2",
            "CX 1 2",
            "CZ 3 4",
            # Diffusion
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "X 0", "X 1", "X 2", "X 3", "X 4",
            "CZ 0 1",
            "X 0", "X 1", "X 2", "X 3", "X 4",
            "H 0", "H 1", "H 2", "H 3", "H 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_multiple_5q_fusions(self):
        """Two independent 5-qubit clusters on 10 qubits."""
        ir = "\n".join([
            "H 0", "CX 0 1", "CX 1 2", "CX 2 3", "CX 3 4",
            "H 5", "CX 5 6", "CX 6 7", "CX 7 8", "CX 8 9",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=10)


# ===================================================================
# Parametrized tests for systematic gate coverage
# ===================================================================
class TestParametrizedGates:
    """Parametrized rotation tests with various angles."""

    @pytest.mark.parametrize(
        "angle",
        [0.0, math.pi / 4, math.pi / 2, math.pi, 3 * math.pi / 2, -0.5, 2.0],
    )
    def test_rx_angles(self, angle):
        assert_fusion_preserves_statevector(f"RX 0 {angle}", num_qubits=1)

    @pytest.mark.parametrize(
        "angle",
        [0.0, math.pi / 4, math.pi / 2, math.pi, 3 * math.pi / 2, -0.5, 2.0],
    )
    def test_ry_angles(self, angle):
        assert_fusion_preserves_statevector(f"RY 0 {angle}", num_qubits=1)

    @pytest.mark.parametrize(
        "angle",
        [0.0, math.pi / 4, math.pi / 2, math.pi, 3 * math.pi / 2, -0.5, 2.0],
    )
    def test_rz_angles(self, angle):
        assert_fusion_preserves_statevector(f"RZ 0 {angle}", num_qubits=1)

    @pytest.mark.parametrize(
        "angle",
        [0.0, math.pi / 4, math.pi / 2, math.pi, -0.3, 2.5],
    )
    def test_cp_angles(self, angle):
        assert_fusion_preserves_statevector(f"CP 0 1 {angle}", num_qubits=2)

    @pytest.mark.parametrize(
        "angle",
        [0.0, math.pi / 4, math.pi / 2, math.pi, -0.3, 2.5],
    )
    def test_rzz_angles(self, angle):
        assert_fusion_preserves_statevector(f"RZZ 0 1 {angle}", num_qubits=2)

    @pytest.mark.parametrize(
        "angle",
        [0.1, 0.5, 1.0, math.pi / 3, math.pi],
    )
    def test_rx_ry_fusion_angles(self, angle):
        ir = f"RX 0 {angle}\nRY 0 {angle}"
        assert_fusion_preserves_statevector(ir, num_qubits=1)


# ===================================================================
# Edge case and stress tests
# ===================================================================
class TestEdgeCases:
    """Edge cases and stress tests."""

    def test_all_identity_like(self):
        """Circuit that reduces to identity."""
        ir = "H 0\nH 0\nX 0\nX 0"
        assert_fusion_preserves_statevector(ir, num_qubits=1)

    def test_single_qubit_on_many_qubits(self):
        """Single-qubit gate in a multi-qubit system."""
        ir = "H 3"
        assert_fusion_preserves_statevector(ir, num_qubits=5)

    def test_only_cx_gates(self):
        """Circuit with only CX gates."""
        ir = "CX 0 1\nCX 1 2\nCX 2 0"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_all_diagonal_2q(self):
        """Only diagonal 2-qubit gates."""
        ir = "CZ 0 1\nCP 0 1 0.5\nRZZ 0 1 0.3"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_all_diagonal_3q(self):
        """Only diagonal gates across 3 qubits."""
        ir = "RZ 0 0.3\nRZ 1 0.5\nRZ 2 0.7\nCZ 0 1\nCZ 1 2\nCP 0 2 0.4"
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_long_chain_2q(self):
        """Long sequence of 2-qubit gates."""
        gates = []
        for i in range(10):
            gates.append(f"CX 0 1")
            gates.append(f"RZ 0 {0.1 * (i + 1)}")
        ir = "\n".join(gates)
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_wide_circuit_no_entanglement(self):
        """Many qubits, each with independent single-qubit gates."""
        gates = []
        n = 5
        for q in range(n):
            gates.append(f"H {q}")
            gates.append(f"RZ {q} {0.3 * (q + 1)}")
        ir = "\n".join(gates)
        assert_fusion_preserves_statevector(ir, num_qubits=n)

    def test_alternating_cx_direction(self):
        """CX gates alternating direction."""
        ir = "\n".join([
            "H 0", "H 1",
            "CX 0 1", "CX 1 0", "CX 0 1", "CX 1 0",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_swap_identity(self):
        """Two SWAP-decompositions = identity."""
        ir = "CX 0 1\nCX 1 0\nCX 0 1\nCX 0 1\nCX 1 0\nCX 0 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_cx_with_x_preparation(self):
        """X gate prepares |1⟩, then CX should flip target."""
        ir = "X 0\nCX 0 1"
        assert_fusion_preserves_statevector(ir, num_qubits=2)

    def test_deep_3q_circuit(self):
        """Deeper 3-qubit circuit."""
        ir = "\n".join([
            "H 0", "H 1", "H 2",
            "CX 0 1", "CX 1 2",
            "RZ 0 0.5", "RY 1 0.3", "RX 2 0.7",
            "CX 2 0", "CZ 0 1",
            "T 0", "T 1", "T 2",
            "CX 0 1", "CX 1 2",
            "H 0", "H 1", "H 2",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=3)

    def test_deep_4q_circuit(self):
        """Deeper 4-qubit circuit."""
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3",
            "CX 0 1", "CX 2 3",
            "CZ 1 2",
            "RZ 0 0.3", "RY 1 0.5", "RX 2 0.7", "T 3",
            "CX 1 2", "CX 0 3",
            "S 0", "SDG 1", "T 2", "TDG 3",
            "CX 0 1", "CX 2 3",
            "H 0", "H 1", "H 2", "H 3",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=4)

    def test_deep_5q_circuit(self):
        """Deeper 5-qubit circuit."""
        ir = "\n".join([
            "H 0", "H 1", "H 2", "H 3", "H 4",
            "CX 0 1", "CX 2 3",
            "CX 1 2", "CX 3 4",
            "RZ 0 0.3", "RY 1 0.5", "RX 2 0.7", "T 3", "S 4",
            "CZ 0 4", "CX 1 3",
            "CP 0 2 0.8", "RZZ 2 4 0.6",
            "H 0", "H 1", "H 2", "H 3", "H 4",
        ])
        assert_fusion_preserves_statevector(ir, num_qubits=5)


# ===================================================================
# Large case tests (from tests/large_case.txt)
# ===================================================================
class TestLargeCase:
    """Test fusion correctness on the large 21-qubit circuit from large_case.txt."""

    LARGE_CASE_FILE = os.path.join(TESTS_DIR, "large_case.txt")

    def test_large_case_21q(self):
        """Full 21-qubit large_case.txt circuit preserves state vector after fusion."""
        assert_fusion_file_preserves_statevector(
            self.LARGE_CASE_FILE, num_qubits=21
        )
