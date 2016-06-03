#!/usr/bin/bash

pool_viewer="pool_viewer/pool_viewer"
prefix="data_"

len=$1
table=$2

pool_zero=$(head -n1 "${table}")

if [[ -z "${pool_zero}" ]]; then
	echo "Problems with table ${table}, aborting..."
	exit
fi

if [[ ! -e "${pool_viewer}" ]]; then
	echo "Can't find ${pool_viewer}, aborting..."
	exit
fi

combine_suffix=""

if [[ ${pool_zero} -eq 1 ]]; then combine_suffix="c"; fi

"./${pool_viewer}" -f "${table}" -- "${len}${combine_suffix}"

datafile="${prefix}${table}${len}"
gnuplot -e "datafile='${datafile}'" "pool_viewer/macro.gp"
rm -v "${datafile}"
