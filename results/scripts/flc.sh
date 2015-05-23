#!/bin/bash
inf=$1
sup=$2

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	filename="flc"$(printf "%0.3d" $i)
	filenameu="flc"$(printf "%0.3d" $i)"u"
	desc="$filename - gemforce $(git describe --tags | cut -f1 -d "-") - table_frleech"
	descu="$filenameu - gemforce $(git describe --tags | cut -f1 -d "-") - table_frleech"
	if [ $i -le 256 ]
	then
		bin/firerate/frleechquery -qpte  "$i" | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/firerate/frleechquery -qupte "$i" | tail -n +11 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	else
		bin/firerate/frleechquery -qpe  "$i" | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/firerate/frleechquery -qupe "$i" | tail -n +11 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	fi
done
