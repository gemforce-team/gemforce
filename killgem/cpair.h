#ifndef _CPAIR_H
#define _CPAIR_H

#include "crit_utils.h"
#include "killgem_utils.h"

struct cpair {
	double  power;
	double  rdmg;
	double  rcrit;
	gem_YB* combg;
	gem_Y*  comba;
	int     place;
};

inline double cpair_BgDaCa(cpair cp)
{
	return cp.power*cp.rdmg*cp.rcrit;
}

inline double cpair_BgDgCa(cpair cp)
{
	return cp.power*cp.rcrit;
}

inline double cpair_BgDaCg(cpair cp)
{
	return cp.power*cp.rdmg;
}

inline bool cpair_less_xyz(cpair cpair1, cpair cpair2)
{
	if (cpair1.rdmg != cpair2.rdmg)
		return cpair1.rdmg<cpair2.rdmg;
	if (cpair1.power != cpair2.power)
		return cpair1.power<cpair2.power;
	return cpair1.rcrit<cpair2.rcrit;
}

inline bool cpair_less_rcrit(cpair cpair1, cpair cpair2)
{
	return cpair1.rcrit<cpair2.rcrit;
}


#endif // _CPAIR_H
