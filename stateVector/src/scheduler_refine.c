#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include "simulator.h"

void update_gateMap (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int dim, int eater_gateIdx, int gateIdx, float res_real [64], float res_imag [64]) {
	// // // for preeater_gateIdx, update real_matrix and imag_matrix
	// printf("update_gateMap\n");
	for (int i = 0; i < dim; i++){
		for (int j = 0; j < dim; j++){
			gateMap[eater_gateIdx].real_matrix[i*dim+j] = res_real[i*dim+j];
			gateMap[eater_gateIdx].imag_matrix[i*dim+j] = res_imag[i*dim+j];
		}
	}
	// 一旦你併入了別人，那麼你就會變成 unitary gate
	if (gateMap[eater_gateIdx].val_num == 4){
		gateMap[eater_gateIdx].gate_which = 0;
		// printf("aaa\n");
	}
	else if (gateMap[eater_gateIdx].val_num == 16){
		gateMap[eater_gateIdx].gate_which = 1;
		// printf("bbb\n");
	}
	else {
		gateMap[eater_gateIdx].gate_which = 2;
		// printf("ccc\n");
	}

	// sorting idx, 保證 index 永遠都是小到大
	if (dim == 2){
		gateMap[eater_gateIdx].targs[0] = gateMap[gateIdx].targs[0];
	} else if (dim == 4) { // single 倍 two 吃掉
		int small_idx, large_idx;
		if (gateMap[eater_gateIdx].numCtrls != 0){
			small_idx = gateMap[eater_gateIdx].ctrls[0];
			large_idx = gateMap[eater_gateIdx].targs[0];
			if (small_idx > large_idx){
				int tmp = small_idx;
				small_idx = large_idx;
				large_idx = tmp;
			}
			gateMap[eater_gateIdx].targs[0] = small_idx;
			gateMap[eater_gateIdx].targs[1] = large_idx;
			assert(gateMap[eater_gateIdx].targs[0] != gateMap[eater_gateIdx].targs[1]);
		} else {
			gateMap[eater_gateIdx].targs[0] = gateMap[eater_gateIdx].targs[0];
			gateMap[eater_gateIdx].targs[1] = gateMap[eater_gateIdx].targs[1];			
			if (gateMap[eater_gateIdx].targs[0] == gateMap[eater_gateIdx].targs[1]){
				printf("\n\n!!!!!!!!!!eater_gateIdx = %d\n\n\n", eater_gateIdx);
			}
			// assert(gateMap[eater_gateIdx].targs[0] != gateMap[eater_gateIdx].targs[1]);
		}
	} else { // dim = 8
		int small_idx, mediu_idx, large_idx; 
		if (gateMap[eater_gateIdx].numCtrls != 0) {
			small_idx = gateMap[eater_gateIdx].ctrls[0];
			mediu_idx = gateMap[eater_gateIdx].ctrls[1];
			large_idx = gateMap[eater_gateIdx].targs[0];

			if (mediu_idx > large_idx) {
				int tmp = mediu_idx;
				mediu_idx = large_idx; 
				large_idx = tmp;			
				if (small_idx > mediu_idx) {
					tmp = small_idx;
					small_idx = mediu_idx;
					mediu_idx = tmp;
				}
			}
			gateMap[eater_gateIdx].targs[0] = small_idx;
			gateMap[eater_gateIdx].targs[1] = mediu_idx;
			gateMap[eater_gateIdx].targs[2] = large_idx;			
		} 
	}

	// 如此一來所有人都在 targ，就沒有 control 了
    gateMap[eater_gateIdx].numTargs = gateMap[eater_gateIdx].numTargs + gateMap[eater_gateIdx].numCtrls;
    gateMap[eater_gateIdx].numCtrls = 0;

	// // printf("gateMap[%d].numCtrls = %d, gateMap[%d].numTargs = %d\n", eater_gateIdx, gateMap[eater_gateIdx].numCtrls, eater_gateIdx, gateMap[eater_gateIdx].numTargs);

	// for gateIdx 被吃掉的, setting the action = 0
	gateMap[gateIdx].action = 0;
}

void update_qubitTime (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int qubitTime [MAX_QUBIT][MAX_DEPTH], int eater_gateIdx, int gateIdx, int total_qubit) {	 
	// 被吃掉的那個 gate 要在大家的 qubitTime 被刪除，該 column 被刪除
	// printf("update_qubitTime, gateIdx = %d, total_qubit = %d\n", gateIdx, total_qubit);
	for (int j = 0; j < MAX_QUBIT; j++){
		qubitTime[j][gateIdx] = -1; // 因為你同一個時間段下，只會有一個 gate
	}
	// // printf("huahua: gateMap[%d].gate_which = %d\n", eater_gateIdx, gateMap[eater_gateIdx].gate_which);
	// eater 的時間序也要更新一下
	// for (int i = 0; i < gateMap[eater_gateIdx].numCtrls; i++){
	// 	int idx = gateMap[eater_gateIdx].ctrls[i];
	// 	qubitTime[idx][eater_gateIdx] = eater_gateIdx; // 該時間該 qubit 要執行 gateMap 上的第幾個 gate
	// }
	for (int i = 0; i < gateMap[eater_gateIdx].numTargs; i++){
		int idx = gateMap[eater_gateIdx].targs[i];
		qubitTime[idx][eater_gateIdx] = eater_gateIdx; 		
	}
}

void comupte_combine (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int dim, int pre_gateIdx, int gateIdx, float res_real [64], float res_imag [64]) {
	// printf("comupte_combine: pre_gateIdx = %d, gateIdx = %d\n", pre_gateIdx, gateIdx);
	double complex z1, z2, product;
	// printf("original matrix, gateIdx:\n");
	// for (int i = 0; i < dim; i++){
	// 	for (int j = 0; j < dim; j++){
	// 		printf("%f ", gateMap[gateIdx].real_matrix[i*dim+j]);
	// 	}
	// 	printf("\n");
	// }
	// // printf("original matrix, eater_Idx:\n");
	// for (int i = 0; i < dim; i++){
	// 	for (int j = 0; j < dim; j++){
	// 		printf("%f ", gateMap[pre_gateIdx].real_matrix[i*dim+j]);
	// 	}
	// 	printf("\n");
	// }	

	for (int i = 0; i < dim; i++) { // R1
		for (int j = 0; j < dim; j++){ // C2
			for (int k = 0; k < dim; k++){ // R2
				z1 = gateMap[pre_gateIdx].real_matrix[i*dim+k] + gateMap[pre_gateIdx].imag_matrix[i*dim+k] * I;
				z2 = gateMap[gateIdx].real_matrix[k*dim+j] + gateMap[gateIdx].imag_matrix[k*dim+j] * I;
				// // printf("Z1 idx = %d, Z2 idx = %d\n", i*dim+k, k*dim+j);
				// // printf("Z1 = %.2f + %.2fi\tZ2 = %.2f %+.2fi\n", creal(z1), cimag(z1), creal(z2), cimag(z2));
				product = z1 * z2;
				res_real[i*dim+j] += creal(product);
				res_imag[i*dim+j] += cimag(product);
			}
			// // printf("res_real[%d] = %f, res_imag[%d] = %f\n", i*dim+j, res_real[i*dim+j], i*dim+j, res_imag[i*dim+j]);
		}
	}

	// printf("product:\n");
	// for (int i = 0; i < dim; i++){
	// 	for (int j = 0; j < dim; j++){
	// 		printf("%f ", res_real[i*dim+j]);
	// 		// printf("%f + %f*i ", res_real[i*dim+j], res_imag[i*dim+j]);
	// 	}
	// 	printf("\n");
	// }
}

void single_combine (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int qubitTime [MAX_QUBIT][MAX_DEPTH], int eater_gateIdx, int gateIdx, int total_qubit) {
	// printf("deal with single_combine: eater_gateIdx = %d, gateIdx = %d\n", eater_gateIdx, gateIdx);
	float res_real [64] = {0};
	float res_imag [64] = {0};

	comupte_combine (gateMap, 2, gateIdx, eater_gateIdx, res_real, res_imag); // single qubit gate 是從後面吃過來，所以 eater 放後面
	update_gateMap  (gateMap, 2, eater_gateIdx, gateIdx, res_real, res_imag);
	update_qubitTime (gateMap, qubitTime, eater_gateIdx, gateIdx, total_qubit);
}

void prep_gates_mm (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int small_dim, int big_dim, int eater_gateIdx, int gateIdx) {
	// printf("prep_gates_mm\n");
	// printf("original matrix\n");
	// for (int i = 0; i < small_dim; i++){
	// 	for (int j = 0; j < small_dim; j++){
	// 		printf("%f ", gateMap[gateIdx].real_matrix[i*small_dim+j]);
	// 	}
	// 	printf("\n");
	// }

	// for (int i = 0; i < 4; i++){
	// 	for (int j = 0; j < 4; j++){
	// 		printf("%f ", gateMap[gateIdx].real_matrix[i*4+j]);
	// 	}
	// 	printf("\n");
	// }

	float tmp_real [64] = {0};
	float tmp_imag [64] = {0};

	// pre-store the data of single qubit gate
	for (int i = 0; i < small_dim*small_dim; i++){
		tmp_real[i] = gateMap[gateIdx].real_matrix[i];
		tmp_imag[i] = gateMap[gateIdx].imag_matrix[i];
	}
	for (int i = 0; i < big_dim*big_dim; i++){
		gateMap[gateIdx].real_matrix[i] = 0;
		gateMap[gateIdx].imag_matrix[i] = 0;
	}

	if ((gateMap[gateIdx].val_num == 4) && (gateMap[eater_gateIdx].val_num == 16)) { // 2*2 becomes 4*4
		// printf("2*2 becomes 4*4\n");
		// prepare index
		int small_idx, large_idx;
		if (gateMap[eater_gateIdx].numCtrls != 0){
			small_idx = gateMap[eater_gateIdx].ctrls[0];
			large_idx = gateMap[eater_gateIdx].targs[0];
			// sorting 
			if (small_idx > large_idx){
				int tmp = small_idx;
				small_idx = large_idx;
				large_idx = tmp;
			}
		} else {
			small_idx = gateMap[eater_gateIdx].targs[0];
			large_idx = gateMap[eater_gateIdx].targs[1];
		}

		// re-write the gateIdx's matrix
		if (gateMap[gateIdx].targs[0] == small_idx) { // case 1. single qubit gate applied on the small index
			for (int i = 0; i < 2; i++){ // 填兩次
				// printf("1~~~~\n");
				for (int j = 0; j < 4; j++){ // 4個參數
					gateMap[gateIdx].real_matrix[big_dim*((j/2)*2+i)+i+(j%2)*2] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[big_dim*((j/2)*2+i)+i+(j%2)*2] = tmp_imag[j];
				}
			}
		} else {  // case 2. single qubit gate applied on the large index, gateMap[gateIdx].targs[0] == large_idx
			for (int i = 0; i < 2; i++){
				// printf("2~~~~\n");
				for (int j = 0; j < 4; j++){
					gateMap[gateIdx].real_matrix[big_dim*(2*i+(j/2))+(2*i+(j%2))] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[big_dim*(2*i+(j/2))+(2*i+(j%2))] = tmp_imag[j];
				}
			}
		}
	} else if ((gateMap[gateIdx].val_num == 4) && (gateMap[eater_gateIdx].val_num == 64)) { // 2*2 becomes 8*8
		// printf("2*2 becomes 8*8\n");
		// prepare index
		int small_idx, mediu_idx, large_idx;
		if (gateMap[eater_gateIdx].numCtrls != 0){
			small_idx = gateMap[eater_gateIdx].ctrls[0];
			mediu_idx = gateMap[eater_gateIdx].ctrls[1]; // ensure mediu_idx > small_idx is true
			large_idx = gateMap[eater_gateIdx].targs[0];
			// sorting 
			if (mediu_idx > large_idx){
				int tmp = mediu_idx;
				mediu_idx = large_idx;
				large_idx = tmp;
				if (small_idx > mediu_idx){
					int tmp = small_idx;
					small_idx = mediu_idx;
					mediu_idx = tmp;
				}
			}
		} else {
			small_idx = gateMap[eater_gateIdx].targs[0];
			mediu_idx = gateMap[eater_gateIdx].targs[1];
			large_idx = gateMap[eater_gateIdx].targs[2];		
		}
		// re-write the gateIdx's matrix
		if (gateMap[gateIdx].targs[0] == small_idx) { // case 1. single qubit gate applied on the small index
			for (int i = 0; i < 4; i++){ // 填四次
				for (int j = 0; j < 4; j++){ // 4個參數
					// gateMap[gateIdx].real_matrix[big_dim*(i+0.0)+i] = tmp_real[0];
					// gateMap[gateIdx].real_matrix[big_dim*(i+0.5)+i] = tmp_real[1];
					// gateMap[gateIdx].real_matrix[big_dim*(i+4.0)+i] = tmp_real[2];
					// gateMap[gateIdx].real_matrix[big_dim*(i+4.5)+i] = tmp_real[3];
					gateMap[gateIdx].real_matrix[(int)(big_dim*(i+((j/2)*4 + (j%2)*0.5)) + i)] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[(int)(big_dim*(i+((j/2)*4 + (j%2)*0.5)) + i)] = tmp_imag[j];
				}
			}
		} else if (gateMap[gateIdx].targs[0] == mediu_idx) {  // case 2. single qubit gate applied on the mediu index
			for (int i = 0; i < 4; i++){
				for (int j = 0; j < 4; j++){
					// gateMap[gateIdx].real_matrix[big_dim*(4*(i/2)+(i%2))   + 4*(i/2)+(i%2)] = tmp_real[0];
					// gateMap[gateIdx].real_matrix[big_dim*(4*(i/2)+(i%2))   + 4*(i/2)+(i%2)+2] = tmp_real[1];
					// gateMap[gateIdx].real_matrix[big_dim*(4*(i/2)+(i%2)+2) + 4*(i/2)+(i%2)] = tmp_real[2];
					// gateMap[gateIdx].real_matrix[big_dim*(4*(i/2)+(i%2)+2) + 4*(i/2)+(i%2)+2] = tmp_real[3];
					gateMap[gateIdx].real_matrix[big_dim*(4*(i/2)+(i%2)+(j/2)*2) + 4*(i/2)+(i%2)+(j%2)*2] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[big_dim*(4*(i/2)+(i%2)+(j/2)*2) + 4*(i/2)+(i%2)+(j%2)*2] = tmp_imag[j];
				}
			}
		} else { // case 3. single qubit gate applied on the large index, gateMap[gateIdx].targs[0] == large_idx
			for (int i = 0; i < 4; i++){
				for (int j = 0; j < 4; j++){
					// gateMap[gateIdx].real_matrix[18*i+0] = tmp_real[0];
					// gateMap[gateIdx].real_matrix[18*i+1] = tmp_real[1];
					// gateMap[gateIdx].real_matrix[18*i+big_dim+0] = tmp_real[2];
					// gateMap[gateIdx].real_matrix[18*i+big_dim+1] = tmp_real[3];
					gateMap[gateIdx].real_matrix[18*i+big_dim*(j/2)+(j%2)] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[18*i+big_dim*(j/2)+(j%2)] = tmp_imag[j];
				}
			}
		} 
	} else { // 4*4 becomes 8*8
		// printf("4*4 becomes 8*8\n");
		// prepare index
		int small_idx, mediu_idx, large_idx;
		if (gateMap[eater_gateIdx].numCtrls != 0){
			small_idx = gateMap[eater_gateIdx].ctrls[0];
			mediu_idx = gateMap[eater_gateIdx].ctrls[1]; // ensure mediu_idx > small_idx is true
			large_idx = gateMap[eater_gateIdx].targs[0];
			// sorting 
			if (mediu_idx > large_idx){
				int tmp = mediu_idx;
				mediu_idx = large_idx;
				large_idx = tmp;
				if (small_idx > mediu_idx){
					int tmp = small_idx;
					small_idx = mediu_idx;
					mediu_idx = tmp;
				}
			}
		} else {
			small_idx = gateMap[eater_gateIdx].targs[0];
			mediu_idx = gateMap[eater_gateIdx].targs[1];
			large_idx = gateMap[eater_gateIdx].targs[2];		
		}	
		// two qubit gate index prepartion
		int two_small_idx, two_large_idx;
		if (gateMap[gateIdx].numCtrls != 0){
			two_small_idx = gateMap[gateIdx].targs[0];
			two_large_idx = gateMap[gateIdx].ctrls[0];
			if (two_small_idx > two_large_idx){
				int tmp = two_small_idx; 
				two_small_idx = two_large_idx;
				two_large_idx = tmp;
			}
		} else {
			two_small_idx = gateMap[gateIdx].targs[0];
			two_large_idx = gateMap[gateIdx].targs[1];
		}
				
		if ((two_small_idx == small_idx) && (two_small_idx == mediu_idx)) { // applied on small and medium idx
			for (int i = 0; i < 2; i++){ // 填2次
				for (int j = 0 ; j < 16; j++){  // 16 個參數
					gateMap[gateIdx].real_matrix[big_dim*(2*(j/4)+i) + 2*(j%4) +i] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[big_dim*(2*(j/4)+i) + 2*(j%4) +i] = tmp_imag[j];
				}
			}
		} else if ((two_small_idx == small_idx) && (two_small_idx == large_idx)) { // applied on small and large idx
			for (int i = 0; i < 2; i++){ // 填2次
				for (int j = 0 ; j < 16; j++){  // 16 個參數
					// 32*int(j/8) + (int(j/4)%2)*big_dim + ((int(j/2))%2)*0.5*big_dim + (j%2) + i*18
					gateMap[gateIdx].real_matrix[32*(j/8) + ((j/4)%2)*big_dim + ((j/2)%2)*(big_dim/2) + (j%2) + i*18] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[32*(j/8) + ((j/4)%2)*big_dim + ((j/2)%2)*(big_dim/2) + (j%2) + i*18] = tmp_imag[j];
				}
			}
		} else { // (two_small_idx == mediu_idx) && (two_small_idx == large_idx), applied on medium and large idx
			for (int i = 0; i < 2; i++){ // 填2次
				for (int j = 0 ; j < 16; j++){  // 16 個參數
					gateMap[gateIdx].real_matrix[(j/4)*big_dim + (j%4) + i*36] = tmp_real[j];
					gateMap[gateIdx].imag_matrix[(j/4)*big_dim + (j%4) + i*36] = tmp_imag[j];
				}
			}
		}
	}
	

	// printf("After prepare\n");
	// for (int i = 0; i < big_dim; i++){
	// 	for (int j = 0; j < big_dim; j++){
	// 		printf("%f ", gateMap[gateIdx].real_matrix[i*big_dim+j]);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");

	// for (int i = 0; i < big_dim; i++){
	// 	for (int j = 0; j < big_dim; j++){
	// 		printf("%f ", gateMap[gateIdx].imag_matrix[i*big_dim+j]);
	// 	}
	// 	printf("\n");
	// }
}

void two_combine (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int qubitTime [MAX_QUBIT][MAX_DEPTH], int eater_gateIdx, int gateIdx, int total_qubit, int eat_order) {
	float res_real [64] = {0};
	float res_imag [64] = {0};

	// printf("gateMap[%d].val_num = %d\n", gateIdx, gateMap[gateIdx].val_num);
	if (gateMap[gateIdx].val_num == 4){   // single qubit gate 才要擴大矩陣
		prep_gates_mm (gateMap, 2, 4, eater_gateIdx, gateIdx);
	}
	if (eat_order == 0) // eat_order == 0，由前往後吃
		comupte_combine (gateMap, 4, eater_gateIdx, gateIdx, res_real, res_imag);
	else
		comupte_combine (gateMap, 4, gateIdx, eater_gateIdx, res_real, res_imag);

	// printf("hua real\n");
	// for (int i = 0; i < 4; i++){
	// 	for (int j = 0; j < 4; j++){
	// 		printf("%f ", res_real[i*4+j]);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");
	// for (int i = 0; i < 4; i++){
	// 	for (int j = 0; j < 4; j++){
	// 		printf("%f ", res_imag[i*4+j]);
	// 	}
	// 	printf("\n");
	// }

	update_gateMap  (gateMap, 4, eater_gateIdx, gateIdx, res_real, res_imag);


	// printf("kk\n");
	// for (int i = 0; i < 4; i++){
	// 	for (int j = 0; j < 4; j++){
	// 		printf("%f ", gateMap[eater_gateIdx].imag_matrix[i*4+j]);
	// 	}
	// 	printf("\n");
	// }

	update_qubitTime (gateMap, qubitTime, eater_gateIdx, gateIdx, total_qubit);
}

void three_combine (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int qubitTime [MAX_QUBIT][MAX_DEPTH], int eater_gateIdx, int gateIdx, int total_qubit, int eat_order) {
	float res_real [64] = {0};
	float res_imag [64] = {0};

	if (gateMap[gateIdx].val_num == 4) // single qubit gate, 2 補成 8
		prep_gates_mm (gateMap, 2, 8, eater_gateIdx, gateIdx);
	if (gateMap[gateIdx].val_num == 16) // two qubit gate, 4 補成 8
		prep_gates_mm (gateMap, 4, 8, eater_gateIdx, gateIdx);

	if (eat_order == 0) // eat_order == 0，由前往後吃
		comupte_combine (gateMap, 8, eater_gateIdx, gateIdx, res_real, res_imag);
	else
		comupte_combine (gateMap, 8, gateIdx, eater_gateIdx, res_real, res_imag);


	// printf("hua real\n");
	// for (int i = 0; i < 8; i++){
	// 	for (int j = 0; j < 8; j++){
	// 		printf("%f ", res_real[i*8+j]);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");
	// for (int i = 0; i < 8; i++){
	// 	for (int j = 0; j < 8; j++){
	// 		printf("%f ", res_imag[i*8+j]);
	// 	}
	// 	printf("\n");
	// }


	update_gateMap  (gateMap, 8, eater_gateIdx, gateIdx, res_real, res_imag);
	
	// printf("kk\n");
	// for (int i = 0; i < 8; i++){
	// 	for (int j = 0; j < 8; j++){
	// 		printf("%f ", gateMap[eater_gateIdx].real_matrix[i*8+j]);
	// 	}
	// 	printf("\n");
	// }
	// printf("\n");
	// for (int i = 0; i < 8; i++){
	// 	for (int j = 0; j < 8; j++){
	// 		printf("%f ", gateMap[eater_gateIdx].imag_matrix[i*8+j]);
	// 	}
	// 	printf("\n");
	// }

	update_qubitTime (gateMap, qubitTime, eater_gateIdx, gateIdx, total_qubit);
}



void mm_extension (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int small_dim, int big_dim, int eater_Idx, struct gate tmp_gate) {
	// for (int i = 0; i < 4; i++){
	// 	for (int j = 0; j < 4; j++){
	// 		printf("%f ", tmp_gate.real_matrix[i*4+j]);
	// 	}
	// 	printf("\n");
	// }

	float tmp_real [64] = {0};
	float tmp_imag [64] = {0};

	// pre-store the data of single qubit gate
	for (int i = 0; i < small_dim*small_dim; i++){
		tmp_real[i] = tmp_gate.real_matrix[i];
		tmp_imag[i] = tmp_gate.imag_matrix[i];
	}
	for (int i = 0; i < big_dim*big_dim; i++){
		tmp_gate.real_matrix[i] = 0;
		tmp_gate.imag_matrix[i] = 0;
	}

	if (big_dim == 4) { // 2*2 becomes 4*4
		// printf("2*2 becomes 4*4\n");
		// prepare index
		int small_idx, large_idx;
		if (gateMap[eater_Idx].numCtrls != 0){
			small_idx = gateMap[eater_Idx].ctrls[0];
			large_idx = gateMap[eater_Idx].targs[0];
			// sorting 
			if (small_idx > large_idx){
				int tmp = small_idx;
				small_idx = large_idx;
				large_idx = tmp;
			}
		} else {
			small_idx = gateMap[eater_Idx].targs[0];
			large_idx = gateMap[eater_Idx].targs[1];
		}

		// re-write the gateIdx's matrix
		if (tmp_gate.targs[0] == small_idx) { // case 1. single qubit gate applied on the small index
			for (int i = 0; i < 2; i++){ // 填兩次
				for (int j = 0; j < 4; j++){ // 4個參數
					tmp_gate.real_matrix[big_dim*((j/2)*2+i)+i+(j%2)*2] = tmp_real[j];
					tmp_gate.imag_matrix[big_dim*((j/2)*2+i)+i+(j%2)*2] = tmp_imag[j];
				}
			}
		} else {  // case 2. single qubit gate applied on the large index, tmp_gate.targs[0] == large_idx
			for (int i = 0; i < 2; i++){
				for (int j = 0; j < 4; j++){
					tmp_gate.real_matrix[big_dim*(2*i+(j/2))+(2*i+(j%2))] = tmp_real[j];
					tmp_gate.imag_matrix[big_dim*(2*i+(j/2))+(2*i+(j%2))] = tmp_imag[j];
				}
			}
		}
	} else if ((big_dim == 8) && (big_dim == 2)) { // 2*2 becomes 8*8
		// printf("2*2 becomes 8*8\n");
		// prepare index
		int small_idx, mediu_idx, large_idx;
		if (gateMap[eater_Idx].numCtrls != 0){
			small_idx = gateMap[eater_Idx].ctrls[0];
			mediu_idx = gateMap[eater_Idx].ctrls[1]; // ensure mediu_idx > small_idx is true
			large_idx = gateMap[eater_Idx].targs[0];
			// sorting 
			if (mediu_idx > large_idx){
				int tmp = mediu_idx;
				mediu_idx = large_idx;
				large_idx = tmp;
				if (small_idx > mediu_idx){
					int tmp = small_idx;
					small_idx = mediu_idx;
					mediu_idx = tmp;
				}
			}
		} else {
			small_idx = gateMap[eater_Idx].targs[0];
			mediu_idx = gateMap[eater_Idx].targs[1];
			large_idx = gateMap[eater_Idx].targs[2];		
		}
		// re-write the gateIdx's matrix
		if (tmp_gate.targs[0] == small_idx) { // case 1. single qubit gate applied on the small index
			for (int i = 0; i < 4; i++){ // 填四次
				for (int j = 0; j < 4; j++){ // 4個參數
					// tmp_gate.real_matrix[big_dim*(i+0.0)+i] = tmp_real[0];
					// tmp_gate.real_matrix[big_dim*(i+0.5)+i] = tmp_real[1];
					// tmp_gate.real_matrix[big_dim*(i+4.0)+i] = tmp_real[2];
					// tmp_gate.real_matrix[big_dim*(i+4.5)+i] = tmp_real[3];
					tmp_gate.real_matrix[(int)(big_dim*(i+((j/2)*4 + (j%2)*0.5)) + i)] = tmp_real[j];
					tmp_gate.imag_matrix[(int)(big_dim*(i+((j/2)*4 + (j%2)*0.5)) + i)] = tmp_imag[j];
				}
			}
		} else if (tmp_gate.targs[0] == mediu_idx) {  // case 2. single qubit gate applied on the mediu index
			for (int i = 0; i < 4; i++){
				for (int j = 0; j < 4; j++){
					// tmp_gate.real_matrix[big_dim*(4*(i/2)+(i%2))   + 4*(i/2)+(i%2)] = tmp_real[0];
					// tmp_gate.real_matrix[big_dim*(4*(i/2)+(i%2))   + 4*(i/2)+(i%2)+2] = tmp_real[1];
					// tmp_gate.real_matrix[big_dim*(4*(i/2)+(i%2)+2) + 4*(i/2)+(i%2)] = tmp_real[2];
					// tmp_gate.real_matrix[big_dim*(4*(i/2)+(i%2)+2) + 4*(i/2)+(i%2)+2] = tmp_real[3];
					tmp_gate.real_matrix[big_dim*(4*(i/2)+(i%2)+(j/2)*2) + 4*(i/2)+(i%2)+(j%2)*2] = tmp_real[j];
					tmp_gate.imag_matrix[big_dim*(4*(i/2)+(i%2)+(j/2)*2) + 4*(i/2)+(i%2)+(j%2)*2] = tmp_imag[j];
				}
			}
		} else { // case 3. single qubit gate applied on the large index, tmp_gate.targs[0] == large_idx
			for (int i = 0; i < 4; i++){
				for (int j = 0; j < 4; j++){
					// tmp_gate.real_matrix[18*i+0] = tmp_real[0];
					// tmp_gate.real_matrix[18*i+1] = tmp_real[1];
					// tmp_gate.real_matrix[18*i+big_dim+0] = tmp_real[2];
					// tmp_gate.real_matrix[18*i+big_dim+1] = tmp_real[3];
					tmp_gate.real_matrix[18*i+big_dim*(j/2)+(j%2)] = tmp_real[j];
					tmp_gate.imag_matrix[18*i+big_dim*(j/2)+(j%2)] = tmp_imag[j];
				}
			}
		} 
	} else { // 4*4 becomes 8*8
		// printf("4*4 becomes 8*8\n");
		// prepare index
		int small_idx, mediu_idx, large_idx;
		if (gateMap[eater_Idx].numCtrls != 0){
			small_idx = gateMap[eater_Idx].ctrls[0];
			mediu_idx = gateMap[eater_Idx].ctrls[1]; // ensure mediu_idx > small_idx is true
			large_idx = gateMap[eater_Idx].targs[0];
			// sorting 
			if (mediu_idx > large_idx){
				int tmp = mediu_idx;
				mediu_idx = large_idx;
				large_idx = tmp;
				if (small_idx > mediu_idx){
					int tmp = small_idx;
					small_idx = mediu_idx;
					mediu_idx = tmp;
				}
			}
		} else {
			small_idx = gateMap[eater_Idx].targs[0];
			mediu_idx = gateMap[eater_Idx].targs[1];
			large_idx = gateMap[eater_Idx].targs[2];		
		}	
		// two qubit gate index prepartion
		int two_small_idx, two_large_idx;
		if (tmp_gate.numCtrls != 0){
			two_small_idx = tmp_gate.targs[0];
			two_large_idx = tmp_gate.ctrls[0];
			if (two_small_idx > two_large_idx){
				int tmp = two_small_idx; 
				two_small_idx = two_large_idx;
				two_large_idx = tmp;
			}
		} else {
			two_small_idx = tmp_gate.targs[0];
			two_large_idx = tmp_gate.targs[1];
		}
				
		if ((two_small_idx == small_idx) && (two_small_idx == mediu_idx)) { // applied on small and medium idx
			for (int i = 0; i < 2; i++){ // 填2次
				for (int j = 0 ; j < 16; j++){  // 16 個參數
					tmp_gate.real_matrix[big_dim*(2*(j/4)+i) + 2*(j%4) +i] = tmp_real[j];
					tmp_gate.imag_matrix[big_dim*(2*(j/4)+i) + 2*(j%4) +i] = tmp_imag[j];
				}
			}
		} else if ((two_small_idx == small_idx) && (two_small_idx == large_idx)) { // applied on small and large idx
			for (int i = 0; i < 2; i++){ // 填2次
				for (int j = 0 ; j < 16; j++){  // 16 個參數
					// 32*int(j/8) + (int(j/4)%2)*big_dim + ((int(j/2))%2)*0.5*big_dim + (j%2) + i*18
					tmp_gate.real_matrix[32*(j/8) + ((j/4)%2)*big_dim + ((j/2)%2)*(big_dim/2) + (j%2) + i*18] = tmp_real[j];
					tmp_gate.imag_matrix[32*(j/8) + ((j/4)%2)*big_dim + ((j/2)%2)*(big_dim/2) + (j%2) + i*18] = tmp_imag[j];
				}
			}
		} else { // (two_small_idx == mediu_idx) && (two_small_idx == large_idx), applied on medium and large idx
			for (int i = 0; i < 2; i++){ // 填2次
				for (int j = 0 ; j < 16; j++){  // 16 個參數
					tmp_gate.real_matrix[(j/4)*big_dim + (j%4) + i*36] = tmp_real[j];
					tmp_gate.imag_matrix[(j/4)*big_dim + (j%4) + i*36] = tmp_imag[j];
				}
			}
		}
	}
}

int comupte_communicate (int dim, struct gate pre_gate, struct gate tmp_gate, 
						float R_1 [64], float I_1 [64], float R_2 [64], float I_2 [64]) {
	for (int i = 0; i < 64; i++){
		R_1[i] = 0;
		R_2[i] = 0;
		I_1[i] = 0;
		I_2[i] = 0;
	}
	// 第一次
	double complex z1, z2, product;
	for (int i = 0; i < dim; i++) { // R1
		for (int j = 0; j < dim; j++){ // C2
			for (int k = 0; k < dim; k++){ // R2
				z1 = pre_gate.real_matrix[i*dim+k] + pre_gate.imag_matrix[i*dim+k] * I;
				z2 = tmp_gate.real_matrix[k*dim+j] + tmp_gate.imag_matrix[k*dim+j] * I;
				product = z1 * z2;
				R_1[i*dim+j] += creal(product);
				I_1[i*dim+j] += cimag(product);
			}
		}
	}
	// 第二次， // 交換順序
	for (int i = 0; i < dim; i++) { // R1
		for (int j = 0; j < dim; j++){ // C2
			for (int k = 0; k < dim; k++){ // R2
				z1 = tmp_gate.real_matrix[k*dim+j] + tmp_gate.imag_matrix[k*dim+j] * I;
				z2 = pre_gate.real_matrix[i*dim+k] + pre_gate.imag_matrix[i*dim+k] * I;
				product = z1 * z2;
				R_2[i*dim+j] += creal(product);
				I_2[i*dim+j] += cimag(product);
			}
		}
	}

	for (int i = 0; i < dim; i++){
		for (int j = 0; j < dim; j++){
			if (R_1[i*dim +j] != R_2[i*dim+j])
				return 0;
			if (I_1[i*dim +j] != I_2[i*dim+j])
				return 0;

		}
	}
	return 1;
}

int commutative (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int pre_gateIdx, int gateIdx) {
	float R_1 [64] = {0};
	float I_1 [64] = {0};
	float R_2 [64] = {0};
	float I_2 [64] = {0};

	struct gate tmp_gate = gateMap[gateIdx]; //  新建立一個 gates，讓他去當 gateIdx，因為我不想動到原本的資
    // memcpy(q_write+s*chunk_state+move+i, q+s*chunk_state+i, move*sizeof(Type)); // upper
	int commu = 0;
	if (gateMap[gateIdx].val_num == 4 && gateMap[pre_gateIdx].val_num == 16){  // single qubi gate 遇到 two qubit gate
		mm_extension (gateMap, 2, 4, pre_gateIdx, tmp_gate);
		commu = comupte_communicate (4, gateMap[pre_gateIdx], tmp_gate, R_1, I_1, R_2, I_2);
	} else if (gateMap[gateIdx].val_num == 4 && gateMap[pre_gateIdx].val_num == 64) {   // single qubit gate 遇到 three qubit gate
		mm_extension (gateMap, 2, 4, pre_gateIdx, tmp_gate);
		commu = comupte_communicate (8, gateMap[pre_gateIdx], tmp_gate, R_1, I_1, R_2, I_2);
	} else if (gateMap[gateIdx].val_num == 16 && gateMap[pre_gateIdx].val_num == 64){ 
		mm_extension (gateMap, 4, 8, pre_gateIdx, tmp_gate);
		commu = comupte_communicate (8, gateMap[pre_gateIdx], tmp_gate, R_1, I_1, R_2, I_2);
	} else {
		printf("In theory should not be here\n");
	}

	return commu;
}

int two_check_same_qubit (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int post_gateIdx, int gateIdx) {
	int applied_qubit [2][2] = {0};
	for (int i = 0; i < 2; i++)
		applied_qubit[i][0] = -1;

	applied_qubit[0][0] = gateMap[gateIdx].targs[0];
	applied_qubit[1][0] = (gateMap[gateIdx].numCtrls == 0) ? (gateMap[gateIdx].targs[1]) : (gateMap[gateIdx].ctrls[0]);

	for (int i = 0; i < gateMap[post_gateIdx].numCtrls; i++){
		if (gateMap[post_gateIdx].ctrls[i] == applied_qubit[0][0])
			applied_qubit[0][1] = 1;
		if (gateMap[post_gateIdx].ctrls[i] == applied_qubit[1][0])
			applied_qubit[1][1] = 1;		
	}
	for (int i = 0; i < gateMap[post_gateIdx].numTargs; i++){
		if (gateMap[post_gateIdx].targs[i] == applied_qubit[0][0])
			applied_qubit[0][1] = 1;
		if (gateMap[post_gateIdx].targs[i] == applied_qubit[1][0])
			applied_qubit[1][1] = 1;		
	}	

	if (applied_qubit[0][1] == 1 && applied_qubit[1][1] == 1)
		return 1;
	return 0;
	// 這裡要另外 check 一個是 unitary 間 是否有 其他 gates
}

int three_check_same_qubit (struct gate gateMap[MAX_QUBIT*MAX_DEPTH], int eater_gateIdx, int gateIdx) {		
	// init
	int applied_qubit [3][3] = {0};
	for (int i = 0; i < 3; i++)
		applied_qubit[i][0] = -1;


	for (int i = 0; i < gateMap[eater_gateIdx].numCtrls; i++) // 加入 eater 的 control
		applied_qubit[i][0] = gateMap[eater_gateIdx].ctrls[i]; 
	for (int i = 0; i < gateMap[eater_gateIdx].numTargs; i++) // 加入 eater 的 target
		applied_qubit[i+gateMap[eater_gateIdx].numCtrls][0] = gateMap[eater_gateIdx].targs[i];

	for (int i = 0; i < gateMap[gateIdx].numCtrls; i++)
		for (int j = 0; j < 3; j++)
			if (gateMap[gateIdx].ctrls[i] == applied_qubit[j][0])
				applied_qubit[j][1] = 1;
	for (int i = 0; i < gateMap[gateIdx].numTargs; i++)
		for (int j = 0; j < 3; j++)
			if (gateMap[gateIdx].targs[i] == applied_qubit[j][0])
				applied_qubit[j][1] = 1;
	
	int ctl_add_tar = 0;
	for (int j = 0; j < 3; j++)
		ctl_add_tar += applied_qubit[j][1];
		
	if (ctl_add_tar == (gateMap[eater_gateIdx].numCtrls + gateMap[eater_gateIdx].numTargs))
		return 1;
	return 0;

}
//gateMap[MAX_num_qubit*MAX_circuit_depth]
//qubitTime [MAX_num_qubit][MAX_circuit_depth]
void circuit_scheduler (gate *gateMap, int (*qubitTime)[MAX_DEPTH], int total_qubit, int total_gate) {
    // see_gateMap(gateMap, total_gate);
    // see_qubitTime(qubitTime, total_qubit, total_gate);

	// printf("single qubit \n");
	for (int q = 0; q < total_qubit; q++){
		for (int g = total_gate-1; g > -1; g--){ // 特定要被 combine 的 gate, specific-gate
			int gateIdx = qubitTime[q][g];
			int ctl_plus_tar = gateMap[gateIdx].numCtrls + gateMap[gateIdx].numTargs; // check whether single qubit gate
			// printf("g = %d, gateMap[%d].action = %d, ctl_plus_tar= %d\n", g, gateIdx, gateMap[gateIdx].action, ctl_plus_tar);
			if ((gateMap[gateIdx].action == 1) && (ctl_plus_tar == 1)){ // 是 active 的 single qubit gate
				for (int k = g - 1; k >= 0; k--){ // specific-gate 前面的 gate，specific-gate 要被吃進去
					int pre_gateIdx = qubitTime[q][k];
					int pre_ctl_plus_tar = gateMap[pre_gateIdx].numCtrls + gateMap[pre_gateIdx].numTargs;
					// printf("specific qubit = %d, specific gate = %d, pre_gate = %d, gateIdx = %d, pre_gateIdx = %d ,ctl_plus_tar = %d, pre_ctl_plus_tar = %d\n", q, g, k, gateIdx, pre_gateIdx, ctl_plus_tar, pre_ctl_plus_tar);
					if ((gateMap[pre_gateIdx].action == 1) && (pre_ctl_plus_tar == 1)) { // previous is single qubit gate, then combine
						single_combine (gateMap, qubitTime, gateIdx, pre_gateIdx, total_qubit);
						// break; // 吃完之後就要換下一個 specific-gate
					} else if ((gateMap[pre_gateIdx].action == 1) && (commutative(gateMap, pre_gateIdx, gateIdx) == 1)) { // 沒有吃到，但不可以繼續向前
						// printf("meet with barrier \n");
						break; 
					}
					// 若沒吃到，但可以向前，就繼續向前 
					// printf("cannot eat but can move continuously, commutative(gateMap, pre_gateIdx, gateIdx) == 1\n");
				}		
			}
		}	
		// printf("\n");	
	}
 //    // see_gateMap(gateMap, total_gate);
 //    see_qubitTime(qubitTime, total_qubit, total_gate);
	// printf("-----------\n\n");

	// // combine two qubit gates
	// printf("\n\nTwo qubit \n"); // 由前面 trace 到後面，那就是前面的 gate 把後面吃掉，PASS
	for (int q = 0; q < total_qubit; q++){
		// printf("q = %d\n", q);
		for (int g = 0; g < total_gate; g++){ 
			int gateIdx = qubitTime[q][g];
			int ctl_plus_tar = gateMap[gateIdx].numCtrls + gateMap[gateIdx].numTargs; 
			// printf("g = %d, gateMap[%d].action = %d, ctl_plus_tar= %d, \n", g, gateIdx, gateMap[gateIdx].action, ctl_plus_tar);
			if ((gateMap[gateIdx].action == 1) && (ctl_plus_tar == 2)){ // 找到一個 two qubit gate,  
				for (int k = g + 1; k < total_gate; k++){ 
					int post_gateIdx = qubitTime[q][k];
					int post_ctl_plus_tar = gateMap[post_gateIdx].numCtrls + gateMap[post_gateIdx].numTargs;
					// printf("specific qubit = %d, specific gate = %d, post_gate = %d, gateIdx = %d, post_gateIdx = %d ,ctl_plus_tar = %d, post_ctl_plus_tar = %d\n", q, g, k, gateIdx, post_gateIdx, ctl_plus_tar, post_ctl_plus_tar);

					if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 1)) { // 遇到 single, single 要反吃前面
						// printf("meet with single qubit gate, combine\n");
						two_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit,0 ); // eater 放前面
					} 
					else if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 2)) { // 遇到 two qubit gate, two 要反吃前面
						// printf("meet with two qubit gate:\n");
						int same = two_check_same_qubit (gateMap, post_gateIdx, gateIdx);
						int barrier = 0; // 同時要檢查，另一邊的 qubit 中間有沒有東西擋著
						int inter_idx_0 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[1]) : (gateMap[g].ctrls[0]);
						int inter_idx_1 = gateMap[g].targs[0]; 
						// printf("inter_idx_0 = %d, inter_idx_1 = %d\n", inter_idx_0, inter_idx_1);
						// printf("gateMap[g].targs[1] = %d, gateMap[g].targs[0] = %d, gateMap[g].ctrls[0] = %d\n", gateMap[g].targs[0], gateMap[g].targs[1], gateMap[g].ctrls[0]);
						for (int tmp_g = g+1; tmp_g < k; tmp_g++){ // 自己是第 g 個 gate，就從下一個 gate 開始檢查到現在要合併的那一個
							if(qubitTime[inter_idx_0][tmp_g] != -1){
								barrier = 1;
							}
							if(qubitTime[inter_idx_1][tmp_g] != -1){
								barrier = 1;
							}
						}
						// printf("post_gateIdx = %d, same = %d, barrier = %d\n", post_gateIdx, same, barrier);
						if (same == 1 && barrier == 0)
							two_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 0);
						else{
							// printf("kokokoko\n");
							break;
						}
					} else if ((gateMap[post_gateIdx].action == 1) && (commutative(gateMap, post_gateIdx, gateIdx) == 1)) {
						// printf("meet with barrier \n");						
						break; 
					} 
					// // printf("cannot eat but can move continuously, commutative(gateMap, pre_gateIdx, gateIdx) == 1\n");
				}		
			}
		}	
	}
 // //    // see_gateMap(gateMap, total_gate);
 // //    see_qubitTime(qubitTime, total_qubit, total_gate);
	// // printf("-----------\n\n");
	
	// // // // // combine three qubit gates，從前面吃過去
	// printf("Three qubit \n");
	for (int q = 0; q < total_qubit; q++){
		// printf("q = %d\n", q);
		for (int g = 0; g < total_gate; g++){ 
			int gateIdx = qubitTime[q][g];
			int ctl_plus_tar = gateMap[gateIdx].numCtrls + gateMap[gateIdx].numTargs; 
			// printf("g = %d, gateMap[%d].action = %d, ctl_plus_tar= %d, \n", g, gateIdx, gateMap[gateIdx].action, ctl_plus_tar);
			if ((gateMap[gateIdx].action == 1) && (ctl_plus_tar == 3)){ 
				for (int k = g + 1; k < total_gate; k++){ 
					int post_gateIdx = qubitTime[q][k];
					int post_ctl_plus_tar = gateMap[post_gateIdx].numCtrls + gateMap[post_gateIdx].numTargs;
					// printf("specific qubit = %d, specific gate = %d, post_gate = %d, gateIdx = %d, post_gateIdx = %d ,ctl_plus_tar = %d, post_ctl_plus_tar = %d\n", q, g, k, gateIdx, post_gateIdx, ctl_plus_tar, post_ctl_plus_tar);
					if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 1)) { // 遇到 single, single 要反吃後面
						three_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 0);
					} else if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 2)) { // 遇到 two qubit gate, two 要反吃後面
						int same = three_check_same_qubit (gateMap, post_gateIdx, gateIdx); // 判斷能不能吃 (applied 在相同的 Gate 上)，能吃就吃不能吃就 break 掉
						// printf("post_gateIdx = %d, same = %d\n", post_gateIdx, same);
						int barrier = 0;
						if (same == 1){
							int inter_idx_0 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[1]) : (gateMap[g].ctrls[0]);
							int inter_idx_1 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[2]) : (gateMap[g].ctrls[1]); 
							int inter_idx_2 = gateMap[g].targs[0]; 
							for (int tmp_g = g+1; tmp_g < k; tmp_g++){ // 自己是第 g 個 gate，就從下一個 gate 開始檢查到現在要合併的那一個
								if(qubitTime[inter_idx_0][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_1][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_2][tmp_g] != -1){
									barrier = 1;
								}							
							}								
						}					
						if (same == 1 && barrier == 0)	
							three_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 0);
						else
							break;
					} else if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 3)) { // 遇到 two qubit gate, 可以吃就把它吃掉
						int same = three_check_same_qubit (gateMap, post_gateIdx, gateIdx); // 判斷能不能吃 (applied 在相同的 Gate 上)，能吃就吃不能吃就 break 掉
						// printf("post_gateIdx = %d, same = %d\n", post_gateIdx, same);
						int barrier = 0;
						if (same == 1){
							int inter_idx_0 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[1]) : (gateMap[g].ctrls[0]);
							int inter_idx_1 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[2]) : (gateMap[g].ctrls[1]); 
							int inter_idx_2 = gateMap[g].targs[0]; 
							for (int tmp_g = g+1; tmp_g < k; tmp_g++){ // 自己是第 g 個 gate，就從下一個 gate 開始檢查到現在要合併的那一個
								if(qubitTime[inter_idx_0][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_1][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_2][tmp_g] != -1){
									barrier = 1;
								}							
							}							
						}
						if (same == 1 && barrier == 0) {	
							three_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 0);
						} else {
							// printf("kokokoko\n");
							break;
						}
					} else if ((gateMap[post_gateIdx].action == 1) && (commutative(gateMap, post_gateIdx, gateIdx) == 1)) {
						// printf("meet with barrier \n");						
						break; 
					} 
					// printf("cannot eat but can move continuously\n");
				}		
			}
		}	
		// printf("\n\n");
	}
 // //    // see_gateMap(gateMap, total_gate);
 // //    see_qubitTime(qubitTime, total_qubit, total_gate);
	// // printf("-----------\n\n");


	// // // three qubit gate 從後面吃回來
	// printf("\nThree qubit Second\n"); // 由後面 trace 到後面，那就是前面的 gate 把後面吃掉，PASS	
	for (int q = 0; q < total_qubit; q++){
		// printf("q = %d\n", q);
		for (int g = total_gate-1; g > -1; g--){ 
			int gateIdx = qubitTime[q][g];
			int ctl_plus_tar = gateMap[gateIdx].numCtrls + gateMap[gateIdx].numTargs; 
			// printf("g = %d, gateMap[%d].action = %d, ctl_plus_tar= %d, \n", g, gateIdx, gateMap[gateIdx].action, ctl_plus_tar);
			if ((gateMap[gateIdx].action == 1) && (ctl_plus_tar == 3)){ // 找到一個 two qubit gate, 且是 applied 在該 qubit 的 target 上
				for (int k = g - 1; k >= 0; k--){ // specific-gate 前面的 gate，specific-gate 要被吃進去
					int post_gateIdx = qubitTime[q][k];
					int post_ctl_plus_tar = gateMap[post_gateIdx].numCtrls + gateMap[post_gateIdx].numTargs;
					// printf("specific qubit = %d, specific gate = %d, post_gate = %d, gateIdx = %d, post_gateIdx = %d ,ctl_plus_tar = %d, post_ctl_plus_tar = %d\n", q, g, k, gateIdx, post_gateIdx, ctl_plus_tar, post_ctl_plus_tar);

					if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 1)) { // 遇到 single, single 要反吃後面
						// printf("meet with single qubit gate, combine\n");
						three_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 1);
					} else if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 2)) { // 遇到 two qubit gate, two 要反吃後面
						// printf("meet with two qubit gate:\n");
						int same = three_check_same_qubit (gateMap, post_gateIdx, gateIdx); // 判斷能不能吃 (applied 在相同的 qubit 上)，能吃就吃不能吃就 break 掉
						int barrier = 0;
						if (same == 1){
							int inter_idx_0 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[1]) : (gateMap[g].ctrls[0]);
							int inter_idx_1 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[2]) : (gateMap[g].ctrls[1]); 
							int inter_idx_2 = gateMap[g].targs[0]; 
							for (int tmp_g = k+1; tmp_g < g; tmp_g++){ // 要小心這裡 index 會跟上面反過來
								if(qubitTime[inter_idx_0][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_1][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_2][tmp_g] != -1){
									barrier = 1;
								}							
							}								
						}	
						// printf("post_gateIdx = %d, same = %d\n", post_gateIdx, same);
						if (same == 1 && barrier == 0)	
							three_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 1);
						else 
							break;
					} else if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 3)) { // 遇到 two qubit gate, 可以吃就把它吃掉
						int same = three_check_same_qubit (gateMap, post_gateIdx, gateIdx); // 判斷能不能吃 (applied 在相同的 Gate 上)，能吃就吃不能吃就 break 掉
						// printf("post_gateIdx = %d, same = %d\n", post_gateIdx, same);
						int barrier = 0;
						if (same == 1){
							int inter_idx_0 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[1]) : (gateMap[g].ctrls[0]);
							int inter_idx_1 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[2]) : (gateMap[g].ctrls[1]); 
							int inter_idx_2 = gateMap[g].targs[0]; 
							for (int tmp_g = k+1; tmp_g < g; tmp_g++){ // 要小心這裡 index 會跟上面反過來
								if(qubitTime[inter_idx_0][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_1][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_2][tmp_g] != -1){
									barrier = 1;
								}							
							}							
						}						
						if (same == 1 && barrier == 0)	
							three_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 1);
						else
							break;						
					} else if ((gateMap[post_gateIdx].action == 1) && (commutative(gateMap, post_gateIdx, gateIdx) == 1)) {
						// printf("meet with barrier \n");						
						break; 
					} 
					// printf("cannot eat but can move continuously\n");
				}		
			}
		}	
	}
 // //    // see_gateMap(gateMap, total_gate);
 // //    see_qubitTime(qubitTime, total_qubit, total_gate);
	// // printf("-----------\n\n");

	// printf("\n\nTwo qubit Second\n"); // 由後面 trace 到後面，那就是前面的 gate 把後面吃掉，PASS
	for (int q = 0; q < total_qubit; q++){
		// printf("q = %d\n", q);
		for (int g = total_gate-1; g > -1; g--){ 
			int gateIdx = qubitTime[q][g];
			int ctl_plus_tar = gateMap[gateIdx].numCtrls + gateMap[gateIdx].numTargs; 
			// printf("g = %d, gateMap[%d].action = %d, ctl_plus_tar= %d, \n", g, gateIdx, gateMap[gateIdx].action, ctl_plus_tar);
			if ((gateMap[gateIdx].action == 1) && (ctl_plus_tar == 2)){ // 找到一個 two qubit gate,  
				for (int k = g - 1; k >= 0; k--){ 
					int post_gateIdx = qubitTime[q][k];
					int post_ctl_plus_tar = gateMap[post_gateIdx].numCtrls + gateMap[post_gateIdx].numTargs;
					// printf("specific qubit = %d, specific gate = %d, post_gate = %d, gateIdx = %d, post_gateIdx = %d ,ctl_plus_tar = %d, post_ctl_plus_tar = %d\n", q, g, k, gateIdx, post_gateIdx, ctl_plus_tar, post_ctl_plus_tar);

					if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 1)) { // 遇到 single, single 要反吃前面
						// printf("meet with single qubit gate, combine\n");
						two_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 1); // eater 放前面
					} 
					else if ((gateMap[post_gateIdx].action == 1) && (post_ctl_plus_tar == 2)) { // 遇到 two qubit gate, two 要反吃前面
						// printf("meet with two qubit gate:\n");
						int same = two_check_same_qubit (gateMap, post_gateIdx, gateIdx);
						int barrier = 0; // 同時要檢查，另一邊的 qubit 中間有沒有東西擋著
						if (same == 1){
							int inter_idx_0 = (gateMap[g].numCtrls == 0) ? (gateMap[g].targs[1]) : (gateMap[g].ctrls[0]);
							int inter_idx_1 = gateMap[g].targs[0]; 
							// printf("inter_idx_0 = %d, inter_idx_1 = %d, tmp_g = %d, k = %d\n", inter_idx_0, inter_idx_1, g+1, k);
							for (int tmp_g = k+1; tmp_g < g; tmp_g++){ // 要小心這裡 index 會跟上面反過來
								// printf("qubitTime[%d][%d] = %d\n", inter_idx_1, tmp_g, qubitTime[inter_idx_1][tmp_g]);
								if(qubitTime[inter_idx_0][tmp_g] != -1){
									barrier = 1;
								}
								if(qubitTime[inter_idx_1][tmp_g] != -1){
									barrier = 1;
								}
							}							
						}
						// printf("post_gateIdx = %d, same = %d, barrier = %d\n", post_gateIdx, same, barrier);
						if (same == 1 && barrier == 0)
							two_combine (gateMap, qubitTime, gateIdx, post_gateIdx, total_qubit, 1);
						else
							break;
					} else if ((gateMap[post_gateIdx].action == 1) && (commutative(gateMap, post_gateIdx, gateIdx) == 1)) {
						// printf("meet with barrier \n");						
						break; 
					} 
					// printf("cannot eat but can move continuously, commutative(gateMap, pre_gateIdx, gateIdx) == 1\n");
				}		
			}
		}	
	}
 //    see_gateMap(gateMap, total_gate);
 //    see_qubitTime(qubitTime, total_qubit, total_gate);
	// printf("-----------\n\n");


}

