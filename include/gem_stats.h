#ifndef _GEM_STATS_H
#define _GEM_STATS_H

// ----------------------
// Combining coefficients
// ----------------------

// Damage Min
constexpr double DAMAGE_MIN_EQ_1 = 0.83;
constexpr double DAMAGE_MIN_EQ_2 = 0.71;
constexpr double DAMAGE_MIN_D1_1 = 0.82;
constexpr double DAMAGE_MIN_D1_2 = 0.70;
constexpr double DAMAGE_MIN_GN_1 = 0.81;
constexpr double DAMAGE_MIN_GN_2 = 0.69;

// Damage Max
constexpr double DAMAGE_EQ_1 = 0.87;
constexpr double DAMAGE_EQ_2 = 0.71;
constexpr double DAMAGE_D1_1 = 0.86;
constexpr double DAMAGE_D1_2 = 0.70;
constexpr double DAMAGE_GN_1 = 0.85;
constexpr double DAMAGE_GN_2 = 0.69;

// Range
constexpr double RANGE_EQ_1 = 0.694;
constexpr double RANGE_EQ_2 = 0.388;
constexpr double RANGE_D1_1 = 0.80;
constexpr double RANGE_D1_2 = 0.25;
constexpr double RANGE_GN_1 = 0.92;
constexpr double RANGE_GN_2 = 0.09;

// Firerate
constexpr double FIRERATE_EQ_1 = 0.74;
constexpr double FIRERATE_EQ_2 = 0.44;
constexpr double FIRERATE_D1_1 = 0.80;
constexpr double FIRERATE_D1_2 = 0.25;
constexpr double FIRERATE_GN_1 = 0.92;
constexpr double FIRERATE_GN_2 = 0.09;

// Bloodbound
constexpr double BBOUND_EQ_1 = 0.78;
constexpr double BBOUND_EQ_2 = 0.31;
constexpr double BBOUND_D1_1 = 0.79;
constexpr double BBOUND_D1_2 = 0.29;
constexpr double BBOUND_GN_1 = 0.80;
constexpr double BBOUND_GN_2 = 0.27;

// Slow
constexpr double SLOW_EQ_1 = 0.81;
constexpr double SLOW_EQ_2 = 0.35;
constexpr double SLOW_D1_1 = 0.80;
constexpr double SLOW_D1_2 = 0.28;
constexpr double SLOW_GN_1 = 0.79;
constexpr double SLOW_GN_2 = 0.26;

// Poison
constexpr double POISON_EQ_1 = 0.96;
constexpr double POISON_EQ_2 = 0.85;
constexpr double POISON_D1_1 = 0.97;
constexpr double POISON_D1_2 = 0.62;
constexpr double POISON_GN_1 = 0.98;
constexpr double POISON_GN_2 = 0.42;

// Crit Chance
constexpr double CRIT_CHANCE_EQ_1 = 0.81;
constexpr double CRIT_CHANCE_EQ_2 = 0.35;
constexpr double CRIT_CHANCE_D1_1 = 0.80;
constexpr double CRIT_CHANCE_D1_2 = 0.28;
constexpr double CRIT_CHANCE_GN_1 = 0.79;
constexpr double CRIT_CHANCE_GN_2 = 0.26;

// Crit Mult
constexpr double CRIT_EQ_1 = 0.88;
constexpr double CRIT_EQ_2 = 0.5;
constexpr double CRIT_D1_1 = 0.88;
constexpr double CRIT_D1_2 = 0.44;
constexpr double CRIT_GN_1 = 0.88;
constexpr double CRIT_GN_2 = 0.44;

constexpr double CRIT_NEQ_1 = 0.88;
constexpr double CRIT_NEQ_2 = 0.44;

// Chain Hit
constexpr double CHAINHIT_EQ_1 = 0.88;
constexpr double CHAINHIT_EQ_2 = 0.50;
constexpr double CHAINHIT_D1_1 = 0.90;
constexpr double CHAINHIT_D1_2 = 0.47;
constexpr double CHAINHIT_GN_1 = 0.92;
constexpr double CHAINHIT_GN_2 = 0.44;

// Leech
constexpr double LEECH_EQ_1 = 0.88;
constexpr double LEECH_EQ_2 = 0.50;
constexpr double LEECH_D1_1 = 0.89;
constexpr double LEECH_D1_2 = 0.44;
constexpr double LEECH_GN_1 = 0.90;
constexpr double LEECH_GN_2 = 0.38;

// Armor Tearing
constexpr double ARMOR_EQ_1 = 0.94;
constexpr double ARMOR_EQ_2 = 0.69;
constexpr double ARMOR_D1_1 = 0.95;
constexpr double ARMOR_D1_2 = 0.57;
constexpr double ARMOR_GN_1 = 0.96;
constexpr double ARMOR_GN_2 = 0.45;

// Poolbound
constexpr double PBOUND_EQ_1 = 0.87;
constexpr double PBOUND_EQ_2 = 0.38;
constexpr double PBOUND_D1_1 = 0.87;
constexpr double PBOUND_D1_2 = 0.38;
constexpr double PBOUND_GN_1 = 0.87;
constexpr double PBOUND_GN_2 = 0.38;

constexpr double PBOUND_1 = 0.87;
constexpr double PBOUND_2 = 0.38;

// Suppressive
constexpr double SUPPR_EQ_1 = 0.96;
constexpr double SUPPR_EQ_2 = 1.91;
constexpr double SUPPR_D1_1 = 0.92;
constexpr double SUPPR_D1_2 = 1.13;
constexpr double SUPPR_GN_1 = 0.92;
constexpr double SUPPR_GN_2 = 0.73;

// Bleed
constexpr double BLEED_EQ_1 = 0.81;
constexpr double BLEED_EQ_2 = 0.35;
constexpr double BLEED_D1_1 = 0.80;
constexpr double BLEED_D1_2 = 0.28;
constexpr double BLEED_GN_1 = 0.79;
constexpr double BLEED_GN_2 = 0.26;

// -------------------------
// Damage (relative to Crit)
// -------------------------

constexpr double DAMAGE_POISON = 10./11.;
constexpr double DAMAGE_CRIT   = 11./11.;
constexpr double DAMAGE_BBOUND = 13./11.;
constexpr double DAMAGE_SLOW   = 12./11.;
constexpr double DAMAGE_CHHIT  = 10./11.;
constexpr double DAMAGE_LEECH  =  8./11.;
constexpr double DAMAGE_ARMOR  = 11./11.;
constexpr double DAMAGE_SUPPR  = 12./11.;
constexpr double DAMAGE_PBOUND = 12./11.;

constexpr double DAMAGE_LEECH_GCFW =  9./11.;
constexpr double DAMAGE_BLEED      = 12./11.;

// -------------------------
// Colors (single char)
// -------------------------

constexpr char COLOR_POISON = 'G';
constexpr char COLOR_CRIT   = 'y';
constexpr char COLOR_BBOUND = 'b';
constexpr char COLOR_SLOW   = 'B';
constexpr char COLOR_CHHIT  = 'r';
constexpr char COLOR_LEECH  = 'o';
constexpr char COLOR_ARMOR  = 'p';
constexpr char COLOR_SUPPR  = 'c';
constexpr char COLOR_PBOUND = 'w';

constexpr char COLOR_MANAGEM = 'm';
constexpr char COLOR_KILLGEM = 'k';
constexpr char COLOR_SBGEM   = 'h';

constexpr char COLOR_BLEED   = 'r';

constexpr char COLOR_GENERIC = 'g';
constexpr char COLOR_UNKNOWN = 'x';

#endif // _GEM_STATS_H
