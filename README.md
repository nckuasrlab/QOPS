# QOPS: A Compiler Framework for <ins>Q</ins>uantum Circuit <ins>S</ins>imulation Acceleration with <ins>P</ins>rofile Guided <ins>O</ins>ptimizations

QOPS is a quantum compiler framework that enables profile-guided optimization (PGO) for faster quantum circuit simulations. It collects performance data during simulation and uses it to generate an optimized version of the circuit, improving efficiency.

## Architecture

+ qcor-pgo: quantum compiler frontend ([qir-alliance/qcor f685743](https://github.com/qir-alliance/qcor/tree/f68574384335a1b4a303c7abf00e33e2020e469b)), [MIT license](https://github.com/nckuasrlab/QOPS/blob/main/qcor-pgo/LICENSE)
+ xacc-pgo: quantum compiler backend ([eclipse/xacc c7c4c795](https://github.com/eclipse/xacc/tree/c7c4c79541c1cc6b63d49dd433248cc6be85d3fb)), dual licensed - [Eclipse Public License](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo/LICENSE.EPL) and [Eclipse Distribution License](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo/LICENSE.EDL)
+ Quokka: storage-based statevector quantum circuit simulator ([drazermega7203/Quokka 64457d5](https://github.com/drazermega7203/Quokka/tree/64457d5d66fb305954aca383114b27a3672be264)), [GPL-3.0 license](https://github.com/nckuasrlab/QOPS/blob/main/Quokka/LICENSE)
+ llvm-pass-qpgo: Clang/LLVM instrumenter for quantum circuit simulator, [BSD-3-Clause license](https://github.com/nckuasrlab/QOPS/blob/main/llvm-pass-qpgo/LICENSE)
+ qviz-gui: interactive quantum performance analyzer and debugger, [GPL-3.0 license](https://github.com/nckuasrlab/QOPS/blob/main/qviz-gui/LICENSE)
+ qcor-code: example quantum programs, [BSD-3-Clause license](https://github.com/nckuasrlab/QOPS/blob/main/qcor-code/LICENSE)

## Requirements

+ Ubuntu 20.04 LTS
+ Clone QOPS to `~/QOPS`
+ Follow [AIDE-QC - Build the LLVM-CSP Fork](https://aide-qc.github.io/deploy/getting_started/build_from_source/#a-idllvmcspa-build-the-llvm-csp-fork) to clone, build, and install LLVM at `~/.llvm/`
+ Follow [AIDE-QC - Build Everything from Source](https://aide-qc.github.io/deploy/getting_started/build_from_source/) to build XACC ([xacc-pgo](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo)) and QCOR ([qcor-pgo](https://github.com/nckuasrlab/QOPS/blob/main/qcor-pgo)) from source

## Citation

```bib
@misc{wu2024qopscompilerframeworkquantum,
      title={QOPS: A Compiler Framework for Quantum Circuit Simulation Acceleration with Profile Guided Optimizations},
      author={Yu-Tsung Wu and Po-Hsuan Huang and Kai-Chieh Chang and Chia-Heng Tu and Shih-Hao Hung},
      year={2024},
      eprint={2410.09326},
      archivePrefix={arXiv},
      primaryClass={quant-ph},
      url={https://arxiv.org/abs/2410.09326},
}
```
