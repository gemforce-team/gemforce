#ifndef _KILLGEM_LIMITER_HPP
#define _KILLGEM_LIMITER_HPP

#include <vector>
#include "interval_tree.hpp"
using namespace std;
bool limit_killgem_cmp (const Gem& a, const Gem& b)
{
  if (a.grade!=b.grade)
    return a.grade>b.grade;
  if (int(a.damage*DAMAGE_ACC)!=int(b.damage*DAMAGE_ACC))
    return a.damage>b.damage;
  if (int(a.yellow*YELLOW_ACC)!=int(b.yellow*YELLOW_ACC))
    return a.yellow>b.yellow;
  return a.black>b.black;
}

void limit_killgem (vector<Gem>*& gems)
{
  sort(gems->begin(), gems->end(), limit_killgem_cmp);
  vector<Gem>* tmp = new vector<Gem>;
  int current_grade=0;
  IntervalTree tree(20);
  for (Gem g:(*gems))
  {
    if (g.grade!=current_grade)
    {
      current_grade=g.grade;
      tree.clear();
    }
    if (tree.read_max(int(g.yellow*YELLOW_ACC))<int(g.black*BLACK_ACC))
    {
      tree.add_element(int(g.yellow*YELLOW_ACC), int(g.black*BLACK_ACC));
      tmp->push_back(g);
    }
  }
  swap(gems, tmp);
  delete tmp;
}

#endif // _KILLGEM_LIMITER_HPP