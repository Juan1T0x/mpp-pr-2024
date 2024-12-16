#!/bin/bash

make sec

in_file="../input/igen/in.txt"
in_file_distances="../input/input"
out_file="../output/output.txt"

while read line ; do 

	if [[ $line != \#* ]]
	then
		n=$(echo $line | tr -s ' ' | cut -f1 -d ' ')
		m=$(echo $line | tr -s ' ' | cut -f2 -d ' ')
		gen=$(echo $line | tr -s ' ' | cut -f3 -d ' ')
		tam=$(echo $line | tr -s ' ' | cut -f4 -d ' ')
	
		echo -e
		echo -n "Executing with: "
		echo -e "N = "$n" M = "$m" N_GEN = "$gen" TAM_POB = "$tam
		make test_sec N=$n M=$m N_GEN=$gen T_POB=$tam < "${in_file_distances}_${n}_${m}_${gen}_${tam}.txt"
	fi
done < $in_file

