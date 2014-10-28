#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
const int ACC=80;							// ACC is for crit pooling & sorting-> results with 80 are indistinguishable from 1000+ up to 500s
const int ACC_CUT=250;				// ACC_CUT is accuracy for other inexact operations -> 100 differs from exact from 32s
															// while 250 is ok even for 40s+, but takes 2x time
#include "killgem_utils.h"


void worker(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, int size)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(sizeof(gem));
	gem_init(gems   ,1,1,1,1);			// grade damage crit bbound
	gem_init(pool[0],1,1,1,1);			// start gem does not matter
	pool_length[0]=1;
	gem_print(gems);
	if (size==0) size=1000;				// reasonable sizing

	for (i=1; i<len; ++i) {
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
				for (k=0; k< pool_length[j]; ++k) {
					for (h=0; h< pool_length[i-1-j]; ++h) {
						int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gem temp;
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
								
								gem_sort(temp_array,length);									// work starts
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
				for (l=0; l<length; ++l) {      // copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}   
				}
				free(temp_array);     // free
			}												// subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i]=malloc(pool_length[i]*sizeof(gem));
		
		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {      // copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				pool[i][place]=subpools[grd][j];
				place++;
			}   
		}
		for (grd=0;grd<grade_max-1;++grd) {     // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}
		gems[i]=pool[i][0];						// choosing gem (criteria moved to more_power def)
		for (j=1;j<pool_length[i];++j) if (gem_more_powerful(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}

		printf("Value:\t%d\n",i+1);
		if (output_info) {
			printf("Raw:\t%d\n",comb_tot);
			printf("Pool:\t%d\n",pool_length[i]);
		}
		gem_print(gems+i);
		fflush(stdout);								// forces buffer write, so redirection works well
	}
	
	if (output_parens) {
		printf("Combining scheme:\n");
		print_parens(gems+len-1);
		printf("\n\n");
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
	}
	if (output_tree) {
		printf("Gem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
	}
	if (output_table) print_table(gems, len);
	
	if (output_debug) {
		printf("Dumping whole pool of value %d:\n\n",len);
		for (i=0;i<pool_length[len-1];++i) {
			gem_print(pool[len-1]+i);
			print_parens(pool[len-1]+i);
			printf("\n\n");
		}
	}
	if (output_equations) {		// it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gems+len-1);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);		// free
}


int main(int argc, char** argv)
{
	return get_opts_and_call_worker(argc, argv);
}
