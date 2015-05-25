#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	filename="kc"$(printf "%0.6d" $i)
	filenameu="kc"$(printf "%0.6d" $i)"u"
	desc="$filename - gemforce $(git describe --tags | cut -f1 -d "-") - $file"
	descu="$filenameu - gemforce $(git describe --tags | cut -f1 -d "-") - $file"
	if [ $i -le 256 ]
	then
		bin/kgquery-alone -qpte  -f "$file" "$i"c | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/kgquery-alone -qupte -f "$file" "$i"c | tail -n +12 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	elif [ $i -le 16384 ]
	then
		bin/kgquery-alone -qpe  -f "$file" "$i"c | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/kgquery-alone -qupe -f "$file" "$i"c | tail -n +12 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	else
		bin/kgquery-alone -qe  -f "$file" "$i"c | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/kgquery-alone -que -f "$file" "$i"c | tail -n +12 > results/$filenameu".txt"
		echo "$descu" >> results/$filenameu".txt"
	fi
done
