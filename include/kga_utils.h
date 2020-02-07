#ifndef _KGA_UTILS_H
#define _KGA_UTILS_H

#include <cstdio>

#include "sort_utils.h"
#include "build_utils_2D.h"
#include "options_utils.h"

inline double gem_amp_power(const gem_YB& gem1, const gem_Y& amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*gem1.bbound*(gem1.crit+crit_ratio*amp1.crit)*gem1.bbound;
}

void print_omnia_table(const gem_Y* amps, const double* powers, int len)
{
	printf("Killgem\tAmps\tPower\n");
	for (int i=0; i<len; i++)
		printf("%d\t%d\t%#.7g\n", i+1, gem_getvalue(amps+i), powers[i]);
	printf("\n");
}

template<class gem>
inline void amps_compression(gem** poolYf, int* poolYf_length, const gem*const* poolY, const int* poolY_length, int lena, options output_options)
{
	for (int i = 0; i < lena; ++i) {
		gem* temp_pool = (gem*)malloc(poolY_length[i] * sizeof(gem));
		std::copy_n(poolY[i], poolY_length[i], temp_pool);

		compression_2D<0>(poolYf + i, poolYf_length + i, temp_pool, poolY_length[i]);
		free(temp_pool);

		if (output_options.debug)
			printf("Amp value %d compressed pool size:\t%d\n", i+1, poolYf_length[i]);
	}
}

/* Exact macro blobs used for compressions in various files. Handle with more care */

struct gemP {
	int     grade;
	float   damage;
	float   crit;
	float   bbound;
	gem_YB* father;
	gem_YB* mother;
	int     place;
};

inline gemP gem2gemP(const gem_YB& g)
{
	return {g.grade, g.damage, g.crit, g.bbound, g.father, g.mother, 0};
}

inline gem_YB gemP2gem(const gemP& g)
{
	return {g.grade, g.damage, g.crit, g.bbound, g.father, g.mother};
}

inline double gem_bbd(const gemP& gem1)
{
	return gem1.bbound*gem1.bbound*gem1.damage;
}

inline double gem_bbc(const gemP& gem1)
{
	return gem1.bbound*gem1.bbound*gem1.crit;
}

inline void compression_3D_full(gem_YB** pool_out_p, int* pool_length_out_p, gemP* temp_pool, int pool_length_in)
{
	gem_sort(temp_pool, pool_length_in, AS_LAMBDA(gem_2_less<gemP>)); /* work starts */
	float lastcrit = -1;
	int tree_cells = 0;
	for (int l = 0; l < pool_length_in; ++l) {
		if (temp_pool[l].crit == lastcrit)
			temp_pool[l].place = tree_cells - 1;
		else {
			temp_pool[l].place = tree_cells++;
			lastcrit = temp_pool[l].crit;
		}
	}

	gem_sort(temp_pool, pool_length_in, AS_LAMBDA((gem_132_less<0, gemP>)));
	int broken=0;
	int tree_length = 1 << (int)ceil(log2(tree_cells));	/* this is pow(2, ceil()) bitwise */

	float* tree = (float*)malloc((tree_length + tree_cells + 1) * sizeof(float));
	std::fill_n(tree, tree_length + tree_cells + 1, 0); /* also remove gems with bb = 0 */

	for (int l = pool_length_in - 1; l >= 0; --l) { 	/* start from large dmg */
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

	for (int l=0; l<pool_length_in; ++l) {					/* start from low dmg = high idmg */
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
	for (int l=0; l<pool_length_in; ++l) {					/* start from low dmg = high idmg */
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
	for (int l=pool_length_in-1; l>=0; --l) {				/* start from large dmg */
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

	*pool_length_out_p = pool_length_in - broken;
	*pool_out_p = (gem_YB*)malloc(*pool_length_out_p * sizeof(gem_YB));
	int index = 0;
	for (int j = 0; j < pool_length_in; ++j) {
		if (temp_pool[j].grade != 0) {
			(*pool_out_p)[index] = gemP2gem(temp_pool[j]);
			index++;
		}
	}
}

template<class gem>
inline void kgspec_compression(gem** poolf, int* poolf_length, const gem*const* pool, const int* pool_length, int len, options output_options)
{
	for (int i = 0; i < len; ++i) {
		gemP* temp_array = (gemP*)malloc(pool_length[i] * sizeof(gemP));
		for (int j = 0; j < pool_length[i]; ++j) {
			temp_array[j] = gem2gemP(pool[i][j]);
		}

		compression_3D_full(poolf + i, poolf_length + i, temp_array, pool_length[i]);
		free(temp_array);

		if (output_options.debug)
			printf("Killgem value %d speccing compressed pool size:\t%d\n", i + 1, poolf_length[i]);
	}
}

template<class gem>
inline void kgcomb_compression(gem** poolcf_p, int* poolcf_length_p, gem* poolc, int poolc_length)
{
	gemP* temp_array = (gemP*)malloc(poolc_length * sizeof(gemP));
	for (int j = 0; j < poolc_length; ++j) {
		temp_array[j] = gem2gemP(poolc[j]);
	}

	compression_3D_full(poolcf_p, poolcf_length_p, temp_array, poolc_length);
	free(temp_array);
}


#endif // _KGA_UTILS_H
