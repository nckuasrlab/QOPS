#! /bin/bash

# SOURCE=[main.c circuit.c gate.c init.c]
N=8
NGQB=3
NSQB=4
NLQB=2
F=$((2**$NGQB))
ST=$((2**$NSQB))
CT=$((2**$NLQB))
echo N$N-F$F-S$ST-CT$CT
# sudo sh -c 'echo 1 > /proc/sys/vm/drop_caches'
gcc -g -fopenmp -fcommon -D N=$N -D NGQB=$NGQB -D NSQB=$NSQB -D NLQB=$NLQB main.c circuit.c gate.c init.c -O3 -lm -w -o exe/N$N-F$F-S$ST-CT$CT

./exe/N$N-F$F-S$ST-CT$CT $1 $2
python3 test_by_qiskit.py $1 $2 $N $NGQB $NSQB $NLQB

# << 'MULTILINE-COMMENT'
# MULTILINE-COMMENT

