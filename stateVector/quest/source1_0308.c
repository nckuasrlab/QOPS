// revise from QuEST demo

#include "QuEST.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define strtN 21
#define N 31 // 32 kill

#define MEASURET_START \
        struct  timeval start; \
        struct  timeval end; \
        unsigned  long diff; \
        gettimeofday(&start,NULL);
#define MEASURET_END(s) \
        gettimeofday(&end,NULL); \
        diff = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec; \
        printf(s); \
        printf("%ld (us)\n", diff); \



int main (int narg, char *varg[]) {
    
 //    // single qubit gate testing
 // 	for (int n = strtN; n < N; n++){

	//     QuESTEnv env = createQuESTEnv();

	//     Qureg qubits = createQureg(n, env);
	//     initZeroState(qubits);
	//     printf("testing qubit number = %d\n", n);

	//     MEASURET_START;
	//     for (int i = 0; i < n; i++){
	//         // hadamard(qubits, i);
	//         // pauliX(qubits, i);
	//     }
	//     MEASURET_END("Total: ");


	//     qreal prob;
	//     prob = getProbAmp(qubits, 0);
	//     printf("Probability amplitude of |000...0>: %g\n", prob);
	//     printf("\n\n");

	//     destroyQureg(qubits, env); 
 //    	destroyQuESTEnv(env);
	// }

    // CNOT qubit gate testing
 	for (int n = strtN; n < N; n++){

	    QuESTEnv env = createQuESTEnv();

	    Qureg qubits = createQureg(n, env);
	    initZeroState(qubits);
	    printf("testing qubit number = %d\n", n);

	    MEASURET_START;
	    for (int i = 0; i < n; i++){
	    	for (int j = 0; j < i; j++){
	    		// printf("con = %d, tar = %d\n", i, j);
		    	controlledNot(qubits, i, j);
	    	}
	    }
	    MEASURET_END("Total: ");


	    qreal prob;
	    prob = getProbAmp(qubits, 0);
	    printf("Probability amplitude of |000...0>: %g\n", prob);
	    printf("\n\n");

	    destroyQureg(qubits, env); 
    	destroyQuESTEnv(env);
	}

    return 0;
}


// int main (int narg, char *varg[]) {
//     // * PREPARE QuEST environment
//     QuESTEnv env = createQuESTEnv();

//     /*
//      * PREPARE QUBIT SYSTEM
//      */

//     Qureg qubits = createQureg(N, env);
//     initZeroState(qubits);
//     printf("testing qubit number = %d\n", N);

//     /*
//      * REPORT SYSTEM AND ENVIRONMENT
//      */
//     // printf("\nThis is our environment:\n");
//     // reportQuregParams(qubits);
//     // reportQuESTEnv(env);



//     /*
//      * APPLY CIRCUIT
//      */

//     // printf("check single qubit gate cases\n");
//     MEASURET_START;
//     for (int i = 0; i < N; i++){
//         // single_gate ('H', i, q_read, q_write, fd_1, fd_2, fd_arr);
//         hadamard(qubits, i);
//     }
//     MEASURET_END("Total: ");


//     // controlledNot(qubits, 0, 1);
//     // rotateY(qubits, 2, .1);

//     // int targs[] = {0,1,2};
//     // multiControlledPhaseFlip(qubits, targs, 3);

//     // ComplexMatrix2 u = {
//     //     .real={{.5,.5},{.5,.5}},
//     //     .imag={{.5,-.5},{-.5,.5}}
//     // };
//     // unitary(qubits, 0, u);

//     // Complex a, b;
//     // a.real = .5; a.imag =  .5;
//     // b.real = .5; b.imag = -.5;
//     // compactUnitary(qubits, 1, a, b);

//     // Vector v;
//     // v.x = 1; v.y = 0; v.z = 0;
//     // rotateAroundAxis(qubits, 2, 3.14/2, v);

//     // controlledCompactUnitary(qubits, 0, 1, a, b);

//     // int ctrls[] = {0,1};
//     // multiControlledUnitary(qubits, ctrls, 2, 2, u);
    
//     // ComplexMatrixN toff = createComplexMatrixN(3);
//     // toff.real[6][7] = 1;
//     // toff.real[7][6] = 1;
//     // for (int i=0; i<6; i++)
//     //     toff.real[i][i] = 1;
//     // multiQubitUnitary(qubits, targs, 3, toff);
    
//     printf("\nCircuit output:\n");

//     qreal prob;
//     prob = getProbAmp(qubits, 0);
//     printf("Probability amplitude of |000...0>: %g\n", prob);

//     // prob = calcProbOfOutcome(qubits, 2, 1);
//     // printf("Probability of qubit 2 being in state 1: %g\n", prob);

//     // int outcome = measure(qubits, 0);
//     // printf("Qubit 0 was measured in state %d\n", outcome);

//     // outcome = measureWithStats(qubits, 2, &prob);
//     // printf("Qubit 2 collapsed to %d with probability %g\n", outcome, prob);



//     /*
//      * FREE MEMORY
//      */
//     destroyQureg(qubits, env); 
//     // destroyComplexMatrixN(toff);


//     /*
//      * CLOSE QUEST ENVIRONMET
//      * (Required once at end of program)
//      */
//     destroyQuESTEnv(env);
//     return 0;
// }

