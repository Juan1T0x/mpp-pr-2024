#!/bin/bash

make clean
make sec

# Parámetros fijos
n=30000
m=28000
n_gen=0
tam_pob=6

# Bucle externo: n_hilos_ini en {1,2,4,6,8,10,12,14,16,18,20}
for n_hilos_ini in 1 2 4 6 8 10 12 14 16 18 20
do
    # Bucle interno: n_hilos_fit en {1,2,4,8,16}, por ejemplo
    for n_hilos_fit in 1 2 4 8 16
    do
        echo "Ejecutando con: n=$n m=$m n_gen=$n_gen tam_pob=$tam_pob n_hilos_ini=$n_hilos_ini n_hilos_fit=$n_hilos_fit"

        make test_sec \
          N=$n \
          M=$m \
          N_GEN=$n_gen \
          T_POB=$tam_pob \
          N_HILOS_INI=$n_hilos_ini \
          N_HILOS_FIT=$n_hilos_fit
    done
done
