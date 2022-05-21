#ifndef _KGEXACT_UTILS_H
#define _KGEXACT_UTILS_H

struct gem_YBp {
	gem_YBp* father;
	gem_YBp* mother;
	int grade;
	float damage;           // this is MAX damage, with the rand() part neglected
	float crit;             // assumptions: crit chance capped
	float bbound;           // BB hit lv >> 1
	int place;              // maximize damage*bbound*crit*bbound
};

// ----------------
// Place interface
// ----------------

inline int get_place(const gem_YBp& gem)
{
	return gem.place;
}

inline void set_place(gem_YBp& gem, int place)
{
	gem.place = place;
}

#include "killgem_utils.h"

#endif // _KGEXACT_UTILS_H
