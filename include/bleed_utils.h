#ifndef _BLEED_UTILS_H
#define _BLEED_UTILS_H

#include <algorithm>

struct gem_R {
	int grade;			//using short does NOT improve time/memory usage
	double bleed;
	gem_R* father;
	gem_R* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

inline double gem_power(const gem_R& gem1) {
	return gem1.bleed;
}

inline void gem_print(const gem_R* p_gem) {
	printf("Grade:\t%d\nBleed:\t%f\n\n", p_gem->grade, p_gem->bleed);
}

inline char gem_color(const gem_R* p_gem) {
	if (p_gem->bleed==0) return COLOR_CHHIT;
	else return COLOR_BLEED;
}

// -----------------
// Combining section
// -----------------

inline void gem_comb_eq(const gem_R *p_gem1, const gem_R *p_gem2, gem_R *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->bleed > p_gem2->bleed) p_gem_combined->bleed = BLEED_EQ_1*p_gem1->bleed + BLEED_EQ_2*p_gem2->bleed;
	else p_gem_combined->bleed = BLEED_EQ_1*p_gem2->bleed + BLEED_EQ_2*p_gem1->bleed;
}

inline void gem_comb_d1(const gem_R *p_gem1, const gem_R *p_gem2, gem_R *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->bleed > p_gem2->bleed) p_gem_combined->bleed = BLEED_D1_1*p_gem1->bleed + BLEED_D1_2*p_gem2->bleed;
	else p_gem_combined->bleed = BLEED_D1_1*p_gem2->bleed + BLEED_D1_2*p_gem1->bleed;
}

inline void gem_comb_gn(const gem_R *p_gem1, const gem_R *p_gem2, gem_R *p_gem_combined)
{
	p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->bleed > p_gem2->bleed) p_gem_combined->bleed = BLEED_GN_1*p_gem1->bleed + BLEED_GN_2*p_gem2->bleed;
	else p_gem_combined->bleed = BLEED_GN_1*p_gem2->bleed + BLEED_GN_2*p_gem1->bleed;
}

inline void gem_combine(gem_R *p_gem1, gem_R *p_gem2, gem_R *p_gem_combined)
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

inline void gem_init(gem_R *p_gem, int grd, double bleed)
{
	p_gem->grade=grd;
	p_gem->bleed=bleed;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

#include "build_utils_1D.h"

// -------------------
// Chain adder section
// -------------------

#include "chain_adder.h"

gem_R* gem_putchain(const gem_R* pool, int pool_length, gem_R** gem_array)
{
	return gem_putchain_templ(pool, pool_length, gem_array,
							  [](gem_R* arg) {gem_init(arg, 1, 0);},
							  [](const gem_R& arg) {return gem_power(arg);});
}

#endif // _BLEED_UTILS_H
