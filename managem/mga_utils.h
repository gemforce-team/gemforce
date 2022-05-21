#ifndef _MGA_UTILS_H
#define _MGA_UTILS_H

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "container_utils.h"
#include "sort_utils.h"

template<class gem, class gemA>
inline double gem_amp_power(const gem& gem1, const gemA& amp1, double leech_ratio)
{
	return gem1.bbound*(gem1.leech+leech_ratio*amp1.leech);
}

template<class gem>
inline void compression_2D_full(pool_t<gem>& pool_out, size_t& pool_length_out,
                                pool_t<gem>& temp_pool, size_t pool_length_in)
{
	gem_sort(temp_pool + 0, pool_length_in, AS_LAMBDA(gem_12_less));

	int broken = 0;

	decltype(get_second(gem {})) lim_second = 0; /* also remove gems with second = 0 */
	/* Look at gems from high X to low, keep track of the
	 * best Y seen till now, discard gems that don't beat that Y
	 */
	for (ssize_t l = pool_length_in - 1; l >= 0; --l) {
		if (get_second(temp_pool[l]) <= lim_second) {
			temp_pool[l].grade = 0;
			broken++;
		}
		else
			lim_second = get_second(temp_pool[l]);
	} // all unnecessary gems destroyed

	decltype(gem_power(gem {})) lim_power = 0; /* power-ileech compression */
	/* Look at gems from high XY to low, keep track of the
	 * best Y seen till now, discard gems that don't beat that Y
	 */
	for (size_t l = 0; l < pool_length_in; ++l) { /* leech min->max equals ileech max->min */
		if (temp_pool[l].grade == 0) continue;

		if (gem_power(temp_pool[l]) <= lim_power) {
			temp_pool[l].grade = 0;
			broken++;
		}
		else
			lim_power = gem_power(temp_pool[l]);
	} // all unnecessary gems destroyed

	pool_length_out = pool_length_in - broken;
	pool_out = make_uninitialized_pool<gem>(pool_length_out);
	std::copy_if(temp_pool + 0, temp_pool + pool_length_in, pool_out + 0, [](gem g) {return g.grade != 0;});
}

template<class gem>
inline void specs_compression(vector<pool_t<gem>>& poolf, vector<size_t>& poolf_length,
                              const vector<pool_t<gem>>& pool, const vector<size_t>& pool_length,
							  size_t len, bool debug)
{
	for (size_t i = 0; i < len; ++i) {
		pool_t<gem> temp_pool = make_uninitialized_pool<gem>(pool_length[i]);
		std::copy_n(pool[i] + 0, pool_length[i], temp_pool + 0);

		compression_2D_full(poolf[i], poolf_length[i], temp_pool, pool_length[i]);

		if (debug)
			printf("Managem value %zu speccing compressed pool size:\t%zu\n", i + 1, poolf_length[i]);
	}
}

template<class gem>
inline void combs_compression(pool_t<gem>& poolcf, size_t& poolcf_length,
                              pool_t<gem>& poolc, size_t poolc_length)
{
	compression_2D_full(poolcf, poolcf_length, poolc, poolc_length);
}

template<class gemA>
inline void amps_compression(vector<gemA>& bestA, const vector<pool_t<gemA>>& poolA,
                             const vector<size_t>& poolA_length, int lena)
{
	for (int i = 0; i < lena; ++i) {
		compression_1D(bestA + i, poolA[i], poolA_length[i]);
	}
}

#endif // _MGA_UTILS_H
