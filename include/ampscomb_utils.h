#ifndef _AMPSCOMB_UTILS_H
#define _AMPSCOMB_UTILS_H

#include <cstdlib>
#include <cstdio>

#include "gem_utils.h"

template<class gemA>
void print_omnia_table(const char* gem_name, const gemA* amps, const double* powers, int len)
{
	printf("%s\tAmps\tPower\n", gem_name);
	for (int i=0; i < len; i++)
		printf("%d\t%d\t%#.7g\n", i+1, gem_getvalue(amps+i), powers[i]);
	printf("\n");
}

#endif // _AMPSCOMB_UTILS_H
