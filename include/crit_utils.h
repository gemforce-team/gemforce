#ifndef _CRIT_UTILS_H
#define _CRIT_UTILS_H

#include <algorithm>

constexpr unsigned int SIZE = 1000;

struct gem_Y {
	int grade;				//using short does NOT improve time/memory usage
	float damage;
	float crit;
	gem_Y* father;
	gem_Y* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

inline double gem_power(const gem_Y& gem1)
{
	return gem1.damage*gem1.crit;
}

inline void gem_print(const gem_Y *p_gem) {
	printf("Grade:\t%d\n"
		   "Damage:\t%f\n"
		   "Crit:\t%f\n"
		   "Power:\t%f\n\n",
		   p_gem->grade, p_gem->damage, p_gem->crit, p_gem->damage * p_gem->crit);
}

inline char gem_color(const gem_Y* p_gem)
{
	if (p_gem->crit==0) return COLOR_CHHIT;
	else return COLOR_CRIT;
}

// ----------------
// 2D gem interface
// ---------------.

inline auto get_first(const gem_Y& gem)
{
	return gem.damage;
}

inline auto get_second(const gem_Y& gem)
{
	return gem.crit;
}

// -----------------
// Combining section
// -----------------

inline void gem_comb_eq(const gem_Y *p_gem1, const gem_Y *p_gem2, gem_Y *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_EQ_1*p_gem1->damage + DAMAGE_EQ_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_EQ_1*p_gem2->damage + DAMAGE_EQ_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_EQ_1*p_gem1->crit + CRIT_EQ_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_EQ_1*p_gem2->crit + CRIT_EQ_2*p_gem1->crit;
}

inline void gem_comb_d1(const gem_Y *p_gem1, const gem_Y *p_gem2, gem_Y *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_D1_1*p_gem1->damage + DAMAGE_D1_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_D1_1*p_gem2->damage + DAMAGE_D1_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_D1_1*p_gem1->crit + CRIT_D1_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_D1_1*p_gem2->crit + CRIT_D1_2*p_gem1->crit;
}

inline void gem_comb_gn(const gem_Y *p_gem1, const gem_Y *p_gem2, gem_Y *p_gem_combined)
{
	p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_GN_1*p_gem1->damage + DAMAGE_GN_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_GN_1*p_gem2->damage + DAMAGE_GN_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_GN_1*p_gem1->crit + CRIT_GN_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_GN_1*p_gem2->crit + CRIT_GN_2*p_gem1->crit;
}

inline void gem_combine(gem_Y *p_gem1, gem_Y *p_gem2, gem_Y *p_gem_combined)
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

inline void gem_init(gem_Y *p_gem, int grd, float damage, float crit)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit  =crit;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// -------------------
// Chain adder section
// -------------------

inline double gem_cfr_power(const gem_Y& gem1, double amp_damage_scaled, double amp_crit_scaled)
{
	return (gem1.damage + amp_damage_scaled) * (gem1.crit + amp_crit_scaled);
}

#include "chain_adder.h"

gem_Y* gem_putchain(const gem_Y* pool, int pool_length, gem_Y** gem_array)
{
	return gem_putchain_templ(pool, pool_length, gem_array,
							  [](gem_Y* arg) {gem_init(arg, 1, DAMAGE_CHHIT, 0);},
							  [](gem_Y arg) {return gem_power(arg);});
}

gem_Y* gem_putchain(const gem_Y* pool, int pool_length, gem_Y** gem_array, double amp_damage_scaled, double amp_crit_scaled)
{
	return gem_putchain_templ(pool, pool_length, gem_array,
							  [](gem_Y* arg) {gem_init(arg, 1, DAMAGE_CHHIT, 0);},
							  [=](gem_Y arg) {return gem_cfr_power(arg, amp_damage_scaled, amp_crit_scaled);});
}


#endif // _CRIT_UTILS_H
