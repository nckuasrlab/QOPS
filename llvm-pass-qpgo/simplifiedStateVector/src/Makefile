all: qSim.out
CC:=gcc
INCLUDE:=.
CFLAGS:=-g -O0 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
OMPFLAGES:=-fopenmp

qSim.out: main.o common.o gate.o
	$(CC) $(CFLAGS) $(OMPFLAGES) $(LDFLAGS) $^ -o $@ -lm
# -lm for math.h

main.o: main.c common.h gate.h
	$(CC) -I$(INCLUDE) -c $(CFLAGS) $(OMPFLAGES) main.c

# init.o: init.c init.h common.h gate.h
# 	$(CC) -c $(CFLAGS) $(OMPFLAGES) init.c

common.o: common.c common.h gate.h
	$(CC) -c $(CFLAGS) $(OMPFLAGES) common.c

gate.o: gate.c common.h gate.h
	$(CC) -c $(CFLAGS) $(OMPFLAGES) gate.c

# gate_util.o: gate_util.c gate_util.h common.h gate.h
# 	$(CC) -c $(CFLAGS) $(OMPFLAGES) gate_util.c
#
# gate_chunk.o: gate_chunk.c gate_chunk.h common.h gate_util.h
# 	$(CC) -c $(CFLAGS) $(OMPFLAGES) gate_chunk.c

# measure.o: measure.c measure.h common.h gate.h gate_util.h gate_chunk.h
# 	$(CC) -c $(CFLAGS) $(OMPFLAGES) measure.c
#
# ini.o: ini.c ini.h
# 	$(CC) -c $(CFLAGS) ini.c

.pgo:
	$(eval CC = ~/.llvm/bin/clang)
	$(eval LLVM_PROFDATA = llvm-profdata)
	$(eval LDFLAGS += -fopenmp=libomp)
	$(eval CFLAGS += -Xclang -load -Xclang /home/nckucsieserver/pro/selfpro/llvm-pass-qpgo/build/qpgo/libQpgoPass.so)

# $(eval CFLAGS += -S -emit-llvm)
# $(eval CFLAGS += -fprofile-instr-generate -Xclang -load -Xclang /home/nckucsieserver/pro/selfpro/learn_llvm/llvm-pass-inject/build/inject/libInjectPass.so)
# $(eval LDFLAGS += -fprofile-instr-generate -fopenmp=libomp)

pgo: clean .pgo qSim.out
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/nckucsieserver/.llvm/lib ./qSim.out

.pgol:
	$(eval CC = ~/.llvm/bin/clang)
	$(eval LLVM_PROFDATA = llvm-profdata)
	$(eval LDFLAGS += -fopenmp=libomp)
	$(eval CFLAGS += -Xclang -load -Xclang /home/nckucsieserver/pro/selfpro/llvm-pass-qpgo/build/qpgo/libQpgoPass.so)
	$(eval CFLAGS += -S -emit-llvm)

pgol: clean .pgol qSim.out
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/nckucsieserver/.llvm/lib ./qSim.out
clean:
	rm -f qSim.out *.o *.ll

