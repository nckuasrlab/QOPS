# qfusion-opt

qfusion-opt is a novel gate fusion workflow and algorithm that leverages profile-informed techniques.

## Prerequisite

### 1. Create all the directory we need.

```bash
$ cd qfusion-opt
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
$ make
g++-10 -std=c++2a -O3 -march=native -flto=auto -funroll-loops -fno-rtti -fno-exceptions -pipe -Wall -Wextra -Wpedantic -o fusion fusion.cpp -lpthread
```

**NOTE:** You should have `./cpu` and `./finder` in this directory to verify the result of `fusion`.

## Usage

### 1. Microbenchmark Suite

Check `python/microbenchmark_suite/README.md`.

### 2. Test on Aer and Queen simulators with fusion methods

```bash
$ ./exe.sh "test on aer" "python python/exe_fusion_aer.py"
$ ./exe.sh "test on queen" "python python/exe_fusion_queen.py"
```

## Manual run (Optional)

### Fusion algorithm

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

### Run fusion and simulate the fused circuit

```bash
$ DYNAMIC_COST_FILENAME=./log/gate_exe_time_queen.csv ./fusion ./circuit/sc32.txt ./xxx.txt 5 32 4 >fusion_dump.txt
$ cat <<EOF > cpu.ini
[system]
total_qbit=32
device_qbit=0
chunk_qbit=17
buffer_qbit=26
threads_bit=6
EOF
$ PATH_TO/finder ./xxx.txt 17 32 32 1 0 5 1 > xxx_finder.txt
$ PATH_TO/Queen -i ./cpu.ini -c xxx_finder.txt
```

## Misc

### Visualization of fusion

```bash
CIRC=test F=2 M=8 T=5 sh -c 'DYNAMIC_COST_FILENAME=./log/gate_exe_time_aer.csv ./fusion ./circuit/${CIRC}.txt ./xxx.txt ${F} ${T} ${M} >fusion_dump.txt && python python/circuit_drawer.py circuit/${CIRC}.txt -q ${T} && python python/circuit_drawer.py xxx.txt -q ${T}'
```

### if `USE_SHORTEST_PATH_ONLY=1`: gMethod

+ static for queen: 3
+ dynamic for queen: 4,6
+ static for aer: 1,2,5
+ dynamic for aer: 7,8

### Download microbenchmark results from the machine mentioned in paper (not recommanded)

```bash
$ wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1d5rNiB2oge7w1Q6mzmrQRn3YAdt8bZPY' -O ./qfusion-opt/log/microbenchmark_result_queen.csv 2>/dev/null
$ wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1UKZ0ipUKnzI-flgqqJJ9MzHdAM85kUZa' -O ./qfusion-opt/log/microbenchmark_result_aer.csv 2>/dev/null
```