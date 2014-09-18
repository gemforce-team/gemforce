#include <vector>
#include <iostream>
#include "gem_utils.hpp"
#include "killgem_limiter.hpp"
using namespace std;

bool better_killgem (const Gem* a, const Gem* b)
{
  return  a->damage*a->black*a->yellow*a->black >
          b->damage*b->black*b->yellow*b->black;
}

int main ()
{
  vector<Gem*>* base_gems = new vector<Gem*>;
  base_gems->push_back(new Gem(1, 0, 0, 1));
  base_gems->push_back(new Gem(0, 0, 1, 1));
  vector<Gem*>** gemset=generate_gemset(base_gems, 32, limit_killgem);
  Gem* g=best_from(gemset[32], better_killgem);
  print_tree(g);
  print_stats(g);
  return 0;
}
