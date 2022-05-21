#ifndef _3D_UTILS_H
#define _3D_UTILS_H

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <numeric>

#include "2D_utils.h"
#include "sort_utils.h"
#include "interval_tree.h"

template<int ACC, int ACC_TR, class gem>
inline void compression_3D(pool_t<gem>& pool_out, size_t& pool_length_out,
                           pool_t<gem>& temp_pool, size_t pool_length_in)
{
	int broken = 0;
	using second_t = decltype(get_second(gem{}));

	if constexpr (ACC == 0 && ACC_TR == 0) {
		gem_sort(temp_pool.get(), pool_length_in, AS_LAMBDA(gem_2_less<gem>));
		second_t last_second = -1;
		int tree_cells = 0;
		for (size_t l = 0; l < pool_length_in; ++l) {
			if (get_second(temp_pool[l]) == last_second)
				set_place(temp_pool[l], tree_cells - 1);
			else {
				set_place(temp_pool[l], tree_cells++);
				last_second = get_second(temp_pool[l]);
			}
		}

		gem_sort(temp_pool.get(), pool_length_in, AS_LAMBDA((gem_132_less<ACC, gem>)));

		int tree_length = 1u << (int)ceil(log2(tree_cells));		// this is pow(2, ceil()) bitwise for speed improvement

		second_t* tree = (second_t*)malloc((tree_length + tree_cells + 1) * sizeof(second_t));
		std::fill_n(tree, tree_length + tree_cells + 1, -1); // init also tree[0], it's faster

		for (int l = pool_length_in - 1; l >= 0; --l) {				// start from large z
			gem& r_gem = temp_pool[l];
			if (tree_check_after(tree, tree_length, get_place(r_gem), get_third(r_gem))) {
				tree_add_element(tree, tree_length, get_place(r_gem), get_third(r_gem));
			}
			else {
				r_gem.grade = 0;
				broken++;
			}
		}												// all unnecessary gems destroyed
	}
	else {
		second_t max_second = 0;				// this will help me create the minimum tree
		for (size_t l = 0; l < pool_length_in; ++l) {
			max_second = std::max(max_second, get_second(temp_pool[l]));
		}

		gem_sort(temp_pool.get(), pool_length_in, AS_LAMBDA((gem_132_less<ACC, gem>)));

		int crit_cells = (int)(max_second * ACC) + 1;		// this pool will be big from the beginning, but we avoid binary search
		int tree_length = 1 << (int)ceil(log2(crit_cells));		// this is pow(2, ceil()) bitwise for speed improvement

		int* tree = (int*)malloc((tree_length + crit_cells + 1) * sizeof(int));	// memory improvement, 2* is not needed
		std::fill_n(tree, tree_length + crit_cells + 1, -1); // init also tree[0], it's faster

		for (int l = pool_length_in - 1; l >= 0; --l) {									// start from large z
			gem& r_gem = temp_pool[l];
			int index = (int)(get_second(r_gem) * ACC);								// find its place in x
			if (tree_check_after(tree, tree_length, index, (int)(get_third(r_gem) * ACC_TR))) {	// look at y
				tree_add_element(tree, tree_length, index, (int)(get_third(r_gem) * ACC_TR));
			}
			else {
				r_gem.grade = 0;
				broken++;
			}
		}											// all unnecessary gems destroyed
	}

	pool_length_out = pool_length_in - broken;
	pool_out = make_uninitialized_pool<gem>(pool_length_out);
	std::copy_if(temp_pool + 0, temp_pool + pool_length_in, pool_out + 0, [](gem g) {return g.grade != 0;});
}

template<int ACC, int ACC_TR, class gem>
static inline void merge_subpool_3D(pool_t<gem>& subpool, size_t& subpool_length,
                                    const pool_t<gem>& temp_pool, size_t temp_length)
{
	size_t full_length = temp_length + subpool_length;
	pool_t<gem> full_array = make_uninitialized_pool<gem>(full_length);

	gem* temp_end = std::copy_n(temp_pool + 0, temp_length, full_array + 0);
	std::copy_n(subpool + 0, subpool_length, temp_end);

	compression_3D<ACC, ACC_TR>(subpool, subpool_length, full_array, full_length);
}

template<size_t SIZE, int ACC, int ACC_TR, class gem>
inline size_t fill_pool_3D(vector<pool_t<gem>>& pool, vector<size_t>& pool_length, int i)
{
	return fill_pool_nD<SIZE, gem>(pool, pool_length, i, merge_subpool_3D<ACC, ACC_TR, gem>);
}

template<const size_t SIZES[], const int ACCS[], int ACC_TR, class gem>
inline auto fill_pool_3D_selector(int pool_zero)
{
	if (pool_zero == 1)
		return fill_pool_3D<SIZES[1], ACCS[1], ACC_TR, gem>;
	else
		return fill_pool_3D<SIZES[2], ACCS[2], ACC_TR, gem>;
}

#endif // _3D_UTILS_H
