#!/bin/bash

# qubits = 20
for s in {0..4..1}
do 
	for ct in {12..20..1}
	do
		STM=$((2**$s))ULL
		CT=$((2**$ct))ULL
		echo CHUNKSTATE = $CT, STREAM = $STM
		echo 20 $CT $STM
		nvcc -D N=20 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N20-$CT-$STM
		./exeF/N20-$CT-$STM > ./1204_res/N20-$CT-$STM.out
	done
done

# # qubits = 21
# for s in {0..4..1}
# do 
# 	for ct in {12..21..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 21 $CT $STM
# 		nvcc -D N=21 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N21-$CT-$STM
# 		./exeF/N21-$CT-$STM > ./1204_res/N21-$CT-$STM.out
# 	done
# done

# #qubits = 22
# for s in {0..4..1}
# do 
# 	for ct in {12..22..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 22 $CT $STM
# 		nvcc -D N=22 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N22-$CT-$STM
# 		./exeF/N22-$CT-$STM > ./1204_res/N22-$CT-$STM.out
# 	done
# done

# #qubits = 23
# for s in {0..4..1}
# do 
# 	for ct in {12..23..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 23 $CT $STM
# 		nvcc -D N=23 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N23-$CT-$STM
# 		./exeF/N23-$CT-$STM > ./1204_res/N23-$CT-$STM.out
# 	done
# done

# #qubits = 24
# for s in {0..4..1}
# do 
# 	for ct in {12..24..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 24 $CT $STM
# 		nvcc -D N=24 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N24-$CT-$STM
# 		./exeF/N24-$CT-$STM > ./1204_res/N24-$CT-$STM.out
# 	done
# done

# #qubits = 25
# for s in {0..4..1}
# do 
# 	for ct in {12..25..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 25 $CT $STM
# 		nvcc -D N=25 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N25-$CT-$STM
# 		./exeF/N25-$CT-$STM > ./1204_res/N25-$CT-$STM.out
# 	done
# done

# #qubits = 26
# for s in {0..4..1}
# do 
# 	for ct in {12..26..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 26 $CT $STM
# 		nvcc -D N=26 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N26-$CT-$STM
# 		./exeF/N26-$CT-$STM > ./1204_res/N26-$CT-$STM.out
# 	done
# done

# #qubits = 27
# for s in {0..4..1}
# do 
# 	for ct in {12..27..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 27 $CT $STM
# 		nvcc -D N=27 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N27-$CT-$STM
# 		./exeF/N27-$CT-$STM > ./1204_res/N27-$CT-$STM.out
# 	done
# done

# #qubits = 28
# for s in {0..4..1}
# do 
# 	for ct in {12..28..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 28 $CT $STM
# 		nvcc -D N=28 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N28-$CT-$STM
# 		./exeF/N28-$CT-$STM > ./1204_res/N28-$CT-$STM.out
# 	done
# done

# #qubits = 29
# for s in {0..4..1}
# do 
# 	for ct in {12..29..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 29 $CT $STM
# 		nvcc -D N=29 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N29-$CT-$STM
# 		./exeF/N29-$CT-$STM > ./1204_res/N29-$CT-$STM.out
# 	done
# done

# #qubits = 30
# for s in {0..4..1}
# do 
# 	for ct in {12..30..1}
# 	do
# 		STM=$((2**$s))ULL
# 		CT=$((2**$ct))ULL
# 		echo CHUNKSTATE = $CT, STREAM = $STM
# 		echo 30 $CT $STM
# 		nvcc -D N=30 -D CHUNKSTATE=$CT -D STREAM=$STM gateBased_disk.cu -o ./exeF/N30-$CT-$STM
# 		./exeF/N30-$CT-$STM > ./1204_res/N30-$CT-$STM.out
# 	done
# done

