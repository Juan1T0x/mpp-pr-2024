#!/bin/bash

make clean

in_file="../input/igen/in.txt"
in_file_distances="../input/input"
out_file="../output/output.txt"

# Lista de números de hilos a utilizar
thread_counts=(1 2 4 8 16)

# Lista de chunk_sizes a probar. Puedes ajustar estos valores según tus necesidades
chunk_sizes=(4 8 16 32)

while read line ; do
    if [[ $line != \#* ]]
    then
        n=$(echo $line | tr -s ' ' | cut -f1 -d ' ')
        m=$(echo $line | tr -s ' ' | cut -f2 -d ' ')
        gen=$(echo $line | tr -s ' ' | cut -f3 -d ' ')
        tam=$(echo $line | tr -s ' ' | cut -f4 -d ' ')

        # Para cada chunk_size en la lista definida
        for csize in "${chunk_sizes[@]}"; do
            make clean
            make sec CHUNK_SIZE=$csize

            # Para cada número de hilos
            for nthreads in "${thread_counts[@]}"; do
                export OMP_NUM_THREADS=$nthreads

                echo
                echo -n "Executing with: "
                echo -e "N = $n M = $m N_GEN = $gen TAM_POB = $tam NTHREADS = $nthreads CHUNK_SIZE = $csize"

                make test_sec N=$n M=$m N_GEN=$gen T_POB=$tam NTHREADS=$nthreads CHUNK_SIZE=$csize < "${in_file_distances}_${n}_${m}_${gen}_${tam}.txt"
            done
        done
    fi
done < $in_file
