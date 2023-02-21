import sys

gate_list = {}
gate_count = -4

two_word = ["u3", "rx", "ry", "rz"]

with open(sys.argv[1]) as file:
    while (line := file.readline().rstrip()):
        flag = 0
        gate_count += 1
        for gate in two_word:
            if line.split( )[0][0:2] == gate:
                flag = 1
                if gate not in gate_list:
                    gate_list[gate] = 1
                else:
                    gate_list[gate] += 1
                break
        if flag == 1:
            continue
        if line.split( )[0] not in gate_list:
            gate_list[line.split( )[0]] = 1
        else:   
            gate_list[line.split( )[0]] += 1

gate_list.pop('OPENQASM', None)
gate_list.pop('include', None)
gate_list.pop('qreg', None)
gate_list.pop('creg', None)
print(gate_list)
print("gate_count: " + str(gate_count))

## rx ry rz u3