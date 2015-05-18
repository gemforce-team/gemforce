#!/bin/bash
inf=$1
sup=$2

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	filename="lc"$(printf "%0.7d" $i)
	filenameu="lc"$(printf "%0.7d" $i)"u"
	desc="$filename - gemforce $(git describe --tags | cut -f1 -d "-") - table_leech"
	descu="$filenameu - gemforce $(git describe --tags | cut -f1 -d "-") - table_leech"
	if [ $i -le 256 ]
	then
		bin/leechquery -qpte  "$i" | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/leechquery -qupte "$i" | tail -n +9 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	elif [ $i -le 16384 ]
	then
		bin/leechquery -qpe  "$i" | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/leechquery -qupe "$i" | tail -n +9 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	else
		bin/leechquery -qe  "$i" | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/leechquery -que "$i" | tail -n +9 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	fi
done
