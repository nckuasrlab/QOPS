/*******************************************************************************
 * Copyright (c) 2020 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Alexander J. McCaskey - initial API and implementation
 *******************************************************************************/
#include "fusion.hpp"
#include "optimization/simplify.hpp"
#include "xacc_service.hpp"
#include "xacc.hpp"
#include "staq_visitors.hpp"
#include "optimization/fusion.hpp"

namespace xacc {
namespace quantum {

using namespace staq;
using namespace staq::ast;

void Fusion::apply(std::shared_ptr<CompositeInstruction> program,
                            const std::shared_ptr<Accelerator> accelerator,
                            const HeterogeneousMap &options) {

  // map to openqasm
  auto staq = xacc::getCompiler("staq");
  auto src = staq->translate(program);

  // parse that to get staq ast
  auto prog = parser::parse_string(src);

  // Gate fusion
  std::string fusion_res = optimization::fusion(*prog, options);
  std::string kernel_name = options.get<std::string>("kernel_name");
  std::ofstream fused_file ((kernel_name + "_fused.txt"));
  if (fused_file.is_open()) {
    fused_file << fusion_res;
    fused_file.close();
  }
}

} // namespace quantum
} // namespace xacc
