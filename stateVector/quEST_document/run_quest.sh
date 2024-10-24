#!/bin/bash

for n in {26..31..1}
do
	for s in {1..4..1} #2(1)-16(4) 
	do
		ST=$((2**$s))
		cmake .. -DUSER_SOURCE="./source_file/source$n.c"  -DCMAKE_C_COMPILER=gcc-6  -DOUTPUT_EXE="myExecutable"  -DPRECISION=1  -DMULTITHREADED=$ST
		make 
		export OMP_NUM_THREADS=$ST
		./myExecutable

	done
done