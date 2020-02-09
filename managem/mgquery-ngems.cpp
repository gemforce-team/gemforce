#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "managem_utils.h"
#include "leech_utils.h"
#include "mga_utils.h"
#include "effective_skills.h"
#include "gfon.h"
#include "print_utils.h"
#include "cmdline_options.h"

using gem = gem_OB;
using gemO = gem_O;

void print_ngems_table(const gem* gems, const gemO* amps, double leech_ratio, int len)
{
	printf("# Gems\tManagem\tAmps\tPower\n");
	
	for (int i =0; i<len; i++)
		printf("%d\t%d\t%d\t%#.7g\n", i+1, gem_getvalue(gems+i), gem_getvalue(amps+i), gem_amp_power(gems[i], amps[i], leech_ratio));
	printf("\n");
}

void worker(const cmdline_options& options)
{
	FILE* table = file_check(options.tables[0]);	// file is open to read
	if (table==NULL) exit(1);						// if the file is not good we exit

	int len = options.target.len;
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1,0);
	gem_init(pool[0]+1,1,0,1);
	
	int prevmax = pool_from_table(pool, pool_length, options.tuning.spec_limit ? options.tuning.spec_limit : len, table);		// managem pool filling
	fclose(table);
	if (prevmax<len-1) {					// if the managems are not enough
		for (int i =prevmax+1; i<len; ++i) {
			pool_length[i]=0;
			pool[i]=NULL;
		}
	}
	
	gem* poolf[len];
	int poolf_length[len];
	
	mgspec_compression(poolf, poolf_length, pool, pool_length, len, options.output.debug);
	if (!options.output.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(options.tables[1]);	// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena = options.amps.number_per_gem ? len/options.amps.number_per_gem : 1;		// if options.amps.number_per_gem==0 let lena=1
	gemO* poolO[lena];
	int poolO_length[lena];
	poolO[0] = (gemO*)malloc(sizeof(gemO));
	poolO_length[0]=1;
	gem_init(poolO[0],1,1);
	
	int prevmaxA=pool_from_table(poolO, poolO_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (int i =0;i<=prevmaxA;++i) free(poolO[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	gemO* bestO = (gemO*)malloc(lena*sizeof(gemO));		// if not malloc-ed 140k is the limit
	
	amps_compression(bestO, poolO, poolO_length, lena);
	if (!options.output.quiet) printf("Amp pool compression done!\n\n");

	// let's choose the right gem-amp combo
	gem gems[len];
	gemO amps[len];
	gem_init(gems,1,1,0);
	amps[0] = {};
	double leech_ratio = special_ratio_gccs(options);

	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i =first; i<len; ++i) {						// for every total value
		gems[i] = {};										// we init the gems
		amps[i] = {};										// to extremely weak ones
		for (int k=0;k<poolf_length[i];++k) {				// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		double power = gem_power(gems[i]);
		if (options.amps.number_per_gem!=0)
		for (int j=1;j<=i/options.amps.number_per_gem;++j) {		// for every amount of amps we can fit in
			int value = i-options.amps.number_per_gem*j;			// this is the amount of gems we have left
			for (int k=0; k<poolf_length[value]; ++k) {			// we search in that pool
				if (gem_amp_power(poolf[value][k], bestO[j-1], leech_ratio) > power)
				{
					power = gem_amp_power(poolf[value][k], bestO[j-1], leech_ratio);
					gems[i]=poolf[value][k];
					amps[i]=bestO[j-1];
				}
			}
		}
		if (!options.output.quiet) {
			printf("Total value:\t%d\n\n", i+1);
			if (prevmax<len-1) printf("Managem limit:\t%d\n", prevmax+1);
			printf("Managem\n");
			printf("Value:\t%d\n",gem_getvalue(gems+i));
			if (options.output.debug) printf("Pool:\t%d\n",poolf_length[gem_getvalue(gems+i)-1]);
			gem_print(gems+i);
			printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			gem_print(amps+i);
			printf("Spec base power: \t%#.7g\n\n", gem_amp_power(gems[i], amps[i], leech_ratio));
		}
	}
	
	if (options.output.quiet) {		// outputs last if we never seen any
		printf("Total value:\t%d\n\n", len);
		if (prevmax<len-1) printf("Managem limit:\t%d\n", prevmax+1);
		printf("Managem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+len-1));
		gem_print(gems+len-1);
		printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n", gem_getvalue(amps+len-1));
		gem_print(amps+len-1);
		printf("Spec base power: \t%#.7g\n\n", gem_amp_power(gems[len-1], amps[len-1], leech_ratio));
	}

	gem*  gemf=gems+len-1;  // gem that will be displayed
	gemO* ampf=amps+len-1;  // amp that will be displayed

	gem* gem_array = NULL;
	if (options.target.chain) {
		if (len < 3) printf("I could not add chain!\n\n");
		else {
			int value=gem_getvalue(gemf);
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], &gem_array, leech_ratio*ampf->leech);
			printf("Setup with chain added:\n\n");
			printf("Total value:\t%d\n\n", value+options.amps.number_per_gem*gem_getvalue(ampf));
			printf("Managem\n");
			printf("Value:\t%d\n", value);
			gem_print(gemf);
			printf("Amplifier (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n", gem_getvalue(ampf));
			gem_print(ampf);
			printf("Spec base power with chain:\t%#.7g\n\n", gem_amp_power(*gemf, *ampf, leech_ratio));
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
	if (options.print.table) print_ngems_table(gems, amps, leech_ratio, len);
	
	if (options.print.equations) {		// it ruins gems, must be last
		printf("Managem equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations(ampf);
		printf("\n");
	}
	
	for (int i =0;i<len;++i) free(pool[i]);			// free gems
	for (int i =0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (int i =0;i<lena;++i) free(poolO[i]);		// free amps
	free(bestO);										// free amps compressed
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
	options.has_spec_limit();
	options.has_nonpures();
	options.set_num_tables(2);

	options.skills.TC = 120;
	options.skills.amps = 60;
	options.amps.number_per_gem = 6;

	if(!options.parse_args(argc, argv))
		return 1;
	options.table_selection(0, "table_mgspec");
	options.table_selection(1, "table_leech");

	worker(options);
	return 0;
}

