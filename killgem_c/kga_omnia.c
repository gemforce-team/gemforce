#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
const int ACC=80;							// ACC is for crit pooling & sorting-> results with 60 are indistinguishable from 1000+ up to 40s
const int ACC_CUT=280;				// ACC_CUT is accuracy for other inexact operations -> 100 differs from exact from 32s
															// while 280 is ok even for 40s+, but takes 2x time
const int NT=1048576;					// 2^20 ~ 1m, it's still low, but there's no difference going on (even 10k gives the same results)
#include "killgem_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"
#include <time.h>
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

void print_amps_table(gem* gems, gemY* amps, double* powers, int len)
{
	printf("# Gems\tKillgem\tAmps\tPower (resc. 10m)\n");			// we'll rescale again for 10m, no need to have 10 digits
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%d\t%.6f\n", i+1, gem_getvalue(gems+i), gem_getvalue_Y(amps+i), powers[i]/10000/1000);
	printf("\n");
}

void worker_omnia(int len, int lenc, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, int sizes, int sizec)
{
	printf("\n");
	int i;
	int size=sizes;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	gem_init(pool[0]  ,1,1,1,0);
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	pool_length[0]=2;
	
	for (i=1; i<len; ++i) {				// killgem speccing
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
								int crit_cells=(int)(maxcrit*ACC)+1;											// this pool will be big from the beginning,
								int tree_length=pow(2, ceil(log2(crit_cells)));						// but we avoid binary search
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
					
					gem_sort(temp_array,length);						// work starts
					int broken=0;
					int crit_cells=(int)(maxcrit*ACC)+1;									// this pool will be big from the beginning,
					int tree_length=pow(2, ceil(log2(crit_cells)));				// but we avoid binary search
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
	printf("Gem speccing done!\n\n");

	size=sizec;
	gem* poolc[lenc];
	int poolc_length[lenc];
	poolc[0]=malloc(sizeof(gem));
	gem_init(poolc[0],1,1,1,1);		// grade damage crit bbound
	poolc_length[0]=1;						// start gem does not matter
	
	for (i=1; i<lenc; ++i) {			// killgem combining
		int j,k,h,l;
		int eoc=(i+1)/2;        //end of combining
		int comb_tot=0;
		
		int grade_max=(int)(log2(i+1)+1);       		// gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];								// get the temp pools for every grade
		int  temp_index[grade_max-1];								// index of work point in temp pools
		gem* subpools[grade_max-1];									// get subpools for every grade
		int  subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {							// init everything
			temp_pools[j]=malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=malloc(size*sizeof(gem));
			subpools_length[j]=1;
			gem_init(subpools[j],j+1,0,0,0);
		}
		for (j=0;j<eoc;++j) {												// combine gems and put them in temp pools
			if ((i-j)/(j+1) < 10) {										// value ratio < 10
				for (k=0; k< poolc_length[j]; ++k) {
					for (h=0; h< poolc_length[i-1-j]; ++h) {
						int delta=(poolc[j]+k)->grade - (poolc[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gem temp;
							gem_combine(poolc[j]+k, poolc[i-1-j]+h, &temp);
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
								int crit_cells=(int)(maxcrit*ACC)+1;									// this pool will be big from the beginning,
								int tree_length=pow(2, ceil(log2(crit_cells)));						// but we avoid binary search
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
						free(subpools[grd]);		// free
						
						gem_sort(temp_array,length);								// work starts
						int broken=0;
						int crit_cells=(int)(maxcrit*ACC)+1;											// this pool will be big from the beginning,
						int tree_length=pow(2, ceil(log2(crit_cells)));						// but we avoid binary search
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
				for (l=0; l<length; ++l) {      // copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}   
				}
				free(temp_array);     // free
			}												// subpool[grd] is now full
		}
		poolc_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) poolc_length[i]+=subpools_length[grd];
		poolc[i]=malloc(poolc_length[i]*sizeof(gem));
		
		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {      // copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				poolc[i][place]=subpools[grd][j];
				place++;
			}   
		}
		for (grd=0;grd<grade_max-1;++grd) {     // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}

		printf("Value:\t%d\n",i+1);
		if (output_info) {
			printf("Raw:\t%d\n",comb_tot);
			printf("Average raw:\t%d\n",comb_tot/(grade_max-1));
			printf("Pool:\t%d\n",poolc_length[i]);
		}
		fflush(stdout);								// forces buffer write, so redirection works well
	}
	printf("Gem combining done!\n\n");

	gem* poolcf;
	int poolcf_length;
	
	{															// killgem pool compression
		float maxcrit=0;
		for (i=0; i<poolc_length[lenc-1]; ++i) {		// get maxcrit;
			maxcrit=max(maxcrit, (poolc[lenc-1]+i)->crit);
		}
		gem_sort(poolc[lenc-1],poolc_length[lenc-1]);								// work starts
		int broken=0;
		int crit_cells=(int)(maxcrit*ACC)+1;											// this pool will be big from the beginning,
		int tree_length=pow(2, ceil(log2(crit_cells)));						// but we avoid binary search
		float* tree=malloc((tree_length+crit_cells+1)*(sizeof(float)));					// memory improvement, 2* is not needed	
		for (i=1; i<tree_length+crit_cells+1; ++i) tree[i]=-1;
		int index;
		for (i=poolc_length[lenc-1]-1;i>=0;--i) {																// start from large z
			gem* p_gem=poolc[lenc-1]+i;
			index=(int)(p_gem->crit*ACC);																					// find its place in x
			int wall = (int)(tree_read_max(tree,tree_length,index)*ACC_CUT);			// look at y
			if ((int)(p_gem->bbound*ACC_CUT) > wall) tree_add_element(tree,tree_length,index,p_gem->bbound);
			else {
				p_gem->grade=0;
				broken++;
			}
		}														// all unnecessary gems destroyed
		free(tree);									// free
		
		poolcf_length=poolc_length[lenc-1]-broken;
		poolcf=malloc(poolcf_length*sizeof(gem));			// pool init via broken
		index=0;
		for (i=0; i<poolc_length[lenc-1]; ++i) {			// copying to subpool
			if (poolc[lenc-1][i].grade!=0) {
				poolcf[index]=poolc[lenc-1][i];
				index++;
			}
		}
		printf("Killgem comb compressed pool size:\t%d\n",poolcf_length);
	}
	printf("Gem combining pool compression done!\n\n");

	int lena;
	if (lenc > 2*len) lena=lenc;	// see which is bigger between 2x spec len and comb len
	else lena=2*len;							// and we'll combspec till there
	
	gemY* poolY[lena];						// 2x killgem value
	int poolY_length[lena];
	poolY[0]=malloc(sizeof(gemY));
	gem_init_Y(poolY[0],1,1,1);
	poolY_length[0]=1;
	size=20000;

 for (i=1; i<lena; ++i) {			//amplifier computing
		int j,k,h,l;
		int eoc=(i+1)/2;        	//end of combining
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

	gemY* poolYf;
	int poolYf_length;
	
	{														// amps pool compression
		gem_sort_Y(poolY[lenc-1],poolY_length[lenc-1]);								// work starts
		int broken=0;
		float lim_crit=-1;
		for (i=poolY_length[lenc-1]-1;i>=0;--i) {
			if (poolY[lenc-1][i].crit<=lim_crit) {
				poolY[lenc-1][i].grade=0;
				broken++;
			}
			else lim_crit=poolY[lenc-1][i].crit;
		}													// all unnecessary gems destroyed
		poolYf_length=poolY_length[lenc-1]-broken;
		poolYf=malloc(poolYf_length*sizeof(gemY));		// pool init via broken
		int index=0;
		for (i=0; i<poolY_length[lenc-1]; ++i) {      // copying to pool
			if (poolY[lenc-1][i].grade!=0) {
				poolYf[index]=poolY[lenc-1][i];
				index++;
			}
		}
		printf("Amp comb compressed pool size:\t%d\n",poolYf_length);
	}
	printf("Amp combining pool compression done!\n\n");

	int j,k,h,l,m;								// let's choose the right gem-amp combo
	gem gems[len];								// for every speccing value
	gemY amps[len];								// we'll choose the best amps
	gem gemsc[len];								// and the best NC combine
	gemY ampsc[len];							// for both;
	double powers[len];
	gem_init(gems,1,1,1,0);
	gem_init_Y(amps,0,0,0);
	gem_init(gemsc,1,1,0,0);
	gem_init_Y(ampsc,0,0,0);
	powers[0]=0;
	printf("Gem:\n");
	gem_print(gems);
	printf("Amplifier:\n");
	gem_print_Y(amps);
double time=clock();
	for (i=1;i<len;++i) {																		// for every gem value
		gem_init(gems+i, 0,0,0,0);														// we init the gems
		gem_init_Y(amps+i, 0,0,0);														// to extremely weak ones
		gem_init(gemsc+i,0,0,0,0);
		gem_init_Y(ampsc+i,0,0,0);
		powers[i]=0;
		for (j=-1;j<2*i+2;++j) {															// for every amp value from 0 to to 2*gem_value
			int NS=(i+1)+6*(j+1);																// we get the num of gems used in speccing
			double c = log((double)NT/NS)/log(lenc);						// we compute the combination number
			for (l=0; l<poolcf_length; ++l) {										// then we search in the NC gem comb pool
				double Cbg = pow(poolcf[l].bbound,c);
				double Cdg = pow(poolcf[l].damage,c);
				double Ccg = pow(poolcf[l].crit  ,c);
				double Cg  = pow(gem_power(poolcf[l]),c);
				for (m=0;m<poolYf_length;++m) {										// and in the amp NC pool
					double Cda = pow(poolYf[m].damage,c);
					double Cca = pow(poolYf[m].crit  ,c);
					for (k=0;k<pool_length[i];++k) {								// then we look at the gem pool
						if (pool[i][k].crit!=0) {											// if the gem has crit we go on
							double Palone = Cg * gem_power(pool[i][k]);
							if (j==-1) {																// if no amp is needed we compare the gem alone
								if (Palone>powers[i]) {
									powers[i]=Palone;
									gems[i]=pool[i][k];
									gem_init_Y(amps+i,0,0,0);
									gemsc[i]=poolcf[l];
									gem_init_Y(ampsc+i,0,0,0);
								}
							}
							else {
								double Pb2 = Cbg * pool[i][k].bbound * Cbg * pool[i][k].bbound;
								double Pdg = Cdg * pool[i][k].damage;
								double Pcg = Ccg * pool[i][k].crit  ;
								for (h=0;h<poolY_length[j];++h) {					// and in the amp pool
									double Pdamage = Pdg + 1.47 * Cda * poolY[j][h].damage ;
									double Pcrit   = Pcg + 2.576* Cca * poolY[j][h].crit   ;
									double power   = Pb2 * Pdamage * Pcrit ;
									if (power>powers[i]) {
										powers[i]=power;
										gems[i]=pool[i][k];
										amps[i]=poolY[j][h];
										gemsc[i]=poolcf[l];
										ampsc[i]=poolYf[m];
									}
								}
							}
						}
					}
				}
			}
		}
		printf("Killgem spec\n");
		printf("Value:\t%d\n",i+1);
		if (output_info) printf("Pool:\t%d\n",pool_length[i]);
		gem_print(gems+i);
		printf("Amplifier spec\n");
		printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
		if (output_info) printf("Pool:\t%d\n",poolY_length[gem_getvalue_Y(amps+i)-1]);
		gem_print_Y(amps+i);
		printf("Killgem combine\n");
		printf("Comb:\t%d\n",lenc);
		if (output_info) printf("Pool:\t%d\n",poolcf_length);
		gem_print(gemsc+i);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		if (output_info) printf("Pool:\t%d\n",poolYf_length);
		gem_print_Y(ampsc+i);
		printf("Global power (resc. 10m):\t%f\n\n\n", powers[i]/10000/1000);
		fflush(stdout);								// forces buffer write, so redirection works well
	}
printf("%.0f\n",clock()-time);
	if (output_parens) {
		printf("Killgem speccing scheme:\n");
		print_parens(gems+len-1);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_Y(amps+len-1);
		printf("\n\n");
		printf("Killgem combining scheme:\n");
		print_parens(gemsc+len-1);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_Y(ampsc+len-1);
		printf("\n\n");
	}
	if (output_tree) {
		printf("Killgem speccing tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree_Y(amps+len-1, "");
		printf("\n");
		printf("Killgem combining tree:\n");
		print_tree(gemsc+len-1, "");
		printf("\n");
		printf("Amplifier combining tree:\n");
		print_tree_Y(ampsc+len-1, "");
		printf("\n");
	}
	if (output_table) print_amps_table(gems, amps, powers, len);

	if (output_debug) {			// useless
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
		printf("Killgem speccing equations:\n");
		print_equations(gems+len-1);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations_Y(amps+len-1);
		printf("\n");
		printf("Killgem combining equations:\n");
		print_equations(gemsc+len-1);
		printf("\n");
		printf("Amplifier combining equations:\n");
		print_equations_Y(ampsc+len-1);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<lenc;++i) free(poolc[i]);		// free gems
	free(poolcf);
	for (i=0;i<lena;++i) free(poolY[i]);		// free amps
}


int main(int argc, char** argv)
{
	int len;
	int lenc;
	char opt;
	int output_parens=0;
	int output_equations=0;
	int output_tree=0;
	int output_table=0;
	int output_debug=0;
	int output_info=0;
	int sizes=20000;
	int sizec=1000;

	while ((opt=getopt(argc,argv,"petcdis:"))!=-1) {
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
			case 's':		// must be sizes,sizec
				;
				char* p=optarg;
				while (*p != ',') p++;
				*p='\0';
				sizes=atoi(optarg);
				sizec=atoi(p+1);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
		lenc= len;
	}
	else if (optind+2==argc) {
		len = atoi(argv[optind]);
		lenc= atoi(argv[optind+1]);
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1 || lenc<1) printf("Improper gem number\n");
	else worker_omnia(len, lenc, output_parens, output_equations, output_tree, output_table, output_debug, output_info, sizes, sizec);
	return 0;
}
