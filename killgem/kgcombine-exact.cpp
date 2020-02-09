#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>

#include "kgexact_utils.h"
#include "build_utils_1D.h"
#include "build_utils_3D.h"
#include "print_utils.h"
#include "cmdline_options.h"

using gem = gem_YBp;

void worker(const cmdline_options& options)
{
	printf("\n");
	int len = options.target.len;
	int pool_zero = options.target.pool_zero;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;

	if (pool_zero==1) {					// combine
		gem_init(pool[0],1,1,1,1);		// start gem does not matter
		gem_init(gems   ,1,1,1,1);		// grade damage crit bbound
	}
	else {									// spec
		gem_init(pool[0]  ,1,DAMAGE_CRIT  ,1,0);
		gem_init(pool[0]+1,1,DAMAGE_BBOUND,0,1);	// BB has more dmg
		gem_init(gems     ,1,DAMAGE_CRIT  ,1,0);	// grade damage crit bbound
	}
	if (!options.output.quiet) gem_print(gems);

	for (int i =1; i<len; ++i) {
		int comb_tot;
		if (pool_zero == 1)
			comb_tot = fill_pool_3D<SIZES[1], 0, 0>(pool, pool_length, i);
		else
			comb_tot = fill_pool_3D<SIZES[2], 0, 0>(pool, pool_length, i);
		
		compression_1D(gems + i, pool[i], pool_length[i]);
		
		if (!options.output.quiet) {
			printf("Value:\t%d\n",i+1);
			if (options.output.info)
				printf("Growth:\t%f\n", log(gem_power(gems[i]))/log(i+1));
			if (options.output.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n",pool_length[i]);
			}
			gem_print(gems+i);
		}
	}
	
	if (options.output.quiet) {     // outputs last if we never seen any
		printf("Value:\t%d\n",len);
		printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(len));
		if (options.output.debug)
			printf("Pool:\t%d\n",pool_length[len-1]);
		gem_print(gems+len-1);
	}

	gem* gemf=gems+len-1;  // gem that will be displayed

	if (options.target.upto) {
		double best_growth=-INFINITY;
		int best_index=0;
		for (int i =0; i<len; ++i) {
			if (log(gem_power(gems[i]))/log(i+1) > best_growth) {
				best_index=i;
				best_growth=log(gem_power(gems[i]))/log(i+1);
			}
		}
		printf("Best gem up to %d:\n\n", len);
		printf("Value:\t%d\n",best_index+1);
		printf("Growth:\t%f\n", best_growth);
		gem_print(gems+best_index);
		gemf = gems+best_index;
	}

	gem* gem_array = NULL;
	if (options.target.chain) {
		if (len < 3 || pool_zero!=2) printf("I could not add chain!\n\n");
		else {
			int value=gem_getvalue(gemf);
			gemf = gem_putchain(pool[value-1], pool_length[value-1], &gem_array, 0, 0);
			printf("Gem with chain added:\n\n");
			printf("Value:\t%d\n", value);    // made to work well with -u
			printf("Growth:\t%f\n", log(gem_power(*gemf))/log(value));
			gem_print(gemf);
		}
	}

	if (options.print.parens) {
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
	}
	if (options.print.tree) {
		printf("Gem tree:\n");
		print_tree(gemf, "");
		printf("\n");
	}
	if (options.print.table) print_table(gems, len);
	
	if (options.print.equations) {   // it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gemf);
		printf("\n");
	}
	
	for (int i =0;i<len;++i) free(pool[i]);		// free
	if (options.target.chain && len > 2 && pool_zero==2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();

	if(!options.parse_args(argc, argv))
		return 1;

	worker(options);
	return 0;
}

