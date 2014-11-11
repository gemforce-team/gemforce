#!/bin/bash

# this script should be called in the repo root
# but I'll check all the folders anyway

if [ ! -d "include" ]; then
	echo "No include folder found, aborting"
	exit 1
fi

mkdir bin

if [ -d "managem_c" ]; then
	echo "Compiling leech/managem programs"
	gcc -O2 -lm -I "include" managem_c/leechquery.c   -o bin/leechquery
	gcc -O2 -lm -I "include" managem_c/managemquery.c -o bin/managemquery
	gcc -O2 -lm -I "include" managem_c/mgaquery.c     -o bin/mgaquery
	gcc -O2 -lm -I "include" managem_c/mgomniaquery.c -o bin/mgomniaquery
else
	echo "No managem_c folder found, you get no leech/managem programs"
fi

if [ -d "killgem_c" ]; then
	echo "Compiling killgem programs"
	gcc -O2 -lm -I "include" killgem_c/killgemquery.c -o bin/killgemquery
	gcc -O2 -lm -I "include" killgem_c/kgaquery.c     -o bin/kgaquery
	gcc -O2 -lm -I "include" killgem_c/kgomniaquery.c -o bin/kgomniaquery
else
	echo "No killgem_c folder found, you get no killgem programs"
fi

if [ -d "gem_tables" ]; then
	echo "Copying tables over"
	cp gem_tables/table_leech  bin/table_leech
	cp gem_tables/table_mgspec bin/table_mgspec
	cp gem_tables/table_mgcomb bin/table_mgcomb
	cp gem_tables/table_crit   bin/table_crit
	cp gem_tables/table_kgspec bin/table_kgspec
	cp gem_tables/table_kgcomb bin/table_kgcomb
else
	echo "No gem_tables folder found, I hope you know what you are doing, or nothing will work"
fi
