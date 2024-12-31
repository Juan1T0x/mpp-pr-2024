#!/bin/bash

# Compilación
make clean
make sec

# Archivos de entrada y salida
in_file="../input/igen/in.txt"
in_file_distances="../input/input"
out_file="../output/output"

# Bucle sobre el archivo de entrada
while read line ; do
    # Ignoramos líneas que comienzan con '#'
    if [[ $line != \#* ]]; then

        # Extraer parámetros de la línea
        n=$(echo $line | tr -s ' ' | cut -f1 -d ' ')
        m=$(echo $line | tr -s ' ' | cut -f2 -d ' ')
        gen=$(echo $line | tr -s ' ' | cut -f3 -d ' ')
        tam=$(echo $line | tr -s ' ' | cut -f4 -d ' ')

        # Distintos valores de NP
        for np in 2 4 8; do
            # Distintos valores de NGM
            for ngm in 1 5; do
                # Distintos valores de hilos OpenMP
                for threads in 1 2 4 8; do

                    echo
                    echo -n "Executing with: "
                    echo "N = $n, M = $m, N_GEN = $gen, TAM_POB = $tam, NP = $np, NGM = $ngm, THREADS = $threads"

                    # Exportar la variable OMP_NUM_THREADS (opcional)
                    export OMP_NUM_THREADS=$threads

                    # Ejecutar con mpirun y redirigir al archivo deseado
                    mpirun -np $np ./sec $n $m $gen $tam $ngm \
                      < "${in_file_distances}_${n}_${m}_${gen}_${tam}.txt" \
                      >> "${out_file}_${n}_${m}_${gen}_${tam}_${np}_${ngm}_T${threads}.txt"

                done
            done
        done
    fi
done < $in_file
