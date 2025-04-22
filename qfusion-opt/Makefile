CXX:=g++
CXXFLAGS:=-std=c++17 -O3 -march=native -flto=auto -funroll-loops -pipe -Wall -Wextra -Wpedantic

TEST:=qv32.txt
QUBIT:=32
NUM=10
MODE=3

fusion: fusion.cpp
	${CXX} ${CXXFLAGS} -o $@ $^

# fusion_ori: fusion_ori.cpp
# 	${CXX} ${CXXFLAGS} -o $@ $^

run: fusion
	@for i in `seq 1 ${NUM}`; do \
		./fusion ./circuit/${TEST} ./tmp/${TEST} 3 ${QUBIT} ${MODE}; \
	done;

# check: fusion fusion_ori
# 	@./fusion ./circuit/${TEST} ./tmp/${TEST} 3 ${QUBIT} ${MODE};
# 	@./fusion_ori ./circuit/${TEST} ./tmp/ori_${TEST} 3 ${QUBIT} ${MODE};
# 	@(git --no-pager diff --no-index --color-words ./tmp/${TEST} ./tmp/ori_${TEST} && echo "\npassed!") || echo "\nfailed!"

clean:
	rm fusion