#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
#include "killgem_utils.h"

const int ACC=100;					// ACC is accuracy for crit pooling -> results with 100 are indistinguishable from 1000+ up to 32s

int gem_has_less_damage_bbound(gem gem1, gem gem2)
{
	if (gem1.grade < gem2.grade) return 1;
	else if (gem1.grade == gem2.grade && gem1.damage < gem2.damage) return 1;
	else if (gem1.grade == gem2.grade && gem1.damage == gem2.damage && gem1.bbound < gem2.bbound) return 1;
	else return 0;
}

void gem_sort_grade_damage_bbound_exact(gem* gems, int len) 	//exact sort
{
  if (len<=1) return;
  int pivot=0;
  int i;
  for (i=1;i<len;++i) {
    if (gem_has_less_damage_bbound(gems[i],gems[pivot])) {
      gem temp=gems[pivot];
      gems[pivot]=gems[i];
      gems[i]=gems[pivot+1];
      gems[pivot+1]=temp;
      pivot++;
    }
  }
  gem_sort_grade_damage_bbound_exact(gems,pivot);
  gem_sort_grade_damage_bbound_exact(gems+1+pivot,len-pivot-1);
}

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	gem_init(gems,1,1,1,0);
	gem_init(pool[0]	,1,1			 ,1,0);
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has mor dmg
	pool_length[0]=2;
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
		
		gem_sort_grade_damage_bbound_exact(pool_big,comb_tot);
		
		int grade_max=(int)(log2(i+1)+1);			// gems with max grade cannot be destroyed, so this is a max, not a sup	
		int subpools_length[grade_max-1];			// let's divide in grades
		float maxcrit[grade_max-1];						// this will help me create the minimum tree	
		for (j=0;j<grade_max-1;++j) {
			subpools_length[j]=0;
			maxcrit[j]=0;
		}
		int grd=0;
		for (j=0;j<comb_tot;++j) {						// see how long subpools are and find maxcrits
			if ((pool_big+j)->grade==grd+2) {
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
					
			for (j=subpools_length[grd]-1;j>=0;--j) {												// start from large z	
				gem* p_gem=pool_big+subpools_to_big_convert(subpools_length,grd,j);
				place=(int)(p_gem->crit*ACC);																	// find its place in x
				float wall = tree_read_max(tree,tree_length,place);						// look at y
				if (p_gem->bbound > wall) tree_add_element(tree,tree_length,place,p_gem->bbound);
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
