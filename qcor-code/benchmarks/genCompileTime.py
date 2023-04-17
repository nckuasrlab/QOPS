import sys
import subprocess

roundtime = '1'

def exe(name: str, command: list):
    with open(name, "w") as file:
        for qasm_file in qasm_files:
            print("gen file:", qasm_file)
            file.write("gen compile time: " + qasm_file + "\n")
            try:
                exe_command = command.copy()
                exe_command.append(qasm_file)
                output = subprocess.run(exe_command, capture_output=True, text=True, timeout=60*int(roundtime))
                lines = output.stderr.split("\n")
                time1 = lines[3].split()[0]
                output = subprocess.run(['perf', 'stat', '--null', '-r', roundtime, './a.out'], capture_output=True, text=True, timeout=(900-float(time1))*int(roundtime))
                lines = output.stderr.split("\n")
                time2 = lines[3].split()[0]
                file.write("qcor time: " + str(round(float(time1), 4)) + "\n")
                file.write("exe time: " + str(round(float(time2), 4)) + "\n")
                file.write("total time: " + str(round(float(time1) + float(time2), 4)) + "\n")
                file.flush()
            except:
                file.write("timeout\n")
                file.flush()

if __name__ == '__main__':
    output = subprocess.check_output("find *.cpp -printf \"%f\n\"", shell=True, text=True)
    qasm_files = output.split("\n")
    qasm_files.pop()
    if(len(sys.argv) == 2):
        exe("record_" + sys.argv[1] + ".txt", ['perf', 'stat', '--null', '-r', roundtime, 'qcor', '-opt-pass', sys.argv[1]])
    if(len(sys.argv) == 1):
        exe("record.txt", ['perf', 'stat', '--null', '-r', roundtime, 'qcor'])
    if(len(sys.argv) == 3):
        exe("record_lv" + sys.argv[2] + ".txt", ['perf', 'stat', '--null', '-r', roundtime, 'qcor', '-opt', sys.argv[2]])