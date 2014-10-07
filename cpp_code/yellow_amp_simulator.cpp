#include <vector>
#include <iostream>
#include "gem_utils.hpp"
#include "gemset.hpp"
#include "killgem_limiter.hpp"
using namespace std;

bool better_yellow (const Gem* a, const Gem* b)
{
  return a->yellow*a->damage>b->yellow*b->damage;
}

int main ()
{
  vector<Gem*>* base_gems = new vector<Gem*>;
  base_gems->push_back(new Gem(1, 0, 0, 1));
  vector<Gem*>** gemset=generate_gemset(base_gems, 32, limit_killgem);
  Gem* g=best_from(gemset[32], better_yellow);
  print_tree(g);
  print_stats(g);
  return 0;
}
