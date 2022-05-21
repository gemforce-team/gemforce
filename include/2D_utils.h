#ifndef _2D_UTILS_H
#define _2D_UTILS_H

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <numeric>

#include "container_utils.h"
#include "sort_utils.h"
#include "1D_utils.h"

// -------------
// Build section
// -------------

template<int ACC, class gem>
inline void compression_2D(pool_t<gem>& pool_out, size_t& pool_length_out,
                           pool_t<gem>& temp_pool, size_t pool_length_in)
{
	gem_sort(temp_pool.get(), pool_length_in, AS_LAMBDA(gem_12_less<ACC>));

	int broken = 0;
	decltype(get_second(gem {})) lim_second = -1; /* do not remove gems with second = 0 */
	/* Look at gems from high X to low, keep track of the
	 * best Y seen till now, discard gems that don't beat that Y
	 */
	for (ssize_t l = pool_length_in - 1; l >= 0; --l) {
		bool is_broken;
		if constexpr (ACC == 0) {
			is_broken = get_second(temp_pool[l]) <= lim_second;
		}
		else {
			is_broken = (int)(ACC * get_second(temp_pool[l])) <= (int)(ACC * lim_second);
		}
		if (is_broken) {
			temp_pool[l].grade = 0;
			broken++;
		}
		else
			lim_second = get_second(temp_pool[l]);
	} // all unnecessary gems destroyed

	pool_length_out = pool_length_in - broken;
	pool_out = make_uninitialized_pool<gem>(pool_length_out);
	std::copy_if(temp_pool + 0, temp_pool + pool_length_in, pool_out + 0, [](gem g) {return g.grade != 0;});
}

template<int ACC, class gem>
static inline void merge_subpool_2D(pool_t<gem>& subpool, size_t& subpool_length,
                                    const pool_t<gem>& temp_pool, size_t temp_length)
{
	size_t full_length = temp_length + subpool_length;
	pool_t<gem> full_array = make_uninitialized_pool<gem>(full_length);

	gem* temp_end = std::copy_n(temp_pool + 0, temp_length, full_array + 0);
	std::copy_n(subpool + 0, subpool_length, temp_end);

	compression_2D<ACC>(subpool, subpool_length, full_array, full_length);
}

template<size_t SIZE, class gem, class subpool_merger>
inline size_t fill_pool_nD(vector<pool_t<gem>>& pool, vector<size_t>& pool_length, int i,
                           subpool_merger&& merge_subpool)
{
	const int eoc = (i+1)/ (1+1);      // end of combining
	const int j0  = (i+1)/(10+1);      // value ratio < 10
	size_t comb_tot = 0;

	const int grade_max = (int)(log2(i + 1) + 1); // gems with max grade cannot be destroyed, so this is a max, not a sup
	pool_t<gem> temp_pools[grade_max - 1];        // get the temp pools for every grade
	size_t      temp_index[grade_max - 1];        // index of work point in temp pools
	pool_t<gem> subpools[grade_max - 1];          // get subpools for every grade
	size_t      subpools_length[grade_max - 1];
	for (int j = 0; j < grade_max - 1; ++j) {     // init everything
		temp_pools[j] = make_uninitialized_pool<gem>(SIZE);
		temp_index[j] = 0;
		subpools[j] = NULL;
		subpools_length[j] = 0;
	}

	for (int j = j0; j < eoc; ++j) {         // combine gems and put them in temp array
		for (size_t k = 0; k < pool_length[j]; ++k) {
			int g1 = (pool[j] + k)->grade;
			for (size_t h = 0; h < pool_length[i - 1 - j]; ++h) {
				int delta = g1 - (pool[i - 1 - j] + h)->grade;
				if (abs(delta) <= 2) {        // grade difference <= 2
					comb_tot++;
					gem temp;
					gem_combine(pool[j] + k, pool[i - 1 - j] + h, &temp);
					int grd = temp.grade - 2;
					temp_pools[grd][temp_index[grd]] = temp;
					temp_index[grd]++;
					if (temp_index[grd] == SIZE) {		// let's skim a pool
						merge_subpool(subpools[grd], subpools_length[grd], temp_pools[grd], SIZE);
						temp_index[grd] = 0;            // temp index reset
					}                                   // rebuilt subpool[grd], work restarts
				}
			}
		}
	}

	for (int grd = 0; grd < grade_max - 1; ++grd) {     // let's put remaining gems on
		if (temp_index[grd] != 0) {
			merge_subpool(subpools[grd], subpools_length[grd], temp_pools[grd], temp_index[grd]);
		}                                   // subpool[grd] is now full
	}

	pool_length[i] = std::accumulate(subpools_length, subpools_length + grade_max-1, 0);
	pool[i] = make_uninitialized_pool<gem>(pool_length[i]);

	int place = 0;
	for (int grd = 0; grd < grade_max - 1; ++grd) {     // copying to pool
		for (size_t j = 0; j < subpools_length[grd]; ++j) {
			pool[i][place] = subpools[grd][j];
			place++;
		}
	}

	return comb_tot;
}

template<size_t SIZE, int ACC, class gem>
inline size_t fill_pool_2D(vector<pool_t<gem>>& pool, vector<size_t>& pool_length, int i)
{
	return fill_pool_nD<SIZE, gem>(pool, pool_length, i, merge_subpool_2D<ACC, gem>);
}

template<const size_t SIZES[], int ACC, class gem>
inline auto fill_pool_2D_selector(int pool_zero)
{
	if (pool_zero == 1)
		return fill_pool_2D<SIZES[1], ACC, gem>;
	else
		return fill_pool_2D<SIZES[2], ACC, gem>;
}

template<int NCHECKS, class gem, class cascade_checker>
inline size_t fill_pool_c6(vector<pool_t<gem>>& pool, vector<size_t>& pool_length, int i,
                           cascade_checker&& cascade_checks)
{
	const int eoc=(i+1)/ (1+1);       // end of combining
	const int j0 =(i+1)/(10+1);       // value ratio < 10
	size_t comb_tot=0;

	const int ngrades=(int)log2(i+1);
	const int temp_length=NCHECKS*ngrades;
	gem temp_array[temp_length];      // this will have all the grades
	double pow_array[temp_length];    // this will have all the powers
	memset(temp_array, 0, sizeof(temp_array));
	memset(pow_array, 0, sizeof(pow_array));

	for (int j=j0; j<eoc; ++j) {          // combine gems and put them in temp array
		for (size_t k=0; k< pool_length[j]; ++k) {
			int g1=(pool[j]+k)->grade;
			for (size_t h=0; h< pool_length[i-1-j]; ++h) {
				int delta=g1 - (pool[i-1-j]+h)->grade;
				if (abs(delta)<=2) {     // grade difference <= 2
					comb_tot++;
					gem temp;
					gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
					int grd=temp.grade-2;
					int p0 = grd*NCHECKS;

					cascade_checks(temp, p0, temp_array, pow_array);
				}
			}
		}
	}
	int gemNum=0;
	for (int j=0; j<temp_length; ++j) if (temp_array[j].grade!=0) gemNum++;
	pool_length[i]=gemNum;
	pool[i] = make_uninitialized_pool<gem>(pool_length[i]);
	
	int place=0;
	for (int j=0; j<temp_length; ++j) {				// copying to pool
		if (temp_array[j].grade!=0) {
			pool[i][place]=temp_array[j];
			place++;
		}
	}

	return comb_tot;
}

#endif // _2D_UTILS_H
