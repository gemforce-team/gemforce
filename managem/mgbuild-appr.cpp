#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "managem_utils.h"
#include "build_utils_2D.h"
#include "gfon.h"
#include "options_utils.h"

using gem = gem_OB;

void worker(int len, options output_options, int pool_zero, char* filename)
{
	FILE* table=table_init(filename, pool_zero);    // init managem
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;
	
	if (pool_zero==1) {              // combine
		gem_init(pool[0],1,1,1);
	}
	else {                           // spec
		gem_init(pool[0]  ,1,1,0);
		gem_init(pool[0]+1,1,0,1);
	}
	
	int prevmax=pool_from_table(pool, pool_length, len, table);    // pool filling
	if (prevmax+1==len) {
		fclose(table);
		for (int i=0;i<len;++i) free(pool[i]);  // free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(filename,"a", table);    // append -> updating possible

	for (int i=prevmax+1; i<len; ++i) {
		int comb_tot;
		if (pool_zero == 1)
			comb_tot = fill_pool_2D<SIZES[1], ACC>(pool, pool_length, i);
		else
			comb_tot = fill_pool_2D<SIZES[2], ACC>(pool, pool_length, i);

		if (!output_options.quiet) {
			printf("Value:\t%d\n",i+1);
			if (output_options.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);         // write on file
	}
	
	fclose(table);       // close
	for (int i=0;i<len;++i) free(pool[i]);     // free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;        // speccing by default
	options output_options = {};
	char filename[256]="";  // it should be enough

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
	if (filename[0]=='\0') {
		if (pool_zero==2) strcpy(filename, "table_mgsappr");
		else strcpy(filename, "table_mgcappr");
	}
	worker(len, output_options, pool_zero, filename);
	return 0;
}

