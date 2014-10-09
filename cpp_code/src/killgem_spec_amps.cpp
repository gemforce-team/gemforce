#include <vector>
#include <iostream>
#include <cmath>
#include "gem_utils_killgem.hpp"
#include "gemset_multithreaded.hpp"
#include "killgem_limiter.hpp"
using namespace std;

const int KILLGEM_VALUE=64;
const int AMP_MAX_VALUE=32;
const float GROWTH = 1.4274;

float amp_killgem_power (const Gem* killgem, const Gem* amplifier)
{
  int value = killgem->value+6*amplifier->value;
  float power = (killgem->yellow*1.5+0.23*6*2.8*amplifier->yellow)*killgem->black*
                (killgem->damage*3.2+0.28*6*2.8*amplifier->damage)*killgem->black;
  return power/pow(value, GROWTH);
}

int main ()
{
  vector<Gem*>* basegems_killgem = new vector<Gem*>;
  basegems_killgem->push_back(new Gem(1, 0, 0, 1));
  basegems_killgem->push_back(new Gem(0, 0, 1, 1.1861));
  vector<Gem*>** gemset_killgem=generate_gemset_multithreaded(basegems_killgem, KILLGEM_VALUE, limit_killgem);

  vector<Gem*>* basegems_amplifier = new vector<Gem*>;
  basegems_amplifier->push_back(new Gem(1, 0, 0, 1));
  vector<Gem*>** gemset_amplifier=generate_gemset_multithreaded(basegems_amplifier, AMP_MAX_VALUE, limit_killgem);

  vector<Gem*>* killgems = gemset_killgem[KILLGEM_VALUE];
  
  vector<Gem*>* amplifiers = new vector<Gem*>;
  for (int i=1; i<=AMP_MAX_VALUE; i++)
    for (Gem* g : *gemset_amplifier[i])
      amplifiers->push_back(g);
  
  Gem* best_killgem = (*killgems)[0];
  Gem* best_amplifier = (*amplifiers)[0];
  float power = amp_killgem_power(best_killgem, best_amplifier);
  for (Gem* m : *killgems)
    for (Gem* a : *amplifiers)
      if (amp_killgem_power(m, a)>power)
      {
        power=amp_killgem_power(m, a);
        best_killgem=m;
        best_amplifier=a;
      }
  print_tree(best_killgem);
  print_tree(best_amplifier);
  print_stats(best_killgem);
  print_stats(best_amplifier);
  cout<<"power together: "<<power<<"\n";
  return 0;
}
