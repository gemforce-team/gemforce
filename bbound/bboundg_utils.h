#ifndef _BBOUNDG_UTILS_H
#define _BBOUNDG_UTILS_H

struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double bbound;		//using float does NOT improve time/memory usage
	struct Gem_O* father;
	struct Gem_O* mother;
};

#include "gem_utils.h"

inline int gem_better(gem gem1, gem gem2)
{
	return gem1.bbound>gem2.bbound;
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.78*p_gem1->bbound + 0.31*p_gem2->bbound;
	else p_gem_combined->bbound = 0.78*p_gem2->bbound + 0.31*p_gem1->bbound;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.79*p_gem1->bbound + 0.29*p_gem2->bbound;
	else p_gem_combined->bbound = 0.79*p_gem2->bbound + 0.29*p_gem1->bbound;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.8*p_gem1->bbound + 0.27*p_gem2->bbound;
	else p_gem_combined->bbound = 0.8*p_gem2->bbound + 0.27*p_gem1->bbound;
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

void gem_init(gem *p_gem, int grd, double bbound)
{
	p_gem->grade=grd;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

#include "red_adder.h"

inline double gem_power(gem gem1) {
	return gem1.bbound;
}

gem* gem_putred(gem* pool, int pool_length, int value, gem* red, gem** gem_array)
{
	int isRed;
	int last;
	int curr;
	double best_pow=0;
	gem_init(red,1,0);
	gem* best_gem=NULL;
	gem* new_array;
	gem* best_array=NULL;
	int new_index;
	int i;
	for (i=0; i<pool_length; ++i) {
		gem* gemf=pool+i;
		for (last=0; last<value; last++) {
			isRed=0;
			curr=0;
			new_array=malloc(value*sizeof(gem));
			new_index=0;
			gem* gp=gem_explore(gemf, &isRed, red, last, &curr, new_array, &new_index);
			double new_pow=gem_power(*gp);
			if (new_pow > best_pow) {
				best_pow=new_pow;
				free(best_array);
				best_gem=gp;
				best_array=new_array;
			}
			else free(new_array);
		}
	}
	(*gem_array)=best_array;
	return best_gem;
}

#endif // _BBOUNDG_UTILS_H