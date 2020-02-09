#ifndef _BUILD_UTILS_3D_H
#define _BUILD_UTILS_3D_H

#include <sort_utils.h>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <numeric>

#include "interval_tree.h"

template<int ACC, int ACC_TR, class gem>
static inline void merge_subpool(gem** subpool_p, int* subpool_length_p, const gem* temp_pool, int temp_length)
{
	int full_length = temp_length + *subpool_length_p;
	gem* full_array = (gem*)malloc(full_length * sizeof(gem));

	gem* temp_end = std::copy_n(temp_pool, temp_length, full_array);
	std::copy_n(*subpool_p, *subpool_length_p, temp_end);

	free(*subpool_p);    // free old pool

	int broken = 0;
	using second_t = decltype(get_second(gem{}));

	if constexpr (ACC == 0 && ACC_TR == 0) {
		gem_sort(full_array, full_length, AS_LAMBDA(gem_2_less<gem>));
		second_t last_second = -1;
		int tree_cells = 0;
		for (int l = 0; l < full_length; ++l) {
			if (get_second(full_array[l]) == last_second)
				set_place(full_array[l], tree_cells - 1);
			else {
				set_place(full_array[l], tree_cells++);
				last_second = get_second(full_array[l]);
			}
		}

		gem_sort(full_array, full_length, AS_LAMBDA((gem_132_less<ACC, gem>)));

		int tree_length = 1u << (int)ceil(log2(tree_cells));		// this is pow(2, ceil()) bitwise for speed improvement

		second_t* tree = (second_t*)malloc((tree_length + tree_cells + 1) * sizeof(second_t));
		std::fill_n(tree, tree_length + tree_cells + 1, -1); // init also tree[0], it's faster

		for (int l = full_length - 1; l >= 0; --l) {				// start from large z
			gem& r_gem = full_array[l];
			if (tree_check_after(tree, tree_length, get_place(r_gem), get_third(r_gem))) {
				tree_add_element(tree, tree_length, get_place(r_gem), get_third(r_gem));
			}
			else {
				r_gem.grade = 0;
				broken++;
			}
		}												// all unnecessary gems destroyed

		free(tree);
	}
	else {
		second_t max_second = 0;				// this will help me create the minimum tree
		for (int l = 0; l < full_length; ++l) {
			max_second = std::max(max_second, get_second(full_array[l]));
		}

		gem_sort(full_array, full_length, AS_LAMBDA((gem_132_less<ACC, gem>)));

		int crit_cells = (int)(max_second * ACC) + 1;		// this pool will be big from the beginning, but we avoid binary search
		int tree_length = 1 << (int)ceil(log2(crit_cells));				// this is pow(2, ceil()) bitwise for speed improvement

		int* tree = (int*)malloc((tree_length + crit_cells + 1) * sizeof(int));	// memory improvement, 2* is not needed
		std::fill_n(tree, tree_length + crit_cells + 1, -1); // init also tree[0], it's faster

		for (int l = full_length - 1; l >= 0; --l) {									// start from large z
			gem& r_gem = full_array[l];
			int index = (int)(get_second(r_gem) * ACC);								// find its place in x
			if (tree_check_after(tree, tree_length, index, (int)(get_third(r_gem) * ACC_TR))) {	// look at y
				tree_add_element(tree, tree_length, index, (int)(get_third(r_gem) * ACC_TR));
			}
			else {
				r_gem.grade = 0;
				broken++;
			}
		}											// all unnecessary gems destroyed

		free(tree);
	}

	*subpool_length_p = full_length - broken;
	*subpool_p = (gem*)malloc(*subpool_length_p * sizeof(gem));   // pool init via broken
	std::copy_if(full_array, full_array + full_length, *subpool_p, [](gem g) {return g.grade != 0;});

	free(full_array);			// free
}

template<unsigned int SIZE, int ACC, int ACC_TR, class gem>
inline int fill_pool_3D(gem** pool, int* pool_length, int i)
{
	const int eoc = (i + 1) / (1 + 1);      // end of combining
	const int j0 = (i + 1) / (10 + 1);      // value ratio < 10
	int comb_tot = 0;

	const int grade_max = (int)(log2(i + 1) + 1); // gems with max grade cannot be destroyed, so this is a max, not a sup
	gem* temp_pools[grade_max - 1];               // get the temp pools for every grade
	int  temp_index[grade_max - 1];               // index of work point in temp pools
	gem* subpools[grade_max - 1];                 // get subpools for every grade
	int  subpools_length[grade_max - 1];
	for (int j = 0; j < grade_max - 1; ++j) {     // init everything
		temp_pools[j] = (gem*)malloc(SIZE * sizeof(gem));
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
					if (temp_index[grd] == SIZE) {		// let's skim a pool
						merge_subpool<ACC, ACC_TR>(subpools + grd, subpools_length + grd, temp_pools[grd], SIZE);
						temp_index[grd] = 0;			// temp index reset
					}									// rebuilt subpool[grd], work restarts
				}
			}
		}
	}

	for (int grd = 0; grd < grade_max - 1; ++grd) {	// let's put remaining gems on
		if (temp_index[grd] != 0) {
			merge_subpool<ACC, ACC_TR>(subpools + grd, subpools_length + grd, temp_pools[grd], temp_index[grd]);
		}									// subpool[grd] is now full
	}

	pool_length[i] = std::accumulate(subpools_length, subpools_length + grade_max - 1, 0);
	pool[i] = (gem*)malloc(pool_length[i] * sizeof(gem));

	int place = 0;
	for (int grd = 0; grd < grade_max - 1; ++grd) {			// copying to pool
		for (int j = 0; j < subpools_length[grd]; ++j) {
			pool[i][place] = subpools[grd][j];
			place++;
		}
	}
	for (int grd = 0; grd < grade_max - 1; ++grd) {			// free
		free(temp_pools[grd]);
		free(subpools[grd]);
	}

	return comb_tot;
}

#endif // _BUILD_UTILS_3D_H
