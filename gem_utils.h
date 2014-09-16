#ifndef _GEM_UTILS_H
#define _GEM_UTILS_H

const int ACC=1000;

typedef struct Gem {
  int grade;          //using short does NOT improve time/memory usage
  float leech;        //float is a good 5% faster than double and loses nearly nothing
  float bbound;
  struct Gem* father;
  struct Gem* mother;
} gem;

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

char gem_color(gem* p_gem)
{
  if (p_gem->leech==0) return 'b';
  else return 'o';
}

int gem_less_equal(gem gem1, gem gem2)
{
  if (gem1.grade != gem2.grade)
    return gem1.grade<gem2.grade;
  if ((int)(gem1.leech*ACC) != (int)(gem2.leech*ACC))
    return gem1.leech<gem2.leech;
  return gem1.bbound<gem2.bbound;
}

void gem_sort(gem* gems, int len) 
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
  gem_sort(gems,pivot);
  gem_sort(gems+1+pivot,len-pivot-1);
}

void print_table(gem* gems, int len)
{
  printf("# Gems\tPower\n");
  int i;
  for (i=0;i<len;i++) printf("%d\t%.6lf\n",i+1,gems[i].leech*gems[i].bbound);
  printf("\n");
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

#endif // _GEM_UTILS_H
