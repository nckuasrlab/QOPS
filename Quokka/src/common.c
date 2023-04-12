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

unsigned int N;
unsigned int thread_segment;
unsigned int file_segment;
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
    // call gates
    srand(time(NULL));
    #pragma omp parallel
    {
        int t = omp_get_thread_num();

        for (int i = 0; i < total_gate; i++){
            gate *g = gateMap+i;

            switch(g->gate_ops){
                case 0: // H gate
                    H_gate (g->targs[0]);
                    break;

                default:
                    printf("no such gate.\n");
                    exit(1);
            }

            #pragma omp barrier
            
            if(IsDensity){
                switch(g->gate_ops){
                    case 0: // H gate
                        H_gate(g->targs[0] + N/2);
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
