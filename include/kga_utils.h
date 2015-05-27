#ifndef _KGA_UTILS_H
#define _KGA_UTILS_H

double gem_amp_power(gem gem1, gemY amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*gem1.bbound*(gem1.crit+crit_ratio*amp1.crit)*gem1.bbound;
}

void print_omnia_table(gem* gems, gemY* amps, double* powers, int len)
{
	printf("Killgem\tAmps\tPower\n");
	int i;
	for (i=0; i<len; i++)
		printf("%d\t%d\t%#.7g\n", i+1, gem_getvalue_Y(amps+i), powers[i]);
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

#define KGSPEC_COMPRESSION_APPR\
	for (i=0;i<len;++i) { \
		int j; \
		float maxcrit=0; \
		gem* temp_pool=malloc(pool_length[i]*sizeof(gem)); \
		for (j=0; j<pool_length[i]; ++j) { \
			temp_pool[j]=pool[i][j]; \
			maxcrit=max(maxcrit, (pool[i]+j)->crit); \
		} \
		gem_sort(temp_pool,pool_length[i]); \
		int broken=0; \
		int crit_cells=(int)(maxcrit*ACC)+1; \
		int tree_length= 1 << (int)ceil(log2(crit_cells)) ; \
		float* tree=malloc((tree_length+crit_cells+1)*sizeof(float)); \
		for (j=0; j<tree_length+crit_cells+1; ++j) tree[j]=-1; \
		int index; \
		for (j=pool_length[i]-1;j>=0;--j) { \
			gem* p_gem=temp_pool+j; \
			index=(int)(p_gem->crit*ACC); \
			if (ftree_check_after(tree, tree_length, index, p_gem->bbound)) { \
				ftree_add_element(tree, tree_length, index, p_gem->bbound); \
			} \
			else { \
				p_gem->grade=0; \
				broken++; \
			} \
		} \
		free(tree); \
		 \
		poolf_length[i]=pool_length[i]-broken; \
		poolf[i]=malloc(poolf_length[i]*sizeof(gem)); \
		index=0; \
		for (j=0; j<pool_length[i]; ++j) { \
			if (temp_pool[j].grade!=0) { \
				poolf[i][index]=temp_pool[j]; \
				index++; \
			} \
		} \
		free(temp_pool); \
		if (output_options & mask_debug) printf("Killgem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]); \
	}

#define AMPS_COMPRESSION\
	for (i=0; i<lena; ++i) { \
		int j; \
		gemY* temp_pool=malloc(poolY_length[i]*sizeof(gem)); \
		for (j=0; j<poolY_length[i]; ++j) { \
			temp_pool[j]=poolY[i][j]; \
		} \
		gem_sort_Y(temp_pool,poolY_length[i]); \
		int broken=0; \
		float lim_crit=-1; \
		for (j=poolY_length[i]-1;j>=0;--j) { \
			if (temp_pool[j].crit<=lim_crit) { \
				temp_pool[j].grade=0; \
				broken++; \
			} \
			else lim_crit=temp_pool[j].crit; \
		} \
		poolYf_length[i]=poolY_length[i]-broken; \
		poolYf[i]=malloc(poolYf_length[i]*sizeof(gemY)); \
		int index=0; \
		for (j=0; j<poolY_length[i]; ++j) { \
			if (temp_pool[j].grade!=0) { \
				poolYf[i][index]=temp_pool[j]; \
				index++; \
			} \
		} \
		free(temp_pool); \
		if (output_options & mask_debug) printf("Amp value %d compressed pool size:\t%d\n", i+1, poolYf_length[i]); \
	}

#define KGCOMB_COMPRESSION\
	{ \
		float maxcrit=0; \
		for (i=0; i<poolc_length[lenc-1]; ++i) { \
			maxcrit=max(maxcrit, (poolc[lenc-1]+i)->crit); \
		} \
		gem_sort(poolc[lenc-1],poolc_length[lenc-1]); \
		int broken=0; \
		int crit_cells=(int)(maxcrit*ACC)+1; \
		int tree_length= 1 << (int)ceil(log2(crit_cells)) ; \
		float* tree=malloc((tree_length+crit_cells+1)*sizeof(float)); \
		for (i=0; i<tree_length+crit_cells+1; ++i) tree[i]=-1; \
		int index; \
		for (i=poolc_length[lenc-1]-1;i>=0;--i) { \
			gem* p_gem=poolc[lenc-1]+i; \
			index=(int)(p_gem->crit*ACC); \
			if (ftree_check_after(tree, tree_length, index, p_gem->bbound)) { \
				ftree_add_element(tree, tree_length, index, p_gem->bbound); \
			} \
			else { \
				p_gem->grade=0; \
				broken++; \
			} \
		} \
		free(tree); \
		 \
		poolcf_length=poolc_length[lenc-1]-broken; \
		poolcf=malloc(poolcf_length*sizeof(gem)); \
		index=0; \
		for (i=0; i<poolc_length[lenc-1]; ++i) { \
			if (poolc[lenc-1][i].grade!=0) { \
				poolcf[index]=poolc[lenc-1][i]; \
				index++; \
			} \
		} \
	}

/* Exact macro blobs used for compressions in various files. Handle with more care */

typedef struct Gem_YBs {
	int   grade;
	float damage;
	float crit;
	float bbound;
	gem*  father;
	gem*  mother;
	int   place;
} gemP;

#include "kgexact_utils.h"

inline gemP gem2gemP(gem g)
{
	return (gemP){g.grade, g.damage, g.crit, g.bbound, g.father, g.mother, 0};
}

inline gem gemP2gem(gemP g)
{
	return (gem){g.grade, g.damage, g.crit, g.bbound, g.father, g.mother};
}

#define KGSPEC_COMPRESSION                                                                       \
	for (i=0;i<len;++i) {                                                                         \
		int length = pool_length[i];                                                               \
		gemP* temp_array=malloc(length*sizeof(gemP));                                              \
		for (int j=0; j<length; ++j) {                                                             \
			temp_array[j]=gem2gemP(pool[i][j]);                                                     \
		}                                                                                          \
		gem_sort_crit(temp_array,length);				/* work starts */                             \
		float lastcrit=-1;                                                                         \
		int tree_cell=0;                                                                           \
		for (int l=0; l<length; ++l) {                                                             \
			if (temp_array[l].crit == lastcrit) temp_array[l].place=tree_cell-1;                    \
			else {                                                                                  \
				temp_array[l].place=tree_cell++;                                                     \
				lastcrit = temp_array[l].crit;                                                       \
			}                                                                                       \
		}                                                                                          \
		gem_sort_exact(temp_array,length);                                                         \
		int broken=0;                                                                              \
		int tree_length= 1 << (int)ceil(log2(tree_cell));		/* this is pow(2, ceil()) bitwise */ \
		float* tree=malloc((tree_length*2)*sizeof(float));                                         \
		for (int l=0; l<tree_length*2; ++l) tree[l]=-1;			/* init also tree[0], it's faster */ \
		for (int l=length-1;l>=0;--l) {								/* start from large z */             \
			gemP* p_gem=temp_array+l;                                                               \
			if (ftree_check_after(tree, tree_length, p_gem->place, p_gem->bbound)) {                \
				ftree_add_element(tree, tree_length, p_gem->place, p_gem->bbound);                   \
			}                                                                                       \
			else {                                                                                  \
				p_gem->grade=0;                                                                      \
				broken++;                                                                            \
			}                                                                                       \
		}                                                                                          \
		free(tree);                                                                                \
		                                                                                           \
		poolf_length[i]=pool_length[i]-broken;                                                     \
		poolf[i]=malloc(poolf_length[i]*sizeof(gem));                                              \
		int index=0;                                                                               \
		for (int j=0; j<pool_length[i]; ++j) {                                                     \
			if (temp_array[j].grade!=0) {                                                           \
				poolf[i][index] = gemP2gem(temp_array[j]);                                           \
				index++;                                                                             \
			}                                                                                       \
		}                                                                                          \
		free(temp_array);                                                                          \
		if (output_options & mask_debug)                                                           \
			printf("Killgem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);    \
	}


#endif // _KGA_UTILS_H
