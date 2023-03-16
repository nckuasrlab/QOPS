# llvm-pass-qpgo

Advenced version of injecting printf function to the specific call instruments

+ timestamp

## Build

```bash
$ cd llvm-pass-qpgo
$ mkdir build && cd build
$ cmake -G Ninja ..
$ ninja
```

## Run

```bash
$ cd simplifiedStateVector/src/
$ make pgo 2>|out.txt
```

