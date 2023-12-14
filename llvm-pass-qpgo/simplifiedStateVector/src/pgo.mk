CC:=~/.llvm/bin/clang
MODE:=context
FILE:=default.profdata
SHARED_CFLAGS:=-O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
CLANG_PGO_CFLAGS:=$(SHARED_CFLAGS) -Qunused-arguments -Xclang -load -Xclang /home/balalalalau/QOPS/llvm-pass-qpgo/build/qpgo/libQpgoPass.so -mllvm -profile-gen=$(FILE) -mllvm -profile-mode=$(MODE)

all:
	$(MAKE) -f makefile CC='$(CC)' CFLAGS='$(CLANG_PGO_CFLAGS)'

clang:
	$(MAKE) -f makefile CC='$(CC)' CFLAGS='$(SHARED_CFLAGS)'

gcc:
	$(MAKE) -f makefile CC='gcc' CFLAGS='$(SHARED_CFLAGS)'

run:
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/balalalalau/.llvm/lib QPGO_PROFILE_FILE=$(FILE) ./qSim.out

emit:
	$(MAKE) -f makefile CC='$(CC)' CFLAGS='$(CLANG_PGO_CFLAGS) -S -emit-llvm'

clean:
	$(MAKE) -f makefile clean

