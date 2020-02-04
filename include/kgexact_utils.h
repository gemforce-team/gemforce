#ifndef _KGEXACT_UTILS_H
#define _KGEXACT_UTILS_H

struct gem_YBp {
	int grade;
	float damage;           // this is MAX damage, with the rand() part neglected
	float crit;             // assumptions: crit chance capped
	float bbound;           // BB hit lv >> 1
	gem_YBp* father;        // maximize damage*bbound*crit*bbound
	gem_YBp* mother;
	int place;
};

#include "killgem_utils.h"

#endif // _KGEXACT_UTILS_H
