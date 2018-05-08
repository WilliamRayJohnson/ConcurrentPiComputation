CC=gcc
CFLAGS=-lpthread -lm

monte: circleMonte.o
	$(CC) -o circleMonte circleMonte.o $(CFLAGS)
