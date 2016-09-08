#ifndef _GFON_H
#define _GFON_H

/* GemForce Object Notation */
/* Remember to redeclare pool_from_table in the amplifier_utils */

#include <stdio.h>
#include <stdlib.h>

static const char base64encode[64] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<',  '=', '>', '?',
                                      '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',  'M', 'N', 'O',
                                      'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
                                      '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',  'm', 'n', 'o'};

static const char base64decode[64] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
                                      16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                                      32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                                      48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

void line_init(FILE* table, int pool_zero)
{
	switch (pool_zero) {
		case 1:       // combines
			fprintf(table, "1\n");
			fprintf(table, "-1 0 0\n");
		break;
		case 2:       // specs
			fprintf(table, "2\n");
			fprintf(table, "-1 0 0\n");
			fprintf(table, "-1 0 1\n");
		break;
		default:
		break;
	}
	fprintf(table, "0\n\n");
}

FILE* table_init(char* filename, int pool_zero)
{
	FILE* table;
	table=fopen(filename,"rb");         // binary to check size
	if(table==NULL) {
		table=fopen(filename,"w");       // creation
		line_init(table, pool_zero);     // printed g1
	}
	else {
		fseek(table, 0, SEEK_END);
		if (ftell(table)==0) {
			table=freopen(filename,"w", table);   // init
			line_init(table, pool_zero);          // printed g1
		}
	}                                           // we now have the file with at least g1
	table=freopen(filename,"r", table);         // read
	return table;
}

FILE* file_check(char* filename)
{
	FILE* table;
	table=fopen(filename,"rb");         // binary to check size
	if(table==NULL) {
		printf("Unexistant table: %s\n",filename);
		return NULL;
	}
	fseek(table, 0, SEEK_END);
	if (ftell(table)==0) {
		fclose(table);
		printf("Empty table: %s\n",filename);
		return NULL;
	}
	table=freopen(filename,"r", table);    // read
	return table;
}

void exit_on_corruption(long int position)
{
	printf("\nERROR: integrity check failed at byte %ld\n", position);
	printf("Your table may be corrupt, brutally exiting...\n");
	exit(1);
}

static int fscan64(char* s, int* n)
{
	*n=0;
	if (s==NULL || s[0]=='\0') return 0;
	for (int m=0; *s; m+=6) {
		(*n) += (base64decode[*s-'0'] << m);
		s++;
	}
	return 1;
}

int pool_from_table(gem** pool, int* pool_length, int len, FILE* table)
{
	printf("\nBuilding pool..."); fflush(stdout);
	rewind(table);

	int pool_zero;
	int check;
	check = fscanf(table, "%d\n", &pool_zero);   // get pool_zero
	if (check!=1) exit_on_corruption(ftell(table));
	if (pool_zero != pool_length[0]) {               // and check if it's right
		printf("\nWrong table type, exiting...\n");
		return -1;      // the program will then exit gracefully
	}

	for (int i=0;i<pool_length[0];++i) {             // discard value 0 gems
		int check = fscanf(table, "%*[^\n]\n");
		if (check!=0) exit_on_corruption(ftell(table));
	}

	int iter;
	check = fscanf(table, "%d\n\n", &iter);      // check iteration number
	if (check!=1) exit_on_corruption(ftell(table));
	if (iter!=0) exit_on_corruption(ftell(table));

	int prevmax=0;
	for (int i=1;i<len;++i) {
		int eof_check=fscanf(table, "%d\n", pool_length+i);      // get pool length
		if (eof_check==EOF) break;
		else {
			pool[i]=malloc(pool_length[i]*sizeof(gem));
			int j;
			for (j=0; j<pool_length[i]; ++j) {
				char b1[9], b2[9], b3[9];
				int nscan = fscanf(table, "%8s %8s %8s\n", b1, b2, b3);
				int value_father;
				int offset_father;
				int offset_mother;
				int check = (nscan == 3);
				check &= fscan64(b1, &value_father);
				check &= fscan64(b2, &offset_father);
				check &= fscan64(b3, &offset_mother);
				if (!check) exit_on_corruption(ftell(table));
				else {
					int value_mother=i-1-value_father;
					gem_combine(pool[value_father]+offset_father, pool[value_mother]+offset_mother, pool[i]+j);
				}
			}
			int iteration_check;
			int check = fscanf(table, "%d\n\n", &iteration_check);    // check iteration number
			if (check!=1 || iteration_check!=i) exit_on_corruption(ftell(table));
			prevmax++;
		}
	}
	printf(" %d blocks read\n\n", prevmax+1);
	return prevmax;
}

static void fprint64(int n, FILE* steam)
{
	if (n) while (n) {
		fputc(base64encode[n & 63], steam);
		n>>=6;
	}
	else fputc('0', steam);
}

void table_write_iteration(gem** pool, int* pool_length, int iteration, FILE* table)
{
	int i=iteration;
	int j;
	fprintf(table, "%d\n", pool_length[i]);
	for (j=0;j<pool_length[i];++j) {
		int k;
		for (k=0; ; k++) {                   // find and print parents
			int place=pool[i][j].father - pool[k];
			if (place < pool_length[k] && place >=0) {
				fprint64(k, table);
				fputc(' ', table);
				fprint64(place, table);
				int mom_pool=i-1-k;
				place=pool[i][j].mother - pool[mom_pool];
				fputc(' ', table);
				fprint64(place, table);
				fputc('\n', table);
				break;
			}
		}
	}
	fprintf(table, "%d\n\n", i);
	fflush(table);
}


#endif // _GFON_H
