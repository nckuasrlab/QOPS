# QuEST

* QuEST code
* build QuEST: ref their github
---
# compile
cmake .. -DUSER_SOURCE="source1.c"  -DCMAKE_C_COMPILER=gcc-6  -DOUTPUT_EXE="myExecutable" -DMULTITHREADED=1
make
./myExecutable
