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
#include "gfon.h"

void print_amps_table(gem* gems, gemY* amps, double* spec_coeffs, double damage_ratio, double crit_ratio, int len)
{
	printf("Killgem\tAmps\tPower\t\tSpec coeff\n");
	int i;
	for (i=0; i<len; i++)
		printf("%d\t%d\t%#.7g\t%f\n", i+1, gem_getvalue_Y(amps+i), gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio), spec_coeffs[i]);
	printf("\n");
}

void worker(int len, int output_options, double growth_comb, char* filename, char* filenameA, int TC, int As, int Namps)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);						// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1.000000,1,0);		// grade damage crit bbound
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// killgem pool filling
	fclose(table);
	if (prevmax<len-1) {										// if the killgems are not enough
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		if (prevmax>0) printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	gem* poolf[len];
	int poolf_length[len];
	
	KGSPEC_COMPRESSION
	printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena=len;
	gemY* poolY[lena];
	int poolY_length[lena];
	poolY[0]=malloc(sizeof(gemY));
	poolY_length[0]=1;
	gem_init_Y(poolY[0],1,1,1);
	
	int prevmaxA=pool_from_table_Y(poolY, poolY_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (i=0;i<=prevmaxA;++i) free(poolY[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	gemY* poolYf[lena];
	int poolYf_length[lena];
	
	AMPS_COMPRESSION
	printf("Amp pool compression done!\n\n");

	int j,k,h;								// let's choose the right gem-amp combo
	gem gems[len];
	gemY amps[len];
	double spec_coeffs[len];
	gem_init(gems,1,1,1,0);
	amps[0]=(gemY){0};
	spec_coeffs[0]=0;
	double crit_ratio  =Namps*(0.15+As/3*0.004)*2*(1+0.03*TC)/(1.0+TC/3*0.1);
	double damage_ratio=Namps*(0.20+As/3*0.004) * (1+0.03*TC)/(1.2+TC/3*0.1);
	if (!(output_options & mask_quiet)) {
		printf("Total value:\t1\n\n");
		printf("Killgem\n");
		gem_print(gems);
		printf("Amplifier (x%d)\n", Namps);
		gem_print_Y(amps);
	}

	for (i=1;i<len;++i) {											// for every gem value
		gems[i]=(gem){0};												// we init the gems
		amps[i]=(gemY){0};											// to extremely weak ones
		for (k=0;k<poolf_length[i];++k) {						// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int NS=i+1;
		double comb_coeff=pow(NS, -growth_comb);
		spec_coeffs[i]=comb_coeff*gem_power(gems[i]);
																			// now with amps
		for (j=0, NS+=Namps; j<i+1; ++j, NS+=Namps) {		// for every amp value from 1 to to gem_value
			double comb_coeff=pow(NS, -growth_comb);			// we compute comb_coeff
			for (k=0;k<poolf_length[i];++k)						// then we search in the gem pool
			if (poolf[i][k].crit!=0) {								// if the gem has crit we go on
				double Pb2 = poolf[i][k].bbound * poolf[i][k].bbound;
				double Pdg = poolf[i][k].damage;
				double Pcg = poolf[i][k].crit  ;
				for (h=0;h<poolYf_length[j];++h) {				// to the amp pool and compare
					double Pdamage = Pdg + damage_ratio* poolYf[j][h].damage ;
					double Pcrit   = Pcg + crit_ratio  * poolYf[j][h].crit   ;
					double power   = Pb2 * Pdamage * Pcrit ;
					double spec_coeff=power*comb_coeff;
					if (spec_coeff>spec_coeffs[i]) {
						spec_coeffs[i]=spec_coeff;
						gems[i]=poolf[i][k];
						amps[i]=poolYf[j][h];
					}
				}
			}
		}
		if (!(output_options & mask_quiet)) {
			printf("Total value:\t%d\n\n", i+1+Namps*gem_getvalue_Y(amps+i));
			printf("Killgem\n");
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_debug) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
			if (output_options & mask_debug) printf("Pool:\t%d\n",poolYf_length[gem_getvalue_Y(amps+i)-1]);
			gem_print_Y(amps+i);
			printf("Spec base power: \t%#.7g\n", gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
			printf("Spec coefficient:\t%f\n\n", spec_coeffs[i]);
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Total value:\t%d\n\n", len+Namps*gem_getvalue_Y(amps+len-1));
		printf("Killgem\n");
		printf("Value:\t%d\n", len);
		gem_print(gems+len-1);
		printf("Amplifier (x%d)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue_Y(amps+len-1));
		gem_print_Y(amps+len-1);
		printf("Spec base power: \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
		printf("Spec coefficient:\t%f\n\n", spec_coeffs[len-1]);
	}

	gem*  gemf=gems+len-1;  // gem that will be displayed
	gemY* ampf=amps+len-1;  // amp that will be displayed

	if (output_options & mask_upto) {
		double best_sc=0;
		int best_index=0;
		for (i=0; i<len; ++i) {
			if (spec_coeffs[i] > best_sc) {
				best_index=i;
				best_sc=spec_coeffs[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Total value:\t%d\n\n", gem_getvalue(gems+best_index)+Namps*gem_getvalue_Y(amps+best_index));
		printf("Killgem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier (x%d)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue_Y(amps+best_index));
		gem_print_Y(amps+best_index);
		printf("Spec base power: \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], damage_ratio, crit_ratio));
		printf("Spec coefficient:\t%f\n\n", best_sc);
		gemf = gems+best_index;
		ampf = amps+best_index;
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 3) printf("I could not add red!\n\n");
		else {
			int value = gem_getvalue(gemf);
			int valueA= gem_getvalue_Y(ampf);
			double NS = value + Namps*valueA;
			double amp_damage_scaled = damage_ratio * ampf->damage;
			double amp_crit_scaled = crit_ratio * ampf->crit;
			gemf = gem_putred(poolf[value-1], poolf_length[value-1], value, &red, &gem_array, amp_damage_scaled, amp_crit_scaled);
			printf("Setup with red added:\n\n");
			printf("Total value:\t%d\n\n", value+Namps*gem_getvalue_Y(ampf));
			printf("Killgem\n");
			printf("Value:\t%d\n", value);
			gem_print(gemf);
			printf("Amplifier (x%d)\n", Namps);
			printf("Value:\t%d\n", valueA);
			gem_print_Y(ampf);
			printf("Spec base power w. red:\t%#.7g\n", gem_amp_power(*gemf, *ampf, damage_ratio, crit_ratio));
			double CgP = pow(NS, -growth_comb);
			printf("Spec coefficient:\t%f\n\n", CgP*gem_cfr_power(*gemf, amp_damage_scaled, amp_crit_scaled));
		}
	}

	if (output_options & mask_parens) {
		printf("Killgem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed_Y(ampf);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Killgem tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree_Y(ampf, "");
		printf("\n");
	}
	if (output_options & mask_table) print_amps_table(gems, amps, spec_coeffs, damage_ratio, crit_ratio, len);

	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Killgem equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations_Y(ampf);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (i=0;i<lena;++i) free(poolY[i]);		// free amps
	for (i=0;i<lena;++i) free(poolYf[i]);		// free amps compressed
	if (output_options & mask_red && len > 2) {
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
	double growth_comb=1.414061;		// 16c
	int output_options=0;
	char filename[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"hptecdqurf:g:T:A:N:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecdqurf:g:T:A:N:");
				return 0;
			PTECIDQUR_OPTIONS_BLOCK
			case 'f':			// can be "filename,filenameA", if missing default is used
				;
				char* p=optarg;
				while (*p != ',' && *p != '\0') p++;
				if (*p==',') *p='\0';			// ok, it's "f,fA"
				else p--;							// not ok, it's "f" -> empty string
				strcpy(filename,optarg);
				strcpy(filenameA,p+1);
				break;
			case 'g':
				growth_comb = atof(optarg);
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
	worker(len, output_options, growth_comb, filename, filenameA, TC, As, Namps);
	return 0;
}

