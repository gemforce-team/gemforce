#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "crit_utils.h"
#include "2D_utils.h"
#include "cmdline_options.h"
#include "workers.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		options.tables[0] = "table_crit";
	}

	worker_build<gem_Y>(options, fill_pool_2D<SIZE, 0, gem_Y>);
	return 0;
}
