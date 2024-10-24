#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <omp.h>
#include <time.h>
#include "common.h"
#include "gate.h"
#include "measure.h"

unsigned int N;
unsigned int thread_segment;
unsigned int file_segment;
unsigned int para_segment;
unsigned int chunk_segment;
unsigned int max_path;
//unsigned int MAX_QUBIT; // FP16
unsigned int max_depth; // 最多一千的 gates
//char *state_file;
char **state_paths;
int **fd_arr_set; //int fd_arr_set [set][num_file];
int *fd_arr; //int fd_arr [num_file];
int **multi_res;
int *single_res;

ull num_file; // for extending to more devices
ull num_thread; // number of thread
ull half_num_thread;
ull quarter_num_thread;
ull eighth_num_thread;
ull num_thread_per_file;  // applying threads to files
ull file_state; // number of states in single file
ull thread_state; // number of states in single thread
ull file_size;
ull thread_size;
ull chunk_state; // unit: Type
ull chunk_size;
ull buffer_size; // buffer_size for each thread

Type *q_read;
int IsDensity;
int SkipInithread_state;
int SetOfSaveState;

inline int file_exists(char *filename) {
    struct stat buffer;
    return (stat (filename, &buffer) == 0);
}

inline int mk_dir(char *dir) {
    DIR *mydir = NULL;
    if((mydir = opendir(dir)) == NULL) { // check is dir or not
        int ret = mkdir(dir,(S_IRWXU | S_IRWXG | S_IRWXO));
        if (ret != 0)
            return -1;
        printf("[DIR]: %s created sucess! \n", dir);
    }
    else
        printf("[DIR]: %s exist! \n", dir);
    closedir(mydir);
    return 0;
}

void run_simulator (){
    // printf("N = %d, STREAM = %d, chunk_state = %d\n", N, STREAM, chunk_state);

    // call gates
    srand(time(NULL));
    #pragma omp parallel
    {
        int t = omp_get_thread_num();

        for (int i = 0; i < total_gate; i++){
            gate *g = gateMap+i;
            real = g->real_matrix;
            imag = g->imag_matrix;

            switch(g->gate_ops){
                case 0: // H
                case 1: // S
                case 2: // T
                case 3: // X
                case 4: // Y
                case 5: // Z
                case 6: // Phase
                case 7: // Unitary 1-qubit gate
                    single_gate (g->targs[0]+N/2*IsDensity, g->gate_ops, 0);
                    break;

                case 8:     // CX
                case 9:     // CY
                case 10:    // CZ
                case 11:    // CPhase
                case 12:    // Control-Unitary 1-qubit gate
                    control_gate(g->ctrls[0]+N/2*IsDensity, g->targs[0]+N/2*IsDensity, g->gate_ops, 0);
                    break;

                case 13:
                    SWAP (g->targs[0]+N/2*IsDensity, g->targs[1]+N/2*IsDensity, 0);
                    break;
                
                // case 14:
                //     Toffoli (g.ctrls[0], g.ctrls[1], g.targs[0], q_read, q_write, fd_1, fd_2, fd_arr); 
                //     break;

                case 14:
                    break;

                case 20: // measure one qubit
                    if(t==0){
                        single_res = (int*)malloc(g->ctrls[2]*sizeof(int));
                    }
                    for(int shot = 0; shot < g->ctrls[2]; shot++){
                        save_state(g->ctrls[0], g->ctrls[1]);
                #pragma omp barrier

                        measure(g->targs[0], g->ctrls[1]);
                #pragma omp barrier

                        if(t==0) single_res[shot] = real[1];
                    }

                    if(t == 0){
                        for(int shot = 0; shot < g->ctrls[2]; shot++){
                            printf("[MEASURE]: %d\n", single_res[shot]);
                        }
                        fflush(stdout);
                        free(single_res);
                    }

                    break;

                case 21: // measure multi qubits
                    if(t==0){
                        multi_res = (int**)malloc(g->ctrls[2]*sizeof(int*));
                        for(int shot = 0; shot < g->ctrls[2]; shot++){
                            multi_res[shot] = (int*)malloc(g->val_num*sizeof(int));
                        }
                    }
                    for(int shot = 0; shot < g->ctrls[2]; shot++){
                        save_state(g->ctrls[0], g->ctrls[1]);
                #pragma omp barrier

                        for(int q = 0; q < g->val_num; q++){
                            measure((int)(g->imag_matrix[q]), g->ctrls[1]);
                #pragma omp barrier

                            if(t==0) multi_res[shot][q] = real[1];
                        }
                    }

                    if(t == 0){
                        for(int shot = 0; shot < g->ctrls[2]; shot++){
                            printf("[MEASURE]: ");
                            for(int q = 0; q < g->val_num; q++){
                                printf("%d", multi_res[shot][q]);
                            }
                            printf("\n");
                        }
                        fflush(stdout);
                        for(int shot = 0; shot < g->ctrls[2]; shot++){
                            free(multi_res[shot]);
                        }
                        free(multi_res);
                    }

                    break;
                
                case 22: // copy
                    save_state(g->ctrls[0], g->ctrls[1]);
                    break;

                case 31: // Unitary 2-qubit gate
                    unitary4x4 (g->targs[0]+N/2*IsDensity, g->targs[1]+N/2*IsDensity, 0);
                    break;

                case 32: // Unitary 3-qubit gate
                    unitary8x8 (g->targs[0]+N/2*IsDensity, g->targs[1]+N/2*IsDensity, g->targs[2]+N/2*IsDensity, 0);
                    break;

                default:
                    printf("no such gate.\n");
                    exit(1);
            }

            #pragma omp barrier
            
            if(IsDensity){
                switch(g->gate_ops){
                    case 0: // H
                    case 1: // S
                    case 2: // T
                    case 3: // X
                    case 4: // Y
                    case 5: // Z
                    case 6: // Phase
                    case 7: // Unitary 1-qubit gate
                        single_gate(g->targs[0], g->gate_ops, 1);
                        break;

                    case 8:     // CX
                    case 9:     // CY
                    case 10:    // CZ
                    case 11:    // CPhase
                    case 12:    // Control-Unitary 1-qubit gate
                        control_gate(g->ctrls[0], g->targs[0], g->gate_ops, 1);
                        break;

                    case 13:
                        SWAP (g->targs[0], g->targs[1], 1);
                        break;

                    // case 14:
                    //     Toffoli (g.ctrls[0], g.ctrls[1], g.targs[0], q_read, q_write, fd_1, fd_2, fd_arr);
                    //     break;

                    case 14:
                        break;

                    case 31: // Unitary 2-qubit gate
                        unitary4x4 (g->targs[0], g->targs[1], 1);
                        break;

                    case 32: // Unitary 3-qubit gate
                        unitary8x8 (g->targs[0], g->targs[1], g->targs[2], 0);
                        break;

                    default:
                        printf("no such gate.\n");
                        exit(1);
                }
                #pragma omp barrier
            }

        }
    }
}
