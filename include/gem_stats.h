#ifndef _GEM_STATS_H
#define _GEM_STATS_H

// ----------------------
// Combining coefficients
// ----------------------

// Damage Min
const double DAMAGE_MIN_EQ_1 = 0.83;
const double DAMAGE_MIN_EQ_2 = 0.71;
const double DAMAGE_MIN_D1_1 = 0.82;
const double DAMAGE_MIN_D1_2 = 0.70;
const double DAMAGE_MIN_GN_1 = 0.81;
const double DAMAGE_MIN_GN_2 = 0.69;

// Damage Max
const double DAMAGE_EQ_1 = 0.87;
const double DAMAGE_EQ_2 = 0.71;
const double DAMAGE_D1_1 = 0.86;
const double DAMAGE_D1_2 = 0.70;
const double DAMAGE_GN_1 = 0.85;
const double DAMAGE_GN_2 = 0.69;

// Range
const double RANGE_EQ_1 = 0.694;
const double RANGE_EQ_2 = 0.388;
const double RANGE_D1_1 = 0.80;
const double RANGE_D1_2 = 0.25;
const double RANGE_GN_1 = 0.92;
const double RANGE_GN_2 = 0.09;

// Firerate
const double FIRERATE_EQ_1 = 0.74;
const double FIRERATE_EQ_2 = 0.44;
const double FIRERATE_D1_1 = 0.80;
const double FIRERATE_D1_2 = 0.25;
const double FIRERATE_GN_1 = 0.92;
const double FIRERATE_GN_2 = 0.09;

// Bloodbound
const double BBOUND_EQ_1 = 0.78;
const double BBOUND_EQ_2 = 0.31;
const double BBOUND_D1_1 = 0.79;
const double BBOUND_D1_2 = 0.29;
const double BBOUND_GN_1 = 0.80;
const double BBOUND_GN_2 = 0.27;

// Slow
const double SLOW_EQ_1 = 0.81;
const double SLOW_EQ_2 = 0.35;
const double SLOW_D1_1 = 0.80;
const double SLOW_D1_2 = 0.28;
const double SLOW_GN_1 = 0.79;
const double SLOW_GN_2 = 0.26;

// Poison
const double POISON_EQ_1 = 0.96;
const double POISON_EQ_2 = 0.85;
const double POISON_D1_1 = 0.97;
const double POISON_D1_2 = 0.62;
const double POISON_GN_1 = 0.98;
const double POISON_GN_2 = 0.42;

// Crit Chance
const double CRIT_CHANCE_EQ_1 = 0.81;
const double CRIT_CHANCE_EQ_2 = 0.35;
const double CRIT_CHANCE_D1_1 = 0.80;
const double CRIT_CHANCE_D1_2 = 0.28;
const double CRIT_CHANCE_GN_1 = 0.79;
const double CRIT_CHANCE_GN_2 = 0.26;

// Crit Mult
const double CRIT_EQ_1 = 0.88;
const double CRIT_EQ_2 = 0.5;
const double CRIT_D1_1 = 0.88;
const double CRIT_D1_2 = 0.44;
const double CRIT_GN_1 = 0.88;
const double CRIT_GN_2 = 0.44;

const double CRIT_NEQ_1 = 0.88;
const double CRIT_NEQ_2 = 0.44;

// Chain Hit
const double CHAINHIT_EQ_1 = 0.88;
const double CHAINHIT_EQ_2 = 0.50;
const double CHAINHIT_D1_1 = 0.90;
const double CHAINHIT_D1_2 = 0.47;
const double CHAINHIT_GN_1 = 0.92;
const double CHAINHIT_GN_2 = 0.44;

// Leech
const double LEECH_EQ_1 = 0.88;
const double LEECH_EQ_2 = 0.50;
const double LEECH_D1_1 = 0.89;
const double LEECH_D1_2 = 0.44;
const double LEECH_GN_1 = 0.90;
const double LEECH_GN_2 = 0.38;

// Armor Tearing
const double ARMOR_EQ_1 = 0.94;
const double ARMOR_EQ_2 = 0.69;
const double ARMOR_D1_1 = 0.95;
const double ARMOR_D1_2 = 0.57;
const double ARMOR_GN_1 = 0.96;
const double ARMOR_GN_2 = 0.45;

// Poolbound
const double PBOUND_EQ_1 = 0.87;
const double PBOUND_EQ_2 = 0.38;
const double PBOUND_D1_1 = 0.87;
const double PBOUND_D1_2 = 0.38;
const double PBOUND_GN_1 = 0.87;
const double PBOUND_GN_2 = 0.38;

const double PBOUND_1 = 0.87;
const double PBOUND_2 = 0.38;

// Suppressive
const double SUPPR_EQ_1 = 0.96;
const double SUPPR_EQ_2 = 1.91;
const double SUPPR_D1_1 = 0.92;
const double SUPPR_D1_2 = 1.13;
const double SUPPR_GN_1 = 0.92;
const double SUPPR_GN_2 = 0.73;


// -------------------------
// Damage (relative to Crit)
// -------------------------

const double DAMAGE_POISON = 10./11.;
const double DAMAGE_CRIT   = 11./11.;
const double DAMAGE_BBOUND = 13./11.;
const double DAMAGE_SLOW   = 12./11.;
const double DAMAGE_CHHIT  = 10./11.;
const double DAMAGE_LEECH  =  8./11.;
const double DAMAGE_ARMOR  = 11./11.;
const double DAMAGE_SUPPR  = 12./11.;
const double DAMAGE_PBOUND = 12./11.;

#endif // _GEM_STATS_H
