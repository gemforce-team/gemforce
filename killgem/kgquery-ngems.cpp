#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "killgem_utils.h"
#include "crit_utils.h"
#include "kga_utils.h"
#include "effective_skills.h"
#include "gfon.h"
#include "print_utils.h"
#include "cmdline_options.h"

using gem = gem_YB;
using gemA = gem_Y;

void print_ngems_table(const gem* gems, const gemA* amps, double damage_ratio, double crit_ratio, int len)
{
	printf("# Gems\tKillgem\tAmps\tPower\n");
	for (int i =0; i < len; i++)
		printf("%d\t%d\t%d\t%#.7g\n", i+1, gem_getvalue(gems+i), gem_getvalue(amps+i), gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
	printf("\n");
}

void worker(const cmdline_options& options)
{
	FILE* table = file_check(options.tables[0]);	// file is open to read
	if (table == NULL) exit(1);						// if the file is not good we exit

	int len = options.target.len;
	vector pool = init_pool<gem>(len, 2);
	vector pool_length = init_pool_length(len, 2);
	
	int prevmax = pool_from_table(pool, pool_length, options.tuning.spec_limit ? options.tuning.spec_limit : len, table);	// killgem pool filling
	fclose(table);
	if (prevmax < len-1) {					// if the killgems are not enough
		if (prevmax == -1) exit(1);
		for (int i = prevmax+1; i < len; ++i) {
			pool_length[i]=0;
			pool[i]=NULL;
		}
	}

	vector poolf = vector<pool_t<gem>>(len);
	vector poolf_length = vector<size_t>(len);
	
	specs_compression(poolf, poolf_length, pool, pool_length, len, options.output.debug);
	if (!options.output.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(options.tables[1]);	// fileA is open to read
	if (tableA == NULL) exit(1);				// if the file is not good we exit
	
	int lena = options.amps.number_per_gem ? len/options.amps.number_per_gem : 1;		// if options.amps.number_per_gem==0 let lena=1
	vector poolA = init_pool<gemA>(lena);
	vector poolA_length = init_pool_length(lena);
	
	int prevmaxA = pool_from_table(poolA, poolA_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA < lena-1) {
		poolA.~vector();
		poolA_length.~vector();
		if (prevmaxA != -1) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	vector poolAf = vector<pool_t<gemA>>(len);
	vector poolAf_length = vector<size_t>(len);
	
	amps_compression(poolAf, poolAf_length, poolA, poolA_length, lena, options.output.debug);
	if (!options.output.quiet) printf("Amp pool compression done!\n\n");

	// let's choose the right gem-amp combo
	gem gems[len];
	gemA amps[len];
	gem_init(gems,1,1,1,0);
	amps[0] = {};
	double crit_ratio   = special_ratio_gccs(options);
	double damage_ratio = damage_ratio_gccs(options);
	
	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i = first; i < len; ++i) {						// for every total value
		gems[i] = {};										// we init the gems
		amps[i] = {};										// to extremely weak ones
		for (size_t k=0;k<poolf_length[i];++k) {			// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		double power = gem_power(gems[i]);
		if (options.amps.number_per_gem!=0)
		for (int j=1;j<=i/options.amps.number_per_gem;++j) {	// for every amount of amps we can fit in
			int value = i-options.amps.number_per_gem*j;		// this is the amount of gems we have left
			for (size_t k=0; k<poolf_length[value]; ++k) {		// we search in that pool
				for (size_t h=0;h<poolAf_length[j-1];++h) {		// and we look in the amp pool
					if (gem_amp_power(poolf[value][k], poolAf[j-1][h], damage_ratio, crit_ratio) > power)
					{
						power = gem_amp_power(poolf[value][k], poolAf[j-1][h], damage_ratio, crit_ratio);
						gems[i]=poolf[value][k];
						amps[i]=poolAf[j-1][h];
					}
				}
			}
		}
		if (!options.output.quiet) {
			printf("Total value:\t%d\n\n", i+1);
			if (prevmax < len-1) printf("Killgem limit:\t%d\n", prevmax+1);
			printf("Killgem\n");
			printf("Value:\t%d\n",gem_getvalue(gems+i));
			if (options.output.debug) printf("Pool:\t%zu\n", poolf_length[gem_getvalue(gems+i)-1]);
			gem_print(gems+i);
			printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			if (options.output.debug) printf("Pool:\t%zu\n", poolAf_length[gem_getvalue(amps+i)-1]);
			gem_print(amps+i);
			printf("Spec base power: \t%#.7g\n\n", gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
		}
	}
	
	if (options.output.quiet) {		// outputs last if we never seen any
		printf("Total value:\t%d\n\n", len);
		if (prevmax < len-1) printf("Killgem limit:\t%d\n", prevmax+1);
		printf("Killgem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+len-1));
		gem_print(gems+len-1);
		printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n", gem_getvalue(amps+len-1));
		gem_print(amps+len-1);
		printf("Spec base power: \t%#.7g\n\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
	}

	gem*  gemf=gems+len-1;  // gem that will be displayed
	gemA* ampf=amps+len-1;  // amp that will be displayed

	vector<gem> chain_gems;
	if (options.target.chain) {
		if (len < 3) printf("I could not add chain!\n\n");
		else {
			int value = gem_getvalue(gemf);
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], chain_gems, damage_ratio*ampf->damage, crit_ratio*ampf->crit);
			printf("Setup with chain added:\n\n");
			printf("Total value:\t%d\n\n", value+options.amps.number_per_gem*gem_getvalue(ampf));
			printf("Killgem\n");
			printf("Value:\t%d\n", value);
			gem_print(gemf);
			printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n", gem_getvalue(ampf));
			gem_print(ampf);
			printf("Spec base power with chain:\t%#.7g\n\n", gem_amp_power(*gemf, *ampf, damage_ratio, crit_ratio));
		}
	}

	if (options.print.parens) {
		printf("Killgem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed(ampf);
		printf("\n\n");
	}
	if (options.print.tree) {
		printf("Killgem tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree(ampf, "");
		printf("\n");
	}
	if (options.print.table) print_ngems_table(gems, amps, damage_ratio, crit_ratio, len);

	
	if (options.print.equations) {		// it ruins gems, must be last
		printf("Killgem equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations(ampf);
		printf("\n");
	}
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();
	options.has_amps();
	options.has_spec_limit();
	options.has_nonpures();
	options.set_num_tables(2);

	options.skills.TC = 120;
	options.skills.amps = 60;
	options.amps.number_per_gem = 8;

	if(!options.parse_args(argc, argv))
		return 1;
	options.table_selection(0, "table_kgspec");
	options.table_selection(1, "table_crit");

	worker(options);
	return 0;
}
