#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
const int ACC=0;								// I don't really use it here
#include "killgem_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"
#include "gfon.h"

double gem_amp_power(gem gem1, gemY amp1)
{
	return (gem1.damage+6*0.28*(2.8/3.2)*amp1.damage)*gem1.bbound*(gem1.crit+4*0.23*2.8*amp1.crit)*gem1.bbound;		// yes, fraction and 4, due to 3.2 and 1.5 rescaling
}

int gem_amp_more_powerful(gem gem1, gemY amp1, gem gem2, gemY amp2)
{
	return gem_amp_power(gem1, amp1) > gem_amp_power(gem2, amp2);
}

void print_global_table(gem* gems, gemY* amps, int len)
{
	printf("# Gems\tKillgem\tAmps\tPower (resc.)\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%d\t%.6lf\n", i+1, gem_getvalue(gems+i), gem_getvalue_Y(amps+i), gem_amp_power(gems[i], amps[i]));
	printf("\n");
}

void print_spec_table(gem* gems, gemY* amps, double* spec_coeffs, int len)
{
	printf("Killgem\tAmps\tPower (resc.)\tSpec coeff\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%.6f\t%.6lf\n", i+1, gem_getvalue_Y(amps+i), gem_amp_power(gems[i], amps[i]), spec_coeffs[i]);
	printf("\n");
}

void worker(int len, int output_options, int global_mode, double growth_comb, char* filename, char* filenameA)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);							// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1.000000,1,0);		// grade damage crit bbound
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// killgem pool filling
	if (prevmax<len-1) {						// if the killgems are not enough
		if (global_mode==0) {					// behave as kga_spec -> quit if not enough killgems
			fclose(table);
			for (i=0;i<=prevmax;++i) free(pool[i]);		// free
			printf("Gem table stops at %d, not %d\n",prevmax+1,len);
			exit(1);
		}
		else {												// behave as killgem_amps -> fill the remaining with false gems
			for (i=prevmax+1; i<len; ++i) {
				pool_length[i]=1;
				pool[i]=malloc(sizeof(gem));
				pool[i][0]=(gem){0};			// null gems
			}
		}
	}
	
	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);						// if the file is not good we exit
	int lena;
	if (global_mode) lena=len/6;					// killgem_amps -> len/6
	else lena=2*len;											// kga_spec -> 2x mg value
	gemY* poolY[lena];
	int poolY_length[lena];
	poolY[0]=malloc(sizeof(gemY));
	poolY_length[0]=1;
	gem_init_Y(poolY[0],1,1,1);
	
	int prevmaxA=pool_from_table_Y(poolY, poolY_length, lena, tableA);		// amps pool filling
	if (prevmaxA<lena-1) {
		fclose(tableA);
		for (i=0;i<=prevmaxA;++i) free(poolY[i]);		// free
		printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}
	
	int j,k,h;											// let's choose the right gem-amp combo
	gemY amps[len];
	gem gems[len];
	gem_init(gems,1,1,1,0);
	amps[0]=(gemY){0};
	printf("Total value:\t1\n\n");
	printf("Killgem:\n");
	gem_print(gems);
	printf("Amplifier:\n");
	gem_print_Y(amps);
	double spec_coeffs[len];
	spec_coeffs[0]=0;
	
	if (global_mode) { 							// behave like killgem_amps
		for (i=1;i<len;++i) {																	// for every total value
			gems[i]=(gem){0};																		// we init the gems
			amps[i]=(gemY){0};																	// to extremely weak ones
			for (k=0;k<pool_length[i];++k) {										// first we compare the gem alone
				if (gem_power(pool[i][k]) > gem_power(gems[i])) {
					gems[i]=pool[i][k];
				}
			}
			for (j=1;j<=i/6;++j) {															// for every amount of amps we can fit in
				int value = i-6*j;																// this is the amount of gems we have left
				for (k=0;k<pool_length[value];++k)								// we search in that pool
				if (pool[value][k].crit!=0) {											// if the gem has leech we go on
					for (h=0;h<poolY_length[j-1];++h) {							// and we look in the amp pool
						if (gem_amp_more_powerful(pool[value][k],poolY[j-1][h],gems[i],amps[i])) {
							gems[i]=pool[value][k];
							amps[i]=poolY[j-1][h];
						}
					}
				}
			}
			printf("Total value:\t%d\n\n", i+1);
			printf("Killgem\n");
			if (prevmax<len-1) printf("Killgem limit:\t%d\n", prevmax+1);
			printf("Value:\t%d\n",gem_getvalue(gems+i));
			if (output_options & mask_info) printf("Pool:\t%d\n",pool_length[gem_getvalue(gems+i)-1]);
			gem_print(gems+i);
			printf("Amplifier\n");
			printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
			if (output_options & mask_info) printf("Pool:\t%d\n",poolY_length[gem_getvalue_Y(amps+i)-1]);
			gem_print_Y(amps+i);
			printf("Global power (resc.):\t%f\n\n", gem_amp_power(gems[i], amps[i]));
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}
	else { 										// behave like kga_spec
		for (i=1;i<len;++i) {															// for every gem value
			gems[i]=(gem){0};																// we init the gems
			amps[i]=(gemY){0};															// to extremely weak ones
			spec_coeffs[i]=0;																// and init a spec coeff
			for (k=0;k<pool_length[i];++k) {								// first we compare the gem alone
				if (gem_power(pool[i][k]) > gem_power(gems[i])) {
					gems[i]=pool[i][k];
				}
			}
			int NS=i+1;
			double comb_coeff=pow(NS, -growth_comb);
			spec_coeffs[i]=comb_coeff*gem_power(pool[i][k]);
																											// now with amps
			for (j=0;j<2*i+2;++j) {													// for every amp value from 1 to to 2*gem_value
				NS+=6;																				// we get total num of gems used
				double comb_coeff=pow(NS, -growth_comb);			// we compute comb_coeff
				for (k=0;k<pool_length[i];++k)								// then we search in the gem pool
				if (pool[i][k].crit!=0) {											// if the gem has crit we go on
					double Pb2 = pool[i][k].bbound * pool[i][k].bbound;
					double Pdg = pool[i][k].damage;
					double Pcg = pool[i][k].crit  ;
					for (h=0;h<poolY_length[j];++h) {						// to the amp pool and compare
						double Pdamage = Pdg + 1.47 * poolY[j][h].damage ;
						double Pcrit   = Pcg + 2.576* poolY[j][h].crit   ;
						double power   = Pb2 * Pdamage * Pcrit ;
						double spec_coeff=power*comb_coeff;
						if (spec_coeff>spec_coeffs[i]) {
							spec_coeffs[i]=spec_coeff;
							gems[i]=pool[i][k];
							amps[i]=poolY[j][h];
						}
					}
				}
			}
			printf("Total value:\t%d\n\n", i+1+6*gem_getvalue_Y(amps+i));
			printf("Killgem\n");
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) printf("Pool:\t%d\n",pool_length[i]);
			gem_print(gems+i);
			printf("Amplifier\n");
			printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
			if (output_options & mask_info) printf("Pool:\t%d\n",poolY_length[gem_getvalue_Y(amps+i)-1]);
			gem_print_Y(amps+i);
			printf("Global power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i]));
			printf("Spec coefficient:\t%f\n\n", spec_coeffs[i]);
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}
	
	if (output_options & mask_parens) {
		printf("Killgem combining scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed_Y(amps+len-1);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Killgem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree_Y(amps+len-1, "");
		printf("\n");
	}
	if (output_options & mask_table) {
		if (global_mode) print_global_table(gems, amps, len);
		else print_spec_table(gems, amps, spec_coeffs, len);
	}
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Killgem equations:\n");
		print_equations(gems+len-1);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations_Y(amps+len-1);
		printf("\n");
	}
	
	fclose(table);
	fclose(tableA);
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<lena;++i) free(poolY[i]);		// free amps
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_options=0;
	int global_mode=0;
	double growth_comb=1.414061;		// 16c
	char filename[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"iptcef:g:"))!=-1) {
		switch(opt) {
			case 'i':
				output_options |= mask_info;
				break;
			case 'p':
				output_options |= mask_parens;
				break;
			case 't':
				output_options |= mask_tree;
				break;
			case 'c':
				output_options |= mask_table;
				break;
			case 'e':
				output_options |= mask_equations;
				break;
			case 'f':			// can be "filename,filenameA", if missing default is used
				;
				char* p=optarg;
				while (*p != ',' && *p != '\0') p++;
				if (*p==',') *p='\0';			// ok, it's "f,fA"
				else p--;									// not ok, it's "f" -> empty string
				strcpy(filename,optarg);
				strcpy(filenameA,p+1);
				break;
			case 'g':
				growth_comb = atof(optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
		char* p=argv[optind];
		while (*p != '\0') p++;
		if (*(p-1)=='g') global_mode=1;
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1) {
		printf("Improper gem number\n");
		return 1;
	}
	if (filename[0]=='\0') strcpy(filename, "table_kgspec");
	if (filenameA[0]=='\0') strcpy(filenameA, "table_crit");
	worker(len, output_options, global_mode, growth_comb, filename, filenameA);
	return 0;
}

