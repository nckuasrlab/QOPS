# QOPS: A Compiler Framework for <ins>Q</ins>uantum Circuit <ins>S</ins>imulation Acceleration with <ins>P</ins>rofile Guided <ins>O</ins>ptimizations

QOPS is a quantum compiler framework that enables profile-guided optimization (PGO) for faster quantum circuit simulations. It collects performance data during simulation and uses it to generate an optimized version of the circuit, improving efficiency.

## Architecture

+ qcor-pgo: quantum compiler frontend (qir-alliance/qcor f685743), [MIT license](https://github.com/nckuasrlab/QOPS/blob/main/qcor-pgo/LICENSE)
+ xacc-pgo: quantum compiler backend (eclipse/xacc c7c4c795), dual licensed - [Eclipse Public License](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo/LICENSE.EPL) and [Eclipse Distribution License](https://github.com/nckuasrlab/QOPS/blob/main/xacc-pgo/LICENSE.EDL)
+ stateVector: quantum circuit simulator, GPL-3.0 license
+ llvm-pass-qpgo: Clang/LLVM instrumenter for quantum circuit simulator, [BSD-3-Clause license](https://github.com/nckuasrlab/QOPS/blob/main/llvm-pass-qpgo/LICENSE)
+ qviz-gui: interactive quantum performance analyzer and debugger, [GPL-3.0 license](https://github.com/nckuasrlab/QOPS/blob/main/qviz-gui/LICENSE)
+ qcor-code: example quantum programs, [BSD-3-Clause license](https://github.com/nckuasrlab/QOPS/blob/main/qcor-code/LICENSE)

## Requirements

+ Follow [AIDE-QC - Build the LLVM-CSP Fork](https://aide-qc.github.io/deploy/getting_started/build_from_source/#a-idllvmcspa-build-the-llvm-csp-fork) to build LLVM and install it at `~/.llvm/`
+ Follow [AIDE-QC - Build Everything from Source](https://aide-qc.github.io/deploy/getting_started/build_from_source/) to build XACC and QCOR from source

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
