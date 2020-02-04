#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <getopt.h>
#include <cstring>

#include "crit_utils.h"
#include "gem_sort.h"
#include "gfon.h"
#include "options_utils.h"

using gem = gem_Y;

void worker(int len, options output_options, char* filename)
{
	FILE* table=table_init(filename, 1);		// init crit
	int i;
	int size=1000;
	gem* pool[len];
	int pool_length[len];
	pool[0] = (gem*)malloc(sizeof(gem));
	gem_init(pool[0],1,1,1);
	pool_length[0]=1;

	int prevmax=pool_from_table(pool, pool_length, len, table);		// pool filling
	if (prevmax+1==len) {
		fclose(table);
		for (i=0;i<len;++i) free(pool[i]);		// free
		printf("Table is longer than %d, no need to do anything\n\n",prevmax+1);
		exit(1);
	}
	table=freopen(filename,"a", table);		// append -> updating possible

	for (i=prevmax+1; i<len; ++i) {
		int j,k,h,l;
		const int eoc=(i+1)/ (1+1);      // end of combining
		const int j0 =(i+1)/(10+1);      // value ratio < 10
		int comb_tot=0;

		int grade_max=(int)(log2(i+1)+1);          // gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];              // get the temp pools for every grade
		int  temp_index[grade_max-1];              // index of work point in temp pools
		gem* subpools[grade_max-1];                // get subpools for every grade
		int  subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {            // init everything
			temp_pools[j] = (gem*)malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=NULL;                       // just to be able to free it
			subpools_length[j]=0;
		}

		for (j=j0; j<eoc; ++j) {         // combine gems and put them in temp array
			for (k=0; k< pool_length[j]; ++k) {
				int g1=(pool[j]+k)->grade;
				for (h=0; h< pool_length[i-1-j]; ++h) {
					int delta=g1 - (pool[i-1-j]+h)->grade;
					if (abs(delta)<=2) {        // grade difference <= 2
						comb_tot++;
						gem temp;
						gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
						int grd=temp.grade-2;
						temp_pools[grd][temp_index[grd]]=temp;
						temp_index[grd]++;
						if (temp_index[grd]==size) {							// let's skim a pool
							int length=size+subpools_length[grd];
							gem* temp_array = (gem*)malloc(length*sizeof(gem));
							int index=0;
							for (l=0; l<temp_index[grd]; ++l) {				// copy new gems
								temp_array[index]=temp_pools[grd][l];
								index++;
							}
							temp_index[grd]=0;				// temp index reset
							for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
								temp_array[index]=subpools[grd][l];
								index++;
							}
							free(subpools[grd]);			// free
							gem_sort(temp_array,length, gem_less_equal);	// work starts
	
							int broken=0;
							float lim_crit=-1;
							for (l=length-1;l>=0;--l) {
								if (temp_array[l].crit<=lim_crit) {
									temp_array[l].grade=0;
									broken++;
								}
								else lim_crit=temp_array[l].crit;
							}													// all unnecessary gems destroyed
	
							subpools_length[grd]=length-broken;
							subpools[grd] = (gem*)malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
	
							index=0;
							for (l=0; l<length; ++l) {      // copying to subpool
								if (temp_array[l].grade!=0) {
									subpools[grd][index]=temp_array[l];
									index++;
								}
							}
							free(temp_array);			// free
						}												// rebuilt subpool[grd], work restarts
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd) {					// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array = (gem*)malloc(length*sizeof(gem));
				int index=0;
				for (l=0; l<temp_index[grd]; ++l) {				// copy new gems
					temp_array[index]=temp_pools[grd][l];
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
					temp_array[index]=subpools[grd][l];
					index++;
				}
				free(subpools[grd]);		// free
				gem_sort(temp_array, length, gem_less_equal);		// work starts
				int broken=0;
				float lim_crit=-1;
				for (l=length-1;l>=0;--l) {
					if (temp_array[l].crit<=lim_crit) {
						temp_array[l].grade=0;
						broken++;
					}
					else lim_crit=temp_array[l].crit;
				}													// all unnecessary gems destroyed
				subpools_length[grd]=length-broken;
				subpools[grd] = (gem*)malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {			// copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}
				}
				free(temp_array);			// free
			}												// subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i] = (gem*)malloc(pool_length[i]*sizeof(gem));

		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {      // copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				pool[i][place]=subpools[grd][j];
				place++;
			}
		}
		for (grd=0;grd<grade_max-1;++grd) {     // free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}
		if (!output_options.quiet) {
			printf("Value:\t%d\n",i+1);
			if (output_options.debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);         // write on file
	}
	fclose(table);
	for (i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	options output_options = {};
	char filename[256]="";     // it should be enough

	while ((opt=getopt(argc,argv,"hdqf:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hdqf:");
				return 0;
			DQ_OPTIONS_BLOCK
			case 'f':
				strcpy(filename,optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind==argc) {
		printf("No length specified\n");
		return 1;
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
	}
	else {
		printf("Too many arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		printf("\n");
		return 1;
	}
	if (len<1) {
		printf("Improper gem number\n");
		return 1;
	}
	if (filename[0]=='\0') strcpy(filename, "table_crit");
	worker(len, output_options, filename);
	return 0;
}
