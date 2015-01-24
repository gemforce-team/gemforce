#ifndef _WMGA_UTILS_H
#define _WMGA_UTILS_H

double gem_amp_power(gem gem1, gemO amp1, double leech_ratio)
{
	return pb_power(gem1.pbound)*(gem1.leech+leech_ratio*amp1.leech);
}

void print_omnia_table(gem* gems, gemO* amps, double* powers, int len)
{
	printf("Managem\tAmps\tPower\n");
	int i;
	for (i=0; i<len; i++)
		printf("%d\t%d\t%#.7g\n", i+1, gem_getvalue_O(amps+i), powers[i]);
	printf("\n");
}

/* Macro blob that is used for some flag options. Handle with care */
#define TAN_OPTIONS_BLOCK\
			case 'T': \
				TC=atoi(optarg); \
				break; \
			case 'A': \
				As=atoi(optarg); \
				break; \
			case 'N': \
				Namps=atoi(optarg); \
				break;

/* Macro blobs used for compressions in various files. Handle with care */

#define MGSPEC_COMPRESSION                                                                        \
	for (i=0;i<len;++i) {                                                                          \
		int j;                                                                                      \
		gem* temp_pool=malloc(pool_length[i]*sizeof(gem));                                          \
		for (j=0; j<pool_length[i]; ++j) {                                                          \
			temp_pool[j]=pool[i][j];                                                                 \
		}                                                                                           \
		gem_sort_exact(temp_pool,pool_length[i]);                                                   \
		int broken=0;                                                                               \
		float lim_pbound=0;							/* delete gems with bb=0 */                          \
		for (j=pool_length[i]-1;j>=0;--j) {                                                         \
			if (temp_pool[j].pbound<=lim_pbound) {                                                   \
				temp_pool[j].grade=0;                                                                 \
				broken++;                                                                             \
			}                                                                                        \
			else lim_pbound=temp_pool[j].pbound;                                                     \
		}                                                                                           \
		double lim_power=0;							/* power-ileech compression */                       \
		for (j=0; j<pool_length[i]; ++j) {		/* leech min->max equals ileech max->min */          \
			if (temp_pool[j].grade!=0) {			/* gems with leech=0 have power=0 and are broken */  \
				if (gem_power(temp_pool[j])<=lim_power) {                                             \
					temp_pool[j].grade=0;                                                              \
					broken++;                                                                          \
				}                                                                                     \
				else lim_power=gem_power(temp_pool[j]);                                               \
			}                                                                                        \
		}                                                                                           \
		poolf_length[i]=pool_length[i]-broken;                                                      \
		poolf[i]=malloc(poolf_length[i]*sizeof(gem));		/* pool init via broken */               \
		int index=0;                                                                                \
		for (j=0; j<pool_length[i]; ++j) {						/* copying to subpool */                 \
			if (temp_pool[j].grade!=0) {                                                             \
				poolf[i][index]=temp_pool[j];                                                         \
				index++;                                                                              \
			}                                                                                        \
		}                                                                                           \
		free(temp_pool);                                                                            \
		if (output_options & mask_debug)                                                            \
			printf("Managem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);     \
	}

#define MGCOMB_COMPRESSION                                                                        \
	{                                                                                              \
		gem_sort_exact(poolc[lenc-1],poolc_length[lenc-1]);                                         \
		int broken=0;                                                                               \
		float lim_pbound=0;				/* combines have no gem with bb=0 */                          \
		for (i=poolc_length[lenc-1]-1;i>=0;--i) {                                                   \
			if (poolc[lenc-1][i].pbound<=lim_pbound) {                                               \
				poolc[lenc-1][i].grade=0;                                                             \
				broken++;                                                                             \
			}                                                                                        \
			else lim_pbound=poolc[lenc-1][i].pbound;                                                 \
		}                                                                                           \
		double lim_power=0;									/* power-ileech compression */                 \
		for (i=0; i<poolc_length[lenc-1]; ++i) {		/* leech min->max equals ileech max->min */    \
			if (poolc[lenc-1][i].grade!=0) {                                                         \
				if (gem_power(poolc[lenc-1][i])<=lim_power) {                                         \
					poolc[lenc-1][i].grade=0;                                                          \
					broken++;                                                                          \
				}                                                                                     \
				else lim_power=gem_power(poolc[lenc-1][i]);                                           \
			}                                                                                        \
		}                                                                                           \
		poolcf_length=poolc_length[lenc-1]-broken;                                                  \
		poolcf=malloc(poolcf_length*sizeof(gem));		/* pool init via broken */                     \
		int index=0;                                                                                \
		for (i=0; i<poolc_length[lenc-1]; ++i) {		/* copying to subpool */                       \
			if (poolc[lenc-1][i].grade!=0) {                                                         \
				poolcf[index]=poolc[lenc-1][i];                                                       \
				index++;                                                                              \
			}                                                                                        \
		}                                                                                           \
	}

#define AMPS_COMPRESSION                            \
	for (i=0; i<lena; ++i) {                         \
		bestO[i]=(gemO){0};                           \
		for (int j=0; j<poolO_length[i]; ++j) {       \
			if (gem_better(poolO[i][j], bestO[i])) {   \
				bestO[i]=poolO[i][j];                   \
			}                                          \
		}                                             \
	}


#endif // _WMGA_UTILS_H
