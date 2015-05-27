#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
#include "killgem_utils.h"
#include "gfon.h"

void worker(int len, int output_options, int pool_zero, char* filename)
{
	FILE* table=table_init(filename, pool_zero);		// init killgem
	int i;
	int size;
	const int ACC_TR=750;				//   750  ACC_TR is for bbound comparisons inside tree
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(pool_zero*sizeof(gem));
	pool_length[0]=pool_zero;

	if (pool_zero==1) {					// combine
		ACC=80;								// ACC is for z-axis sorting and for the length of the interval tree
		gem_init(pool[0],1,1,1,1);		// start gem does not matter
		size=1000;							// reasonable comb sizing
	}
	else {									// spec
		ACC=60;								// ACC is for z-axis sorting and for the length of the interval tree
		gem_init(pool[0]  ,1,1.000000,1,0);
		gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
		size=20000;							// reasonable spec sizing
	}
	
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
			temp_pools[j]=malloc(size*sizeof(gem));
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
							gem* temp_array=malloc(length*sizeof(gem));
							int index=0;
							float maxcrit=0;				// this will help me create the minimum tree
							for (l=0; l<size; ++l) {					// copy new gems
								temp_array[index]=temp_pools[grd][l];
								maxcrit=max(maxcrit, (temp_array+index)->crit);
								index++;
							}
							temp_index[grd]=0;			// temp index reset
							for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
								temp_array[index]=subpools[grd][l];
								maxcrit=max(maxcrit, (temp_array+index)->crit);
								index++;
							}
							free(subpools[grd]);		// free
							
							gem_sort(temp_array,length);					// work starts
							int broken=0;
							int crit_cells=(int)(maxcrit*ACC)+1;		// this pool will be big from the beginning, but we avoid binary search
							int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
							int* tree=malloc((tree_length+crit_cells+1)*sizeof(int));		// memory improvement, 2* is not needed
							for (l=0; l<tree_length+crit_cells+1; ++l) tree[l]=-1;			// init also tree[0], it's faster
							for (l=length-1;l>=0;--l) {												// start from large z
								gem* p_gem=temp_array+l;
								int index=(int)(p_gem->crit*ACC);									// find its place in x
								if (tree_check_after(tree, tree_length, index, (int)(p_gem->bbound*ACC_TR))) {		// look at y
									tree_add_element(tree, tree_length, index, (int)(p_gem->bbound*ACC_TR));
								}
								else {
									p_gem->grade=0;
									broken++;
								}
							}												// all unnecessary gems destroyed
							free(tree);									// free
							
							subpools_length[grd]=length-broken;
							subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
							
							index=0;
							for (l=0; l<length; ++l) {			// copying to subpool
								if (temp_array[l].grade!=0) {
									subpools[grd][index]=temp_array[l];
									index++;
								}   
							}
							free(temp_array);			// free
						}									// rebuilt subpool[grd], work restarts
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd) {						// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array=malloc(length*sizeof(gem));
				int index=0;
				float maxcrit=0;				// this will help me create the minimum tree
				for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
					temp_array[index]=temp_pools[grd][l];
					maxcrit=max(maxcrit, (temp_array+index)->crit);
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
					temp_array[index]=subpools[grd][l];
					maxcrit=max(maxcrit, (temp_array+index)->crit);
					index++;
				}
				free(subpools[grd]);		// free
				
				gem_sort(temp_array,length);						// work starts
				int broken=0;
				int crit_cells=(int)(maxcrit*ACC)+1;		// this pool will be big from the beginning, but we avoid binary search
				int tree_length= 1 << (int)ceil(log2(crit_cells)) ;				// this is pow(2, ceil()) bitwise for speed improvement
				int* tree=malloc((tree_length+crit_cells+1)*sizeof(int));		// memory improvement, 2* is not needed
				for (l=0; l<tree_length+crit_cells+1; ++l) tree[l]=-1;			// init also tree[0], it's faster
				for (l=length-1;l>=0;--l) {												// start from large z
					gem* p_gem=temp_array+l;
					int index=(int)(p_gem->crit*ACC);									// find its place in x
					if (tree_check_after(tree, tree_length, index, (int)(p_gem->bbound*ACC_TR))) {		// look at y
						tree_add_element(tree, tree_length, index, (int)(p_gem->bbound*ACC_TR));
					}
					else {
						p_gem->grade=0;
						broken++;
					}
				}											// all unnecessary gems destroyed
				free(tree);								// free
			
				subpools_length[grd]=length-broken;
				subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {			// copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}   
				}
				free(temp_array);			// free
			}									// subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i]=malloc(pool_length[i]*sizeof(gem));
		
		int place=0;
		for (grd=0;grd<grade_max-1;++grd) {			// copying to pool
			for (j=0; j<subpools_length[grd]; ++j) {
				pool[i][place]=subpools[grd][j];
				place++;
			}   
		}
		for (grd=0;grd<grade_max-1;++grd) {			// free
			free(temp_pools[grd]);
			free(subpools[grd]);
		}

		if (!(output_options & mask_quiet)) {
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_debug) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		}
		table_write_iteration(pool, pool_length, i, table);			// write on file
	}
	
	fclose(table);			// close
	for (i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int pool_zero=2;			// speccing by default
	int output_options=0;
	char filename[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"hdqf:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hdqf:");
				return 0;
			PTECIDCUR_OPTIONS_BLOCK
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
		char* p=argv[optind];
		while (*p != '\0') p++;
		if (*(p-1)=='c') pool_zero=1;
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
	if (filename[0]=='\0') {
		if (pool_zero==2) strcpy(filename, "table_kgsappr");
		else strcpy(filename, "table_kgcappr");
	}
	worker(len, output_options, pool_zero, filename);
	return 0;
}

