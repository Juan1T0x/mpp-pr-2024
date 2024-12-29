#!/bin/bash

make sec

# Archivo de entrada y salida
in_file="../input/igen/in.txt"
in_file_distances="../input/input"
out_file="../output/output"

# Bucle sobre el archivo de entrada
while read line ; do 

	if [[ $line != \#* ]]
	then
		# Extraer parámetros de la línea
		n=$(echo $line | tr -s ' ' | cut -f1 -d ' ')
		m=$(echo $line | tr -s ' ' | cut -f2 -d ' ')
		gen=$(echo $line | tr -s ' ' | cut -f3 -d ' ')
		tam=$(echo $line | tr -s ' ' | cut -f4 -d ' ')
	
		# Ejecutar para cada número de procesos
		for np in 2 4 8; do
			# Ejecutar para diferentes valores de NGM
			for ngm in 1 5; do
				echo -e
				echo -n "Executing with: "
				echo -e "N = $n, M = $m, N_GEN = $gen, TAM_POB = $tam, NP = $np, NGM = $ngm"

				# Ejecutar con mpirun y guardar salida en archivo correspondiente
				mpirun -np $np ./sec $n $m $gen $tam $ngm < "${in_file_distances}_${n}_${m}_${gen}_${tam}.txt" >> "${out_file}_${n}_${m}_${gen}_${tam}_${np}_${ngm}.txt"
			done
		done
	fi
done < $in_file
