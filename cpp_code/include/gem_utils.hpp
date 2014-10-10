#ifndef _GEM_UTILS_HPP
#define _GEM_UTILS_HPP

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>
using namespace std;

class Gem;
typedef pair<Gem*, Gem*> gem_pair;

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
  string color ()
  {
    if (abs(yellow)>0.0001)
      return "yellow";
    else if (abs(orange)>0.0001)
      return "orange";
    return "black";
  }
};

Gem combine (Gem* self, Gem* other)
{
  if (self->value<other->value)
    swap(self, other);
  switch (abs(self->grade-other->grade))
  {
    case 0:
      return Gem(
        max(self->yellow, other->yellow)*0.88+min(self->yellow, other->yellow)*0.5,
                     max(self->orange, other->orange)*0.88+min(self->orange, other->orange)*0.5,
                     max(self->black, other->black)*0.78+min(self->black, other->black)*0.31,
                     max(self->damage, other->damage)*0.87+min(self->damage, other->damage)*0.71,
                     self->grade+1,
                     self->value+other->value,
                     make_pair(self, other)
      );
    case 1:
      return Gem(
        max(self->yellow, other->yellow)*0.88+min(self->yellow, other->yellow)*0.44,
                     max(self->orange, other->orange)*0.89+min(self->orange, other->orange)*0.44,
                     max(self->black, other->black)*0.79+min(self->black, other->black)*0.29,
                     max(self->damage, other->damage)*0.86+min(self->damage, other->damage)*0.7,
                     max(self->grade, other->grade),
                     self->value+other->value,
                     make_pair(self, other)
      );
    default:
      return Gem(
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

void print_tree (Gem* g, string prefix="")
{
  if (g->value==1)
    cout<<"━ (g1 "<<g->color()<<")\n";
  else
  {
    cout<<"━"<<g->value<<"\n";
    cout<<prefix<<"┣";
    print_tree(g->parents.first, prefix+"┃ ");
    cout<<prefix<<"┗";
    print_tree(g->parents.second, prefix+"  ");
  }
}

void print_equations (Gem* g)
{
  vector<Gem*> harvested;
  set<long long> present;
  harvested.push_back(g);
  present.insert(g->number);
  for (unsigned i=0; i<harvested.size(); i++)
  {
    Gem*& w=harvested[i];
    if (w->value!=1)
      for (Gem* c : {w->parents.first, w->parents.second})
        if (present.find(c->number)==present.end())
        {
          present.insert(c->number);
          harvested.push_back(c);
        }
  }
  int k=0;
  map<long long, int> renum;
  for (auto it:present)
    renum[it]=k++;
  sort(harvested.begin(), harvested.end(), [](const Gem* a, const Gem* b){return a->number<b->number;});
  for (Gem* w : harvested)
  {
    cout<<"(val = "<<w->value<<")\t"<<renum[w->number]<<"=";
    if (w->value==1)
      cout<<"(g1 "<<w->color()<<")\n";
    else
      cout<<renum[w->parents.first->number]<<"+"<<renum[w->parents.second->number]<<"\n";
  }
}

void print_stats (Gem* g)
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