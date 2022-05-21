#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <getopt.h>
#include <cmath>
#include <cstring>
#include <cctype>

#include "0D_utils.h"
#include "print_utils.h"
#include "gfon.h"
#include "cmdline_options.h"

using namespace std;

class gem
{
public:
	int grade;
	gem* father;
	gem* mother;
	
	int value;
	char color;

public:
	gem() {}
	explicit gem(char color) : grade(1), father(nullptr), mother(nullptr), value(1), color(color) {}
};

inline char gem_color(const gem* gemf)
{
	return gemf->color;
}

inline void gem_init(gem* gemf, char color)
{
	new(gemf) gem(color);
}

void gem_combine(gem* p_gem1, gem* p_gem2, gem* p_gem_combined)
{
	p_gem_combined->father = p_gem1;
	p_gem_combined->mother = p_gem2;
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta) {
		case 0:
			p_gem_combined->grade = p_gem1->grade + 1;
			break;
		case 1:
			p_gem_combined->grade = p_gem1->grade;
			break;
		case -1:
			p_gem_combined->grade = p_gem2->grade;
			break;
		default:
			p_gem_combined->grade = std::max(p_gem1->grade, p_gem2->grade);
			break;
	}
	p_gem_combined->color = p_gem1->color == p_gem2->color ? p_gem1->color : '?';
	p_gem_combined->value = p_gem1->value + p_gem2->value;
}

void print_stuff(gem* gemf, const cmdline_options& options)
{
	if (options.print.parens) {
		print_parens_compressed(gemf);
	}
	if (options.print.tree) {
		print_tree(gemf, "");
	}
	if (options.print.equations) {
		print_equations(gemf);
	}
	printf("\n");
}

void worker(const cmdline_options& options)
{
	FILE* table=file_check(options.tables[0]);      // file is open to read
	if (table == NULL) exit(1);              // if the file is not good we exit

	int len = options.target.len;
	int pool_zero = options.target.pool_zero;
	vector pool = vector<pool_t<gem>>(len);
	vector pool_length = init_pool_length(len, pool_zero);
	pool[0] = make_uninitialized_pool<gem>(pool_zero);

	if (pool_zero==1) {                    // combine
		gem_init(pool[0]+0, 'g');
	}
	else {                                 // spec
		gem_init(pool[0]+0, 'x');
		gem_init(pool[0]+1, 'y');
	}

	int prevmax = pool_from_table(pool, pool_length, len, table);    // pool filling
	fclose(table);				// close
	if (prevmax < len-1) {
		pool.~vector();
		pool_length.~vector();
		if (prevmax != -1) printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}
	
	pool_t<gem>& poolf = pool[len - 1];
	size_t poolf_length = pool_length[len - 1];

	int pos = options.target.lenc;

	if (pos == -1) {
		for (size_t i = 0; i < poolf_length; i++) {
			print_stuff(poolf + i, options);
		}
	}
	else {
		print_stuff(poolf + pos, options);
	}
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.has_printing();
	options.set_num_tables(1);
	options.has_lenc(-1);
	options.has_pool_zero(2);

	options.print.parens = true;

	if(!options.parse_args(argc, argv))
		return 1;

	worker(options);
	return 0;
}
