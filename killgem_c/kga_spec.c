#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
const int ACC=60;							// ACC is for crit pooling & sorting-> results with 60 are indistinguishable from 1000+ up to 40s
const int ACC_CUT=280;				// ACC_CUT is accuracy for other inexact operations -> 100 differs from exact from 32s
															// while 280 is ok even for 40s+, but takes 2x time
#include "killgem_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"

void worker(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, int size)
{
	// utils compatibility
}

float gem_amp_power(gem gem1, gemY amp1)		// should be ok...
{
	return (gem1.damage+6*0.28*(2.8/3.2)*amp1.damage)*gem1.bbound*(gem1.crit+4*0.23*2.8*amp1.crit)*gem1.bbound;		// yes, fraction and 4, due to 3.2 and 1.5 rescaling
}

int gem_alone_more_powerful(gem gem1, gem gem2, gemY amp2)
{
	return gem1.damage*gem1.bbound*gem1.crit*gem1.bbound > gem_amp_power(gem2, amp2);
}

int gem_amp_more_powerful(gem gem1, gemY amp1, gem gem2, gemY amp2)
{
	return gem_amp_power(gem1, amp1) > gem_amp_power(gem2, amp2);
}

void print_amps_table(gem* gems, gemY* amps, float* spec_coeffs, int len)
{
	printf("Killgem\tAmps\tPower (resc.)\tSpec coeff\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%.6f\t%.6lf\n", i+1, gem_getvalue_Y(amps+i), gem_amp_power(gems[i], amps[i]), spec_coeffs[i]);
	printf("\n");
}

void worker_amps(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, float growth_comb, int size)
{
	printf("\n");
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	gem_init(pool[0]	,1,1,1,0);
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	pool_length[0]=2;

	for (i=1; i<len; ++i) {		// killgem computing
		int j,k,h,l;
		int eoc=(i+1)/2;				//end of combining
		long comb_tot=0;
		
		int grade_max=(int)(log2(i+1)+1);						// gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];								// get the temp pools for every grade
		int	temp_index[grade_max-1];								// index of work point in temp pools
		gem* subpools[grade_max-1];									// get subpools for every grade
		int	subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {							// init everything
			temp_pools[j]=malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=malloc(size*sizeof(gem));
			subpools_length[j]=1;
			gem_init(subpools[j],j+1,0,0,0);
		}
		for (j=0;j<eoc;++j) {												// combine gems and put them in temp pools
			if ((i-j)/(j+1) < 10) {										// value ratio < 10
				for (k=0; k< pool_length[j]; ++k) {
					for (h=0; h< pool_length[i-1-j]; ++h) {
						int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							gem temp;
							comb_tot++;
							gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
							int grd=temp.grade-2;
							temp_pools[grd][temp_index[grd]]=temp;
							temp_index[grd]++;
							if (temp_index[grd]==size) {									// let's skim a pool
								int length=size+subpools_length[grd];
								gem* temp_array=malloc(length*sizeof(gem));
								int index=0;
								float maxcrit=0;							// this will help me create the minimum tree
								for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
									temp_array[index]=temp_pools[grd][l];
									maxcrit=max(maxcrit, (temp_array+index)->crit);
									index++;
								}
								temp_index[grd]=0;				// temp index reset
								for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
									temp_array[index]=subpools[grd][l];
									maxcrit=max(maxcrit, (temp_array+index)->crit);
									index++;
								}
								free(subpools[grd]);		// free
								
								gem_sort(temp_array,length);								// work starts
								int broken=0;
								int crit_cells=(int)(maxcrit*ACC)+1;				// this pool will be big from the beginning, but we avoid binary search
								int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
								float* tree=malloc((tree_length+crit_cells+1)*(sizeof(float)));					// memory improvement, 2* is not needed
								for (l=1; l<tree_length+crit_cells+1; ++l) tree[l]=-1;
								for (l=length-1;l>=0;--l) {																							// start from large z
									gem* p_gem=temp_array+l;
									index=(int)(p_gem->crit*ACC);																					// find its place in x
									int wall = (int)(tree_read_max(tree,tree_length,index)*ACC_CUT);			// look at y
									if ((int)(p_gem->bbound*ACC_CUT) > wall) tree_add_element(tree,tree_length,index,p_gem->bbound);
									else {
										p_gem->grade=0;
										broken++;
									}
								}														// all unnecessary gems destroyed
								free(tree);									// free
								
								subpools_length[grd]=length-broken;
								subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
								
								index=0;
								for (l=0; l<length; ++l) {			// copying to subpool
									if (temp_array[l].grade!=0) {
										subpools[grd][index]=temp_array[l];
										index++;
									}
								}
								free(temp_array);			// free
							}												// rebuilt subpool[grd], work restarts
						}
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd)	{									// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array=malloc(length*sizeof(gem));
				int index=0;
				float maxcrit=0;							// this will help me create the minimum tree
					for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
						temp_array[index]=temp_pools[grd][l];
						maxcrit=max(maxcrit, (temp_array+index)->crit);
						index++;
					}
					for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
						temp_array[index]=subpools[grd][l];
						maxcrit=max(maxcrit, (temp_array+index)->crit);
						index++;
					}
					free(subpools[grd]);			// free
					
					gem_sort(temp_array,length);								// work starts
					int broken=0;
					int crit_cells=(int)(maxcrit*ACC)+1;				// this pool will be big from the beginning, but we avoid binary search
					int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
					float* tree=malloc((tree_length+crit_cells+1)*(sizeof(float)));					// memory improvement, 2* is not needed
					for (l=1; l<tree_length+crit_cells+1; ++l) tree[l]=-1;
					for (l=length-1;l>=0;--l) {																							// start from large z
						gem* p_gem=temp_array+l;
						index=(int)(p_gem->crit*ACC);																					// find its place in x
						int wall = (int)(tree_read_max(tree,tree_length,index)*ACC_CUT);			// look at y
						if ((int)(p_gem->bbound*ACC_CUT) > wall) tree_add_element(tree,tree_length,index,p_gem->bbound);
						else {
							p_gem->grade=0;
							broken++;
						}
					}														// all unnecessary gems destroyed
					free(tree);									// free

					subpools_length[grd]=length-broken;
					subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {			// copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}
				}
				free(temp_array);			// free
			}												// subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i]=malloc(pool_length[i]*sizeof(gem));

		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {			// copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				pool[i][place]=subpools[grd][j];
				place++;
			}
		}
		for (grd=0;grd<grade_max-1;++grd) {		 // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}

		printf("Killgem: %d\n",i+1);
		if (output_info) {
			printf("Total raw:\t%ld\n",comb_tot);
			printf("Average raw:\t%ld\n",comb_tot/(grade_max-1));
			printf("Pool:\t%d\n\n",pool_length[i]);
		}
		fflush(stdout);
	}
	printf("Gem pooling done!\n\n");

	gemY* poolY[2*len];					// 2x killgem value
	int poolY_length[2*len];
	poolY[0]=malloc(sizeof(gemY));
	gem_init_Y(poolY[0],1,1,1);
	poolY_length[0]=1;
	size=20000;

 for (i=1; i<2*len; ++i) {			//amplifier computing
		int j,k,h,l;
		int eoc=(i+1)/2;        //end of combining
		int comb_tot=0;

		int grade_max=(int)(log2(i+1)+1);						// gems with max grade cannot be destroyed, so this is a max, not a sup
		gemY* temp_pools[grade_max-1];							// get the temp pools for every grade
		int  temp_index[grade_max-1];								// index of work point in temp pools
		gemY* subpools[grade_max-1];								// get subpools for every grade
		int  subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {							// init everything
			temp_pools[j]=malloc(size*sizeof(gemY));
			temp_index[j]=0;
			subpools[j]=malloc(size*sizeof(gemY));
			subpools_length[j]=1;
			gem_init_Y(subpools[j],j+1,0,0);
		}
		for (j=0;j<eoc;++j) {												// combine gems and put them in temp pools
			if ((i-j)/(j+1) < 10) {										// value ratio < 10
				for (k=0; k< poolY_length[j]; ++k) {
					for (h=0; h< poolY_length[i-1-j]; ++h) {
						int delta=(poolY[j]+k)->grade - (poolY[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gemY temp;
							gem_combine_Y(poolY[j]+k, poolY[i-1-j]+h, &temp);
							int grd=temp.grade-2;
							temp_pools[grd][temp_index[grd]]=temp;
							temp_index[grd]++;
							if (temp_index[grd]==size) {									// let's skim a pool
								int length=size+subpools_length[grd];
								gemY* temp_array=malloc(length*sizeof(gemY));
								int index=0;
								for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
									temp_array[index]=temp_pools[grd][l];
									index++;
								}
								temp_index[grd]=0;				// temp index reset
								for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
									temp_array[index]=subpools[grd][l];
									index++;
								}
								free(subpools[grd]);		// free
								gem_sort_Y(temp_array,length);								// work starts
		
								int broken=0;
								float lim_crit=-1;
								for (l=length-1;l>=0;--l) {
									if (temp_array[l].crit<=lim_crit) {
										temp_array[l].grade=0;
										broken++;
									}
									else lim_crit=temp_array[l].crit;
								}													// all unnecessary gems destroyed
		
								subpools_length[grd]=length-broken;
								subpools[grd]=malloc(subpools_length[grd]*sizeof(gemY));		// pool init via broken
		
								index=0;
								for (l=0; l<length; ++l) {      // copying to subpool
									if (temp_array[l].grade!=0) {
										subpools[grd][index]=temp_array[l];
										index++;
									}
								}
								free(temp_array);     // free
							}												// rebuilt subpool[grd], work restarts
						}
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd)  {									// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gemY* temp_array=malloc(length*sizeof(gemY));
				int index=0;
				for (l=0; l<temp_index[grd]; ++l) {										// copy new gems
					temp_array[index]=temp_pools[grd][l];
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
					temp_array[index]=subpools[grd][l];
					index++;
				}
				free(subpools[grd]);		// free
				gem_sort_Y(temp_array,length);								// work starts
				int broken=0;
				float lim_crit=-1;
				for (l=length-1;l>=0;--l) {
					if (temp_array[l].crit<=lim_crit) {
						temp_array[l].grade=0;
						broken++;
					}
					else lim_crit=temp_array[l].crit;
				}													// all unnecessary gems destroyed
				subpools_length[grd]=length-broken;
				subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {      // copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}
				}
				free(temp_array);     // free
			}												// subpool[grd] is now full
		}
		poolY_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) poolY_length[i]+=subpools_length[grd];
		poolY[i]=malloc(poolY_length[i]*sizeof(gemY));

		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {      // copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				poolY[i][place]=subpools[grd][j];
				place++;
			}
		}
		for (grd=0;grd<grade_max-1;++grd) {     // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}
		printf("Amplifier: %d\n",i+1);
		if (output_info) {
			printf("Raw:\t%d\n",comb_tot);
			printf("Pool:\t%d\n\n",poolY_length[i]);
		}
		fflush(stdout);
	}
	printf("Amplifier pooling done!\n\n");

	int j,k,h;											// let's choose the right gem-amp combo
	gemY amps[len];
	gem gems[len];
	float spec_coeffs[len];
	gem_init(gems,1,1,0,0);
	gem_init_Y(amps,0,0,0);
	spec_coeffs[0]=0;
	printf("Gem:\n");
	gem_print(gems);
	printf("Amplifier:\n");
	gem_print_Y(amps);
	
	for (i=1;i<len;++i) {															// for every gem value
		gem_init(gems+i,0,0,0,0);												// we init the gems
		gem_init_Y(amps+i,0,0,0);												// to extremely weak ones
		spec_coeffs[i]=0;
		for (j=-1;j<2*i+2;++j) {												// for every amp value from 0 to to 2*gem_value
			int NS=(i+1)+6*(j+1);
			float comb_coeff=pow(NS, -growth_comb);
			for (k=0;k<pool_length[i];++k) {							// we search in the gem pool 
				if (j==-1) {																// if no amp is needed we already know
					float power=gem_power(pool[i][k]);
					float spec_coeff=power*comb_coeff;
					if (spec_coeff>spec_coeffs[i]) {
						spec_coeffs[i]=spec_coeff;
						gems[i]=pool[i][k];
						gem_init_Y(amps+i,0,0,0);
					}
				}
				else for (h=0;h<poolY_length[j];++h) {			// else we look in the amp pool
					float power=gem_amp_power(pool[i][k], poolY[j][h]);
					float spec_coeff=power*comb_coeff;
					if (spec_coeff>spec_coeffs[i]) {
						spec_coeffs[i]=spec_coeff;
						gems[i]=pool[i][k];
						amps[i]=poolY[j][h];
					}
				}
			}
		}
		printf("Total value:\t%d\n\n", i+1+6*gem_getvalue_Y(amps+i));
		printf("Killgem\n");
		printf("Value:\t%d\n",gem_getvalue(gems+i));
		if (output_info) printf("Pool:\t%d\n",pool_length[gem_getvalue(gems+i)-1]);
		gem_print(gems+i);
		printf("Amplifier\n");
		printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
		if (output_info) printf("Pool:\t%d\n",poolY_length[gem_getvalue_Y(amps+i)-1]);
		gem_print_Y(amps+i);
		printf("Global power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i]));
		printf("Spec coefficient:\t%f\n\n", spec_coeffs[i]);
		fflush(stdout);								// forces buffer write, so redirection works well
	}

	if (output_parens) {
		printf("Killgem combining scheme:\n");
		print_parens(gems+len-1);
		printf("\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_Y(amps+len-1);
		printf("\n");
		print_parens_compressed_Y(amps+len-1);
		printf("\n\n");
	}
	if (output_tree) {
		printf("Killgem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree_Y(amps+len-1, "");
		printf("\n");
	}
	if (output_table) print_amps_table(gems, amps, spec_coeffs, len);

	if (output_debug) {
		printf("Printing all parens for every best setup:\n\n");
		for (i=2;i<len;++i) {
			printf("Total value:\t%d\n\n",i+1);
			printf("Killgem combining scheme:\n");
			print_parens(gems+i-1);
			printf("\n\n");
			printf("Amplifier combining scheme:\n");
			print_parens_Y(amps+i-1);
			printf("\n\n\n");
		}
	}
	if (output_equations) {		// it ruins gems, must be last
		printf("Killgem equations:\n");
		print_equations(gems+len-1);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations_Y(amps+len-1);
		printf("\n");
	}

	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<2*len;++i) free(poolY[i]);		// free amps
}


int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_parens=0;
	int output_equations=0;
	int output_tree=0;
	int output_table=0;
	int output_debug=0;
	int output_info=0;
	int size=20000;
	float growth_comb=1.415847;		// 16c

	while ((opt=getopt(argc,argv,"petcdis:g:"))!=-1) {
		switch(opt) {
			case 'p':
				output_parens = 1;
				break;
			case 'e':
				output_equations = 1;
				break;
			case 't':
				output_tree = 1;
				break;
			case 'c':
				output_table = 1;
				break;
			case 'd':
				output_debug = 1;
				output_info = 1;
				break;
			case 'i':
				output_info = 1;
				break;
			case 'g':
				growth_comb = atof(optarg);
				break;
			case 's':
				size = atoi(optarg);
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
	else worker_amps(len, output_parens, output_equations, output_tree, output_table, output_debug, output_info, growth_comb, size);
	return 0;
}
