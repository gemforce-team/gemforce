#ifndef _SBGEM_UTILS_H
#define _SBGEM_UTILS_H

const int ACC=200;			// accuracy for comparisons

struct Gem_OB {
	short grade;
	float suppr;
	float bbound;
	struct Gem_OB* father;
	struct Gem_OB* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

inline double gem_power(gem gem1)
{
	return gem1.suppr*gem1.bbound;
}

inline int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem1.suppr*gem1.bbound > gem2.suppr*gem2.bbound);		// optimization at infinity hits (hit lv infinity)
}

void gem_print(gem *p_gem) {
	printf("Grade:\t%d\nSuppr:\t%f\nBbound:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->suppr, p_gem->bbound, p_gem->suppr*p_gem->bbound);
}

char gem_color(gem* p_gem)
{
	if (p_gem->suppr==0 && p_gem->bbound==0) return COLOR_CHHIT;
	if (p_gem->suppr==0) return COLOR_BBOUND;
	if (p_gem->bbound==0) return COLOR_SUPPR;
	else return COLOR_SBGEM;
}

// -----------------
// Combining section
// -----------------

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->suppr > p_gem2->suppr) p_gem_combined->suppr = SUPPR_EQ_1*p_gem1->suppr + SUPPR_EQ_2*p_gem2->suppr;
	else p_gem_combined->suppr = SUPPR_EQ_1*p_gem2->suppr + SUPPR_EQ_2*p_gem1->suppr;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_EQ_1*p_gem1->bbound + BBOUND_EQ_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_EQ_1*p_gem2->bbound + BBOUND_EQ_2*p_gem1->bbound;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->suppr > p_gem2->suppr) p_gem_combined->suppr = SUPPR_D1_1*p_gem1->suppr + SUPPR_D1_2*p_gem2->suppr;
	else p_gem_combined->suppr = SUPPR_D1_1*p_gem2->suppr + SUPPR_D1_2*p_gem1->suppr;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_D1_1*p_gem1->bbound + BBOUND_D1_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_D1_1*p_gem2->bbound + BBOUND_D1_2*p_gem1->bbound;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->suppr > p_gem2->suppr) p_gem_combined->suppr = SUPPR_GN_1*p_gem1->suppr + SUPPR_GN_2*p_gem2->suppr;
	else p_gem_combined->suppr = SUPPR_GN_1*p_gem2->suppr + SUPPR_GN_2*p_gem1->suppr;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_GN_1*p_gem1->bbound + BBOUND_GN_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_GN_1*p_gem2->bbound + BBOUND_GN_2*p_gem1->bbound;
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

void gem_init(gem *p_gem, int grd, double suppr, double bbound)
{
	p_gem->grade=grd;
	p_gem->suppr=suppr;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// ---------------
// Sorting section
// ---------------

inline int gem_less_equal(gem gem1, gem gem2)
{
	if ((int)(gem1.suppr*ACC) != (int)(gem2.suppr*ACC))
		return gem1.suppr<gem2.suppr;
	return gem1.bbound<gem2.bbound;
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
	quick_sort (gems, len);    // partially sort
	ins_sort (gems, len);      // finish the nearly sorted array
}

inline int gem_less_eq_exact(gem gem1, gem gem2)
{
	if (gem1.suppr != gem2.suppr)
		return gem1.suppr<gem2.suppr;
	return gem1.bbound<gem2.bbound;
}

void ins_sort_exact (gem* gems, int len)
{
	int i,j;
	gem element;
	for (i=1; i<len; i++) {
		element=gems[i];
		for (j=i; j>0 && gem_less_eq_exact(element, gems[j-1]); j--) {
			gems[j]=gems[j-1];
		}
		gems[j]=element;
	}
}

void quick_sort_exact (gem* gems, int len)
{
	if (len > 20)  {
		gem pivot = gems[len/2];
		gem* beg = gems;
		gem* end = gems+len-1;
		while (beg <= end) {
			while (gem_less_eq_exact(*beg, pivot)) {
				beg++;
			}
			while (gem_less_eq_exact(pivot,*end)) {
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
			quick_sort_exact(gems, end-gems+1);
			quick_sort_exact(beg, gems-beg+len);
		}
		else {
			quick_sort_exact(beg, gems-beg+len);
			quick_sort_exact(gems, end-gems+1);
		}
	}
}

void gem_sort_exact (gem* gems, int len)
{
	quick_sort_exact (gems, len);    // partially sort
	ins_sort_exact (gems, len);      // finish the nearly sorted array
}

// -----------------
// Red adder section
// -----------------

#include "red_adder.h"

inline double gem_cfr_power(gem gem1, double amp_suppr_scaled)
{
	if (gem1.suppr==0) return 0;
	return (gem1.suppr+amp_suppr_scaled)*gem1.bbound;
}

gem* gem_putred(gem* pool, int pool_length, int value, gem* red, gem** gem_array, double amp_suppr_scaled)
{
	gem_init(red,1,0,0);
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
			double new_pow=gem_cfr_power(*gp, amp_suppr_scaled);
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

#endif // _SBGEM_UTILS_H
