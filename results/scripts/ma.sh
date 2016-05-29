#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	filename="ma"$(printf "%0.4d" $i)
	desc="$filename - N6 TC120 A60 - gemforce $(git describe --tags | cut -f1 -d "-") - $file"",table_leech"
	if [ $i -le 256 ]
	then
		bin/mgquery-amps -rqpte -f "$file" "$i" | tail -n +26 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
	else
		bin/mgquery-amps -rqpe  -f "$file" "$i" | tail -n +26 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
	fi
done
