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

#define strtN 25
#define N 32// 32 kill
#define PI 3.14159265

#define MEASURET_START \
        struct  timeval start; \
        struct  timeval end; \
        unsigned  long diff; \
        gettimeofday(&start,NULL);
#define MEASURET_END(s) \
        gettimeofday(&end,NULL); \
        diff = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec; \
        printf(s); \
        printf("%ld (us)", diff); \



int main (int narg, char *varg[]) {
 
 //    // single qubit gate testing
  // 	for (int n = strtN; n < N; n++){

	 //     QuESTEnv env = createQuESTEnv();

	 //     Qureg qubits = createQureg(n, env);
	 //     initZeroState(qubits);
	 //     printf("testing qubit number = %d\n", n);

	 //     MEASURET_START;
	 //     for (int i = 0; i < n; i++){
  //           MEASURET_START;
		//     hadamard(qubits, i);      
	 //        // pauliX(qubits, i);
		//     printf("\nH gate on qubit %d: ", i);
		//     MEASURET_END(" ");
	 //     }
	 //     MEASURET_END("\nTotal: ");
	     

	 //     qreal prob;
	 //     prob = getProbAmp(qubits, 0);
	 //     printf("\nProbability amplitude of |000...0>: %g\n", prob);
	 //     printf("\n\n");

	 //     destroyQureg(qubits, env); 
  //    	destroyQuESTEnv(env);
	 // }


//  // // CNOT qubit gate testing
 	for (int n = strtN; n < N; n++){

	    QuESTEnv env = createQuESTEnv();

	    Qureg qubits = createQureg(n, env);
	    initZeroState(qubits);
	    printf("testing qubit number = %d\n", n);
	    int a = 0;
	    int control[2], target[1];

	    MEASURET_START;
	    for (int i = 0; i < n; i++){
	    	for (int j = i; j < n; j++){
			    if (i != j ){
			    	control[0] = i, control[1] = j, target[0] = j+1;
			    	// controlledNot(qubits, i, j); // CX
				    // controlledPhaseFlip(qubits, i, j); // CZ, 跑 fin
				    // controlledPhaseShift(qubits, i, j, PI/4);   // CP, 跑 fin
				    swapGate(qubits, 0, 1); // swap , 跑 fin
				    // multiControlledMultiQubitNot(qubits, control, 2, target, 1);
		    		a++;
			    }
			}
	    }
	 	
	 	printf("a = %d\n", a);
	    MEASURET_END("Total: ");
	    printf("\n");


	    // qreal prob;
	    // prob = getProbAmp(qubits, 0);
	    // printf("Probability amplitude of |000...0>: %g\n", prob);
	    // printf("\n\n");

	    destroyQureg(qubits, env); 
    	destroyQuESTEnv(env);
	}

// // CNOT 清理 cache
 // 	for (int n = strtN; n < N; n++){

	//     QuESTEnv env = createQuESTEnv();

	//     Qureg qubits = createQureg(n, env);
	//     initZeroState(qubits);
	//     printf("testing qubit number = %d\n", n);

	//     MEASURET_START;
 //    	controlledNot(qubits, 1, 2);


	//     MEASURET_END("Total: ");

	//     destroyQureg(qubits, env); 
 //    	destroyQuESTEnv(env);
	// 	printf("\n");
	// }


// three qutbi gate
 //    controlledNot(qubits, 0, 1);
 //    control[0] = 0, control[1] = 1, target[0] = 2;
	// multiControlledMultiQubitNot(qubits, control, 2, target, 1);    


 //    QuESTEnv env = createQuESTEnv();
 //    Qureg qubits = createQureg(N, env);
 //    initZeroState(qubits);
	// int control [2]; 
	// int target [1];
	// int a = 0;

 //    printf("measure #qubit = %d\n", N);

 //    hadamard(qubits, 0);
 //    hadamard(qubits, 1);
 //    hadamard(qubits, 2);    	
 //    MEASURET_START;
 //    for (int c2 = 0; c2 < N; c2++){
 //        for (int c1 = c2-1; c1 >= 0; c1--){
 //            for (int t1 = 0; t1 < N; t1 ++) {            
 //                if ((c1 != t1) && (c2 != t1) && a < 1000){
	// 				control[0] = c1, control[1] = c2, target[0] = t1;
	// 			    multiControlledMultiQubitNot(qubits, control, 2, target, 1);
	// 			    a++;            
 //            	}
 //            }
 //        }
 //    }
 //    printf("Final conduct %d gates\n", a);
 //    MEASURET_END("Total: ");
 //    printf("\n");
 //    destroyQureg(qubits, env); 
	// destroyQuESTEnv(env);
 //    // }

    // printf("\n");

    // qreal prob;
    // prob = getProbAmp(qubits, 0);
    // printf("Probability amplitude of |000...0>: %g\n", prob);
    // printf("\n\n");

// // // // toffoli 清理 cache
// 	for (int n = strtN; n < N; n++){
// 	    QuESTEnv env = createQuESTEnv();
// 	    Qureg qubits = createQureg(n, env);
// 	    initZeroState(qubits);
// 		int control [2]; 
// 		int target [1];
// 		int a = 0;

// 		control[0] = 5, control[1] = 2, target[0] = 1;
// 	    printf("measure #qubit = %d\n", n);

// 	    MEASURET_START;
// 	    multiControlledMultiQubitNot(qubits, control, 2, target, 1);
// 	    MEASURET_END("Total: ");



// 	    printf("\n");
// 	    destroyQureg(qubits, env); 
// 		destroyQuESTEnv(env);


// 	}


// multiQubitUnitary
 //    QuESTEnv env = createQuESTEnv();
 //    Qureg qubits = createQureg(N, env);
 //    initZeroState(qubits);
	// int target [3];
	// int a = 0;


    // ComplexMatrixN toff = createComplexMatrixN(3);
    // toff.real[6][7] = 1;
    // toff.real[7][6] = 1;
    // for (int i=0; i<6; i++)
    //     toff.real[i][i] = 1;

 //    printf("measure #qubit = %d\n", N);

 //    MEASURET_START;
 //    // for (int c2 = 0; c2 < N; c2++){
 //    //     for (int c1 = c2-1; c1 >= 0; c1--){
 //    //         for (int t1 = 0; t1 < N; t1 ++) {            
 //    //             if ((c1 != t1) && (c2 != t1) && a < 1000){
	// 				target[0] = 11, target[1] = 6, target[2] = 1;
	// 			    multiQubitUnitary(qubits, target, 3, toff);



	// 			//     a++;            
 //    //         	}
 //    //         }
 //    //     }
 //    // }
 //    printf("Final conduct %d gates\n", a);
 //    MEASURET_END("Total: ");
 //    printf("\n");
 //    destroyQureg(qubits, env); 
	// destroyQuESTEnv(env);
 //    // }


	// for (int n = strtN; n < N; n++){
	//     QuESTEnv env = createQuESTEnv();
	//     Qureg qubits = createQureg(n, env);
	//     initZeroState(qubits);	
	//     int target [3];
	//     target[0] = 5, target[1] = 2, target[2] = 1;

	//     MEASURET_START;
	//     multiQubitUnitary(qubits, target, 3, toff);
	//     MEASURET_END("Total: ");

	//     printf("\n");
	//     destroyQureg(qubits, env); 
	// 	destroyQuESTEnv(env);

	// }
    // printf("\n");

    // 目前是 four unitary
 //    for (int n = strtN; n < N; n++){

	//     QuESTEnv env = createQuESTEnv();
	//     Qureg qubits = createQureg(n, env);
	//     initZeroState(qubits);
	// 	int target [4];

	//     ComplexMatrixN toff = createComplexMatrixN(4);
	//     toff.real[14][15] = 1;
	//     toff.real[15][14] = 1;
	//     for (int i=0; i<14; i++)
	//         toff.real[i][i] = 1;

	//     printf("measure #qubit = %d\n", n);

	//     MEASURET_START;

	// 	target[0] = 0, target[1] = 1, target[2] = 2, target[3] = 3;
	//     multiQubitUnitary(qubits, target, 4, toff);
 //        MEASURET_END("Total: ");
 //        printf("\n");
 //    	destroyQureg(qubits, env); 
	// 	destroyQuESTEnv(env);

	// }

 //    // }

 //    printf("\n");




// // // // // // twoQubitUnitary

	// ComplexMatrix4 m = {
	//      .real = {{1, 0, 0, 0},
	//               {0, 1, 0, 0},
	//               {0, 0, 0, 1},
	//               {0, 0, 1, 0}},
	//      .imag = {{0},{0},{0},{0}}};

 // 	for (int n = strtN; n < N; n++){

	//     QuESTEnv env = createQuESTEnv();

	//     Qureg qubits = createQureg(n, env);
	//     initZeroState(qubits);
	//     printf("testing qubit number = %d\n", n);

	//     MEASURET_START;
	//     // for (int i = 0; i < n; i++){
	//     // 	for (int j = 0; j < i; j++){
	// 	    	// twoQubitUnitary(qubits, 0, 1, m);
	// 	    	twoQubitUnitary(qubits, 0, 1, m);
	// 		// }
	//   //   }
	//     MEASURET_END("Total: ");
	//  printf("\n");

	//     destroyQureg(qubits, env); 
 //    	destroyQuESTEnv(env);
	// }



// // // /// // unitary
	// ComplexMatrix2 m2 = {
	//     .real = {{1,0}, 
	//              {0,1}},
	//     .imag = {{0,0}, 
	//              {0, 0}}};

 //  	for (int n = strtN; n < N; n++){

	// 	// QuESTEnv env = createQuESTEnv();
	// 	// Qureg qubits = createQureg(n, env);
	// 	// initZeroState(qubits);

	// 	// MEASURET_START;
	// 	// // for (int i = 0; i < n; i++){
	// 	// // unitary(qubits, i, m2);
	// 	// // }
	// 	// unitary(qubits, 0, m2);
	// 	// MEASURET_END("\nTotal: ");
	// 	// printf("\n");


	// 	// destroyQureg(qubits, env); 
	// 	// destroyQuESTEnv(env);


	// 	QuESTEnv env1 = createQuESTEnv();
	// 	Qureg qqq = createQureg(n, env1);
	// 	initZeroState(qqq);

	//     printf("testing qubit number = %d\n", n);

	// 	MEASURET_START;
	// 	unitary(qqq, 0, m2);
	// 	MEASURET_END("\nTotal: ");


	// 	destroyQureg(qqq, env1); 
	// 	destroyQuESTEnv(env1);
	//  }
	// printf("\n");




 //    //randsom
 //    srand( time(NULL) );
	// int min = 0;
	// int max = 13;

	// int q1 = 0, q2 = 0, q3 = 0;
	// int qmax =  19;
	// int qmin = 0;
	// int apply [100] = {0};
	// for (int i = 0; i < 100; i++){
	// 	apply[i] = rand() % (max - min + 1) + min;
	// }

	// int q[100][3];
	// for (int i = 0; i < 100; i++){
	// 	q1 = rand() % (qmax - qmin + 1) + qmin;
	// 	q2 = rand() % (qmax - qmin + 1) + qmin;
	// 	q3 = rand() % (qmax - qmin + 1) + qmin;

	// 	if ((q1 == q2) || (q3 == q2) || (q3 == q1)){
	// 		q1 = rand() % (qmax - qmin + 1) + qmin;
	// 		q2 = rand() % (qmax - qmin + 1) + qmin;
	// 		q3 = rand() % (qmax - qmin + 1) + qmin;	
	// 	}

	// 	q[i][0] = q1;
	// 	q[i][1] = q2;
	// 	q[i][2] = q3;
	// }


	// random 在外面做準備


	// QuESTEnv env = createQuESTEnv();
	// Qureg qubits = createQureg(N, env);
	// initZeroState(qubits);

	// MEASURET_START;
 //    for (int dep = 0; dep < 100; dep++){

	//     if (apply[dep] == 0) {
	// 		ComplexMatrix2 m2 = {
	// 		    .real = {{1,0}, 
	// 		             {0,1}},
	// 		    .imag = {{0,0}, 
	// 		             {0, 0}}};

	// 		unitary(qubits, q[dep][0], m2);


	//     } else if (apply[dep] == 1){
	// 		ComplexMatrix4 m = {
	// 		     .real = {{1, 0, 0, 0},
	// 		              {0, 1, 0, 0},
	// 		              {0, 0, 0, 1},
	// 		              {0, 0, 1, 0}},
	// 		     .imag = {{0},{0},{0},{0}}};

	//     	twoQubitUnitary(qubits, q[dep][0], q[dep][1], m);

	//     }  else if (apply[dep] == 2){
	//     	int target[3];
	// 		ComplexMatrixN toff = createComplexMatrixN(3);
	// 		toff.real[6][7] = 1;
	// 		toff.real[7][6] = 1;
	// 		for (int i=0; i<6; i++)
	// 		    toff.real[i][i] = 1;

	// 		target[0] = q[dep][0], target[1] = q[dep][1], target[2] = q[dep][2];
	// 		multiQubitUnitary(qubits, target, 3, toff);


	//     }  else if (apply[dep] == 3){
	// 		hadamard(qubits, q[dep][0]);
	    	
	//     }  else if (apply[dep] == 4){
	// 		sGate(qubits, q[dep][0]);	    	

	//     }  else if (apply[dep] == 5){
	// 		tGate(qubits, q[dep][0]);		 

	//     } else if (apply[dep] == 6){
	// 		pauliX(qubits, q[dep][0]);	

	//     } else if (apply[dep] == 7){
	// 		pauliY(qubits, q[dep][0]);	    	
	    	
	//     } else if (apply[dep] == 8){
	// 		pauliX(qubits, q[dep][0]);	    	
	    	
	//     } else if (apply[dep] == 9){
	//     	controlledNot(qubits, q[dep][0], q[dep][1]);

	//     } else if (apply[dep] == 10){
	//     	controlledPhaseFlip(qubits, q[dep][0], q[dep][1]);    

	//     } else if (apply[dep] == 11){
	//     	controlledPhaseShift(qubits, q[dep][0], q[dep][1], PI/4);

	//     } else if (apply[dep] == 12){
	//     	swapGate(qubits, q[dep][0], q[dep][1]);

	//     } else if (apply[dep] == 13){
	// 		int control [2]; 
	// 		int target [1];
	// 		control[0] = q[dep][0];
	// 		control[1]  = q[dep][1];
	// 		target[0]  = q[dep][2];
	//     	multiControlledMultiQubitNot(qubits, control, 2, target, 1);  
	//     }
 //    }
    
 //    MEASURET_END("\nTotal: ");
	// destroyQureg(qubits, env); 
	// destroyQuESTEnv(env);

	// printf("\n");

    return 0;

}


