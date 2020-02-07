#ifndef _OPTIONS_UTILS_H
#define _OPTIONS_UTILS_H

struct options {
	unsigned parens:1;
	unsigned tree:1;
	unsigned equations:1;
	unsigned table:1;
	unsigned info:1;
	unsigned debug:1;
	unsigned quiet:1;
	unsigned upto:1;
	unsigned chain:1;
};

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
		output_options.chain = 1;			\
		break;

/* Macro blob that is used for amps flag options. Handle with care */
#define TAN_OPTIONS_BLOCK			\
			case 'T':				\
				TC=atoi(optarg);	\
				break;				\
			case 'A':				\
				As=atoi(optarg);	\
				break;				\
			case 'N':				\
				Namps=atoi(optarg);	\
				break;

#include <cstdio>

void print_help(const char* flags)
{
	// can (and should) be improved
	printf("%s\n", flags);
}

#endif // _OPTIONS_UTILS_H
