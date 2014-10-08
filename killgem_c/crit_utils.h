#ifndef _CRIT_UTILS_H
#define _CRIT_UTILS_H

struct Gem_Y {
	int grade;				//using short does NOT improve time/memory usage
	float damage;
	float crit;
	struct Gem_Y* father;
	struct Gem_Y* mother;
};

void gem_print_Y(gemY *p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nPower:\t%f\n\n",
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->damage*p_gem->crit);
}

void gem_comb_eq_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
	else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
}

void gem_comb_d1_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)     //bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.86*p_gem1->damage + 0.7*p_gem2->damage;
	else p_gem_combined->damage = 0.86*p_gem2->damage + 0.7*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
}

void gem_comb_gn_Y(gemY *p_gem1, gemY *p_gem2, gemY *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.85*p_gem1->damage + 0.69*p_gem2->damage;
	else p_gem_combined->damage = 0.85*p_gem2->damage + 0.69*p_gem1->damage;
	if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.44*p_gem2->crit;
	else p_gem_combined->crit = 0.88*p_gem2->crit + 0.44*p_gem1->crit;
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
}

void gem_init_Y(gemY *p_gem, int grd, float damage, float crit)
{
	p_gem->grade =grd;
	p_gem->damage=damage;
	p_gem->crit=crit;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

//int gem_better(gemO gem1, gemO gem2)
//{
	//return gem1.leech>=gem2.leech;
//}

int gem_has_less_grade_damage(gemY gem1, gemY gem2)
{
	if (gem1.grade < gem2.grade) return 1;
	else if (gem1.grade == gem2.grade && gem1.damage < gem2.damage) return 1;
	else if (gem1.grade == gem2.grade && gem1.damage == gem2.damage && gem1.crit < gem2.crit) return 1;
	else return 0;
}

void gem_sort(gemY* gems, int len)
{
	if (len<=1) return;
	int pivot=0;
	int i;
	for (i=1;i<len;++i) {
		if (gem_has_less_grade_damage(gems[i],gems[pivot])) {
			gemY temp=gems[pivot];
			gems[pivot]=gems[i];
			gems[i]=gems[pivot+1];
			gems[pivot+1]=temp;
			pivot++;
		}
	}
	gem_sort(gems,pivot);
	gem_sort(gems+1+pivot,len-pivot-1);
}

void print_parens_Y(gemY* gemf)
{
	if (gemf->father==NULL) printf("y");
	else {
		printf("(");
		print_parens_Y(gemf->father);
		printf("+");
		print_parens_Y(gemf->mother);
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

void print_tree_Y(gemY* gemf, char* prefix)
{
	if (gemf->father==NULL) {
		printf("━ g1 y\n");
	}
	else {
		printf("━%d\n",gem_getvalue_Y(gemf));
		printf("%s ┣",prefix);
		char string[strlen(prefix)+2];
		strcpy(string,prefix);
		strcat(string," ┃");
		gemY* gem1;
		gemY* gem2;
		if (gem_getvalue_Y(gemf->father)>gem_getvalue_Y(gemf->mother)) {
			gem1=gemf->father;
			gem2=gemf->mother;
		}
		else {
			gem2=gemf->father;
			gem1=gemf->mother;
		}
		print_tree_Y(gem1, string);
		printf("%s ┗",prefix);
		char string2[strlen(prefix)+2];
		strcpy(string2,prefix);
		strcat(string2,"  ");
		print_tree_Y(gem2, string2);
	}
}

#endif // _CRIT_UTILS_H
