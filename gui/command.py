import sys
import os
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("operation")
parser.add_argument("input_file")
parser.add_argument("profile_mode", nargs="?", default="context") # this argument would be the output file name or profile mode.
args = parser.parse_args()

if args.operation == "init" :
    subprocess.run(["qcor", args.input_file])
    subprocess.run(["./a.out"]);
elif args.operation == "sim":
    subprocess.run(["python3", os.path.expanduser("~/QOPS/qcor-code/qasm/transform.py"), args.input_file])
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
    if args.profile_mode == "context" :
        subprocess.run(["LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/.llvm/lib QPGO_PROFILE_FILE=xxx.out python3 qft.py context"], shell=True)
    elif args.profile_mode == "counter" : 
        subprocess.run(["LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/.llvm/lib QPGO_PROFILE_FILE=xxx.out python3 qft.py counter"], shell=True)
    elif args.profile_mode == "normal" :
        subprocess.run(["LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/.llvm/lib QPGO_PROFILE_FILE=xxx.out python3 qft.py normal"], shell=True)
    os.chdir(os.path.expanduser("~/QOPS"))
elif args.operation == "pgo" :
    subprocess.run(["qcor", "-opt-pass", "pgo", args.input_file])
    subprocess.run(["./a.out"])
