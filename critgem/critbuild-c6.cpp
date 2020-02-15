#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "crit_utils.h"
#include "gfon.h"
#include "cmdline_options.h"

using gem = gem_Y;

constexpr int nchecks=6;

inline double gem_rk23(gem gem1)
{
	return gem1.damage*gem1.damage*gem1.crit*gem1.crit*gem1.crit;
}

inline double gem_rk32(gem gem1)
{
	return gem1.damage*gem1.damage*gem1.damage*gem1.crit*gem1.crit;
}

inline double gem_rk12(gem gem1)
{
	return gem1.damage*gem1.crit*gem1.crit;
}

inline double gem_rk21(gem gem1)
{
	return gem1.damage*gem1.damage*gem1.crit;
}

void worker(const cmdline_options& options)
{
	FILE* table=table_init(options.tables[0], 1);

	int len = options.target.len;
	gem** pool = (gem**)malloc(len*sizeof(gem*));		// win 262k
	int* pool_length = (int*)malloc(len*sizeof(int));
	pool[0] = (gem*)malloc(sizeof(gem));
	pool_length[0]=1;
	gem_init(pool[0],1,1,1);		// grade damage crit
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax+1==len) {
		fclose(table);
		for (int i=0;i<len;++i) free(pool[i]);		// free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(options.tables[0].c_str(),"a", table);		// append -> updating possible

	for (int i=prevmax+1; i<len; ++i) {
		const int eoc=(i+1)/ (1+1);       // end of combining
		const int j0 =(i+1)/(10+1);       // value ratio < 10
		int comb_tot=0;

		const int ngrades=(int)log2(i+1);
		const int temp_length=nchecks*ngrades;
		gem temp_array[temp_length] = {};      // this will have all the grades
		double pow_array[temp_length] = {};    // this will have all the powers

		for (int j=j0; j<eoc; ++j) {          // combine gems and put them in temp array
			for (int k=0; k< pool_length[j]; ++k) {
				int g1=(pool[j]+k)->grade;
				for (int h=0; h< pool_length[i-1-j]; ++h) {
					int delta=g1 - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {     // grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						int p0 = grd*nchecks;

						if      ( gem_power(temp) > pow_array[p0] ) {			// rk11 check
							pow_array[p0]=gem_power(temp);
							temp_array[p0]=temp;
						}
						else if ( gem_rk23(temp) > pow_array[p0+1] ) {			// rk23 check
							pow_array[p0+1]=gem_rk23(temp);
							temp_array[p0+1]=temp;
						}
						else if ( gem_power(temp) > pow_array[p0+2] ) {			// rk11 check
							pow_array[p0+2]=gem_power(temp);
							temp_array[p0+2]=temp;
						}
						else if ( gem_rk32(temp) > pow_array[p0+3] ) {			// rk32 check
							pow_array[p0+3]=gem_rk32(temp);
							temp_array[p0+3]=temp;
						}
						else if ( gem_rk23(temp) > pow_array[p0+4] ) {			// rk23 check
							pow_array[p0+4]=gem_rk23(temp);
							temp_array[p0+4]=temp;
						}
						else if ( gem_power(temp) > pow_array[p0+5] ) {			// rk11 check
							pow_array[p0+5]=gem_power(temp);
							temp_array[p0+5]=temp;
						}
					}
				}
			}
		}
		int gemNum=0;
		for (int j=0; j<temp_length; ++j) if (temp_array[j].grade!=0) gemNum++;
		pool_length[i]=gemNum;
		pool[i] = (gem*)malloc(pool_length[i]*sizeof(gem));
		
		int place=0;
		for (int j=0; j<temp_length; ++j) {				// copying to pool
			if (temp_array[j].grade!=0) {
				pool[i][place]=temp_array[j];
				place++;
			}
		}
		
		if (!options.output.quiet) {
			printf("Value:\t%d\n",i+1);
			if (options.output.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);			// write on file
	}
	
	fclose(table);      // close
	for (int i =0;i<len;++i) free(pool[i]);   // free
	free(pool);         // free
	free(pool_length);  // free
}

int main(int argc, char** argv)
{
	cmdline_options options = cmdline_options();
	options.set_num_tables(1);

	if(!options.parse_args(argc, argv))
		return 1;
	if (options.tables[0].empty()) {
		options.tables[0] = "table_critcomb";
	}

	worker(options);
	return 0;
}

