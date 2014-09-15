#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>


typedef struct Gem {
	int grade;			//using short does NOT improve time/memory usage
	float leech;		//float is 10% faster than double and loses nothing
	float bbound;
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
	printf("Grade:\t%d\nLeech:\t%f\nBbound:\t%f\nPower:\t%f\n\n", p_gem->grade, p_gem->leech, p_gem->bbound, p_gem->leech*p_gem->bbound);
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.78*p_gem1->bbound + 0.31*p_gem2->bbound;
	else p_gem_combined->bbound = 0.78*p_gem2->bbound + 0.31*p_gem1->bbound;	
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
	else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
	if (p_gem1->bbound > p_gem2->bbound) p_gem_combined->bbound = 0.79*p_gem1->bbound + 0.29*p_gem2->bbound;
	else p_gem_combined->bbound = 0.79*p_gem2->bbound + 0.29*p_gem1->bbound;	
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
	else p_gem_combined->leech = 0.9*p_gem2->leech + 0.38*p_gem1->leech;
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

void gem_init_orange(gem *p_gem, int grd)
{
	p_gem->grade=grd;
	p_gem->leech= pow(1.38, (double)(grd-1));
	p_gem->bbound=0;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

void gem_init_black(gem *p_gem, int grd)
{
	p_gem->grade=grd;
	p_gem->leech= 0;
	p_gem->bbound=pow(1.09, (double)(grd-1));
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem1.leech*gem1.bbound > gem2.leech*gem2.bbound);		// optimization at infinity hits (hit lv infinity)
}																	// the *0.7 for dual is not required because they'll all be dual

char gem_colour(gem* p_gem)
{
	if (p_gem->leech==0) return 'b';
	else return 'o';
}

int gem_is_minor(gem gem1, gem gem2)
{
	if (gem1.grade < gem2.grade) return 1;
	else if (gem1.grade == gem2.grade && gem1.leech < gem2.leech) return 1;
	else return 0;
}

void gem_sort(gem* gems, int len) 
{
	if (len<=1) return;
	int pivot=0;
	int i;
	for (i=1;i<len;++i) {
		if (gem_is_minor(gems[i],gems[pivot])) {
			gem temp=gems[pivot];
			gems[pivot]=gems[i];
			gems[i]=gems[pivot+1];
			gems[pivot+1]=temp;
			pivot++;
		}
	}
	gem_sort(gems,pivot);
	gem_sort(gems+1+pivot,len-pivot-1);
}

int subpools_to_big_convert(int* subpools_length, int grd, int index)
{
	int result=0;
	int i;
	for (i=0;i<grd;++i) result+=subpools_length[i];
	result+=index;
	return result;
}

void print_table(gem* gems, int len)
{
	printf("# Gems\tPower\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gems[i].leech*gems[i].bbound);
	printf("\n");
}

void print_parens(gem* gemf)
{
	if (gemf->father==NULL) printf("%c",gem_colour(gemf));
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
		printf("━ g1 %c\n",gem_colour(gemf));
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

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	gem_init_orange(gems,1);
	gem_init_orange(pool[0],1);
	gem_init_black(pool[0]+1,1);
	pool_length[0]=2;
	gem_print(gems);
	
	for (i=1; i<len; ++i) {	
		int j,k,h;
		int count_big=0;
		int eoc=(i+1)/2;		//end of combining
		int comb_tot=0;
		for (j=0; j<eoc; ++j) comb_tot+=pool_length[j]*pool_length[i-j-1];
		gem* pool_big = malloc(comb_tot*sizeof(gem));		//a very big array needs to be in the heap
				
		for (j=0;j<eoc;++j) {								// pool_big gets filled of candidate gems
			for (k=0; k< pool_length[j]; ++k) {
				for (h=0; h< pool_length[i-1-j]; ++h) {
				gem_combine(pool[j]+k, pool[i-1-j]+h, pool_big+count_big);
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
			float lim_bbound=-1;				// thank you Enrico for this great algorithm
			for (j=subpools_length[grd]-1;j>=0;--j) {
				if (pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound<=lim_bbound) {
					pool_big[subpools_to_big_convert(subpools_length,grd,j)].grade=0;
					broken++;
				}
				else lim_bbound=pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound;
			}
		}										// all unnecessary gems destroyed
		pool_length[i]=comb_tot-broken;		
		pool[i]=malloc(pool_length[i]*sizeof(gem));			// pool init via broken
		
		int place=0;
		for (j=0;j<comb_tot;++j) {		// copying to pool
			if (pool_big[j].grade!=0) {
				pool[i][place]=pool_big[j];
				place++;
			}	
		}
		free(pool_big);		// free
		gems[i]=pool[i][0];						// choosing gem (criteria moved to more_power def)
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
	else worker(len, output_parens, output_tree, output_table, output_debug);
	return 0;
}

