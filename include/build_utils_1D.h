#ifndef _BUILD_UTILS_1D_H
#define _BUILD_UTILS_1D_H

#include <cstdlib>
#include <cmath>
#include <algorithm>

template<class gem>
inline int fill_pool_1D(gem** pool, int* pool_length, int i)
{
	const int eoc = (i + 1) / (1 + 1);     // end of combining
	const int j0 = (i + 1) / (10 + 1);     // value ratio < 10
	int comb_tot = 0;

	const int grade_max = (int)(log2(i + 1) + 1);  // gems with max grade cannot be destroyed, so this is a max, not a sup
	gem temp_array[grade_max - 1] = {};            // this will have all the grades

	for (int j = j0; j < eoc; ++j) {          // combine gems and put them in temp array
		for (int k = 0; k < pool_length[j]; ++k) {
			int g1 = (pool[j] + k)->grade;
			for (int h = 0; h < pool_length[i - 1 - j]; ++h) {
				int delta = g1 - (pool[i - 1 - j] + h)->grade;
				if (abs(delta) <= 2) {        // grade difference <= 2
					comb_tot++;
					gem temp;
					gem_combine(pool[j] + k, pool[i - 1 - j] + h, &temp);
					int grd = temp.grade - 2;
					if (gem_more_powerful(temp, temp_array[grd])) {
						temp_array[grd] = temp;
					}
				}
			}
		}
	}
	int gemNum = 0;
	for (int j = 0; j < grade_max - 1; ++j)
		if (temp_array[j].grade != 0)
			gemNum++;
	pool_length[i] = gemNum;
	pool[i] = (gem*)malloc(pool_length[i] * sizeof(gem));

	std::copy_if(temp_array, temp_array + grade_max - 1, pool[i], [](gem g) {return g.grade != 0;});

	return comb_tot;
}

#endif // _BUILD_UTILS_1D_H
