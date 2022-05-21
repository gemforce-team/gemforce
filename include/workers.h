#ifndef _WORKERS_H
#define _WORKERS_H

#include "2D_utils.h"
#include "alone_utils.h"
#include "gfon.h"

// --------------
// Worker section
// --------------

template<class gem, class pool_filler>
void worker_build(const cmdline_options& options, pool_filler&& fill_pool)
{
	int pool_zero = options.target.pool_zero;
	FILE* table = table_init(options.tables[0], pool_zero);

	int len = options.target.len;
	vector pool = init_pool<gem>(len, pool_zero);
	vector pool_length = init_pool_length(len, pool_zero);
	
	int prevmax = pool_from_table(pool, pool_length, len, table);   // pool filling
	if (prevmax+1==len) {
		fclose(table);       // close
		pool.~vector();
		pool_length.~vector();
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table = freopen(NULL, "a", table);  // append -> updating possible

	for (int i=prevmax+1; i < len; ++i) {      // more building
		size_t comb_tot = fill_pool(pool, pool_length, i);

		print_build_iteration(options, i+1, pool_length[i], comb_tot);
		table_write_iteration(pool, pool_length, i, table);    // write on file
	}
	
	fclose(table);      // close
}

template<int NCHECKS, class gem, class cascade_checker>
void worker_build_c6(const cmdline_options& options, cascade_checker&& cascade_checks)
{
    auto fill_pool = [&](vector<pool_t<gem>>& pool, vector<size_t>& pool_length, int i) {
        return fill_pool_c6<NCHECKS, gem>(pool, pool_length, i, cascade_checks);
    };
    return worker_build<gem>(options, fill_pool);
}

template<class gem, class pool_filler>
void worker_combine(const cmdline_options& options, pool_filler&& fill_pool)
{
	printf("\n");
	int len = options.target.len;
	int pool_zero = options.target.pool_zero;
	vector pool = init_pool<gem>(len, pool_zero);
	vector pool_length = init_pool_length(len, pool_zero);
	vector gems = init_gems(pool);
	if (!options.output.quiet) gem_print(gems + 0);

	for (int i = 1; i < len; ++i) {
		size_t comb_tot = fill_pool(pool, pool_length, i);
		
		compression_1D(gems + i, pool[i], pool_length[i]);
		print_winner_iteration(options, i+1, gems[i], pool_length[i], comb_tot);
	}
	
	print_winner(options, len, gems[len-1], pool_length[len-1]);  // outputs last if we never seen any

	gem* gemf=gems+len-1;  // gem that will be displayed

	if (options.target.upto) {
		gemf = find_best_upto_alone(gems);
	}

	vector<gem> chain_gems;
	if (options.target.chain) {
		if (len < pool_zero + 1)
			printf("I could not add chain!\n\n");
		else {
			int value = gem_getvalue(gemf);
			gemf = gem_putchain(pool[value-1], pool_length[value-1], chain_gems);
			printf("Gem with chain added:\n\n");
			printf("Value:\t%d\n", value);    // made to work well with -u
			printf("Growth:\t%f\n", log(gem_power(*gemf))/log(value));
			gem_print(gemf);
		}
	}

	pretty_print_alone(gems, gemf, options);
}

template<class gem>
void worker_query(const cmdline_options& options)
{
	FILE* table = file_check(options.tables[0]);    // file is open to read
	if (table == NULL) exit(1);              // if the file is not good we exit

	int len = options.target.len;
	int pool_zero = options.target.pool_zero;
	vector pool = init_pool<gem>(len, pool_zero);
	vector pool_length = init_pool_length(len, pool_zero);
	vector gems = init_gems(pool);
	
	int prevmax = pool_from_table(pool, pool_length, len, table);    // pool filling
	fclose(table);				// close
	if (prevmax < len-1) {
		pool.~vector();
		pool_length.~vector();
		gems.~vector();
		if (prevmax != -1) printf("Table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	bool skip_computations = options.output.quiet && !(options.print.table || options.target.upto);
	int first = skip_computations ? len-1 : 0;
	for (int i = first; i < len; ++i) {
		compression_1D(gems + i, pool[i], pool_length[i]);
		print_winner_iteration(options, i+1, gems[i], pool_length[i]);
	}
	
	print_winner(options, len, gems[len-1], pool_length[len-1]);  // outputs last if we never seen any

	gem* gemf=gems+len-1;  // gem that will be displayed

	if (options.target.upto) {
		gemf = find_best_upto_alone(gems);
	}

	vector<gem> chain_gems;
	if (options.target.chain) {
		if (len < pool_zero + 1)
			printf("I could not add chain!\n\n");
		else {
			int value = gem_getvalue(gemf);
			gemf = gem_putchain(pool[value-1], pool_length[value-1], chain_gems);
			printf("Gem with chain added:\n\n");
			printf("Value:\t%d\n", value);    // made to work well with -u
			printf("Growth:\t%f\n", log(gem_power(*gemf))/log(value));
			gem_print(gemf);
		}
	}

	pretty_print_alone(gems, gemf, options);
}

#endif // _WORKERS_H
