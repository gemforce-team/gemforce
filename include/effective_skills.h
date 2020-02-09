#ifndef INCLUDE_EFFECTIVE_SKILLS_H_
#define INCLUDE_EFFECTIVE_SKILLS_H_

#include "cmdline_options.h"

/*
 * All the amplification formulae of the game are in blocks of
 * (Sg + Rs * Sa), this file computes that Rs for various situations
 */

inline double TC_pure_gccs(int TC)
{
	return 1 + 0.03 * TC;
}

inline double TCd_triple_gccs(int TC)
{
	return 1.2 + TC / 3 * 0.1;
}

inline double TCs_triple_gccs(int TC)
{
	return 0.5 * (1 + TC / 3 * 0.1);
}

inline double amps_skill_gccs(int as)
{
	return (0.15 + as / 3 * 0.004);
}

inline double special_ratio_gccs(const cmdline_options& options)
{
	int TC = options.skills.TC;
	int amps = options.skills.amps;
	return options.amps.number_per_gem * options.amps.average_gems_seen * amps_skill_gccs(amps) * TC_pure_gccs(TC) / TCs_triple_gccs(TC);
}

inline double damage_ratio_gccs(const cmdline_options& options)
{
	int TC = options.skills.TC;
	int amps = options.skills.amps;
	return options.amps.number_per_gem * options.amps.average_gems_seen * amps_skill_gccs(amps) * TC_pure_gccs(TC) / TCd_triple_gccs(TC);
}


#endif /* INCLUDE_EFFECTIVE_SKILLS_H_ */
