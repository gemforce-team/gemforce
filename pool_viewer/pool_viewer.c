#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
typedef struct Gem_OB gem;    // the strange order is so that managem_utils knows which gem type are we defining as "gem"
#include "managem_utils.h"
#include "gfon.h"

void worker(int len, int pool_zero, char* filename)
{
	FILE* table=file_check(filename);      // file is open to read
	if (table==NULL) exit(1);              // if the file is not good we exit
	int i;
	gem** pool=malloc(len*sizeof(gem*));
	int pool_length[len];
	pool[0]=malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;
	
	if (pool_zero==1) {                    // combine
		gem_init(pool[0],1,1,1);
	}
	else {                                 // spec
		gem_init(pool[0]  ,1,1,0);
		gem_init(pool[0]+1,1,0,1);
	}
	
	int prevmax=pool_from_table(pool, pool_length, len, table);    // pool filling
	fclose(table);				// close
	if (prevmax<len-1) {
		for (i=0;i<=prevmax;++i) free(pool[i]);      // free
		free(pool);				// free
		if (prevmax>0) printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	FILE* data;
	char buffer[256];
	sprintf(buffer, "data_%s%d", filename, len);
	data=fopen(buffer,"rb");				// binary to check size

	if(data==NULL) {
		data=fopen(buffer,"w");				// creation
		fprintf(data, "# Data from %s\n",filename);
	}
	else {
		printf("File already exists, exiting...\n");
		fclose(data);
		exit(1);
	}										// we now have the datafile

	i=len-1;
	int j;
	for (j=0;j<pool_length[i];++j) {
		fprintf(data, "%d %f %f\n", pool[i][j].grade, pool[i][j].leech, pool[i][j].bbound);
	}
	printf("%s built\n", buffer);
	
	fclose(data);
	for (i=0;i<len;++i) free(pool[i]);		// free
	free(pool);    // free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;		// speccing by default
	char filename[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"f:"))!=-1) {
		switch(opt) {
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
		sprintf(filename, "table_mg%s", pool_zero==1 ? "comb" : "spec");
	}
	worker(len, pool_zero, filename);
	return 0;
}

