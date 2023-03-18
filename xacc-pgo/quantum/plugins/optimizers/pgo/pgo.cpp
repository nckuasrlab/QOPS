#include "pgo.hpp"
#include "xacc.hpp"

namespace xacc {
namespace quantum {
void PGO::apply(std::shared_ptr<CompositeInstruction> program,
                             const std::shared_ptr<Accelerator> accelerator,
                             const HeterogeneousMap &options) {
  std::cout << "Test PGO\n";
  // staq_rotation_folding if t gate is up to XX%
  int t = 3;
  if(t > 10){
    auto opt = xacc::getIRTransformation("rotation-folding");
    opt->apply(program, nullptr);
  }
  
}
} // namespace quantum
} // namespace xacc