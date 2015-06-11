#ifndef _CHAINHIT_UTILS_H
#define _CHAINHIT_UTILS_H

struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double chainhit;		//using float does NOT improve time/memory usage
	struct Gem_O* father;
	struct Gem_O* mother;
};

#include "gem_utils.h"

inline double chain_length(double chainhit)
{
	// Red skill is assumed at 15
	double length = log(3.7 + chainhit) / log(3.7) * (1 + 15*0.04);
	if(length > 8) length = 7 + pow(length - 7, 0.7);
	return length;
}

inline int gem_better(gem gem1, gem gem2)
{
	return gem1.chainhit>gem2.chainhit;
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->chainhit > p_gem2->chainhit) p_gem_combined->chainhit = 0.88*p_gem1->chainhit + 0.5*p_gem2->chainhit;
	else p_gem_combined->chainhit = 0.88*p_gem2->chainhit + 0.5*p_gem1->chainhit;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->chainhit > p_gem2->chainhit) p_gem_combined->chainhit = 0.9*p_gem1->chainhit + 0.47*p_gem2->chainhit;
	else p_gem_combined->chainhit = 0.9*p_gem2->chainhit + 0.47*p_gem1->chainhit;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->chainhit > p_gem2->chainhit) p_gem_combined->chainhit = 0.92*p_gem1->chainhit + 0.44*p_gem2->chainhit;
	else p_gem_combined->chainhit = 0.92*p_gem2->chainhit + 0.44*p_gem1->chainhit;
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

void gem_init(gem *p_gem, int grd, double chainhit)
{
	p_gem->grade=grd;
	p_gem->chainhit=chainhit;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

inline double gem_power(gem gem1) {
	return gem1.chainhit;
}

#endif // _CHAINHIT_UTILS_H
