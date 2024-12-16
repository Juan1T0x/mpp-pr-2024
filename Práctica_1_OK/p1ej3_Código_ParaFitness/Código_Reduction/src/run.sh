#!/bin/bash

make sec

in_file="../input/igen/in.txt"
in_file_distances="../input/input"
out_file="../output/output.txt"

# Lista de números de hilos a utilizar
thread_counts=(1 2 4 8 16)

for nthreads in "${thread_counts[@]}"; do
    export OMP_NUM_THREADS=$nthreads
    while read line ; do 

        if [[ $line != \#* ]]
        then
            n=$(echo $line | tr -s ' ' | cut -f1 -d ' ')
            m=$(echo $line | tr -s ' ' | cut -f2 -d ' ')
            gen=$(echo $line | tr -s ' ' | cut -f3 -d ' ')
            tam=$(echo $line | tr -s ' ' | cut -f4 -d ' ')
        
            echo -e
            echo -n "Executing with: "
            echo -e "N = "$n" M = "$m" N_GEN = "$gen" TAM_POB = "$tam" NTHREADS = "$nthreads
            make test_sec N=$n M=$m N_GEN=$gen T_POB=$tam NTHREADS=$nthreads < "${in_file_distances}_${n}_${m}_${gen}_${tam}.txt"
        fi
    done < $in_file
done
