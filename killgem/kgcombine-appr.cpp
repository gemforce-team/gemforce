#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "killgem_utils.h"
#include "3D_utils.h"
#include "cmdline_options.h"
#include "workers.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();
	options.has_pool_zero(1);

	if(!options.parse_args(argc, argv))
		return 1;

	auto fill_pool = fill_pool_3D_selector<SIZES, ACCS, ACC_TR, gem_YB>(options.target.pool_zero);
	worker_combine<gem_YB>(options, fill_pool);
	return 0;
}
