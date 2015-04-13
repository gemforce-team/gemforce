#include <iostream>
#include <getopt.h>
#include <string>
#include <cstring>

using namespace std;

class gem
{
	public:
	int uid;			// needed for eq. output
	int grade;
	double damage;		// reference: yellow damage=1
	double crit;
	double leech;
	double bbound;
	gem* father;
	gem* mother;
	
	int value;
	double mana_power;
	double kill_power;
	char color;
};

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

double gem_amp_global_mana_power(gem gem1, gem amp1)
{
	return (gem1.leech+4*0.23*2.8*amp1.leech)*gem1.bbound;		// yes, 4, because of 1.5 rescaling
}

double gem_amp_global_kill_power(gem gem1, gem amp1)
{
	return (gem1.damage+6*0.28*(2.8/3.2)*amp1.damage)*gem1.bbound*(gem1.crit+4*0.23*2.8*amp1.crit)*gem1.bbound;		// yes, fraction and 4, due to 3.2 and 1.5 rescaling
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
	if (p_gem_combined->damage < p_gem1->damage) p_gem_combined->damage = p_gem1->damage;
	if (p_gem_combined->damage < p_gem2->damage) p_gem_combined->damage = p_gem2->damage;
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
	switch (color) {				//  gr dmg       cr le bl
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

int gem_getvalue(gem* p_gem)
{
	if (p_gem->grade==0) return 0;
	if (p_gem->father==NULL) return 1;
	else return gem_getvalue(p_gem->father)+gem_getvalue(p_gem->mother);
}

void gem_print(gem* p_gem) {
	switch (gem_color(p_gem)) {
		case 'y':
		printf("Yellow gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", gem_getvalue(p_gem), p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\n\n", p_gem->damage, p_gem->crit);
		break;
		case 'o':
		printf("Orange gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", gem_getvalue(p_gem), p_gem->grade);
		printf("Leech:\t%f\n\n", p_gem->leech);
		break;
		case 'b':
		printf("Black gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", gem_getvalue(p_gem), p_gem->grade);
		printf("Damage:\t%f\nBbound:\t%f\n\n", p_gem->damage, p_gem->bbound);
		break;
		case 'm':
		printf("Managem\n");
		printf("Value:\t%d\nGrade:\t%d\n", gem_getvalue(p_gem), p_gem->grade);
		printf("Leech:\t%f\nBbound:\t%f\n", p_gem->leech, p_gem->bbound);
		printf("Mana power:\t%f\n\n", p_gem->leech*p_gem->bbound);
		break;
		case 'k':
		printf("Killgem\n");
		printf("Value:\t%d\nGrade:\t%d\n", gem_getvalue(p_gem), p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\nBbound:\t%f\n", p_gem->damage, p_gem->crit, p_gem->bbound);
		printf("Kill power:\t%f\n\n", p_gem->damage*p_gem->bbound*p_gem->crit*p_gem->bbound);
		break;
		default:
		printf("Strange gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", gem_getvalue(p_gem), p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\nLeech:\t%f\nBbound:\t%f\n", p_gem->damage, p_gem->crit, p_gem->leech, p_gem->bbound);
		printf("Mana power:\t%f\n", p_gem->leech*p_gem->bbound);
		printf("Kill power:\t%f\n\n", p_gem->damage*p_gem->bbound*p_gem->crit*p_gem->bbound);
	}
}

void print_tree(gem* gemf, const char* prefix)
{
	if (gemf->father==NULL) {
		printf("─ g1 %c\n",gem_color(gemf));
	}
	else {
		printf("─%d\n",gem_getvalue(gemf));
		printf("%s ├",prefix);
		char string[strlen(prefix)+2];
		strcpy(string,prefix);
		strcat(string," │");
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
		printf("%s └",prefix);
		char string2[strlen(prefix)+2];
		strcpy(string2,prefix);
		strcat(string2,"  ");
		print_tree(gem2, string2);
	}
}

int gem_build(const char* parens, int len, gem* gems, int place)
{
	if (len==1) {
		gem_color_init(gems+place+1, parens[0]);
		return place+1;
	}
	int open_parens=0;
	int i=0;
	for (i=1; i<len-1; ++i) {
		if (parens[i]=='(') open_parens++;
		if (parens[i]==')') open_parens--;
		if (open_parens==0) break;
	}
	int father_chain=gem_build(parens+1, i, gems, place);
	int mother_chain=gem_build(parens+i+2, len-i-3, gems, father_chain);
	gem_combine(gems+father_chain, gems+mother_chain, gems+mother_chain+1);
	return mother_chain+1;
}

void print_eq(gem* p_gem, int* printed_uid)
{
	if (printed_uid[p_gem->uid]==1) return;
	if (gem_getvalue(p_gem)==1) printf("(val = 1)\t%2d = g1 %c\n", p_gem->uid, gem_color(p_gem));
	else {
		print_eq(p_gem->father, printed_uid);
		print_eq(p_gem->mother, printed_uid);
		printf("(val = %d)\t%2d = %2d + %2d\n", gem_getvalue(p_gem), p_gem->uid, p_gem->mother->uid, p_gem->father->uid);
	}
	printed_uid[p_gem->uid]=1;
}

int gem_are_equal(gem gem1, gem gem2)
{
	if (gem1.grade  != gem2.grade ) return 0;
	if (gem1.damage != gem2.damage) return 0;
	if (gem1.crit   != gem2.crit  ) return 0;
	if (gem1.leech  != gem2.leech ) return 0;
	if (gem1.bbound != gem2.bbound) return 0;
	return 1;
}

void print_equations(gem* gems, int len, int place_last)
{
	int i,j;
	for (i=0; i<len; ++i) (gems+i)->uid=-1;
	int current_uid=0;
	for (i=0; i<len; ++i) {
		if ((gems+i)->uid == -1 ) {			// never marked
			(gems+i)->uid=current_uid;			// mark current gem
			for (j=i+1; j<len; ++j) {			// and look for her friends
				if (gem_are_equal(gems[i], gems[j])) {
					(gems+j)->uid=current_uid;	// mark her friend
				}
			}
			current_uid++;
		}
	}		// all uid setted
	int printed_uid[len/4];
	for (i=0; i<len/4; ++i) printed_uid[i]=0;
	print_eq(gems+place_last, printed_uid);
}

string ieeePreParser(string recipe)
{
	for (int i = 20; i > 1; i--)
	{
		string grd_str = to_string(i);
		size_t place = recipe.find(grd_str);
		while (place != string::npos)
		{
			char color = recipe[place + grd_str.length()];
			string weakerGem = to_string(i - 1) + color;
			if (i == 2) weakerGem = color;
			recipe = recipe.replace(place, grd_str.length()+1, "(" + weakerGem + "+" + weakerGem + ")");
			place = recipe.find(grd_str);
		}
	}
	return recipe;
}

int main(int argc, char** argv)
{
	char opt;
	int output_tree=0;
	int output_eq = 0;
	string parens_amps="";
	while ((opt=getopt(argc,argv,"htea:"))!=-1) {
		switch(opt) {
			case 'h':
				printf("htea:");
			return 0;
			case 't':
				output_tree = 1;
			break;
			case 'e':
				output_eq = 1;
			break;
			case 'a':
				parens_amps = ieeePreParser(optarg);
			break;
			case '?':
				return 1;
			default:
			break;
		}
	}
	string parens;
	if (optind+1==argc) {		// get input
		parens = ieeePreParser(argv[optind]);
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	int len = parens.length();
	if (len<1) printf("Improper gem recipe\n");
	else {
		int value=(len+3)/4;			// gem
		gem gems[2*value-1];
		int place=gem_build(parens.c_str(), len, gems, -1);
		printf("\nMain gem:\n");
		gem_print(gems+place);
		if (output_tree) {
			printf("Tree:\n");
			print_tree(gems+place, "");
			printf("\n");
		}
		if (output_eq) {
			printf("Equations:\n");
			print_equations(gems, 2*value-1, place);
			printf("\n");
		}
		int len_amps = parens_amps.length();
		if (len_amps>0) {
			value=(len_amps+3)/4;			// amps
			gem amps[2*value-1];
			int place_amps=gem_build(parens_amps.c_str(), len_amps, amps, -1);
			printf("Amplifier:\n");
			gem_print(amps+place_amps);
			if (output_tree) {
				printf("Tree:\n");
				print_tree(amps+place_amps, "");
				printf("\n");
			}
			if (output_eq) {
				printf("Equations:\n");
				print_equations(amps, 2*value-1, place_amps);
				printf("\n");
			}
			printf("Global mana power (resc.):\t%f\n", gem_amp_global_mana_power(gems[place], amps[place_amps]));
			printf("Global kill power (resc.):\t%f\n\n", gem_amp_global_kill_power(gems[place], amps[place_amps]));
		}
	}
	return 0;
}
