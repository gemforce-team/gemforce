#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double leech;
	struct Gem_O* father;
	struct Gem_O* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_stats.h"

inline int gem_better(gemO gem1, gemO gem2)
{
	return gem1.leech>gem2.leech;
}

void gem_print(gemO *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\n\n", p_gem->grade, p_gem->leech);
}

char gem_color(gemO*)
{
	return COLOR_LEECH;
}

// -----------------
// Combining section
// -----------------

void gem_comb_eq(gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_EQ_1*p_gem1->leech + LEECH_EQ_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_EQ_1*p_gem2->leech + LEECH_EQ_2*p_gem1->leech;
}

void gem_comb_d1(gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_D1_1*p_gem1->leech + LEECH_D1_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_D1_1*p_gem2->leech + LEECH_D1_2*p_gem1->leech;
}

void gem_comb_gn(gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = LEECH_GN_1*p_gem1->leech + LEECH_GN_2*p_gem2->leech;
	else p_gem_combined->leech = LEECH_GN_1*p_gem2->leech + LEECH_GN_2*p_gem1->leech;
}

void gem_combine(gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)
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

void gem_init(gemO *p_gem, int grd, float leech)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

#endif // _LEECH_UTILS_H
