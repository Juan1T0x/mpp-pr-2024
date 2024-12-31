#!/bin/bash

# Compilar (limpiar antes, opcional)
make clean
make mpi

# Definimos parámetros fijos
N=5000
M=2000
N_GEN=1
T_POB=500

# Bucle externo para NP en {1,2,4,6,12,18,24,30}
for np in 1 2 4 6 8
do
    # Bucle interno para ngm en [1..5]
    for ngm in 1 2 3 4 5
    do
        echo "Ejecutando con: np=$np, ngm=$ngm (N=$N, M=$M, N_GEN=$N_GEN, T_POB=$T_POB)"

        # Llamada a la regla 'test_mpi' con los 5 parámetros (más NP)
        make test_mpi \
          N=$N \
          M=$M \
          N_GEN=$N_GEN \
          T_POB=$T_POB \
          NP=$np \
          NGM=$ngm
    done
done
