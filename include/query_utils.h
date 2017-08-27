#ifndef _QUERY_UTILS_H
#define _QUERY_UTILS_H

/* Query utils (table file selection, filename handling) */

#include <stdio.h>
#include <string.h>

void table_selection2(char* input, char* filename, char* filenameA)
{
	char* p=input;
	while (*p != ',' && *p != '\0') p++;
	if (*p==',') *p='\0';			// it's "f,fA"
	else p--;						// it's "f" -> empty string
	strcpy(filename,  input);
	strcpy(filenameA, p+1);
}

void table_selection3(char* input, char* filename, char* filenamec, char* filenameA)
{
	char* p=input;
	while (*p != ',' && *p != '\0') p++;
	if (*p==',') *p='\0';			// it's "f,..."
	else p--;						// it's "f" -> empty string
	char* q=p+1;
	while (*q != ',' && *q != '\0') q++;
	if (*q==',') *q='\0';			// it's "f,fc,fA"
	else q--;						// it's "f,fc" or "f" -> empty string
	strcpy(filename,  input);
	strcpy(filenamec, p+1);
	strcpy(filenameA, q+1);
}

int file_exists (const char* name)
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

#endif // _QUERY_UTILS_H
