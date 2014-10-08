#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
#include "killgem_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"

const int ACC=60;							// ACC is for crit pooling & sorting-> results with 60 are indistinguishable from 1000+ up to 40s
const int ACC_CUT=250;				// ACC_CUT is accuracy for other inexact operations -> 100 differs from exact from 32s
															// while 250 is ok even for 40s+, but takes 2x time
															// Note: (60,250) is as fast as (100,100) but more precise

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info)
{
	// utils compatibility
}

float gem_amp_global_power(gem gem1, gemY amp1)		// should be ok...
{
  return (gem1.damage*3.2+6*0.28*2.8*amp1.damage)*gem1.bbound*(gem1.crit*1.5+6*0.23*2.8*amp1.crit)*gem1.bbound;
}

int gem_alone_more_powerful(gem gem1, gem gem2, gemY amp2)
{
  return gem1.damage*3.2*gem1.bbound*gem1.crit*1.5*gem1.bbound > gem_amp_global_power(gem2, amp2);
}

int gem_amp_more_powerful(gem gem1, gemY amp1, gem gem2, gemY amp2)
{
	return gem_amp_global_power(gem1, amp1) > gem_amp_global_power(gem2, amp2);
}

int gem_less_equal(gem gem1, gem gem2)
{
  if ((int)(gem1.damage*ACC) != (int)(gem2.damage*ACC))
    return gem1.damage<gem2.damage;
  if ((int)(gem1.bbound*ACC) != (int)(gem2.bbound*ACC))
    return gem1.bbound<gem2.bbound;
  return gem1.crit<gem2.crit;
}

void gem_sort_damage_bbound(gem* gems, int len)
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
  gem_sort_damage_bbound(gems,pivot);
  gem_sort_damage_bbound(gems+1+pivot,len-pivot-1);
}

void print_amps_table(gem* gems, gemY* amps, int len)
{
  printf("# Gems\tKillgem\tAmps\tPower (rescaled)\n");
  int i;
  for (i=0;i<len;i++) printf("%d\t%d\t%d\t%.6lf\n", i+1, gem_getvalue(gems+i), gem_getvalue_Y(amps+i), gem_amp_global_power(gems[i], amps[i])/(1.5*3.2));
  printf("\n");
}

void worker_amps(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info, int killgem_limit, int size)
{
  printf("\n");
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	gem_init(pool[0]  ,1,1,1,0);
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	pool_length[0]=2;

	for (i=1; i<len; ++i) {		// killgem computing
		if (killgem_limit!=0 && i+1>killgem_limit) {			// null gems here
			pool_length[i]=1;
			pool[i]=malloc(sizeof(gem));
			gem_init(pool[i],0,0,0,0);
		}
		else {
			
			int j,k,h,l;
	    int eoc=(i+1)/2;        //end of combining
	    int comb_tot=0;
	    for (j=0; j<eoc; ++j) comb_tot+=pool_length[j]*pool_length[i-j-1];
	    
	    int grade_max=(int)(log2(i+1)+1);       		// gems with max grade cannot be destroyed, so this is a max, not a sup
	    gem* temp_pools[grade_max-1];								// get the temp pools for every grade
	    int  temp_index[grade_max-1];								// index of work point in temp pools
	    gem* subpools[grade_max-1];									// get subpools for every grade
	    int  subpools_length[grade_max-1];
	    for (j=0; j<grade_max-1; ++j) {							// init everything
				temp_pools[j]=malloc(size*sizeof(gem));
				temp_index[j]=0;
				subpools_length[j]=0;
			}
	    for (j=0;j<eoc;++j) {                               // combine gems and put them in temp pools
	      for (k=0; k< pool_length[j]; ++k) {
	        for (h=0; h< pool_length[i-1-j]; ++h) {
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
							if (subpools_length[grd]!=0) free(subpools[grd]);		// free
							
							gem_sort_damage_bbound(temp_array,length);								// work starts
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
						if (subpools_length[grd]!=0) free(subpools[grd]);		// free
						
						gem_sort_damage_bbound(temp_array,length);								// work starts
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
	    
	    printf("Killgem: %d\n",i+1);
	    if (output_info) {
				printf("Total raw:\t%d\n",comb_tot);
				printf("Average raw:\t%d\n",comb_tot/(grade_max-1));
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
			fflush(stdout);
	  }
	}
  printf("Gem pooling done!\n\n");

  gemY* poolY[len/6];
  int poolY_length[len/6];
  poolY[0]=malloc(sizeof(gemY));
  gem_init_Y(poolY[0],1,1,1);
  poolY_length[0]=1;
  
 for (i=1; i<len/6; ++i) {			//amplifier computing
		int j,k,h;
		int count_big=0;
		int eoc=(i+1)/2;		//end of combining
		int comb_tot=0;
		for (j=0; j<eoc; ++j) comb_tot+=poolY_length[j]*poolY_length[i-j-1];
		gemY* pool_big = malloc(comb_tot*sizeof(gemY));		//a very big array needs to be in the heap
				
		for (j=0;j<eoc;++j) {								// pool_big gets filled of candidate gems
			for (k=0; k< poolY_length[j]; ++k) {
				for (h=0; h< poolY_length[i-1-j]; ++h) {
				gem_combine_Y(poolY[j]+k, poolY[i-1-j]+h, pool_big+count_big);
				count_big++;
				}
			}
		}

		gem_sort(pool_big,comb_tot);
		int grade_max=(int)(log2(i+1)+1);		// gems with max grade cannot be destroyed, so this is a max, not a sup	
		int subpools_length[grade_max-1];		// let's divide in grades
		
		for (j=0;j<grade_max-1;++j) subpools_length[j]=0;

		int grd=0;
		
		for (j=0;j<comb_tot;++j) {				// see how long subpools are
			if ((pool_big+j)->grade==grd+2) subpools_length[grd]++;
			else {
				grd++;
				subpools_length[grd]++;
			}
		}

		int broken=0;
		
		for (grd=0;grd<grade_max-1;++grd) {		// now we work on the single pools
			float lim_crit=-1;
			for (j=subpools_length[grd]-1;j>=0;--j) {
				if (pool_big[subpools_to_big_convert(subpools_length,grd,j)].crit<=lim_crit) {
					pool_big[subpools_to_big_convert(subpools_length,grd,j)].grade=0;
					broken++;
				}
				else lim_crit=pool_big[subpools_to_big_convert(subpools_length,grd,j)].crit;
			}
		}										// all unnecessary gems destroyed
		poolY_length[i]=comb_tot-broken;		
		poolY[i]=malloc(poolY_length[i]*sizeof(gem));			// pool init via broken
		
		int place=0;
		for (j=0;j<comb_tot;++j) {		// copying to pool
			if (pool_big[j].grade!=0) {
				poolY[i][place]=pool_big[j];
				place++;
			}	
		}
		free(pool_big);		// free
		
		printf("Amplifier: %d\n",i+1);
    if (output_info) {
			printf("Raw:\t%d\n",comb_tot);
			printf("Pool:\t%d\n\n",poolY_length[i]);
		}
  }
  printf("Amplifier pooling done!\n\n");
  
  int j,k,h;											// let's choose the right gem-amp combo
  gemY amps[len];
  gem gems[len];
  gem_init(gems,1,1,1,0);
  gem_init_Y(amps,0,0,0);
  printf("Total value: 1\n");
  printf("Gem:\n");
  gem_print(gems);
  printf("Amplifier:\n");
  gem_print_Y(amps);
  
  for (i=1;i<len;++i) {																	// for every total value
		gem_init(gems+i,0,0,0,0);														// we init the gems 
		gem_init_Y(amps+i,0,0,0);														// to extremely weak ones
		for (j=0;j<=i/6;++j) {															// for every amount of amps we can fit in
			int value = i-6*j;																// this is the amount of gems we have left
			for (k=0;k<pool_length[value];++k) {							// we search in that pool
				if (j!=0) {																			// and if we need an amp
					for (h=0;h<poolY_length[j-1];++h) {						// we look in the amp pool
						if (pool[value][k].crit!=0 && gem_amp_more_powerful(pool[value][k],poolY[j-1][h],gems[i],amps[i])) {
							gems[i]=pool[value][k];
							amps[i]=poolY[j-1][h];
						}
					}
				}
				else if (gem_alone_more_powerful(pool[value][k],gems[i],amps[i])) {
					gems[i]=pool[value][k];
					gem_init_Y(amps+i,0,0,0);
				}
			}
		}
		printf("Total value:\t%d\n\n", i+1);
		printf("Killgem\n");
		if (killgem_limit!=0) printf("Killgem limit:\t%d\n", killgem_limit);
		printf("Value:\t%d\n",gem_getvalue(gems+i));
		if (output_info) printf("Pool:\t%d\n",pool_length[gem_getvalue(gems+i)-1]);
		gem_print(gems+i);
		printf("Amplifier\n");
		printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
		if (output_info) printf("Pool:\t%d\n",poolY_length[gem_getvalue_Y(amps+i)-1]);
		gem_print_Y(amps+i);
		printf("Global power (rescaled):\t%f\n\n", (gem_amp_global_power(gems[i], amps[i])/(3.2*1.5)));
		fflush(stdout);								// forces buffer write, so redirection works well
	}

  if (output_parens) {
    printf("Killgem combining scheme:\n");
    print_parens(gems+len-1);
    printf("\n\n");
    printf("Amplifier combining scheme:\n");
    print_parens_Y(amps+len-1);
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
  if (output_table) print_amps_table(gems, amps, len);
  
  if (output_debug) {											// quite useless...
    printf("Dumping whole pool of value %d:\n\n",len);
    for (i=0;i<pool_length[len-1];++i) {
      gem_print(pool[len-1]+i);
      print_parens(pool[len-1]+i);
      printf("\n\n");
    }
  }
  
  for (i=0;i<len;++i) free(pool[i]);      // free gems
  for (i=0;i<len/6;++i) free(poolY[i]);   // free amps
}


int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_parens=0;
	int output_tree=0;
	int output_table = 0;
	int output_debug=0;
	int output_info=0;
	int size=20000;
	int managem_limit=0;
	
	while ((opt=getopt(argc,argv,"ptedis:l:"))!=-1) {
		switch(opt) {
			case 'p':
				output_parens = 1;
				break;
			case 't':
				output_tree = 1;
				break;
			case 'e':
				output_table = 1;
				break;
			case 'd':
				output_debug = 1;
				output_info = 1;
				break;
			case 'i':
				output_info = 1;
				break;
			case 'l':
				managem_limit = atoi(optarg);
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
	else worker_amps(len, output_parens, output_tree, output_table, output_debug, output_info, managem_limit, size);
	return 0;
}

