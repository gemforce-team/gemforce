#ifndef _GEM_UTILS_HPP
#define _GEM_UTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
using namespace std;

int ORANGE_ACC=150;
int YELLOW_ACC=150;
int  BLACK_ACC=150;
int DAMAGE_ACC=150;

class Gem;
typedef pair<const Gem*, const Gem*> gem_pair;

class Gem
{
public:
  long long number;
  double yellow;
  double orange;
  double black;
  double damage;
  int grade;
  int value;
  gem_pair parents;
  Gem (double y=0, double o=0, double b=0, double d=0, int g=1, int v=1, gem_pair p=gem_pair(nullptr, nullptr))
  : yellow(y)
  , orange(o)
  , black(b)
  , damage(d)
  , grade(g)
  , value(v)
  , parents(p)
  {
    static long long counter=0;
    number=counter++;
  }
};

Gem* combine (const Gem* self, const Gem* other)
{
  if (self->value<other->value)
    swap(self, other);
  switch (abs(self->grade-other->grade))
  {
    case 0:
      return new Gem(
        max(self->yellow, other->yellow)*0.88+min(self->yellow, other->yellow)*0.5,
                     max(self->orange, other->orange)*0.88+min(self->orange, other->orange)*0.5,
                     max(self->black, other->black)*0.78+min(self->black, other->black)*0.31,
                     max(self->damage, other->damage)*0.87+min(self->damage, other->damage)*0.71,
                     self->grade+1,
                     self->value+other->value,
                     make_pair(self, other)
      );
    case 1:
      return new Gem(
        max(self->yellow, other->yellow)*0.88+min(self->yellow, other->yellow)*0.44,
                     max(self->orange, other->orange)*0.89+min(self->orange, other->orange)*0.44,
                     max(self->black, other->black)*0.79+min(self->black, other->black)*0.29,
                     max(self->damage, other->damage)*0.86+min(self->damage, other->damage)*0.7,
                     max(self->grade, other->grade),
                     self->value+other->value,
                     make_pair(self, other)
      );
    default:
      return new Gem(
        max(self->yellow, other->yellow)*0.88+min(self->yellow, other->yellow)*0.44,
                     max(self->orange, other->orange)*0.9+min(self->orange, other->orange)*0.38,
                     max(self->black, other->black)*0.8+min(self->black, other->black)*0.27,
                     max(self->damage, other->damage)*0.85+min(self->damage, other->damage)*0.69,
                     max(self->grade, other->grade),
                     self->value+other->value,
                     make_pair(self, other)
      );
  }
}

Gem* best_from(vector<Gem*>* v, bool (*comparator) (const Gem*, const Gem*))
{
  Gem* best=(*v)[0];
  for (Gem* e:*v)
  {
    if (comparator(e, best))
      best=e;
  }
  return best;
}

void print_tree (const Gem* g, string prefix="")
{
  if (g->value==1)
    cout<<"━ (g1 "<<(abs(g->yellow)>0.0001 ? "yellow" : (abs(g->orange)>0.0001 ? "orange" : "black"))<<")\n";
  else
  {
    
    cout<<"━"<<g->value<<"\n";
    cout<<prefix<<"┣";
    print_tree(g->parents.first, prefix+"┃ ");
    cout<<prefix<<"┗";
    print_tree(g->parents.second, prefix+"  ");
  }
}

void print_stats (const Gem* g)
{
  cout<<"Value: "<<g->value<<"\n";
  cout<<"Grade: "<<g->grade<<"\n";
  cout<<"Crit: "<<g->yellow<<"\n";
  cout<<"Leech: "<<g->orange<<"\n";
  cout<<"Bloodbound: "<<g->black<<"\n";
  cout<<"Damage: "<<g->damage<<"\n";
  float mp=g->orange*g->black;
  float dp=g->yellow*g->black*g->damage*g->black;
  cout<<"Mana power: "<<mp<<"\n";
  cout<<"Mana power growth: "<<log(mp)/log(g->value)<<"\n";
  cout<<"Damage power: "<<dp<<"\n";
  cout<<"Damage power growth: "<<log(dp)/log(g->value)<<"\n";
}

#endif // _GEM_UTILS_HPP