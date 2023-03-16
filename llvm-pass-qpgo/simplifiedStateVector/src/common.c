#include "common.h"
#include "gate.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern unsigned int total_gate;
extern gate *gateMap;
int IsDensity;
unsigned int N; // total_qubit

void run_simulator() {
  // printf("N = %d, STREAM = %d, chunk_state = %d\n", N, STREAM, chunk_state);

  // call gates
  srand(time(NULL));
#pragma omp parallel
  {
    int t = omp_get_thread_num();

    for (int i = 0; i < total_gate; i++) {
      gate *g = gateMap + i;
      // Type_t *real = g->real_matrix;
      // Type_t *imag = g->imag_matrix;

      switch (g->gate_ops) {
      case GATE_H:     // H
      case GATE_S:     // S
      case GATE_T:     // T
      case GATE_X:     // X
      case GATE_Y:     // Y
      case GATE_Z:     // Z
      case GATE_Phase: // Phase
      case GATE_U1:    // Unitary 1-qubit gate
        single_gate(g->targs[0] + N / 2 * IsDensity, g->gate_ops, 0);
        break;

      case GATE_CX:     // CX
      case GATE_CY:     // CY
      case GATE_CZ:     // CZ
      case GATE_CPhase: // CPhase
      case GATE_CU1:    // Control-Unitary 1-qubit gate
        control_gate(g->ctrls[0] + N / 2 * IsDensity,
                     g->targs[0] + N / 2 * IsDensity, g->gate_ops, 0);
        break;

      case GATE_SWAP:
        SWAP(g->targs[0] + N / 2 * IsDensity, g->targs[1] + N / 2 * IsDensity,
             0);
        break;
      default:
        break;
      }

#pragma omp barrier
      // printf("%d barrier\n", t);
      // #pragma omp barrier
    }
  }
}
