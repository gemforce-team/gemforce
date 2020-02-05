#ifndef _KILLGEM_UTILS_H
#define _KILLGEM_UTILS_H

#include <algorithm>

int ACC;					// 80,60  ACC is for z-axis sorting and for the length of the interval tree

struct gem_YB {
	int grade;
	float damage;           // this is MAX damage, with the rand() part neglected
	float crit;             // assumptions: crit chance capped
	float bbound;           // BB hit lv >> 1
	gem_YB* father;         // maximize damage*bbound*crit*bbound
	gem_YB* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

template<class gemYB>
inline double gem_power(gemYB gem1)
{
	return gem1.damage*gem1.bbound*gem1.crit*gem1.bbound;
}

template<class gemYB>
void gem_print(gemYB* p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nBbound:\t%f\nPower:\t%f\n\n", 
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->bbound, gem_power(*p_gem));
}

template<class gemYB>
inline char gem_color(gemYB* p_gem)
{
	if (p_gem->crit==0 && p_gem->bbound==0) return COLOR_CHHIT;
	if (p_gem->crit==0) return COLOR_BBOUND;
	if (p_gem->bbound==0) return COLOR_CRIT;
	else return COLOR_KILLGEM;
}

// -----------------
// Combining section
// -----------------

template<class gemYB>
void gem_comb_eq(gemYB* p_gem1, gemYB* p_gem2, gemYB* p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_EQ_1*p_gem1->damage + DAMAGE_EQ_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_EQ_1*p_gem2->damage + DAMAGE_EQ_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_EQ_1*p_gem1->crit + CRIT_EQ_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_EQ_1*p_gem2->crit + CRIT_EQ_2*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_EQ_1*p_gem1->bbound + BBOUND_EQ_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_EQ_1*p_gem2->bbound + BBOUND_EQ_2*p_gem1->bbound;
}

template<class gemYB>
void gem_comb_d1(gemYB *p_gem1, gemYB *p_gem2, gemYB *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_D1_1*p_gem1->damage + DAMAGE_D1_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_D1_1*p_gem2->damage + DAMAGE_D1_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_D1_1*p_gem1->crit + CRIT_D1_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_D1_1*p_gem2->crit + CRIT_D1_2*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_D1_1*p_gem1->bbound + BBOUND_D1_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_D1_1*p_gem2->bbound + BBOUND_D1_2*p_gem1->bbound;
}

template<class gemYB>
void gem_comb_gn(gemYB *p_gem1, gemYB *p_gem2, gemYB *p_gem_combined)
{
	p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_GN_1*p_gem1->damage + DAMAGE_GN_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_GN_1*p_gem2->damage + DAMAGE_GN_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_GN_1*p_gem1->crit + CRIT_GN_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_GN_1*p_gem2->crit + CRIT_GN_2*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_GN_1*p_gem1->bbound + BBOUND_GN_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_GN_1*p_gem2->bbound + BBOUND_GN_2*p_gem1->bbound;
}

template<class gemYB>
void gem_combine (gemYB *p_gem1, gemYB *p_gem2, gemYB *p_gem_combined)
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

template<class gemYB>
inline void gem_init(gemYB* p_gem, int grd, double damage, double crit, double bbound)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit  =crit;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// ---------------
// Sorting section
// ---------------

template<class gemYB>
inline bool gem_less(gemYB gem1, gemYB gem2)
{
	if ((int)(gem1.damage*ACC) != (int)(gem2.damage*ACC))
		return gem1.damage<gem2.damage;
	if ((int)(gem1.bbound*ACC) != (int)(gem2.bbound*ACC))
		return gem1.bbound<gem2.bbound;
	return gem1.crit<gem2.crit;
}

template<class gemYB>
inline bool gem_less_exact(gemYB gem1, gemYB gem2)
{
	if (gem1.damage != gem2.damage)
		return gem1.damage<gem2.damage;
	if (gem1.bbound != gem2.bbound)
		return gem1.bbound<gem2.bbound;
	return gem1.crit<gem2.crit;
}

template<class gemYB>
inline bool gem_less_crit(gemYB gem1, gemYB gem2)
{
	return gem1.crit<gem2.crit;
}

// -------------------
// Chain adder section
// -------------------

template<class gemYB>
inline double gem_cfr_power(gemYB gem1, double amp_damage_scaled, double amp_crit_scaled)
{
	if (gem1.crit==0) return 0;
	return (gem1.damage+amp_damage_scaled)*gem1.bbound*(gem1.crit+amp_crit_scaled)*gem1.bbound;
}

#include "chain_adder.h"

template<class gemYB>
gemYB* gem_putchain(gemYB* pool, int pool_length, gemYB** gem_array, double amp_damage_scaled, double amp_crit_scaled)
{
	return gem_putchain_templ(pool, pool_length, gem_array,
							  [](gemYB* arg) {gem_init(arg, 1, DAMAGE_CHHIT, 0, 0);},
							  [=](gemYB arg) {return gem_cfr_power(arg, amp_damage_scaled, amp_crit_scaled);});
}

#endif // _KILLGEM_UTILS_H
