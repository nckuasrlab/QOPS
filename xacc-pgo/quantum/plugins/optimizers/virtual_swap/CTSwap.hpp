#ifndef QUANTUM_GATE_COMPILER_CTSwap_HPP_
#define QUANTUM_GATE_COMPILER_CTSwap_HPP_

#include "IRTransformation.hpp"

namespace xacc {
namespace quantum {

class CTSwap : public IRTransformation, public OptionsProvider {

public:
  CTSwap() {}
  void apply(std::shared_ptr<CompositeInstruction> program,
                     const std::shared_ptr<Accelerator> accelerator,
                     const HeterogeneousMap& options = {}) override;
  const IRTransformationType type() const override {return IRTransformationType::Optimization;}

  const std::string name() const override { return "control-target-swap"; }
  const std::string description() const override { return ""; }

private:

};
} // namespace quantum
} // namespace xacc

#endif