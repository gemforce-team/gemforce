#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
const int ACC=80;					// ACC is for z-axis sorting and for the length of the interval tree
const int ACC_TR=750;			// ACC_TR is for bbound comparisons inside tree
#include "killgem_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"
#include "gfon.h"

#define Namps 8
#define damage_ratio (Namps*0.28*(2.8/3.2))
#define crit_ratio (Namps*0.23*(2.8/1.5))

double gem_amp_power(gem gem1, gemY amp1)
{
	return (gem1.damage+damage_ratio*amp1.damage)*gem1.bbound*(gem1.crit+crit_ratio*amp1.crit)*gem1.bbound;
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
	if (table==NULL) exit(1);						// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1.000000,1,0);		// grade damage crit bbound
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// killgem pool filling
	if (prevmax<len-1) {							// if the killgems are not enough
		if (global_mode==0) {					// behave as kga_spec -> quit if not enough killgems
			fclose(table);
			for (i=0;i<=prevmax;++i) free(pool[i]);		// free
			printf("Gem table stops at %d, not %d\n",prevmax+1,len);
			exit(1);
		}
		else {										// behave as killgem_amps -> fill the remaining with false gems
			for (i=prevmax+1; i<len; ++i) {
				pool_length[i]=1;
				pool[i]=malloc(sizeof(gem));
				pool[i][0]=(gem){0};			// null gems
			}
		}
	}

	gem* poolf[len];
	int poolf_length[len];
	
	for (i=0;i<len;++i) {								// killgem spec compression
		int j;
		float maxcrit=0;
		gem* temp_pool=malloc(pool_length[i]*sizeof(gem));
		for (j=0; j<pool_length[i]; ++j) {			// copy gems and get maxcrit
			temp_pool[j]=pool[i][j];
			maxcrit=max(maxcrit, (pool[i]+j)->crit);
		}
		gem_sort(temp_pool,pool_length[i]);			// work starts
		int broken=0;
		int crit_cells=(int)(maxcrit*ACC)+1;		// this pool will be big from the beginning, but we avoid binary search
		int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
		int* tree=malloc((tree_length+crit_cells+1)*sizeof(int));		// memory improvement, 2* is not needed
		for (j=0; j<tree_length+crit_cells+1; ++j) tree[j]=-1;			// init also tree[0], it's faster
		int index;
		for (j=pool_length[i]-1;j>=0;--j) {										// start from large z
			gem* p_gem=temp_pool+j;
			index=(int)(p_gem->crit*ACC);											// find its place in x
			if (tree_check_after(tree, tree_length, index, (int)(p_gem->bbound*ACC_TR))) {		// look at y
				tree_add_element(tree, tree_length, index, (int)(p_gem->bbound*ACC_TR));
			}
			else {
				p_gem->grade=0;
				broken++;
			}
		}												// all unnecessary gems broken
		free(tree);									// free
		
		poolf_length[i]=pool_length[i]-broken;
		poolf[i]=malloc(poolf_length[i]*sizeof(gem));			// pool init via broken
		index=0;
		for (j=0; j<pool_length[i]; ++j) {							// copying to subpool
			if (temp_pool[j].grade!=0) {
				poolf[i][index]=temp_pool[j];
				index++;
			}
		}
		free(temp_pool);
		if (output_options & mask_info) printf("Killgem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);
	}
	printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena;
	if (global_mode) lena=len/Namps;			// killgem_amps -> len/Namps
	else lena=len;									// kga_spec -> kg value
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

	gemY* poolYf[lena];
	int poolYf_length[lena];
	
	for (i=0; i<lena; ++i) {			// amps pool compression
		int j;
		gemY* temp_pool=malloc(poolY_length[i]*sizeof(gemY));
		for (j=0; j<poolY_length[i]; ++j) {			// copy gems
			temp_pool[j]=poolY[i][j];
		}
		gem_sort_Y(temp_pool,poolY_length[i]);		// work starts
		int broken=0;
		float lim_crit=-1;
		for (j=poolY_length[i]-1;j>=0;--j) {
			if (temp_pool[j].crit<=lim_crit) {
				temp_pool[j].grade=0;
				broken++;
			}
			else lim_crit=temp_pool[j].crit;
		}													// all unnecessary gems marked
		poolYf_length[i]=poolY_length[i]-broken;
		poolYf[i]=malloc(poolYf_length[i]*sizeof(gemY));		// pool init via broken
		int index=0;
		for (j=0; j<poolY_length[i]; ++j) {			// copying to pool
			if (temp_pool[j].grade!=0) {
				poolYf[i][index]=temp_pool[j];
				index++;
			}
		}
		free(temp_pool);
		if (output_options & mask_info) printf("Amp value %d compressed pool size:\t%d\n", i+1, poolYf_length[i]);
	}

	int j,k,h;											// let's choose the right gem-amp combo
	gemY amps[len];
	gem gems[len];
	gem_init(gems,1,1,1,0);
	amps[0]=(gemY){0};
	if (!(output_options & mask_quiet)) {
		printf("Total value:\t1\n\n");
		printf("Killgem\n");
		gem_print(gems);
		printf("Amplifier (x%d)\n", Namps);
		gem_print_Y(amps);
	}
	double spec_coeffs[len];
	spec_coeffs[0]=0;
	
	if (global_mode) {							// behave like killgem_amps
		for (i=1;i<len;++i) {												// for every total value
			gems[i]=(gem){0};													// we init the gems
			amps[i]=(gemY){0};												// to extremely weak ones
			for (k=0;k<poolf_length[i];++k) {							// first we compare the gem alone
				if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
					gems[i]=poolf[i][k];
				}
			}
			for (j=1;j<=i/Namps;++j) {										// for every amount of amps we can fit in
				int value = i-Namps*j;										// this is the amount of gems we have left
				for (k=0;k<poolf_length[value];++k)						// we search in that pool
				if (poolf[value][k].crit!=0) {							// if the gem has crit we go on
					for (h=0;h<poolYf_length[j-1];++h) {				// and we look in the amp pool
						if (gem_amp_more_powerful(poolf[value][k],poolYf[j-1][h],gems[i],amps[i])) {
							gems[i]=poolf[value][k];
							amps[i]=poolYf[j-1][h];
						}
					}
				}
			}
			if (!(output_options & mask_quiet)) {
				printf("Total value:\t%d\n\n", i+1);
				printf("Killgem\n");
				if (prevmax<len-1) printf("Killgem limit:\t%d\n", prevmax+1);
				printf("Value:\t%d\n",gem_getvalue(gems+i));
				if (output_options & mask_info) printf("Pool:\t%d\n",poolf_length[gem_getvalue(gems+i)-1]);
				gem_print(gems+i);
				printf("Amplifier (x%d)\n", Namps);
				printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
				if (output_options & mask_info) printf("Pool:\t%d\n",poolYf_length[gem_getvalue_Y(amps+i)-1]);
				gem_print_Y(amps+i);
				printf("Global power (resc.):\t%f\n\n", gem_amp_power(gems[i], amps[i]));
			}
		}
	}
	else {										// behave like kga_spec
		for (i=1;i<len;++i) {											// for every gem value
			gems[i]=(gem){0};												// we init the gems
			amps[i]=(gemY){0};											// to extremely weak ones
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
				if (output_options & mask_info) printf("Pool:\t%d\n",poolf_length[i]);
				gem_print(gems+i);
				printf("Amplifier (x%d)\n", Namps);
				printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
				if (output_options & mask_info) printf("Pool:\t%d\n",poolYf_length[gem_getvalue_Y(amps+i)-1]);
				gem_print_Y(amps+i);
				printf("Global power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i]));
				printf("Spec coefficient:\t%f\n\n", spec_coeffs[i]);
			}
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Total value:\t%d\n\n", gem_getvalue(gems+len-1)+Namps*gem_getvalue_Y(amps+len-1));
		printf("Killagem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+len-1));
		gem_print(gems+len-1);
		printf("Amplifier\n");
		printf("Value:\t%d\n", gem_getvalue_Y(amps+len-1));
		gem_print_Y(amps+len-1);
		printf("Global power (resc.):\t%f\n", gem_amp_power(gems[len-1], amps[len-1]));
		if (!global_mode) printf("Spec coefficient:\t%f\n", spec_coeffs[len-1]);
		printf("\n");
	}

	gem*  gemf=gems+len-1;  // gem that will be displayed
	gemY* ampf=amps+len-1;  // amp that will be displayed

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
		printf("Total value:\t%d\n\n", gem_getvalue(gems+best_index)+Namps*gem_getvalue_Y(amps+best_index));
		printf("Killgem\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier\n");
		printf("Value:\t%d\n", gem_getvalue_Y(amps+best_index));
		gem_print_Y(amps+best_index);
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
			gemf = gem_putred(poolf[value-1], poolf_length[value-1], value, &red, &gem_array, ampf->damage, ampf->crit, damage_ratio, crit_ratio);
			printf("Setup with red added:\n\n");
			printf("Total value:\t%d\n\n", value+Namps*gem_getvalue_Y(ampf));
			printf("Killgem\n");
			printf("Value:\t%d\n", value);
			gem_print(gemf);
			printf("Amplifier\n");
			printf("Value:\t%d\n", gem_getvalue_Y(ampf));
			gem_print_Y(ampf);
			printf("Global power with red:\t%f\n\n", gem_amp_power(*gemf, *ampf));
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
	if (output_options & mask_table) {
		if (global_mode) print_global_table(gems, amps, len);
		else print_spec_table(gems, amps, spec_coeffs, len);
	}
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Killgem equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier equations:\n");
		print_equations_Y(ampf);
		printf("\n");
	}
	
	fclose(table);
	fclose(tableA);
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
	int output_options=0;
	int global_mode=0;
	double growth_comb=1.414061;		// 16c
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
	if (filename[0]=='\0') strcpy(filename, "table_kgspec");
	if (filenameA[0]=='\0') strcpy(filenameA, "table_crit");
	worker(len, output_options, global_mode, growth_comb, filename, filenameA);
	return 0;
}

