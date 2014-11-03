#ifndef _GFON_H
#define _GFON_H

FILE* table_init(char* filename)
{
	FILE* table;
	table=fopen(filename,"rb");				// binary to check size
	if(table==NULL) {
		table=fopen(filename,"w");			// creation
		fprintf(table, "1\n1 1.000000 -1 0 -1 0\n\n");		// printed g1
	}
	else {
		fseek(table, 0, SEEK_END);
		if (ftell(table)==0) {
			table=freopen(filename,"w", table);							// init
			fprintf(table, "1\n1 1.000000 -1 0 -1 0\n\n");	// printed g1
		}
	}																			// we now have the file with at least g1
	table=freopen(filename,"r", table);							// read
	return table;
}

FILE* file_check(char* filename)
{
	FILE* table;
	table=fopen(filename,"rb");				// binary to check size
	if(table==NULL) {
		printf("Unexistant table\n");
		return NULL;
	}
	fseek(table, 0, SEEK_END);
	if (ftell(table)==0) {
		fclose(table);
		printf("Empty table\n");
		return NULL;
	}
	table=freopen(filename,"r", table);		// read
	return table;
}

int pool_from_table(gem** pool, int* pool_length, int len, FILE* table)
{
	rewind(table);
	int i;
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
	return prevmax;
}

void table_write_iteration(gem** pool, int* pool_length, int iteration, FILE* table)
{
	int i=iteration;
	int j,k;
	int broken=0;
	if (pool[i][0].father==NULL) broken=1;				// solve false g2 problem
	fprintf(table, "%d\n", pool_length[i]-broken);
	for (j=broken;j<pool_length[i];++j) {
		fprintf(table, "%d %la", pool[i][j].grade, pool[i][j].leech);
		for (k=0; ; k++) {								// print parents
			int place=pool[i][j].father - pool[k];
			if (place < pool_length[k]) {
				fprintf(table, " %d %d", k, place);
				int mom_pool=i-1-k;
				place=pool[i][j].mother - pool[mom_pool];
				fprintf(table, " %d\n", place);
				break;
			}
		}
	}
	fprintf(table, "\n");
	fflush(table);
}


#endif // _GFON_H
