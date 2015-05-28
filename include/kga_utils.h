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

/* Macro blob used for amps compressions in various files. Handle with care */

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
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* delete gems with bb=0 */          \
		for (int l=length-1;l>=0;--l) {								/* start from large z */             \
			gemP* p_gem=temp_array+l;                                                               \
			if (p_gem->crit!=0											/* delete gems with crit=0 */        \
			&&  ftree_check_after(tree, tree_length, p_gem->place, p_gem->bbound)) {                \
				ftree_add_element(tree, tree_length, p_gem->place, p_gem->bbound);                   \
			}                                                                                       \
			else {                                                                                  \
				p_gem->grade=0;                                                                      \
				broken++;                                                                            \
			}                                                                                       \
		}                                                                                          \
		free(tree);                                                                                \
		                                                                                           \
		poolf_length[i]=length-broken;                                                             \
		poolf[i]=malloc(poolf_length[i]*sizeof(gem));                                              \
		int index=0;                                                                               \
		for (int j=0; j<length ; ++j) {                                                            \
			if (temp_array[j].grade!=0) {                                                           \
				poolf[i][index] = gemP2gem(temp_array[j]);                                           \
				index++;                                                                             \
			}                                                                                       \
		}                                                                                          \
		free(temp_array);                                                                          \
		if (output_options & mask_debug)                                                           \
			printf("Killgem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);    \
	}

#define KGCOMB_COMPRESSION                                                                       \
	{                                                                                             \
		int length = poolc_length[lenc-1];                                                         \
		gemP* temp_array=malloc(length*sizeof(gemP));                                              \
		for (int j=0; j<length; ++j) {                                                             \
			temp_array[j]=gem2gemP(poolc[lenc-1][j]);                                               \
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
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* combines have no gem with bb=0 */ \
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
		poolcf_length=length-broken;                                                               \
		poolcf=malloc(poolcf_length*sizeof(gem));                                                  \
		int index=0;                                                                               \
		for (int j=0; j<length; ++j) {                                                             \
			if (temp_array[j].grade!=0) {                                                           \
				poolcf[index] = gemP2gem(temp_array[j]);                                             \
				index++;                                                                             \
			}                                                                                       \
		}                                                                                          \
		free(temp_array);                                                                          \
	}


#endif // _KGA_UTILS_H
