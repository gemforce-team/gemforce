#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "killgem_utils.h"
#include "3D_utils.h"
#include "workers.h"
#include "cmdline_options.h"

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();
	options.set_num_tables(1);
	options.has_pool_zero(2);

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.target.pool_zero == 2)
		options.table_selection(0, "table_kgspec");
	else
		options.table_selection(0, "table_kgcomb");

	worker_query<gem_YB>(options);
	return 0;
}
