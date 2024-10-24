#!/usr/bin/env python
# coding: utf-8
import numpy as np
from scipy.stats import ortho_group
import math

gate_which = 14
st_1o2 = 0.70710678118
PI = 3.14159265359

gate_num = [0,  1,  2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13]
numCtrls = [0,  0,  0, 0, 0, 0, 0, 0, 0,  1,  1,  1,  1,  2]
numTargs = [1,  2,  3, 1, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1]
val_num  = [4, 16, 64, 4, 4, 4, 4, 4, 4, 16, 16, 16, 16, 64]
real_matrix = [  
[0, 0, 0, 0], 
[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], 
[0, 0, 0, 0, 0, 0, 0, 0, 
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0],
[st_1o2, st_1o2, st_1o2, -st_1o2], 
[1, 0, 0, 0], 
[1, 0, 0, st_1o2], 
[0, 1, 1, 0], 
[0, 0, 0, 0], 
[1, 0, 0, -1], 
[[1, 0, 0, 0],
 [0, 1, 0, 0], 
 [0, 0, 0, 1],
 [0, 0, 1, 0]], 
[[1, 0, 0, 0],
 [0, 1, 0, 0], 
 [0, 0, 1, 0],
 [0, 0, 0, -1]],  
[[1, 0, 0, 0],
 [0, 1, 0, 0], 
 [0, 0, 1, 0],
 [0, 0, 0, 0]], 
[[1, 0, 0, 0],
 [0, 0, 1, 0], 
 [0, 1, 0, 0],
 [0, 0, 0, 1]],  
[[1, 0, 0, 0, 0, 0, 0, 0],
 [0, 1, 0, 0, 0, 0, 0, 0],
 [0, 0, 1, 0, 0, 0, 0, 0],
 [0, 0, 0, 1, 0, 0, 0, 0],
 [0, 0, 0, 0, 1, 0, 0, 0],
 [0, 0, 0, 0, 0, 1, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 1],
 [0, 0, 0, 0, 0, 0, 1, 0]]]

imag_matrix = [
[0, 0, 0, 0], 
[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], 
[0, 0, 0, 0, 0, 0, 0, 0, 
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0],
[0, 0, 0, 0], 
[0, 0, 0, 1], 
[0, 0, 0, st_1o2], 
[0, 0, 0, 0], 
[0, -1, 1, 0], 
[0, 0, 0, 0], 
[[0, 0, 0, 0],
 [0, 0, 0, 0], 
 [0, 0, 0, 0],
 [0, 0, 0, 0]],  
[[0, 0, 0, 0],
 [0, 0, 0, 0], 
 [0, 0, 0, 0],
 [0, 0, 0, 0]], 
[[0, 0, 0, 0],
 [0, 0, 0, 0], 
 [0, 0, 0, 0],
 [0, 0, 0, 0]], 
[[0, 0, 0, 0],
 [0, 0, 0, 0], 
 [0, 0, 0, 0],
 [0, 0, 0, 0]], 
[[0, 0, 0, 0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0, 0, 0, 0]]]

def gen_unitary (d):
	m = ortho_group.rvs(dim=d)
	# mm = np.matmul(m.T, m)
	# print(m)
	# print(mm)
	# print(kk)
	# print(kk[0][0])
	# print(kk[0][1])
	# print(kk[1][0])
	# print(kk[1][1])
	k = m
	return k

def row_col_exchange (dim, new_real, new_imag, idx1, idx2):
	#  row exchange
	tmp_real = [0]*dim
	tmp_imag = [0]*dim
	
	for i in range(dim):
		tmp_real[i] = new_real[idx1][i]
		tmp_imag[i] = new_imag[idx1][i]
	
	for i in range (dim):
		new_real[idx1][i] = new_real[idx2][i]
		new_imag[idx1][i] = new_imag[idx2][i]
	for i in range(dim):
		new_real[idx2][i] = tmp_real[i]
		new_imag[idx2][i] = tmp_imag[i]

	# col exchange
	for i in range(dim):
		tmp_real[i] = new_real[i][idx1]
		tmp_imag[i] = new_imag[i][idx1]
	
	for i in range (dim):
		new_real[i][idx1] = new_real[i][idx2]
		new_imag[i][idx1] = new_imag[i][idx2]
	for i in range(dim):
		new_real[i][idx2] = tmp_real[i]
		new_imag[i][idx2] = tmp_imag[i]


def uni_matirx_reformate(dim, ori_real, ori_imag, t1, t2, t3):	# 三個都是 unitary matrix
	# print("into uni_matirx_reformate: " + "t1 = " + str(t1) +  ", t2 = " + str(t2) +  ", t3 = " + str(t3))
	# print("original matrix:")
	# print(ori_real)
	# print(ori_imag)

	new_real = [[0]*dim]*dim
	new_imag = [[0]*dim]*dim
	for i in range (dim):
		new_real[i] = ori_real[i]
		new_imag[i] = ori_imag[i]

	# print("Initial duplication")
	# for i in range (dim):
	# 	for j in range (dim):
	# 		print(new_real[i][j], end =" ")
	# 	print()
	# print()
	# for i in range (dim):
	# 	for j in range (dim):
	# 		print(new_imag[i][j], end =" ")
	# 	print()

	# single qubit gate no need to reformate

	# two qubit gate reformate
	if (dim == 4) and (t1 > t2):
		# order: 1 3 2 4, row exchange first, column exchange (1-based), hence, exchange row of 1 and 2 (0-based) 
		row_col_exchange (dim, new_real, new_imag, 1, 2) # test success
		return new_real, new_imag

	# three qubit gate reformate
	if dim == 8:
		if ((t1 < t3) and (t3 < t2)):
			# print("case 1")
			row_col_exchange (dim, new_real, new_imag, 1, 2) 
			row_col_exchange (dim, new_real, new_imag, 5, 6) 
			return new_real, new_imag
		elif ((t2 < t1) and (t1 < t3)):
			# print("case 2")
			row_col_exchange (dim, new_real, new_imag, 2, 4) 
			row_col_exchange (dim, new_real, new_imag, 3, 5) 
			return new_real, new_imag

		elif ((t2 < t3) and (t3 < t1)):
			# print("case 3")
			row_col_exchange (dim, new_real, new_imag, 1, 2) 
			row_col_exchange (dim, new_real, new_imag, 1, 4) 
			row_col_exchange (dim, new_real, new_imag, 5, 6) 
			row_col_exchange (dim, new_real, new_imag, 3, 6) 
			return new_real, new_imag

		elif ((t3 < t1) and (t1 < t2)):
			# print("case 4")
			row_col_exchange (dim, new_real, new_imag, 1, 2) 
			row_col_exchange (dim, new_real, new_imag, 2, 4) 
			row_col_exchange (dim, new_real, new_imag, 5, 6) 
			row_col_exchange (dim, new_real, new_imag, 3, 5) 
			return new_real, new_imag

		elif ((t3 < t2) and (t2 < t1)):
			# print("case 5")
			row_col_exchange (dim, new_real, new_imag, 1, 4) 
			row_col_exchange (dim, new_real, new_imag, 3, 6) 
			return new_real, new_imag

	return new_real, new_imag

def print_uni_matrix (dim, targetQubit_1, targetQubit_2, targetQubit_3):
	tmp_real_matrix = gen_unitary(dim)
	tmp_imag_matrix = [[0]*dim]*dim
	new_real, new_imag = uni_matirx_reformate(dim, tmp_real_matrix, tmp_imag_matrix, targetQubit_1, targetQubit_2, targetQubit_3)

	# sorting index
	if dim == 8:
		small = targetQubit_1
		mediu = targetQubit_2
		large = targetQubit_3
		if targetQubit_2 > targetQubit_3:
			tmp = targetQubit_2
			targetQubit_2 = targetQubit_3
			targetQubit_3 = tmp
			if (targetQubit_1 > targetQubit_2):
				tmp = targetQubit_1
				targetQubit_1 = targetQubit_2
				targetQubit_2 = tmp

	if dim == 2:
		print(str(gate_num[0]) + ' ' + str(numCtrls[0]) + ' ' + str(numTargs[0]) + ' ' + str(val_num[0]) + ' ' + str(targetQubit_1) + ' ', end='   ') # skip controlQubit 
	elif dim == 4:
		print(str(gate_num[1]) + ' ' + str(numCtrls[1]) + ' ' + str(numTargs[1]) + ' ' + str(val_num[1]) + ' ' + str(targetQubit_1) + ' ' + str(targetQubit_2) + ' ', end='   ') # skip controlQubit 
	else:
		print(str(gate_num[2]) + ' ' + str(numCtrls[2]) + ' ' + str(numTargs[2]) + ' ' + str(val_num[2]) + ' ' + str(targetQubit_1) + ' ' + str(targetQubit_2) + ' ' + str(targetQubit_3) + ' ', end='   ') # skip controlQubit 

	# print("New matrix:")
	# for i in range (dim):
	# 	for j in range (dim):
	# 		print(new_real[i][j], end =" ")
	# 	print()
	# for i in range (dim):
	# 	for j in range (dim):
	# 		print(new_imag[i][j], end =" ")
	# 	print()
	# print("\ntesting over\n\n\n")

	for i in range (dim):
		for j in range (dim):
			print(new_real[i][j], end=' ')
	# 	print()
	# print()
	for i in range (dim):
		for j in range (dim):
			print('0 ', end=' ')
		# print()
	print()

def print_stable_single (gate, targetQubit):
	print(str(gate_num[gate]) + ' ' + str(numCtrls[gate]) + ' ' + str(numTargs[gate]) + ' ' + str(val_num[gate]) + ' ' + str(targetQubit) + ' ', end='   ') # skip controlQubit 	
	for i in range (len(real_matrix[gate])):
		print(real_matrix[gate][i], end=' ')	
	for i in range (len(imag_matrix[gate])):
		print(imag_matrix[gate][i], end=' ')	

def print_stable (gate, dim, controlQubit, controlQubit_2, targetQubit, PhaseShift, angle):
	g_real_matrix = [[0]*dim]*dim
	g_imag_matrix = [[0]*dim]*dim

	for i in range (dim):
		g_real_matrix[i] = real_matrix[gate][i].copy()
		g_imag_matrix[i] = imag_matrix[gate][i].copy()
	

	if controlQubit_2 == -1: # two qubit gate
		new_real, new_imag = uni_matirx_reformate(dim, g_real_matrix, g_imag_matrix, controlQubit, targetQubit, -1)
	else: # three qubit gate
		new_real, new_imag = uni_matirx_reformate(dim, g_real_matrix, g_imag_matrix, controlQubit, controlQubit_2, targetQubit)

	if PhaseShift == True:
		new_real[0][0] = angle # 直接把 anele 傳進去，這樣後面 simulator 可以直接接數字
		# new_imag[dim-1][dim-1] = math.sin(angle)
	

	# sorting index
	if controlQubit_2 != -1: # three qubit gate
		if controlQubit > controlQubit_2:
			tmp = controlQubit
			controlQubit = controlQubit_2
			controlQubit_2 = tmp

	if controlQubit_2 == -1: # two qubit gate
		print(str(gate_num[gate]) + ' '+ str(numCtrls[gate])  + ' '+ str(numTargs[gate]) + ' ' + str(val_num[gate]) + ' ' + str(controlQubit) + ' ' + str(targetQubit) + ' ', end='   ') 
	else: # three qubit gate
		print(str(gate_num[gate]) + ' '+ str(numCtrls[gate])  + ' '+ str(numTargs[gate]) + ' ' + str(val_num[gate]) + ' ' + str(controlQubit) + ' ' + str(controlQubit_2) + ' ' + str(targetQubit) + ' ', end='   ') 

	# print()
	for i in range (dim):
		for j in range (dim):
			print(new_real[i][j], end=' ')
	# 	print()
	# print()	
	for i in range (dim):
		for j in range (dim):
			print(new_imag[i][j], end=' ')	
		# print()
	# print()


def unitary (targetQubit):
	print_uni_matrix(2, targetQubit, -1, -1)
	print()

def twoQubitUnitary (targetQubit_1, targetQubit_2):
	print_uni_matrix(4, targetQubit_1, targetQubit_2, -1)
	print()

def multiQubitUnitary (targetQubit_1, targetQubit_2, targetQubit_3):
	print_uni_matrix(8, targetQubit_1, targetQubit_2, targetQubit_3)
	print()

def hadamard (targetQubit):
	print_stable_single(3, targetQubit)
	print()

def sGate (targetQubit):
	print_stable_single(4, targetQubit)
	print()

def tGate (targetQubit):
	print_stable_single(5, targetQubit)
	print()

def pauliX (targetQubit):
	print_stable_single(6, targetQubit)
	print()

def pauliY (targetQubit):
	print_stable_single(7, targetQubit)
	print()

def pauliZ (targetQubit):
	print_stable_single(8, targetQubit)
	print()

def controlledNot (controlQubit, targetQubit):
	print_stable(9, 4, controlQubit, -1, targetQubit, False, 0)
	print()

def controlledPhaseFlip (controlQubit, targetQubit):
	print_stable(10, 4, controlQubit, -1, targetQubit, False, 0)
	print()

def controlledPhaseShift (controlQubit, targetQubit, angle):
	# 要換 angle 在這裡
	print_stable(11, 4, controlQubit, -1, targetQubit, True, angle)
	print()

def swapGate (controlQubit, targetQubit):
	print_stable(12, 4, controlQubit, -1, targetQubit, False, 0)
	print()

def multiControlledMultiQubitNot (controlQubit_1, controlQubit_2, targetQubit):
	# make the controlQubit in order
	if (controlQubit_1 > controlQubit_2):
		tmp = controlQubit_1
		controlQubit_1 = controlQubit_2
		controlQubit_2 = tmp
	# print(controlQubit_1, controlQubit_2, targetQubit)
	print_stable(13, 8, controlQubit_1, controlQubit_2, targetQubit, False, 0)
	print()
	


# testing
# k = gen_unitary(2)
# print(k)
# unitary(0)
# twoQubitUnitary(1, 0)
# print("case 0")
# multiQubitUnitary(0, 1, 2)
# print("case 1")
# multiQubitUnitary(0, 2, 1)
# print("case 2")
# multiQubitUnitary(1, 0, 2)
# print("case 3")
# multiQubitUnitary(2, 0, 1)
# print("case 4")
# multiQubitUnitary(1, 2, 0)
# print("case 5")
# multiQubitUnitary(2, 1, 0)
# print("-----------")
# print(y)
# hadamard(0)
# print("\n\ncontrolledNot")
# controlledNot(0, 1) 
# controlledNot(1, 0) 
# controlledNot(0, 1) 
# print("\n\ncontrolledPhaseFlip")
# controlledPhaseFlip(1, 0)
# print("\n\ncontrolledPhaseShift")
# controlledPhaseShift(1, 0, PI/4)
# print("\n\nswapGate")
# swapGate(1, 0)

# print("\n\nToffoli")
# print("case 0")
# multiControlledMultiQubitNot(0, 1, 2)
# print("case 2")
# multiControlledMultiQubitNot(1, 0, 2)
# print("case 1")
# multiControlledMultiQubitNot(0, 2, 1)
# # print("case 3")
# multiControlledMultiQubitNot(2, 0, 1)
# # print("case 4")
# multiControlledMultiQubitNot(1, 2, 0)
# # print("case 5")
# multiControlledMultiQubitNot(2, 1, 0)


# # 1.txt
# total_qubit = 13
# total = 14
# print(str(total_qubit) + ' ' + str(total ))

# unitary(0)
# twoQubitUnitary(0, 1)
# multiQubitUnitary(0, 1, 2)
# hadamard(3)
# sGate(4)
# tGate(5)
# pauliX(6) 
# pauliY(7) 
# pauliZ(8) 
# controlledNot(0, 9) 
# controlledPhaseFlip(0, 10)
# controlledPhaseShift(0, 11, PI/4)
# swapGate(0, 12)
# multiControlledMultiQubitNot(0, 1, 13)


# # # # # 2.txt，正確性已經 PASS
# total_qubit = 3
# total = 6
# print(str(total_qubit) + ' ' + str(total ))

# hadamard(0)
# pauliX(0) 
# pauliY(0) 
# hadamard(1)
# pauliX(1) 
# hadamard(2)

# # 3.txt，正確性已經 PASS
# total_qubit = 3
# total = 8
# print(str(total_qubit) + ' ' + str(total ))

# hadamard(0)
# hadamard(1)
# hadamard(2)
# controlledNot(0, 1) 

# pauliX(0) 
# pauliX(1) 
# controlledNot(0, 1) 

# pauliY(0) 


# # # # # 4.txt，正確性已經 PASS
total_qubit = 2
total = 8
print(str(total_qubit) + ' ' + str(total ))


controlledNot(0, 1)
pauliX(0) 
controlledNot(1, 0)
pauliY(0) 
controlledNot(0, 1)
controlledNot(0, 1)
pauliZ(0)
controlledNot(1, 0)


# 5.txt，正確性已經 PASS
# total_qubit = 5
# total = 14
# print(str(total_qubit) + ' ' + str(total ))

# pauliX(0) #0
# controlledNot(0, 1) # 1
# pauliY(0) # 2

# controlledNot(4, 2)  # 3 
# pauliZ(4) # 4 
# controlledNot(4, 2) # 5

# controlledNot(0, 2)  #6
# pauliZ(0) # 7
# controlledNot(0, 1) # 8
# pauliX(3) # 9
# pauliX(0) # 10

# controlledNot(4, 2) # 11
# controlledNot(0, 2) # 12
# pauliY(0) # 13



# # 6.txt，正確性已經 PASS
# total_qubit = 3
# total = 5
# print(str(total_qubit) + ' ' + str(total ))

# multiControlledMultiQubitNot(0, 1, 2)
# multiControlledMultiQubitNot(0, 1, 2)
# multiControlledMultiQubitNot(0, 1, 2)
# multiControlledMultiQubitNot(0, 1, 2)
# multiControlledMultiQubitNot(0, 1, 2)

# # # # # # 7.txt，正確性已經 PASS
# total_qubit = 5
# total = 17
# print(str(total_qubit) + ' ' + str(total ))

# pauliX(0) # 0
# pauliX(0) # 1
# multiControlledMultiQubitNot(1, 3, 2) # 2
# pauliX(4) # 3
# controlledNot(0, 1) # 4
# controlledNot(4, 3) # 5
# pauliX(4) # 6
# multiControlledMultiQubitNot(0, 1, 2) # 7
# controlledNot(4, 3) # 8
# pauliX(1) # 9
# pauliX(1) # 10
# pauliX(2) # 11
# pauliX(4) # 12
# multiControlledMultiQubitNot(2, 1, 0) # 13
# controlledNot(3, 4) # 14
# pauliX(0) # 15
# pauliX(4) # 16


# # # 8.txt，正確性已經 PASS
# total_qubit = 3
# total = 3
# print(str(total_qubit) + ' ' + str(total ))
# multiControlledMultiQubitNot(0, 1, 2)
# pauliX(2)
# multiControlledMultiQubitNot(2, 1, 0)

# # # 9.txt
# total_qubit = 2
# total = 3
# print(str(total_qubit) + ' ' + str(total ))

# twoQubitUnitary(0, 1)
# pauliX(1)
# twoQubitUnitary(1, 0)

# # # # 10.txt 跑 single qubit gate 測資的那個
# total_qubit = 21
# total = 21
# print(str(total_qubit) + ' ' + str(total ))

# for i in range (total):
# 	pauliX(i)

#### 11.txt CNOT
# total_qubit = 31
# total = 465
# print(str(total_qubit) + ' ' + str(total ))

# for i in range (total_qubit):
# 	for j in range (i):
# 		controlledNot(i, j)

# ### 12.txt，正確性已經 PASS
# total_qubit = 2
# total = 3
# print(str(total_qubit) + ' ' + str(total ))
# controlledNot(0, 1)
# pauliX(1)
# controlledNot(0, 1)



# ### random gate generator
import random

N = 31
gate_cnt = 100
total_qubit = N
total = gate_cnt

print(str(total_qubit) + ' ' + str(total ))

gate = [random.randint(0, 1) for p in range(0, gate_cnt)] # 共有 14 種 gate 

i = 0
# for i in range (gate_cnt):
while i < gate_cnt:
	q_1 = random.randrange(N-1)
	q_2 = random.randrange(N-1)
	q_3 = random.randrange(N-1)

	if (q_1 != q_2) and (q_1 != q_3) and (q_2 != q_3):
		if gate[i] == 0:
			unitary(q_1)		
		elif  gate[i] == 1:
			twoQubitUnitary(q_1, q_2)	
		elif  gate[i] == 2:
			hadamard(q_1)
		elif  gate[i] == 3:
			multiQubitUnitary(q_1, q_2, q_3)		
		elif  gate[i] == 4:
			hadamard(q_1)
		elif  gate[i] == 5:
			sGate(q_1)
		elif  gate[i] == 6:
			tGate(q_1)
		elif  gate[i] == 7:
			pauliX(q_1) 
		elif  gate[i] == 8:
			pauliY(q_1) 
		elif  gate[i] == 9:
			pauliZ(q_1) 
		elif  gate[i] == 10:
			controlledNot(q_1, q_2) 
		elif  gate[i] == 11:		
			controlledPhaseFlip(q_1, q_2)
		elif  gate[i] == 12:
			controlledPhaseShift(q_1, q_2, PI/4)
		elif  gate[i] == 13:
			swapGate(q_1, q_2)
		elif  gate[i] == 14:
			multiControlledMultiQubitNot(q_1, q_2, q_3)
		i += 1



