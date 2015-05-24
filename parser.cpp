#include <iostream>
#include <fstream>
#include <getopt.h>
#include <string>
#include <cstring>
#include <vector>

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
	bool red;
	gem* father;
	gem* mother;
	
	int value;
	double mana_power;
	double kill_power;
	char color;
	
	gem(){}
	
	gem(int grd, double damage, double crit, double leech, double bbound, bool red=0, gem* father=nullptr, gem* mother=nullptr):
	grade(grd), damage(damage), crit(crit), leech(leech), bbound(bbound), red(red), father(father), mother(mother) {}
	
	gem(const char color)
	{
		switch (color)
		{                     // gr dmg       cr le bl r
			case 'y': *this = gem(1, 1       , 1, 0, 0, 0);
			break;
			case 'o': *this = gem(1, 0.727273, 0, 1, 0, 0);
			break;
			case 'b': *this = gem(1, 1.186168, 0, 0, 1, 0);
			break;
			case 'r': *this = gem(1, 0.909091, 0, 0, 0, 1);
			break;
			case 'm': *this = gem(1, 1       , 0, 1, 1, 0);
			break;
			case 'k': *this = gem(1, 1       , 1, 0, 1, 0);
			break;
			default:  *this = gem(0, 0       , 0, 0, 0, 0);
		}
		this->color=color;
		this->value=1;
	}
	
	bool operator==(const gem& gem2)
	{
		if (grade  != gem2.grade ) return 0;
		if (damage != gem2.damage) return 0;
		if (crit   != gem2.crit  ) return 0;
		if (leech  != gem2.leech ) return 0;
		if (bbound != gem2.bbound) return 0;
		if (red    != gem2.red   ) return 0;
		return 1;
	}
	
	char get_color()
	{
		int info=0;
		if (this->crit!=0) info+=4;
		if (this->leech!=0) info+=2;
		if (this->bbound!=0) info+=1;
		switch (info) {
			case  0: return this->red?'r':'0';
			case  1: return 'b';
			case  2: return 'o';
			case  3: return 'm';
			case  4: return 'y';
			case  5: return 'k';
			default: return 'x';
		}
	}
	int getvalue() {
		return this->value;
	}
};

char gem_color(gem* gemf)
{
	return gemf->color;
}

#include "gem_utils.h"

double gem_amp_mana_power(gem gem1, gem amp1, double leech_ratio)
{
	return gem1.bbound*(gem1.leech+leech_ratio*amp1.leech);
}

double gem_amp_kill_power(gem gem1, gem amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*gem1.bbound*(gem1.crit+crit_ratio*amp1.crit)*gem1.bbound;
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
	p_gem_combined->red = p_gem1->red || p_gem2->red;
	p_gem_combined->color = p_gem_combined->get_color();
	p_gem_combined->value = p_gem1->getvalue() + p_gem2->getvalue();
}

int monocolor_ancestors(gem* gemf)
{
	if (gemf->father==NULL) return 1;
	else if (gem_color(gemf->father)!=gem_color(gemf->mother)) return 0;
	else return monocolor_ancestors(gemf->mother) & monocolor_ancestors(gemf->father);
}

void print_parens_compressed(gem* gemf)
{
	if (gemf->father==NULL) printf("%c",gem_color(gemf));
	else if (monocolor_ancestors(gemf)							// if gem is uniform combination (g1 are already done)
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

void fill_array(gem* gemf, gem** p_gems, int* place)
{
	if (gemf-> father != NULL) {
		fill_array(gemf->father, p_gems, place);
		fill_array(gemf->mother, p_gems, place);
	}
	int i;
	int uniq=1;
	for (i=0; i<*place; ++i) if (gemf==p_gems[i]) uniq=0;
	if (uniq) {
		gemf->uid=*place;			// mark
		p_gems[*place]=gemf;
		(*place)++;
	}
}

void print_eq(gem* p_gem, int* printed_uid)
{
	if (printed_uid[p_gem->uid]==1) return;
	if (p_gem->getvalue()==1) printf("(val = 1)\t%2d = g1 %c\n", p_gem->uid, gem_color(p_gem));
	else {
		print_eq(p_gem->father, printed_uid);		// mother is always bigger
		print_eq(p_gem->mother, printed_uid);
		printf("(val = %d)\t%2d = %2d + %2d\n", p_gem->getvalue(), p_gem->uid, p_gem->mother->uid, p_gem->father->uid);
	}
	printed_uid[p_gem->uid]=1;
}

void print_equations(gem* gemf)
{
	int value=gemf->getvalue();
	int len=2*value-1;
	gem** p_gems = (gem**)malloc(len*sizeof(gem*));		// let's store all the gem pointers
	int place=0;
	fill_array(gemf, p_gems, &place);			// this array contains marked uniques only and is long "place"
	int i;
	int printed_uid[place];
	for (i=0; i<place; ++i) printed_uid[i]=0;
	print_eq(gemf, printed_uid);
	free(p_gems);
}

void print_tree(gem* gemf, const char* prefix)
{
	if (gemf->father==NULL) {
		printf("─ g1 %c\n",gem_color(gemf));
	}
	else {
		printf("─%d\n",gemf->getvalue());
		printf("%s ├",prefix);
		char string1[strlen(prefix)+5];   // 1 space, 1 unicode bar and and the null term are 5 extra chars
		strcpy(string1,prefix);
		strcat(string1," │");
		print_tree(gemf->mother, string1);
		
		printf("%s └",prefix);
		char string2[strlen(prefix)+3];  // 2 spaces and the null term are 3 extra chars
		strcpy(string2,prefix);
		strcat(string2,"  ");
		print_tree(gemf->father, string2);
	}
}

void gem_print(gem* p_gem) {
	switch (p_gem->color) {
		case 'y':
		printf("Yellow gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", p_gem->getvalue(), p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\n\n", p_gem->damage, p_gem->crit);
		break;
		case 'o':
		printf("Orange gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", p_gem->getvalue(), p_gem->grade);
		printf("Leech:\t%f\n\n", p_gem->leech);
		break;
		case 'b':
		printf("Black gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", p_gem->getvalue(), p_gem->grade);
		printf("Damage:\t%f\nBbound:\t%f\n\n", p_gem->damage, p_gem->bbound);
		break;
		case 'r':
		printf("Red gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", p_gem->getvalue(), p_gem->grade);
		printf("Damage:\t%f\nRed:\t%d\n\n", p_gem->damage, p_gem->red);
		break;
		case 'm':
		printf("Managem\n");
		printf("Value:\t%d\nGrade:\t%d\n", p_gem->getvalue(), p_gem->grade);
		printf("Leech:\t%f\nBbound:\t%f\n", p_gem->leech, p_gem->bbound);
		cout << "Red:\t" << boolalpha << p_gem->red << '\n';
		printf("Mana power:\t%f\n\n", p_gem->leech*p_gem->bbound);
		break;
		case 'k':
		printf("Killgem\n");
		printf("Value:\t%d\nGrade:\t%d\n", p_gem->getvalue(), p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\nBbound:\t%f\n", p_gem->damage, p_gem->crit, p_gem->bbound);
		cout << "Red:\t" << boolalpha << p_gem->red << '\n';
		printf("Kill power:\t%f\n\n", p_gem->damage*p_gem->bbound*p_gem->crit*p_gem->bbound);
		break;
		default:
		printf("Strange gem\n");
		printf("Value:\t%d\nGrade:\t%d\n", p_gem->getvalue(), p_gem->grade);
		printf("Damage:\t%f\nCrit:\t%f\nLeech:\t%f\nBbound:\t%f\n", p_gem->damage, p_gem->crit, p_gem->leech, p_gem->bbound);
		cout << "Red:\t" << boolalpha << p_gem->red << '\n';
		printf("Mana power:\t%f\n", p_gem->leech*p_gem->bbound);
		printf("Kill power:\t%f\n\n", p_gem->damage*p_gem->bbound*p_gem->crit*p_gem->bbound);
	}
}

gem* gem_build(string parens, gem* gems, int& index)
{
	int len = parens.length();
	gem newgem;
	if (len==1) newgem = gem(parens[0]);
	else {
		int open_parens=0;
		int i;
		for (i=1; i<len-1; ++i) {
			if (parens[i]=='(') open_parens++;
			if (parens[i]==')') open_parens--;
			if (open_parens==0) break;
		}
		gem* mother_gemp;
		gem* father_gemp;
		if (i > len-i-3) {
			mother_gemp = gem_build(parens.substr(  1,       i), gems, index);
			father_gemp = gem_build(parens.substr(i+2, len-i-3), gems, index);
		}
		else {
			mother_gemp = gem_build(parens.substr(i+2, len-i-3), gems, index);
			father_gemp = gem_build(parens.substr(  1,       i), gems, index);
		}
		gem_combine(father_gemp, mother_gemp, &newgem);
	}
	for (int i=0; i < index; ++i) {
		if (gems[i]==newgem) return gems+i;
	}
	gems[index]=newgem;
	return gems+(index++);
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

void worker(string parens, string parens_amps, int output_options, int TC, int As, int Namps)
{
	int index=0;
	int value=(parens.length()+3)/4;
	gem* gems = new gem[2*value-1];
	gem* gemf = gem_build(parens, gems, index);
	
	printf("\nMain gem:\n");
	gem_print(gemf);
	if (output_options & mask_parens) {
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Gem tree:\n");
		print_tree(gemf, "");
		printf("\n");
	}
	if (output_options & mask_equations) {
		printf("Equations:\n");
		print_equations(gemf);
		printf("\n");
	}
	
	if (parens_amps.length() > 0) {
		int index=0;
		int value=(parens_amps.length()+3)/4;
		gem* amps = new gem[2*value-1];
		gem* ampf = gem_build(parens_amps, amps, index);
		
		printf("Amplifier (x%d)\n", Namps);
		gem_print(ampf);
		if (output_options & mask_parens) {
			printf("Amplifier scheme:\n");
			print_parens_compressed(ampf);
			printf("\n\n");
		}
		if (output_options & mask_tree) {
			printf("Amplifier tree:\n");
			print_tree(ampf, "");
			printf("\n");
		}
		if (output_options & mask_equations) {
			printf("Amplifier equations:\n");
			print_equations(ampf);
			printf("\n");
		}
		double specials_ratio=Namps*(0.15+As/3*0.004)*2*(1+0.03*TC)/(1.0+TC/3*0.1);
		double damage_ratio  =Namps*(0.20+As/3*0.004) * (1+0.03*TC)/(1.2+TC/3*0.1);
		printf("Global mana power:\t%#.7g\n",   gem_amp_mana_power(*gemf, *ampf, specials_ratio));
		printf("Global kill power:\t%#.7g\n\n", gem_amp_kill_power(*gemf, *ampf, damage_ratio, specials_ratio));
		delete[] amps;
	}
	delete[] gems;
}

int main(int argc, char** argv)
{
	string parens="";
	string parens_amps="";
	char opt;
	int TC=120;
	int As=60;
	int Namps=6;
	int output_options=0;
	while ((opt=getopt(argc,argv,"hptef:a:T:A:N:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptef:a:T:A:N:\n");
			return 0;
			PTECIDCUR_OPTIONS_BLOCK
			case 'f': {
				ifstream file;
				file.open(optarg, fstream::out);
				getline(file, parens);
				file.close();
				parens = ieeePreParser(parens);
			} break;
			case 'a':
				parens_amps = ieeePreParser(optarg);
			break;
			//TAN_OPTIONS_BLOCK
			case 'T':
				TC=atoi(optarg);
				break;
			case 'A':
				As=atoi(optarg);
				break;
			case 'N':
				Namps=atoi(optarg);
				break;
			case '?':
				return 1;
			default:
			break;
		}
	}
	if (optind==argc) {
		printf("No recipe specified\n");
		return 1;
	}
	if (optind+1==argc) {
		parens = ieeePreParser(argv[optind]);
	}
	else {
		printf("Too many arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		printf("\n");
		return 1;
	}
	if (parens.length() < 1) {
		printf("Improper gem number\n");
		return 1;
	}
	worker(parens, parens_amps, output_options, TC, As, Namps);
	return 0;
}
