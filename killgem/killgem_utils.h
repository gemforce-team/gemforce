#ifndef _KILLGEM_UTILS_H
#define _KILLGEM_UTILS_H

#include <algorithm>

// arrays indexed by pool_zero (1 = combine, 2 = spec)
constexpr int ACCS[] = {0, 80, 60};	// ACC is for z-axis sorting and for the length of the interval tree
constexpr int ACCS_EXACT[] = {0, 0, 0};
constexpr int ACC_TR = 750;			// ACC_TR is for bbound comparisons inside tree
constexpr size_t SIZES[] = {0, 1000, 20000};

struct gem_YB {
	gem_YB* father; 
	gem_YB* mother;
	int grade;
	float damage;           // this is MAX damage, with the rand() part neglected
	float crit;             // assumptions: crit chance capped
	float bbound;           // BB hit lv >> 1
};                          // maximize damage*bbound*crit*bbound

// --------------------
// Common gem interface
// --------------------

#include "gem_utils.h"
#include "gem_stats.h"

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline double gem_power(const gemYB& gem1)
{
	return gem1.damage*gem1.bbound*gem1.crit*gem1.bbound;
}

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline void gem_print(const gemYB* p_gem) {
	printf("Grade:\t%d\n"
	       "Damage:\t%f\n"
		   "Crit:\t%f\n"
		   "Bbound:\t%f\n"
		   "Power:\t%f\n\n", 
		   p_gem->grade, p_gem->damage, p_gem->crit, p_gem->bbound, gem_power(*p_gem));
}

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline char gem_color(const gemYB* p_gem)
{
	if (p_gem->crit==0 && p_gem->bbound==0) return COLOR_CHHIT;
	if (p_gem->crit==0) return COLOR_BBOUND;
	if (p_gem->bbound==0) return COLOR_CRIT;
	else return COLOR_KILLGEM;
}

// ----------------
// 3D gem interface
// ----------------

template<class gemYB>
inline auto get_first(const gemYB& gem)
{
	return gem.damage;
}

template<class gemYB>
inline auto get_second(const gemYB& gem)
{
	return gem.crit;
}

template<class gemYB>
inline auto get_third(const gemYB& gem)
{
	return gem.bbound;
}

// ----------------
// Place interface
// ----------------

inline int get_place(const gem_YB&)
{
	return 0;
}

inline void set_place(const gem_YB&, int) {}

// -----------------
// Combining section
// -----------------

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline void gem_comb_eq(const gemYB* p_gem1, const gemYB* p_gem2, gemYB* p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_EQ_1*p_gem1->damage + DAMAGE_EQ_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_EQ_1*p_gem2->damage + DAMAGE_EQ_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_EQ_1*p_gem1->crit + CRIT_EQ_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_EQ_1*p_gem2->crit + CRIT_EQ_2*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_EQ_1*p_gem1->bbound + BBOUND_EQ_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_EQ_1*p_gem2->bbound + BBOUND_EQ_2*p_gem1->bbound;
}

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline void gem_comb_d1(const gemYB *p_gem1, const gemYB *p_gem2, gemYB *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_D1_1*p_gem1->damage + DAMAGE_D1_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_D1_1*p_gem2->damage + DAMAGE_D1_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_D1_1*p_gem1->crit + CRIT_D1_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_D1_1*p_gem2->crit + CRIT_D1_2*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_D1_1*p_gem1->bbound + BBOUND_D1_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_D1_1*p_gem2->bbound + BBOUND_D1_2*p_gem1->bbound;
}

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline void gem_comb_gn(const gemYB *p_gem1, const gemYB *p_gem2, gemYB *p_gem_combined)
{
	p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_GN_1*p_gem1->damage + DAMAGE_GN_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_GN_1*p_gem2->damage + DAMAGE_GN_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_GN_1*p_gem1->crit + CRIT_GN_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_GN_1*p_gem2->crit + CRIT_GN_2*p_gem1->crit;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = BBOUND_GN_1*p_gem1->bbound + BBOUND_GN_2*p_gem2->bbound;
	else p_gem_combined->bbound = BBOUND_GN_1*p_gem2->bbound + BBOUND_GN_2*p_gem1->bbound;
}

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline void gem_combine (gemYB *p_gem1, gemYB *p_gem2, gemYB *p_gem_combined)
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

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline void gem_init(gemYB* p_gem, int grd, double damage, double crit, double bbound)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit  =crit;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// -----------------
// Pool init section
// -----------------

#include "0D_utils.h"

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline vector<pool_t<gemYB>> init_pool(int len, uint pool_zero) {
	vector pool = vector<pool_t<gemYB>>(len);
	pool[0] = make_uninitialized_pool<gemYB>(pool_zero);
	if (pool_zero==1) {									// combine
		gem_init(pool[0]+0, 1, 1, 1, 1);				// grade damage crit bbound
	}
	else {												// spec
		gem_init(pool[0]+0, 1, DAMAGE_CRIT  , 1, 0);	// grade damage crit bbound
		gem_init(pool[0]+1, 1, DAMAGE_BBOUND, 0, 1);
	}
	return pool;
}

// -------------------
// Chain adder section
// -------------------

template<class gemYB> requires requires (gemYB g) { g.bbound; }
inline double gem_cfr_power(const gemYB& gem1, double amp_damage_scaled, double amp_crit_scaled)
{
	if (gem1.crit==0) return 0;
	return (gem1.damage+amp_damage_scaled)*gem1.bbound*(gem1.crit+amp_crit_scaled)*gem1.bbound;
}

#include "chain_adder.h"

template<class gemYB> requires requires (gemYB g) { g.bbound; }
gemYB* gem_putchain(const pool_t<gemYB>& pool, size_t pool_length, vector<gemYB>& chain_gems,
                    double amp_damage_scaled = 0, double amp_crit_scaled = 0)
{
	return gem_putchain_templ(pool, pool_length, chain_gems,
							  [](gemYB* arg) {gem_init(arg, 1, DAMAGE_CHHIT, 0, 0);},
							  [=](const gemYB& arg) {return gem_cfr_power(arg, amp_damage_scaled, amp_crit_scaled);});
}

#endif // _KILLGEM_UTILS_H
