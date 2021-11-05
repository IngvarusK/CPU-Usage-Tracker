CC=gcc
CFLAGS=-I.
ccflags-y := -std=gnu99	
	
compile:
	${CC} -c main.c
	${CC} -c readCPU/readCPU.c
	${CC} -g -pthread main.o readCPU.o -o threads
	rm *.o
