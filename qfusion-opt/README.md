# qfusion-opt

## Prerequisite

### 1. Create all the directory we need.

```bash
$ mkdir -p fusionCircuit model qiskitFusionCircuit subCircuit txt
```

### 2. Install requirements with uv

```bash
$ uv venv -p 3.12.10
$ source .venv/bin/activate
$ uv pip install -r requirements.txt
```

### 3. Compile the target program

``` bash
$ cd qfusion-opt
$ make
```

**NOTE:** You should have `./cpu` and `./finder` in this directory to verify the result of `fusion`.

## Usage

### 1. Microbenchmark Suite

Check `python/microbenchmark_suite/README.md`.

### 2. Fusion algorithm

If mode=4,6,7,8, `fusion` needs to read `./log/gate_exe_time.csv` by setting `DYNAMIC_COST_FILENAME`.

```bash
$ DYNAMIC_COST_FILENAME=[csv_file] ./fusion [input_file] [output_file] [max_fusion_qubit] [total_qubit] [mode]
```

Fusion mode setting:

+ 0: origin
+ 1: cutsubtree
+ 2: smallCircuit
+ 3: all opt (cutsubtree + smallCircuit)
+ 4: all opt with dynamic cost function
+ 5: same as mode3，but with diagonal fusion
+ 6: same as mode4，but with diagonal fusion
+ 7: same as mode4 but for qiskit execution version
+ 8: same as mode7，but with diagonal fusion

### 3. Run on simulator

```bash
$ python python/microbenchmark_suite/gen_cost_table/quokka.py gen_table 32 17
$ DYNAMIC_COST_FILENAME=./log/gate_exe_time_aer.csv ./fusion ./circuit/sc32.txt ./xxx.txt 5 32 8 >fusion_dump.txt
$ PATH_TO/finder ./xxx.txt 17 32 32 1 0 5 1 > xxx_finder.txt
$ PATH_TO/Queen -i ../circuit/sub0/32/cpu.ini -c xxx_finder.txt
```

## Test on Aer and Queen simulators with fusion methods

```bash
$ date +"%Y%m%d_%H%M%S" >> out.txt; python python/exe_fusion_aer.py >> out.txt; date +"%Y%m%d_%H%M%S" >> out.txt; python python/exe_fusion_queen.py >> out.txt; date +"%Y%m%d_%H%M%S" >> out.txt
```

# Misc

## Visualization of fusion

```bash
CIRC=test F=2 M=8 T=5 sh -c 'DYNAMIC_COST_FILENAME=./log/gate_exe_time_aer.csv ./fusion ./circuit/${CIRC}.txt ./xxx.txt ${F} ${T} ${M} >fusion_dump.txt && python python/circuit_drawer.py circuit/${CIRC}.txt -q ${T} && python python/circuit_drawer.py xxx.txt -q ${T}'
```

## USE_SHORTEST_PATH_ONLY: gMethod

+ static for queen: 3
+ dynamic for queen: 4,6
+ static for aer: 1,2,5
+ dynamic for aer: 7,8