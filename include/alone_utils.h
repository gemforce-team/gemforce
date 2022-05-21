#ifndef _ALONE_UTILS_H
#define _ALONE_UTILS_H

#include <cstdlib>

#include "cmdline_options.h"
#include "container_utils.h"
#include "print_utils.h"

inline void print_build_iteration(const cmdline_options& options, int value, size_t pool_length, size_t comb_tot)
{
    if (!options.output.quiet) {
        printf("Value:\t%d\n", value);
        if (options.output.debug) {
            printf("Pool:\t%zu\n", pool_length);
			printf("Raw:\t%zu\n\n", comb_tot);
        }
    }
}

template<class gem>
inline void print_winner_iteration(const cmdline_options& options, int value, const gem& winner,
                                   size_t pool_length, size_t comb_tot = 0)
{
	if (!options.output.quiet) {
		printf("Value:\t%d\n", value);
		if (options.output.info)
			printf("Growth:\t%f\n", log(gem_power(winner))/log(value));
		if (options.output.debug) {
			printf("Pool:\t%zu\n",pool_length);
			if (comb_tot)
				printf("Raw:\t%zu\n",comb_tot);
		}
		gem_print(&winner);
	}
}

// outputs last if we never seen any
template<class gem>
inline void print_winner(const cmdline_options& options, int value, const gem& winner, size_t pool_length)
{
	if (options.output.quiet) {
		printf("Value:\t%d\n", value);
		printf("Growth:\t%f\n", log(gem_power(winner))/log(value));
		if (options.output.debug)
			printf("Pool:\t%zu\n", pool_length);
		gem_print(&winner);
	}
}

template<class gem>
inline gem* find_best_upto_alone(vector<gem>& gems)
{
	double best_growth=-INFINITY;
	size_t best_index=0;
	for (size_t i = 0; i < gems.size(); ++i) {
		if (log(gem_power(gems[i]))/log(i+1) > best_growth) {
			best_index=i;
			best_growth=log(gem_power(gems[i]))/log(i+1);
		}
	}
	printf("Best gem up to %zu:\n\n", gems.size());
	printf("Value:\t%zu\n",best_index+1);
	printf("Growth:\t%f\n", best_growth);
	gem_print(gems+best_index);
	return gems+best_index;
}

template<class gem>
inline void pretty_print_alone(const vector<gem>& gems, gem* gemf, const cmdline_options& options)
{
	if (options.print.parens) {
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
	}
	if (options.print.tree) {
		printf("Gem tree:\n");
		print_tree(gemf, "");
		printf("\n");
	}
	if (options.print.table) print_table(gems);
	
	if (options.print.equations) {
		printf("Equations:\n");
		print_equations(gemf);
		printf("\n");
	}
}

#endif // _ALONE_UTILS_H
