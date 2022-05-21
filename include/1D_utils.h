#ifndef _1D_UTILS_H
#define _1D_UTILS_H

#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "0D_utils.h"
#include "container_utils.h"

// -------------
// Build section
// -------------

template<class gem>
inline size_t fill_pool_1D(vector<pool_t<gem>>& pool, vector<size_t>& pool_length, int i)
{
	const int eoc = (i + 1) / (1 + 1);     // end of combining
	const int j0 = (i + 1) / (10 + 1);     // value ratio < 10
	size_t comb_tot = 0;

	const int grade_max = (int)(log2(i + 1) + 1);  // gems with max grade cannot be destroyed, so this is a max, not a sup
	gem temp_array[grade_max - 1];                 // this will have all the grades
	memset(temp_array, 0, sizeof(temp_array));

	for (int j = j0; j < eoc; ++j) {          // combine gems and put them in temp array
		for (size_t k = 0; k < pool_length[j]; ++k) {
			int g1 = (pool[j] + k)->grade;
			for (size_t h = 0; h < pool_length[i - 1 - j]; ++h) {
				int delta = g1 - (pool[i - 1 - j] + h)->grade;
				if (abs(delta) <= 2) {        // grade difference <= 2
					comb_tot++;
					gem temp;
					gem_combine(pool[j] + k, pool[i - 1 - j] + h, &temp);
					int grd = temp.grade - 2;
					if (gem_more_powerful(temp, temp_array[grd])) {
						temp_array[grd] = temp;
					}
				}
			}
		}
	}
	int gemNum = 0;
	for (int j = 0; j < grade_max - 1; ++j)
		if (temp_array[j].grade != 0)
			gemNum++;
	pool_length[i] = gemNum;
	pool[i] = make_uninitialized_pool<gem>(pool_length[i]);

	std::copy_if(temp_array, temp_array + grade_max - 1, pool[i] + 0, [](gem g) {return g.grade != 0;});

	return comb_tot;
}

template<class gem>
inline void compression_1D(gem* best_p, const pool_t<gem>& pool, size_t pool_length)
{
	const gem* curr_best = pool + 0;
	for (size_t j = 1; j < pool_length; ++j) {
		if (gem_more_powerful(pool[j], *curr_best)) {
			curr_best = pool + j;
		}
	}
	*best_p = *curr_best;
}

// -------------------
// Chain adder section
// -------------------

#include "chain_adder.h"

template<class gem> requires requires (gem* g) { gem_init(g, 1, 0); }
gem* gem_putchain(const pool_t<gem>& pool, size_t pool_length, vector<gem>& chain_gems)
{
	return gem_putchain_templ(pool, pool_length, chain_gems,
							  [](gem* arg) {gem_init(arg, 1, 0);},
							  [](const gem& arg) {return gem_power(arg);});
}

// -----------------
// Pool init section
// -----------------

template<class gem>
inline vector<pool_t<gem>> init_pool_1D(int len) {
	vector pool = vector<pool_t<gem>>(len);
	pool[0] = make_uninitialized_pool<gem>(1);
	gem_init(pool[0] + 0, 1, 1);
	return pool;
}

#endif // _1D_UTILS_H
