#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "crit_utils.h"
#include "2D_utils.h"
#include "cmdline_options.h"
#include "workers.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();

	if(!options.parse_args(argc, argv))
		return 1;

	worker_combine<gem_Y>(options, fill_pool_2D<SIZE, 0, gem_Y>);
	return 0;
}
