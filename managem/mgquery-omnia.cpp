#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "managem_utils.h"
#include "leech_utils.h"
#include "mga_utils.h"
#include "gfon.h"
#include "print_utils.h"
#include "effective_skills.h"
#include "cmdline_options.h"

using gem = gem_OB;
using gemA = gem_O;

void worker(const cmdline_options& options)
{
	FILE* table = file_check(options.tables[0]);	// file is open to read
	if (table==NULL) exit(1);						// if the file is not good we exit

	int len = options.target.len;
	int lenc = options.target.lenc;
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(2 * sizeof(gem));
	pool_length[0] = 2;
	gem_init(pool[0]  ,1,1,0);
	gem_init(pool[0]+1,1,0,1);
	
	int prevmax = pool_from_table(pool, pool_length, len, table);		// managem spec pool filling
	fclose(table);
	if (prevmax<len-1) {										// if the managems are not enough
		for (int i =0;i<=prevmax;++i) free(pool[i]);		// free
		if (prevmax>0) printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	
	gem* poolf[len];
	int poolf_length[len];
	
	specs_compression(poolf, poolf_length, pool, pool_length, len, options.output.debug);
	if (!options.output.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(options.tables[1]);	// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena = std::max(int(options.tuning.max_ag_cost_ratio * len), lenc);
	gemA** poolA = (gemA**)malloc(lena*sizeof(gemA*));
	int* poolA_length = (int*)malloc(lena*sizeof(int));
	poolA[0] = (gemA*)malloc(sizeof(gemA));
	poolA_length[0]=1;
	gem_init(poolA[0],1,1);
	
	int prevmaxA = pool_from_table(poolA, poolA_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (int i =0;i<=prevmaxA;++i) free(poolA[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}
	
	gemA* bestA = (gemA*)malloc(lena*sizeof(gemA));		// if not malloc-ed 140k is the limit
	
	amps_compression(bestA, poolA, poolA_length, lena);
	gemA combA=bestA[lenc-1];			// amps fast access combine
	if (!options.output.quiet) printf("Amp pool compression done!\n");

	FILE* tablec=file_check(options.tables[2]);		// file is open to read
	if (tablec==NULL) exit(1);					// if the file is not good we exit
	gem** poolc = (gem**)malloc(lenc*sizeof(gem*));
	int* poolc_length = (int*)malloc(lenc*sizeof(int));
	poolc[0] = (gem*)malloc(sizeof(gem));
	poolc_length[0]=1;
	gem_init(poolc[0],1,1,1);
	
	int prevmaxc=pool_from_table(poolc, poolc_length, lenc, tablec);		// managem comb pool filling
	fclose(tablec);
	if (prevmaxc<lenc-1) {									// if the managems are not enough
		for (int i =0;i<=prevmaxc;++i) free(poolc[i]);		// free
		if (prevmaxc>0) printf("Gem table stops at %d, not %d\n",prevmaxc+1,lenc);
		exit(1);
	}
	
	gem* poolcf;
	int poolcf_length;
	
	combs_compression(&poolcf, &poolcf_length, poolc[lenc-1], poolc_length[lenc-1]);
	if (!options.output.quiet) printf("Gem combine compressed pool size:\t%d\n\n",poolcf_length);

	// let's choose the right gem-amp combo
	gem gems[len];						// for every speccing value
	gemA amps[len];						// we'll choose the best amps
	gem gemsc[len];						// and the best NC combine
	gemA ampsc[len];					// for both
	double powers[len];
	gem_init(gems, 1, 1, 0);
	gem_init(amps, 0, 0);
	gem_init(gemsc, 1, 0, 0);
	gem_init(ampsc, 0, 0);
	powers[0] = 0;
	double iloglenc = 1 / log(lenc);
	double leech_ratio = special_ratio_gccs(options);
	double NT = pow(2, options.tuning.final_eq_grade - 1);
	
	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i =first; i<len; ++i) {						// for every gem value
		gems[i] = {};												// we init the gems
		amps[i] = {};												// to extremely weak ones
		gemsc[i] = {};
		ampsc[i]=combA;												// <- this is ok only for mg
																	// first we compare the gem alone
		for (int l=0; l<poolcf_length; ++l) {						// first search in the NC gem comb pool
			if (gem_power(poolcf[l]) > gem_power(gemsc[i])) {
				gemsc[i]=poolcf[l];
			}
		}
		for (int k=0;k<poolf_length[i];++k) {						// and then in the the gem pool
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int j;
		int NS=i+1;
		double c0 = log(NT/(i+1))*iloglenc;							// last we compute the combination number
		powers[i] = pow(gem_power(gemsc[i]),c0) * gem_power(gems[i]);
																	// now we compare the whole setup
		int amps_bound = options.tuning.max_ag_cost_ratio * (i + 1);
		for (j=0, NS+=options.amps.number_per_gem; j<amps_bound; ++j, NS+=options.amps.number_per_gem) {
			double c = log(NT/NS)*iloglenc;							// we compute the combination number
			double Ca= leech_ratio * pow(combA.leech,c);			// <- this is ok only for mg
			double Pa= Ca * bestA[j].leech;							// <- because we already know the best amps
			for (int l=0; l<poolcf_length; ++l) {					// then we search in NC gem comb pool
				double Cbg = pow(poolcf[l].bbound,c);
				double Cg  = pow(gem_power(poolcf[l]),c);
				for (int k=0; k<poolf_length[i]; ++k) {			// and in the reduced gem pool
					double Palone = Cg * gem_power(poolf[i][k]);
					double Pbg = Cbg * poolf[i][k].bbound;
					double power = Palone + Pbg * Pa;  
					if (power>powers[i]) {
						powers[i]=power;
						gems[i]=poolf[i][k];
						amps[i]=bestA[j];
						gemsc[i]=poolcf[l];
					}
				}
			}
		}
		if (!options.output.quiet) {
			printf("Managem spec\n");
			printf("Value:\t%d\n",i+1);
			if (options.output.debug) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			gem_print(amps+i);
			printf("Managem combine\n");
			printf("Comb:\t%d\n",lenc);
			if (options.output.debug) printf("Pool:\t%d\n",poolcf_length);
			gem_print(gemsc+i);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(ampsc+i);
			printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[i], amps[i], leech_ratio));
			printf("Global power at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, powers[i]);
		}
	}
	
	if (options.output.quiet) {		// outputs last if we never seen any
		printf("Managem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n",gem_getvalue(amps+len-1));
		gem_print(amps+len-1);
		printf("Managem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+len-1);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(ampsc+len-1);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], leech_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, powers[len-1]);
	}

	gem*  gemf = gems+len-1;  // gem  that will be displayed
	gemA* ampf = amps+len-1;  // amp  that will be displayed
	gem*  gemfc=gemsc+len-1;  // gemc that will be displayed
	gemA* ampfc=ampsc+len-1;  // ampc that will be displayed

	if (options.target.upto) {
		double best_pow=0;
		int best_index=0;
		for (int i =0; i<len; ++i) {
			if (powers[i] > best_pow) {
				best_index=i;
				best_pow=powers[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Managem spec\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n", gem_getvalue(amps+best_index));
		gem_print(amps+best_index);
		printf("Managem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+best_index);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(ampsc+best_index);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], leech_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, powers[best_index]);
		gemf = gems+best_index;
		ampf = amps+best_index;
		gemfc = gemsc+best_index;
		ampfc = ampsc+best_index;
	}

	gem* gem_array = NULL;
	if (options.target.chain) {
		if (len < 3) printf("I could not add chain!\n\n");
		else {
			int value = gem_getvalue(gemf);
			int valueA= gem_getvalue(ampf);
			double NS = value + options.amps.number_per_gem*valueA;
			double c = log(NT/NS)*iloglenc;
			double amps_resc_coeff = pow((ampfc->leech/gemfc->leech), c);
			double amp_leech_scaled = leech_ratio*amps_resc_coeff*ampf->leech;
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], &gem_array, amp_leech_scaled);
			printf("Setup with chain added:\n\n");
			printf("Managem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n", valueA);
			gem_print(ampf);
			printf("Managem combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(ampfc);
			if (options.output.debug) printf("Leech rescaling coeff.:   \t%f\n", amps_resc_coeff);
			printf("Spec base power with chain:\t%#.7g\n", gem_amp_power(*gemf, *ampf, leech_ratio));
			double CgP = pow(gem_power(*gemfc), c);
			printf("Global power w. chain at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, CgP*gem_cfr_power(*gemf, amp_leech_scaled));
		}
	}

	if (options.print.parens) {
		printf("Managem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed(ampf);
		printf("\n\n");
		printf("Managem combining scheme:\n");
		print_parens_compressed(gemfc);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed(ampfc);
		printf("\n\n");
	}
	if (options.print.tree) {
		printf("Managem speccing tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree(ampf, "");
		printf("\n");
		printf("Managem combining tree:\n");
		print_tree(gemfc, "");
		printf("\n");
		printf("Amplifier combining tree:\n");
		print_tree(ampfc, "");
		printf("\n");
	}
	if (options.print.table) print_omnia_table(amps, powers, len);
	
	if (options.print.equations) {		// it ruins gems, must be last
		printf("Managem speccing equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations(ampf);
		printf("\n");
		printf("Managem combining equations:\n");
		print_equations(gemfc);
		printf("\n");
		printf("Amplifier combining equations:\n");
		print_equations(ampfc);
		printf("\n");
	}
	
	for (int i =0;i<len;++i) free(pool[i]);     // free gems
	for (int i =0;i<len;++i) free(poolf[i]);    // free gems compressed
	for (int i =0;i<lenc;++i) free(poolc[i]);   // free gems
	free(poolc);
	free(poolc_length);
	free(poolcf);
	for (int i =0;i<lena;++i) free(poolA[i]);   // free amps
	free(poolA);
	free(poolA_length);
	free(bestA);                           // free amps compressed
	if (options.target.chain && len > 2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.has_extra_search();
	options.has_amps();
	options.has_final_eq_grade();
	options.has_nonpures();
	options.set_num_tables(3);
	options.has_lenc();

	options.skills.TC = 120;
	options.skills.amps = 60;
	options.amps.number_per_gem = 2;      // multiple gems in trap
	options.amps.average_gems_seen = 2.5; // with amps on the side

	if(!options.parse_args(argc, argv))
		return 1;
	options.table_selection(0, "table_mgspec");
	options.table_selection(1, "table_leech");
	options.table_selection(2, "table_mgcomb");
	options.tuning.max_ag_cost_ratio = std::max(options.tuning.max_ag_cost_ratio,
		1. + (special_ratio_gccs(options) > 2.5)); // A100 = 2 with defaults

	worker(options);
	return 0;
}
