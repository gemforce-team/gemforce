#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "killgem_utils.h"
#include "3D_utils.h"
#include "cmdline_options.h"
#include "workers.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);
	options.has_pool_zero(2);

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		if (options.target.pool_zero == 2)
			options.tables[0] = "table_kgsappr";
		else
			options.tables[0] = "table_kgcappr";
	}

	auto fill_pool = fill_pool_3D_selector<SIZES, ACCS, ACC_TR, gem_YB>(options.target.pool_zero);
	worker_build<gem_YB>(options, fill_pool);
	return 0;
}
