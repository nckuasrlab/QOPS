all: quokka
CC:=gcc
INCLUDE:=.
CFLAGS:=-g -O3
OMPFLAGES:=-fopenmp

quokka: main.o ini.o init.o common.o gate.o
	$(CC) $(CFLAGS) $(OMPFLAGES) $^ -o $@ -lm
# -lm for math.h

main.o: main.c init.h common.h gate.h
	$(CC) -I$(INCLUDE) -c $(CFLAGS) $(OMPFLAGES) main.c

init.o: init.c init.h common.h gate.h
	$(CC) -c $(CFLAGS) $(OMPFLAGES) init.c

common.o: common.c common.h gate.h
	$(CC) -c $(CFLAGS) $(OMPFLAGES) common.c

gate.o: gate.c common.h gate.h
	$(CC) -c $(CFLAGS) $(OMPFLAGES) gate.c

ini.o: ini.c ini.h
	$(CC) -c $(CFLAGS) ini.c

clean:
	rm -f quokka main.o gate.o ini.o init.o common.o
