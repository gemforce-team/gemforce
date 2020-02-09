#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "killgem_utils.h"
#include "build_utils_3D.h"
#include "gfon.h"
#include "cmdline_options.h"

using gem = gem_YB;

void worker(const cmdline_options& options)
{
	int pool_zero = options.target.pool_zero;
	FILE* table=table_init(options.tables[0], pool_zero);		// init killgem

	int len = options.target.len;
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;

	if (pool_zero==1) {					// combine
		gem_init(pool[0],1,1,1,1);		// start gem does not matter
	}
	else {									// spec
		gem_init(pool[0]  ,1,DAMAGE_CRIT  ,1,0);	// grade damage crit bbound
		gem_init(pool[0]+1,1,DAMAGE_BBOUND,0,1);	// BB has more dmg
	}
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax+1==len) {
		fclose(table);
		for (int i=0;i<len;++i) free(pool[i]);		// free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(options.tables[0].c_str(),"a", table);		// append -> updating possible

	for (int i=prevmax+1; i<len; ++i) {
		int comb_tot;
		if (pool_zero == 1)
			comb_tot = fill_pool_3D<SIZES[1], ACCS[1], ACC_TR>(pool, pool_length, i);
		else
			comb_tot = fill_pool_3D<SIZES[2], ACCS[2], ACC_TR>(pool, pool_length, i);

		if (!options.output.quiet) {
			printf("Value:\t%d\n",i+1);
			if (options.output.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);			// write on file
	}
	
	fclose(table);			// close
	for (int i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);

	options.target.pool_zero = 2;

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		if (options.target.pool_zero == 2)
			options.tables[0] = "table_kgsappr";
		else
			options.tables[0] = "table_kgcappr";
	}

	worker(options);
	return 0;
}

