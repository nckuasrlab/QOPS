# qfusion-opt

## Prerequisite

### 1. Create all the directory we need.

```bash
$ mkdir -p fusionCircuit model qiskitFusionCircuit subCircuit txt
```

### 2. Install requirements with conda

```bash
$ conda config --set channel_priority disabled
$ conda env create --name qfusion-opt --file requirements.yml
$ conda activate qfusion-opt
```

+ Note: the `requirements.yml` is prepared by `conda env export | head -n -1 > requirements.yml`, which ignores the last line for the prefix path.

### 3. Compile the target program

``` bash
$ g++ -O3 -o fusion fusion.cpp
```

**NOTE:** You should have `./cpu` and `./finder` in this directory to verify the result of `fusion`.

## Usage

### 1. Microbenchmark Suite

Check `python/microbenchmark_suite/README.md`.

### 2. Fusion algorithm

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

### 3. Fused circuit transform (optional)

If you're going to run on `Quokka` simulator, you should transform the circuit by `./finder`.

```bash
$ g++ -o finder/finder finder/finder.cpp
$ finder/finder [input_circuit] [output_circuit] [chunk_size]
```

### 4. Run on simulator
Executing circuit on `Quokka` needs to setting `ini` file(`sub_cpu.ini`).

```bash
$ ./cpu/Quokka -i sub_cpu.ini -c [circuit_file]
```

**NOTE:** Executing circuit on qiskit needs to transform the circuit format. Please reference to `exe_circuit` function in `python/exe_fusion_qiskit.py`

## Example

```bash
$ python python/microbenchmark_suite/gen_cost_table/quokka.py gen_table 32 18
$ ./fusion ./circuit/sc24.txt ./fusionCircuit/sc24.txt 3 24 3
$ finder/finder ./fusionCircuit/sc24.txt out.txt 18
$ cpu/Quokka -i sub_cpu.ini -c out.txt
```

## Test on Aer simulator with fusion methods

```bash
python python/exe_fusion_aer.py > out.txt
```