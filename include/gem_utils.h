#ifndef _GEM_UTILS_H
#define _GEM_UTILS_H

#include <algorithm>

template<class gem>
int gem_getvalue(gem* p_gem)
{
	if(p_gem->father==NULL) return 1;
	else return gem_getvalue(p_gem->father)+gem_getvalue(p_gem->mother);
}

template<class gem>
int gem_getdepth(gem* p_gem)
{
	if (p_gem->father==NULL)
		return 1;
	return std::max(gem_getdepth(p_gem->father), gem_getdepth(p_gem->mother)) + 1;
}

template<class gem>
inline bool gem_more_powerful(gem gem1, gem gem2)
{
	return gem_power(gem1) > gem_power(gem2);
}

#endif // _GEM_UTILS_H
