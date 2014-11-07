#ifndef _GFON_H
#define _GFON_H

/* GemForce Object Notation */
/* Remember to redeclare pool_from_table in the amplifier_utils */

void line_init(FILE* table, int pool_zero)
{
	switch (pool_zero) {
		case 1:				// combines
			fprintf(table, "1\n-1 0 0\n\n");
		break;
		case 2:				// specces
			fprintf(table, "2\n-1 1 0\n");
			fprintf(table, "-1 0 1\n\n");
		break;
		default:
		break;
	}
}

FILE* table_init(char* filename, int pool_zero)
{
	FILE* table;
	table=fopen(filename,"rb");				// binary to check size
	if(table==NULL) {
		table=fopen(filename,"w");			// creation
		line_init(table, pool_zero);		// printed g1
	}
	else {
		fseek(table, 0, SEEK_END);
		if (ftell(table)==0) {
			table=freopen(filename,"w", table);					// init
			line_init(table, pool_zero);			// printed g1
		}
	}																			// we now have the file with at least g1
	table=freopen(filename,"r", table);		// read
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
	printf("\nBuilding pool...");
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
				fscanf(table, "%d %x %x\n", &value_father, &offset_father, &offset_mother);
				if (value_father != -1) {
					value_mother=i-1-value_father;
					gem_combine(pool[value_father]+offset_father, pool[value_mother]+offset_mother, pool[i]+j);
				}
				else pool[i][j]=(gem){0};			// 0-NULL init
			}
			fscanf(table, "\n");						// discard newline
			prevmax++;
		}
	}
	printf(" Done\n\n");
	return prevmax;
}

void table_write_iteration(gem** pool, int* pool_length, int iteration, FILE* table)
{
	int i=iteration;
	int j,k;
	fprintf(table, "%d\n", pool_length[i]);
	for (j=0;j<pool_length[i];++j) {
		if (pool[i][j].father==NULL) fprintf(table, "-1 0 0\n");		// solve false g2(3) pointer problem
		else {
			for (k=0; ; k++) {								// find and print parents
				int place=pool[i][j].father - pool[k];
				if (place < pool_length[k] && place >=0) {
					fprintf(table, "%d %x", k, place);
					int mom_pool=i-1-k;
					place=pool[i][j].mother - pool[mom_pool];
					fprintf(table, " %x\n", place);
					break;
				}
			}
		}
	}
	fprintf(table, "\n");
	fflush(table);
}


#endif // _GFON_H
