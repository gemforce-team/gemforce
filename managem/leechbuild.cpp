#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "leech_utils.h"
#include "gfon.h"
#include "cmdline_options.h"

using gem = gem_O;

void worker(const cmdline_options& options)
{
	FILE* table=table_init(options.tables[0], 1);    // init leech

	int len = options.target.len;
	gem** pool = (gem**)malloc(len*sizeof(gem*));    // if not malloc-ed 690k is the limit
	int* pool_length = (int*)malloc(len*sizeof(int));
	pool[0] = (gem*)malloc(sizeof(gem));
	gem_init(pool[0],1,1);
	pool_length[0]=1;
	
	int prevmax=pool_from_table(pool, pool_length, len, table);   // pool filling
	if (prevmax+1==len) {
		fclose(table);       // close
		for (int i=0;i<len;++i) free(pool[i]);   // free
		free(pool);          // free
		free(pool_length);   // free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(options.tables[0].c_str(), "a", table);  // append -> updating possible

	for (int i=prevmax+1; i<len; ++i) {      // more building
		int comb_tot = fill_pool_1D(pool, pool_length, i);

		if (!options.output.quiet) {
			printf("Value:\t%d\n",i+1);
			if (options.output.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);    // write on file
	}
	
	fclose(table);      // close
	for (int i=0;i<len;++i) free(pool[i]);   // free
	free(pool);         // free
	free(pool_length);  // free
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		options.tables[0] = "table_leech";
	}

	worker(options);
	return 0;
}

