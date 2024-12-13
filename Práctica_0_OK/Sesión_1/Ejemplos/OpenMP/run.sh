#!/bin/bash

export OMP_NUM_THREADS=6

make omp
make test_omp N=1000
