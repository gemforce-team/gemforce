#ifndef _0D_UTILS_H
#define _0D_UTILS_H

#include <cstdlib>

#include "container_utils.h"

// -----------------
// Pool init section
// -----------------

template<class gem>
inline vector<pool_t<gem>> init_pool(int len, uint pool_zero = 1);

inline vector<size_t> init_pool_length(int len, uint pool_zero = 1) {
	vector pool_length = vector<size_t>(len);
	pool_length[0] = pool_zero;
	return pool_length;
}

template<class gem>
inline vector<gem> init_gems(const vector<pool_t<gem>>& pool) {
	vector gems = vector<gem>(pool.size());
	gems[0] = pool[0][0];
	return gems;
}

#endif // _0D_UTILS_H
