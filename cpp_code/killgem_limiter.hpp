#ifndef _KILLGEM_LIMITER_HPP
#define _KILLGEM_LIMITER_HPP

#include <vector>
#include "gem_utils.hpp"
#include "interval_tree.hpp"
using namespace std;
bool limit_killgem_cmp (const Gem* a, const Gem* b)
{
  if (a->grade!=b->grade)
    return a->grade>b->grade;
  if (int(a->damage*ACC)!=int(b->damage*ACC))
    return a->damage>b->damage;
  if (int(a->yellow*ACC)!=int(b->yellow*ACC))
    return a->yellow>b->yellow;
  return a->black>b->black;
}

void limit_killgem (vector<Gem*>*& gems)
{
  sort(gems->begin(), gems->end(), limit_killgem_cmp);
  vector<Gem*>* tmp = new vector<Gem*>;
  int current_grade=0;
  IntervalTree tree(20);
  for (Gem* g:(*gems))
  {
    if (g->grade!=current_grade)
    {
      current_grade=g->grade;
      tree.clear();
    }
    if (tree.read_max(int(g->yellow*ACC))<int(g->black*ACC))
    {
      tree.add_element(int(g->yellow*ACC), int(g->black*ACC));
      tmp->push_back(g);
    }
    else
      delete g;
  }
  swap(gems, tmp);
  delete tmp;
}

#endif // _KILLGEM_LIMITER_HPP