#ifndef _GFON_H
#define _GFON_H

/* GemForce Object Notation */
/* Remember to redeclare pool_from_table in the amplifier_utils */

inline int file_exists (const char* name)
{
	FILE* file;
	if ((file = fopen(name, "r"))) {
		fclose(file);
		return 1;
	}
	else return 0;
}

void file_selection(char* filename, const char* table_name)
{
	if (filename[0]!='\0') return;
	else if (file_exists(table_name)) {
		strcpy(filename, table_name);
	}
	else {
		char buffer[256];
		strcat(strcpy(buffer, "gem_tables/"), table_name);
		if (file_exists(buffer)) strcpy(filename, buffer);
		else strcpy(filename, table_name);
	}
}

void line_init(FILE* table, int pool_zero)
{
	switch (pool_zero) {
		case 1:       // combines
			fprintf(table, "1\n-1 0 0\n");
		break;
		case 2:       // specces
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
				int value_father, offset_father;
				int value_mother, offset_mother;
				int integrity_check=fscanf(table, "%x %x %x\n", &value_father, &offset_father, &offset_mother);
				if (integrity_check!=3) exit_on_corruption(ftell(table));
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
	printf(" Done\n\n");
	return prevmax;
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
				fprintf(table, "%x %x", k, place);
				int mom_pool=i-1-k;
				place=pool[i][j].mother - pool[mom_pool];
				fprintf(table, " %x\n", place);
				break;
			}
		}
	}
	fprintf(table, "%d\n\n", i);
	fflush(table);
}


#endif // _GFON_H
