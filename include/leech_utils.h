#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <algorithm>
#include <cstdio>

struct gem_O {
	gem_O* father;
	gem_O* mother;
	int grade;			//using short does NOT improve time/memory usage
	double leech;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

inline double gem_power(const gem_O& gem1) {
	return gem1.leech;
}

inline void gem_print(const gem_O* p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\n\n", p_gem->grade, p_gem->leech);
}

inline char gem_color(const gem_O* p_gem) {
	if (p_gem->leech==0) return COLOR_CHHIT;
	else return COLOR_LEECH;
}

// -----------------
// Combining section
// -----------------

inline void gem_comb_eq(const gem_O *p_gem1, const gem_O *p_gem2, gem_O *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_EQ_1*p_gem1->leech + LEECH_EQ_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_EQ_1*p_gem2->leech + LEECH_EQ_2*p_gem1->leech;
}

inline void gem_comb_d1(const gem_O *p_gem1, const gem_O *p_gem2, gem_O *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_D1_1*p_gem1->leech + LEECH_D1_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_D1_1*p_gem2->leech + LEECH_D1_2*p_gem1->leech;
}

inline void gem_comb_gn(const gem_O *p_gem1, const gem_O *p_gem2, gem_O *p_gem_combined)
{
	p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_GN_1*p_gem1->leech + LEECH_GN_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_GN_1*p_gem2->leech + LEECH_GN_2*p_gem1->leech;
}

inline void gem_combine(gem_O *p_gem1, gem_O *p_gem2, gem_O *p_gem_combined)
{
	p_gem_combined->father=p_gem1;
	p_gem_combined->mother=p_gem2;
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta){
		case 0:
			gem_comb_eq(p_gem1, p_gem2, p_gem_combined);
			break;
		case 1:
			gem_comb_d1(p_gem1, p_gem2, p_gem_combined);
			break;
		case -1:
			gem_comb_d1(p_gem2, p_gem1, p_gem_combined);
			break;
		default:
			gem_comb_gn(p_gem1, p_gem2, p_gem_combined);
			break;
	}
}

inline void gem_init(gem_O *p_gem, int grd, double leech)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// -----------------
// Pool init section
// -----------------

#include "1D_utils.h"

template<>
inline vector<pool_t<gem_O>> init_pool(int len, uint) {
	return init_pool_1D<gem_O>(len);
}

#endif // _LEECH_UTILS_H
