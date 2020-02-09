#ifndef INCLUDE_EFFECTIVE_SKILLS_H_
#define INCLUDE_EFFECTIVE_SKILLS_H_

#include "cmdline_options.h"

/*
 * All the amplification formulae of the game are in blocks of
 * (Sg + Rs * Sa), this file computes that Rs for various situations
 */

//-------
// Common
//-------

enum game
{
	GCCS,
	GCFW,
};

inline double TC_pure(int TC)
{
	return 1 + 0.03 * TC;
}

inline double amps_speed(int as)
{
	return 0.10 + as * 0.004;
}

//-----
// GCCS
//-----

inline double TCd_triple_gccs(int TC)
{
	return 1.2 + TC / 3 * 0.1;
}

inline double TCs_triple_gccs(int TC)
{
	return 0.5 * (1 + TC / 3 * 0.1);
}

inline double amps_special_gccs(int as)
{
	return 0.15 + as / 3 * 0.004;
}

inline double amps_damage_gccs(int as)
{
	return 0.20 + as / 3 * 0.004;
}

inline double special_ratio_gccs(const cmdline_options& options)
{
	int TC = options.skills.TC;
	int amps = options.skills.amps;
	return options.amps.number_per_gem * options.amps.average_gems_seen * amps_special_gccs(amps) * TC_pure(TC) / TCs_triple_gccs(TC);
}

inline double damage_ratio_gccs(const cmdline_options& options)
{
	int TC = options.skills.TC;
	int amps = options.skills.amps;
	return options.amps.number_per_gem * options.amps.average_gems_seen * amps_damage_gccs(amps) * TC_pure(TC) / TCd_triple_gccs(TC);
}

//-----
// GCFW
//-----

inline double amps_special_gcfw(int as)
{
	return 0.15 + as / 5 * 0.004;
}

inline double amps_damage_gcfw(int as)
{
	return 0.20 + as / 5 * 0.004;
}

inline double special_ratio_gcfw(const cmdline_options& options)
{
	int amps = options.skills.amps;
	return options.amps.number_per_gem * options.amps.average_gems_seen * amps_special_gcfw(amps);
}

inline double damage_ratio_gcfw(const cmdline_options& options)
{
	int amps = options.skills.amps;
	return options.amps.number_per_gem * options.amps.average_gems_seen * amps_damage_gcfw(amps);
}

#endif /* INCLUDE_EFFECTIVE_SKILLS_H_ */
