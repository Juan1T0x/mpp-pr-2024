#!/bin/bash

# Recorre todos los archivos .txt en la carpeta actual
for file in *.txt; do
    echo "Archivo: $file"  # Imprime el nombre del archivo

    # Extrae y muestra el Execution Time
    execution_time=$(grep "Execution Time:" "$file" | awk -F': ' '{print $2}')
    if [[ -n "$execution_time" ]]; then
        echo "  Execution Time: $execution_time"
    else
        echo "  Execution Time: No encontrado"
    fi

    # Extrae y muestra la Distance
    distance=$(grep "Distance:" "$file" | awk -F': ' '{print $2}')
    if [[ -n "$distance" ]]; then
        echo "  Distance: $distance"
    else
        echo "  Distance: No encontrado"
    fi

    echo  # Línea en blanco para separar los archivos
done
