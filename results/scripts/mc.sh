#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	if [ $i -le 256 ]
	then
		bin/mgquery-alone -qpte  -f "$file" "$i"c | tail -n +4 > results/mc$(printf "%0.6d" $i).txt
		bin/mgquery-alone -qupte -f "$file" "$i"c | tail -n +11 > results/mc$(printf "%0.6d" $i)u.txt
	elif [ $i -le 16384 ]
	then
		bin/mgquery-alone -qpe  -f "$file" "$i"c | tail -n +4 > results/mc$(printf "%0.6d" $i).txt
		bin/mgquery-alone -qupe -f "$file" "$i"c | tail -n +11 > results/mc$(printf "%0.6d" $i)u.txt
	else
		bin/mgquery-alone -qe  -f "$file" "$i"c | tail -n +4 > results/mc$(printf "%0.6d" $i).txt
		bin/mgquery-alone -que -f "$file" "$i"c | tail -n +11 > results/mc$(printf "%0.6d" $i)u.txt
	fi
done
