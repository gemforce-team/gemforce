#ifndef _GEM_UTILS_H
#define _GEM_UTILS_H

typedef struct Options {
	unsigned parens:1;
	unsigned tree:1;
	unsigned equations:1;
	unsigned table:1;
	unsigned info:1;
	unsigned debug:1;
	unsigned quiet:1;
	unsigned upto:1;
	unsigned red:1;
} options;

/* Macro blobs used for all flag options. Handle with care */
#define DQ_OPTIONS_BLOCK\
	case 'd':								\
		output_options.debug = 1;			\
		break;								\
	case 'q':								\
		output_options.quiet = 1;			\
		break;

#define PTECIDQUR_OPTIONS_BLOCK\
	case 'p':								\
		output_options.parens = 1;			\
		break;								\
	case 't':								\
		output_options.tree = 1;			\
		break;								\
	case 'e':								\
		output_options.equations = 1;		\
		break;								\
	case 'c':								\
		output_options.table = 1;			\
		break;								\
	case 'i':								\
		output_options.info = 1;			\
		break;								\
	DQ_OPTIONS_BLOCK						\
	case 'u':								\
		output_options.upto = 1;			\
		break;								\
	case 'r':								\
		output_options.red = 1;				\
		break;

#include <stdio.h>

void print_help(const char* flags)
{
	// can (and should) be improved
	printf("%s\n", flags);
}

static inline int int_max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

int gem_getvalue(gem* p_gem)
{
	if(p_gem->father==NULL) return 1;
	else return gem_getvalue(p_gem->father)+gem_getvalue(p_gem->mother);
}

int gem_getdepth(gem* p_gem)
{
	if (p_gem->father==NULL)
		return 1;
	return int_max(gem_getdepth(p_gem->father), gem_getdepth(p_gem->mother)) + 1;
}

#endif // _GEM_UTILS_H
