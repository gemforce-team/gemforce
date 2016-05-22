#ifndef _GFON_H
#define _GFON_H

/* GemForce Object Notation */
/* Remember to redeclare pool_from_table in the amplifier_utils */

void line_init(FILE* table, int pool_zero)
{
	switch (pool_zero) {
		case 1:       // combines
			fprintf(table, "1\n-1 0 0\n");
		break;
		case 2:       // specs
			fprintf(table, "2\n-1 1 0\n");
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

int fscan64(char* s, int* n)
{
	*n=0;
	if (s==NULL || s[0]=='\0') return 0;
	for (int m=0; *s; m+=6) {
		(*n) += (((int)(*s)-48) << m);
		s++;
	}
	return 1;
}

int pool_from_table(gem** pool, int* pool_length, int len, FILE* table)
{
	printf("\nBuilding pool...");
	rewind(table);
	int i;
	int pool_zero;
	fscanf(table, "%d\n", &pool_zero);        // get pool_zero
	if (pool_zero != pool_length[0]) {        // and check if it's right
		printf("\nWrong table type, exiting...\n");
		return -1;      // the program will then exit gracefully
	}
	for (i=0;i<pool_length[0];++i) {          // discard value 0 gems
		fscanf(table, "%*[^\n]\n");
	}
	fscanf(table, "%d\n\n", &i);              // check iteration number
	if (i!=0) exit_on_corruption(ftell(table));
	int prevmax=0;
	for (i=1;i<len;++i) {
		int eof_check=fscanf(table, "%d\n", pool_length+i);      // get pool length
		if (eof_check==-1) break;
		else {
			pool[i]=malloc(pool_length[i]*sizeof(gem));
			int j;
			for (j=0; j<pool_length[i]; ++j) {
				char b1[9], b2[9], b3[9];
				fscanf(table, "%8s %8s %8s\n", b1, b2, b3);
				int value_father, offset_father;
				int value_mother, offset_mother;
				int check;
				check =fscan64(b1, &value_father);
				check&=fscan64(b2, &offset_father);
				check&=fscan64(b3, &offset_mother);
				if (!check) exit_on_corruption(ftell(table));
				else {
					value_mother=i-1-value_father;
					gem_combine(pool[value_father]+offset_father, pool[value_mother]+offset_mother, pool[i]+j);
				}
			}
			int iteration_check;
			fscanf(table, "%d\n\n", &iteration_check);    // check iteration number
			if (iteration_check!=i) exit_on_corruption(ftell(table));
			prevmax++;
		}
	}
	printf(" %d blocks read\n\n", prevmax+1);
	return prevmax;
}

void fprint64(int n, FILE* steam)
{
	if (n) while (n) {
		fputc((n & 63)+48, steam);
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
