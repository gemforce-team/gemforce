#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

typedef struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double leech;		//using float does NOT improve time/memory usage
	struct Gem_O* father;
	struct Gem_O* mother;
} gem;

void line_from_table(FILE* table, gem* p_gem, int* value_father, int* offset_father, int* offset_mother)
{
	fscanf(table, "%d %la %d %d %d\n", &(p_gem->grade), &(p_gem->leech), value_father, offset_father, offset_mother);
}

void write_stats(FILE* table, gem* p_gem)
{
	fprintf(table, " %la", p_gem->leech);
}

#include "gfon.h"

void gem_print(gem *p_gem) {
	printf("Grade:\t%d\nLeech:\t%f\n\n", p_gem->grade, p_gem->leech);
}

void gem_init(gem *p_gem, int grd, double leech)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_better(gem gem1, gem gem2)
{
	return gem1.leech>gem2.leech;
}

char gem_color(gem* p_gem) {
	return 'o';
}

double gem_power(gem gem1) {
	return gem1.leech;
}

#include "print_utils.h"

void worker(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_info, char* filename)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);							// if the file is not good we exit
	int i;
	gem* gems=malloc(len*sizeof(gem));		// if not malloc-ed 230k is the limit
	gem** pool=malloc(len*sizeof(gem*));	// if not malloc-ed 690k is the limit
	int pool_length[len];
	pool[0]=malloc(sizeof(gem));
	gem_init(gems,1,1);
	gem_init(pool[0],1,1);
	pool_length[0]=1;

	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax<len-1) {
		fclose(table);
		for (i=0;i<len;++i) free(pool[i]);		// free
		free(pool);		// free
		free(gems);		// free
		printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	gem_print(gems);

	for (i=1;i<len;++i) {
		int j;
		gems[i]=pool[i][0];
		for (j=1;j<pool_length[i];++j) if (gem_better(pool[i][j],gems[i])) {
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
	free(pool);		// free
	free(gems);		// free
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
	if (filename[0]=='\0') strcpy(filename, "table_leech");
	worker(len, output_parens, output_equations, output_tree, output_table, output_info, filename);
	return 0;
}


