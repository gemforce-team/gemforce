#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "killgem_utils.h"
#include "build_utils_3D.h"
#include "gfon.h"
#include "options_utils.h"

using gem = gem_YB;

void worker(int len, options output_options, int pool_zero, char* filename)
{
	FILE* table=table_init(filename, pool_zero);		// init killgem
	int i;
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
		for (i=0;i<len;++i) free(pool[i]);		// free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(filename,"a", table);		// append -> updating possible

	for (i=prevmax+1; i<len; ++i) {
		int comb_tot;
		if (pool_zero == 1)
			comb_tot = fill_pool_3D<SIZES[1], ACCS[1], ACC_TR>(pool, pool_length, i);
		else
			comb_tot = fill_pool_3D<SIZES[2], ACCS[2], ACC_TR>(pool, pool_length, i);

		if (!output_options.quiet) {
			printf("Value:\t%d\n",i+1);
			if (output_options.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);			// write on file
	}
	
	fclose(table);			// close
	for (i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;			// speccing by default
	options output_options = {};
	char filename[256]="";		// it should be enough

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
		if (pool_zero==2) strcpy(filename, "table_kgsappr");
		else strcpy(filename, "table_kgcappr");
	}
	worker(len, output_options, pool_zero, filename);
	return 0;
}

