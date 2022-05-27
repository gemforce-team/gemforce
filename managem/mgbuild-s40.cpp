#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "managem_utils.h"
#include "cmdline_options.h"
#include "2D_utils.h"
#include "workers.h"

using gem = gem_OB;

template<size_t SIZE, size_t MAX_SIZE_GRD, int NUM_GRADES, int ACC, int SWITCH, int R0, int MAX_GRADE_DELTA>
inline size_t fill_pool(vector<pool_t<gem>>& pool, vector<size_t>& pool_length, int i)
{
	static_assert(SWITCH >= (1 << NUM_GRADES)); // ensure we never deal with less pools
	if (i < SWITCH)
		return fill_pool_2D<SIZE, ACC>(pool, pool_length, i);

	const int eoc = (i+1)/ (1+1);      // end of combining
	const int j0  = (i+1)/(R0+1);      // value ratio < R0
	size_t comb_tot = 0;

	const int grade_max = (int)(log2(i + 1) + 1); // gems with max grade cannot be destroyed, so this is a max, not a sup
	pool_t<gem> temp_pools[NUM_GRADES];        // get the temp pools for every grade
	size_t      temp_index[NUM_GRADES];        // index of work point in temp pools
	pool_t<gem> subpools[NUM_GRADES];          // get subpools for every grade
	size_t      subpools_length[NUM_GRADES];
	for (int j = 0; j < NUM_GRADES; ++j) {     // init everything
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
				if (abs(delta) <= MAX_GRADE_DELTA) {        // grade difference <= MAX_GRADE_DELTA
					comb_tot++;
					gem temp;
					gem_combine(pool[j] + k, pool[i - 1 - j] + h, &temp);
					int grd = temp.grade - (grade_max + 1) + NUM_GRADES;
					if (grd >= 0) {
						temp_pools[grd][temp_index[grd]] = temp;
						temp_index[grd]++;
						if (temp_index[grd] == SIZE) {		// let's skim a pool
							merge_subpool_2D<ACC>(subpools[grd], subpools_length[grd], temp_pools[grd], SIZE);
							temp_index[grd] = 0;            // temp index reset
						}                                   // rebuilt subpool[grd], work restarts
					}
				}
			}
		}
	}

	for (int grd = 0; grd < NUM_GRADES; ++grd) {     // let's put remaining gems on
		if (temp_index[grd] != 0) {
			merge_subpool_2D<ACC>(subpools[grd], subpools_length[grd], temp_pools[grd], temp_index[grd]);
		}                                   // subpool[grd] is now full
	}

	pool_length[i] = MAX_SIZE_GRD * NUM_GRADES; // every subpool is longer than MAX_SIZE_GRD by this point
	pool[i] = make_uninitialized_pool<gem>(pool_length[i]);

	// cut each subpool down to MAX_SIZE_GRD, choosing uniformly (first and last gem are always kept)
	int place = 0;
	for (int grd = 0; grd < NUM_GRADES; ++grd) {     // copying to pool
		for (size_t j = 0; j < MAX_SIZE_GRD; ++j) {
			size_t idx = size_t(j*(subpools_length[grd]-1)/(MAX_SIZE_GRD-1));
			pool[i][place] = subpools[grd][idx];
			place++;
		}
	}

	return comb_tot;
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);
	options.target.pool_zero = 2;

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		options.tables[0] = "table_mgs40";
	}

	constexpr size_t MAX_SIZE_GRD = 40; // the more the better
	constexpr int NUM_GRADES = 3; // going to 4 helps negligibly
	constexpr int ACC = 100;      // raising this makes things worse, as this does high quality preselection
	constexpr int SWITCH = 32;    // more doesn't help, this takes little extra time
	constexpr int R0 = 6;         // lowering doesn't help speed much
	constexpr int MAX_GRADE_DELTA = 1; // we don't need 2
	worker_build<gem>(options, fill_pool<SIZES[2], MAX_SIZE_GRD, NUM_GRADES, ACC, SWITCH, R0, MAX_GRADE_DELTA>);
	return 0;
}
