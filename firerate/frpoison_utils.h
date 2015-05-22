#ifndef _FRPOISON_UTILS_H
#define _FRPOISON_UTILS_H

/* From crit */

struct Gem_GF {
	int grade;			// using short does NOT improve time/memory usage
	float poison;
	float firerate;	// not enforced, but fr < 48
	struct Gem_GF* father;
	struct Gem_GF* mother;
};

#include "gem_utils.h"

inline double gem_power(gem gem1)
{
	return gem1.poison*gem1.firerate;
}

inline int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem_power(gem1) > gem_power(gem2));
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->poison > p_gem2->poison) p_gem_combined->poison = 0.96*p_gem1->poison + 0.85*p_gem2->poison;
	else p_gem_combined->poison = 0.96*p_gem2->poison + 0.85*p_gem1->poison;
	if (p_gem1->firerate > p_gem2->firerate) p_gem_combined->firerate = 0.74*p_gem1->firerate + 0.44*p_gem2->firerate;
	else p_gem_combined->firerate = 0.74*p_gem2->firerate + 0.44*p_gem1->firerate;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->poison > p_gem2->poison) p_gem_combined->poison = 0.97*p_gem1->poison + 0.62*p_gem2->poison;
	else p_gem_combined->poison = 0.97*p_gem2->poison + 0.62*p_gem1->poison;
	if (p_gem1->firerate > p_gem2->firerate) p_gem_combined->firerate = 0.8*p_gem1->firerate + 0.25*p_gem2->firerate;
	else p_gem_combined->firerate = 0.8*p_gem2->firerate + 0.25*p_gem1->firerate;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->poison > p_gem2->poison) p_gem_combined->poison = 0.98*p_gem1->poison + 0.42*p_gem2->poison;
	else p_gem_combined->poison = 0.98*p_gem2->poison + 0.42*p_gem1->poison;
	if (p_gem1->firerate > p_gem2->firerate) p_gem_combined->firerate = 0.92*p_gem1->firerate + 0.09*p_gem2->firerate;
	else p_gem_combined->firerate = 0.92*p_gem2->firerate + 0.09*p_gem1->firerate; 
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
	if (p_gem_combined->poison < p_gem1->poison) p_gem_combined->poison = p_gem1->poison;
	if (p_gem_combined->poison < p_gem2->poison) p_gem_combined->poison = p_gem2->poison;
}

void gem_init(gem *p_gem, int grd, double poison, double firerate)
{
	p_gem->grade =grd;
	p_gem->poison=poison;
	p_gem->firerate  =firerate;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

inline int gem_less_equal(gem gem1, gem gem2)
{
	if (gem1.poison < gem2.poison) return 1;
	else if (gem1.poison == gem2.poison && gem1.firerate < gem2.firerate) return 1;
	else return 0;
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
	ins_sort (gems, len);		// finish the nearly sorted array
}

#include "red_adder.h"

gem* gem_putred(gem* pool, int pool_length, int value, gem* red, gem** gem_array)
{
	int isRed;
	int last;
	int curr;
	double best_pow=0;
	gem_init(red,1,0,0);
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
				if (best_gem!=NULL) free(best_array);
				best_gem=gp;
				best_array=new_array;
			}
			else free(new_array);
		}
	}
	(*gem_array)=best_array;
	return best_gem;
}

#endif // _FRPOISON_UTILS_H
