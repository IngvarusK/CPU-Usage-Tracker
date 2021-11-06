.PHONY: all
all: compile clean

CC=gcc
CFLAGS = -Wall -std=gnu99	
	
compile:
	@echo "Compiling..."
        
	${CC} -c ${CFLAGS} main.c
	${CC} -c ${CFLAGS} readCPU/readCPU.c
	${CC} -g -pthread ${CFLAGS} main.o readCPU.o -o threads
	
clean:
	@echo "Cleaning up..."
	rm *.o
