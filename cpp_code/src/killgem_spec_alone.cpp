#include <vector>
#include <iostream>
#include "gem_utils_killgem.hpp"
#include "gemset_multithreaded.hpp"
#include "killgem_limiter.hpp"
using namespace std;

const int VALUE=128;

bool better_killgem (const Gem* a, const Gem* b)
{
  return  a->damage*a->black*a->yellow*a->black >
          b->damage*b->black*b->yellow*b->black;
}

int main ()
{
  vector<Gem*>* base_gems = new vector<Gem*>;
  base_gems->push_back(new Gem(1, 0, 0, 1));
  base_gems->push_back(new Gem(0, 0, 1, 1.1861));
  vector<Gem*>** gemset=generate_gemset_multithreaded(base_gems, VALUE, limit_killgem);
  Gem* g=best_from(gemset[VALUE], better_killgem);
  print_tree(g);
  print_stats(g);
  print_equations(g);
  return 0;
}
