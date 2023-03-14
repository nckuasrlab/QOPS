#ifndef QUANTUM_GATE_COMPILER_PGO_HPP_
#define QUANTUM_GATE_COMPILER_PGO_HPP_

#include "IRTransformation.hpp"
#include "InstructionIterator.hpp"
#include "OptionsProvider.hpp"

namespace xacc {
namespace quantum {

class PGO : public IRTransformation, public OptionsProvider {

public:
  PGO() {}
  void apply(std::shared_ptr<CompositeInstruction> program,
                     const std::shared_ptr<Accelerator> accelerator,
                     const HeterogeneousMap& options = {}) override;
  const IRTransformationType type() const override {return IRTransformationType::Optimization;}

  const std::string name() const override { return "pgo"; }
  const std::string description() const override { return ""; }

private:

};
} // namespace quantum
} // namespace xacc

#endif