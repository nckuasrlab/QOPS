#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ini.h"
#include "init.h"
#include "common.h"
#include "gate.h"

inline void set_buffer() {
    q_read = (Type*) malloc(buffer_size);
    memset((void *) (q_read),  0.0, buffer_size);
    thread_settings = (setStreamv2*)malloc(num_thread*sizeof(setStreamv2));
    for (int i = 0; i < num_thread; i++){
        thread_settings[i].rd = (void*)q_read + i * 2*chunk_size;
    }
}

inline void set_ini(char *path) {
    // read from .ini file
    const char *section = "system";
    N = read_profile_int(section, "total_qbit", 0, path);
    file_segment = read_profile_int(section, "file_qbit", 0, path);
    thread_segment = file_segment;
    chunk_segment = read_profile_int(section, "local_qbit", 0, path);
    max_path = read_profile_int(section, "max_path", 260, path);
    //MAX_QUBIT = read_profile_int(section, "max_qbit", 38, path);
    max_depth = read_profile_int(section, "max_depth", 1000, path);
    IsDensity =  read_profile_int(section, "is_density", 0, path);
    SkipInithread_state = read_profile_int(section, "skip_init_state", 0, path);
    SetOfSaveState = read_profile_int(section, "set_of_save_state", 1, path);
    num_file = (1ULL << file_segment);
    num_thread = (1ULL << thread_segment);
    half_num_thread = (1ULL << (thread_segment-1));
    quarter_num_thread = (1ULL << (thread_segment-2));
    eighth_num_thread = (1ULL << (thread_segment-3));
    file_state = (1ULL << (N-file_segment));
    thread_state = (1ULL << (N-thread_segment));
    chunk_state = (1ULL << chunk_segment);
    file_size = file_state * sizeof(Type);
    thread_size = thread_state * sizeof(Type);
    chunk_size = chunk_state*sizeof(Type);
    buffer_size = (num_thread*8*chunk_size);

    printf("is density: %d\n", IsDensity);
    if(IsDensity)
        N = 2*N;
    
    char *state_form_ini = (char *) malloc(SetOfSaveState*num_file*max_path*sizeof(char));
    state_paths = (char **) malloc(SetOfSaveState*num_file*sizeof(char*));
    for(int i = 0; i < SetOfSaveState*num_file; i++)
        state_paths[i] = (char *) malloc(max_path*sizeof(char));
    read_profile_string(section, "state_paths", state_form_ini,
        SetOfSaveState*num_file*max_path, "", path);

    char *token = strtok(state_form_ini, ",");
    int cnt = 0;
    while(token != NULL) {
        //printf("Token no. %d : %s, len: %lu \n", cnt, token, strlen(token));
	    //printf("Dir name: %s \n", dirname(token));
        if(cnt < SetOfSaveState*num_file)
            strcpy(state_paths[cnt], token);
	    token = strtok(NULL, ",");
        cnt++;
    }
    // assert for invalid case
    assert (N >= (file_segment + chunk_segment));
    free(token);
    return;
}

inline void set_circuit(char *path) {
    FILE *circuit;
    if((circuit = fopen(path, "r"))== NULL) {
        printf("no circuit file.\n");
        exit(1);
    }
    if(fscanf(circuit, "%d", &total_gate));
    assert(total_gate < max_depth);

    set_gates(circuit);
    // set_qubitTimes();

    fclose(circuit);
}

inline void set_gates(FILE *circuit) {
    gateMap = (gate*) malloc(total_gate*sizeof(gate));
    for (int i = 0; i < total_gate; i++)
        gateMap[i].action = 0; // default is not to execute the gate
    for (int i = 0; i < total_gate; i++) {
        gate g;
        if(fscanf(circuit, "%d%d%d%d",
            &g.gate_ops, &g.numCtrls, &g.numTargs, &g.val_num));
        for (int j = 0; j < g.numCtrls; j++)
            if(fscanf(circuit, "%d", &g.ctrls[j]));
        for (int j = 0; j < g.numTargs; j++)
            if(fscanf(circuit, "%d", &g.targs[j]));

        g.real_matrix = (Type_t*) malloc(g.val_num*sizeof(Type_t));
        g.imag_matrix = (Type_t*) malloc(g.val_num*sizeof(Type_t));
        for (int j = 0; j < g.val_num; j++)
            if(fscanf(circuit, "%lf", &g.real_matrix[j]));
        for (int j = 0; j < g.val_num; j++)
            if(fscanf(circuit, "%lf", &g.imag_matrix[j]));

        g.action = 1;
        gateMap[i] = g; // add the gate to the gateMap
        // print_gate(&g); //print the gate
    }
}

void set_state_files() {
    // fd_arr malloc
    fd_arr_set = (int**) malloc(SetOfSaveState*sizeof(int*));
    for (int i = 0; i <= SetOfSaveState; i++){
        fd_arr_set[i] = (int*) malloc(num_file*sizeof(int));
    }

    fd_arr = fd_arr_set[0];

    // create the dir of the output path and touch them
    if(SkipInithread_state){
        for(int i = 0; i < SetOfSaveState*num_file; i++) {
            if (!file_exists(state_paths[i])){
                printf("[FILE]: %s skip init but not exists.\n", state_paths[i]);
                exit(-1);
            }
            fd_arr_set[i/num_file][i%num_file] = open(state_paths[i], O_RDWR);
            assert(fd_arr_set[i/num_file][i%num_file] > 0);
            printf("[FILE]: previous state %s open success!, fd: %2d \n", state_paths[i], fd_arr_set[i/num_file][i%num_file]);
            lseek(fd_arr_set[i/num_file][i%num_file], 0, SEEK_SET);
        }
        return;
    }

    char *state_dir = (char *) malloc(max_path*sizeof(char));
    for(int i = 0; i < SetOfSaveState*num_file; i++) {
        strcpy(state_dir, state_paths[i]);
        int md = mk_dir(dirname(state_dir));
        if (md != -1) {
            if (file_exists(state_paths[i]))
                remove(state_paths[i]);
            fd_arr_set[i/num_file][i%num_file] = open(state_paths[i], O_RDWR|O_CREAT, 0777);
            assert(fd_arr_set[i/num_file][i%num_file] > 0);
            printf("[FILE]: %s create success!, fd: %2d \n", state_paths[i], fd_arr_set[i/num_file][i%num_file]);
            lseek(fd_arr_set[i/num_file][i%num_file], 0, SEEK_SET);
        }
    }
    free(state_dir);

    // init files: reset all strings in file as 0
    // use "od -tfD [file_name] to verify"
    #pragma omp parallel for num_threads(num_thread) schedule(static, 1)
    for (int t = 0; t < num_thread; t++) {
        int f = t;
        int td = 0;
        for(int set = 0; set < SetOfSaveState; set++){
            ull fd = fd_arr_set[set][f];
            void *wr = thread_settings[t].rd;
            ull fd_off = td * thread_size;
            for (ull sz = 0; sz < thread_state; sz += chunk_state) {
                pwrite(fd, wr, chunk_size, fd_off);
                fd_off += chunk_size;
            }

            if (t == 0) {
                q_read[0].real = 1.;
                q_read[0].imag = 0.;
                pwrite(fd, q_read, sizeof(Type), 0);
            }
        }
    }
    fflush(stdout);
}

void set_all(char *ini, char *cir) {
    set_ini(ini);
    set_circuit(cir);
    set_buffer();
    set_state_files();
}

inline int read_args(int argc, char* argv[], char **ini, char **cir) {
    int cmd_opt = 0;
    int ret_val = 0;
    size_t destination_size;
    //fprintf(stderr, "argc:%d\n", argc);
    while(1) {
        //fprintf(stderr, "proces index:%d\n", optind);
        cmd_opt = getopt(argc, argv, "vc:i:");
        /* End condition always first */
        if (cmd_opt == -1) {
            break;
        }
        /* Print option when it is valid */
        //if (cmd_opt != '?') {
        //    fprintf(stderr, "option:-%c\n", cmd_opt);
        //}
        /* Lets parse */
        switch (cmd_opt) {
            case 'v':
                break;
            /* Single arg */
            case 'c':
                destination_size = strlen(optarg);
                *cir = (char *) malloc(sizeof(char) * destination_size);
                strncpy(*cir, optarg, destination_size);
                (*cir)[destination_size] = '\0';
                ++ret_val;
                break;
            case 'i':
                // fprintf(stderr, "option arg:%s\n", optarg);
                destination_size = strlen(optarg);
                *ini = (char *) malloc(sizeof(char) * destination_size);
                strncpy(*ini, optarg, destination_size);
                (*ini)[destination_size] = '\0';
                ++ret_val;
                break;
            /* Error handle: Mainly missing arg or illegal option */
            case '?':
                fprintf(stderr, "Illegal option:-%c\n", isprint(optopt)?optopt:'#');
                ret_val = 0;
                break;
            default:
                fprintf(stderr, "Not supported option\n");
                break;
        }
    }
    /* Do we have args? */
    if (argc > optind) {
        int i = 0;
        for (i = optind; i < argc; i++) {
            fprintf(stderr, "argv[%d] = %s\n", i, argv[i]);
        }
        ret_val = 0;
    }
    return ret_val;
}