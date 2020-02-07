#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "interval_tree.h"
#include "killgem_utils.h"
#include "query_utils.h"
#include "gfon.h"
#include "print_utils.h"
#include "options_utils.h"

using gem = gem_YB;

void worker(int len, options output_options, int pool_zero, char* filename)
{
	FILE* table=file_check(filename);		// file is open to read
	if (table==NULL) exit(1);				// if the file is not good we exit
	gem* gems = (gem*)malloc(len*sizeof(gem));		// if not malloc-ed 230k is the limit
	gem** pool = (gem**)malloc(len*sizeof(gem*));
	int* pool_length = (int*)malloc(len*sizeof(int));
	pool[0] = (gem*)malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;

	if (pool_zero==1) {					// combine
		gem_init(pool[0],1,1,1,1);		// start gem does not matter
		gem_init(gems   ,1,1,1,1);		// grade damage crit bbound
	}
	else {											// spec
		gem_init(pool[0]  ,1,DAMAGE_CRIT  ,1,0);
		gem_init(pool[0]+1,1,DAMAGE_BBOUND,0,1);	// BB has more dmg
		gem_init(gems     ,1,DAMAGE_CRIT  ,1,0);	// grade damage crit bbound
	}
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	fclose(table);				// close
	if (prevmax<len-1) {
		for (int i =0;i<=prevmax;++i) free(pool[i]);      // free
		free(pool);				// free
		free(pool_length);	// free
		free(gems);				// free
		if (prevmax>0) printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	
	bool skip_computations = output_options.quiet && !(output_options.table || output_options.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i =first; i<len; ++i) {
		gems[i]=pool[i][0];
		for (int j=1; j<pool_length[i]; ++j) {
			if (gem_more_powerful(pool[i][j],gems[i])) {
				gems[i]=pool[i][j];
			}
		}
		
		if (!output_options.quiet) {
			printf("Value:\t%d\n",i+1);
			if (output_options.info)
				printf("Growth:\t%f\n", log(gem_power(gems[i]))/log(i+1));
			if (output_options.debug)
				printf("Pool:\t%d\n",pool_length[i]);
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
	if (output_options.chain) {
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
	
	for (int i =0;i<len;++i) free(pool[i]);		// free
	free(pool);
	free(pool_length);
	free(gems);
	if (output_options.chain && len > 2 && pool_zero==2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;     // speccing by default
	options output_options = {};
	char filename[256]="";     // it should be enough

	while ((opt=getopt(argc,argv,"hptecidqurf:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecidqurf:");
				return 0;
			PTECIDQUR_OPTIONS_BLOCK
			case 'f':
				strcpy(filename,optarg);
				break;
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
	if (pool_zero==2) file_selection(filename, "table_kgspec");
	else file_selection(filename, "table_kgcomb");
	worker(len, output_options, pool_zero, filename);
	return 0;
}
