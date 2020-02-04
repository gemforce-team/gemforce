#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

#include <algorithm>

struct gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double leech;
	gem_O* father;
	gem_O* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

inline double gem_power(gem_O gem1) {
	return gem1.leech;
}

inline void gem_print(gem_O *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\n\n", p_gem->grade, p_gem->leech);
}

inline char gem_color(gem_O* p_gem) {
	if (p_gem->leech==0) return COLOR_CHHIT;
	else return COLOR_LEECH;
}

// -----------------
// Combining section
// -----------------

void gem_comb_eq(gem_O *p_gem1, gem_O *p_gem2, gem_O *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_EQ_1*p_gem1->leech + LEECH_EQ_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_EQ_1*p_gem2->leech + LEECH_EQ_2*p_gem1->leech;
}

void gem_comb_d1(gem_O *p_gem1, gem_O *p_gem2, gem_O *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_D1_1*p_gem1->leech + LEECH_D1_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_D1_1*p_gem2->leech + LEECH_D1_2*p_gem1->leech;
}

void gem_comb_gn(gem_O *p_gem1, gem_O *p_gem2, gem_O *p_gem_combined)
{
	p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_GN_1*p_gem1->leech + LEECH_GN_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_GN_1*p_gem2->leech + LEECH_GN_2*p_gem1->leech;
}

void gem_combine(gem_O *p_gem1, gem_O *p_gem2, gem_O *p_gem_combined)
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

// -------------------
// Chain adder section
// -------------------

#include "chain_adder.h"

gem_O* gem_putchain(gem_O* pool, int pool_length, gem_O** gem_array)
{
	return gem_putchain_templ(pool, pool_length, gem_array,
							  [](gem_O* arg) {gem_init(arg, 1, 0);},
							  [](gem_O arg) {return gem_power(arg);});
}

#endif // _LEECH_UTILS_H
