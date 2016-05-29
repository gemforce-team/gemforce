#!/bin/bash
s=$1
c=$2
file=$3

echo "$s - $c"
filename="ko"$(printf "%0.4d-%0.6d" $s $c)
desc="$filename - N6 TC120 A60 - gemforce $(git describe --tags | cut -f1 -d "-") - $file"
if [ $s -le 128 ] && [ $c -le 128 ]
then
	bin/kgquery-omnia -rqpte  -f "$file" "$s" "$c" | tail -n +46 > results/$filename".txt"
	echo "$desc" >> results/$filename".txt"
elif [ $s -le 4096 ] && [ $c -le 4096 ]
then
	bin/kgquery-omnia -rqpe  -f "$file" "$s" "$c" | tail -n +46 > results/$filename".txt"
	echo "$desc" >> results/$filename".txt"
else
	bin/kgquery-omnia -rqe  -f "$file" "$s" "$c" | tail -n +46 > results/$filename".txt"
	echo "$desc" >> results/$filename".txt"
fi
