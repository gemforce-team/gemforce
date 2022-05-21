#ifndef _CRITA_UTILS_H
#define _CRITA_UTILS_H

#include <cstdio>

#include "crit_utils.h"
#include "interval_tree.h"
#include "sort_utils.h"
#include "2D_utils.h"

inline double gem_amp_power(const gem_Y& gem1, const gem_Y& amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*(gem1.crit+crit_ratio*amp1.crit);
}

template<class gem>
inline void specs_compression(vector<pool_t<gem>>& poolf, vector<size_t>& poolf_length,
                              const vector<pool_t<gem>>& pool, const vector<size_t>& pool_length,
							  size_t len, bool debug)
{
	for (size_t i = 0; i < len; ++i) {
		pool_t<gem> temp_pool = make_uninitialized_pool<gem>(pool_length[i]);
		std::copy_n(pool[i] + 0, pool_length[i], temp_pool + 0);

		compression_2D<0>(poolf[i], poolf_length[i], temp_pool, pool_length[i]);

		if (debug)
			printf("Critgem value %zu speccing compressed pool size:\t%zu\n", i + 1, poolf_length[i]);
	}
}


#endif // _CRITA_UTILS_H
