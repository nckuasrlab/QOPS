# QiskitFusion

## Files

+ `fusion.hpp`: The original version of Qiskit Aer simulator, used for recovery.
+ `static_qiskit.hpp`: The patched version for static fusion (the original version) but with the functionality to write the fused circuit into a given filename (`FUSED_CIRCUIT_FILENAME`). The maximum number of qubits for fusion can be set by `FUSION_MAX_QUBIT`.
+ `dynamic_qiskit.hpp`: The patched version for dynamic fusion, which reads runtime costs from a given path (`DYNAMIC_COST_FILENAME`). It also outputs the circuit to `FUSED_CIRCUIT_FILENAME`. The maximum number of qubits for fusion can be set by `FUSION_MAX_QUBIT`.
    + Note: `DYNAMIC_COST_FILENAME` may need to be modified for different simulators.
+ `Makefile`: used to build different binaries from different fusion strategies.
+ `qiskit_fusion.py`: A wrapper for the qiskit fusion binary. Use following options to config mentioned variables in .hpp files.
    + `--fusion_max_qubit`: `FUSION_MAX_QUBIT`
    + `--dynamic_cost_filename`: `DYNAMIC_COST_FILENAME`
    + `--output_filename`: `FUSED_CIRCUIT_FILENAME`

## Show the patches

```bash
$ git diff --no-index fusion.hpp static_qiskit.hpp
$ git diff --no-index fusion.hpp dynamic_qiskit.hpp
```

## Build

```bash
sudo apt install nlohmann-json3-dev libspdlog-dev nlohmann-json3-dev
cd /path/to/QiskitFusion/
make
```

## Usage

```bash
$ python qiskit_fusion.py -h
usage: python qiskit_fusion.py [-h] [-m NUM] [-d PATH] -o PATH circuit_filename num_qubits {dynamic_qiskit,static_qiskit}

Qiskit Fusion Script

positional arguments:
  circuit_filename      Path to the circuit file
  num_qubits            Number of qubits
  {dynamic_qiskit,static_qiskit}
                        Fusion mode

options:
  -h, --help            show this help message and exit
  -m NUM, --fusion_max_qubit NUM
                        Maximum number of qubits for fusion (default: 3)
  -d PATH, --dynamic_cost_filename PATH
                        Dynamic cost filename (default: ../log/gate_exe_time.csv)
  -o PATH, --output_filename PATH
                        Output filename for the fused circuit (default: None)
```

+ static

```bash
python qiskit_fusion.py ../circuit/sc32.txt 32 static_qiskit -o fused_sc32.txt
```

+ dynamic

```bash
python qiskit_fusion.py ../circuit/sc32.txt 32 dynamic_qiskit -o fused_sc32.txt
```