#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_OB_appr gem;		// the strange order is so that managem_utils knows which gem type are we defining as "gem"
const int ACC=450;								// 450 is as good as 1000 up to reasonable numbers and takes half the time
#include "managem_utils.h"
typedef struct Gem_O gemO;
#include "leech_utils.h"

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info)
{
	// utils compatibility
}

float gem_amp_global_power(gem gem1, gemO amp1)
{
  return (gem1.leech*1.5+6*0.23*2.8*amp1.leech)*gem1.bbound;
}

int gem_alone_more_powerful(gem gem1, gem gem2, gemO amp2)
{
  return gem1.leech*1.5*gem1.bbound > gem_amp_global_power(gem2, amp2);
}

int gem_amp_more_powerful(gem gem1, gemO amp1, gem gem2, gemO amp2)
{
	return gem_amp_global_power(gem1, amp1) > gem_amp_global_power(gem2, amp2);
}

int subpools_to_big_convert(int* subpools_length, int grd, int index)
{
  int result=0;
  int i;
  for (i=0;i<grd;++i) result+=subpools_length[i];
  result+=index;
  return result;
}

void print_amps_table(gem* gems, gemO* amps, int len)
{
  printf("# Gems\tPower (rescaled)\n");
  int i;
  for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gem_amp_global_power(gems[i], amps[i])/1.5);
  printf("\n");
}

void worker_amps(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info, int managem_limit)
{
  printf("\n");
  int i;
  gem* pool[len];
  int pool_length[len];
  pool[0]=malloc(2*sizeof(gem));
  gem_init(pool[0],1, 1, 0);
  gem_init(pool[0]+1,1, 0, 1);
  pool_length[0]=2;
  
  for (i=1; i<len; ++i) {						// managem computing
    if (managem_limit!=0 && i+1>managem_limit) {			// null gems here
			pool_length[i]=1;
			pool[i]=malloc(sizeof(gem));
			gem_init(pool[i],0,0,0);
		}
		else {
	    int j,k,h;
	    int count_big=0;
	    int eoc=(i+1)/2;        //end of combining
	    int comb_tot=0;
	    for (j=0; j<eoc; ++j) comb_tot+=pool_length[j]*pool_length[i-j-1];
	    gem* pool_big = malloc(comb_tot*sizeof(gem));       //a very big array needs to be in the heap
	    
	    for (j=0;j<eoc;++j) {                               // pool_big gets filled of candidate gems
	      for (k=0; k< pool_length[j]; ++k) {
	        for (h=0; h< pool_length[i-1-j]; ++h) {
	          gem_combine(pool[j]+k, pool[i-1-j]+h, pool_big+count_big);
	          count_big++;
	        }
	      }
	    }
	    
	    gem_sort(pool_big,comb_tot);        
	    int grade_max=(int)(log2(i+1)+1);       // gems with max grade cannot be destroyed, so this is a max, not a sup 
	    int subpools_length[grade_max-1];       // let's divide in grades
	    
	    for (j=0;j<grade_max-1;++j) subpools_length[j]=0;
	    
	    int grd=0;
	    
	    for (j=0;j<comb_tot;++j) {              // see how long subpools are
	      if ((pool_big+j)->grade==grd+2) subpools_length[grd]++;
	      else {
	        grd++;
	        subpools_length[grd]++;
	      }
	    }
	    
	    int broken=0;
	    
	    for (grd=0;grd<grade_max-1;++grd) {     // now we work on the single pools
	      double lim_bbound=-1;
	      for (j=subpools_length[grd]-1;j>=0;--j) {
	        if ((int)(ACC*pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound)<=(int)(ACC*lim_bbound)) {
	          pool_big[subpools_to_big_convert(subpools_length,grd,j)].grade=0;
	          broken++;
	        }
	        else lim_bbound=pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound;
	      }
	    }																// all unnecessary gems destroyed
	    pool_length[i]=comb_tot-broken;     
	    pool[i]=malloc(pool_length[i]*sizeof(gem));         // pool init via broken
	    
	    int place=0;
	    for (j=0;j<comb_tot;++j) {      // copying to pool
	      if (pool_big[j].grade!=0) {
	        pool[i][place]=pool_big[j];
	        place++;
	      }   
	    }
	    free(pool_big);     // free
	    
	    printf("Managem: %d\n",i+1);
	    if (output_info) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
			fflush(stdout);
	  }
	}
  printf("Gem pooling done!\n\n");

  gemO* poolO[len/6];
  int poolO_length[len/6];
  poolO[0]=malloc(sizeof(gemO));
  gem_init_O(poolO[0],1,1);
  poolO_length[0]=1;
  
  for (i=1; i<len/6; ++i) {			// amps computing
		int j,k,h;
		int count_big=0;
		int eoc=(i+1)/2;				//end of combining
		int comb_tot=0;
		for (j=0; j<eoc; ++j) comb_tot+=poolO_length[j]*poolO_length[i-j-1];
		gemO* pool_big = malloc(comb_tot*sizeof(gemO));		//a very big array needs to be in heap
				
		for (j=0;j<eoc;++j) {										// pool_big gets fulled by candidate gems
			for (k=0; k< poolO_length[j]; ++k) {
				for (h=0; h< poolO_length[i-1-j]; ++h) {
				gem_combine_O(poolO[j]+k, poolO[i-1-j]+h, pool_big+count_big);
				count_big++;
				}
			}
		}
		int grade_limsup=(int)(log2(i+1)+1);		//pool initialization (not good for more colours)
		poolO_length[i]=grade_limsup-1;
		poolO[i]=malloc(poolO_length[i]*sizeof(gemO));
		
		for (j=0;j<poolO_length[i];++j) {				//pool fulling (not good for more colours)
			gem_init_O(poolO[i]+j,j+2,0);
			for (k=0;k<comb_tot;k++) {
				if ((pool_big[k].grade==j+2) && gem_better(pool_big[k], poolO[i][j])) {
					poolO[i][j]=pool_big[k];
				}
			}
		}
		free(pool_big);
		
		printf("Amplifier: %d\n",i+1);
    if (output_info) {
			printf("Raw:\t%d\n",comb_tot);
			printf("Pool:\t%d\n\n",poolO_length[i]);
		}
  }
  printf("Amplifier pooling done!\n\n");
  	
  int j,k,h;											// let's choose the right gem-amp combo
  gemO amps[len];
  gem gems[len];
  gem_init(gems,1,1,0);
  gem_init_O(amps,0,0);
  printf("Total value: 1\n");
  printf("Gem:\n");
  gem_print(gems);
  printf("Amplifier:\n");
  gem_print_O(amps);
  
  for (i=1;i<len;++i) {																	// for every total value
		gem_init(gems+i,1,0,0);															// we init the gems 
		gem_init_O(amps+i,0,0);															// to extremely weak ones
		for (j=0;j<=i/6;++j) {															// for every amount of amps we can fit in
			int value = i-6*j;																// this is the amount of gems we have left
			for (k=0;k<pool_length[value];++k) {							// we search in that pool
				if (j!=0) {																			// and if we need an amp
					for (h=0;h<poolO_length[j-1];++h) {						// we look in the amp pool
						if (gem_amp_more_powerful(pool[value][k],poolO[j-1][h],gems[i],amps[i])) {
							gems[i]=pool[value][k];
							amps[i]=poolO[j-1][h];
						}
					}
				}
				else if (gem_alone_more_powerful(pool[value][k],gems[i],amps[i])) {
					gems[i]=pool[value][k];
					gem_init_O(amps+i,0,0);
				}
			}
		}
		printf("Total value:\t%d\n\n", i+1);
		printf("Managem\n");
		if (managem_limit!=0) printf("Managem limit:\t%d\n", managem_limit);
		printf("Value:\t%d\n",gem_getvalue(gems+i));
		if (output_info) printf("Pool:\t%d\n",pool_length[gem_getvalue(gems+i)-1]);
		gem_print(gems+i);
		printf("Amplifier\n");
		printf("Value:\t%d\n",gem_getvalue_O(amps+i));
		if (output_info) printf("Pool:\t%d\n",poolO_length[gem_getvalue_O(amps+i)-1]);
		gem_print_O(amps+i);
		printf("Global power (rescaled):\t%f\n\n", (gems[i].leech*1.5+6*0.23*2.8*amps[i].leech)*gems[i].bbound/1.5);
		fflush(stdout);								// forces buffer write, so redirection works well
	}

  if (output_parens) {
    printf("Managem combining scheme:\n");
    print_parens(gems+len-1);
    printf("\n\n");
    printf("Amplifier combining scheme:\n");
    print_parens_O(amps+len-1);
    printf("\n\n");
  }   
  if (output_tree) {
    printf("Managem tree:\n");
    print_tree(gems+len-1, "");
    printf("\n");
    printf("Amplifier tree:\n");
    print_tree_O(amps+len-1, "");
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
  for (i=0;i<len/6;++i) free(poolO[i]);   // free amps
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
	int managem_limit=0;
	while ((opt=getopt(argc,argv,"ptedil:"))!=-1) {
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
	else worker_amps(len, output_parens, output_tree, output_table, output_debug, output_info, managem_limit);
	return 0;
}

