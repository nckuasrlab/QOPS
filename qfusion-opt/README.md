# qfusion-opt

## Prerequisite

1. Create all the directory we need.

```bash
$ mkdir -p fusionCircuit model qiskitFusionCircuit subCircuit txt
```

2. Install requirements in python virtual environment.(conda)

```bash
$ conda config --set channel_priority disabled
$ conda env create --name qfusion-opt --file requirements.yml
$ conda activate qfusion-opt
```

3. Compile the target program

``` bash
$ g++ -O3 -o fusion fusion.cpp
```

**NOTE:** You should have `./cpu` and `./finder` in this directory to verify the result of `fusion`.

## Usage

1. Initalization of `.env` setting.

```bash
$ python3 python/env_init.py
```

The `.env` setting can be adjusted in `python/env_init.py`

```python
env_content = """\
# microbenchmark
TIMES=100
TOTAL_QUBIT=24
CHUNK_SIZE=18
CHUNK_START=10
TEST_SIZE=9
RUNNER_TYPE=MEM
# other setting
RUN_MICROBENCHMARK=0
RE_TRAIN=1
MODE=1
    """
```

2. Feature selection

If `RUN_MICROBENCHMARK=1` in `.env` file, generate new test file and obtain the execution result for feature selection.
If `RUN_MICROBENCHMARK=0`, analyze according to previous execution result(`./log/feature_one_qubit.csv`, `./log/feature_two_qubit.csv`).

```bash
$ python3 python/find_feature.py
```

3. Generate performance model

If `RUN_MICROBENCHMARK=1`, generate microbenchmark(`./log/microbenchmark_result.csv`).

```bash
# If `MODE=0` in `.env` file.
$ python3 python/performance_model_quokka.py [input_gate_type] [total_qubit] [chunk_size]
# If `MODE=1` in `.env` file.
$ python3 python/performance_model_quokka.py [total_qubit] [chunk_size]
```

4. Fusion algorithm

If `mode=4,6,7,8`(different with `.env` setting), `fusion` needs to read `./log/gate_exe_time.csv`.

```bash
$ ./fusion [input_file] [output_file] [max_fusion_qubit] [total_qubit] [mode]
```

Fusion mode setting

```text
mode 

0: origin
1: cutsubtree
2: smallCircuit
3: all opt (cutsubtree + smallCircuit)
4: all opt with dynamic cost function
5: same as mode3，but with diagonal fusion
6: same as mode4，but with diagonal fusion
7: same as mode4 but for qiskit execution version
8: same as mode7，but with diagonal fusion
```

1. Fused circuit transform (optional)

If you're going to run on `Quokka` simulator, you should transform the circuit by `./finder`.

```bash
$ g++ -o finder/finder finder/finder.cpp
$ finder/finder [input_circuit] [output_circuit] [chunk_size]
```

1. Run on simulator
Executing circuit on `Quokka` needs to setting `ini` file(`sub_cpu.ini`).

```bash
$ ./cpu/Quokka -i sub_cpu.ini -c [circuit_file]
```

**NOTE:** Executing circuit on qiskit needs to transform the circuit format. Please reference to `exe_circuit` function in `python/exe_fusion_qiskit.py`

### Example

```bash
$ python3 python/env_init.py
$ python3 python/performance_model_quokka.py 32 18
$ ./fusion ./circuit/sc24.txt ./fusionCircuit/sc24.txt 3 24 3
$ finder/finder ./fusionCircuit/sc24.txt out.txt 18
$ cpu/Quokka -i sub_cpu.ini -c out.txt
```
