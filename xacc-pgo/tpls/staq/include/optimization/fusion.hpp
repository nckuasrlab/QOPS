/*
 * This file is part of staq.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * \file optimization/fusion.hpp
 * \brief Gate fusion algorithm
 */
#pragma once

#include "ast/visitor.hpp"
#include "ast/replacer.hpp"
#include "gates/channel.hpp"
#include "fusion_algo.hpp"

#include <list>
#include <string>
#include <unordered_map>
#include <sstream>
#include <regex>
#include <iomanip>

namespace staq {
namespace optimization {

/**
 * \class staq::optimization::RotationOptimizer
 * \brief Rotation gate merging algorithm based on arXiv:1903.12456
 *
 * Returns a replacement list giving the nodes to the be replaced (or erased)
 */
class FusionOptimizer final : public ast::Visitor {
public:
  /* Variables */
  void visit(ast::VarAccess &) {}

  /* Expressions */
  void visit(ast::BExpr &) {}
  void visit(ast::UExpr &) {}
  void visit(ast::PiExpr &) {}
  void visit(ast::IntExpr &) {}
  void visit(ast::RealExpr &) {}
  void visit(ast::VarExpr &) {}
  /* Statements */
  void visit(ast::MeasureStmt &stmt) {}
  void visit(ast::ResetStmt &stmt) {}
  void visit(ast::IfStmt &stmt) {}

  /* Gates */
  void visit(ast::UGate &gate) {}
  void visit(ast::CNOTGate &gate) {}
  void visit(ast::BarrierGate &gate) {}
  void visit(ast::CommentGate &gate) {}
  void visit(ast::DeclaredGate &gate) {}
  /* Declarations */
  void visit(ast::GateDecl &decl) {}
  void visit(ast::OracleDecl &) {}
  void visit(ast::RegisterDecl &) {}
  void visit(ast::AncillaDecl &) {}
  /* Program */
  void visit(ast::Program &prog) {
    std::ostringstream oss;

    prog.pretty_print(oss);
    oss = xacc_to_fusion_format(oss);
    int mode = fusion_option.get<int>("mode");
    int total_qubit = fusion_option.get<int>("total_qubit");
    int max_fusion_qubit = fusion_option.get<int>("max_fusion_qubit");
    std::string res = fusion_algo(oss.str(), mode, max_fusion_qubit, total_qubit);
    fusion_res = res;
  }

  /* run */
  std::string run(ast::ASTNode &node, const xacc::HeterogeneousMap &options) {
      fusion_option = options;
      node.accept(*this);
      return fusion_res;
  }

private:
  std::string fusion_res;
  xacc::HeterogeneousMap fusion_option;

  std::ostringstream xacc_to_fusion_format(std::ostringstream &xacc_format) {
    std::ostringstream res_oss;
    std::string line;

    std::istringstream iss(xacc_format.str());
    std::regex one_qubit_regex(R"(([X|H|tdg|t|h])\s+qrg_nWlrB\[(\d+)\];)");
    std::regex two_qubit_regex(
        R"((CX|CY|CZ|CP)\s+qrg_nWlrB\[(\d+)\],qrg_nWlrB\[(\d+)\];)");
    std::regex rotation_qubit_regex(
        R"((rx|ry|rz)\(([-+]?[0-9]*\.?[0-9]+)\)\s+qrg_nWlrB\[(\d+)\];)");

    while (std::getline(iss, line)) {
      std::smatch match;
      if (regex_match(line, match, two_qubit_regex)) {
        std::string gateType = match[1];
        for (size_t i = 0; i < gateType.size(); i++)
          gateType[i] = std::toupper(static_cast<unsigned char>(gateType[i]));
        res_oss << gateType << " " << match[2] << " " << match[3] << "\n";
      } else if (regex_match(line, match, one_qubit_regex)) {
        std::string gateType = match[1];
        for (size_t i = 0; i < gateType.size(); i++)
          gateType[i] = std::toupper(static_cast<unsigned char>(gateType[i]));
        res_oss << gateType << " " << match[2] << " " << match[3] << "\n";
      } else if (regex_match(line, match, rotation_qubit_regex)) {
        std::string gateType = match[1];
        for (size_t i = 0; i < gateType.size(); i++)
          gateType[i] = std::toupper(static_cast<unsigned char>(gateType[i]));
        res_oss << gateType << " " << match[3] << " " << match[2] << "\n";
      }
    }
    return res_oss;
  }
};

inline std::string fusion(ast::ASTNode &node, const xacc::HeterogeneousMap &options) {
  FusionOptimizer opt;
  return opt.run(node, options);
}

} // namespace optimization
} // namespace staq
