#ifndef _CRIT_UTILS_H
#define _CRIT_UTILS_H

struct Gem_Y {
	int grade;				//using short does NOT improve time/memory usage
	float damage;
	float crit;
	struct Gem_Y* father;
	struct Gem_Y* mother;
};

// --------------------
// Common gem interface
// --------------------

#include "gem_stats.h"

void gem_print_Y(gemY *p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->damage*p_gem->crit);
}

// -----------------
// Combining section
// -----------------

void gem_comb_eq_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_EQ_1*p_gem1->damage + DAMAGE_EQ_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_EQ_1*p_gem2->damage + DAMAGE_EQ_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_EQ_1*p_gem1->crit + CRIT_EQ_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_EQ_1*p_gem2->crit + CRIT_EQ_2*p_gem1->crit;
}

void gem_comb_d1_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_D1_1*p_gem1->damage + DAMAGE_D1_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_D1_1*p_gem2->damage + DAMAGE_D1_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_D1_1*p_gem1->crit + CRIT_D1_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_D1_1*p_gem2->crit + CRIT_D1_2*p_gem1->crit;
}

void gem_comb_gn_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = DAMAGE_GN_1*p_gem1->damage + DAMAGE_GN_2*p_gem2->damage;
	else p_gem_combined->damage = DAMAGE_GN_1*p_gem2->damage + DAMAGE_GN_2*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = CRIT_GN_1*p_gem1->crit + CRIT_GN_2*p_gem2->crit;
	else p_gem_combined->crit = CRIT_GN_1*p_gem2->crit + CRIT_GN_2*p_gem1->crit;
}

void gem_combine_Y (gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->father=p_gem1;
	p_gem_combined->mother=p_gem2;
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta){
		case 0:
			gem_comb_eq_Y(p_gem1, p_gem2, p_gem_combined);
			break;
		case 1:
			gem_comb_d1_Y(p_gem1, p_gem2, p_gem_combined);
			break;
		case -1:
			gem_comb_d1_Y(p_gem2, p_gem1, p_gem_combined);
			break;
		default:
			gem_comb_gn_Y(p_gem1, p_gem2, p_gem_combined);
			break;
	}
	if (p_gem_combined->damage < p_gem1->damage) p_gem_combined->damage = p_gem1->damage;
	if (p_gem_combined->damage < p_gem2->damage) p_gem_combined->damage = p_gem2->damage;
}

void gem_init_Y(gemY *p_gem, int grd, float damage, float crit)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit  =crit;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

// ------------------------
// Redefine pool_from_table
// ------------------------

#include "gfon.h"

int pool_from_table_Y(gemY** pool, int* pool_length, int len, FILE* table)
{
	printf("\nBuilding pool..."); fflush(stdout);
	rewind(table);

	int pool_zero;
	int check;
	check = fscanf(table, "%d\n", &pool_zero);   // get pool_zero
	if (check!=1) exit_on_corruption(ftell(table));
	if (pool_zero != pool_length[0]) {               // and check if it's right
		printf("\nWrong table type, exiting...\n");
		return -1;      // the program will then exit gracefully
	}

	for (int i=0;i<pool_length[0];++i) {             // discard value 0 gems
		int check = fscanf(table, "%*[^\n]\n");
		if (check!=0) exit_on_corruption(ftell(table));
	}

	int iter;
	check = fscanf(table, "%d\n\n", &iter);      // check iteration number
	if (check!=1) exit_on_corruption(ftell(table));
	if (iter!=0) exit_on_corruption(ftell(table));

	int prevmax=0;
	for (int i=1;i<len;++i) {
		int eof_check=fscanf(table, "%d\n", pool_length+i);      // get pool length
		if (eof_check==EOF) break;
		else {
			pool[i]=malloc(pool_length[i]*sizeof(gemY));
			int j;
			for (j=0; j<pool_length[i]; ++j) {
				char b1[9], b2[9], b3[9];
				int nscan = fscanf(table, "%8s %8s %8s\n", b1, b2, b3);
				int value_father;
				int offset_father;
				int offset_mother;
				int check = (nscan == 3);
				check &= fscan64(b1, &value_father);
				check &= fscan64(b2, &offset_father);
				check &= fscan64(b3, &offset_mother);
				if (!check) exit_on_corruption(ftell(table));
				else {
					int value_mother=i-1-value_father;
					gem_combine_Y(pool[value_father]+offset_father, pool[value_mother]+offset_mother, pool[i]+j);
				}
			}
			int iteration_check;
			int check = fscanf(table, "%d\n\n", &iteration_check);    // check iteration number
			if (check!=1 || iteration_check!=i) exit_on_corruption(ftell(table));
			prevmax++;
		}
	}
	printf(" %d blocks read\n\n", prevmax+1);
	return prevmax;
}

// ---------------
// Sorting section
// ---------------

inline int gem_has_less_damage_crit(gemY gem1, gemY gem2)
{
	if (gem1.damage < gem2.damage) return 1;
	else if (gem1.damage == gem2.damage && gem1.crit < gem2.crit) return 1;
	else return 0;
}

void ins_sort_Y (gemY* gems, int len)
{
	int i,j;
	gemY element;
	for (i=1; i<len; i++) {
		element=gems[i];
		for (j=i; j>0 && gem_has_less_damage_crit(element, gems[j-1]); j--) {
			gems[j]=gems[j-1];
		}
		gems[j]=element;
	}
}

void quick_sort_Y (gemY* gems, int len)
{
	if (len > 20)  {
		gemY pivot = gems[len/2];
		gemY* beg = gems;
		gemY* end = gems+len-1;
		while (beg <= end) {
			while (gem_has_less_damage_crit(*beg, pivot)) {
				beg++;
			}
			while (gem_has_less_damage_crit(pivot,*end)) {
				end--;
			}
			if (beg <= end) {
				gemY temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		if (end-gems+1 < gems-beg+len) {		// sort smaller first
			quick_sort_Y(gems, end-gems+1);
			quick_sort_Y(beg, gems-beg+len);
		}
		else {
			quick_sort_Y(beg, gems-beg+len);
			quick_sort_Y(gems, end-gems+1);
		}
	}
}

void gem_sort_Y (gemY* gems, int len)
{
	quick_sort_Y (gems, len);		// partially sort
	ins_sort_Y (gems, len);			// finish the nearly sorted array
}

// ---------------------------
// Redefine printing functions
// ---------------------------

void print_parens_Y(gemY* gemf)
{
	if (gemf->grade==0) printf("-");
	else if (gemf->father==NULL) printf("y");
	else {
		printf("(");
		print_parens_Y(gemf->mother);
		printf("+");
		print_parens_Y(gemf->father);
		printf(")");
	}
	return;
}

int gem_getvalue_Y(gemY* p_gem)
{
	if (p_gem->grade==0) return 0;
	if (p_gem->father==NULL) return 1;
	else return gem_getvalue_Y(p_gem->father)+gem_getvalue_Y(p_gem->mother);
}

void print_parens_compressed_Y(gemY* gemf)
{
	if (gemf->grade==0) printf("-");
	else if (gemf->father==NULL) {
		printf("y");
	}
	else if (pow(2,gemf->grade-1)==gem_getvalue_Y(gemf)) {				// if gem is standard combine
		printf("%dy",gemf->grade);
	}
	else {
		printf("(");
		print_parens_compressed_Y(gemf->mother);
		printf("+");
		print_parens_compressed_Y(gemf->father);
		printf(")");
	}
}

void fill_array_Y(gemY* gemf, gemY** p_gems, int* place)
{
	if (gemf-> father != NULL) {
		fill_array_Y(gemf->father, p_gems, place);
		fill_array_Y(gemf->mother, p_gems, place);
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

void print_eq_Y(gemY* p_gem, int* printed_uid)
{
	if (printed_uid[p_gem->grade/1000]==1) return;
	if (gem_getvalue_Y(p_gem)==1) printf("(val = 1)\t%2d = g1 y\n", p_gem->grade/1000);
	else {
		print_eq_Y(p_gem->father, printed_uid);		// mother is always bigger
		print_eq_Y(p_gem->mother, printed_uid);
		printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue_Y(p_gem), p_gem->grade/1000, p_gem->mother->grade/1000, p_gem->father->grade/1000);
	}
	printed_uid[p_gem->grade/1000]=1;
}

void print_equations_Y(gemY* gemf)
{
	if (gemf->grade==0) {
		printf("-\n");
		return;
	}
	int value=gem_getvalue_Y(gemf);
	int len=2*value-1;
	gemY** p_gems=malloc(len*sizeof(gemY*));		// let's store all the gem pointers
	int place=0;
	fill_array_Y(gemf, p_gems, &place);					// this array contains marked uniques only and is long "place"
	int i;
	int printed_uid[place];
	for (i=0; i<place; ++i) printed_uid[i]=0;
	print_eq_Y(gemf, printed_uid);
	free(p_gems);
}

void print_tree_Y(gemY* gemf, const char* prefix)
{
	if (gemf->grade==0) printf("-\n");
	else if (gemf->father==NULL) {
		printf("─ g1 y\n");
	}
	else {
		printf("─%d\n",gem_getvalue_Y(gemf));
		printf("%s ├",prefix);
		char string[strlen(prefix)+5];   // 1 space, 1 unicode bar and and the null term are 5 extra chars
		strcpy(string,prefix);
		strcat(string," │");
		print_tree_Y(gemf->mother, string);
		
		printf("%s └",prefix);
		char string2[strlen(prefix)+3];  // 2 spaces and the null term are 3 extra chars
		strcpy(string2,prefix);
		strcat(string2,"  ");
		print_tree_Y(gemf->father, string2);
	}
}

#endif // _CRIT_UTILS_H
