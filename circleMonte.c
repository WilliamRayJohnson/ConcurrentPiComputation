/**
 * A concurrent program that uses Monte Carlo simulations to approximate the
 * value of pi
 * Author: William Ray Johnson
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

#define CIRCLE_DIAMETER 1.0

void *monteWorker(void *arg);
void *monteControl(void *arg);

double monteExperiment(double min , double max);

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

pthread_mutex_t *rand_lock;

pthread_barrier_t *calcBarrier;
pthread_barrier_t *checkBarrier;
bool *experimentsDone;

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

    // Initialize barriers for pi calculation and delta check
    calcBarrier = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
    pthread_barrier_init(calcBarrier, NULL, threadCount + 1);
    checkBarrier = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
    pthread_barrier_init(checkBarrier, NULL, threadCount + 1);

    rand_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(rand_lock, NULL);

    // Initialize global variables involved in pi approximation
    successCount = (Atomic_Int *) malloc(sizeof(Atomic_Int));
    successCount->value = 0;
    successCount->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(successCount->lock, NULL);
    totalCount = (Atomic_Int *) malloc(sizeof(Atomic_Int));
    totalCount->value = 0;
    totalCount->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(totalCount->lock, NULL);
    experimentsDone = (bool *) malloc(sizeof(bool));
    *experimentsDone = false;

    // Initialize cooridinator thread
    pthread_t *coordinatorThd = (pthread_t *) malloc(sizeof(pthread_t));
    if (pthread_create(coordinatorThd, NULL, monteControl, (void *) &delta)) {
        fprintf(stderr, "Error creating coordinator thread");
        exit(-1);
    }

    // Initialize worker threads
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

double monteExperiment(double min, double max) {
    double value;
    double range = (max - min);
    double div = RAND_MAX / range;
    value = min + (rand() / div);
    return value;
}

double calcDistance(double xOne, double yOne, double xTwo, double yTwo) {
    double xPart = pow((xTwo - xOne), 2.0);
    double yPart = pow((yTwo - yOne), 2.0);
    double distance = sqrt(xPart + yPart);
    return distance;
}

void *monteWorker(void *arg) {
    Worker_Input *input = (Worker_Input *) arg;
    double experimentX;
    double experimentY;
    double experimentDist;
    double circleCenter = CIRCLE_DIAMETER / 2.0;
    int i;
    int successfulExperiments;
    int totalExperiments;

    while(!*experimentsDone) {
        successfulExperiments = 0;
        totalExperiments = 0;
        for (i = 0; i < input->experimentCount; i++) {
            pthread_mutex_lock(rand_lock);
            experimentX = monteExperiment(0.0, CIRCLE_DIAMETER);
            pthread_mutex_unlock(rand_lock);
            pthread_mutex_lock(rand_lock);
            experimentY = monteExperiment(0.0, CIRCLE_DIAMETER);
            pthread_mutex_unlock(rand_lock);
            experimentDist = calcDistance(experimentX, experimentY,
                    circleCenter, circleCenter);
            if (experimentDist <= circleCenter)
                successfulExperiments++;
            totalExperiments++;
        }

        // Update globals
        pthread_mutex_lock(successCount->lock);
        successCount->value += successfulExperiments;
        pthread_mutex_unlock(successCount->lock);
        pthread_mutex_lock(totalCount->lock);
        totalCount->value += totalExperiments;
        pthread_mutex_unlock(totalCount->lock);

        pthread_barrier_wait(calcBarrier);
        pthread_barrier_wait(checkBarrier);
    }

    pthread_exit(NULL);
}

void *monteControl(void *arg) {
    double *delta = (double *) arg;
    double circleRadius = CIRCLE_DIAMETER / 2.0;
    double circleAreaEstimation;
    double currPiEstimation = 0.0;
    double pastPiEstimation = 0.0;
    int round = 0;

    while(!*experimentsDone) {
        pthread_barrier_wait(calcBarrier);

        pthread_mutex_lock(successCount->lock);
        pthread_mutex_lock(totalCount->lock);
        circleAreaEstimation = (double) successCount->value / totalCount->value;
        pthread_mutex_unlock(successCount->lock);
        pthread_mutex_unlock(totalCount->lock);

        currPiEstimation = circleAreaEstimation / pow(circleRadius, 2.0);
        printf("Round %d's pi estimation: %lf\n", round, currPiEstimation);

        if (fabs(pastPiEstimation - currPiEstimation) <= *delta)
            *experimentsDone = true;
        pastPiEstimation = currPiEstimation;

        round++;
        pthread_barrier_wait(checkBarrier);
    }

    pthread_exit(NULL);
}
