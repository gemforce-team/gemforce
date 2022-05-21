#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "managem_utils.h"
#include "leech_utils.h"
#include "mga_utils.h"
#include "effective_skills.h"
#include "gfon.h"
#include "print_utils.h"
#include "cmdline_options.h"
#include "1D_utils.h"
#include "2D_utils.h"

using gem = gem_OB;
using gemA = gem_O;

void print_amps_table(const gem* gems, const gemA* amps, const double* spec_coeffs, double leech_ratio, int len)
{
	printf("Managem\tAmps\tPower\t\tSpec coeff\n");
	for (int i=0; i < len; i++)
		printf("%d\t%d\t%#.7g\t%f\n", i+1, gem_getvalue(amps+i), gem_amp_power(gems[i], amps[i], leech_ratio), spec_coeffs[i]);
	printf("\n");
}

void worker(const cmdline_options& options)
{
	FILE* table = file_check(options.tables[0]);	// file is open to read
	if (table == NULL) exit(1);						// if the file is not good we exit

	int len = options.target.len;
	vector pool = init_pool<gem>(len, 2);
	vector pool_length = init_pool_length(len, 2);
	
	int prevmax = pool_from_table(pool, pool_length, len, table);		// managem pool filling
	fclose(table);
	if (prevmax < len-1) {										// if the managems are not enough
		pool.~vector();
		pool_length.~vector();
		if (prevmax != -1) printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	vector poolf = vector<pool_t<gem>>(len);
	vector poolf_length = vector<size_t>(len);
	
	specs_compression(poolf, poolf_length, pool, pool_length, len, options.output.debug);
	if (!options.output.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(options.tables[1]);	// fileA is open to read
	if (tableA == NULL) exit(1);					// if the file is not good we exit
	
	int lena = options.tuning.max_ag_cost_ratio * len;
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

	vector bestA = vector<gemA>(lena);
	
	amps_compression(bestA, poolA, poolA_length, lena);
	if (!options.output.quiet) printf("Amp pool compression done!\n\n");

	// let's choose the right gem-amp combo
	gem gems[len];
	gemA amps[len];
	double spec_coeffs[len];
	gem_init(gems,1,1,0);
	amps[0] = {};
	spec_coeffs[0]=0;
	double leech_ratio = special_ratio_gccs(options);
	double growth_comb = options.tuning.combine_growth;
	
	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i = first; i < len; ++i) {						// for every gem value
		gems[i] = {};										// we init the gems
		amps[i] = {};										// to extremely weak ones
		for (size_t k=0;k<poolf_length[i];++k) {			// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int j;
		int NS=i+1;
		double comb_coeff=pow(NS, -growth_comb);
		spec_coeffs[i]=comb_coeff*gem_power(gems[i]);

		int amps_bound = options.tuning.max_ag_cost_ratio * (i + 1);	// now with amps
		for (j = 0, NS += options.amps.number_per_gem; j < amps_bound; ++j, NS += options.amps.number_per_gem) {
			comb_coeff = pow(NS, -growth_comb);				// we compute comb_coeff
			double Pa = leech_ratio * bestA[j].leech;		// <- this is ok only for mg
			for (size_t k=0; k<poolf_length[i]; ++k) {		// then we search in the reduced gem pool
				double Palone = gem_power(poolf[i][k]);
				double power = Palone + poolf[i][k].bbound * Pa;
				double spec_coeff=power*comb_coeff;
				if (spec_coeff>spec_coeffs[i]) {
					spec_coeffs[i]=spec_coeff;
					gems[i]=poolf[i][k];
					amps[i]=bestA[j];
				}
			}
		}
		if (!options.output.quiet) {
			printf("Total value:\t%d\n\n", i + 1 + options.amps.number_per_gem * gem_getvalue(amps + i));
			printf("Managem\n");
			printf("Value:\t%d\n",i+1);
			if (options.output.debug) printf("Pool:\t%zu\n", poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			gem_print(amps+i);
			printf("Spec base power: \t%#.7g\n", gem_amp_power(gems[i], amps[i], leech_ratio));
			printf("Spec coefficient:\t%f\n\n", spec_coeffs[i]);
		}
	}
	
	if (options.output.quiet) {		// outputs last if we never seen any
		printf("Total value:\t%d\n\n", len+options.amps.number_per_gem*gem_getvalue(amps+len-1));
		printf("Managem\n");
		printf("Value:\t%d\n", len);
		gem_print(gems+len-1);
		printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n", gem_getvalue(amps+len-1));
		gem_print(amps+len-1);
		printf("Spec base power: \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], leech_ratio));
		printf("Spec coefficient:\t%f\n\n", spec_coeffs[len-1]);
	}

	gem*  gemf=gems+len-1;  // gem that will be displayed
	gemA* ampf=amps+len-1;  // amp that will be displayed

	if (options.target.upto) {
		double best_sc=0;
		int best_index=0;
		for (int i = 0; i < len; ++i) {
			if (spec_coeffs[i] > best_sc) {
				best_index=i;
				best_sc=spec_coeffs[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Total value:\t%d\n\n", gem_getvalue(gems+best_index)+options.amps.number_per_gem*gem_getvalue(amps+best_index));
		printf("Managem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n", gem_getvalue(amps+best_index));
		gem_print(amps+best_index);
		printf("Spec base power: \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], leech_ratio));
		printf("Spec coefficient:\t%f\n\n", best_sc);
		gemf = gems+best_index;
		ampf = amps+best_index;
	}

	vector<gem> chain_gems;
	if (options.target.chain) {
		if (len < 3) printf("I could not add chain!\n\n");
		else {
			int value = gem_getvalue(gemf);
			int valueA= gem_getvalue(ampf);
			double NS = value + options.amps.number_per_gem*valueA;
			double amp_leech_scaled = leech_ratio * ampf->leech;
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], chain_gems, amp_leech_scaled);
			printf("Setup with chain added:\n\n");
			printf("Total value:\t%d\n\n", value+options.amps.number_per_gem*gem_getvalue(ampf));
			printf("Managem\n");
			printf("Value:\t%d\n", value);
			gem_print(gemf);
			printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n", gem_getvalue(ampf));
			gem_print(ampf);
			printf("Spec base power w. chain:\t%#.7g\n", gem_amp_power(*gemf, *ampf, leech_ratio));
			double CgP = pow(NS, -growth_comb);
			printf("Spec coefficient:\t%f\n\n", CgP*gem_cfr_power(*gemf, amp_leech_scaled));
		}
	}

	if (options.print.parens) {
		printf("Managem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed(ampf);
		printf("\n\n");
	}
	if (options.print.tree) {
		printf("Managem tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree(ampf, "");
		printf("\n");
	}
	if (options.print.table) print_amps_table(gems, amps, spec_coeffs, leech_ratio, len);
	
	if (options.print.equations) {		// it ruins gems, must be last
		printf("Managem equations:\n");
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
	options.has_combine_growth();
	options.has_nonpures();
	options.set_num_tables(2);

	options.skills.TC = 120;
	options.skills.amps = 60;
	options.amps.number_per_gem = 2;      // multiple gems in trap
	options.amps.average_gems_seen = 2.5; // with amps on the side
	options.tuning.combine_growth = 0.627216;	// 16c

	if(!options.parse_args(argc, argv))
		return 1;
	options.table_selection(0, "table_mgspec");
	options.table_selection(1, "table_leech");
	options.tuning.max_ag_cost_ratio = std::max(options.tuning.max_ag_cost_ratio,
		1. + (special_ratio_gccs(options) > 2.5)); // A100 = 2 with defaults

	worker(options);
	return 0;
}
