#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

typedef struct Gem_Y {
	int grade;				//using short does NOT improve time/memory usage
	double damage;		//note that float is faster but loses some precision after 2k in pure gems
	double crit;
	struct Gem_Y* father;
	struct Gem_Y* mother;
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
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->damage*p_gem->crit);
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
	else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.86*p_gem1->damage + 0.7*p_gem2->damage;
	else p_gem_combined->damage = 0.86*p_gem2->damage + 0.7*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.85*p_gem1->damage + 0.69*p_gem2->damage;
	else p_gem_combined->damage = 0.85*p_gem2->damage + 0.69*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
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

void gem_init(gem *p_gem, int grd, double damage, double crit)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit=crit;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem1.damage*gem1.crit > gem2.damage*gem2.crit);		// not really useful, but at least it puts out something
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
	for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gems[i].damage*gems[i].crit);	// not really useful, but at least it puts out something
	printf("\n");
}

int gem_has_less_grade_damage(gem gem1, gem gem2)
{
	if (gem1.grade < gem2.grade) return 1;
	else if (gem1.grade == gem2.grade && gem1.damage < gem2.damage) return 1;
	else if (gem1.grade == gem2.grade && gem1.damage == gem2.damage && gem1.crit < gem2.crit) return 1;
	else return 0;
}

void gem_sort(gem* gems, int len)
{
	if (len<=1) return;
	int pivot=0;
	int i;
	for (i=1;i<len;++i) {
		if (gem_has_less_grade_damage(gems[i],gems[pivot])) {
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

void print_parens(gem* gemf)
{
	if (gemf->father==NULL) printf("y");
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
		printf("━ g1 y\n");
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
	pool[0]=malloc(sizeof(gem));
	gem_init(gems,1,1,1);
	gem_init(pool[0],1,1,1);
	pool_length[0]=1;
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
			float lim_crit=-1;
			for (j=subpools_length[grd]-1;j>=0;--j) {
				if (pool_big[subpools_to_big_convert(subpools_length,grd,j)].crit<=lim_crit) {
					pool_big[subpools_to_big_convert(subpools_length,grd,j)].grade=0;
					broken++;
				}
				else lim_crit=pool_big[subpools_to_big_convert(subpools_length,grd,j)].crit;
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
	int output_info = 0;
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

