#include <vector>
#include <iostream>
#include "gem_utils.hpp"
#include "gemset.hpp"
#include "managem_limiter.hpp"
using namespace std;

bool better_managem (const Gem* a, const Gem* b)
{
  return a->orange*a->black>b->orange*b->black;
}

int main ()
{
  vector<Gem*>* base_gems = new vector<Gem*>;
  base_gems->push_back(new Gem(0, 1, 1));
//   base_gems->push_back(new Gem(0, 0, 1));
//   base_gems->push_back(new Gem(0, 1, 0));
  vector<Gem*>** gemset=generate_gemset(base_gems, 1*1024, limit_managem);
  Gem* g=best_from(gemset[1*1024], better_managem);
  print_tree(g);
  print_stats(g);
  return 0;
}
