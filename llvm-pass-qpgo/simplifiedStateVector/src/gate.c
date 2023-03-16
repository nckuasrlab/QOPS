#include "gate.h"
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

gate *gateMap;
unsigned int total_gate;

void single_gate(int targ, int ops, int density) {

  int t = omp_get_thread_num();
  if (t == 0) {
    usleep(100000);
    printf("%d\n", ops);
  }
}
void control_gate(int ctrl, int targ, int ops, int density) {
  int t = omp_get_thread_num();
  if (t == 0) {
    usleep(200000);
    printf("%d\n", ops);
  }
}
void SWAP(int q0, int q1, int density) {
  int t = omp_get_thread_num();
  if (t == 0) {
    usleep(300000);
    printf("%d\n", GATE_SWAP);
  }
}
