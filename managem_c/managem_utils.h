#ifndef _MANAGEM_UTILS_H
#define _MANAGEM_UTILS_H

//const int ACC=1000; 			// EXPLICITELY PUT IT IN THE OTHER FILE

/* Info: to go from low to high accuracy: change gems_sort in gems_sort_exact and
 * if ((int)(ACC*pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound)<=(int)(ACC*lim_bbound)) {
 * in
 * if (pool_big[subpools_to_big_convert(subpools_length,grd,j)].bbound<=lim_bbound) {
 */

struct Gem_OB_exact {
	int grade;
	double leech;
	double bbound;
	struct Gem_OB_exact* father;
	struct Gem_OB_exact* mother;
};

struct Gem_OB_appr {
	short grade;
	float leech;
	float bbound;
	struct Gem_OB_appr* father;
	struct Gem_OB_appr* mother;
};

// remember to define the right gem in your file

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem1.leech*gem1.bbound > gem2.leech*gem2.bbound);   // optimization at infinity hits (hit lv infinity)
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

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)     //bigger is always gem1
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

void gem_init(gem *p_gem, int grd, double leech, double bbound)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->bbound=bbound;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_less_equal(gem gem1, gem gem2)
{
	if ((int)(gem1.leech*ACC) != (int)(gem2.leech*ACC))
	return gem1.leech<gem2.leech;
	return gem1.bbound<gem2.bbound;
}

void gem_sort_old(gem* gems, int len)
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
	gem_sort_old(gems,pivot);
	gem_sort_old(gems+1+pivot,len-pivot-1);
}

void gem_sort (gem* gems, int len) {
	if (len < 2) return;
	gem pivot = gems[len/2];
	gem* beg = gems;
	gem* end = gems+len-1;
	while (beg <= end) {
		if (gem_less_equal(*beg, pivot)) {
			beg++;
		}
		else if (gem_less_equal(pivot,*end)) {
			end--;
		}
		else {
			gem temp = *beg;
			*beg = *end;
			*end = temp;
			beg++;
			end--;
		}
	}
	gem_sort(gems, end-gems+1);
	gem_sort(beg, gems-beg+len);
}

void print_table(gem* gems, int len)
{
	printf("# Gems\tPower\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gems[i].leech*gems[i].bbound);
	printf("\n");
}

char gem_color(gem* p_gem)
{
	if (p_gem->leech==0) return 'b';
	if (p_gem->bbound==0) return 'o';
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
	if (p_gem->grade==0) return 0;
	if (p_gem->father==NULL) return 1;
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

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info, int size);

int get_opts_and_call_worker(int argc, char** argv)
{
	int len;
	char opt;
	int output_parens=0;
	int output_tree=0;
	int output_table = 0;
	int output_debug=0;
	int output_info=0;
	int size=0;				// worker or user must initialize it
	
	while ((opt=getopt(argc,argv,"ptedis:"))!=-1) {
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
	else worker(len, output_parens, output_tree, output_table, output_debug, output_info, size);
	return 0;
}


#endif // _MANAGEM_UTILS_H
