/**
 * A concurrent program that uses Monte Carlo simulations to approximate the
 * value of pi
 * Author: William Ray Johnson
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <thread_count> <iteration_count> <delta>\n", argv[0]);
        exit(-1);
    }

    exit(0);
}
