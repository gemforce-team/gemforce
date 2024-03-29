#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "interval_tree.h"
#include "killgem_utils.h"
#include "crit_utils.h"
#include "kga_utils.h"
#include "effective_skills.h"
#include "gfon.h"
#include "print_utils.h"
#include "cmdline_options.h"
#include "ampscomb_utils.h"

using gem = gem_YB;
using gemA = gem_Y;

void worker(const cmdline_options& options)
{
	FILE* table = file_check(options.tables[0]);	// file is open to read
	if (table == NULL) exit(1);						// if the file is not good we exit

	int len = options.target.len;
	vector pool = init_pool<gem>(len, 2);
	vector pool_length = init_pool_length(len, 2);
	
	int prevmax = pool_from_table(pool, pool_length, len, table);		// killgem spec pool filling
	fclose(table);
	if (prevmax < len-1) {										// if the killgems are not enough
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
	
	vector poolAf = init_pool<gemA>(lena);
	vector poolAf_length = init_pool_length(lena);
	
	amps_compression(poolAf, poolAf_length, poolA, poolA_length, lena, options.output.debug);
	if (!options.output.quiet) printf("Amp pool compression done!\n");

	FILE* tablec = file_check(options.tables[2]);	// file is open to read
	if (tablec==NULL) exit(1);					// if the file is not good we exit
	
	int lenc = options.target.lenc;
	vector poolc = init_pool<gem>(len, 1);
	vector poolc_length = init_pool_length(len, 1);
	
	int prevmaxc=pool_from_table(poolc, poolc_length, lenc, tablec);	// killgem comb pool filling
	fclose(tablec);
	if (prevmaxc<lenc-1) {									// if the killgems are not enough
		poolc.~vector();
		poolc_length.~vector();
		if (prevmaxc != -1) printf("Gem table stops at %d, not %d\n",prevmaxc+1,lenc);
		exit(1);
	}
	
	gem bestc = {};				// choosing best combine
	
	compression_1D(&bestc, poolc[lenc -1], poolc_length[lenc-1]);

	double bestc_growth=log(gem_power(bestc))/log(lenc);
	
	if (!options.output.quiet) printf("Combining pool compression done!\n\n");

	// let's choose the right gem-amp combo
	gem gems[len];						// for every speccing value
	gemA amps[len];						// we'll choose the best amps
	double powers[len];
	gem_init(gems,1,1,1,0);
	gem_init(amps,0,0,0);
	powers[0]=0;
	double crit_ratio   = special_ratio_gccs(options);
	double damage_ratio = damage_ratio_gccs(options);
	double NT = pow(2, options.tuning.final_eq_grade - 1);

	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i = first; i < len; ++i) {								// for every gem value
		gems[i] = {};												// we init the gems
		amps[i] = {};												// to extremely weak ones
		
		for (size_t k=0;k<poolf_length[i];++k) {					// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int j;
		int NS=i+1;
		double C0 = pow(NT/(i+1), bestc_growth);					// last we compute the combination number
		powers[i] = C0 * gem_power(gems[i]);
																	// now we compare the whole setup
		int amps_bound = options.tuning.max_ag_cost_ratio * (i + 1);
		for (j=0, NS+=options.amps.number_per_gem; j<amps_bound; ++j, NS+=options.amps.number_per_gem) {
			double Cg = pow(NT/NS, bestc_growth);					// we compute the combination number
			for (size_t k=0;k<poolf_length[i];++k) {						// then in the gem pool
				double Pb2 = poolf[i][k].bbound * poolf[i][k].bbound;
				double Pdg = poolf[i][k].damage;
				double Pcg = poolf[i][k].crit  ;
				for (size_t h=0;h<poolAf_length[j];++h) {					// and in the reduced amp pool
					double Pdamage = Pdg + damage_ratio* poolAf[j][h].damage ;
					double Pcrit   = Pcg + crit_ratio  * poolAf[j][h].crit   ;
					double Pbase   = Pb2 * Pdamage * Pcrit ;
					double power   = Cg * Pbase;
					if (power>powers[i]) {
						powers[i]=power;
						gems[i]=poolf[i][k];
						amps[i]=poolAf[j][h];
					}
				}
			}
		}
		if (!options.output.quiet) {
			printf("Killgem spec\n");
			printf("Value:\t%d\n",i+1);
			if (options.output.debug) printf("Pool:\t%zu\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			if (options.output.debug) printf("Pool:\t%zu\n", poolAf_length[gem_getvalue(amps+i)-1]);
			gem_print(amps+i);
			printf("Setup combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(&bestc);
			printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
			printf("Global power at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, powers[i]);
		}
	}
	
	if (options.output.quiet) {		// outputs last if we never seen any
		printf("Killgem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n",gem_getvalue(amps+len-1));
		gem_print(amps+len-1);
		printf("Setup combine\n");
		printf("Comb:\t%d\n",lenc);
		printf("Growth:\t%f\n", bestc_growth);
		gem_print(&bestc);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, powers[len-1]);
	}

	gem*  gemf = gems+len-1;  // gem  that will be displayed
	gemA* ampf = amps+len-1;  // amp  that will be displayed
	gem*  gemfc= &bestc;      // gemc that will be displayed

	if (options.target.upto) {
		double best_pow=0;
		int best_index=0;
		for (int i =0; i < len; ++i) {
			if (powers[i] > best_pow) {
				best_index=i;
				best_pow=powers[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Killgem spec\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n", gem_getvalue(amps+best_index));
		gem_print(amps+best_index);
		printf("Setup combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemfc);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], damage_ratio, crit_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, powers[best_index]);
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
			double amp_damage_scaled = damage_ratio * ampf->damage;
			double amp_crit_scaled = crit_ratio * ampf->crit;
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], chain_gems, amp_damage_scaled, amp_crit_scaled);
			printf("Setup with chain added:\n\n");
			printf("Killgem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n", valueA);
			gem_print(ampf);
			printf("Setup combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Spec base power with chain:\t%#.7g\n", gem_amp_power(*gemf, *ampf, damage_ratio, crit_ratio));
			double CgP = pow(NT/NS, bestc_growth);
			printf("Global power w. chain at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade,
				   CgP * gem_cfr_power(*gemf, amp_damage_scaled, amp_crit_scaled));
		}
	}

	if (options.print.parens) {
		printf("Killgem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed(ampf);
		printf("\n\n");
		printf("Setup combining scheme:\n");
		print_parens_compressed(gemfc);
		printf("\n\n");
	}
	if (options.print.tree) {
		printf("Killgem speccing tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree(ampf, "");
		printf("\n");
		printf("Setup combining tree:\n");
		print_tree(gemfc, "");
		printf("\n");
	}
	if (options.print.table) print_omnia_table("Killgem", amps, powers, len);
	
	if (options.print.equations) {		// it ruins gems, must be last
		printf("Killgem speccing equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations(ampf);
		printf("\n");
		printf("Setup combining equations:\n");
		print_equations(gemfc);
		printf("\n");
	}
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();
	options.has_amps();
	options.has_nonpures();
	options.has_final_eq_grade();
	options.set_num_tables(3);
	options.has_lenc();

	options.skills.TC = 120;
	options.skills.amps = 60;
	options.amps.number_per_gem = 8;

	if(!options.parse_args(argc, argv))
		return 1;
	options.table_selection(0, "table_kgspec");
	options.table_selection(1, "table_crit");
	options.table_selection(2, "table_kgcomb");

	worker(options);
	return 0;
}
