#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "init.h"
#include "common.h"
#include "gate.h"

int main(int argc, char *argv[]) {
    char *ini, *cir;
    int ret = read_args(argc, argv, &ini, &cir);
    if(ret == 2)
        printf("[ini]: %s, [cir]: %s\n", ini, cir);
    else{
        printf("Error \n");
        return 0;
    }
    set_all(ini, cir);
    fflush(stdout);

    omp_set_num_threads(num_thread);
    MEASURET_START;
    run_simulator();
    MEASURET_END("Total: ");

    return 0;
}
