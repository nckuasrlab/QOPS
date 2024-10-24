#ifndef INIT_H_
#define INIT_H_
#include <stdio.h>

extern unsigned long long thread_state;
extern int *fd_pair;
extern int *td_pair;
void set_buffer();
void set_ini(char *path);
void set_circuit (char *path);
void set_gates(FILE *circuit);
void set_qubitTimes();
void set_state_files();
void set_all(char *ini, char *cir);
int read_args(int argc, char *argv[], char **ini, char **cir);

#endif