#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_YB gem;		// the strange order is so that killgem_utils knows which gem type are we defining as "gem"
const int ACC=0;							// I don't really use it here
#include "killgem_utils.h"
#include "gfon.h"

const int pool_depth=2;
const int nchecks=2;

double gem_ranking(gem gem1)
{
	return gem1.bbound*gem1.bbound*gem1.bbound*gem1.crit*gem1.damage;			// 311 seems to be the optimal proportion for low values
}

void worker(int len, int output_options, char* filename)
{
	FILE* table=table_init(filename, 1);		// init killgem
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(sizeof(gem));
	pool_length[0]=1;
	gem_init(pool[0],1,1,1,1);		// grade damage crit bbound
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax+1==len) {
		fclose(table);
		for (i=0;i<len;++i) free(pool[i]);		// free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(filename,"a", table);		// append -> updating possible

	for (i=prevmax+1; i<len; ++i) {
		int j,k,h,l;
		int eoc=(i+1)/2;        //end of combining
		int comb_tot=0;
		int ngrades=(int)log2(i+1);
		pool_length[i]=nchecks*pool_depth*ngrades;
		pool[i]=malloc(pool_length[i]*sizeof(gem));
		for (j=0; j<pool_length[i]; ++j) pool[i][j]=(gem){0};

		for (j=0;j<eoc;++j)										// combine gems and put them in temp pools
		if ((i-j)/(j+1) < 10) {								// value ratio < 10
			for (k=0; k< pool_length[j]; ++k)
			if ((pool[j]+k)->grade!=0) {				// extensive false gems check ahead
				for (h=0; h< pool_length[i-1-j]; ++h)
				if ((pool[i-1-j]+h)->grade!=0) {
					int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {						// grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						if ( gem_power(temp) >= gem_power(pool[i][pool_depth*grd]) ) {			// power check
							for (l=pool_depth-1; l>=0; --l) {						// compression check
								if ((temp.damage <= pool[i][pool_depth*grd+l].damage) &&
								    (temp.crit   <= pool[i][pool_depth*grd+l].crit  ) &&
								    (temp.bbound <= pool[i][pool_depth*grd+l].bbound) ) goto exit;
							}
							for (l=1; l<pool_depth && (gem_power(temp) >= gem_power(pool[i][pool_depth*grd+l])); ++l) {		// sort
								pool[i][pool_depth*grd+l-1]=pool[i][pool_depth*grd+l];
							}
							pool[i][pool_depth*grd+l-1]=temp;						// put in pool
							goto exit;
						}
						if ( gem_ranking(temp) >= gem_ranking(pool[i][pool_depth*ngrades+pool_depth*grd]) ) {			// ranking check
							for (l=pool_depth-1; l>=0; --l) {						// compression check
								if ((temp.damage <= pool[i][pool_depth*ngrades+pool_depth*grd+l].damage) &&
								    (temp.crit   <= pool[i][pool_depth*ngrades+pool_depth*grd+l].crit  ) &&
								    (temp.bbound <= pool[i][pool_depth*ngrades+pool_depth*grd+l].bbound) ) goto exit;
							}
							for (l=1; l<pool_depth && (gem_ranking(temp) >= gem_ranking(pool[i][pool_depth*ngrades+pool_depth*grd+l])); ++l) {		// sort
								pool[i][pool_depth*ngrades+pool_depth*grd+l-1]=pool[i][pool_depth*ngrades+pool_depth*grd+l];
							}
							pool[i][pool_depth*ngrades+pool_depth*grd+l-1]=temp;						// put in pool
						}
						exit:;
					}
				}
			}
		}
		if (!(output_options & mask_quiet)) {
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		fflush(stdout);								// forces buffer write, so redirection works well
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
	int output_options=0;
	char filename[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"iqf:"))!=-1) {
		switch(opt) {
			case 'i':
				output_options |= mask_info;
				break;
			case 'q':
				output_options |= mask_quiet;
				break;
			case 'f':
				strcpy(filename,optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1) {
		printf("Improper gem number\n");
		return 1;
	}
	if (filename[0]=='\0') strcpy(filename, "table_kgsloppy");
	worker(len, output_options, filename);
	return 0;
}
