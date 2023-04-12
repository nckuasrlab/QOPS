# Introduction
Quokka is a powerful simulator designed for high-performance simulations of quantum circuits, state-vectors, and density matrices.
As a demonstration, we are currently releasing a sample of the H (Hadamard) gate for users to explore and experience the performance of a given hardware.
In the near future, we plan to release additional quantum operations, including CX gate, Phase gate, Toffoli gate, and unitary gates for multi-qubits, to further enhance the capabilities. Stay tuned for these exciting updates!
# Getting started
To rocket right in, download Quokka with git at the terminal
```
git clone https://github.com/drazermega7203/Quokka.git
cd Quokka
```

Compile the example (source) using make
```
cd src
make -j
```

then run it with
```
./quokka -c circuit/H.txt -i ini/1.ini
```
It will run a 20-qubit circuit with 10 H-gate on qubit 0, where H.txt and 1.ini are the circuit file and the config file, respectively.
