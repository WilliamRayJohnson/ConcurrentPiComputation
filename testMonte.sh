#!/bin/bash

printf "Testing 3 threads, 10000 iterations, delta 0.0001\n";
./circleMonte 3 10000 0.0001
printf "\n";

printf "Testing 10 threads, 100000 iterations, delta 0.00001\n";
./circleMonte 10 100000 0.00001
printf "\n";
