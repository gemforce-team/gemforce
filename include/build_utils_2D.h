#ifndef _BUILD_UTILS_2D_H
#define _BUILD_UTILS_2D_H

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <numeric>

#include "gem_sort.h"

template<unsigned int ACC, class gem>
static inline void merge_subpool(gem** subpool_p, int* subpool_length_p, gem* temp_pool, int temp_length)
{
	int full_length = temp_length + *subpool_length_p;
	gem* full_array = (gem*)malloc(full_length * sizeof(gem));

	gem* temp_end = std::copy_n(temp_pool, temp_length, full_array);
	std::copy_n(*subpool_p, *subpool_length_p, temp_end);

	free(*subpool_p);    // free old pool

	gem_sort(full_array, full_length, AS_LAMBDA(gem_2D_less<ACC>));             // work starts
	int broken = 0;
	decltype(get_second(gem{})) lim_second = -1;
	for (int l = full_length - 1; l >= 0; --l) {
		bool is_broken;
		if constexpr (ACC == 0) {
			is_broken = get_second(full_array[l]) <= lim_second;
		}
		else {
			is_broken = (int)(ACC * get_second(full_array[l])) <= (int)(ACC * lim_second);
		}
		if (is_broken) {
			full_array[l].grade = 0;
			broken++;
		}
		else
			lim_second = get_second(full_array[l]);
	}                                      // all unnecessary gems destroyed

	*subpool_length_p = full_length - broken;
	*subpool_p = (gem*)malloc(*subpool_length_p * sizeof(gem));   // pool init via broken
	std::copy_if(full_array, full_array + full_length, *subpool_p, [](gem g) {return g.grade != 0;});

	free(full_array);     // free
}

template<unsigned int ACC, class gem>
inline int fill_pool_2D(gem** pool, int* pool_length, int i, int size)
{
	const int eoc=(i+1)/ (1+1);      // end of combining
	const int j0 =(i+1)/(10+1);      // value ratio < 10
	int comb_tot=0;

	const int grade_max = (int)(log2(i + 1) + 1); // gems with max grade cannot be destroyed, so this is a max, not a sup
	gem* temp_pools[grade_max - 1];               // get the temp pools for every grade
	int  temp_index[grade_max - 1];               // index of work point in temp pools
	gem* subpools[grade_max - 1];                 // get subpools for every grade
	int  subpools_length[grade_max - 1];
	for (int j = 0; j < grade_max - 1; ++j) {     // init everything
		temp_pools[j] = (gem*)malloc(size * sizeof(gem));
		temp_index[j] = 0;
		subpools[j] = NULL;                       // just to be able to free it
		subpools_length[j] = 0;
	}

	for (int j = j0; j < eoc; ++j) {         // combine gems and put them in temp array
		for (int k = 0; k < pool_length[j]; ++k) {
			int g1 = (pool[j] + k)->grade;
			for (int h = 0; h < pool_length[i - 1 - j]; ++h) {
				int delta = g1 - (pool[i - 1 - j] + h)->grade;
				if (abs(delta) <= 2) {        // grade difference <= 2
					comb_tot++;
					gem temp;
					gem_combine(pool[j] + k, pool[i - 1 - j] + h, &temp);
					int grd = temp.grade - 2;
					temp_pools[grd][temp_index[grd]] = temp;
					temp_index[grd]++;
					if (temp_index[grd] == size) {		// let's skim a pool
						merge_subpool<ACC>(subpools + grd, subpools_length + grd, temp_pools[grd], size);
						temp_index[grd] = 0;            // temp index reset
					}                                   // rebuilt subpool[grd], work restarts
				}
			}
		}
	}

	for (int grd=0; grd<grade_max-1; ++grd) {               // let's put remaining gems on
		if (temp_index[grd] != 0) {
			merge_subpool<ACC>(subpools + grd, subpools_length + grd, temp_pools[grd], temp_index[grd]);
		}                                   // subpool[grd] is now full
	}

	pool_length[i] = std::accumulate(subpools_length, subpools_length + grade_max-1, 0);
	pool[i] = (gem*)malloc(pool_length[i]*sizeof(gem));

	int place=0;
	for (int grd=0;grd<grade_max-1;++grd) {      // copying to pool
		for (int j=0; j<subpools_length[grd]; ++j) {
			pool[i][place]=subpools[grd][j];
			place++;
		}
	}
	for (int grd=0;grd<grade_max-1;++grd) {     // free
		free(temp_pools[grd]);
		free(subpools[grd]);
	}

	return comb_tot;
}

#endif // _BUILD_UTILS_2D_H
