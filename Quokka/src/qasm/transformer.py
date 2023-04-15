import os
import sys
import math
from os import listdir
from os.path import isfile, join
from circuit_generator import *
import decimal

# create a new context for this task
ctx = decimal.Context()
ctx.prec = 8

def float_to_str(entry):
    """
    Convert the given float to a string,
    without resorting to scientific notation
    """
    d1 = ctx.create_decimal(repr(entry))
    return format(d1, 'f')

def getArg(line:str, arg:int = 1):
    return int(line.split(' ')[arg].split('[')[1].split(']')[0])

def toStrList(mat:list):
    res = []
    for entry in mat:
        res.append(float_to_str(entry))
    return res


def gen(filename):
    gate_list = {}
    gate_count = -4
    
    special = ["u3", "rx", "ry", "rz"]
    
    circuit = get_circuit()
    with open(filename) as file:
        for file_line in iter(file):
            line = file_line.rstrip()
            special_flag = 0
            gate_count += 1
            if gate_count <= 0:
                continue
            for gate in special:
                if line.split(' ')[0][0:2] == gate:
                    special_flag = 1
                    if gate not in gate_list:
                        gate_list[gate] = 1
                    else:
                        gate_list[gate] += 1
                    break
            if special_flag == 1:
                ## output file
                if line.split(' ')[0][0:2] == "rx":
                    n_1 = line.split(' ')[0].split('(')[1].split(')')[0]
                    real_1 = round(math.cos(float(n_1)/2), 8)
                    imag_1 = round(-math.sin(float(n_1)/2), 8)
                    U1(circuit, getArg(line), toStrList([real_1, 0, 0, real_1]), toStrList([0, imag_1, imag_1, 0]))
    
                elif line.split(' ')[0][0:2] == "ry":
                    n_1 = line.split(' ')[0].split('(')[1].split(')')[0]
                    real_1 = round(math.cos(float(n_1)/2), 8)
                    real_3 = round(math.sin(float(n_1)/2), 8)
                    U1(circuit, getArg(line), toStrList([real_1, -real_3, real_3, real_1]), toStrList([0, 0, 0, 0]))
    
                elif line.split(' ')[0][0:2] == "rz":
                    n_1 = line.split(' ')[0].split('(')[1].split(')')[0]
                    real_1 = round(math.cos(float(n_1)/2), 8)
                    imag_1 = round(-math.sin(float(n_1)/2), 8)
                    U1(circuit, getArg(line), toStrList([real_1, 0, 0, real_1]), toStrList([imag_1, 0, 0, -imag_1]))
    
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
                    U1(circuit, getArg(line), toStrList([real_1, real_2, real_3, real_4]), toStrList([0, imag_2, imag_3, imag_4]))
    
                continue
    
            if line.split(' ')[0] not in gate_list:
                gate_list[line.split(' ')[0]] = 1
            else:   
                gate_list[line.split(' ')[0]] += 1
            ## output file
            if line.split(' ')[0] == "CX":
                CX(circuit, getArg(line), getArg(line, 2))
            
            elif line.split(' ')[0] == "cy":
                CY(circuit, getArg(line), getArg(line, 2))

            elif line.split(' ')[0] == "cz":
                CZ(circuit, getArg(line), getArg(line, 2))

            elif line.split(' ')[0] == "h":
                H(circuit, getArg(line))
    
            elif line.split(' ')[0] == "t":
                T(circuit, getArg(line))
            elif line.split(' ')[0] == "x":
                X(circuit, getArg(line))
            elif line.split(' ')[0] == "z":
                Z(circuit, getArg(line))
            elif line.split(' ')[0] == "s":
                S(circuit, getArg(line))
            elif line.split(' ')[0] == "tdg":
                U1(circuit, getArg(line), toStrList([1, 0, 0, 0.70710678]), toStrList([0, 0, 0, -0.70710678]))
    
            elif line.split(' ')[0] == "sdg":
                U1(circuit, getArg(line), toStrList([1, 0, 0, 0]), toStrList([0, 0, 0, -1]))
    
            elif line.split(' ')[0] == "measure":
                measure(circuit, getArg(line))
    
            elif line.split(' ')[0] == "swap":
                SWAP(circuit, getArg(line), getArg(line, 2))
    
            else:
                print("no this gate")
                print(line)
    return circuit, gate_list, gate_count

mypath = sys.argv[1]
onlyfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))]

output_dir = "ground_truth"
isExist = os.path.exists(output_dir)
if not isExist:
   os.makedirs(output_dir) # create dir for output

for f in onlyfiles:
    if f[-5:] != ".qasm":
        continue
    circuit, gate_list, gate_count = gen(os.path.join(mypath, f))
    newName = os.path.join(output_dir, os.path.splitext(f)[0] + '_out.txt')
    create_circuit(circuit, newName)
    print(gate_list)
    print("gate_count: " + str(gate_count))