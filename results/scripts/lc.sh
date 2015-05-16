#!/bin/bash
inf=$1
sup=$2

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	if [ $i -le 256 ]
	then
		bin/leechquery -qpte  $i | tail -n +4 > results/lc$(printf "%0.7d" $i).txt
		bin/leechquery -qupte $i | tail -n +9 > results/lc$(printf "%0.7d" $i)u.txt
	elif [ $i -le 16384 ]
	then
		bin/leechquery -qpe  $i | tail -n +4 > results/lc$(printf "%0.7d" $i).txt
		bin/leechquery -qupe $i | tail -n +9 > results/lc$(printf "%0.7d" $i)u.txt
	else
		bin/leechquery -qe  $i | tail -n +4 > results/lc$(printf "%0.7d" $i).txt
		bin/leechquery -que $i | tail -n +9 > results/lc$(printf "%0.7d" $i)u.txt
	fi
done
