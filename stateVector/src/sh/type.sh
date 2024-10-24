#! /bin/bash

for N in {26..35..1}
do
    NG=$((2**$G))
    for n in {0..($N-1)..1}
    do
        echo "N=$N, H gates on q$n.\n"
        echo "N=$N, H gates on q$n." >> res/type/$N.out
        ./qSim.out -c circuit/type/Hq$NG.txt -i ini/type/$N.ini >> res/type/$N.out
        echo "=======================================================" >> res/type/$N.out
    done
done
    

# << 'MULTILINE-COMMENT'
# MULTILINE-COMMENT

