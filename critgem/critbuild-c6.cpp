#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "crit_utils.h"
#include "cmdline_options.h"
#include "workers.h"

using gem = gem_Y;

constexpr int NCHECKS=6;

inline double gem_rk23(const gem& gem1)
{
	return gem1.damage*gem1.damage*gem1.crit*gem1.crit*gem1.crit;
}

inline double gem_rk32(const gem& gem1)
{
	return gem1.damage*gem1.damage*gem1.damage*gem1.crit*gem1.crit;
}

inline double gem_rk12(const gem& gem1)
{
	return gem1.damage*gem1.crit*gem1.crit;
}

inline double gem_rk21(const gem& gem1)
{
	return gem1.damage*gem1.damage*gem1.crit;
}

inline void cascade_checks(const gem& temp, int p0, gem temp_array[], double pow_array[]) {
	if      ( gem_power(temp) > pow_array[p0] ) {			// rk11 check
		pow_array[p0]=gem_power(temp);
		temp_array[p0]=temp;
	}
	else if ( gem_rk23(temp) > pow_array[p0+1] ) {			// rk23 check
		pow_array[p0+1]=gem_rk23(temp);
		temp_array[p0+1]=temp;
	}
	else if ( gem_power(temp) > pow_array[p0+2] ) {			// rk11 check
		pow_array[p0+2]=gem_power(temp);
		temp_array[p0+2]=temp;
	}
	else if ( gem_rk32(temp) > pow_array[p0+3] ) {			// rk32 check
		pow_array[p0+3]=gem_rk32(temp);
		temp_array[p0+3]=temp;
	}
	else if ( gem_rk23(temp) > pow_array[p0+4] ) {			// rk23 check
		pow_array[p0+4]=gem_rk23(temp);
		temp_array[p0+4]=temp;
	}
	else if ( gem_power(temp) > pow_array[p0+5] ) {			// rk11 check
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
		options.tables[0] = "table_critcomb";
	}

	worker_build_c6<NCHECKS, gem>(options, cascade_checks);
	return 0;
}
