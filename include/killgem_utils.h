#ifndef _KILLGEM_UTILS_H
#define _KILLGEM_UTILS_H

struct Gem_YB {
	int grade;              // short does NOT help
	float damage;           // this is MAX damage, with the rand() part neglected
	float crit;             // assumptions: crit chance capped
	float bbound;           // BB hit lv >> 1
	struct Gem_YB* father;  // maximize damage*bbound*crit*bbound
	struct Gem_YB* mother;
};

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

double gem_power(gem gem1)
{
	return gem1.damage*gem1.bbound*gem1.crit*gem1.bbound;			// amp-less
}

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem_power(gem1) > gem_power(gem2));
}

void gem_print(gem* p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nBbound:\t%f\nPower:\t%f\n\n", 
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->bbound, gem_power(*p_gem));
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
	else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.78*p_gem1->bbound + 0.31*p_gem2->bbound;
	else p_gem_combined->bbound = 0.78*p_gem2->bbound + 0.31*p_gem1->bbound;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.86*p_gem1->damage + 0.7*p_gem2->damage;
	else p_gem_combined->damage = 0.86*p_gem2->damage + 0.7*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.79*p_gem1->bbound + 0.29*p_gem2->bbound;
	else p_gem_combined->bbound = 0.79*p_gem2->bbound + 0.29*p_gem1->bbound;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.85*p_gem1->damage + 0.69*p_gem2->damage;
	else p_gem_combined->damage = 0.85*p_gem2->damage + 0.69*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
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
	if (p_gem_combined->damage < p_gem1->damage) p_gem_combined->damage = p_gem1->damage;
	if (p_gem_combined->damage < p_gem2->damage) p_gem_combined->damage = p_gem2->damage;
}

void gem_init(gem *p_gem, int grd, double damage, double crit, double bbound)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit  =crit;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_less_equal(gem gem1, gem gem2)
{
	if ((int)(gem1.damage*ACC) != (int)(gem2.damage*ACC))
		return gem1.damage<gem2.damage;
	if ((int)(gem1.bbound*ACC) != (int)(gem2.bbound*ACC))
		return gem1.bbound<gem2.bbound;
	return gem1.crit<gem2.crit;
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
	if (len > 10)  {
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

char gem_color(gem* p_gem)
{
	if (p_gem->crit==0 && p_gem->bbound==0) return 'r';
	if (p_gem->crit==0) return 'b';
	if (p_gem->bbound==0) return 'y';
	else return 'k';
}

gem* gem_explore(gem* gemf, int* isRed, gem* pred, int last, int* curr, gem** tobe_freed, int* tbf_index)
{		// more magic
	if (gemf->father==NULL || *isRed) return gemf;
	if (gemf->father->father==NULL) {		// father is g1
		if (*curr < last) (*curr)++;
		else {
			gem* gemt=malloc(sizeof(gem));
			tobe_freed[(*tbf_index)++]=gemt;
			gem_combine(pred, gemf->mother, gemt);
			*isRed=1;
			return gemt;
		}
	}
	if (gemf->mother->father==NULL) {		// mother is g1
		if (*curr < last) (*curr)++;
		else {
			gem* gemt=malloc(sizeof(gem));
			tobe_freed[(*tbf_index)++]=gemt;
			gem_combine(gemf->father, pred, gemt);
			*isRed=1;
			return gemt;
		}
	}
	int wasRed;
	wasRed=(*isRed);
	gem* g1= gem_explore(gemf->father, isRed, pred, last, curr, tobe_freed, tbf_index);
	if (wasRed==(*isRed)) g1=gemf->father;
	wasRed=(*isRed);
	gem* g2= gem_explore(gemf->mother, isRed, pred, last, curr, tobe_freed, tbf_index);
	if (wasRed==(*isRed)) g2=gemf->mother;
	if (g1==gemf->father && g2==gemf->mother) return gemf;
	
	gem* gemt=malloc(sizeof(gem));
	tobe_freed[(*tbf_index)++]=gemt;
	gem_combine(g1, g2, gemt);
	return gemt;
}

void array_free(gem** tobe_freed, int tbf_index)
{
	int i;
	for (i=0; i<tbf_index; ++i) free(tobe_freed[i]);
}

gem gem_putred(gem* gemf, int len, gem*** gem_array, int* array_index)
{		// magic
	int isRed;
	int last;
	int curr;
	double best_pow=0;
	gem* red=malloc(sizeof(gem));
	gem_init(red,1,0.909091,0,0);
	gem* best_gem=NULL;
	gem** tobe_freed=malloc(2*len*sizeof(gem));
	gem** btb_freed=malloc(2*len*sizeof(gem));
	int tbf_index;
	int btbf_index=0;
	for (last=0; last<len; last++) {
		isRed=0;
		curr=0;
		tbf_index=0;
		gem* gp=gem_explore(gemf, &isRed, red, last, &curr, tobe_freed, &tbf_index);
		if (gem_power(*gp) > best_pow) {
			best_pow=gem_power(*gp);
			if (best_gem!=NULL) array_free(btb_freed, btbf_index);
			best_gem=gp;
			btbf_index=tbf_index;
			int i;
			for (i=0; i< tbf_index; ++i) btb_freed[i]=tobe_freed[i];
		}
		else array_free(tobe_freed, tbf_index);
	}
	free(tobe_freed);
	gem target=*best_gem;
	btb_freed[btbf_index]=red;
	(*gem_array)=btb_freed;
	(*array_index)=btbf_index+1;
	return target;
}

#include "print_utils.h"


#endif // _KILLGEM_UTILS_H
