#ifndef _LEECH_UTILS_H
#define _LEECH_UTILS_H

struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double leech;
	struct Gem_O* father;
	struct Gem_O* mother;
};

void gem_print_O(gemO *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\n\n", p_gem->grade, p_gem->leech);
}

void gem_comb_eq_O(gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
}

void gem_comb_d1_O(gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
	else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
}

void gem_comb_gn_O(gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
	else p_gem_combined->leech = 0.9*p_gem2->leech + 0.38*p_gem1->leech;
}

void gem_combine_O (gemO *p_gem1, gemO *p_gem2, gemO *p_gem_combined)
{
	p_gem_combined->father=p_gem1;
	p_gem_combined->mother=p_gem2;
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta){
		case 0:
			gem_comb_eq_O(p_gem1, p_gem2, p_gem_combined);
			break;
		case 1:
			gem_comb_d1_O(p_gem1, p_gem2, p_gem_combined);
			break;
		case -1:
			gem_comb_d1_O(p_gem2, p_gem1, p_gem_combined);
			break;
		default:
			gem_comb_gn_O(p_gem1, p_gem2, p_gem_combined);
			break;
	}
}

int pool_from_table_O(gemO** pool, int* pool_length, int len, FILE* table)
{
	printf("\nBuilding pool...");
	rewind(table);
	int i;
	int pool_zero;
	fscanf(table, "%d\n", &pool_zero);			// get pool_zero
	if (pool_zero != pool_length[0]) {			// and check if it's right
		printf("\nWrong table type, exiting...\n");
		exit(1);
	}
	for (i=0;i<pool_length[0];++i) {				// discard value 0 gems
		fscanf(table, "%*[^\n]\n");
	}
	fscanf(table, "%*d\n\n");								// discard iteration number
	int prevmax=0;
	for (i=1;i<len;++i) {
		int eof_check=fscanf(table, "%d\n", pool_length+i);				// get pool length
		if (eof_check==-1) break;
		else {
			pool[i]=malloc(pool_length[i]*sizeof(gemO));
			int j;
			for (j=0; j<pool_length[i]; ++j) {
				int value_father, offset_father;
				int value_mother, offset_mother;
				int integrity_check=fscanf(table, "%x %x %x\n", &value_father, &offset_father, &offset_mother);
				if (integrity_check!=3) {
					printf("\nERROR: integrity check failed at byte %ld\n", ftell(table));
					printf("Your table may be corrupt, brutally exiting...\n");
					exit(1);
				}
				else {
					value_mother=i-1-value_father;
					gem_combine_O(pool[value_father]+offset_father, pool[value_mother]+offset_mother, pool[i]+j);
				}
			}
			fscanf(table, "%*d\n\n");						// discard iteration number
			prevmax++;
		}
	}
	printf(" Done\n\n");
	return prevmax;
}

void gem_init_O(gemO *p_gem, int grd, float leech)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

inline int gem_better(gemO gem1, gemO gem2)
{
	return gem1.leech>gem2.leech;
}

void print_parens_O(gemO* gemf)
{
	if (gemf->grade==0) printf("-");
	else if (gemf->father==NULL) printf("o");
	else {
		printf("(");
		print_parens_O(gemf->mother);
		printf("+");
		print_parens_O(gemf->father);
		printf(")");
	}
	return;
}

int gem_getvalue_O(gemO* p_gem)
{
	if (p_gem->grade==0) return 0;
	if (p_gem->father==NULL) return 1;
	else return gem_getvalue_O(p_gem->father)+gem_getvalue_O(p_gem->mother);
}

void print_parens_compressed_O(gemO* gemf)
{
	if (gemf->grade==0) printf("-");
	else if (gemf->father==NULL) {
		printf("o");
	}
	else if (pow(2,gemf->grade-1)==gem_getvalue_O(gemf)) {				// if gem is standard combine
		printf("%do",gemf->grade);
	}
	else {
		printf("(");
		print_parens_compressed_O(gemf->mother);
		printf("+");
		print_parens_compressed_O(gemf->father);
		printf(")");
	}
}

void fill_array_O(gemO* gemf, gemO** p_gems, int* place)
{
	if (gemf-> father != NULL) {
		fill_array_O(gemf->father, p_gems, place);
		fill_array_O(gemf->mother, p_gems, place);
	}
	int i;
	int uniq=1;
	for (i=0; i<*place; ++i) if (gemf==p_gems[i]) uniq=0;
	if (uniq) {
		gemf->grade=gemf->grade%1000;
		gemf->grade+=1000*(*place);			// mark
		p_gems[*place]=gemf;
		(*place)++;
	}
}

void print_eq_O(gemO* p_gem, int* printed_uid)
{
	if (printed_uid[p_gem->grade/1000]==1) return;
	if (gem_getvalue_O(p_gem)==1) printf("(val = 1)\t%2d = g1 o\n", p_gem->grade/1000);
	else {
		print_eq_O(p_gem->father, printed_uid);		// mother is always bigger
		print_eq_O(p_gem->mother, printed_uid);
		printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue_O(p_gem), p_gem->grade/1000, p_gem->mother->grade/1000, p_gem->father->grade/1000);
	}
	printed_uid[p_gem->grade/1000]=1;
}

void print_equations_O(gemO* gemf)
{
	if (gemf->grade==0) {
		printf("-\n");
		return;
	}
	int value=gem_getvalue_O(gemf);
	int len=2*value-1;
	gemO** p_gems=malloc(len*sizeof(gemO*));		// let's store all the gem pointers
	int place=0;
	fill_array_O(gemf, p_gems, &place);					// this array contains marked uniques only and is long "place"
	int i;
	int printed_uid[place];
	for (i=0; i<place; ++i) printed_uid[i]=0;
	print_eq_O(gemf, printed_uid);
	free(p_gems);
}

void print_tree_O(gemO* gemf, char* prefix)
{
	if (gemf->grade==0) printf("-\n");
	else if (gemf->father==NULL) {
		printf("─ g1 o\n");
	}
	else {
		printf("─%d\n",gem_getvalue_O(gemf));
		printf("%s ├",prefix);
		char string[strlen(prefix)+2];
		strcpy(string,prefix);
		strcat(string," │");
		gemO* gem1;
		gemO* gem2;
		if (gem_getvalue_O(gemf->father)>gem_getvalue_O(gemf->mother)) {
			gem1=gemf->father;
			gem2=gemf->mother;
		}
		else {
			gem2=gemf->father;
			gem1=gemf->mother;
		}
		print_tree_O(gem1, string);
		printf("%s └",prefix);
		char string2[strlen(prefix)+2];
		strcpy(string2,prefix);
		strcat(string2,"  ");
		print_tree_O(gem2, string2);
	}
}

#endif // _LEECH_UTILS_H
