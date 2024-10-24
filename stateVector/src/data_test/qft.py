from circuit_generator import *

def c_amod15(circuit, a, power):
    """Controlled multiplication by a mod 15"""
    if a not in [2,4,7,8,11,13]:
        raise ValueError("'a' must be 2,4,7,8,11 or 13")
    
    U = QuantumCircuit(4)
    
    for iteration in range(power):
        if a in [2, 13]:
            SWAP(circuit, 2, 3)
            SWAP(circuit, 1, 2)
            SWAP(circuit, 0, 1)
        if a in [7, 8]:
            SWAP(circuit, 0, 1)
            SWAP(circuit, 1, 2)
            SWAP(circuit, 2, 3)
        if a in [4, 11]:
            SWAP(circuit, 1, 3)
            SWAP(circuit, 0, 2)
        if a in [7,11,13]:
            for q in range(4):
                X(circuit, q)

    U.name = "%i^%i mod 15" % (a, power)
    c_U = U.control()
    return c_U

def qft(circuit):
    circuit=get_circuit()

    return circuit