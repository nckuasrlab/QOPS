//===-- Qpgo.h --------------------------------------------*- C++ -*-===//
#ifndef LLVM_PASS_QPGO_H
#define LLVM_PASS_QPGO_H

#include <llvm/Pass.h>
#include <unordered_map>

// X-macro
#define PROFILED_FUNC                                                          \
  ENTRY(single_gate)                                                           \
  ENTRY(control_gate)                                                          \
  ENTRY(unitary4x4)                                                            \
  ENTRY(SWAP)                                                                  \
  ENTRY(unitary8x8)

enum ProfiledFuncKind {
#define ENTRY(f) PFK_##f,
  PROFILED_FUNC
#undef ENTRY
};

namespace llvm {
class Function;
class Module;

class QpgoPass : public ModulePass {
  /* Other private data members and functions */
public:
  static char ID;
  explicit QpgoPass() : ModulePass(ID) {}
  // Main run interface method.
  virtual bool runOnModule(Module &M) override;
  StringRef getPassName() const override { return "QpgoPass"; }

private:
  inline static std::unordered_map<std::string, ProfiledFuncKind> ProfiledFuncs{
#define ENTRY(f) {#f, PFK_##f},
      PROFILED_FUNC
#undef ENTRY
  };
};
} // namespace llvm

#endif /* LLVM_PASS_QPGO_H */
