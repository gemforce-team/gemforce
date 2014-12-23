#ifndef _SUPPRG_UTILS_H
#define _SUPPRG_UTILS_H

struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double suppr;		//using float does NOT improve time/memory usage
	struct Gem_O* father;
	struct Gem_O* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

inline double gem_power(gem gem1) {
	return gem1.suppr;
}

inline int gem_better(gem gem1, gem gem2)
{
	return gem1.suppr>gem2.suppr;
}

// -----------------
// Combining section
// -----------------

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->suppr > p_gem2->suppr) p_gem_combined->suppr = SUPPR_EQ_1*p_gem1->suppr + SUPPR_EQ_2*p_gem2->suppr;
	else p_gem_combined->suppr = SUPPR_EQ_1*p_gem2->suppr + SUPPR_EQ_2*p_gem1->suppr;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->suppr > p_gem2->suppr) p_gem_combined->suppr = SUPPR_D1_1*p_gem1->suppr + SUPPR_D1_2*p_gem2->suppr;
	else p_gem_combined->suppr = SUPPR_D1_1*p_gem2->suppr + SUPPR_D1_2*p_gem1->suppr;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->suppr > p_gem2->suppr) p_gem_combined->suppr = SUPPR_GN_1*p_gem1->suppr + SUPPR_GN_2*p_gem2->suppr;
	else p_gem_combined->suppr = SUPPR_GN_1*p_gem2->suppr + SUPPR_GN_2*p_gem1->suppr;
}

void gem_combine (gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
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

void gem_init(gem *p_gem, int grd, double suppr)
{
	p_gem->grade=grd;
	p_gem->suppr=suppr;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// -----------------
// Red adder section
// -----------------

#include "red_adder.h"

gem* gem_putred(gem* pool, int pool_length, int value, gem* red, gem** gem_array)
{
	gem_init(red,1,0);
	double best_pow = 0;
	gem* best_gem = NULL;
	gem* new_array = malloc(value*sizeof(gem));
	gem* best_array = malloc(value*sizeof(gem));
	
	for (int i=0; i<pool_length; ++i) {
		gem* gemf=pool+i;
		for (int last=0; last<value; last++) {
			int isRed=0;
			int curr=0;
			int new_index=0;
			gem* gp=gem_explore(gemf, &isRed, red, last, &curr, new_array, &new_index);
			double new_pow=gem_power(*gp);
			if (new_pow > best_pow) {
				best_pow=new_pow;
				best_gem=gp;
				// swap memory areas
				gem* temp = best_array;
				best_array = new_array;
				new_array = temp;
			}
		}
	}
	free(new_array);
	(*gem_array)=best_array;
	return best_gem;
}

#endif // _SUPPRG_UTILS_H