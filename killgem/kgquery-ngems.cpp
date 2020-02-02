#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
#include "killgem_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"
#include "kga_utils.h"
#include "query_utils.h"
#include "gfon.h"
#include "print_utils.h"

void print_ngems_table(gem* gems, gemY* amps, double damage_ratio, double crit_ratio, int len)
{
	printf("# Gems\tKillgem\tAmps\tPower\n");
	int i;
	for (i=0; i<len; i++)
		printf("%d\t%d\t%d\t%#.7g\n", i+1, gem_getvalue(gems+i), gem_getvalue(amps+i), gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
	printf("\n");
}

void worker(int len, options output_options, int gem_limit, char* filename, char* filenameA, int TC, int As, int Namps)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);					// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,DAMAGE_CRIT  ,1,0);	// grade damage crit bbound
	gem_init(pool[0]+1,1,DAMAGE_BBOUND,0,1);	// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, gem_limit, table);		// killgem pool filling
	fclose(table);
	if (prevmax<len-1) {					// if the killgems are not enough
		for (i=prevmax+1; i<len; ++i) {
			pool_length[i]=0;
			pool[i]=NULL;
		}
	}

	gem* poolf[len];
	int poolf_length[len];
	
	KGSPEC_COMPRESSION
	if (!output_options.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena = Namps ? len/Namps : 1;		// if Namps==0 let lena=1
	gemY* poolY[lena];
	int poolY_length[lena];
	poolY[0] = (gemY*)malloc(sizeof(gemY));
	poolY_length[0]=1;
	gem_init(poolY[0],1,1,1);
	
	int prevmaxA=pool_from_table(poolY, poolY_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (i=0;i<=prevmaxA;++i) free(poolY[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	gemY* poolYf[lena];
	int poolYf_length[lena];
	
	AMPS_COMPRESSION
	if (!output_options.quiet) printf("Amp pool compression done!\n\n");

	int j,k,h;											// let's choose the right gem-amp combo
	gem gems[len];
	gemY amps[len];
	gem_init(gems,1,1,1,0);
	amps[0] = {};
	double crit_ratio  =Namps*(0.15+As/3*0.004)*2*(1+0.03*TC)/(1.0+TC/3*0.1);
	double damage_ratio=Namps*(0.20+As/3*0.004) * (1+0.03*TC)/(1.2+TC/3*0.1);
	
	int skip_computations = output_options.quiet && !(output_options.table || output_options.upto);
	int first = skip_computations ? len-1 : 0;
	for (i=first; i<len; ++i) {								// for every gem value
		gems[i] = {};										// we init the gems
		amps[i] = {};										// to extremely weak ones
		for (k=0;k<poolf_length[i];++k) {					// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		double power = gem_power(gems[i]);
		if (Namps!=0)
		for (j=1;j<=i/Namps;++j) {							// for every amount of amps we can fit in
			int value = i-Namps*j;							// this is the amount of gems we have left
			for (k=0;k<poolf_length[value];++k) {			// we search in that pool
				for (h=0;h<poolYf_length[j-1];++h) {		// and we look in the amp pool
					if (gem_amp_power(poolf[value][k], poolYf[j-1][h], damage_ratio, crit_ratio) > power)
					{
						power = gem_amp_power(poolf[value][k], poolYf[j-1][h], damage_ratio, crit_ratio);
						gems[i]=poolf[value][k];
						amps[i]=poolYf[j-1][h];
					}
				}
			}
		}
		if (!output_options.quiet) {
			printf("Total value:\t%d\n\n", i+1);
			if (prevmax<len-1) printf("Killgem limit:\t%d\n", prevmax+1);
			printf("Killgem\n");
			printf("Value:\t%d\n",gem_getvalue(gems+i));
			if (output_options.debug) printf("Pool:\t%d\n",poolf_length[gem_getvalue(gems+i)-1]);
			gem_print(gems+i);
			printf("Amplifier (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue(amps+i));
			if (output_options.debug) printf("Pool:\t%d\n",poolYf_length[gem_getvalue(amps+i)-1]);
			gem_print(amps+i);
			printf("Spec base power: \t%#.7g\n\n", gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
		}
	}
	
	if (output_options.quiet) {		// outputs last if we never seen any
		printf("Total value:\t%d\n\n", len);
		if (prevmax<len-1) printf("Killgem limit:\t%d\n", prevmax+1);
		printf("Killgem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+len-1));
		gem_print(gems+len-1);
		printf("Amplifier (x%d)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue(amps+len-1));
		gem_print(amps+len-1);
		printf("Spec base power: \t%#.7g\n\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
	}

	gem*  gemf=gems+len-1;  // gem that will be displayed
	gemY* ampf=amps+len-1;  // amp that will be displayed

	gem* gem_array = NULL;
	if (output_options.chain) {
		if (len < 3) printf("I could not add chain!\n\n");
		else {
			int value=gem_getvalue(gemf);
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], &gem_array, damage_ratio*ampf->damage, crit_ratio*ampf->crit);
			printf("Setup with chain added:\n\n");
			printf("Total value:\t%d\n\n", value+Namps*gem_getvalue(ampf));
			printf("Killgem\n");
			printf("Value:\t%d\n", value);
			gem_print(gemf);
			printf("Amplifier (x%d)\n", Namps);
			printf("Value:\t%d\n", gem_getvalue(ampf));
			gem_print(ampf);
			printf("Spec base power with chain:\t%#.7g\n\n", gem_amp_power(*gemf, *ampf, damage_ratio, crit_ratio));
		}
	}

	if (output_options.parens) {
		printf("Killgem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed(ampf);
		printf("\n\n");
	}
	if (output_options.tree) {
		printf("Killgem tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree(ampf, "");
		printf("\n");
	}
	if (output_options.table) print_ngems_table(gems, amps, damage_ratio, crit_ratio, len);

	
	if (output_options.equations) {		// it ruins gems, must be last
		printf("Killgem equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations(ampf);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (i=0;i<lena;++i) free(poolY[i]);		// free amps
	for (i=0;i<lena;++i) free(poolYf[i]);		// free amps compressed
	if (output_options.chain && len > 2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int TC=120;
	int As=60;
	int Namps=8;
	int gem_limit=0;
	options output_options = {};
	char filename[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"hptecdqrl:f:T:A:N:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecdqrl:f:T:A:N:");
				return 0;
			PTECIDQUR_OPTIONS_BLOCK
			case 'l':
				gem_limit = atoi(optarg);
				break;
			case 'f':		// can be "filename,filenameA", if missing default is used
				table_selection2(optarg, filename, filenameA);
				break;
			TAN_OPTIONS_BLOCK
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
		if (gem_limit==0) gem_limit=len;
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
	if (len<1) {
		printf("Improper gem number\n");
		return 1;
	}
	file_selection(filename, "table_kgspec");
	file_selection(filenameA, "table_crit");
	worker(len, output_options, gem_limit, filename, filenameA, TC, As, Namps);
	return 0;
}

