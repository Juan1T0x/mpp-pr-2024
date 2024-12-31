#!/bin/bash

# Compilar con make (limpia antes, opcional)
make clean
make sec

# Parámetros fijos
n=30000
n_gen=0

# Bucle externo para m en 12000, 20000, 28000
for m in 12000 20000 28000
do
    # Bucle para tam_pob entre 3 y 10
    for tam_pob in {3..10}
    do
        # Bucle anidado para n_hilos_ini
        for n_hilos_ini in 1 2 4 6 8 10 12 14 16 18 20
        do
            # Bucle interno para n_hilos_fit
            for n_hilos_fit in 1 2 4 8 16
            do
                echo "Ejecutando con: n=$n, m=$m, n_gen=$n_gen, tam_pob=$tam_pob, n_hilos_ini=$n_hilos_ini, n_hilos_fit=$n_hilos_fit"

                make test_sec \
                  N=$n \
                  M=$m \
                  N_GEN=$n_gen \
                  T_POB=$tam_pob \
                  N_HILOS_INI=$n_hilos_ini \
                  N_HILOS_FIT=$n_hilos_fit
            done
        done
    done
done
