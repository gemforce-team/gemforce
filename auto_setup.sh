#!/bin/bash

# this script should be called in the repo root
# but I'll check all the folders anyway

if [ ! -d "include" ]; then
	echo "No include folder found, aborting"
	exit 1
fi

mkdir bin

if [ -d "managem" ]; then
	echo "Compiling leech/managem programs"
	gcc -O3 -lm -I "include" managem/leechquery.c    -o bin/leechquery
	gcc -O3 -lm -I "include" managem/mgquery-alone.c -o bin/mgquery-alone
	gcc -O3 -lm -I "include" managem/mgquery-amps.c  -o bin/mgquery-amps
	gcc -O3 -lm -I "include" managem/mgquery-setup.c -o bin/mgquery-setup
	gcc -O3 -lm -I "include" managem/mgquery-omnia.c -o bin/mgquery-omnia
else
	echo "No managem folder found, you get no leech/managem programs"
fi

if [ -d "killgem" ]; then
	echo "Compiling killgem programs"
	gcc -O3 -lm -I "include" killgem/kgquery-alone.c -o bin/kgquery-alone
	gcc -O3 -lm -I "include" killgem/kgquery-amps.c  -o bin/kgquery-amps
	gcc -O3 -lm -I "include" killgem/kgquery-setup.c -o bin/kgquery-setup
	gcc -O3 -lm -I "include" killgem/kgquery-omnia.c -o bin/kgquery-omnia
else
	echo "No killgem folder found, you get no killgem programs"
fi

if [ -d "gem_tables" ]; then
	echo "Copying tables over"
	cp -r gem_tables/ bin/gem_tables/
else
	echo "No gem_tables folder found, I hope you know what you are doing, or nothing will work"
fi

echo "Setup completed, the programs can be found in the bin/ folder"
