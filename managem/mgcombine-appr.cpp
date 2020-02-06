#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>

#include "managem_utils.h"
#include "build_utils_2D.h"
#include "print_utils.h"
#include "options_utils.h"

using gem = gem_OB;

void worker(int len, options output_options, int pool_zero)
{
	printf("\n");
	int i;
	int size;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;
	
	if (pool_zero==1) {             // combine
		gem_init(pool[0],1,1,1);
		gem_init(gems   ,1,1,1);
		size=100;                    // reasonable comb sizing
	}
	else {                          // spec
		gem_init(pool[0]  ,1,1,0);
		gem_init(pool[0]+1,1,0,1);
		gem_init(gems     ,1,1,0);
		size=2000;                   // reasonable spec sizing
	}
	if (!output_options.quiet) gem_print(gems);

	for (i=1; i<len; ++i) {
		int comb_tot = fill_pool_2D<ACC>(pool, pool_length, i, size);

		gems[i]=pool[i][0];                 // choosing gem (criteria moved to more_power def)
		for (int j = 1; j < pool_length[i]; ++j) {
			if (gem_more_powerful(pool[i][j], gems[i])) {
				gems[i] = pool[i][j];
			}
		}
		
		if (!output_options.quiet) {
			printf("Value:\t%d\n",i+1);
			if (output_options.info)
				printf("Growth:\t%f\n", log(gem_power(gems[i]))/log(i+1));
			if (output_options.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n",pool_length[i]);
			}
			gem_print(gems+i);
		}
	}
	
	if (output_options.quiet) {     // outputs last if we never seen any
		printf("Value:\t%d\n",len);
		printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(len));
		if (output_options.debug)
			printf("Pool:\t%d\n",pool_length[len-1]);
		gem_print(gems+len-1);
	}

	gem* gemf=gems+len-1;  // gem that will be displayed

	if (output_options.upto) {
		double best_growth=-INFINITY;
		int best_index=0;
		for (i=0; i<len; ++i) {
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
	if (output_options.chain) {
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

	if (output_options.parens) {
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
	}
	if (output_options.tree) {
		printf("Gem tree:\n");
		print_tree(gemf, "");
		printf("\n");
	}
	if (output_options.table) print_table(gems, len);
	
	if (output_options.equations) {   // it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gemf);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);     // free
	if (output_options.chain && len > 2 && pool_zero==2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;        // speccing by default
	options output_options = {};
	
	while ((opt=getopt(argc,argv,"hptecidqur"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecidqur");
				return 0;
			PTECIDQUR_OPTIONS_BLOCK
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind==argc) {
		printf("No length specified\n");
		return 1;
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
		char* p=argv[optind];
		while (*p != '\0') p++;
		if (*(p-1)=='c') pool_zero=1;
	}
	else {
		printf("Too many arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		printf("\n");
		return 1;
	}
	if (len<1) {
		printf("Improper gem number\n");
		return 1;
	}
	worker(len, output_options, pool_zero);
	return 0;
}

