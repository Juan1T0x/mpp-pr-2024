#!/bin/bash

make igen

in_file="in.txt"

while read line ; do 

	if [[ $line != \#* ]]
	then
		n=$(echo $line | tr -s ' ' | cut -f1 -d ' ')
		m=$(echo $line | tr -s ' ' | cut -f2 -d ' ')
		gen=$(echo $line | tr -s ' ' | cut -f3 -d ' ')
		tam=$(echo $line | tr -s ' ' | cut -f4 -d ' ')
		m_rate=$(echo $line | tr -s ' ' | cut -f5 -d ' ')
	
		make exe_igen N=$n M=$m N_GEN=$gen T_POB=$tam M_RATE=$m_rate
	fi
done < $in_file

