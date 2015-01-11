#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_O gem;
#include "leechg_utils.h"

void gem_print(gem *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\n\n", p_gem->grade, p_gem->leech);
}

char gem_color(gem* p_gem) {
	if (p_gem->leech==0) return 'r';
	else return 'o';
}

#include "print_utils.h"

void worker(int len, int output_options)
{
	printf("\n");
	int i;
	gem* gems=malloc(len*sizeof(gem));		// if not malloc-ed 230k is the limit
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(sizeof(gem));
	gem_init(gems,1,1);
	gem_init(pool[0],1,1);
	pool_length[0]=1;
	if (!(output_options & mask_quiet)) gem_print(gems);

	for (i=1; i<len; ++i) {
		int j,k,h;
		int grade_max=(int)(log2(i+1)+1);		// gems with max grade cannot be destroyed, so this is a max, not a sup
		gem temp_array[grade_max-1];				// this will have all the grades
		for (j=0; j<grade_max-1; ++j) temp_array[j]=(gem){0};
		int eoc=(i+1)/2;				//end of combining
		int comb_tot=0;

		for (j=0;j<eoc;++j)										// combine gems and put them in temp array
		if ((i-j)/(j+1) < 10) {								// value ratio < 10
			for (k=0; k< pool_length[j]; ++k) {
				for (h=0; h< pool_length[i-1-j]; ++h) {
					int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {						// grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						if (gem_better(temp, temp_array[grd])) {
							temp_array[grd]=temp;
						}
					}
				}
			}
		}
		int gemNum=0;
		for (j=0; j<grade_max-1; ++j) if (temp_array[j].grade!=0) gemNum++;
		pool_length[i]=gemNum;
		pool[i]=malloc(pool_length[i]*sizeof(gem));
		
		int place=0;
		for (j=0; j<grade_max-1; ++j) {				// copying to pool
			if (temp_array[j].grade!=0) {
				pool[i][place]=temp_array[j];
				place++;
			}
		}
		
		gems[i]=pool[i][0];
		for (j=1;j<pool_length[i];++j) if (gem_better(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}
		
		if (!(output_options & mask_quiet)) {
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) {
				printf("Growth:\t%f\n", log(gems[i].leech)/log(i+1));
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n",pool_length[i]);
			}
			gem_print(gems+i);
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Value:\t%d\n",len);
		printf("Growth:\t%f\n", log(gems[len-1].leech)/log(len));
		gem_print(gems+len-1);
	}

	if (output_options & mask_upto) {
		double best_growth=0;
		int best_index=0;
		for (i=0; i<len; ++i) {
			if (log(gems[i].leech)/log(i+1) > best_growth) {
				best_index=i;
				best_growth=log(gems[i].leech)/log(i+1);
			}
		}
		printf("Best gem up to %d:\n\n", len);
		printf("Value:\t%d\n",best_index+1);
		printf("Growth:\t%f\n", best_growth);
		gem_print(gems+best_index);
		gems[len-1]=gems[best_index];
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 2) printf("I could not add red!\n\n");
		else {
			int value=gem_getvalue(gems+len-1);
			gems[len-1]=gem_putred(pool[value-1], pool_length[value-1], value, &red, &gem_array);
			printf("Gem with red added:\n\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(value));
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
	
	for (i=0;i<len;++i) free(pool[i]);		// free
	free(gems);
	if (output_options & mask_red && len > 1) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_options=0;

	while ((opt=getopt(argc,argv,"iptcequr"))!=-1) {
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
	if (len<1) printf("Improper gem number\n");
	else worker(len, output_options);
	return 0;
}

