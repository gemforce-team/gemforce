#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
const int ACC=0;								// I don't really use it here
#include "killgem_utils.h"
#include "gfon.h"

double gem_cfr_power(gem gem1, void* amp, double dr, double cr)
{
	return gem1.damage*gem1.bbound*gem1.crit*gem1.bbound;			// amp-less
}

void worker(int len, int output_options, int pool_zero, char* filename)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);							// if the file is not good we exit
	int i;
	gem* gems=malloc(len*sizeof(gem));		// if not malloc-ed 230k is the limit
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;

	if (pool_zero==1) {							// combine
		gem_init(pool[0],1,1,1,1);		// start gem does not matter
		gem_init(gems   ,1,1,1,1);		// grade damage crit bbound
	}
	else {													// spec
		gem_init(pool[0]  ,1,1.000000,1,0);
		gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
		gem_init(gems     ,1,1.000000,1,0);		// grade damage crit bbound
	}
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax<len-1) {
		fclose(table);
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	if (!(output_options & mask_quiet)) gem_print(gems);

	for (i=1; i<len; ++i) {
		int j;
		gems[i]=pool[i][0];						// choosing gem (criteria moved to more_power def)
		for (j=1;j<pool_length[i];++j) if (gem_more_powerful(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}
		
		if (!(output_options & mask_quiet)) {
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) {
				printf("Growth:\t%f\n", log(gem_power(gems[i]))/log(i+1));
				printf("Pool:\t%d\n",pool_length[i]);
			}
			gem_print(gems+i);
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Value:\t%d\n",len);
		printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(len));
		gem_print(gems+len-1);
	}

	if (output_options & mask_upto) {
		double best_growth=0;
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
		gems[len-1]=gems[best_index];
	}

	gem** gem_array;
	int array_index;
	if (output_options & mask_red) {
		if (len < 3 || pool_zero!=2) printf("I could not add red!\n\n");
		else {
			gems[len-1]=gem_putred(gems+len-1, len, &gem_array, &array_index, NULL, 0, 0);
			printf("Gem with red added:\n\n");
			printf("Value:\t%d\n",len);
			printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(len));
			gem_print(gems+len-1);
		}
	}

	if (output_options & mask_parens) {
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Gem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
	}
	if (output_options & mask_table) print_table(gems, len);
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gems+len-1);
		printf("\n");
	}
	
	fclose(table);
	for (i=0;i<len;++i) free(pool[i]);		// free
	free(gems);
	if (output_options & mask_red && len > 2 && pool_zero==2) {
		array_free(gem_array, array_index);
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;		// speccing by default
	int output_options=0;
	char filename[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"iptcequrf:"))!=-1) {
		switch(opt) {
			case 'i':
				output_options |= mask_info;
				break;
			case 'p':
				output_options |= mask_parens;
				break;
			case 't':
				output_options |= mask_tree;
				break;
			case 'c':
				output_options |= mask_table;
				break;
			case 'e':
				output_options |= mask_equations;
				break;
			case 'q':
				output_options |= mask_quiet;
				break;
			case 'u':
				output_options |= mask_upto;
				break;
			case 'r':
				output_options |= mask_red;
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
		char* p=argv[optind];
		while (*p != '\0') p++;
		if (*(p-1)=='c') pool_zero=1;
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
	if (filename[0]=='\0') {
		if (pool_zero==2) strcpy(filename, "table_kgspec");
		else strcpy(filename, "table_kgcomb");
	}
	worker(len, output_options, pool_zero, filename);
	return 0;
}

