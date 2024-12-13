#!/bin/bash

make mpi
make test_mpi NP=4 N=1000
