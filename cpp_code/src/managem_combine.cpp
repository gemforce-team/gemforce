#include <vector>
#include <iostream>
#include "gem_utils_managem.hpp"
#include "gemset_multithreaded.hpp"
#include "managem_limiter.hpp"
using namespace std;

const int VALUE=512;

bool better_managem (const Gem* a, const Gem* b)
{
  return a->orange*a->black>b->orange*b->black;
}

int main ()
{
  vector<Gem*>* base_gems = new vector<Gem*>;
  base_gems->push_back(new Gem(0, 1, 1, 0));
  vector<Gem*>** gemset=generate_gemset_multithreaded(base_gems, VALUE, limit_managem);
  Gem* g=best_from(gemset[VALUE], better_managem);
  print_tree(g);
  print_stats(g);
  return 0;
}
