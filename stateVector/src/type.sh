#! /bin/bash

for N in {26..35..1}
do
    n1=$(($N-1))
    for n in {0..$n1..1}
    do
        echo "N=$N, H gates on q$n."
        echo "N=$N, H gates on q$n." >> res/type/$N.out
        ./qSim.out -c circuit/type_test/Hq$n.txt -i ini/type/$N.ini >> res/type/$N.out
        echo "=======================================================" >> res/type/$N.out
    done
done
    

# << 'MULTILINE-COMMENT'
# MULTILINE-COMMENT

