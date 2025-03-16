## 1. run_microbenchmark

```bash
$ python python/microbenchmark_suite/run_microbenchmark/quokka.py --simulator_binary=../Quokka/cpu/Quokka

$ python python/microbenchmark_suite/run_microbenchmark/aer.py
```

## 2. train_perf_model

```bash
$ python python/microbenchmark_suite/train_perf_model/quokka.py
Namespace(microbenchmark_result='./log/microbenchmark_result_quokka.csv', model_folder='./model/quokka')
model error rate: 0.09959293174978058

$ python python/microbenchmark_suite/train_perf_model/aer.py 
Namespace(microbenchmark_result='./log/microbenchmark_result_aer.csv', model_folder='./model/aer')
model error rate: 0.03413687985997949
```

## 3. gen_cost_table

```bash
$ python python/microbenchmark_suite/gen_cost_table/quokka.py gen_table 32 18
Namespace(mode='gen_table', model_folder='./model/quokka', input_total_qubit=32, input_chunk_size=18, output_file='./log/gate_exe_time.csv')

$ python python/microbenchmark_suite/gen_cost_table/aer.py gen_table 32
Namespace(mode='gen_table', model_folder='./model/aer', input_total_qubit=32, output_file='./log/gate_exe_time.csv')
```

## 4. predict gate time (optional)

```bash
$ python python/microbenchmark_suite/gen_cost_table/quokka.py predict H 32 18
Namespace(mode='predict', model_folder='./model/quokka', input_gate_type='H', input_total_qubit=32, input_chunk_size=18)
56.89338679999995

$ python python/microbenchmark_suite/gen_cost_table/aer.py predict H 32 18
Namespace(mode='predict', model_folder='./model/aer', input_gate_type='H', input_total_qubit=32, input_target_qubit=18)
1282.4949640816392
```