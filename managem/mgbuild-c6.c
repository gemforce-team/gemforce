#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
typedef struct Gem_OB gem;
#include "managem_utils.h"
#include "gfon.h"

const int nchecks=6;

inline double gem_rk13(gem gem1)
{
	return gem1.bbound*gem1.leech*gem1.leech*gem1.leech;
}

inline double gem_rk12(gem gem1)
{
	return gem1.bbound*gem1.leech*gem1.leech;
}

inline double gem_rk21(gem gem1)
{
	return gem1.bbound*gem1.bbound*gem1.leech;
}

inline double gem_rk31(gem gem1)
{
	return gem1.bbound*gem1.bbound*gem1.bbound*gem1.leech;
}

void worker(int len, int output_options, char* filename)
{
	FILE* table=table_init(filename, 1);    // init managem
	int i;
	gem** pool=malloc(len*sizeof(gem*));    // windows
	int* pool_length=malloc(len*sizeof(int));
	pool[0]=malloc(sizeof(gem));
	pool_length[0]=1;
	gem_init(pool[0],1,1,1);
	
	int prevmax=pool_from_table(pool, pool_length, len, table);   // pool filling
	if (prevmax+1==len) {
		fclose(table);
		for (i=0;i<len;++i) free(pool[i]);    // free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(filename,"a", table);  // append -> updating possible

	for (i=prevmax+1; i<len; ++i) {
		int j,k,h;
		const int eoc=(i+1)/ (1+1);       // end of combining
		const int j0 =(i+1)/(10+1);       // value ratio < 10
		int comb_tot=0;

		const int ngrades=(int)log2(i+1);
		const int temp_length=nchecks*ngrades;
		gem temp_array[temp_length];      // this will have all the grades
		for (j=0; j<temp_length; ++j) temp_array[j]=(gem){0};
		double pow_array[temp_length];    // this will have all the powers
		for (j=0; j<temp_length; ++j) pow_array[j]=0;

		for (j=j0; j<eoc; ++j) {          // combine gems and put them in temp array
			for (k=0; k< pool_length[j]; ++k) {
				int g1=(pool[j]+k)->grade;
				for (h=0; h< pool_length[i-1-j]; ++h) {
					int delta=g1 - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {     // grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						int p0 = grd*nchecks;
						
						if      ( gem_power(temp) > pow_array[p0] ) {			// rk11 check
							pow_array[p0]=gem_power(temp);
							temp_array[p0]=temp;
						}
						else if ( gem_rk13(temp) > pow_array[p0+1] ) {			// rk13 check
							pow_array[p0+1]=gem_rk13(temp);
							temp_array[p0+1]=temp;
						}
						else if ( gem_rk12(temp) > pow_array[p0+2] ) {			// rk12 check
							pow_array[p0+2]=gem_rk12(temp);
							temp_array[p0+2]=temp;
						}
						else if ( gem_rk21(temp) > pow_array[p0+3] ) {			// rk21 check
							pow_array[p0+3]=gem_rk21(temp);
							temp_array[p0+3]=temp;
						}
						else if ( gem_rk31(temp) > pow_array[p0+4] ) {			// rk31 check
							pow_array[p0+4]=gem_rk31(temp);
							temp_array[p0+4]=temp;
						}
						else if ( gem_power(temp) > pow_array[p0+5] ) {			// rk11 check
							pow_array[p0+5]=gem_power(temp);
							temp_array[p0+5]=temp;
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
			if (output_options & mask_debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);			// write on file
	}
	
	fclose(table);      // close
	for (i=0;i<len;++i) free(pool[i]);   // free
	free(pool);         // free
	free(pool_length);  // free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_options=0;
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
	if (filename[0]=='\0') strcpy(filename, "table_mgcomb");
	worker(len, output_options, filename);
	return 0;
}

