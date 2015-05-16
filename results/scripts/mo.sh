#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	j=$((i/2))
	echo "$i - $j"
	if [ $i -le 128 ]
	then
		bin/mgquery-omnia -rqpte  -f "$file" "$i" "$j" | tail -n +44 > results/mo$(printf "%0.4d-%d" $i $j).txt
	else
		bin/mgquery-omnia -rqpe  -f "$file" "$i" "$j" | tail -n +44 > results/mo$(printf "%0.4d-%d" $i $j).txt
	fi
done
