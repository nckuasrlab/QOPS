# Microbenchmark Suite

## 1. Feature selection

```bash
$ python python/microbenchmark_suite/find_feature.py 
Namespace(simulator_binary='./cpu/Quokka', num_repeat_runs=100, total_qubit_min=24, total_qubit_max=32, chunk_min=10, chunk_max=18, simulation_type='MEM', microbenchmark_result_file='./log/microbenchmark_result_quokka.csv')
one qubit gate:
target_qubit_one 1.0886583360282662
total_qubit 3095.3232658495363
CHUNK_MAX 30.615536172855986
gate_type_H 339.72771796217927
gate_type_RX 153.57815634144117
gate_type_RY 157.57045378593688
gate_type_RZ 11.43547482859191
gate_type_U1 146.39253623590074
gate_type_X 253.48048028491058

two qubit gate:
target_qubit_one 7.45563503380855
target_qubit_two 7.72361522119665
total_qubit 7515.879415834489
chunk_size 44.32762223584536
gate_type_CP 1757.2640130822379
gate_type_CX 1714.8809294017628
gate_type_CZ 2732.134644812391
gate_type_RZZ 13.09225046051525
gate_type_U2 31608.323270706
```

## 2. run_microbenchmark

```bash
$ python python/microbenchmark_suite/run_microbenchmark/quokka.py --simulator_binary=../Quokka/cpu/Quokka

$ python python/microbenchmark_suite/run_microbenchmark/aer.py
Namespace(num_repeat_runs=100, total_qubit_min=24, total_qubit_max=32, microbenchmark_result_file='./log/microbenchmark_result_aer.csv')
Total time: 27118.383654820966
```

## 3. train_perf_model

```bash
$ python python/microbenchmark_suite/train_perf_model/quokka.py
Namespace(microbenchmark_result='./log/microbenchmark_result_quokka.csv', model_folder='./model/quokka')
model error rate: 0.09959293174978058

$ python python/microbenchmark_suite/train_perf_model/aer.py 
Namespace(microbenchmark_result='./log/microbenchmark_result_aer.csv', model_folder='./model/aer')
model error rate: 0.036115926738519114
```

## 4. gen_cost_table

```bash
$ python python/microbenchmark_suite/gen_cost_table/quokka.py gen_table 32 18
Namespace(mode='gen_table', model_folder='./model/quokka', input_total_qubit=32, input_chunk_size=18, output_file='./log/gate_exe_time.csv')

$ python python/microbenchmark_suite/gen_cost_table/aer.py gen_table 32
Namespace(mode='gen_table', model_folder='./model/aer', input_total_qubit=32, output_file='./log/gate_exe_time_aer.csv')
```

## 5. predict gate time (optional)

```bash
$ python python/microbenchmark_suite/gen_cost_table/quokka.py predict H 32 18
Namespace(mode='predict', model_folder='./model/quokka', input_gate_type='H', input_total_qubit=32, input_chunk_size=18)
56.89338679999995

$ python python/microbenchmark_suite/gen_cost_table/aer.py predict H 32 18
Namespace(mode='predict', model_folder='./model/aer', input_gate_type='H', input_total_qubit=32, input_target_qubit=18)
1282.4949640816392
```