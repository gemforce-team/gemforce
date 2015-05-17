#!/bin/bash
inf=$1
sup=$2
file=$3

for ((i=inf; i<=sup; i*=2))
do
	echo $i
	filename="mc"$(printf "%0.6d" $i)
	filenameu="mc"$(printf "%0.6d" $i)"u"
	desc="$filename - N6 TC120 A60 - gemforce $(git describe --tags | cut -f1-2 -d "-") - $file"
	descu="$filenameu - N6 TC120 A60 - gemforce $(git describe --tags | cut -f1-2 -d "-") - $file"
	if [ $i -le 256 ]
	then
		bin/mgquery-alone -qpte  -f "$file" "$i"c | tail -n +4 > results/$filename".txt"
		echo "$desc" >> results/$filename".txt"
		bin/mgquery-alone -qupte -f "$file" "$i"c | tail -n +11 > results/$filenameu".txt"
		echo "$descu" >> results/$filename"u.txt"
	elif [ $i -le 16384 ]
	then
		bin/mgquery-alone -qpe  -f "$file" "$i"c | tail -n +4 > results/$filename".txt"
		bin/mgquery-alone -qupe -f "$file" "$i"c | tail -n +11 > results/$filenameu".txt"
	else
		bin/mgquery-alone -qe  -f "$file" "$i"c | tail -n +4 > results/$filename".txt"
		bin/mgquery-alone -que -f "$file" "$i"c | tail -n +11 > results/$filenameu".txt"
	fi
done
