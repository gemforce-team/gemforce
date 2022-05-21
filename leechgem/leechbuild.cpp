#include <cstdio>
#include <cstdlib>

#include "leech_utils.h"
#include "cmdline_options.h"
#include "1D_utils.h"
#include "workers.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		options.tables[0] = "table_leech";
	}

	worker_build<gem_O>(options, fill_pool_1D<gem_O>);
	return 0;
}
