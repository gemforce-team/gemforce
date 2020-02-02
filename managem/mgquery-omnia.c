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
#include "query_utils.h"
#include "gfon.h"
#include "print_utils.h"

void worker(int len, int lenc, options output_options, char* filename, char* filenamec, char* filenameA, int TC, int As, int GT, int Namps)
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
	if (!output_options.quiet) printf("Gem speccing pool compression done!\n");

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena=len;									// see which is bigger between spec len and comb len
	if (lenc > len) lena=lenc;					// and we'll get the amp pool till there
	gemO** poolO=malloc(lena*sizeof(gemO*));
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
	
	gemO* bestO=malloc(lena*sizeof(gemO));		// if not malloc-ed 140k is the limit
	
	AMPS_COMPRESSION
	gemO combO=bestO[lenc-1];			// amps fast access combine
	if (!output_options.quiet) printf("Amp pool compression done!\n");

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
	
	gem* poolcf;
	int poolcf_length;
	
	MGCOMB_COMPRESSION
	if (!output_options.quiet) printf("Gem combine compressed pool size:\t%d\n\n",poolcf_length);

	int j,k,l;								// let's choose the right gem-amp combo
	gem gems[len];							// for every speccing value
	gemO amps[len];						// we'll choose the best amps
	gem gemsc[len];						// and the best NC combine
	gemO ampsc[len];						// for both
	double powers[len];
	gem_init(gems,1,1,0);
	gem_init_O(amps,0,0);
	gem_init(gemsc,1,0,0);
	gem_init_O(ampsc,0,0);
	powers[0]=0;
	double iloglenc=1/log(lenc);
	double leech_ratio=Namps*(0.15+As/3*0.004)*2*(1+0.03*TC)/(1+TC/3*0.1);
	double NT=pow(2, GT-1);
	
	int skip_computations = output_options.quiet && !(output_options.table || output_options.upto);
	int first = skip_computations ? len-1 : 0;
	for (i=first; i<len; ++i) {										// for every gem value
		gems[i]=(gem){0};											// we init the gems
		amps[i]=(gemO){0};											// to extremely weak ones
		gemsc[i]=(gem){0};
		ampsc[i]=combO;												// <- this is ok only for mg
																	// first we compare the gem alone
		for (l=0; l<poolcf_length; ++l) {							// first search in the NC gem comb pool
			if (gem_power(poolcf[l]) > gem_power(gemsc[i])) {
				gemsc[i]=poolcf[l];
			}
		}
		for (k=0;k<poolf_length[i];++k) {							// and then in the the gem pool
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int NS=i+1;
		double c0 = log(NT/(i+1))*iloglenc;							// last we compute the combination number
		powers[i] = pow(gem_power(gemsc[i]),c0) * gem_power(gems[i]);
																	// now we compare the whole setup
		for (j=0, NS+=Namps; j<i+1; ++j, NS+=Namps) {				// for every amp value from 1 up to gem_value
			double c = log(NT/NS)*iloglenc;							// we compute the combination number
			double Ca= leech_ratio * pow(combO.leech,c);			// <- this is ok only for mg
			double Pa= Ca * bestO[j].leech;							// <- because we already know the best amps
			for (l=0; l<poolcf_length; ++l) {						// then we search in NC gem comb pool
				double Cbg = pow(poolcf[l].bbound,c);
				double Cg  = pow(gem_power(poolcf[l]),c);
				for (k=0; k<poolf_length[i]; ++k) {					// and in the reduced gem pool
					double Palone = Cg * gem_power(poolf[i][k]);
					double Pbg = Cbg * poolf[i][k].bbound;
					double power = Palone + Pbg * Pa;  
					if (power>powers[i]) {
						powers[i]=power;
						gems[i]=poolf[i][k];
						amps[i]=bestO[j];
						gemsc[i]=poolcf[l];
					}
				}
			}
		}
		if (!output_options.quiet) {
			printf("Managem spec\n");
			printf("Value:\t%d\n",i+1);
			if (output_options.debug) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_O(amps+i));
			gem_print_O(amps+i);
			printf("Managem combine\n");
			printf("Comb:\t%d\n",lenc);
			if (output_options.debug) printf("Pool:\t%d\n",poolcf_length);
			gem_print(gemsc+i);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print_O(ampsc+i);
			printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[i], amps[i], leech_ratio));
			printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[i]);
		}
	}
	
	if (output_options.quiet) {		// outputs last if we never seen any
		printf("Managem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n",gem_getvalue_O(amps+len-1));
		gem_print_O(amps+len-1);
		printf("Managem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+len-1);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_O(ampsc+len-1);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], leech_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[len-1]);
	}

	gem*  gemf = gems+len-1;  // gem  that will be displayed
	gemO* ampf = amps+len-1;  // amp  that will be displayed
	gem*  gemfc=gemsc+len-1;  // gemc that will be displayed
	gemO* ampfc=ampsc+len-1;  // ampc that will be displayed

	if (output_options.upto) {
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
		printf("Managem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+best_index);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_O(ampsc+best_index);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], leech_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[best_index]);
		gemf = gems+best_index;
		ampf = amps+best_index;
		gemfc = gemsc+best_index;
		ampfc = ampsc+best_index;
	}

	gem* gem_array = NULL;
	if (output_options.chain) {
		if (len < 3) printf("I could not add chain!\n\n");
		else {
			int value = gem_getvalue(gemf);
			int valueA= gem_getvalue_O(ampf);
			double NS = value + Namps*valueA;
			double c = log(NT/NS)*iloglenc;
			double amps_resc_coeff = pow((ampfc->leech/gemfc->leech), c);
			double amp_leech_scaled = leech_ratio*amps_resc_coeff*ampf->leech;
			gemf = gem_putchain(poolf[value-1], poolf_length[value-1], &gem_array, amp_leech_scaled);
			printf("Setup with chain added:\n\n");
			printf("Managem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n", valueA);
			gem_print_O(ampf);
			printf("Managem combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print_O(ampfc);
			if (output_options.debug) printf("Leech rescaling coeff.:   \t%f\n", amps_resc_coeff);
			printf("Spec base power with chain:\t%#.7g\n", gem_amp_power(*gemf, *ampf, leech_ratio));
			double CgP = pow(gem_power(*gemfc), c);
			printf("Global power w. chain at g%d:\t%#.7g\n\n\n", GT, CgP*gem_cfr_power(*gemf, amp_leech_scaled));
		}
	}

	if (output_options.parens) {
		printf("Managem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed_O(ampf);
		printf("\n\n");
		printf("Managem combining scheme:\n");
		print_parens_compressed(gemfc);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed_O(ampfc);
		printf("\n\n");
	}
	if (output_options.tree) {
		printf("Managem speccing tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree_O(ampf, "");
		printf("\n");
		printf("Managem combining tree:\n");
		print_tree(gemfc, "");
		printf("\n");
		printf("Amplifier combining tree:\n");
		print_tree_O(ampfc, "");
		printf("\n");
	}
	if (output_options.table) print_omnia_table(amps, powers, len);
	
	if (output_options.equations) {		// it ruins gems, must be last
		printf("Managem speccing equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations_O(ampf);
		printf("\n");
		printf("Managem combining equations:\n");
		print_equations(gemfc);
		printf("\n");
		printf("Amplifier combining equations:\n");
		print_equations_O(ampfc);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);     // free gems
	for (i=0;i<len;++i) free(poolf[i]);    // free gems compressed
	for (i=0;i<lenc;++i) free(poolc[i]);   // free gems
	free(poolc);
	free(poolc_length);
	free(poolcf);
	for (i=0;i<lena;++i) free(poolO[i]);   // free amps
	free(poolO);
	free(bestO);                           // free amps compressed
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
	int Namps=6;
	options output_options = (options){0};
	char filename[256]="";		// it should be enough
	char filenamec[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"hptecdqurf:T:A:N:G:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecdquf:T:A:N:G:");
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
	file_selection(filename, "table_mgspec");
	file_selection(filenamec, "table_mgcomb");
	file_selection(filenameA, "table_leech");
	worker(len, lenc, output_options, filename, filenamec, filenameA, TC, As, GT, Namps);
	return 0;
}
