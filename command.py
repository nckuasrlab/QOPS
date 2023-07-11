import sys
import os
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("arg1")
parser.add_argument("arg2")
parser.add_argument("arg3", nargs="?", default="context")
args = parser.parse_args()

if args.arg1[-4:] == ".out" :
    subprocess.run(["qcor", "-o", args.arg1, args.arg2])
    output_file = "./" + args.arg1
    subprocess.run([output_file]);
elif args.arg1 == "sim":
    subprocess.run(["python3", "./qcor-code/qasm/transform.py", args.arg2])
    transform_circuit_file_name = "./output.txt"
    qfunc_file_name = os.path.expanduser("~/stateVector/src/correctness/qpu_function.py")
    with open(transform_circuit_file_name, "r") as circuit_file:
        circuit_file_lines = circuit_file.readlines() # read output.txt
    
    qfunc_file_lines = []
    qfunc_file_lines.insert(len(qfunc_file_lines), "from circuit_generator import *\n")
    qfunc_file_lines.insert(len(qfunc_file_lines), "def qpu_function() :\n")
    qfunc_file_lines.insert(len(qfunc_file_lines), "\tcircuit=get_circuit()\n")
    for i in circuit_file_lines :
        qfunc_file_lines.insert(len(qfunc_file_lines), "\t"+i)
    qfunc_file_lines.insert(len(qfunc_file_lines), "\treturn circuit\n")
    with open(qfunc_file_name, "w") as qfunc_file :
        qfunc_file.writelines(qfunc_file_lines)
    
    os.chdir(os.path.expanduser("~/stateVector/src/correctness"))
    subprocess.run(["rm result.txt && touch result.txt"], shell=True);
    if args.arg3 == "context" :
        subprocess.run(["LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/.llvm/lib QPGO_PROFILE_FILE=xxx.out python3 qft.py context"], shell=True)
    elif args.arg3 == "counter" : 
        subprocess.run(["LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/.llvm/lib QPGO_PROFILE_FILE=xxx.out python3 qft.py counter"], shell=True)
    os.chdir(os.path.expanduser("~/QOPS"))
elif args.arg1 == "pgo" :
    subprocess.run(["qcor", "-opt-pass", "pgo", args.arg2])
