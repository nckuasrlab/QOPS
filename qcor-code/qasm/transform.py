import sys
import math

gate_list = {}
gate_count = -4

special = ["u3", "rx", "ry", "rz"]

with open("output.txt", 'w') as out:
    with open(sys.argv[1]) as file:
        while (line := file.readline().rstrip()):
            flag = 0
            gate_count += 1
            if gate_count <= 0:
                continue
            for gate in special:
                if line.split(' ')[0][0:2] == gate:
                    flag = 1
                    if gate not in gate_list:
                        gate_list[gate] = 1
                    else:
                        gate_list[gate] += 1
                    break
            if flag == 1:
                ## output file
                if line.split(' ')[0][0:2] == "rx":
                    n_1 = line.split(' ')[0].split('(')[1].split(')')[0]
                    real_1 = round(math.cos(float(n_1)/2), 8)
                    imag_1 = round(-math.sin(float(n_1)/2), 8)
                    out.write("U1(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] 
                                    + ", [" + str(real_1) + ", 0, 0, " + str(real_1) + "]"
                                    + ", [0, " + str(imag_1) + ", " + str(imag_1) + ", 0])\n")
                elif line.split(' ')[0][0:2] == "ry":
                    n_1 = line.split(' ')[0].split('(')[1].split(')')[0]
                    real_1 = round(math.cos(float(n_1)/2), 8)
                    real_3 = round(math.sin(float(n_1)/2), 8)
                    out.write("U1(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] 
                                    + ", [" + str(real_1) + ", " + str(-real_3) + ", " + str(real_3) + ", " + str(real_1) + "]"
                                    + ", [0, 0, 0, 0])\n")
                elif line.split(' ')[0][0:2] == "rz":
                    n_1 = line.split(' ')[0].split('(')[1].split(')')[0]
                    real_1 = round(math.cos(float(n_1)/2), 8)
                    imag_1 = round(-math.sin(float(n_1)/2), 8)
                    out.write("U1(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] 
                                    + ", [" + str(real_1) + ", 0, 0, " + str(real_1) + "]"
                                    + ", [" + str(imag_1) + ", 0, 0, " + str(-imag_1) + "])\n")
                elif line.split(' ')[0][0:2] == "u3":
                    n_1 = line.split(' ')[0].split('(')[1].split(')')[0].split(',')[0]
                    n_2 = line.split(' ')[0].split('(')[1].split(')')[0].split(',')[1]
                    n_3 = line.split(' ')[0].split('(')[1].split(')')[0].split(',')[2]
                    real_1 = round(math.cos(float(n_1)/2), 8)
                    real_2 = -round(math.cos(float(n_3)) * math.sin(float(n_1)/2), 8)
                    imag_2 = -round(math.sin(float(n_3)) * math.sin(float(n_1)/2), 8)
                    real_3 = round(math.cos(float(n_2)) * math.sin(float(n_1)/2), 8)
                    imag_3 = round(math.sin(float(n_2)) * math.sin(float(n_1)/2), 8)
                    real_4 = round(math.cos(float(n_2) + float(n_3)) * math.cos(float(n_1)/2), 8)
                    imag_4 = round(math.sin(float(n_2) + float(n_3)) * math.cos(float(n_1)/2), 8)
                    out.write("U1(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] 
                                    + ", [" + str(real_1) + ", " + str(real_2) + ", " + str(real_3) + ", " + str(real_4) + "]"
                                    + ", [0, " + str(imag_2) + ", " + str(imag_3) + ", " + str(imag_4) + "])\n")
                continue


            if line.split(' ')[0] not in gate_list:
                gate_list[line.split(' ')[0]] = 1
            else:   
                gate_list[line.split(' ')[0]] += 1
            ## output file
            if line.split(' ')[0] == "CX":
                out.write("CX(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ", " + line.split(' ')[2].split('[')[1].split(']')[0] + ")\n")
            elif line.split(' ')[0] == "h":
                out.write("H(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ")\n")
            elif line.split(' ')[0] == "t":
                out.write("T(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ")\n")
            elif line.split(' ')[0] == "x":
                out.write("X(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ")\n")
            elif line.split(' ')[0] == "z":
                out.write("Z(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ")\n")
            elif line.split(' ')[0] == "s":
                out.write("S(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ")\n")
            elif line.split(' ')[0] == "tdg":
                out.write("U1(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] 
                                + ", [1, 0, 0, 0.70710678]"
                                + ", [0, 0, 0, -0.70710678])\n")
            elif line.split(' ')[0] == "sdg":
                out.write("U1(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] 
                                + ", [1, 0, 0, 0]"
                                + ", [0, 0, 0, -1])\n")
            elif line.split(' ')[0] == "measure":
                out.write("measure(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ", 1)\n")
            elif line.split(' ')[0] == "swap":
                out.write("SWAP(circuit, " + line.split(' ')[1].split('[')[1].split(']')[0] + ", " + line.split(' ')[2].split('[')[1].split(']')[0] + ")\n")
            else:
                print("no this gate")
                print(line)

print(gate_list)
print("gate_count: " + str(gate_count))