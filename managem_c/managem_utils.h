#ifndef _MANAGEM_UTILS_H
#define _MANAGEM_UTILS_H

//const int ACC=1000; 			// EXPLICITELY PUT IT IN THE OTHER FILE

/* Info: to go from low to high accuracy: change gems_sort in gems_sort_exact and
 * if ((int)(ACC*pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound)<=(int)(ACC*lim_bbound)) {
 * in
 * if (pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound<=lim_bbound) {
 */

struct Gem_OB {
	short grade;
	float leech;
	float bbound;
	struct Gem_OB* father;
	struct Gem_OB* mother;
};

// remember to define the gem in your file

void line_from_table(FILE* table, gem* p_gem, int* value_father, int* offset_father, int* offset_mother)
{
	fscanf(table, "%hd %a %a %d %d %d\n", &(p_gem->grade), &(p_gem->leech), &(p_gem->bbound), value_father, offset_father, offset_mother);
}

void line_write_iteration(FILE* table, gem* p_gem)
{
	fprintf(table, " %a %a", p_gem->leech, p_gem->bbound);
}

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem1.leech*gem1.bbound > gem2.leech*gem2.bbound);   // optimization at infinity hits (hit lv infinity)
}

int int_max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

void gem_print(gem *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\nBbound:\t%f\nPower:\t%f\n\n", p_gem->grade, p_gem->leech, p_gem->bbound, p_gem->leech*p_gem->bbound);
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.78*p_gem1->bbound + 0.31*p_gem2->bbound;
	else p_gem_combined->bbound = 0.78*p_gem2->bbound + 0.31*p_gem1->bbound;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
	else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.79*p_gem1->bbound + 0.29*p_gem2->bbound;
	else p_gem_combined->bbound = 0.79*p_gem2->bbound + 0.29*p_gem1->bbound;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
	else p_gem_combined->leech = 0.9*p_gem2->leech + 0.38*p_gem1->leech;
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

void gem_init(gem *p_gem, int grd, double leech, double bbound)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_less_equal(gem gem1, gem gem2)
{
	if ((int)(gem1.leech*ACC) != (int)(gem2.leech*ACC))
	return gem1.leech<gem2.leech;
	return gem1.bbound<gem2.bbound;
}

void gem_sort_old(gem* gems, int len)
{
	if (len<=1) return;
	int pivot=0;
	int i;
	for (i=1;i<len;++i) {
		if (gem_less_equal(gems[i],gems[pivot])) {
			gem temp=gems[pivot];
			gems[pivot]=gems[i];
			gems[i]=gems[pivot+1];
			gems[pivot+1]=temp;
			pivot++;
		}
	}
	gem_sort_old(gems,pivot);
	gem_sort_old(gems+1+pivot,len-pivot-1);
}

void gem_sort (gem* gems, int len) {
	if (len < 10) {		// ins sort
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
	else {					// quick sort
		gem pivot = gems[len/2];
		gem* beg = gems;
		gem* end = gems+len-1;
		while (beg <= end) {
			if (gem_less_equal(*beg, pivot)) {
				beg++;
			}
			else if (gem_less_equal(pivot,*end)) {
				end--;
			}
			else {
				gem temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		gem_sort(gems, end-gems+1);
		gem_sort(beg, gems-beg+len);
	}
}

float gem_power(gem gem1)
{
	return gem1.leech*gem1.bbound;     // amp-less
}

char gem_color(gem* p_gem)
{
	if (p_gem->leech==0) return 'b';
	if (p_gem->bbound==0) return 'o';
	else return 'm';
}

#include "print_utils.h"


#endif // _MANAGEM_UTILS_H
