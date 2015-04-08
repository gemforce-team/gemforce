#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
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

void print_global_table(gem* gems, gemO* amps, int len)
{
	printf("# Gems\tManagem\tAmps\tPower (resc.)\n");
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

void worker(int len, int output_options, int global_mode, double growth_comb, char* filename, char* filenameA)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);						// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1,0);
	gem_init(pool[0]+1,1,0,1);
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// managem pool filling
	if (prevmax<len-1) {						// if the managems are not enough
		if (global_mode==0) {				// behave as mga_spec -> quit if not enough managems
			fclose(table);
			for (i=0;i<=prevmax;++i) free(pool[i]);		// free
			printf("Gem table stops at %d, not %d\n",prevmax+1,len);
			exit(1);
		}
		else {									// behave as managem_amps -> fill the remaining with false gems
			for (i=prevmax+1; i<len; ++i) {
				pool_length[i]=1;
				pool[i]=malloc(sizeof(gem));
				pool[i][0]=(gem){0};			// null gems
			}
		}
	}

	gem* poolf[len];
	int poolf_length[len];
	
	for (i=0;i<len;++i) {								// managem spec compression
		int j;
		gem* temp_pool=malloc(pool_length[i]*sizeof(gem));
		for (j=0; j<pool_length[i]; ++j) {			// copy gems
			temp_pool[j]=pool[i][j];
		}
		gem_sort(temp_pool,pool_length[i]);			// work starts
		int broken=0;
		float lim_bbound=-1;
		for (j=pool_length[i]-1;j>=0;--j) {
			if ((int)(ACC*temp_pool[j].bbound)<=(int)(ACC*lim_bbound)) {
				temp_pool[j].grade=0;
				broken++;
			}
			else lim_bbound=temp_pool[j].bbound;
		}													// unnecessary gems broken
		gem best=(gem){0};							// choosing best i-spec
		for (j=0;j<pool_length[i];++j)
		if (gem_more_powerful(temp_pool[j], best)) {
			best=temp_pool[j];
		}
		for (j=0;j<pool_length[i];++j)							// comparison compression (only for mg):
		if (temp_pool[j].bbound < best.bbound
		&&  temp_pool[j].grade!=0)									// a mg makes sense only if
		{																	// its bbound is bigger than
			temp_pool[j].grade=0;									// the bbound of the best one
			broken++;
		}																	// all the unnecessary gems broked
		poolf_length[i]=pool_length[i]-broken;
		poolf[i]=malloc(poolf_length[i]*sizeof(gem));		// pool init via broken
		int index=0;
		for (j=0; j<pool_length[i]; ++j) {						// copying to subpool
			if (temp_pool[j].grade!=0) {
				poolf[i][index]=temp_pool[j];
				index++;
			}
		}
		free(temp_pool);
		if (output_options & mask_info) printf("Managem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);
	}
	printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena;
	if (global_mode) lena=len/6;				// managem_amps -> len/6
	else lena=2*len;								// mga_spec -> 2x mg value
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

	gemO* bestO=malloc(lena*sizeof(gem));		// if not malloc-ed 140k is the limit
	
	for (i=0; i<lena; ++i) {			// amps pool compression
		int j;
		bestO[i]=(gemO){0};
		for (j=0; j<poolO_length[i]; ++j) {
			if (gem_better(poolO[i][j], bestO[i])) {
				bestO[i]=poolO[i][j];
			}
		}
	}
	printf("Amp pool compression done!\n\n");

	int j,k;											// let's choose the right gem-amp combo
	gemO amps[len];
	gem gems[len];
	gem_init(gems,1,1,0);
	amps[0]=(gemO){0};
	if (!(output_options & mask_quiet)) {
		printf("Total value:\t1\n\n");
		printf("Managem:\n");
		gem_print(gems);
		printf("Amplifier:\n");
		gem_print_O(amps);
	}
	double spec_coeffs[len];
	spec_coeffs[0]=0;
	
	if (global_mode) {							// behave like managem_amps
		for (i=1;i<len;++i) {											// for every total value
			gems[i]=(gem){0};												// we init the gems
			amps[i]=(gemO){0};											// to extremely weak ones
			for (k=0;k<poolf_length[i];++k) {						// first we compare the gem alone
				if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
					gems[i]=poolf[i][k];
				}
			}
			for (j=1;j<=i/6;++j) {										// for every amount of amps we can fit in
				int value = i-6*j;										// this is the amount of gems we have left
				for (k=0;k<poolf_length[value];++k)					// we search in that pool
				if (poolf[value][k].leech!=0							// if the gem has leech we go on and get the amp
				&&  gem_amp_more_powerful(poolf[value][k],bestO[j-1],gems[i],amps[i]))
				{
					gems[i]=poolf[value][k];
					amps[i]=bestO[j-1];
				}
			}
			if (!(output_options & mask_quiet)) {
				printf("Total value:\t%d\n\n", i+1);
				printf("Managem\n");
				if (prevmax<len-1) printf("Managem limit:\t%d\n", prevmax+1);
				printf("Value:\t%d\n",gem_getvalue(gems+i));
				if (output_options & mask_info) printf("Pool:\t%d\n",poolf_length[gem_getvalue(gems+i)-1]);
				gem_print(gems+i);
				printf("Amplifier\n");
				printf("Value:\t%d\n",gem_getvalue_O(amps+i));
				if (output_options & mask_info) printf("Pool:\t1\n");
				gem_print_O(amps+i);
				printf("Global power (resc.):\t%f\n\n", gem_amp_power(gems[i], amps[i]));
			}
		}
	}
	else {										// behave like mga_spec
		for (i=1;i<len;++i) {											// for every gem value
			gems[i]=(gem){0};												// we init the gems
			amps[i]=(gemO){0};											// to extremely weak ones
			spec_coeffs[i]=0;												// and init a spec coeff
			for (k=0;k<poolf_length[i];++k) {						// first we compare the gem alone
				if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
					gems[i]=poolf[i][k];
				}
			}
			int NS=i+1;
			double comb_coeff=pow(NS, -growth_comb);
			spec_coeffs[i]=comb_coeff*gem_power(gems[i]);
																				// now with amps
			for (j=0, NS+=6; j<2*i+2; ++j, NS+=6) {										// for every amp value from 1 to to 2*gem_value
				double comb_coeff=pow(NS, -growth_comb);			// we compute comb_coeff
				double Pa= 2.576 * bestO[j].leech;					// <- this is ok only for mg
				for (k=0;k<poolf_length[i];++k) {					// then we search in the reduced gem pool
					if (poolf[i][k].leech!=0) {						// if the gem has leech we go on
						double Palone = gem_power(poolf[i][k]);
						double power = Palone + poolf[i][k].bbound * Pa;
						double spec_coeff=power*comb_coeff;
						if (spec_coeff>spec_coeffs[i]) {
							spec_coeffs[i]=spec_coeff;
							gems[i]=poolf[i][k];
							amps[i]=bestO[j];
						}
					}
				}
			}
			if (!(output_options & mask_quiet)) {
				printf("Total value:\t%d\n\n", i+1+6*gem_getvalue_O(amps+i));
				printf("Managem\n");
				printf("Value:\t%d\n",i+1);
				if (output_options & mask_info) printf("Pool:\t%d\n",poolf_length[i]);
				gem_print(gems+i);
				printf("Amplifier\n");
				printf("Value:\t%d\n",gem_getvalue_O(amps+i));
				if (output_options & mask_info) printf("Pool:\t1\n");
				gem_print_O(amps+i);
				printf("Global power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i]));
				printf("Spec coefficient:\t%f\n\n", spec_coeffs[i]);
			}
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Total value:\t%d\n\n", gem_getvalue(gems+len-1)+6*gem_getvalue_O(amps+len-1));
		printf("Managem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+len-1));
		gem_print(gems+len-1);
		printf("Amplifier\n");
		printf("Value:\t%d\n", gem_getvalue_O(amps+len-1));
		gem_print_O(amps+len-1);
		printf("Global power (resc.):\t%f\n", gem_amp_power(gems[len-1], amps[len-1]));
		if (!global_mode) printf("Spec coefficient:\t%f\n", spec_coeffs[len-1]);
		printf("\n");
	}

	gem*  gemf=gems+len-1;  // gem that will be displayed
	gemO* ampf=amps+len-1;  // amp that will be displayed

	if ((output_options & mask_upto) && !global_mode) {
		double best_sc=0;
		int best_index=0;
		for (i=0; i<len; ++i) {
			if (spec_coeffs[i] > best_sc) {
				best_index=i;
				best_sc=spec_coeffs[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Total value:\t%d\n\n", gem_getvalue(gems+best_index)+6*gem_getvalue_O(amps+best_index));
		printf("Managem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier\n");
		printf("Value:\t%d\n", gem_getvalue_O(amps+best_index));
		gem_print_O(amps+best_index);
		printf("Global power (resc.):\t%f\n", gem_amp_power(gems[best_index], amps[best_index]));
		printf("Spec coefficient:\t%f\n\n", best_sc);
		gemf = gems+best_index;
		ampf = amps+best_index;
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 3) printf("I could not add red!\n\n");
		else {
			int value=gem_getvalue(gemf);
			gemf = gem_putred(poolf[value-1], poolf_length[value-1], value, &red, &gem_array, ampf->leech, 4*0.23*2.8);
			printf("Setup with red added:\n\n");
			printf("Total value:\t%d\n\n", value+6*gem_getvalue_O(ampf));
			printf("Managem\n");
			printf("Value:\t%d\n", value);
			gem_print(gemf);
			printf("Amplifier\n");
			printf("Value:\t%d\n", gem_getvalue_O(ampf));
			gem_print_O(ampf);
			printf("Global power with red:\t%f\n\n", gem_amp_power(*gemf, *ampf));
		}
	}

	if (output_options & mask_parens) {
		printf("Managem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed_O(ampf);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Managem tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree_O(ampf, "");
		printf("\n");
	}
	if (output_options & mask_table) {
		if (global_mode) print_global_table(gems, amps, len);
		else print_spec_table(gems, amps, spec_coeffs, len);
	}
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Managem equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations_O(ampf);
		printf("\n");
	}
	
	fclose(table);
	fclose(tableA);
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (i=0;i<lena;++i) free(poolO[i]);		// free amps
	free(bestO);										// free amps compressed
	if (output_options & mask_red && len > 2) {
		free(gem_array);
	}
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

	while ((opt=getopt(argc,argv,"iptcequrf:g:"))!=-1) {
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
			case 'q':
				output_options |= mask_quiet;
				break;
			case 'u':
				output_options |= mask_upto;
				break;
			case 'r':
				output_options |= mask_red;
				break;
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
		if (optind==argc) printf("No length specified\n");
		else printf("Unknown arguments:\n");
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

