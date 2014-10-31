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

void worker(int len, int output_parens, int output_equations, int output_tree, int output_table, int output_info)
{
	printf("\n");
	int i;
	FILE* table;
	table=fopen("leechtable","rb");				// binary to check size
	if(table==NULL) {
		printf("Unexistant table\n");
		exit(1);
	}
	else {
		fseek(table, 0, SEEK_END);
		if (ftell(table)==0) {
			fclose(table);
			printf("Unexistant table\n");
			exit(1);
		}
	}
	table=freopen("leechtable","r", table);							// read

	gem* gems=malloc(len*sizeof(gem));		// if not malloc-ed 131k is the limit
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(sizeof(gem));
	gem_init(gems,1,1);
	gem_init(pool[0],1,1);
	pool_length[0]=1;
	gem_print(gems);

	for (i=0;i<1+pool_length[0];++i) {						// discard value 0 gems
		fscanf(table, "%*[^\n]\n");
	}
	fscanf(table, "\n");													// discard newline
	int prevmax=0;
	for (i=1;i<len;++i) {
		int eof_check=fscanf(table, "%d\n", pool_length+i);				// get pool length
		if (eof_check==-1) break;
		else {
			pool[i]=malloc(pool_length[i]*sizeof(gem));
			int j;
			for (j=0; j<pool_length[i]; ++j) {
				int value_father, offset_father;
				int value_mother, offset_mother;
				fscanf(table, "%d %la %d %d %d\n", &pool[i][j].grade, &pool[i][j].leech, &value_father, &offset_father, &offset_mother);
				value_mother=i-1-value_father;
				pool[i][j].father=pool[value_father]+offset_father;
				pool[i][j].mother=pool[value_mother]+offset_mother;
			}
			fscanf(table, "\n");						// discard newline
			prevmax++;
		}
	}
	if (prevmax<len-1) {
		fclose(table);
		printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	table=freopen("leechtable","a", table);				// append -> updating possible
	
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
	
	for (i=0;i<len;++i) free(pool[i]);		// free
	free(gems);
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

	while ((opt=getopt(argc,argv,"petci"))!=-1) {
		switch(opt) {
			case 'p':
				output_parens = 1;
				break;
			case 'e':
				output_equations = 1;
				break;
			case 't':
				output_tree = 1;
				break;
			case 'c':
				output_table = 1;
				break;
			case 'i':
				output_info = 1;
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
	if (len<1) printf("Improper gem number\n");
	else worker(len, output_parens, output_equations, output_tree, output_table, output_info);
	return 0;
}


