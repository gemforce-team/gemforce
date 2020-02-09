#ifndef _MGA_UTILS_H
#define _MGA_UTILS_H

#include <cstdio>
#include <cstdlib>

#include "sort_utils.h"

inline double gem_amp_power(const gem_OB& gem1, const gem_O& amp1, double leech_ratio)
{
	return gem1.bbound*(gem1.leech+leech_ratio*amp1.leech);
}

void print_omnia_table(const gem_O* amps, const double* powers, int len)
{
	printf("Managem\tAmps\tPower\n");
	for (int i=0; i<len; i++)
		printf("%d\t%d\t%#.7g\n", i+1, gem_getvalue(amps+i), powers[i]);
	printf("\n");
}

template<class gem>
inline void compression_2D_full(gem** pool_out_p, int* pool_length_out_p, gem* temp_pool, int pool_length_in)
{
	gem_sort(temp_pool, pool_length_in, AS_LAMBDA(gem_12_less));

	int broken = 0;

	decltype(get_second(gem {})) lim_second = 0; /* also remove gems with second = 0 */
	/* Look at gems from high X to low, keep track of the
	 * best Y seen till now, discard gems that don't beat that Y
	 */
	for (int l = pool_length_in - 1; l >= 0; --l) {
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
	for (int l = 0; l < pool_length_in; ++l) { /* leech min->max equals ileech max->min */
		if (temp_pool[l].grade == 0) continue;

		if (gem_power(temp_pool[l]) <= lim_power) {
			temp_pool[l].grade = 0;
			broken++;
		}
		else
			lim_power = gem_power(temp_pool[l]);
	} // all unnecessary gems destroyed

	*pool_length_out_p = pool_length_in - broken;
	*pool_out_p = (gem*)malloc(*pool_length_out_p * sizeof(gem));
	std::copy_if(temp_pool, temp_pool + pool_length_in, *pool_out_p, [](gem g) {return g.grade != 0;});
}

template<class gem>
inline void specs_compression(gem** poolf, int* poolf_length, const gem*const* pool, const int* pool_length, int len, bool debug)
{
	for (int i = 0; i < len; ++i) {
		gem* temp_pool = (gem*)malloc(pool_length[i] * sizeof(gem));
		std::copy_n(pool[i], pool_length[i], temp_pool);

		compression_2D_full(poolf + i, poolf_length + i, temp_pool, pool_length[i]);
		free(temp_pool);

		if (debug)
			printf("Managem value %d speccing compressed pool size:\t%d\n", i + 1, poolf_length[i]);
	}
}

template<class gem>
inline void combs_compression(gem** poolcf_p, int* poolcf_length_p, gem* poolc, int poolc_length)
{
	compression_2D_full(poolcf_p, poolcf_length_p, poolc, poolc_length);
}

template<class gem>
inline void amps_compression(gem* bestA, const gem*const* poolA, const int* poolA_length, int lena)
{
	for (int i = 0; i < lena; ++i) {
		compression_1D(bestA + i, poolA[i], poolA_length[i]);
	}
}

#endif // _MGA_UTILS_H
