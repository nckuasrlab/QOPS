#! /bin/bash

# SOURCE=[main.c circuit.c gate.c init.c]
NGQB=3
F=$((2**$NGQB))
for N in {20..20..1}
do
        for NSQB in {6..6..1} #2(1)-16(4)
        do
                ST=$((2**$NSQB))
                for NLQB in {10..10..1} # 1024 - 8192
                do
                        CT=$((2**$NLQB))
                        echo N$N-F$F-S$ST-CT$CT
                        # sudo sh -c 'echo 1 > /proc/sys/vm/drop_caches'
                        gcc -g -fopenmp -fcommon -D N=$N -D NGQB=$NGQB -D NSQB=$NSQB -D NLQB=$NLQB main.c circuit.c gate.c init.c -O3 -lm -w -o exe/N$N-F$F-S$ST-CT$CT
                        for i in {1..4..1}
                        do
                                echo $i
                                ./exe/N$N-F$F-S$ST-CT$CT ./circuit/3.txt ./path/set6.txt
                        done
                done
        done
done

# << 'MULTILINE-COMMENT'
# MULTILINE-COMMENT

