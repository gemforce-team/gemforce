#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
typedef struct Gem_OB gem;		// the strange order is so that managem_utils knows which gem type are we defining as "gem"
#include "managem_utils.h"


void worker(int len, int init_number, int output_parens, int output_equations, int output_tree, int output_table, int output_debug, int output_info, int size)
{
	printf("\n");
	int i;
	gem gems[len];
	gem* pool[len];
	int pool_length[len];
	
	if (init_number==1) {						// combine
		pool[0]=malloc(sizeof(gem));
		gem_init(pool[0],1,1,1);
		pool_length[0]=1;
		if (size==0) size=100;				// reasonable comb sizing
	}
	else {													// spec
		pool[0]=malloc(2*sizeof(gem));
		gem_init(pool[0]  ,1,1,0);
		gem_init(pool[0]+1,1,0,1);
		pool_length[0]=2;
		if (size==0) size=2000;				// reasonable spec sizing
	}
	gem_init(gems,1,1,0);
	gem_print(gems);

	for (i=1; i<len; ++i) {
		int j,k,h,l;
		int eoc=(i+1)/2;        //end of combining
		int comb_tot=0;

		int grade_max=(int)(log2(i+1)+1);						// gems with max grade cannot be destroyed, so this is a max, not a sup
		gem* temp_pools[grade_max-1];								// get the temp pools for every grade
		int  temp_index[grade_max-1];								// index of work point in temp pools
		gem* subpools[grade_max-1];									// get subpools for every grade
		int  subpools_length[grade_max-1];
		for (j=0; j<grade_max-1; ++j) {							// init everything
			temp_pools[j]=malloc(size*sizeof(gem));
			temp_index[j]=0;
			subpools[j]=malloc(size*sizeof(gem));
			subpools_length[j]=1;
			gem_init(subpools[j],j+2,0,0);
		}
		for (j=0;j<eoc;++j) {												// combine gems and put them in temp pools
			if ((i-j)/(j+1) < 10) {										// value ratio < 10
				for (k=0; k< pool_length[j]; ++k) {
					for (h=0; h< pool_length[i-1-j]; ++h) {
						int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gem temp;
							gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
							int grd=temp.grade-2;
							temp_pools[grd][temp_index[grd]]=temp;
							temp_index[grd]++;
							if (temp_index[grd]==size) {									// let's skim a pool
								int length=size+subpools_length[grd];
								gem* temp_array=malloc(length*sizeof(gem));
								int index=0;
								for (l=0; l<temp_index[grd]; ++l) {					// copy new gems
									temp_array[index]=temp_pools[grd][l];
									index++;
								}
								temp_index[grd]=0;				// temp index reset
								for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
									temp_array[index]=subpools[grd][l];
									index++;
								}
								free(subpools[grd]);			// free
								gem_sort(temp_array,length);								// work starts
		
								int broken=0;
								float lim_bbound=-1;
								for (l=length-1;l>=0;--l) {
									if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
										temp_array[l].grade=0;
										broken++;
									}
									else lim_bbound=temp_array[l].bbound;
								}													// all unnecessary gems destroyed
		
								subpools_length[grd]=length-broken;
								subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
		
								index=0;
								for (l=0; l<length; ++l) {			// copying to subpool
									if (temp_array[l].grade!=0) {
										subpools[grd][index]=temp_array[l];
										index++;
									}
								}
								free(temp_array);     // free
							}												// rebuilt subpool[grd], work restarts
						}
					}
				}
			}
		}
		int grd;
		for (grd=0; grd<grade_max-1; ++grd)  {									// let's put remaining gems on
			if (temp_index[grd] != 0) {
				int length=temp_index[grd]+subpools_length[grd];
				gem* temp_array=malloc(length*sizeof(gem));
				int index=0;
				for (l=0; l<temp_index[grd]; ++l) {									// copy new gems
					temp_array[index]=temp_pools[grd][l];
					index++;
				}
				for (l=0; l<subpools_length[grd]; ++l) {		// copy old gems
					temp_array[index]=subpools[grd][l];
					index++;
				}
				free(subpools[grd]);		// free
				gem_sort(temp_array,length);								// work starts
				int broken=0;
				float lim_bbound=-1;
				for (l=length-1;l>=0;--l) {
					if ((int)(ACC*temp_array[l].bbound)<=(int)(ACC*lim_bbound)) {
						temp_array[l].grade=0;
						broken++;
					}
					else lim_bbound=temp_array[l].bbound;
				}													// all unnecessary gems destroyed
				subpools_length[grd]=length-broken;
				subpools[grd]=malloc(subpools_length[grd]*sizeof(gem));		// pool init via broken
				index=0;
				for (l=0; l<length; ++l) {      // copying to subpool
					if (temp_array[l].grade!=0) {
						subpools[grd][index]=temp_array[l];
						index++;
					}
				}
				free(temp_array);     // free
			}												// subpool[grd] is now full
		}
		pool_length[i]=0;
		for (grd=0; grd<grade_max-1; ++grd) pool_length[i]+=subpools_length[grd];
		pool[i]=malloc(pool_length[i]*sizeof(gem));

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
		gems[i]=pool[i][0];						// choosing gem (criteria moved to more_power def)
		for (j=1;j<pool_length[i];++j) if (gem_more_powerful(pool[i][j],gems[i])) {
			gems[i]=pool[i][j];
		}

		printf("Value:\t%d\n",i+1);
		if (output_info) {
			printf("Raw:\t%d\n",comb_tot);
			printf("Pool:\t%d\n",pool_length[i]);
		}
		gem_print(gems+i);
		fflush(stdout);								// forces buffer write, so redirection works well
	}

	if (output_parens) {
		printf("Combining scheme:\n");
		print_parens(gems+len-1);
		printf("\n\n");
		printf("Compressed combining scheme:\n");
		print_parens_compressed(gems+len-1);
		printf("\n\n");
	}
	if (output_tree) {
		printf("Gem tree:\n");
		print_tree(gems+len-1, "");
		printf("\n");
	}
	if (output_table) print_table(gems, len);

	if (output_debug) {
		printf("Dumping whole pool of value %d:\n\n",len);
		for (i=0;i<pool_length[len-1];++i) {
			gem_print(pool[len-1]+i);
			print_parens(pool[len-1]+i);
			printf("\n\n");
		}
	}
	if (output_equations) {		// it ruins gems, must be last
		printf("Equations:\n");
		print_equations(gems+len-1);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int init_number=2;		// speccing by default
	int output_parens=0;
	int output_equations=0;
	int output_tree=0;
	int output_table=0;
	int output_debug=0;
	int output_info=0;
	int size=0;						// worker or user must initialize it
	
	while ((opt=getopt(argc,argv,"ptecdis:"))!=-1) {
		switch(opt) {
			case 'p':
				output_parens = 1;
				break;
			case 't':
				output_tree = 1;
				break;
			case 'e':
				output_equations = 1;
				break;
			case 'c':
				output_table = 1;
				break;
			case 'd':
				output_debug = 1;
				output_info = 1;
				break;
			case 'i':
				output_info = 1;
				break;
			case 's':
				size = atoi(optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
		char* p=argv[optind];
		while (*p != '\0') p++;
		if (*(p-1)=='c') init_number=1;
	}
	else {
		printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1) printf("Improper gem number\n");
	else worker(len, init_number, output_parens, output_equations, output_tree, output_table, output_debug, output_info, size);
	return 0;
}

