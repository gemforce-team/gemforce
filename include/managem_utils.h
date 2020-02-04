#ifndef _MANAGEM_UTILS_H
#define _MANAGEM_UTILS_H

#include <algorithm>

const int ACC=1000;			// accuracy for comparisons

struct gem_OB {
	short grade;
	float leech;
	float bbound;
	gem_OB* father;
	gem_OB* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

inline double gem_power(gem_OB gem1)
{
	return gem1.leech*gem1.bbound;
}

void gem_print(gem_OB* p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\nBbound:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->leech, p_gem->bbound, p_gem->leech*p_gem->bbound);
}

inline char gem_color(gem_OB* p_gem)
{
	if (p_gem->leech==0 && p_gem->bbound==0) return COLOR_CHHIT;
	if (p_gem->leech==0) return COLOR_BBOUND;
	if (p_gem->bbound==0) return COLOR_LEECH;
	else return COLOR_MANAGEM;
}

// -----------------
// Combining section
// -----------------

void gem_comb_eq(gem_OB *p_gem1, gem_OB *p_gem2, gem_OB *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_EQ_1*p_gem1->leech + LEECH_EQ_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_EQ_1*p_gem2->leech + LEECH_EQ_2*p_gem1->leech;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_EQ_1*p_gem1->bbound + BBOUND_EQ_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_EQ_1*p_gem2->bbound + BBOUND_EQ_2*p_gem1->bbound;
}

void gem_comb_d1(gem_OB *p_gem1, gem_OB *p_gem2, gem_OB *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_D1_1*p_gem1->leech + LEECH_D1_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_D1_1*p_gem2->leech + LEECH_D1_2*p_gem1->leech;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_D1_1*p_gem1->bbound + BBOUND_D1_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_D1_1*p_gem2->bbound + BBOUND_D1_2*p_gem1->bbound;
}

void gem_comb_gn(gem_OB *p_gem1, gem_OB *p_gem2, gem_OB *p_gem_combined)
{
	p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_GN_1*p_gem1->leech + LEECH_GN_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_GN_1*p_gem2->leech + LEECH_GN_2*p_gem1->leech;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_GN_1*p_gem1->bbound + BBOUND_GN_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_GN_1*p_gem2->bbound + BBOUND_GN_2*p_gem1->bbound;
}

void gem_combine (gem_OB *p_gem1, gem_OB *p_gem2, gem_OB *p_gem_combined)
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

inline void gem_init(gem_OB *p_gem, int grd, double leech, double bbound)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// ---------------
// Sorting section
// ---------------

inline bool gem_less_equal(gem_OB gem1, gem_OB gem2)
{
	if ((int)(gem1.leech*ACC) != (int)(gem2.leech*ACC))
		return gem1.leech<gem2.leech;
	return gem1.bbound<gem2.bbound;
}

inline bool gem_less_eq_exact(gem_OB gem1, gem_OB gem2)
{
	if (gem1.leech != gem2.leech)
		return gem1.leech<gem2.leech;
	return gem1.bbound<gem2.bbound;
}

// -------------------
// Chain adder section
// -------------------

inline double gem_cfr_power(gem_OB gem1, double amp_leech_scaled)
{
	if (gem1.leech==0) return 0;
	return (gem1.leech+amp_leech_scaled)*gem1.bbound;
}

#define EXTRA_PARAMS   , double amp_leech_scaled
#define CHAIN_INIT_EXPR(ARG) gem_init(ARG, 1, 0, 0)
#define CFR_EXPR(ARG)  gem_cfr_power(ARG, amp_leech_scaled)
#include "chain_adder.h"

#endif // _MANAGEM_UTILS_H
