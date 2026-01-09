CXX:=g++
CXXFLAGS:=-std=c++2a -O3 -march=native -flto=auto -funroll-loops -fno-rtti -fno-exceptions -pipe -Wall -Wextra -Wpedantic
GDB_CXXFLAGS:=-std=c++2a -g -O0 -pipe -Wall -Wextra -Wpedantic

TEST:=qv32.txt
QUBIT:=32
NUM=10
MODE=3

# Extract major version number of g++
GXX_VERSION := $(shell $(CXX) -dumpversion | cut -d. -f1)

# Check version at parse time
ifeq ($(shell [ $(GXX_VERSION) -ge 10 ] && echo ok),)
$(error g++ version >= 10 is required, but found $(GXX_VERSION))
endif

fusion: fusion.cpp
	${CXX} ${CXXFLAGS} -o $@ $^ -lpthread

# fusion_ori: fusion_ori.cpp
# 	${CXX} ${CXXFLAGS} -o $@ $^

run: fusion
	@for i in `seq 1 ${NUM}`; do \
		./fusion ./circuit/${TEST} ./tmp/${TEST} 3 ${QUBIT} ${MODE}; \
	done;

gdb: fusion.cpp
	${CXX} ${GDB_CXXFLAGS} -o fusion $^ -lpthread
	DYNAMIC_COST_FILENAME=./log/gate_exe_time_aer.csv gdb -q --args ./fusion ./circuit/sc32.txt ./xxx.txt 5 32 8

perf: fusion.cpp
	${CXX} ${CXXFLAGS} -fno-omit-frame-pointer -o fusion $^ -lpthread
	DYNAMIC_COST_FILENAME=./log/gate_exe_time_aer.csv perf record -F 99 -g ./fusion ./circuit/sc32.txt ./xxx.txt 5 32 8
	perf script > out.perf
	./stackcollapse-perf.pl out.perf > out.folded
	./flamegraph.pl out.folded > flamegraph.svg

clean:
	rm -f fusion