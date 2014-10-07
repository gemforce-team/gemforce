#include <vector>
#include <iostream>
#include "gem_utils.hpp"
#include "gemset.hpp"
#include "managem_limiter.hpp"
using namespace std;

bool better_orange (const Gem* a, const Gem* b)
{
  return a->orange>b->orange;
}

int main ()
{
  vector<Gem*>* base_gems = new vector<Gem*>;
  base_gems->push_back(new Gem(0, 1, 0));
  vector<Gem*>** gemset=generate_gemset(base_gems, 64, limit_managem);
  Gem* g=best_from(gemset[64], better_orange);
  print_tree(g);
  print_stats(g);
  return 0;
}
