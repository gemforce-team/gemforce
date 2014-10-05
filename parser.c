#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

typedef struct Gem_global {
  int grade;
  double damage;				// reference: yellow damage=1
  double crit;
  double leech;
  double bbound;
  struct Gem_global* father;
  struct Gem_global* mother;
} gem;

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
  p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
  else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
  if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
  else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
  if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
  else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
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
  if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
  else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
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

void gem_init(gem *p_gem, int grd, double damage, double crit, double leech, double bbound)
{
  p_gem->grade =grd;
  p_gem->damage=damage;
  p_gem->crit  =crit;
  p_gem->leech =leech;
  p_gem->bbound=bbound;
  p_gem->father=NULL;
  p_gem->mother=NULL;
}

void gem_color_init(gem* p_gem, char color)
{
	switch (color) {				//  gr dm        cr le bl
		case 'y': gem_init(p_gem, 1, 1       , 1, 0, 0);
		break;
		case 'o': gem_init(p_gem, 1, 0.727542, 0, 1, 0);
		break;
		case 'b': gem_init(p_gem, 1, 1.186168, 0, 0, 1);
		break;
		case 'm': gem_init(p_gem, 1, 1       , 0, 1, 1);
		break;
		case 'k': gem_init(p_gem, 1, 1       , 1, 0, 1);
		break;
		default:  gem_init(p_gem, 0, 0       , 0, 0, 0);
	}
}

char gem_color(gem* p_gem)
{
	int info=0;
	if (p_gem->crit!=0) info+=4;
	if (p_gem->leech!=0) info+=2;
	if (p_gem->bbound!=0) info+=1;
	switch (info) {
		case  0: return '0';
		case  1: return 'b';
		case  2: return 'o';
		case  3: return 'm';
		case  4: return 'y';
		case  5: return 'k';
		default: return 'x';
	}
}

void gem_print(gem* p_gem) {
	switch (gem_color(p_gem)) {
		case 'y':
		printf("Yellow gem\n");
		printf("Grade:\t%d\n", p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\n\n", p_gem->damage, p_gem->crit);
		break;
		case 'o':
		printf("Orange gem\n");
		printf("Grade:\t%d\n", p_gem->grade);
		printf("Leech:\t%f\n\n", p_gem->leech);
		break;
		case 'b':
		printf("Black gem\n");
		printf("Grade:\t%d\n", p_gem->grade);
		printf("Damage:\t%f\nBbound:\t%f\n\n", p_gem->damage, p_gem->bbound);
		break;
		case 'm':
		printf("Managem\n");
		printf("Grade:\t%d\n", p_gem->grade);
		printf("Leech:\t%f\nBbound:\t%f\n", p_gem->leech, p_gem->bbound);
		printf("Mana power:\t%f\n\n", p_gem->leech*p_gem->bbound);
		break;
		case 'k':
		printf("Killgem\n");
		printf("Grade:\t%d\n", p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\nBbound:\t%f\n", p_gem->damage, p_gem->crit, p_gem->bbound);
		printf("Kill power:\t%f\n\n", p_gem->damage*p_gem->bbound*p_gem->crit*p_gem->bbound);
		break;
		default:
		printf("Strange gem\n");
		printf("Grade:\t%d\n", p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\nLeech:\t%f\nBbound:\t%f\n", p_gem->damage, p_gem->crit, p_gem->leech, p_gem->bbound);
		printf("Mana power:\t%f\n", p_gem->leech*p_gem->bbound);
		printf("Kill power:\t%f\n\n", p_gem->damage*p_gem->bbound*p_gem->crit*p_gem->bbound);
	}
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

gem gem_build(char* parens, int len)
{
	if (len==1) {
		gem temp;
		gem_color_init(&temp, parens[0]);
		return temp;
	}
	int open_parens=0;
	int i=0;
	for (i=1; i<len-1; ++i) {
		if (parens[i]=='(') open_parens++;
		if (parens[i]==')') open_parens--;
		if (open_parens==0) break;
	}
	gem father=gem_build(parens+1, i);
	gem mother=gem_build(parens+i+2, len-i-3);
	gem temp;
	gem_combine(&father, &mother, &temp);
	return temp;
}

int main(int argc, char** argv)
{
	char opt;
	int output_tree=0;
	while ((opt=getopt(argc,argv,"t"))!=-1) {
		switch(opt) {
			case 't':
				output_tree = 1;
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	char* parens;
	int len=0;
	if (optind+1==argc) {		// get input
		while (argv[optind][len] != '\0') len++;
		parens=malloc((len+1)*sizeof(char));
		strcpy(parens, argv[optind]);
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1) printf("Improper gem recipe\n");
	else {
		gem result=gem_build(parens, len);
		gem_print(&result);
		if (output_tree) {
			printf("Tree:\n");
			print_tree(&result, "");
			printf("\n");
		}
	}
	return 0;
}
