/**
 * A concurrent program that uses Monte Carlo simulations to approximate the
 * value of pi
 * Author: William Ray Johnson
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *monteWorker(void *arg);
void *monteControl(void *arg);

typedef struct workerInput {
    int experimentCount;
    int thdId;
} Worker_Input;

typedef struct atomic_i {
    int value;
    pthread_mutex_t *lock;
} Atomic_Int;

Atomic_Int *successCount;
Atomic_Int *totalCount;

int main(int argc, char *argv[]) {
    int threadCount;
    int iterationCount;
    double delta;
    int i;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <thread_count> <iteration_count> <delta>\n", argv[0]);
        exit(-1);
    }

    sscanf(argv[1], "%d", &threadCount);
    sscanf(argv[2], "%d", &iterationCount);
    sscanf(argv[3], "%lf", &delta);

    successCount = (Atomic_Int *) malloc(sizeof(Atomic_Int));
    successCount->value = 0;
    successCount->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(successCount->lock, NULL);
    totalCount = (Atomic_Int *) malloc(sizeof(Atomic_Int));
    totalCount->value = 0;
    totalCount->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(totalCount->lock, NULL);

    pthread_t *coordinatorThd = (pthread_t *) malloc(sizeof(pthread_t));
    if (pthread_create(coordinatorThd, NULL, monteControl, (void *) &delta)) {
        fprintf(stderr, "Error creating coordinator thread");
        exit(-1);
    }

    Worker_Input *thdArg;
    pthread_t *workerThd[threadCount];
    for (i = 0; i < threadCount; i++) {
        thdArg = (Worker_Input *) malloc(sizeof(Worker_Input));
        thdArg->experimentCount = iterationCount;
        thdArg->thdId = i;
        workerThd[i] = (pthread_t *) malloc(sizeof(pthread_t));
        if (pthread_create(workerThd[i], NULL, monteWorker, (void *) thdArg)) {
            fprintf(stderr, "Error creating worker thread %d", i);
            exit(-1);
        }
    }

    for (i = 0; i < threadCount; i++) {
        if (pthread_join(*workerThd[i], NULL)) {
            fprintf(stderr, "Error joining worker thread %d", i);
            exit(-1);
        }
    }

    if (pthread_join(*coordinatorThd, NULL)) {
        fprintf(stderr, "Error joining coordinator thread");
        exit(-1);
    }

    exit(0);
}

void *monteWorker(void *arg) {
    Worker_Input *input = (Worker_Input *) arg;
    printf("Worker %d created\n", input->thdId);

    pthread_exit(NULL);
}

void *monteControl(void *arg) {
    double *delta;
    delta = (double *) arg;
    printf("Coordinator created with delta: %lf\n", *delta);

    pthread_exit(NULL);
}
