#include <cstdio>
#include <cstdlib>

#include "managem_utils.h"
#include "2D_utils.h"
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

	auto fill_pool = fill_pool_2D_selector<SIZES, ACC, gem_OB>(options.target.pool_zero);
	worker_combine<gem_OB>(options, fill_pool);
	return 0;
}
