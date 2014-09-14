#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

typedef struct Gem {
	int grade;
	double leech;
	struct Gem* father;
	struct Gem* mother;
} gem;

void int_swap (int *p1, int *p2)
{
	int t=*p1;
	*p1=*p2;
	*p2=t;
}

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

void gem_print(gem *p_gem) {
	printf("Grade: %d\nLeech: %f\n\n", p_gem->grade, p_gem->leech);
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
	else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
	else p_gem_combined->leech = 0.9*p_gem2->leech + 0.38*p_gem1->leech;
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

void gem_init(gem *p_gem, int grd)
{
	p_gem->grade=grd;
	p_gem->leech= pow(1.38, (double)(grd-1));
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_better(gem gem1, gem gem2)
{
	return gem1.leech>=gem2.leech;
}

void table_print(gem* gems, int len)
{
	printf("# Gems\tLeech\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gems[i].leech);
	printf("\n");
}

void scheme_find(gem* gemf)
{
	if (gemf->father==NULL) {
		printf("g");
		return;
	}
	else {
		printf("(");
		scheme_find(gemf->father);
		printf("+");
		scheme_find(gemf->mother);
		printf(")");
	}
	return;
}

void worker(int len, int scheme, int table_output)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_lenght[len];		// there is a limsup for every i: (int)(log2(i+1))+1
	pool[0]=malloc(sizeof(gem));
	gem_init(gems,1);
	gem_init(pool[0],1);
	pool_lenght[0]=1;
	gem_print(gems);
	
	for (i=1; i<len; ++i) {	
		int j,k,h;
		int count_big=0;
		int eoc=(i+1)/2;		//end of combining
		int comb_tot=0;
		for (j=0; j<eoc; ++j) comb_tot+=pool_lenght[j]*pool_lenght[i-j-1];
		gem pool_big[comb_tot];
				
		for (j=0;j<eoc;++j) {				// pool_big gets fulled by candidate gems
			for (k=0; k< pool_lenght[j]; ++k) {
				for (h=0; h< pool_lenght[i-1-j]; ++h) {
				gem_combine(pool[j]+k, pool[i-1-j]+h, pool_big+count_big);
				count_big++;
				}
			}
		}
		gem_init(gems+i,1);
		int grade_limsup=log2(i+1)+1;		//pool initialization (not good for more colours)
		pool_lenght[i]=grade_limsup-1;
		pool[i]=malloc(pool_lenght[i]*sizeof(gem));
		
		for (j=0;j<pool_lenght[i];++j) {	//pool fulling (not good for more colours)
			gem_init(pool[i]+j,j+2);
			for (k=0;k<comb_tot;k++) {
				if ((pool_big[k].grade==j+2) && gem_better(pool_big[k], pool[i][j])) {
					pool[i][j]=pool_big[k];
				}
			}
		}	
		gems[i]=pool[i][0];
		for (j=1;j<pool_lenght[i];++j) if (gem_better(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}
		
		printf("%d combined gems:\n",i+1);
		gem_print(gems+i);
	}
	
	if (scheme) {
		printf("Combining scheme:\n");
		scheme_find(gems+len-1);
		printf("\n\n");
	}
		
	if (table_output) table_print(gems, len);
	
	for (i=0;i<len;++i) free(pool[i]);		// free
}



int main(int argc, char** argv)
{
	int len;
	char opt;
	int scheme=0;
	int table_output = 0;
	while ((opt=getopt(argc,argv,"st"))!=-1) {
			switch(opt) {
				case 't':
					table_output = 1;
					break;
				case 's':
					scheme = 1;
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
	else worker(len, scheme, table_output);
	return 0;
}

