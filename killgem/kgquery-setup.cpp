#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "interval_tree.h"
#include "killgem_utils.h"
#include "crit_utils.h"
#include "kga_utils.h"
#include "query_utils.h"
#include "gfon.h"
#include "print_utils.h"
#include "options_utils.h"

using gem = gem_YB;
using gemY = gem_Y;

void worker(int len, int lenc, options output_options, char* filename, char* filenamec, char* filenameA, int TC, int As, int GT, int Namps)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);					// if the file is not good we exit
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,DAMAGE_CRIT  ,1,0);	// grade damage crit bbound
	gem_init(pool[0]+1,1,DAMAGE_BBOUND,0,1);	// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// killgem spec pool filling
	fclose(table);
	if (prevmax<len-1) {										// if the killgems are not enough
		for (int i =0;i<=prevmax;++i) free(pool[i]);		// free
		if (prevmax>0) printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	gem* poolf[len];
	int poolf_length[len];
	
	kgspec_compression(poolf, poolf_length, pool, pool_length, len, output_options);
	if (!output_options.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena=len;									// as long as the spec length
	gemY** poolY = (gemY**)malloc(lena*sizeof(gemY*));
	int* poolY_length = (int*)malloc(lena*sizeof(int));
	poolY[0] = (gemY*)malloc(sizeof(gemY));
	poolY_length[0]=1;
	gem_init(poolY[0],1,1,1);
	
	int prevmaxA=pool_from_table(poolY, poolY_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (int i =0;i<=prevmaxA;++i) free(poolY[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}
	
	gemY** poolYf = (gemY**)malloc(lena*sizeof(gemY*));		// if not malloc-ed 140k is the limit
	int poolYf_length[lena];
	
	amps_compression(poolYf, poolYf_length, poolY, poolY_length, lena, output_options);
	if (!output_options.quiet) printf("Amp pool compression done!\n");

	FILE* tablec=file_check(filenamec);		// file is open to read
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
	
	gem bestc = {};				// choosing best combine
	
	for (int i =0;i<poolc_length[lenc-1];++i) {
		if (gem_more_powerful(poolc[lenc-1][i], bestc)) {
			bestc=poolc[lenc-1][i];
		}
	}
	double bestc_growth=log(gem_power(bestc))/log(lenc);
	
	if (!output_options.quiet) printf("Combining pool compression done!\n\n");

	// let's choose the right gem-amp combo
	gem gems[len];						// for every speccing value
	gemY amps[len];						// we'll choose the best amps
	double powers[len];
	gem_init(gems,1,1,1,0);
	gem_init(amps,0,0,0);
	powers[0]=0;
	double crit_ratio  =Namps*(0.15+As/3*0.004)*2*(1+0.03*TC)/(1.0+TC/3*0.1);
	double damage_ratio=Namps*(0.20+As/3*0.004) * (1+0.03*TC)/(1.2+TC/3*0.1);
	double NT=pow(2, GT-1);
	
	bool skip_computations = output_options.quiet && !(output_options.table || output_options.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i =first; i<len; ++i) {								// for every gem value
		gems[i] = {};												// we init the gems
		amps[i] = {};												// to extremely weak ones
		
		for (int k=0;k<poolf_length[i];++k) {							// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int j;
		int NS=i+1;
		double C0 = pow(NT/(i+1), bestc_growth);					// last we compute the combination number
		powers[i] = C0 * gem_power(gems[i]);
																	// now we compare the whole setup
		for (j=0, NS+=Namps; j<i+1; ++j, NS+=Namps) {				// for every amp value from 1 to to gem_value
			double Cg = pow(NT/NS, bestc_growth);					// we compute the combination number
			for (int k=0;k<poolf_length[i];++k) {						// then in the gem pool
				double Pb2 = poolf[i][k].bbound * poolf[i][k].bbound;
				double Pdg = poolf[i][k].damage;
				double Pcg = poolf[i][k].crit  ;
				for (int h=0;h<poolYf_length[j];++h) {					// and in the reduced amp pool
					double Pdamage = Pdg + damage_ratio* poolYf[j][h].damage ;
					double Pcrit   = Pcg + crit_ratio  * poolYf[j][h].crit   ;
					double Pbase   = Pb2 * Pdamage * Pcrit ;
					double power   = Cg * Pbase;
					if (power>powers[i]) {
						powers[i]=power;
						gems[i]=poolf[i][k];
						amps[i]=poolYf[j][h];
					}
				}
			}
		}
		if (!output_options.quiet) {
			printf("Killgem spec\n");
			printf("Value:\t%d\n",i+1);
			if (output_options.debug) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			if (output_options.debug) printf("Pool:\t%d\n",poolYf_length[gem_getvalue(amps+i)-1]);
			gem_print(amps+i);
			printf("Setup combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(&bestc);
			printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
			printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[i]);
		}
	}
	
	if (output_options.quiet) {		// outputs last if we never seen any
		printf("Killgem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n",gem_getvalue(amps+len-1));
		gem_print(amps+len-1);
		printf("Setup combine\n");
		printf("Comb:\t%d\n",lenc);
		printf("Growth:\t%f\n", bestc_growth);
		gem_print(&bestc);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[len-1]);
	}

	gem*  gemf = gems+len-1;  // gem  that will be displayed
	gemY* ampf = amps+len-1;  // amp  that will be displayed
	gem*  gemfc= &bestc;      // gemc that will be displayed

	if (output_options.upto) {
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
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue(amps+best_index));
		gem_print(amps+best_index);
		printf("Setup combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemfc);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], damage_ratio, crit_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[best_index]);
		gemf = gems+best_index;
		ampf = amps+best_index;
	}

	gem* gem_array = NULL;
	if (output_options.chain) {
		if (len < 3) printf("I could not add chain!\n\n");
		else {
			int value = gem_getvalue(gemf);
			int valueA= gem_getvalue(ampf);
			double NS = value + Namps*valueA;
			double amp_damage_scaled = damage_ratio * ampf->damage;
			double amp_crit_scaled = crit_ratio * ampf->crit;
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], &gem_array, amp_damage_scaled, amp_crit_scaled);
			printf("Setup with chain added:\n\n");
			printf("Killgem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n", valueA);
			gem_print(ampf);
			printf("Setup combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Spec base power with chain:\t%#.7g\n", gem_amp_power(*gemf, *ampf, damage_ratio, crit_ratio));
			double CgP = pow(NT/NS, bestc_growth);
			printf("Global power w. chain at g%d:\t%#.7g\n\n\n", GT, CgP*gem_cfr_power(*gemf, amp_damage_scaled, amp_crit_scaled));
		}
	}

	if (output_options.parens) {
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
	if (output_options.tree) {
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
	if (output_options.table) print_omnia_table(amps, powers, len);
	
	if (output_options.equations) {		// it ruins gems, must be last
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
	
	for (int i =0;i<len;++i) free(pool[i]);			// free gems
	for (int i =0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (int i =0;i<lenc;++i) free(poolc[i]);		// free gems
	free(poolc);
	free(poolc_length);
	for (int i =0;i<lena;++i) free(poolY[i]);		// free amps
	for (int i =0;i<lena;++i) free(poolYf[i]);		// free amps compressed
	free(poolY);
	free(poolY_length);
	free(poolYf);
	if (output_options.chain && len > 2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	int lenc;
	char opt;
	int TC=120;
	int As=60;
	int GT=30;    // NT = pow(2, GT-1)
	int Namps=8;  // killgem in tower
	options output_options = {};
	char filename[256]="";		// it should be enough
	char filenamec[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"hptecdqurf:T:A:N:G:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecdqurf:T:A:N:G:");
				return 0;
			PTECIDQUR_OPTIONS_BLOCK
			case 'f':		// can be "filename,filenamec,filenameA", if missing default is used
				table_selection3(optarg, filename, filenamec, filenameA);
				break;
			TAN_OPTIONS_BLOCK
			case 'G':
				GT=atoi(optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind==argc) {
		printf("No length specified\n");
		return 1;
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
		lenc= 16;		// 16c as default combine
	}
	else if (optind+2==argc) {
		len = atoi(argv[optind]);
		lenc= atoi(argv[optind+1]);
	}
	else {
		printf("Too many arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		printf("\n");
		return 1;
	}
	if (len<1 || lenc<1) {
		printf("Improper gem number\n");
		return 1;
	}
	file_selection(filename, "table_kgspec");
	file_selection(filenamec, "table_kgcomb");
	file_selection(filenameA, "table_crit");
	worker(len, lenc, output_options, filename, filenamec, filenameA, TC, As, GT, Namps);
	return 0;
}