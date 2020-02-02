#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
typedef struct Gem_O gem;
#include "leechg_utils.h"

void gem_print(gem *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\n\n", p_gem->grade, p_gem->leech);
}

char gem_color(gem* p_gem) {
	if (p_gem->leech==0) return COLOR_CHHIT;
	else return COLOR_LEECH;
}

#include "print_utils.h"

void worker(int len, options output_options)
{
	printf("\n");
	int i;
	gem* gems = (gem*)malloc(len*sizeof(gem));      // if not malloc-ed 230k is the limit
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(sizeof(gem));
	gem_init(gems,1,1);
	gem_init(pool[0],1,1);
	pool_length[0]=1;
	if (!output_options.quiet) gem_print(gems);

	for (i=1; i<len; ++i) {
		int j,k,h;
		const int eoc=(i+1)/ (1+1);      // end of combining
		const int j0 =(i+1)/(10+1);      // value ratio < 10
		int comb_tot=0;

		const int grade_max=(int)(log2(i+1)+1);    // gems with max grade cannot be destroyed, so this is a max, not a sup
		gem temp_array[grade_max-1];         // this will have all the grades
		for (j=0; j<grade_max-1; ++j) temp_array[j] = {};

		for (j=j0; j<eoc; ++j) {         // combine gems and put them in temp array
			for (k=0; k< pool_length[j]; ++k) {
				int g1=(pool[j]+k)->grade;
				for (h=0; h< pool_length[i-1-j]; ++h) {
					int delta=g1 - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {        // grade difference <= 2
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
		pool[i] = (gem*)malloc(pool_length[i]*sizeof(gem));
		
		int place=0;
		for (j=0; j<grade_max-1; ++j) {      // copying to pool
			if (temp_array[j].grade!=0) {
				pool[i][place]=temp_array[j];
				place++;
			}
		}
		
		gems[i]=pool[i][0];
		for (j=1;j<pool_length[i];++j) if (gem_better(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
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
	
	if (output_options.quiet) {    // outputs last if we never seen any
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
		if (len < 2) printf("I could not add chain!\n\n");
		else {
			int value=gem_getvalue(gemf);
			gemf = gem_putchain(pool[value-1], pool_length[value-1], &gem_array);
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
	
	for (i=0;i<len;++i) free(pool[i]);    // free
	free(gems);
	if (output_options.chain && len > 1) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
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
	worker(len, output_options);
	return 0;
}

