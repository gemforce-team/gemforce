#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "killgem_utils.h"
#include "cmdline_options.h"
#include "workers.h"

using gem = gem_YB;

constexpr int NCHECKS=6;

double gem_rk311(const gem& gem1)
{
	return gem1.bbound*gem1.bbound*gem1.bbound*gem1.crit*gem1.damage;
}

double gem_rk411(const gem& gem1)
{
	return gem1.bbound*gem1.bbound*gem1.bbound*gem1.bbound*gem1.crit*gem1.damage;
}

double gem_rk511(const gem& gem1)
{
	return gem1.bbound*gem1.bbound*gem1.bbound*gem1.bbound*gem1.bbound*gem1.crit*gem1.damage;
}

inline void cascade_checks(const gem& temp, int p0, gem temp_array[], double pow_array[]) {
	if      ( gem_rk511(temp) >= pow_array[p0] ) {			// rk511 check
		pow_array[p0]=gem_rk511(temp);
		temp_array[p0]=temp;
	}
	else if ( gem_power(temp) >= pow_array[p0+1] ) {		// rk211 check
		pow_array[p0+1]=gem_power(temp);
		temp_array[p0+1]=temp;
	}
	else if ( gem_rk411(temp) >= pow_array[p0+2] ) {		// rk411 check
		pow_array[p0+2]=gem_rk411(temp);
		temp_array[p0+2]=temp;
	}
	else if ( gem_rk311(temp) >= pow_array[p0+3] ) {		// rk311 check
		pow_array[p0+3]=gem_rk311(temp);
		temp_array[p0+3]=temp;
	}
	else if ( gem_power(temp) >= pow_array[p0+4] ) {		// rk211 check
		pow_array[p0+4]=gem_power(temp);
		temp_array[p0+4]=temp;
	}
	else if ( gem_power(temp) >= pow_array[p0+5] ) {		// rk211 check
		pow_array[p0+5]=gem_power(temp);
		temp_array[p0+5]=temp;
	}
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		options.tables[0] = "table_kgcomb";
	}

	worker_build_c6<NCHECKS, gem>(options, cascade_checks);
	return 0;
}
