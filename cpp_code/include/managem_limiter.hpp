#ifndef _MANAGEM_LIMITER_HPP
#define _MANAGEM_LIMITER_HPP

#include <vector>
using namespace std;

bool limit_managem_cmp (const Gem& a, const Gem& b)
{
  if (a.grade!=b.grade)
    return a.grade>b.grade;
  if (int(a.orange*ORANGE_ACC)!=int(b.orange*ORANGE_ACC))
    return a.orange>b.orange;
  return a.black>b.black;
}

void limit_managem (vector<Gem>*& gems)
{
  sort(gems->begin(), gems->end(), limit_managem_cmp);
  vector<Gem>* tmp = new vector<Gem>;
  int boundary=-1;
  int current_grade=0;
  for (Gem g: *gems)
  {
    if (g.grade!=current_grade)
    {
      current_grade=g.grade;
      boundary=-1;
    }
    if (int(g.black*BLACK_ACC)>boundary)
    {
      boundary=int(g.black*BLACK_ACC);
      tmp->push_back(g);
    }
  }
  swap(gems, tmp);
  delete tmp;
}

#endif // _MANAGEM_LIMITER_HPP
