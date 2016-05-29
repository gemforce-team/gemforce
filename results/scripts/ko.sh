#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	j=$((i/2))
	echo "$i - $j"
	filename="ko"$(printf "%0.4d-%d" $i $j)
	desc="$filename - N6 TC120 A60 - gemforce $(git describe --tags | cut -f1 -d "-") - $file"",table_crit"
	if [ $i -le 128 ]
	then
		bin/kgquery-omnia -rqpte  -f "$file" "$i" "$j" | tail -n +46 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
	else
		bin/kgquery-omnia -rqpe  -f "$file" "$i" "$j" | tail -n +46 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
	fi
done
