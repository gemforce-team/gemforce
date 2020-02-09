#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "managem_utils.h"
#include "build_utils_1D.h"
#include "gfon.h"
#include "print_utils.h"
#include "cmdline_options.h"

using gem = gem_OB;

void worker(const cmdline_options& options)
{
	FILE* table=file_check(options.tables[0]);      // file is open to read
	if (table==NULL) exit(1);              // if the file is not good we exit

	int len = options.target.len;
	int pool_zero = options.target.pool_zero;
	gem* gems = (gem*)malloc(len * sizeof(gem)); // if not malloc-ed 230k is the limit
	gem** pool = (gem**)malloc(len * sizeof(gem*)); // if not malloc-ed 200k is the limit (win)
	int* pool_length = (int*)malloc(len * sizeof(int));
	pool[0] = (gem*)malloc(pool_zero * sizeof(gem));
	pool_length[0] = pool_zero;
	
	if (pool_zero==1) {                    // combine
		gem_init(pool[0],1,1,1);
		gem_init(gems   ,1,1,1);
	}
	else {                                 // spec
		gem_init(pool[0]  ,1,1,0);
		gem_init(pool[0]+1,1,0,1);
		gem_init(gems     ,1,1,0);
	}
	
	int prevmax=pool_from_table(pool, pool_length, len, table);    // pool filling
	fclose(table);				// close
	if (prevmax<len-1) {
		for (int i =0;i<=prevmax;++i) free(pool[i]);      // free
		free(pool);				// free
		free(pool_length);	// free
		free(gems);				// free
		if (prevmax>0) printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i =first; i<len; ++i) {
		compression_1D(gems + i, pool[i], pool_length[i]);
		
		if (!options.output.quiet) {
			printf("Value:\t%d\n",i+1);
			if (options.output.info)
				printf("Growth:\t%f\n", log(gem_power(gems[i]))/log(i+1));
			if (options.output.debug)
				printf("Pool:\t%d\n",pool_length[i]);
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
			gemf = gem_putchain(pool[value-1], pool_length[value-1], &gem_array, 0);
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
	
	for (int i =0;i<len;++i) free(pool[i]);     // free
	free(pool);    // free
	free(pool_length);
	free(gems);
	if (options.target.chain && len > 2 && pool_zero==2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();
	options.set_num_tables(1);

	options.target.pool_zero = 2;

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.target.pool_zero == 2)
		options.table_selection(0, "table_mgspec");
	else
		options.table_selection(0, "table_mgcomb");

	worker(options);
	return 0;
}

