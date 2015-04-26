#ifndef _GEM_UTILS_H
#define _GEM_UTILS_H

#define mask_parens 1
#define mask_tree 2
#define mask_equations 4
#define mask_table 8
#define mask_info 16
#define mask_debug 32
#define mask_quiet 64
#define mask_upto 128
#define mask_red 256

/* Macro blob that is used for all flag options. Handle with care */
#define PTECIDCUR_OPTIONS_BLOCK\
			case 'p':\
				output_options |= mask_parens;\
				break;\
			case 't':\
				output_options |= mask_tree;\
				break;\
			case 'e':\
				output_options |= mask_equations;\
				break;\
			case 'c':\
				output_options |= mask_table;\
				break;\
			case 'i':\
				output_options |= mask_info;\
				break;\
			case 'd':\
				output_options |= mask_debug;\
				break;\
			case 'q':\
				output_options |= mask_quiet;\
				break;\
			case 'u':\
				output_options |= mask_upto;\
				break;\
			case 'r':\
				output_options |= mask_red;\
				break;\

#include <stdio.h>

void print_help(char* flags)
{
	// can (and should) be improved
	printf("%s\n", flags);
}

inline int int_max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

int gem_getvalue(gem* p_gem)
{
	if(p_gem->father==NULL) return 1;
	else return gem_getvalue(p_gem->father)+gem_getvalue(p_gem->mother);
}


#endif // _GEM_UTILS_H
