#!/usr/bin/env bash

CMD='g++ --std=c++17 src/main.cpp -o cluster'
CMD_PARALLEL='g++ --std=c++17 -fopenmp src/main.cpp -o cluster_parallel'

echo  $CMD
$CMD

# echo  $CMD_PARALLEL
# $CMD_PARALLEL
