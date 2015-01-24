#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_YB gem;		// the strange order is so that killgem_utils knows which gem type are we defining as "gem"
const int ACC=0;							// I don't really use it here
#include "killgem_utils.h"
#include "gfon.h"

const int nchecks=3;

double gem_rk311(gem gem1)
{
	return gem1.bbound*gem1.bbound*gem1.bbound*gem1.crit*gem1.damage;
}

double gem_rk511(gem gem1)
{
	return gem1.bbound*gem1.bbound*gem1.bbound*gem1.bbound*gem1.bbound*gem1.crit*gem1.damage;
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
		int j,k,h;
		int eoc=(i+1)/2;        //end of combining
		int comb_tot=0;

		int ngrades=(int)log2(i+1);
		int temp_length=nchecks*ngrades;
		gem temp_array[temp_length];					// this will have all the grades
		for (j=0; j<temp_length; ++j) temp_array[j]=(gem){0};

		for (j=0;j<eoc;++j)										// combine gems and put them in temp pools
		if ((i-j)/(j+1) < 10) {								// value ratio < 10
			for (k=0; k< pool_length[j]; ++k) {
				int g1=(pool[j]+k)->grade;
				for (h=0; h< pool_length[i-1-j]; ++h) {
					int delta=g1 - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {						// grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						if      ( gem_rk511(temp) >= gem_rk511(temp_array[grd]) ) {							// rk511 check
							temp_array[grd]=temp;							// put in pool
						}
						else if ( gem_power(temp) >= gem_power(temp_array[ngrades+grd]) ) {			// rk211 check
							temp_array[ngrades+grd]=temp;			// put in pool
						}
						else if ( gem_rk311(temp) >= gem_rk311(temp_array[2*ngrades+grd]) ) {		// rk311 check
							temp_array[2*ngrades+grd]=temp;		// put in pool
						}
					}
				}
			}
		}
		int gemNum=0;
		for (j=0; j<temp_length; ++j) if (temp_array[j].grade!=0) gemNum++;
		pool_length[i]=gemNum;
		pool[i]=malloc(pool_length[i]*sizeof(gem));
		
		int place=0;
		for (j=0; j<temp_length; ++j) {				// copying to pool
			if (temp_array[j].grade!=0) {
				pool[i][place]=temp_array[j];
				place++;
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
	if (filename[0]=='\0') strcpy(filename, "table_kgfast2");
	worker(len, output_options, filename);
	return 0;
}

