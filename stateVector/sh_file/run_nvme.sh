#!/bin/bash


# for s in {1..4..1} #2(1)-16(4) 
# do
# 	ST=$((2**$s))ULL
# 	echo STREAM = ${m}
# 	for n in {25..32..1}
# 	do
# 	        for ct in {10..14..1} # 1024 - 16384
# 	        do
# 	                CT=$((2**$ct))ULL
# 	                gcc -g -fopenmp -D N=$n -D CHUNKSTATE=$CT -D STREAM=$ST  2NVMe.c -O3 -w
# 	                ./a.out
# 	        done
# 	done

# done


for n in {31..32..1}
do
	for s in {1..4..1} #2(1)-16(4) 
	do
		ST=$((2**$s))ULL
		echo STREAM = ${m}        
	        for ct in {12..13..1} # 1024 - 16384
	        do
	                CT=$((2**$ct))ULL
	                gcc -g -fopenmp -D N=$n -D CHUNKSTATE=$CT -D STREAM=$ST  2NVMe.c -O3 -w
	                ./a.out
	        done
	done
done