#ifndef _KGA_UTILS_H
#define _KGA_UTILS_H

#include <cstdio>

#include "crit_utils.h"
#include "interval_tree.h"
#include "sort_utils.h"
#include "build_utils_2D.h"

inline double gem_amp_power(const gem_Y& gem1, const gem_Y& amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*(gem1.crit+crit_ratio*amp1.crit);
}

void print_omnia_table(const gem_Y* amps, const double* powers, int len)
{
	printf("Critgem\tAmps\tPower\n");
	for (int i=0; i<len; i++)
		printf("%d\t%d\t%#.7g\n", i+1, gem_getvalue(amps+i), powers[i]);
	printf("\n");
}

template<class gem>
inline void specs_compression(gem** poolf, int* poolf_length, const gem*const* pool, const int* pool_length, int len, bool debug)
{
	for (int i = 0; i < len; ++i) {
		gem* temp_pool = (gem*)malloc(pool_length[i] * sizeof(gem));
		std::copy_n(pool[i], pool_length[i], temp_pool);

		compression_2D<0>(poolf + i, poolf_length + i, temp_pool, pool_length[i]);
		free(temp_pool);

		if (debug)
			printf("Critgem value %d speccing compressed pool size:\t%d\n", i + 1, poolf_length[i]);
	}
}

template<class gem>
inline void amps_compression(gem** poolAf, int* poolAf_length, const gem*const* poolA, const int* poolA_length, int lena, bool debug)
{
	for (int i = 0; i < lena; ++i) {
		gem* temp_pool = (gem*)malloc(poolA_length[i] * sizeof(gem));
		std::copy_n(poolA[i], poolA_length[i], temp_pool);

		compression_2D<0>(poolAf + i, poolAf_length + i, temp_pool, poolA_length[i]);
		free(temp_pool);

		if (debug)
			printf("Amp value %d compressed pool size:\t%d\n", i+1, poolAf_length[i]);
	}
}


#endif // _KGA_UTILS_H
