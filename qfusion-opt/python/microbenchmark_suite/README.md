## 1. run_microbenchmark

## 2. train_perf_model

```bash
$ python python/microbenchmark_suite/train_perf_model.py -i ./log/microbenchmark_result_quokka.csv -t quokka -o ./model/quokka
Namespace(microbenchmark_result='./log/microbenchmark_result_quokka.csv', target='quokka', model_folder='./model/quokka', force=False)
model error rate: 0.12771817233915025

$ python python/microbenchmark_suite/train_perf_model.py -i ./log/microbenchmark_result_aer.csv -t aer -o ./model/aer
Namespace(microbenchmark_result='./log/microbenchmark_result_aer.csv', target='aer', model_folder='./model/aer', force=False)
model error rate: 0.041560536700683354
```

## 3. gen_cost_table