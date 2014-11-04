#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_OB gem;		// the strange order is so that managem_utils knows which gem type are we defining as "gem"
#include "managem_utils.h"
#include "gfon.h"

void worker(int len, int pool_zero, int output_parens, int output_equations, int output_tree, int output_table, int output_info, char* filename)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);							// if the file is not good we exit
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;
	
	if (pool_zero==1) {							// combine
		gem_init(pool[0],1,1,1);
	}
	else {													// spec
		gem_init(pool[0]  ,1,1,0);
		gem_init(pool[0]+1,1,0,1);
	}
	gem_init(gems,1,1,0);
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax<len-1) {
		fclose(table);
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	gem_print(gems);

	for (i=1; i<len; ++i) {
		int j;
		gems[i]=pool[i][0];						// choosing gem (criteria moved to more_power def)
		for (j=1;j<pool_length[i];++j) if (gem_more_powerful(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}

		printf("Value:\t%d\n",i+1);
		if (output_info) {
			printf("Pool:\t%d\n",pool_length[i]);
		}
		gem_print(gems+i);
		fflush(stdout);								// forces buffer write, so redirection works well
	}

	if (output_parens) {
		printf("Combining scheme:\n");
		print_parens(gems+len-1);
		printf("\n\n");
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
	}
	if (output_tree) {
		printf("Gem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
	}
	if (output_table) print_table(gems, len);

	if (output_equations) {		// it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gems+len-1);
		printf("\n");
	}
	
	fclose(table);
	for (i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;		// speccing by default
	int output_parens=0;
	int output_equations=0;
	int output_tree=0;
	int output_table=0;
	int output_info=0;
	char filename[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"ptecif:"))!=-1) {
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
			case 'f':
				strcpy(filename,optarg);
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
		if (*(p-1)=='c') pool_zero=1;
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
	if (filename[0]=='\0') {
		if (pool_zero==2) strcpy(filename, "table_mgspec");
		else strcpy(filename, "table_mgcomb");
	}
	worker(len, pool_zero, output_parens, output_equations, output_tree, output_table, output_info, filename);
	return 0;
}

