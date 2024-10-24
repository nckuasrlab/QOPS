#! /bin/bash

for G in {0..5..1}
do
    NG=$((2**$G))
    for N in {26..35..1}
    do
        echo "N=$N, $NG gates.\n"
        echo "N=$N, $NG gates." >> res/1qmg/$N.out
        ./qSim.out -c circuit/gate/g$NG.txt -i ini/One_qbit_multi_gate/$N.ini >> res/1qmg/$N.out
        echo "=======================================================" >> res/1qmg/$N.out
    done
done
    

# << 'MULTILINE-COMMENT'
# MULTILINE-COMMENT

