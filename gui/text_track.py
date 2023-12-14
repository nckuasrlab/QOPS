def preprocess(source, qasm): # mapping source and qasm string line by line
    source = source.splitlines()
    qasm = qasm.splitlines()
    source = find_qpu_func(source)
    map = {}
    key = ()
    qasm_idx = 4
    source_idx = 1
    for line in source:
        if line[:2] == "cx" or line[:1] == "x" or line[:1] == "h" or line[:2] == "rz" \
            or line[:2] == "ry" or line[:2] == "rx" or line[:2] == "u3" or line[:2] == "u1" \
            or line[:7] == "measure":

            key = (qasm_idx + 1, qasm_idx + 1)
            map[key] = source_idx
            qasm_idx += 1
        elif line[:2] == "ch" or line[:3] == "ccx" or line[:3] == "crz" or line[:3] == "cu1" \
            or line[:3] == "cu3":
            begin_tmp = qasm_idx + 1
            while qasm[qasm_idx] != "// comment": # use compiler emitted message to distinguish betweeen different gate
                qasm_idx += 1
            del qasm[qasm_idx]
            key = (begin_tmp, qasm_idx)
            map[key] = source_idx
        source_idx += 1
    return map, qasm

def find_qpu_func(source): # handle the cpp source irrelevant code
    cnt = 0
    for line in source:
        if line[:7] == "__qpu__":
            break
        cnt+=1
    source = source[cnt:]
    for line in source:
        if line[0:1] == '}':
            break
        cnt+=1
    source = source[:cnt]
    source = [s.lstrip() for s in source] # remove leading space in source cpp file
    return source
