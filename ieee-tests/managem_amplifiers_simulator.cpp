#include <vector>
#include <iostream>
#include <cmath>
#include "gem_utils.hpp"
#include "managem_limiter.hpp"
using namespace std;

const int MANAGEM_VALUE=32;
const int AMP_MAX_VALUE=128;
const float GROWTH = 0.6343;
const float MANA_GROWTH = 0.4958;

bool better_orange (const Gem* a, const Gem* b)
{
  return a->orange>b->orange;
}


float amp_managem_power_old (const Gem* managem, const Gem* amplifier)
{
  int value = managem->value+6*amplifier->value;
  float power = (managem->orange*1.5+6*0.23*2.8*amplifier->orange)*managem->black;
  return power/pow(value, GROWTH);
}

float amp_managem_power (const Gem* managem, const Gem* amplifier)
{
  if (int(managem->black*ACC)==0 || int(managem->orange*ACC)==0)
    return 0;
  int value = managem->value+6*amplifier->value;
  float power = (managem->orange*1.5+6*0.23*2.8*amplifier->orange)*managem->black;
  float x = power/pow(value, log(managem->orange*managem->black)/log(managem->value+6*amplifier->value));
  cout<<x<<"\n";
  return x;
}

int main ()
{
  vector<Gem*>* basegems_managem = new vector<Gem*>;
  basegems_managem->push_back(new Gem(0, 1, 0));
  basegems_managem->push_back(new Gem(0, 0, 1));
  vector<Gem*>** gemset_managem=generate_gemset(basegems_managem, MANAGEM_VALUE, limit_managem);

  vector<Gem*>* basegems_amplifier = new vector<Gem*>;
  basegems_amplifier->push_back(new Gem(0, 1, 0));
  vector<Gem*>** gemset_amplifier=generate_gemset(basegems_amplifier, AMP_MAX_VALUE, limit_managem);

  vector<Gem*>* managems = gemset_managem[MANAGEM_VALUE];
  
  vector<Gem*>* amplifiers = new vector<Gem*>;
  for (int i=1; i<=AMP_MAX_VALUE; i++)
//     amplifiers->push_back(new Gem(0, pow(i, MANA_GROWTH), 0, 0, 1, i));
    amplifiers->push_back(best_from(gemset_amplifier[i], better_orange));
  
  Gem* best_managem = (*managems)[0];
  Gem* best_amplifier = (*amplifiers)[0];
  float power = amp_managem_power(best_managem, best_amplifier);
  for (Gem* m : *managems)
    for (Gem* a : *amplifiers)
      if (amp_managem_power(m, a)>power)
      {
        power=amp_managem_power(m, a);
        best_managem=m;
        best_amplifier=a;
      }
  print_tree(best_managem);
  print_tree(best_amplifier);
  print_stats(best_managem);
  print_stats(best_amplifier);
  return 0;
}
