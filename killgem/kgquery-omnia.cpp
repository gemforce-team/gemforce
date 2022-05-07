#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "interval_tree.h"
#include "killgem_utils.h"
#include "crit_utils.h"
#include "kga_utils.h"
#include "cpair.h"
#include "gfon.h"
#include "effective_skills.h"
#include "print_utils.h"
#include "cmdline_options.h"

using gem = gem_YB;
using gemA = gem_Y;

void worker(const cmdline_options& options)
{
	FILE* table = file_check(options.tables[0]);	// file is open to read
	if (table==NULL) exit(1);					// if the file is not good we exit

	int len = options.target.len;
	int lenc = options.target.lenc;
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,DAMAGE_CRIT  ,1,0);	// grade damage crit bbound
	gem_init(pool[0]+1,1,DAMAGE_BBOUND,0,1);	// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// killgem spec pool filling
	fclose(table);
	if (prevmax<len-1) {										// if the killgems are not enough
		for (int i = 0; i <= prevmax; ++i) free(pool[i]);		// free
		if (prevmax>0) printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	gem* poolf[len];
	int poolf_length[len];
	
	specs_compression(poolf, poolf_length, pool, pool_length, len, options.output.debug);
	if (!options.output.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(options.tables[1]);	// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena=std::max(options.tuning.max_ag_cost_ratio * len, lenc);
	gemA** poolA = (gemA**)malloc(lena*sizeof(gemA*));
	int* poolA_length = (int*)malloc(lena*sizeof(int));
	poolA[0] = (gemA*)malloc(sizeof(gemA));
	poolA_length[0]=1;
	gem_init(poolA[0],1,1,1);
	
	int prevmaxA=pool_from_table(poolA, poolA_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (int i =0;i<=prevmaxA;++i) free(poolA[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}
	
	gemA** poolAf = (gemA**)malloc(lena*sizeof(gemA*));		// if not malloc-ed 140k is the limit
	int poolAf_length[lena];
	
	amps_compression(poolAf, poolAf_length, poolA, poolA_length, lena, options.output.debug);
	gemA poolAc[poolAf_length[lenc-1]];
	int poolAc_length=poolAf_length[lenc-1];
	
	for (int i =0; i<poolAf_length[lenc-1]; ++i) {		// amps fast access combining pool
		poolAc[i]=poolAf[lenc-1][i];
	}
	if (!options.output.quiet) printf("Amp combining pool compression done!\n");

	FILE* tablec=file_check(options.tables[2]);	// file is open to read
	if (tablec==NULL) exit(1);					// if the file is not good we exit
	gem** poolc = (gem**)malloc(lenc*sizeof(gem*));
	int* poolc_length = (int*)malloc(lenc*sizeof(int));
	poolc[0] = (gem*)malloc(sizeof(gem));
	poolc_length[0]=1;
	gem_init(poolc[0],1,1,1,1);
	
	int prevmaxc=pool_from_table(poolc, poolc_length, lenc, tablec);		// killgem comb pool filling
	fclose(tablec);
	if (prevmaxc<lenc-1) {									// if the killgems are not enough
		for (int i =0;i<=prevmaxc;++i) free(poolc[i]);		// free
		if (prevmaxc>0) printf("Gem table stops at %d, not %d\n",prevmaxc+1,lenc);
		exit(1);
	}
	
	gem* poolcf;
	int poolcf_length;
	
	combs_compression(&poolcf, &poolcf_length, poolc[lenc - 1], poolc_length[lenc - 1]);
	if (!options.output.quiet) printf("Gem combine compressed pool size:\t%d\n",poolcf_length);

	int cpairs_length;
	cpair* cpairs;
	
	{				// cpair compression
		int length = poolcf_length*poolAc_length;
		cpair* temp_array = (cpair*)malloc(length*sizeof(cpair));
		int index=0;
		for (int l=0; l<poolcf_length; ++l) {
			for (int m=0; m<poolAc_length; ++m) {
				double power = gem_power(poolcf[l]);
				double rdmg  = poolAc[m].damage/poolcf[l].damage;
				double rcrit = poolAc[m].crit / poolcf[l].crit;
				temp_array[index++] = (cpair){power, rdmg, rcrit, poolcf+l, poolAc+m, 0};
			}
		}
		gem_sort(temp_array, length, cpair_less_rcrit);			/* work starts */
		double lastrcrit=-1;
		int tree_cell=0;
		for (int l=0; l<length; ++l) {
			if (temp_array[l].rcrit == lastrcrit) temp_array[l].place=tree_cell-1;
			else {
				temp_array[l].place=tree_cell++;
				lastrcrit = temp_array[l].rcrit;
			}
		}
		gem_sort(temp_array, length, cpair_less_xyz);
		int broken=0;
		int tree_length= 1 << (int)ceil(log2(tree_cell));		/* this is pow(2, ceil()) bitwise */
		double* tree = (double*)malloc((tree_length*2)*sizeof(double));
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* init also tree[0], it's faster */
		for (int l=length-1; l>=0; --l) {						/* start from large rdmg */
			cpair* p_cpair=temp_array+l;
			if (tree_check_after(tree, tree_length, p_cpair->place, p_cpair->power)) {
				tree_add_element(tree, tree_length, p_cpair->place, p_cpair->power);
			}
			else {
				p_cpair->combg=NULL;
				broken++;
			}
		}
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* BgDaCa - iDa - iCa  compression */
		for (int l=0; l<length; ++l) {							/* start from low rdmg */
			cpair* p_cpair=temp_array+l;
			if (p_cpair->combg==NULL) continue;
			int place = tree_length -1 - p_cpair->place;		/* reverse crit order */
			if (tree_check_after(tree, tree_length, place, cpair_BgDaCa(*p_cpair))) {
				tree_add_element(tree, tree_length, place, cpair_BgDaCa(*p_cpair));
			}
			else {
				p_cpair->combg=NULL;
				broken++;
			}
		}
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* BgDaCg - iDa - Ca  compression */
		for (int l=0; l<length; ++l) {							/* start from low rdmg */
			cpair* p_cpair=temp_array+l;
			if (p_cpair->combg==NULL) continue;
			int place = p_cpair->place;							/* regular crit order */
			if (tree_check_after(tree, tree_length, place, cpair_BgDaCg(*p_cpair))) {
				tree_add_element(tree, tree_length, place, cpair_BgDaCg(*p_cpair));
			}
			else {
				p_cpair->combg=NULL;
				broken++;
			}
		}
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* BgDgCa - Da - iCa  compression */
		for (int l=length-1; l>=0; --l) {						/* start from large rdmg */
			cpair* p_cpair=temp_array+l;
			if (p_cpair->combg==NULL) continue;
			int place = tree_length -1 - p_cpair->place;		/* reverse crit order */
			if (tree_check_after(tree, tree_length, place, cpair_BgDgCa(*p_cpair))) {
				tree_add_element(tree, tree_length, place, cpair_BgDgCa(*p_cpair));
			}
			else {
				p_cpair->combg=NULL;
				broken++;
			}
		}
		free(tree);
		
		cpairs_length=length-broken;
		cpairs = (cpair*)malloc(cpairs_length*sizeof(cpair));
		index=0;
		for (int j=0; j<length; ++j) {
			if (temp_array[j].combg!=NULL) {
				cpairs[index] = temp_array[j];
				index++;
			}
		}
		free(temp_array);
	}
	if (!options.output.quiet) printf("Combine pairs pool size:\t%d\n\n",cpairs_length);

	// let's choose the right gem-amp combo
	gem gems[len];						// for every speccing value
	gemA amps[len];						// we'll choose the best amps
	gem gemsc[len];						// and the best NC combine
	gemA ampsc[len];					// for both
	double powers[len];
	gem_init(gems,1,1,1,0);
	gem_init(amps,0,0,0);
	gem_init(gemsc,1,1,0,0);
	gem_init(ampsc,0,0,0);
	powers[0]=0;
	double iloglenc=1/log(lenc);
	double crit_ratio   = special_ratio_gccs(options);
	double damage_ratio = damage_ratio_gccs(options);
	double NT=pow(2, options.tuning.final_eq_grade-1);
	
	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i =first; i<len; ++i) {					// for every gem value
		gems[i] = {};									// we init the gems
		amps[i] = {};									// to extremely weak ones
		gemsc[i] = {};
		ampsc[i] = {};
																	// first we compare the gem alone
		for (int l=0; l<poolcf_length; ++l) {						// first search in the NC gem comb pool
			if (gem_power(poolcf[l]) > gem_power(gemsc[i])) {
				gemsc[i]=poolcf[l];
			}
		}
		for (int k=0;k<poolf_length[i];++k) {						// and then in the compressed gem pool
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int j;
		int NS=i+1;
		double c0 = log(NT/(i+1))*iloglenc;							// last we compute the combination number
		powers[i] = pow(gem_power(gemsc[i]),c0) * gem_power(gems[i]);
																	// now we compare the whole setup
		int amps_bound = options.tuning.max_ag_cost_ratio * (i + 1);	// now with amps
		for (j=0, NS+=options.amps.number_per_gem; j<amps_bound; ++j, NS+=options.amps.number_per_gem) {
			double c = log(NT/NS)*iloglenc;							// we compute the combination number
			for (int l=0; l<cpairs_length; ++l) {						// then we search in the comb pair pool
				double Cg = pow(cpairs[l].power,c);
				double Rd = damage_ratio*pow(cpairs[l].rdmg, c);
				double Rc = crit_ratio * pow(cpairs[l].rcrit,c);
				for (int h=0; h<poolAf_length[j]; ++h) {				// then in the reduced amp pool
					double Pad = Rd * poolAf[j][h].damage;
					double Pac = Rc * poolAf[j][h].crit  ;
					for (int k=0; k<poolf_length[i]; ++k) {				// and in the gem pool
						double Pext = Cg * poolf[i][k].bbound * poolf[i][k].bbound;
						double Pdamage = poolf[i][k].damage + Pad;
						double Pcrit   = poolf[i][k].crit   + Pac;
						double power   = Pext * Pdamage * Pcrit ;
						if (power>powers[i]) {
							powers[i]=power;
							gems[i]=poolf[i][k];
							amps[i]=poolAf[j][h];
							gemsc[i]=*(cpairs[l].combg);
							ampsc[i]=*(cpairs[l].comba);
						}
					}
				}
			}
		}
		if (!options.output.quiet) {
			printf("Killgem spec\n");
			printf("Value:\t%d\n",i+1);
			if (options.output.debug) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			if (options.output.debug) printf("Pool:\t%d\n",poolAf_length[gem_getvalue(amps+i)-1]);
			gem_print(amps+i);
			printf("Killgem combine\n");
			printf("Comb:\t%d\n",lenc);
			if (options.output.debug) printf("P.pool:\t%d\n", cpairs_length);
			gem_print(gemsc+i);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(ampsc+i);
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
		printf("Killgem combine\n");
		printf("Comb:\t%d\n",lenc);
		if (options.output.debug) printf("P.pool:\t%d\n", cpairs_length);
		gem_print(gemsc+len-1);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(ampsc+len-1);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
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
		printf("Killgem spec\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
		printf("Value:\t%d\n", gem_getvalue(amps+best_index));
		gem_print(amps+best_index);
		printf("Killgem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+best_index);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(ampsc+best_index);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], damage_ratio, crit_ratio));
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
			int value=gem_getvalue(gemf);
			int valueA= gem_getvalue(ampf);
			double NS = value + options.amps.number_per_gem*valueA;
			double c = log(NT/NS)*iloglenc;
			double ampd_resc_coeff = pow((ampfc->damage/gemfc->damage), c);
			double ampc_resc_coeff = pow((ampfc->crit/gemfc->crit), c);
			double amp_damage_scaled = damage_ratio * ampd_resc_coeff * ampf->damage;
			double amp_crit_scaled = crit_ratio * ampc_resc_coeff * ampf->crit;
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], &gem_array, amp_damage_scaled, amp_crit_scaled);
			printf("Setup with chain added:\n\n");
			printf("Killgem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d@%.1f)\n", options.amps.number_per_gem, options.amps.average_gems_seen);
			printf("Value:\t%d\n", valueA);
			gem_print(ampf);
			printf("Killgem combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(ampfc);
			if (options.output.debug) printf("Damage rescaling coeff.: \t%f\n", ampd_resc_coeff);
			if (options.output.debug) printf("Crit rescaling coeff.:   \t%f\n", ampc_resc_coeff);
			printf("Spec base power with chain:\t%#.7g\n", gem_amp_power(*gemf, *ampf, damage_ratio, crit_ratio));
			double CgP = pow(gem_power(*gemfc), c);
			printf("Global power w. chain at g%d:\t%#.7g\n\n\n", options.tuning.final_eq_grade, CgP*gem_cfr_power(*gemf, amp_damage_scaled, amp_crit_scaled));
		}
	}

	if (options.print.parens) {
		printf("Killgem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed(ampf);
		printf("\n\n");
		printf("Killgem combining scheme:\n");
		print_parens_compressed(gemfc);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed(ampfc);
		printf("\n\n");
	}
	if (options.print.tree) {
		printf("Killgem speccing tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree(ampf, "");
		printf("\n");
		printf("Killgem combining tree:\n");
		print_tree(gemfc, "");
		printf("\n");
		printf("Amplifier combining tree:\n");
		print_tree(ampfc, "");
		printf("\n");
	}
	if (options.print.table) print_omnia_table(amps, powers, len);
	
	if (options.print.equations) {		// it ruins gems, must be last
		printf("Killgem speccing equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations(ampf);
		printf("\n");
		printf("Killgem combining equations:\n");
		print_equations(gemfc);
		printf("\n");
		printf("Amplifier combining equations:\n");
		print_equations(ampfc);
		printf("\n");
	}
	
	for (int i =0;i<len;++i) free(pool[i]);			// free gems
	for (int i =0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (int i =0;i<lenc;++i) free(poolc[i]);		// free gems
	free(poolc);
	free(poolc_length);
	free(poolcf);
	for (int i =0;i<lena;++i) free(poolA[i]);		// free amps
	for (int i =0;i<lena;++i) free(poolAf[i]);		// free amps compressed
	free(poolA);
	free(poolA_length);
	free(poolAf);
	free(cpairs);
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
	options.amps.number_per_gem = 8;

	if(!options.parse_args(argc, argv))
		return 1;
	options.table_selection(0, "table_kgspec");
	options.table_selection(1, "table_kgcomb");
	options.table_selection(2, "table_crit");

	worker(options);
	return 0;
}
