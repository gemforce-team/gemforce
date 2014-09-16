#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_OB_exact gem;	// I'll use accurate and slow methods here
#include "gem_utils.h"

int gem_more_powerful(gem gem1, gem gem2)
{
  return (gem1.leech*gem1.bbound > gem2.leech*gem2.bbound);       // optimization at infinity hits (hit lv infinity)
}                                                                   // the *0.7 for dual is not required because they'll all be dual

int subpools_to_big_convert(int* subpools_length, int grd, int index)
{
  int result=0;
  int i;
  for (i=0;i<grd;++i) result+=subpools_length[i];
  result+=index;
  return result;
}

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug)
{
  printf("\n");
  int i;
  gem gems[len];
  gem* pool[len];
  int pool_length[len];
  pool[0]=malloc(2*sizeof(gem));
  gem_init(gems,1, 1, 0);
  gem_init(pool[0],1, 1, 0);
  gem_init(pool[0]+1,1, 0, 1);
  pool_length[0]=2;
  gem_print(gems);
  
  for (i=1; i<len; ++i) { 
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
    
    gem_sort_exact(pool_big,comb_tot);        
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
      double lim_bbound=-1;                // thank you Enrico for this great algorithm
      for (j=subpools_length[grd]-1;j>=0;--j) {
        if (pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound<=ACC*lim_bbound) {
          pool_big[subpools_to_big_convert(subpools_length,grd,j)].grade=0;
          broken++;
        }
        else lim_bbound=pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound;
      }
    }                                       // all unnecessary gems destroyed
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
    gems[i]=pool[i][0];                     // choosing gem (criteria moved to more_power def)
    for (j=1;j<pool_length[i];++j) if (gem_more_powerful(pool[i][j],gems[i])) {
      gems[i]=pool[i][j];
    }
    
    printf("Value:\t%d\n",i+1);
    printf("Pool:\t%d\n",pool_length[i]);
    gem_print(gems+i);
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
  
  for (i=0;i<len;++i) free(pool[i]);      // free
}


int main(int argc, char** argv)
{
  int len;
  char opt;
  int output_parens=0;
  int output_tree=0;
  int output_table = 0;
  int output_debug=0;
  while ((opt=getopt(argc,argv,"pted"))!=-1) {
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
  else worker(len, output_parens, output_tree, output_table, output_debug);       //wip vectorize
  return 0;
}

