#include "pgo.hpp"
#include "Graph.hpp"
#include "CountGatesOfTypeVisitor.hpp"
#include "CommonGates.hpp"
#include "Circuit.hpp"
#include "GateIR.hpp"
#include "Utils.hpp"
#include "xacc_service.hpp"
#include "xacc.hpp"
#include <assert.h>
#include "PhasePolynomialRepresentation.hpp"
#include <regex>

namespace xacc {
namespace quantum {

void PGO::apply(std::shared_ptr<CompositeInstruction> gateFunction,
                             const std::shared_ptr<Accelerator> accelerator,
                             const HeterogeneousMap &options) {
  std::cout << "Test\n";
}
} // namespace quantum
} // namespace xacc