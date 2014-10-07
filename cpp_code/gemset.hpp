#ifndef _GEMSET_HPP
#define _GEMSET_HPP

#include <vector>
#include "gem_utils.hpp"

vector<Gem*>** generate_gemset (
  vector<Gem*>*  base_gems,
  int           limit_value,
  void          (*limiter) (vector<Gem*>*&)
)
{
  vector<Gem*>** gemset = new vector<Gem*>*[limit_value+1];
  for (int i=1; i<=limit_value; i++)
    gemset[i]=new vector<Gem*>;
  for (Gem* e:*base_gems)
    gemset[e->value]->push_back(e);
  for (int i=2; i<=limit_value; i++)
  {
    cout<<"Iteration: "<<i<<"\n";
    vector<Gem*>*& tmp=gemset[i];
    for (int first_val=1; 2*first_val<=i; first_val++)
    {
      int second_val=i-first_val;
      for (Gem* gem1 : *gemset[first_val])
        for (Gem* gem2 : *gemset[second_val])
          tmp->push_back(combine(gem1, gem2));
    }
    cout<<"Pool size (before limiting): "<<tmp->size()<<"\n";
    limiter(tmp);
    cout<<"Pool size (after limiting): "<<tmp->size()<<"\n";
    cout<<"End of iteration "<<i<<"\n\n\n";
  }
  return gemset;
}

#endif // _GEMSET_HPP
