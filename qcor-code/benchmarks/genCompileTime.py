import sys
import subprocess

output = subprocess.check_output("find *.cpp -printf \"%f\n\"", shell=True, text=True)
qasm_files = output.split("\n")
qasm_files.pop()

roundtime = '2'

if(len(sys.argv) == 2):
    record_name = "record_" + sys.argv[1] + ".txt"
    with open(record_name, "w") as file:
        for qasm_file in qasm_files:
            print("gen file:", qasm_file)
            file.write("gen compile time: " + qasm_file + "\n")
            output = subprocess.run(['perf', 'stat', '--null', '-r', roundtime, 'qcor', '-opt-pass', sys.argv[1], qasm_file], capture_output=True, text=True)
            lines = output.stderr.split("\n")
            time1 = lines[3].split()[0]
            output = subprocess.run(['perf', 'stat', '--null', '-r', roundtime, './a.out'], capture_output=True, text=True)
            lines = output.stderr.split("\n")
            time2 = lines[3].split()[0]
            file.write("qcor time: " + str(round(float(time1), 4)) + "\n")
            file.write("exe time: " + str(round(float(time2), 4)) + "\n")
            file.write("total time: " + str(round(float(time1) + float(time2), 4)) + "\n")

if(len(sys.argv) == 1):
    record_name = "record.txt"
    with open(record_name, "w") as file:
        for qasm_file in qasm_files:
            print("gen file:", qasm_file)
            file.write("gen compile time: " + qasm_file + "\n")
            output = subprocess.run(['perf', 'stat', '--null', '-r', roundtime, 'qcor', qasm_file], capture_output=True, text=True)
            lines = output.stderr.split("\n")
            time1 = lines[3].split()[0]
            output = subprocess.run(['perf', 'stat', '--null', '-r', roundtime, './a.out'], capture_output=True, text=True)
            lines = output.stderr.split("\n")
            time2 = lines[3].split()[0]
            file.write("qcor time: " + str(round(float(time1), 4)) + "\n")
            file.write("exe time: " + str(round(float(time2), 4)) + "\n")
            file.write("total time: " + str(round(float(time1) + float(time2), 4)) + "\n")

if(len(sys.argv) == 3):
    record_name = "record_lv" + sys.argv[2] + ".txt"
    with open(record_name, "w") as file:
        for qasm_file in qasm_files:
            print("gen file:", qasm_file)
            file.write("gen compile time: " + qasm_file + "\n")
            output = subprocess.run(['perf', 'stat', '--null', '-r', roundtime, 'qcor', '-opt', sys.argv[2], qasm_file], capture_output=True, text=True)
            lines = output.stderr.split("\n")
            time1 = lines[3].split()[0]
            output = subprocess.run(['perf', 'stat', '--null', '-r', roundtime, './a.out'], capture_output=True, text=True)
            lines = output.stderr.split("\n")
            time2 = lines[3].split()[0]
            file.write("qcor time: " + str(round(float(time1), 4)) + "\n")
            file.write("exe time: " + str(round(float(time2), 4)) + "\n")
            file.write("total time: " + str(round(float(time1) + float(time2), 4)) + "\n")