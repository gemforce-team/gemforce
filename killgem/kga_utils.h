#ifndef _KGA_UTILS_H
#define _KGA_UTILS_H

#include <cstdio>

#include "crit_utils.h"
#include "interval_tree.h"
#include "killgem_utils.h"
#include "sort_utils.h"
#include "2D_utils.h"

inline double gem_amp_power(const gem_YB& gem1, const gem_Y& amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*gem1.bbound*(gem1.crit+crit_ratio*amp1.crit)*gem1.bbound;
}

template<class gemA>
inline void amps_compression(vector<pool_t<gemA>>& poolAf, vector<size_t>& poolAf_length,
                             const vector<pool_t<gemA>>& poolA, const vector<size_t>& poolA_length,
							 int lena, bool debug)
{
	for (int i = 0; i < lena; ++i) {
		pool_t<gemA> temp_pool = make_uninitialized_pool<gemA>(poolA_length[i]);
		std::copy_n(poolA[i] + 0, poolA_length[i], temp_pool + 0);

		compression_2D<0>(poolAf[i], poolAf_length[i], temp_pool, poolA_length[i]);

		if (debug)
			printf("Amp value %d compressed pool size:\t%zu\n", i+1, poolAf_length[i]);
	}
}

/* Exact blobs used for compressions in various files. Handle with care */

struct gemP {
	gem_YB* father;
	gem_YB* mother;
	int     grade;
	float   damage;
	float   crit;
	float   bbound;
	int     place;
};

inline gemP gem2gemP(const gem_YB& g)
{
	return {g.father, g.mother, g.grade, g.damage, g.crit, g.bbound, 0};
}

inline gem_YB gemP2gem(const gemP& g)
{
	return {g.father, g.mother, g.grade, g.damage, g.crit, g.bbound};
}

inline double gem_bbd(const gemP& gem1)
{
	return gem1.bbound*gem1.bbound*gem1.damage;
}

inline double gem_bbc(const gemP& gem1)
{
	return gem1.bbound*gem1.bbound*gem1.crit;
}

inline void compression_3D_full(pool_t<gem_YB>& pool_out, size_t& pool_length_out,
                                pool_t<gemP>& temp_pool, size_t pool_length_in)
{
	gem_sort(temp_pool + 0, pool_length_in, AS_LAMBDA(gem_2_less<gemP>)); /* work starts */
	float lastcrit = -1;
	int tree_cells = 0;
	for (size_t l = 0; l < pool_length_in; ++l) {
		if (temp_pool[l].crit == lastcrit)
			temp_pool[l].place = tree_cells - 1;
		else {
			temp_pool[l].place = tree_cells++;
			lastcrit = temp_pool[l].crit;
		}
	}

	gem_sort(temp_pool + 0, pool_length_in, AS_LAMBDA((gem_132_less<0, gemP>)));
	int broken=0;
	int tree_length = 1 << (int)ceil(log2(tree_cells));	/* this is pow(2, ceil()) bitwise */

	float* tree = (float*)malloc((tree_length + tree_cells + 1) * sizeof(float));
	std::fill_n(tree, tree_length + tree_cells + 1, 0); /* also remove gems with bb = 0 */

	for (ssize_t l = pool_length_in - 1; l >= 0; --l) { 	/* start from large dmg */
		gemP* p_gem = temp_pool + l;
		if (tree_check_after(tree, tree_length, p_gem->place, p_gem->bbound)) {
			tree_add_element(tree, tree_length, p_gem->place, p_gem->bbound);
		}
		else {
			p_gem->grade = 0;
			broken++;
		}
	}
	free(tree);

	double* dtree = (double*)malloc((tree_length + tree_cells + 1)*sizeof(double));
	std::fill_n(dtree, tree_length + tree_cells + 1, 0);	/* delete gems with power=0 */

	for (size_t l=0; l<pool_length_in; ++l) {					/* start from low dmg = high idmg */
		gemP* p_gem=temp_pool+l;
		if (p_gem->grade==0) continue;
		int place = tree_cells - p_gem->place;				/* reverse crit order */
		if (tree_check_after(dtree, tree_length, place, gem_power(*p_gem))) {
			tree_add_element(dtree, tree_length, place, gem_power(*p_gem));
		}
		else {
			p_gem->grade=0;
			broken++;
		}
	}

	std::fill_n(dtree, tree_length + tree_cells + 1, 0);	/* bbd - id - c compression */
	for (size_t l=0; l<pool_length_in; ++l) {					/* start from low dmg = high idmg */
		gemP* p_gem=temp_pool+l;
		if (p_gem->grade==0) continue;
		int place = p_gem->place;							/* regular crit order */
		if (tree_check_after(dtree, tree_length, place, gem_bbd(*p_gem))) {
			tree_add_element(dtree, tree_length, place, gem_bbd(*p_gem));
		}
		else {
			p_gem->grade=0;
			broken++;
		}
	}

	std::fill_n(dtree, tree_length + tree_cells + 1, 0);	/* bbc - d - ic compression */
	for (ssize_t l=pool_length_in-1; l>=0; --l) {				/* start from large dmg */
		gemP* p_gem=temp_pool+l;
		if (p_gem->grade==0) continue;
		int place = tree_cells - p_gem->place;				/* reverse crit order */
		if (tree_check_after(dtree, tree_length, place, gem_bbc(*p_gem))) {
			tree_add_element(dtree, tree_length, place, gem_bbc(*p_gem));
		}
		else {
			p_gem->grade=0;
			broken++;
		}
	}

	free(dtree);

	pool_length_out = pool_length_in - broken;
	pool_out = make_uninitialized_pool<gem_YB>(pool_length_out);
	int index = 0;
	for (size_t j = 0; j < pool_length_in; ++j) {
		if (temp_pool[j].grade != 0) {
			pool_out[index] = gemP2gem(temp_pool[j]);
			index++;
		}
	}
}

inline void specs_compression(vector<pool_t<gem_YB>>& poolf, vector<size_t>& poolf_length,
                              const vector<pool_t<gem_YB>>& pool, const vector<size_t>& pool_length,
							  size_t len, bool debug)
{
	for (size_t i = 0; i < len; ++i) {
		pool_t<gemP> temp_pool = make_uninitialized_pool<gemP>(pool_length[i]);
		for (size_t j = 0; j < pool_length[i]; ++j) {
			temp_pool[j] = gem2gemP(pool[i][j]);
		}

		compression_3D_full(poolf[i], poolf_length[i], temp_pool, pool_length[i]);

		if (debug)
			printf("Killgem value %zu speccing compressed pool size:\t%zu\n", i + 1, poolf_length[i]);
	}
}

inline void combs_compression(pool_t<gem_YB>& poolcf, size_t& poolcf_length,
                              const pool_t<gem_YB>& poolc, size_t poolc_length)
{
	pool_t<gemP> temp_pool = make_uninitialized_pool<gemP>(poolc_length);
	for (size_t j = 0; j < poolc_length; ++j) {
		temp_pool[j] = gem2gemP(poolc[j]);
	}

	compression_3D_full(poolcf, poolcf_length, temp_pool, poolc_length);
}


#endif // _KGA_UTILS_H
