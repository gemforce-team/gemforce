#ifndef _WMG_UTILS_H
#define _WMG_UTILS_H

const int ACC=1000;

/* Info: to go from low to high accuracy: change gems_sort in gems_sort_exact and
 * if ((int)(ACC*pool_big[subpools_to_big_convert(subpools_length,grd,j)].pbound)<=(int)(ACC*lim_pbound)) {
 * in
 * if (pool_big[subpools_to_big_convert(subpools_length,grd,j)].pbound<=lim_pbound) {
 */

struct Gem_OW {
	short grade;
	float leech;
	float pbound;
	struct Gem_OW* father;
	struct Gem_OW* mother;
};

int int_max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

double pbound_power(gem gem1)
{	// would be pow = ln(3.7 + PB * (1+TC)) * (1+WS) - ln(3.7) but at lv 60/60 I know the coeffs
	if (gem1.pbound==0) return 0;
	return (log(3.7 + 2.8 * gem1.pbound) * 3.4) - 1.308332;
}

void gem_print(gem *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\nPbound:\t%f\nPower:\t%f\n\n", p_gem->grade, p_gem->leech, p_gem->pbound, p_gem->leech*pbound_power(*p_gem));
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
	else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
	else p_gem_combined->leech = 0.9*p_gem2->leech + 0.38*p_gem1->leech;
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
	}																								// pbound is universal
	if (p_gem1->pbound > p_gem2->pbound) p_gem_combined->pbound = 0.87*p_gem1->pbound + 0.38*p_gem2->pbound;
	else p_gem_combined->pbound = 0.87*p_gem2->pbound + 0.38*p_gem1->pbound;
}

void gem_init(gem *p_gem, int grd, double leech, double pbound)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->pbound=pbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_less_equal(gem gem1, gem gem2)
{
	if ((int)(gem1.leech*ACC) != (int)(gem2.leech*ACC))
		return gem1.leech<gem2.leech;
	return gem1.pbound<gem2.pbound;
}

void ins_sort (gem* gems, int len)
{
	int i,j;
	gem element;
	for (i=1; i<len; i++) {
		element=gems[i];
		for (j=i; j>0 && gem_less_equal(element, gems[j-1]); j--) {
			gems[j]=gems[j-1];
		}
		gems[j]=element;
	}
}

void quick_sort (gem* gems, int len)
{
	if (len > 20)  {
		gem pivot = gems[len/2];
		gem* beg = gems;
		gem* end = gems+len-1;
		while (beg <= end) {
			while (gem_less_equal(*beg, pivot)) {
				beg++;
			}
			while (gem_less_equal(pivot,*end)) {
				end--;
			}
			if (beg <= end) {
				gem temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		if (end-gems+1 < gems-beg+len) {		// sort smaller first
			quick_sort(gems, end-gems+1);
			quick_sort(beg, gems-beg+len);
		}
		else {
			quick_sort(beg, gems-beg+len);
			quick_sort(gems, end-gems+1);
		}
	}
}

void gem_sort (gem* gems, int len)
{
	quick_sort (gems, len);		// partially sort
	ins_sort (gems, len);			// finish the nearly sorted array
}

double gem_power(gem gem1)
{
	return gem1.leech*pbound_power(gem1);     // amp-less
}

char gem_color(gem* p_gem)
{
	if (p_gem->leech==0) return 'w';
	if (p_gem->pbound==0) return 'o';
	else return 'm';
}

#include "print_utils.h"


#endif // _WMG_UTILS_H
