#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	if [ $i -le 256 ]
	then
		bin/mgquery-amps -rqpte  -f "$file" "$i"c | tail -n +29 > results/ma$(printf "%0.4d" $i).txt
	elif [ $i -le 16384 ]
	then
		bin/mgquery-amps -rqpe  -f "$file" "$i"c | tail -n +29 > results/ma$(printf "%0.4d" $i).txt
	fi
done
