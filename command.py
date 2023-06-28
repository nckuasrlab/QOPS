import sys
import os
import subprocess

args1 = ''.join(sys.argv[1:2])
args2 = ''.join(sys.argv[2:3])

if args1[-4:] == ".out" :
    subprocess.run(["qcor", "-o", args1, args2])
    output_file = "./" + args1
    subprocess.run([output_file]);
elif args1 == "sim":
    subprocess.run(["python3", "./qcor-code/qasm/transform.py", args2]) # more than one .qasm file ?
    qft_file_name = "/home/nosba0957/stateVector/src/correctness/qft.py"
    transform_circuit_file_name = "./output.txt"
    with open(qft_file_name, 'r') as qft_file :
        qft_file_lines = qft_file.readlines()   # read qft.py to a list
    with open(transform_circuit_file_name, "r") as circuit_file:
        circuit_file_lines = circuit_file.readlines() # read output.txt

    start_line = 29 
    end_line = -6
    del qft_file_lines[start_line:end_line]
    for i in circuit_file_lines:
        qft_file_lines.insert(start_line-1, i)  # write output.txt to qft.py(list)
        start_line+=1
    with open(qft_file_name, 'w') as qft_file : # write back to qft.py
        qft_file.writelines(qft_file_lines)
    os.chdir(os.path.expanduser("~/stateVector/src/correctness"))
    subprocess.run(["rm result.txt && touch result.txt"], shell=True);
    subprocess.run(["LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/.llvm/lib QPGO_PROFILE_FILE=xxx.out python3 qft.py"], shell=True)
    os.chdir(os.path.expanduser("~/QOPS"))
