#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "crit_utils.h"
#include "cmdline_options.h"
#include "workers.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();
	options.set_num_tables(1);

	if(!options.parse_args(argc, argv))
		return 1;
	options.table_selection(0, "table_crit");

	worker_query<gem_Y>(options);
}
