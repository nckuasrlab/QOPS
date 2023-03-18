#ifndef QUANTUM_GATE_COMPILER_Virtual_Swap_HPP_
#define QUANTUM_GATE_COMPILER_Virtual_Swap_HPP_

#include "IRTransformation.hpp"

namespace xacc {
namespace quantum {

class VirtualSwap : public IRTransformation, public OptionsProvider {

public:
  VirtualSwap() {}
  void apply(std::shared_ptr<CompositeInstruction> program,
                     const std::shared_ptr<Accelerator> accelerator,
                     const HeterogeneousMap& options = {}) override;
  const IRTransformationType type() const override {return IRTransformationType::Optimization;}

  const std::string name() const override { return "virtual-swap"; }
  const std::string description() const override { return ""; }

private:

};
} // namespace quantum
} // namespace xacc

#endif