#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
#include "killgem_utils.h"

const int ACC=60;							// ACC is for crit pooling & sorting-> results with 60 are indistinguishable from 1000+ up to 40s
const int ACC_CUT=250;				// ACC_CUT is accuracy for other inexact operations -> 100 differs from exact from 32s
															// while 250 is ok even for 40s+, but takes 2x time
															// Note: (60,250) is as fast as (100,100) but more precise

void gem_init_killgem(gem *p_gem)	//32 spec unamped
{
	p_gem->grade=5;
	p_gem->damage=10.754962;
	p_gem->crit=3.728688;
	p_gem->bbound=1.354566;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_less_equal(gem gem1, gem gem2)
{
  if (gem1.grade!=gem2.grade)
    return gem1.grade<gem2.grade;
  if ((int)(gem1.damage*ACC) != (int)(gem2.damage*ACC))
    return gem1.damage<gem2.damage;
  if ((int)(gem1.bbound*ACC) != (int)(gem2.bbound*ACC))
    return gem1.bbound<gem2.bbound;
  return gem1.crit<gem2.crit;
}

void gem_sort_grade_damage_bbound(gem* gems, int len)
{
  if (len<=1) return;
  int pivot=0;
  int i;
  for (i=1;i<len;++i) {
    if (gem_less_equal(gems[i],gems[pivot])) {
      gem temp=gems[pivot];
      gems[pivot]=gems[i];
      gems[i]=gems[pivot+1];
      gems[pivot+1]=temp;
      pivot++;
    }
  }
  gem_sort_grade_damage_bbound(gems,pivot);
  gem_sort_grade_damage_bbound(gems+1+pivot,len-pivot-1);
}

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	gem_init_killgem(gems);
	gem_init_killgem(pool[0]);
	pool_length[0]=1;
	gem_print(gems);

	for (i=1; i<len; ++i) {
		int j,k,h;
		int count_big=0;
		int eoc=(i+1)/2;		//end of combining
		int comb_tot=0;	
		for (j=0; j<eoc; ++j) comb_tot+=pool_length[j]*pool_length[i-j-1];
		gem* pool_big = malloc(comb_tot*sizeof(gem));			//a very big array needs to be in the heap

		for (j=0;j<eoc;++j) {									// pool_big gets filled of candidate gems
			for (k=0; k< pool_length[j]; ++k) {
				for (h=0; h< pool_length[i-1-j]; ++h) {
				gem_combine(pool[j]+k, pool[i-1-j]+h, pool_big+count_big);
				count_big++;
				}
			}
		}
		
		gem_sort_grade_damage_bbound(pool_big,comb_tot);
		
		int grade_max=(int)(log2(i+1)+1);			// gems with max grade cannot be destroyed, so this is a max, not a sup
		int subpools_length[grade_max-1];			// let's divide in grades
		float maxcrit[grade_max-1];						// this will help me create the minimum tree
		for (j=0;j<grade_max-1;++j) {
			subpools_length[j]=0;
			maxcrit[j]=0;
		}
		int grd=0;
		for (j=0;j<comb_tot;++j) {						// see how long subpools are and find maxcrits
			if ((pool_big+j)->grade==grd+6) {		// careful, the number must be grade of starting gem + 1
				subpools_length[grd]++;
				maxcrit[grd]=max(maxcrit[grd], (pool_big+j)->crit);
			}
			else {
				grd++;
				subpools_length[grd]++;
				maxcrit[grd]=max(maxcrit[grd], (pool_big+j)->crit);
			}
		}
		int broken=0;
		int place;
		for (grd=0;grd<grade_max-1;++grd) {												// now we work on the single pools
			int crit_cells=(int)(maxcrit[grd]*ACC)+1;								// this pool will be big from the beginning,
			int tree_length=pow(2, ceil(log2(crit_cells)));					// but we avoid binary search
			float* tree=malloc((tree_length+crit_cells+1)*(sizeof(float)));					// memory improvement, 2* is not needed	
			for (j=1; j<tree_length+crit_cells+1; ++j) tree[j]=-1;
			for (j=subpools_length[grd]-1;j>=0;--j) {																// start from large z
				gem* p_gem=pool_big+subpools_to_big_convert(subpools_length,grd,j);
				place=(int)(p_gem->crit*ACC);																					// find its place in x
				int wall = (int)(tree_read_max(tree,tree_length,place)*ACC_CUT);			// look at y
				if ((int)(p_gem->bbound*ACC_CUT) > wall) tree_add_element(tree,tree_length,place,p_gem->bbound);
				else {
					p_gem->grade=0;
					broken++;
				}
			}														// all unnecessary gems destroyed
			free(tree);									// free
		}

		pool_length[i]=comb_tot-broken;		
		pool[i]=malloc(pool_length[i]*sizeof(gem));			// pool init via broken
		
		place=0;
		for (j=0;j<comb_tot;++j) {				// copying to pool
			if (pool_big[j].grade!=0) {
				pool[i][place]=pool_big[j];
				place++;
			}
		}		
		free(pool_big);			// free

		gems[i]=pool[i][0];						// choosing gem (criteria moved to gem_power def)
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
  
	for (i=0;i<len;++i) free(pool[i]);		// free
}


int main(int argc, char** argv)
{
	return get_opts_and_call_worker(argc, argv);
}
