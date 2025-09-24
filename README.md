# QOPS: A Compiler Framework for <ins>Q</ins>uantum Circuit <ins>S</ins>imulation Acceleration with <ins>P</ins>rofile Guided <ins>O</ins>ptimizations

QOPS is a quantum compiler framework that enables profile-guided optimization (PGO) for faster quantum circuit simulations. It collects performance data during simulation and uses it to generate an optimized version of the circuit, improving efficiency.

## Architecture

+ qcor-pgo: quantum compiler frontend ([qir-alliance/qcor f685743](https://github.com/qir-alliance/qcor/tree/f68574384335a1b4a303c7abf00e33e2020e469b)), [MIT license](https://github.com/nckuasrlab/QOPS/blob/main/qcor-pgo/LICENSE)
+ xacc-pgo: quantum compiler backend ([eclipse/xacc c7c4c795](https://github.com/eclipse/xacc/tree/c7c4c79541c1cc6b63d49dd433248cc6be85d3fb)), dual licensed - [Eclipse Public License](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo/LICENSE.EPL) and [Eclipse Distribution License](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo/LICENSE.EDL)
+ stateVector: storage-based statevector quantum circuit simulator ([drazermega7203/stateVector a5639e6](https://github.com/drazermega7203/stateVector/tree/a5639e6be6eaab2c30a12e0fbda9422e3cfa9ec3)), [GPL-3.0 license](https://github.com/nckuasrlab/QOPS/blob/main/stateVector/LICENSE)
+ llvm-pass-qpgo: Clang/LLVM instrumenter for quantum circuit simulator, [BSD-3-Clause license](https://github.com/nckuasrlab/QOPS/blob/main/llvm-pass-qpgo/LICENSE)
+ qviz-gui: interactive quantum performance analyzer and debugger, [GPL-3.0 license](https://github.com/nckuasrlab/QOPS/blob/main/qviz-gui/LICENSE)
+ qcor-code: example quantum programs, [BSD-3-Clause license](https://github.com/nckuasrlab/QOPS/blob/main/qcor-code/LICENSE)
+ qfusion-opt: a novel gate fusion workflow and algorithm that leverages profile-informed techniques, [Apache-2.0 license](https://github.com/nckuasrlab/QOPS/blob/main/qfusion-opt/LICENSE)
  + qiskit-aer: a high performance simulator for quantum circuits written in Qiskit ([b77f005; tag:0.14.2](https://github.com/Qiskit/qiskit-aer/tree/b77f00578101d449ae9489a4ab164a7d11dcd1b3)), Apache-2.0 license

## Requirements

+ Ubuntu 20.04 LTS
+ Clone QOPS to `~/QOPS`, and move stateVector to home (`mv ~/QOPS/stateVector ~/stateVector`)
+ Follow [AIDE-QC - Build the LLVM-CSP Fork](https://aide-qc.github.io/deploy/getting_started/build_from_source/#a-idllvmcspa-build-the-llvm-csp-fork) to clone, build, and install LLVM at `~/.llvm/`
+ Follow [AIDE-QC - Build Everything from Source](https://aide-qc.github.io/deploy/getting_started/build_from_source/) to build XACC ([xacc-pgo](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo)) and QCOR ([qcor-pgo](https://github.com/nckuasrlab/QOPS/blob/main/qcor-pgo)) from source

## Citation

```bib
@article{Wu2025QOPS,
  author    = {Wu, Yu-Tsung and Huang, Po-Hsuan and Chang, Kai-Chieh and Tu, Chia-Heng and Hung, Shih-Hao},
  title     = {{QOPS}: a compiler framework for quantum circuit simulation acceleration with profile-guided optimizations},
  journal   = {The Journal of Supercomputing},
  year      = {2025},
  month     = {Mar},
  day       = {30},
  volume    = {81},
  number    = {5},
  articleno = {674},
  issn      = {1573-0484},
  doi       = {10.1007/s11227-025-07157-2}
}
```
