CC=gcc
CFLAGS=-lpthread

monte: circleMonte.o
	$(CC) -o circleMonte circleMonte.o $(CFLAGS)
