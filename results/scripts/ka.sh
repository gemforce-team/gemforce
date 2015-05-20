#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	filename="ka"$(printf "%0.4d" $i)
	desc="$filename - N8 TC120 A60 - gemforce $(git describe --tags | cut -f1 -d "-") - $file"",table_crit"
	if [ $i -le 256 ]
	then
		bin/kgquery-amps -rqpte -f "$file" "$i" | tail -n +32 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
	else
		bin/kgquery-amps -rqpe  -f "$file" "$i" | tail -n +32 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
	fi
done
