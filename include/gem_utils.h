#ifndef _GEM_UTILS_H
#define _GEM_UTILS_H

enum options_mask {
	mask_parens    = 1 <<  0,
	mask_tree      = 1 <<  1,
	mask_equations = 1 <<  2,
	mask_table     = 1 <<  3,
	mask_info      = 1 <<  4,
	mask_debug     = 1 <<  5,
	mask_quiet     = 1 <<  6,
	mask_upto      = 1 <<  7,
	mask_red       = 1 <<  8,
};

/* Macro blobs used for all flag options. Handle with care */
#define DQ_OPTIONS_BLOCK\
	case 'd':								\
		output_options |= mask_debug;		\
		break;								\
	case 'q':								\
		output_options |= mask_quiet;		\
		break;

#define PTECIDQUR_OPTIONS_BLOCK\
	case 'p':								\
		output_options |= mask_parens;		\
		break;								\
	case 't':								\
		output_options |= mask_tree;		\
		break;								\
	case 'e':								\
		output_options |= mask_equations;	\
		break;								\
	case 'c':								\
		output_options |= mask_table;		\
		break;								\
	case 'i':								\
		output_options |= mask_info;		\
		break;								\
	DQ_OPTIONS_BLOCK						\
	case 'u':								\
		output_options |= mask_upto;		\
		break;								\
	case 'r':								\
		output_options |= mask_red;			\
		break;

#include <stdio.h>

void print_help(const char* flags)
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
