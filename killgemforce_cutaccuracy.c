#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"

const int ACC=100;						// accuracy for inexact operations -> 100 results indistinguishable from 1000+ up to 32s

struct Gem_YB {
	int grade;							// short does NOT help
	float damage;						// this is MAX damage, with the rand() part neglected
	float crit;							// assumptions: crit chance capped
	float bbound;						// BB hit lv >> 1
	struct Gem_YB* father;	// maximize damage*crit*bbound
	struct Gem_YB* mother;
};

typedef struct Gem_YB gem;

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

void gem_print(gem* p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nBbound:\t%f\nPower:\t%f\n\n", 
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->bbound, p_gem->damage*p_gem->bbound*p_gem->crit*p_gem->bbound);
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
  p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
  else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
  if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
  else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
  if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.78*p_gem1->bbound + 0.31*p_gem2->bbound;
  else p_gem_combined->bbound = 0.78*p_gem2->bbound + 0.31*p_gem1->bbound;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
  p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.86*p_gem1->damage + 0.7*p_gem2->damage;
  else p_gem_combined->damage = 0.86*p_gem2->damage + 0.7*p_gem1->damage;
  if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
  else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
  if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.79*p_gem1->bbound + 0.29*p_gem2->bbound;
  else p_gem_combined->bbound = 0.79*p_gem2->bbound + 0.29*p_gem1->bbound;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
  p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.85*p_gem1->damage + 0.69*p_gem2->damage;
  else p_gem_combined->damage = 0.85*p_gem2->damage + 0.69*p_gem1->damage;
  if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
  else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
  if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.8*p_gem1->bbound + 0.27*p_gem2->bbound;
  else p_gem_combined->bbound = 0.8*p_gem2->bbound + 0.27*p_gem1->bbound; 
}

void gem_combine (gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
  p_gem_combined->father=p_gem1;
  p_gem_combined->mother=p_gem2;
  int delta = p_gem1->grade - p_gem2->grade;
  switch (delta){
    case 0:
      gem_comb_eq(p_gem1, p_gem2, p_gem_combined);
      break;
    case 1:
      gem_comb_d1(p_gem1, p_gem2, p_gem_combined);
      break;
    case -1:
      gem_comb_d1(p_gem2, p_gem1, p_gem_combined);
      break;
    default: 
      gem_comb_gn(p_gem1, p_gem2, p_gem_combined);
      break;
  }
}

void gem_init(gem *p_gem, int grd, double damage, double crit, double bbound)
{
  p_gem->grade =grd;
  p_gem->damage=damage;
  p_gem->crit  =crit;
  p_gem->bbound=bbound;
  p_gem->father=NULL;
  p_gem->mother=NULL;
}

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem1.damage*gem1.bbound*gem1.crit*gem1.bbound > gem2.damage*gem2.bbound*gem2.crit*gem2.bbound);		// amp-less
}		

int subpools_to_big_convert(int* subpools_length, int grd, int index)
{
	int result=0;
	int i;
	for (i=0;i<grd;++i) result+=subpools_length[i];
	result+=index;
	return result;
}

int gem_has_less_damage_bbound(gem gem1, gem gem2)
{
  if (gem1.grade < gem2.grade) return 1;
  else if (gem1.grade == gem2.grade && gem1.damage < gem2.damage) return 1;
  else if (gem1.grade == gem2.grade && gem1.damage == gem2.damage && gem1.bbound < gem2.bbound) return 1;
  else return 0;
}

void gem_sort_grade_damage_bbound_exact(gem* gems, int len)     //exact sort
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

int gem_less_equal(gem gem1, gem gem2)
{
  if (gem1.grade!=gem2.grade)
    return gem1.grade<gem2.grade;
  if ((int)(gem1.damage*ACC) != (int)(gem2.damage*ACC))
    return gem1.damage<gem2.damage;
  if ((int)(gem1.bbound*ACC) != (int)(gem2.bbound*ACC))
    return gem1.bbound<gem2.bbound;
  return gem1.crit>gem2.crit;
}

void gem_sort_grade_damage_bbound(gem* gems, int len)     //exact sort
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

int gem_has_less_crit(gem gem1, gem gem2)
{
  if (gem1.crit < gem2.crit) return 1;
  else if (gem1.crit == gem2.crit && gem1.bbound < gem2.bbound) return 1;
  else return 0;
}

void gem_sort_crit(gem* gems, int len)    // it assumes all gems are at the same grade
{
  if (len<=1) return;											
  int pivot=0;
  int i;
  for (i=1;i<len;++i) {
    if (gem_has_less_crit(gems[i],gems[pivot])) {
      gem temp=gems[pivot];
      gems[pivot]=gems[i];
      gems[i]=gems[pivot+1];
      gems[pivot+1]=temp;
      pivot++;
    }
  }
  gem_sort_crit(gems,pivot);
  gem_sort_crit(gems+1+pivot,len-pivot-1);
}

void print_table(gem* gems, int len)
{
  printf("# Gems\tPower\n");
  int i;
  for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gems[i].damage*gems[i].crit*gems[i].bbound);
  printf("\n");
}

char gem_color(gem* p_gem)
{
  if (p_gem->crit==0) return 'b';
  if (p_gem->bbound==0) return 'y';
  else return 'm';
}

void print_parens(gem* gemf)
{
  if (gemf->father==NULL) printf("%c",gem_color(gemf));
  else {
    printf("(");
    print_parens(gemf->father);
    printf("+");
    print_parens(gemf->mother);
    printf(")");
  }
  return;
}

int gem_getvalue(gem* p_gem)
{
  if(p_gem->father==NULL) return 1;
  else return gem_getvalue(p_gem->father)+gem_getvalue(p_gem->mother);
}

void print_tree(gem* gemf, char* prefix)
{
  if (gemf->father==NULL) {
    printf("━ g1 %c\n",gem_color(gemf));
  }
  else {
    printf("━%d\n",gem_getvalue(gemf));
    printf("%s ┣",prefix);
    char string[strlen(prefix)+2];
    strcpy(string,prefix);
    strcat(string," ┃");
    gem* gem1;
    gem* gem2;
    if (gem_getvalue(gemf->father)>gem_getvalue(gemf->mother)) {
      gem1=gemf->father;
      gem2=gemf->mother;
    }
    else {
      gem2=gemf->father;
      gem1=gemf->mother;
    }
    print_tree(gem1, string);
    printf("%s ┗",prefix);
    char string2[strlen(prefix)+2];
    strcpy(string2,prefix);
    strcat(string2,"  ");
    print_tree(gem2, string2);
  }
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
	gem_init(pool[0]  ,1,1,1,0);
	gem_init(pool[0]+1,1,1,0,1);
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
		for (grd=0;grd<grade_max-1;++grd) {										// now we work on the single pools
			int tree_length=pow(2, ceil(log2((int)(maxcrit[grd]*ACC)+1)));	// this pool will be big from the beginning,
			float* tree=malloc(2*tree_length*(sizeof(float)));							// but we avoid binary search
			for (j=1; j<2*tree_length; ++j) tree[j]=-1;
			for (j=subpools_length[grd]-1;j>=0;--j) {												// start from large z
				gem* p_gem=pool_big+subpools_to_big_convert(subpools_length,grd,j);
				place=(int)(p_gem->crit*ACC);																	// find its place in x
				int wall = (int)(tree_read_max(tree,tree_length,place)*ACC);						// look at y
				if ((int)(p_gem->bbound*ACC) > wall) tree_add_element(tree,tree_length,place,p_gem->bbound);
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
	int len;
	char opt;
	int output_parens=0;
	int output_tree=0;
	int output_table = 0;
	int output_debug=0;
	int output_info=0;
	while ((opt=getopt(argc,argv,"ptedi"))!=-1) {
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
	else worker(len, output_parens, output_tree, output_table, output_debug, output_info);
	return 0;
}
