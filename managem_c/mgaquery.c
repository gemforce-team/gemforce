#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_OB gem;		// the strange order is so that managem_utils knows which gem type are we defining as "gem"
#include "managem_utils.h"
typedef struct Gem_O gemO;
#include "leech_utils.h"
#include "gfon.h"

double gem_amp_power(gem gem1, gemO amp1)
{
	return (gem1.leech+4*0.23*2.8*amp1.leech)*gem1.bbound;		// yes, 4, because of 1.5 rescaling
}

int gem_amp_more_powerful(gem gem1, gemO amp1, gem gem2, gemO amp2)
{
	return gem_amp_power(gem1, amp1) > gem_amp_power(gem2, amp2);
}

void print_amps_table(gem* gems, gemO* amps, int len)
{
	printf("# Gems\tManagem\tAmps\tPower (rescaled)\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%d\t%.6lf\n", i+1, gem_getvalue(gems+i), gem_getvalue_O(amps+i), gem_amp_power(gems[i], amps[i]));
	printf("\n");
}

void print_spec_table(gem* gems, gemO* amps, double* spec_coeffs, int len)
{
	printf("Managem\tAmps\tPower (resc.)\tSpec coeff\n");
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%.6f\t%.6lf\n", i+1, gem_getvalue_O(amps+i), gem_amp_power(gems[i], amps[i]), spec_coeffs[i]);
	printf("\n");
}

void worker(int len, int output_options, int global_mode, float growth_comb, char* filename, char* filenameA)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);							// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1,0);
	gem_init(pool[0]+1,1,0,1);
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// managem pool filling
	if (prevmax<len-1) {						// if the managems are not enough
		if (global_mode==0) {					// behave as mga_spec -> quit if not enough managems
			fclose(table);
			for (i=0;i<=prevmax;++i) free(pool[i]);		// free
			printf("Gem table stops at %d, not %d\n",prevmax+1,len);
			exit(1);
		}
		else {												// behave as managem_amps -> fill the remaining with false gems
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
	if (global_mode) lena=len/6;					// managem_amps -> len/6
	else lena=2*len;											// mga_spec -> 2x mg value
	gemO* poolO[lena];
	int poolO_length[lena];
	poolO[0]=malloc(sizeof(gemO));
	poolO_length[0]=1;
	gem_init_O(poolO[0],1,1);
	
	int prevmaxA=pool_from_table_O(poolO, poolO_length, lena, tableA);		// amps pool filling
	if (prevmaxA<lena-1) {
		fclose(tableA);
		for (i=0;i<=prevmaxA;++i) free(poolO[i]);		// free
		printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	int j,k,h;											// let's choose the right gem-amp combo
	gemO amps[len];
	gem gems[len];
	gem_init(gems,1,1,0);
	amps[0]=(gemO){0};
	printf("Total value:\t1\n\n");
	printf("Managem:\n");
	gem_print(gems);
	printf("Amplifier:\n");
	gem_print_O(amps);
	double spec_coeffs[len];
	spec_coeffs[0]=0;
	
	if (global_mode) { 							// behave like managem_amps
		for (i=1;i<len;++i) {																	// for every total value
			gems[i]=(gem){0};																		// we init the gems
			amps[i]=(gemO){0};																	// to extremely weak ones
			for (k=0;k<pool_length[i];++k) {										// first we compare the gem alone
				if (gem_power(pool[i][k]) > gem_power(gems[i])) {
					gems[i]=pool[i][k];
				}
			}
			for (j=1;j<=i/6;++j) {															// for every amount of amps we can fit in
				int value = i-6*j;																// this is the amount of gems we have left
				for (k=0;k<pool_length[value];++k)								// we search in that pool
				if (pool[value][k].leech!=0) {										// if the gem has leech we go on
					for (h=0;h<poolO_length[j-1];++h) {							// and we look in the amp pool
						if (gem_amp_more_powerful(pool[value][k],poolO[j-1][h],gems[i],amps[i])) {
							gems[i]=pool[value][k];
							amps[i]=poolO[j-1][h];
						}
					}
				}
			}
			printf("Total value:\t%d\n\n", i+1);
			printf("Managem\n");
			if (prevmax<len-1) printf("Managem limit:\t%d\n", prevmax+1);
			printf("Value:\t%d\n",gem_getvalue(gems+i));
			if (output_options & mask_info) printf("Pool:\t%d\n",pool_length[gem_getvalue(gems+i)-1]);
			gem_print(gems+i);
			printf("Amplifier\n");
			printf("Value:\t%d\n",gem_getvalue_O(amps+i));
			if (output_options & mask_info) printf("Pool:\t%d\n",poolO_length[gem_getvalue_O(amps+i)-1]);
			gem_print_O(amps+i);
			printf("Global power (resc.):\t%f\n\n", gem_amp_power(gems[i], amps[i]));
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}
	else { 										// behave like mga_spec
		for (i=1;i<len;++i) {															// for every gem value
			gems[i]=(gem){0};																// we init the gems
			amps[i]=(gemO){0};															// to extremely weak ones
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
				if (pool[i][k].leech!=0) {										// if the gem has leech we go on
					double Palone = gem_power(pool[i][k]);
					double Pbg = pool[i][k].bbound; 
					for (h=0;h<poolO_length[j];++h) {						// and in the amp pool and compare
						double power = Palone + Pbg * 2.576 * poolO[j][h].leech;		// that number is 6*0.23*2.8/1.5
						double spec_coeff=power*comb_coeff;
						if (spec_coeff>spec_coeffs[i]) {
							spec_coeffs[i]=spec_coeff;
							gems[i]=pool[i][k];
							amps[i]=poolO[j][h];
						}
					}
				}
			}
			printf("Total value:\t%d\n\n", i+1+6*gem_getvalue_O(amps+i));
			printf("Managem\n");
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) printf("Pool:\t%d\n",pool_length[i]);
			gem_print(gems+i);
			printf("Amplifier\n");
			printf("Value:\t%d\n",gem_getvalue_O(amps+i));
			if (output_options & mask_info) printf("Pool:\t%d\n",poolO_length[gem_getvalue_O(amps+i)-1]);
			gem_print_O(amps+i);
			printf("Global power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i]));
			printf("Spec coefficient:\t%f\n\n", spec_coeffs[i]);
			fflush(stdout);								// forces buffer write, so redirection works well
		}
	}

	if (output_options & mask_parens) {
		printf("Managem combining scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed_O(amps+len-1);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Managem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree_O(amps+len-1, "");
		printf("\n");
	}
	if (output_options & mask_table) {
		if (global_mode) print_amps_table(gems, amps, len);
		else print_spec_table(gems, amps, spec_coeffs, len);
	}
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Managem equations:\n");
		print_equations(gems+len-1);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations_O(amps+len-1);
		printf("\n");
	}
	fclose(table);
	fclose(tableA);
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<lena;++i) free(poolO[i]);		// free amps
}


int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_options=0;
	int global_mode=0;
	double growth_comb=0.627216;		// 16c
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
		if (*(p-1)=='t') global_mode=1;
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
	if (filename[0]=='\0') strcpy(filename, "table_mgspec");
	if (filenameA[0]=='\0') strcpy(filenameA, "table_leech");
	worker(len, output_options, global_mode, growth_comb, filename, filenameA);
	return 0;
}
