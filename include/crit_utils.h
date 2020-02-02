#ifndef _CRIT_UTILS_H
#define _CRIT_UTILS_H

struct Gem_Y {
	int grade;				//using short does NOT improve time/memory usage
	float damage;
	float crit;
	struct Gem_Y* father;
	struct Gem_Y* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_stats.h"

void gem_print(gemY *p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->damage*p_gem->crit);
}

char gem_color(gemY*)
{
	return COLOR_CRIT;
}

// -----------------
// Combining section
// -----------------

void gem_comb_eq(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_EQ_1*p_gem1->damage + DAMAGE_EQ_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_EQ_1*p_gem2->damage + DAMAGE_EQ_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_EQ_1*p_gem1->crit + CRIT_EQ_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_EQ_1*p_gem2->crit + CRIT_EQ_2*p_gem1->crit;
}

void gem_comb_d1(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_D1_1*p_gem1->damage + DAMAGE_D1_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_D1_1*p_gem2->damage + DAMAGE_D1_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_D1_1*p_gem1->crit + CRIT_D1_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_D1_1*p_gem2->crit + CRIT_D1_2*p_gem1->crit;
}

void gem_comb_gn(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_GN_1*p_gem1->damage + DAMAGE_GN_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_GN_1*p_gem2->damage + DAMAGE_GN_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_GN_1*p_gem1->crit + CRIT_GN_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_GN_1*p_gem2->crit + CRIT_GN_2*p_gem1->crit;
}

void gem_combine(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
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
	if (p_gem_combined->damage < p_gem1->damage) p_gem_combined->damage = p_gem1->damage;
	if (p_gem_combined->damage < p_gem2->damage) p_gem_combined->damage = p_gem2->damage;
}

void gem_init(gemY *p_gem, int grd, float damage, float crit)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit  =crit;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// ---------------
// Sorting section
// ---------------

inline int gem_has_less_damage_crit(gemY gem1, gemY gem2)
{
	if (gem1.damage < gem2.damage) return 1;
	else if (gem1.damage == gem2.damage && gem1.crit < gem2.crit) return 1;
	else return 0;
}

void ins_sort_Y (gemY* gems, int len)
{
	int i,j;
	gemY element;
	for (i=1; i<len; i++) {
		element=gems[i];
		for (j=i; j>0 && gem_has_less_damage_crit(element, gems[j-1]); j--) {
			gems[j]=gems[j-1];
		}
		gems[j]=element;
	}
}

void quick_sort_Y (gemY* gems, int len)
{
	if (len > 20)  {
		gemY pivot = gems[len/2];
		gemY* beg = gems;
		gemY* end = gems+len-1;
		while (beg <= end) {
			while (gem_has_less_damage_crit(*beg, pivot)) {
				beg++;
			}
			while (gem_has_less_damage_crit(pivot,*end)) {
				end--;
			}
			if (beg <= end) {
				gemY temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		if (end-gems+1 < gems-beg+len) {		// sort smaller first
			quick_sort_Y(gems, end-gems+1);
			quick_sort_Y(beg, gems-beg+len);
		}
		else {
			quick_sort_Y(beg, gems-beg+len);
			quick_sort_Y(gems, end-gems+1);
		}
	}
}

void gem_sort_Y (gemY* gems, int len)
{
	quick_sort_Y (gems, len);		// partially sort
	ins_sort_Y (gems, len);			// finish the nearly sorted array
}

#endif // _CRIT_UTILS_H
