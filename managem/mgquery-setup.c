#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
typedef struct Gem_OB gem;		// the strange order is so that managem_utils knows which gem type are we defining as "gem"
#include "managem_utils.h"
typedef struct Gem_O gemO;
#include "leech_utils.h"
#include "mga_utils.h"
#include "gfon.h"

void worker(int len, int lenc, int output_options, char* filename, char* filenamec, char* filenameA, int TC, int As, int GT, int Namps)
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
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// managem spec pool filling
	fclose(table);
	if (prevmax<len-1) {										// if the managems are not enough
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		if (prevmax>0) printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	
	gem* poolf[len];
	int poolf_length[len];
	
	MGSPEC_COMPRESSION
	printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena=len;									// as long as the spec length
	gemO* poolO[lena];
	int poolO_length[lena];
	poolO[0]=malloc(sizeof(gemO));
	poolO_length[0]=1;
	gem_init_O(poolO[0],1,1);
	
	int prevmaxA=pool_from_table_O(poolO, poolO_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (i=0;i<=prevmaxA;++i) free(poolO[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}
	
	gemO* bestO=malloc(lena*sizeof(gem));		// if not malloc-ed 140k is the limit
	
	AMPS_COMPRESSION
	printf("Amp pool compression done!\n");

	FILE* tablec=file_check(filenamec);		// file is open to read
	if (tablec==NULL) exit(1);					// if the file is not good we exit
	gem** poolc=malloc(lenc*sizeof(gem*));
	int* poolc_length=malloc(lenc*sizeof(int));
	poolc[0]=malloc(sizeof(gem));
	poolc_length[0]=1;
	gem_init(poolc[0],1,1,1);
	
	int prevmaxc=pool_from_table(poolc, poolc_length, lenc, tablec);		// managem comb pool filling
	fclose(tablec);
	if (prevmaxc<lenc-1) {									// if the managems are not enough
		for (i=0;i<=prevmaxc;++i) free(poolc[i]);		// free
		if (prevmaxc>0) printf("Gem table stops at %d, not %d\n",prevmaxc+1,lenc);
		exit(1);
	}
	
	gem bestc=(gem){0};				// choosing best combine
	
	for (i=0;i<poolc_length[lenc-1];++i) {
		if (gem_more_powerful(poolc[lenc-1][i], bestc)) {
			bestc=poolc[lenc-1][i];
		}
	}
	double bestc_growth=log(gem_power(bestc))/log(lenc);
	
	printf("Combining pool compression done!\n\n");

	int j,k;									// let's choose the right gem-amp combo
	gem gems[len];							// for every speccing value
	gemO amps[len];						// we'll choose the best amps
	double powers[len];
	gem_init(gems,1,1,0);
	gem_init_O(amps,0,0);
	powers[0]=0;
	double leech_ratio=Namps*(0.15+As/3*0.004)*2*(1+0.03*TC)/(1+TC/3*0.1);
	double NT=pow(2, GT-1);
	if (!(output_options & mask_quiet)) {
		printf("Managem spec\n");
		gem_print(gems);
		printf("Amplifier spec (x%d)\n", Namps);
		gem_print_O(amps);
		printf("Spec base power:    \t0\n\n\n");
	}

	for (i=1;i<len;++i) {													// for every gem value
		gems[i]=(gem){0};														// we init the gems
		amps[i]=(gemO){0};													// to extremely weak ones
		
		for (k=0;k<poolf_length[i];++k) {								// first we compare the gem alone
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int NS=i+1;
		double C0 = pow(NT/(i+1), bestc_growth);						// last we compute the combination number
		powers[i] = C0 * gem_power(gems[i]);
																					// now we compare the whole setup
		for (j=0, NS+=Namps; j<i+1; ++j, NS+=Namps) {				// for every amp value from 1 up to gem_value
			double Cg = pow(NT/NS, bestc_growth);						// we compute the combination number
			double Pa = leech_ratio * bestO[j].leech;					// we already know the best amps
			for (k=0; k<poolf_length[i]; ++k) {							// we look in the reduced gem pool
				double Palone = gem_power(poolf[i][k]);
				double Pbase = Palone + Pa * poolf[i][k].bbound; 
				double power = Cg * Pbase;
				if (power>powers[i]) {
					powers[i]=power;
					gems[i]=poolf[i][k];
					amps[i]=bestO[j];
				}
			}
		}
		if (!(output_options & mask_quiet)) {
			printf("Managem spec\n");
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_debug) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_O(amps+i));
			gem_print_O(amps+i);
			printf("Setup combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(&bestc);
			printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[i], amps[i], leech_ratio));
			printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[i]);
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Managem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n",gem_getvalue_O(amps+len-1));
		gem_print_O(amps+len-1);
		printf("Setup combine\n");
		printf("Comb:\t%d\n",lenc);
		printf("Growth:\t%f\n", bestc_growth);
		gem_print(&bestc);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], leech_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[len-1]);
	}

	gem*  gemf = gems+len-1;  // gem  that will be displayed
	gemO* ampf = amps+len-1;  // amp  that will be displayed
	gem*  gemfc= &bestc;      // gemc that will be displayed

	if (output_options & mask_upto) {
		double best_pow=0;
		int best_index=0;
		for (i=0; i<len; ++i) {
			if (powers[i] > best_pow) {
				best_index=i;
				best_pow=powers[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Managem spec\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue_O(amps+best_index));
		gem_print_O(amps+best_index);
		printf("Setup combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemfc);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], leech_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[best_index]);
		gemf = gems+best_index;
		ampf = amps+best_index;
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 3) printf("I could not add red!\n\n");
		else {
			int value = gem_getvalue(gemf);
			int valueA= gem_getvalue_O(ampf);
			double NS = value + Namps*valueA;
			double amp_leech_scaled = leech_ratio * ampf->leech;
			gemf = gem_putred(poolf[value-1], poolf_length[value-1], value, &red, &gem_array, amp_leech_scaled);
			printf("Setup with red added:\n\n");
			printf("Managem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_O(ampf));
			gem_print_O(ampf);
			printf("Setup combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Spec base power with red:\t%#.7g\n", gem_amp_power(*gemf, *ampf, leech_ratio));
			double CgP = pow(NT/NS, bestc_growth);
			printf("Global power w. red at g%d:\t%#.7g\n\n\n", GT, CgP*gem_cfr_power(*gemf, amp_leech_scaled));
		}
	}

	if (output_options & mask_parens) {
		printf("Managem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed_O(ampf);
		printf("\n\n");
		printf("Setup combining scheme:\n");
		print_parens_compressed(gemfc);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Managem speccing tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree_O(ampf, "");
		printf("\n");
		printf("Setup combining tree:\n");
		print_tree(gemfc, "");
		printf("\n");
	}
	if (output_options & mask_table) print_omnia_table(gems, amps, powers, len);
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Managem speccing equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations_O(ampf);
		printf("\n");
		printf("Setup combining equations:\n");
		print_equations(gemfc);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);     // free gems
	for (i=0;i<len;++i) free(poolf[i]);    // free gems compressed
	for (i=0;i<lenc;++i) free(poolc[i]);   // free gems
	free(poolc);
	free(poolc_length);
	for (i=0;i<lena;++i) free(poolO[i]);   // free amps
	free(bestO);                           // free amps compressed
	if (output_options & mask_red && len > 2) {
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
	int Namps=6;
	int output_options=0;
	char filename[256]="";		// it should be enough
	char filenamec[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"hptecdqurf:T:A:N:G:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecdqurf:T:A:N:G:");
				return 0;
			PTECIDQUR_OPTIONS_BLOCK
			case 'f':			// can be "filename,filenamec,filenameA", if missing default is used
				;
				char* p=optarg;
				while (*p != ',' && *p != '\0') p++;
				if (*p==',') *p='\0';			// ok, it's "f,..."
				else p--;							// not ok, it's "f" -> empty string
				char* q=p+1;
				while (*q != ',' && *q != '\0') q++;
				if (*q==',') *q='\0';			// ok, it's "...,fc,fA"
				else q--;							// not ok, it's "...,fc" -> empty string
				strcpy(filename,optarg);
				strcpy(filenamec,p+1);
				strcpy(filenameA,q+1);
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
	file_selection(filename, "table_mgspec");
	file_selection(filenamec, "table_mgcomb");
	file_selection(filenameA, "table_leech");
	worker(len, lenc, output_options, filename, filenamec, filenameA, TC, As, GT, Namps);
	return 0;
}
