#ifndef _KILLGEM_UTILS_H
#define _KILLGEM_UTILS_H

struct Gem_YB {
	int grade;							// short does NOT help
	float damage;						// this is MAX damage, with the rand() part neglected
	float crit;							// assumptions: crit chance capped
	float bbound;						// BB hit lv >> 1
	struct Gem_YB* father;	// maximize damage*bbound*crit*bbound
	struct Gem_YB* mother;
};

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

float gem_power(gem gem1)
{
	return gem1.damage*gem1.bbound*gem1.crit*gem1.bbound;			// amp-less
}

void gem_print(gem* p_gem) {
	printf("Grade:\t%d\nDamage:\t%f\nCrit:\t%f\nBbound:\t%f\nPower:\t%f\n\n", 
		p_gem->grade, p_gem->damage, p_gem->crit, p_gem->bbound, gem_power(*p_gem));
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
  p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->damage > p_gem2->damage) p_gem_combined->damage = 0.87*p_gem1->damage + 0.71*p_gem2->damage;
  else p_gem_combined->damage = 0.87*p_gem2->damage + 0.71*p_gem1->damage;
  if (p_gem1->crit > p_gem2->crit) p_gem_combined->crit = 0.88*p_gem1->crit + 0.5*p_gem2->crit;
  else p_gem_combined->crit = 0.88*p_gem2->crit + 0.5*p_gem1->crit;
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

void gem_init(gem *p_gem, int grd, double damage, double crit, double bbound)
{
  p_gem->grade =grd;
  p_gem->damage=damage;
  p_gem->crit  =crit;
  p_gem->bbound=bbound;
  p_gem->father=NULL;
  p_gem->mother=NULL;
}

int gem_more_powerful(gem gem1, gem gem2)
{
	return (gem_power(gem1) > gem_power(gem2));
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
  for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gem_power(gems[i]));
  printf("\n");
}

char gem_color(gem* p_gem)
{
  if (p_gem->crit==0) return 'b';
  if (p_gem->bbound==0) return 'y';
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
  if(p_gem->father==NULL) return 1;
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

int gem_has_less_crit(gem gem1, gem gem2)
{
  if (gem1.crit < gem2.crit) return 1;
  else if (gem1.crit == gem2.crit && gem1.bbound < gem2.bbound) return 1;
  else return 0;
}

void gem_sort_crit(gem* gems, int len)    // it assumes all gems are at the same grade
{
  if (len<=1) return;											
  int pivot=0;
  int i;
  for (i=1;i<len;++i) {
    if (gem_has_less_crit(gems[i],gems[pivot])) {
      gem temp=gems[pivot];
      gems[pivot]=gems[i];
      gems[i]=gems[pivot+1];
      gems[pivot+1]=temp;
      pivot++;
    }
  }
  gem_sort_crit(gems,pivot);
  gem_sort_crit(gems+1+pivot,len-pivot-1);
}

void worker(int len, int output_parens, int output_tree, int output_table, int output_debug, int output_info);

int get_opts_and_call_worker(int argc, char** argv)
{
	int len;
	char opt;
	int output_parens=0;
	int output_tree=0;
	int output_table = 0;
	int output_debug=0;
	int output_info=0;
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


#endif // _KILLGEM_UTILS_H
