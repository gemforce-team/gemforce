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

void worker(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, int size)
{
	// utils compatibility
}

float gem_amp_power(gem gem1, gemO amp1)
{
	return (gem1.leech+4*0.23*2.8*amp1.leech)*gem1.bbound;		// yes, 4, because of 1.5 rescaling
}

int gem_alone_more_powerful(gem gem1, gem gem2, gemO amp2)
{
	return gem1.leech*gem1.bbound > gem_amp_power(gem2, amp2);
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

void worker_amps(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_info, int managem_limit, char* filename, char* filenameA)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);							// if the file is not good we exit
	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);						// if the file is not good we exit
	
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1,0);
	gem_init(pool[0]+1,1,0,1);
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// managem pool filling
	if (prevmax<len-1) {				// FIXME managem_limit
		fclose(table);
		fclose(tableA);
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	int lena=len/6;
	gemO* poolO[lena];
	int poolO_length[lena];
	poolO[0]=malloc(sizeof(gemO));
	poolO_length[0]=1;
	gem_init_O(poolO[0],1,1);

	prevmax=pool_from_table_O(poolO, poolO_length, lena, tableA);		// amps pool filling
	if (prevmax<lena-1) {
		fclose(tableA);
		for (i=0;i<=prevmax;++i) free(poolO[i]);		// free
		printf("Amp table stops at %d, not %d\n",prevmax+1,lena);
		exit(1);
	}

	int j,k,h;											// let's choose the right gem-amp combo
	gemO amps[len];
	gem gems[len];
	gem_init(gems,1,1,0);
	gem_init_O(amps,0,0);
	printf("Total value: 1\n");
	printf("Gem:\n");
	gem_print(gems);
	printf("Amplifier:\n");
	gem_print_O(amps);

	for (i=1;i<len;++i) {																	// for every total value
		gem_init(gems+i,0,0,0);															// we init the gems
		gem_init_O(amps+i,0,0);															// to extremely weak ones
		for (j=0;j<=i/6;++j) {															// for every amount of amps we can fit in
			int value = i-6*j;																// this is the amount of gems we have left
			for (k=0;k<pool_length[value];++k) {							// we search in that pool
				if (j!=0) {																			// and if we need an amp
					for (h=0;h<poolO_length[j-1];++h) {						// we look in the amp pool
						if (pool[value][k].leech!=0 && gem_amp_more_powerful(pool[value][k],poolO[j-1][h],gems[i],amps[i])) {
							gems[i]=pool[value][k];
							amps[i]=poolO[j-1][h];
						}
					}
				}
				else if (gem_alone_more_powerful(pool[value][k],gems[i],amps[i])) {
					gems[i]=pool[value][k];
					gem_init_O(amps+i,0,0);
				}
			}
		}
		printf("Total value:\t%d\n\n", i+1);
		printf("Managem\n");
		if (managem_limit!=0) printf("Managem limit:\t%d\n", managem_limit);
		printf("Value:\t%d\n",gem_getvalue(gems+i));
		if (output_info) printf("Pool:\t%d\n",pool_length[gem_getvalue(gems+i)-1]);
		gem_print(gems+i);
		printf("Amplifier\n");
		printf("Value:\t%d\n",gem_getvalue_O(amps+i));
		if (output_info) printf("Pool:\t%d\n",poolO_length[gem_getvalue_O(amps+i)-1]);
		gem_print_O(amps+i);
		printf("Global power (rescaled):\t%f\n\n", gem_amp_power(gems[i], amps[i]));
		fflush(stdout);								// forces buffer write, so redirection works well
	}

	if (output_parens) {
		printf("Managem combining scheme:\n");
		print_parens(gems+len-1);
		printf("\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_O(amps+len-1);
		printf("\n");
		print_parens_compressed_O(amps+len-1);
		printf("\n\n");
	}
	if (output_tree) {
		printf("Managem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
		printf("Amplifier tree:\n");
		print_tree_O(amps+len-1, "");
		printf("\n");
	}
	if (output_table) print_amps_table(gems, amps, len);

	if (output_equations) {		// it ruins gems, must be last
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
	int output_parens=0;
	int output_equations=0;
	int output_tree=0;
	int output_table=0;
	int output_info=0;
	int managem_limit=0;
	char filename[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"ptecif:l:"))!=-1) {
		switch(opt) {
			case 'p':
				output_parens = 1;
				break;
			case 't':
				output_tree = 1;
				break;
			case 'e':
				output_equations = 1;
				break;
			case 'c':
				output_table = 1;
				break;
			case 'i':
				output_info = 1;
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
			case 'l':
				managem_limit = atoi(optarg);
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
		//if (*(p-1)=='c') pool_zero=1;			FIXME
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
	worker_amps(len, output_parens, output_equations, output_tree, output_table, output_info, managem_limit, filename, filenameA);
	return 0;
}
