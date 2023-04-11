import math

if __name__ == "__main__":
    for number_qbit in range(24, 41, 2):
        name = "qft_n" + str(number_qbit) +".cpp"
        with open(name, "w") as file:
            file.write("__qpu__ void qft_n" + str(number_qbit) + "(qreg q) {\n")
            file.write("    using qcor::openqasm;\n")
            file.write("    creg c[" + str(number_qbit) + "];\n\n")
            for i in range(number_qbit):
                file.write("    h q[" + str(i) + "];\n")
                if(i != number_qbit-1):
                    for j in range(i+1):
                        file.write("    cu1(" + str(math.pi/(2**(i+1-j))) + ") q[" + str(i+1) + "],q[" + str(j) + "];\n")
            for i in range(number_qbit):
                file.write("    measure q[" + str(i) + "] -> c[" + str(i) + "];\n")
            file.write("}\n")
            file.write("int main() {\n")
            file.write("    auto qubits = qalloc(" + str(number_qbit) + ");\n")
            file.write("    qft_n" + str(number_qbit) + "(qubits);\n")
            file.write("}")
            
