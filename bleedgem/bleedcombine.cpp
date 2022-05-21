#include <cstdio>
#include <cstdlib>

#include "bleed_utils.h"
#include "cmdline_options.h"
#include "1D_utils.h"
#include "workers.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();

	if(!options.parse_args(argc, argv))
		return 1;

	worker_combine<gem_R>(options, fill_pool_1D<gem_R>);
	return 0;
}
