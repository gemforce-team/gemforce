#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "crit_utils.h"
#include "build_utils_2D.h"
#include "gfon.h"
#include "options_utils.h"

using gem = gem_Y;

void worker(int len, options output_options, char* filename)
{
	FILE* table=table_init(filename, 1);		// init crit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(sizeof(gem));
	gem_init(pool[0],1,1,1);
	pool_length[0]=1;

	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax+1==len) {
		fclose(table);
		for (i=0;i<len;++i) free(pool[i]);		// free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(filename,"a", table);		// append -> updating possible

	for (i=prevmax+1; i<len; ++i) {
		int comb_tot = fill_pool_2D<SIZE, 0>(pool, pool_length, i);

		if (!output_options.quiet) {
			printf("Value:\t%d\n",i+1);
			if (output_options.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);         // write on file
	}
	
	fclose(table);
	for (i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	options output_options = {};
	char filename[256]="";     // it should be enough

	while ((opt=getopt(argc,argv,"hdqf:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hdqf:");
				return 0;
			DQ_OPTIONS_BLOCK
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
	if (filename[0]=='\0') strcpy(filename, "table_crit");
	worker(len, output_options, filename);
	return 0;
}
