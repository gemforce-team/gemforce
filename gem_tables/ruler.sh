#!/bin/bash

echo

if [ -z "$1" ]
then
    for file in table_*;
    do
	if [ -f $file ]
	then
	    len=$(tail -n 2 $file | tr -d "\n")
	    if [[ "$len" == +([0-9]) ]] # test if it's an integer
	    then
		len=$(($len+1))
		printf "%s:\t%8d\n" $file $len
	    fi
	fi
    done
else
    printf "|Name\t\t|Length\t|\n"
    printf "|:--------------|------:|\n"
    for file in table_*;
    do
	if [ -f $file ]
	then
	    len=$(tail -n 2 $file | tr -d "\n")
	    if [[ "$len" == +([0-9]) ]] # test if it's an integer
	    then
		len=$(($len+1))
		printf "|%s\t|%d\t|\n" $file $len
	    fi
	fi
    done
fi
