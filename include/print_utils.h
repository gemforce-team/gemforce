#ifndef _PRINT_UTILS_H
#define _PRINT_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gem_utils.h"

void print_parens(gem* gemf)
{
	if (gemf->father==NULL) printf("%c", gem_color(gemf));
	else {
		printf("(");
		print_parens(gemf->mother);
		printf("+");
		print_parens(gemf->father);
		printf(")");
	}
	return;
}

int monocolor_ancestors(gem* gemf)
{
	if (gemf->father==NULL) return 1;
	else if (gem_color(gemf->father)!=gem_color(gemf->mother)) return 0;
	else return monocolor_ancestors(gemf->mother) & monocolor_ancestors(gemf->father);
}

void print_parens_compressed(gem* gemf)
{
	if (gemf->father==NULL) printf("%c", gem_color(gemf));
	else if (monocolor_ancestors(gemf)						// if gem is uniform combination (g1 are already done)
	&& 1 << (gemf->grade-1) == gem_getvalue(gemf)) {		// and is standard combine
		printf("%d%c",gemf->grade,gem_color(gemf));
	}
	else {
		printf("(");
		print_parens_compressed(gemf->mother);
		printf("+");
		print_parens_compressed(gemf->father);
		printf(")");
	}
}

void fill_uniques_array(gem* gemf, gem** p_gems, int* uniques)
{
	for (int i=0; i<*uniques; ++i)
		if (gemf==p_gems[i]) return;
	
	if (gemf->father != NULL) {
		fill_uniques_array(gemf->father, p_gems, uniques);
		fill_uniques_array(gemf->mother, p_gems, uniques);
	}
	
	p_gems[*uniques]=gemf;
	(*uniques)++;
}

void print_equations(gem* gemf)
{
	// fill
	int value=gem_getvalue(gemf);
	int len=2*value-1;
	gem** p_gems = (gem**)malloc(len*sizeof(gem*));		// stores all the gem pointers
	int uniques = 0;
	fill_uniques_array(gemf, p_gems, &uniques);			// this array contains uniques only and is long `uniques`
	
	// mark
	int orig_grades[uniques];		// stores all the original gem grades
	for (int i = 0; i < uniques; i++) {
		gem* p_gem = p_gems[i];
		orig_grades[i] = p_gem->grade;
		p_gem->grade = i;
	}
	
	// print
	for (int i = 0; i < uniques; i++) {
		gem* p_gem = p_gems[i];
		if (p_gem->father == NULL)
			printf("(val = 1)\t%2d = g1 %c\n", p_gem->grade, gem_color(p_gem));
		else
			printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue(p_gem), p_gem->grade, p_gem->mother->grade, p_gem->father->grade);
	}
	
	// clean
	for (int i = 0; i < uniques; i++) {
		p_gems[i]->grade = orig_grades[i];
	}
	free(p_gems);
}

void print_tree(gem* gemf, const char* prefix)
{
	if (gemf->father==NULL) {
		printf("─ g1 %c\n",gem_color(gemf));
	}
	else {
		printf("─%d\n",gem_getvalue(gemf));
		printf("%s ├",prefix);
		char string1[strlen(prefix)+5];  // 1 space, 1 unicode bar and and the null term are 5 extra chars
		sprintf(string1, "%s │", prefix);
		print_tree(gemf->mother, string1);
		
		printf("%s └",prefix);
		char string2[strlen(prefix)+3];  // 2 spaces and the null term are 3 extra chars
		sprintf(string2, "%s  ", prefix);
		print_tree(gemf->father, string2);
	}
}

void print_table(gem* gems, int len)
{
	printf("# Gems\tPower\t\tGrowth\n");
	printf("1\t%f\t%f\n", gem_power(gems[0]), log(gem_power(gems[0])));
	int i;
	for (i=1; i<len; i++)
		printf("%d\t%f\t%f\n", i+1, gem_power(gems[i]), log(gem_power(gems[i]))/log(i+1));
	printf("\n");
}

#endif // _PRINT_UTILS_H
