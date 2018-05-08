#!/bin/bash

printf "Testing 3 threads, 10000 iterations, delta 0.0001\n";
./circleMonte 3 10000 0.0001
printf "\n";
