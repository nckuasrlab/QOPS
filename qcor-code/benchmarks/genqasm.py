import sys
import os
import subprocess

output = subprocess.check_output("find *.cpp -printf \"%f\n\"", shell=True, text=True)
qasm_files = output.split("\n")
qasm_files.pop()

for qasm_file in qasm_files:
    print("gen file:", qasm_file)
    os.system("qcor -opt-pass " + sys.argv[1] + " " + qasm_file)
    os.system("./a.out")