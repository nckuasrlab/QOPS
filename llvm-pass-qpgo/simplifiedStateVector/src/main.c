#include "common.h"
#include "gate.h"
#include <omp.h>

int main() {
  IsDensity = 0;
  N = 4;

  gate gates[] = {
      {.gate_ops = GATE_H, .targs = {1}},
      {.gate_ops = GATE_X, .targs = {1}},
      {.gate_ops = GATE_SWAP, .targs = {1, 2}},
      {.gate_ops = GATE_CX, .ctrls = {2}, .targs = {1}},
      {.gate_ops = GATE_T, .targs = {1}},
      {.gate_ops = GATE_X, .targs = {1}},
      {.gate_ops = GATE_H, .targs = {1}},
      {.gate_ops = GATE_X, .targs = {1}},
      {.gate_ops = GATE_H, .targs = {1}},
      {.gate_ops = GATE_X, .targs = {1}},
  };
  total_gate = sizeof(gates) / sizeof(gate);
  gateMap = gates;

  omp_set_num_threads(4);
  run_simulator();
  return 0;
}
