#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
typedef struct Gem_OB gem;    // the strange order is so that managem_utils knows which gem type are we defining as "gem"
#include "managem_utils.h"

void worker(int len, int output_options, int pool_zero, int size)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;
	
	if (pool_zero==1) {             // combine
		gem_init(pool[0],1,1,1);
		gem_init(gems   ,1,1,1);
		if (size==0) size=100;       // reasonable comb sizing
	}
	else {                          // spec
		gem_init(pool[0]  ,1,1,0);
		gem_init(pool[0]+1,1,0,1);
		gem_init(gems     ,1,1,0);
		if (size==0) size=2000;      // reasonable spec sizing
	}
	if (!(output_options & mask_quiet)) gem_print(gems);

	for (i=1; i<len; ++i) {
		int j,k,h,l;
		const int eoc=(i+1)/ (1+1);      // end of combining
		const int j0 =(i+1)/(10+1);      // value ratio < 10
		int comb_tot=0;

		const int grade_max=(int)(log2(i+1)+1);    // gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];              // get the temp pools for every grade
		int  temp_index[grade_max-1];              // index of work point in temp pools
		gem* subpools[grade_max-1];                // get subpools for every grade
		int  subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {            // init everything
			temp_pools[j]=malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=NULL;                       // just to be able to free it
			subpools_length[j]=0;
		}

		for (j=j0; j<eoc; ++j) {         // combine gems and put them in temp array
			gem* dad_array = pool[j];
			gem* mom_array = pool[i-1-j];
			for (k=0; k< pool_length[j]; ++k) {
				int g1=(dad_array+k)->grade;
				for (h=0; h< pool_length[i-1-j]; ++h) {
					int delta=g1 - (mom_array+h)->grade;
					if (abs(delta)<=2) {        // grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						temp_pools[grd][temp_index[grd]]=temp;
						temp_index[grd]++;
						if (temp_index[grd]==size) {                     // let's skim a pool
							int length=size+subpools_length[grd];
							gem* temp_array=malloc(length*sizeof(gem));
							int index=0;
							for (l=0; l<temp_index[grd]; ++l) {           // copy new gems
								temp_array[index]=temp_pools[grd][l];
								index++;
							}
							temp_index[grd]=0;            // temp index reset
							for (l=0; l<subpools_length[grd]; ++l) {      // copy old gems
								temp_array[index]=subpools[grd][l];
								index++;
							}
							free(subpools[grd]);          // free
							gem_sort(temp_array,length);                  // work starts
	
							int broken=0;
							float lim_bbound=-1;
							for (l=length-1;l>=0;--l) {
								if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
									temp_array[l].grade=0;
									broken++;
								}
								else lim_bbound=temp_array[l].bbound;
							}                                // all unnecessary gems destroyed
	
							subpools_length[grd]=length-broken;
							subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));     // pool init via broken
	
							index=0;
							for (l=0; l<length; ++l) {       // copying to subpool
								if (temp_array[l].grade!=0) {
									subpools[grd][index]=temp_array[l];
									index++;
								}
							}
							free(temp_array);     // free
						}                                   // rebuilt subpool[grd], work restarts
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd) {                 // let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array=malloc(length*sizeof(gem));
				int index=0;
				for (l=0; l<temp_index[grd]; ++l) {             // copy new gems
					temp_array[index]=temp_pools[grd][l];
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {        // copy old gems
					temp_array[index]=subpools[grd][l];
					index++;
				}
				free(subpools[grd]);    // free
				gem_sort(temp_array,length);                    // work starts
				int broken=0;
				float lim_bbound=-1;
				for (l=length-1;l>=0;--l) {
					if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
						temp_array[l].grade=0;
						broken++;
					}
					else lim_bbound=temp_array[l].bbound;
				}                                      // all unnecessary gems destroyed
				subpools_length[grd]=length-broken;
				subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));     // pool init via broken
				index=0;
				for (l=0; l<length; ++l) {      // copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}
				}
				free(temp_array);     // free
			}                        // subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i]=malloc(pool_length[i]*sizeof(gem));

		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {       // copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				pool[i][place]=subpools[grd][j];
				place++;
			}
		}
		for (grd=0;grd<grade_max-1;++grd) {       // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}
		gems[i]=pool[i][0];                 // choosing gem (criteria moved to more_power def)
		for (j=1;j<pool_length[i];++j) if (gem_more_powerful(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}
		
		if (!(output_options & mask_quiet)) {
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info)
				printf("Growth:\t%f\n", log(gem_power(gems[i]))/log(i+1));
			if (output_options & mask_debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n",pool_length[i]);
			}
			gem_print(gems+i);
		}
	}
	
	if (output_options & mask_quiet) {     // outputs last if we never seen any
		printf("Value:\t%d\n",len);
		printf("Growth:\t%f\n", log(gem_power(gems[len-1]))/log(len));
		if (output_options & mask_debug)
			printf("Pool:\t%d\n",pool_length[len-1]);
		gem_print(gems+len-1);
	}

	gem* gemf=gems+len-1;  // gem that will be displayed

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
		gemf = gems+best_index;
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 3 || pool_zero!=2) printf("I could not add red!\n\n");
		else {
			int value=gem_getvalue(gemf);
			gemf = gem_putred(pool[value-1], pool_length[value-1], value, &red, &gem_array, 0, 0);
			printf("Gem with red added:\n\n");
			printf("Value:\t%d\n", value);    // made to work well with -u
			printf("Growth:\t%f\n", log(gem_power(*gemf))/log(value));
			gem_print(gemf);
		}
	}

	if (output_options & mask_parens) {
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Gem tree:\n");
		print_tree(gemf, "");
		printf("\n");
	}
	if (output_options & mask_table) print_table(gems, len);
	
	if (output_options & mask_equations) {   // it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gemf);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);     // free
	if (output_options & mask_red && len > 2 && pool_zero==2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;        // speccing by default
	int output_options=0;
	int size=0;             // worker or user must initialize it
	
	while ((opt=getopt(argc,argv,"hptecidqurs:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecidqurs:");
				return 0;
			PTECIDCUR_OPTIONS_BLOCK
			case 's':
				size = atoi(optarg);
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
	else worker(len, output_options, pool_zero, size);
	return 0;
}

