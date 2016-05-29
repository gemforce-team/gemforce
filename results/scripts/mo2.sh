#!/bin/bash
s=$1
c=$2
file=$3

echo "$s - $c"
filename="mo"$(printf "%0.4d-%0.6d" $s $c)
desc="$filename - N6 TC120 A60 - gemforce $(git describe --tags | cut -f1 -d "-") - $file"",table_leech"
if [ $s -le 128 ] && [ $c -le 128 ]
then
	bin/mgquery-omnia -rqpte  -f "$file" "$s" "$c" | tail -n +40 > results/$filename".txt"
	echo "$desc" >> results/$filename".txt"
elif [ $s -le 4096 ] && [ $c -le 4096 ]
then
	bin/mgquery-omnia -rqpe  -f "$file" "$s" "$c" | tail -n +40 > results/$filename".txt"
	echo "$desc" >> results/$filename".txt"
else
	bin/mgquery-omnia -rqe  -f "$file" "$s" "$c" | tail -n +40 > results/$filename".txt"
	echo "$desc" >> results/$filename".txt"
fi
