#ifndef _KGA_UTILS_H
#define _KGA_UTILS_H

#include <cstdio>

#include "gem_sort.h"

inline double gem_amp_power(gem_YB gem1, gem_Y amp1, double damage_ratio, double crit_ratio)
{
	return (gem1.damage+damage_ratio*amp1.damage)*gem1.bbound*(gem1.crit+crit_ratio*amp1.crit)*gem1.bbound;
}

void print_omnia_table(gem_Y* amps, double* powers, int len)
{
	printf("Killgem\tAmps\tPower\n");
	for (int i=0; i<len; i++)
		printf("%d\t%d\t%#.7g\n", i+1, gem_getvalue(amps+i), powers[i]);
	printf("\n");
}

/* Macro blob that is used for some flag options. Handle with care */
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

/* Macro blob used for amps compressions in various files. Handle with care */

#define AMPS_COMPRESSION																\
	for (i=0; i<lena; ++i) {															\
		int j;																			\
		gemY* temp_pool = (gemY*)malloc(poolY_length[i]*sizeof(gemY));					\
		for (j=0; j<poolY_length[i]; ++j) {												\
			temp_pool[j]=poolY[i][j];													\
		}																				\
		gem_sort(temp_pool, poolY_length[i], AS_LAMBDA(gem_2D_less));					\
		int broken=0;																	\
		float lim_crit=-1;																\
		for (j=poolY_length[i]-1;j>=0;--j) {											\
			if (temp_pool[j].crit<=lim_crit) {											\
				temp_pool[j].grade=0;													\
				broken++;																\
			}																			\
			else lim_crit=temp_pool[j].crit;											\
		}																				\
		poolYf_length[i]=poolY_length[i]-broken;										\
		poolYf[i] = (gemY*)malloc(poolYf_length[i]*sizeof(gemY));						\
		int index=0;																	\
		for (j=0; j<poolY_length[i]; ++j) {												\
			if (temp_pool[j].grade!=0) {												\
				poolYf[i][index]=temp_pool[j];											\
				index++;																\
			}																			\
		}																				\
		free(temp_pool);																\
		if (output_options.debug)														\
			printf("Amp value %d compressed pool size:\t%d\n", i+1, poolYf_length[i]);	\
	}

/* Exact macro blobs used for compressions in various files. Handle with more care */

struct gemP {
	int     grade;
	float   damage;
	float   crit;
	float   bbound;
	gem_YB* father;
	gem_YB* mother;
	int     place;
};

inline gemP gem2gemP(gem_YB g)
{
	return {g.grade, g.damage, g.crit, g.bbound, g.father, g.mother, 0};
}

inline gem_YB gemP2gem(gemP g)
{
	return {g.grade, g.damage, g.crit, g.bbound, g.father, g.mother};
}

inline double gem_bbd(gemP gem1)
{
	return gem1.bbound*gem1.bbound*gem1.damage;
}

inline double gem_bbc(gemP gem1)
{
	return gem1.bbound*gem1.bbound*gem1.crit;
}

#define KGSPEC_COMPRESSION																				\
	for (i=0;i<len;++i) {																				\
		int length = pool_length[i];																	\
		gemP* temp_array = (gemP*)malloc(length*sizeof(gemP));											\
		for (int j=0; j<length; ++j) {																	\
			temp_array[j]=gem2gemP(pool[i][j]);															\
		}																								\
		gem_sort(temp_array, length, gem_less_crit<gemP>);		/* work starts */						\
		float lastcrit=-1;																				\
		int tree_cell=0;																				\
		for (int l=0; l<length; ++l) {																	\
			if (temp_array[l].crit == lastcrit) temp_array[l].place=tree_cell-1;						\
			else {																						\
				temp_array[l].place=tree_cell++;														\
				lastcrit = temp_array[l].crit;															\
			}																							\
		}																								\
		gem_sort(temp_array, length, gem_less_exact<gemP>);												\
		int broken=0;																					\
		int tree_length= 1 << (int)ceil(log2(tree_cell));		/* this is pow(2, ceil()) bitwise */	\
		float* tree = (float*)malloc((tree_length*2)*sizeof(float));	/* delete gems with bb=0 */		\
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* init also tree[0], it's faster */	\
		for (int l=length-1;l>=0;--l) {							/* start from large dmg */				\
			gemP* p_gem=temp_array+l;																	\
			if (tree_check_after(tree, tree_length, p_gem->place, p_gem->bbound)) {						\
				tree_add_element(tree, tree_length, p_gem->place, p_gem->bbound);						\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		free(tree);																						\
		double* dtree = (double*)malloc((tree_length*2)*sizeof(double));								\
		for (int l=0; l<tree_length*2; ++l) dtree[l]=0;			/* delete gems with power=0 */			\
		for (int l=0; l<length; ++l) {							/* start from low dmg = high idmg */	\
			gemP* p_gem=temp_array+l;																	\
			if (p_gem->grade==0) continue;																\
			int place = tree_length -1 - p_gem->place;			/* reverse crit order */				\
			if (tree_check_after(dtree, tree_length, place, gem_power(*p_gem))) {						\
				tree_add_element(dtree, tree_length, place, gem_power(*p_gem));							\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		for (int l=0; l<tree_length*2; ++l) dtree[l]=0;			/* bbd - id - c compression */			\
		for (int l=0; l<length; ++l) {							/* start from low dmg = high idmg */	\
			gemP* p_gem=temp_array+l;																	\
			if (p_gem->grade==0) continue;																\
			int place = p_gem->place;							/* regular crit order */				\
			if (tree_check_after(dtree, tree_length, place, gem_bbd(*p_gem))) {							\
				tree_add_element(dtree, tree_length, place, gem_bbd(*p_gem));							\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		for (int l=0; l<tree_length*2; ++l) dtree[l]=0;			/* bbc - d - ic compression */			\
		for (int l=length-1; l>=0; --l) {						/* start from large dmg */				\
			gemP* p_gem=temp_array+l;																	\
			if (p_gem->grade==0) continue;																\
			int place = tree_length -1 - p_gem->place;			/* reverse crit order */				\
			if (tree_check_after(dtree, tree_length, place, gem_bbc(*p_gem))) {							\
				tree_add_element(dtree, tree_length, place, gem_bbc(*p_gem));							\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		free(dtree);																					\
		poolf_length[i]=length-broken;																	\
		poolf[i] = (gem*)malloc(poolf_length[i]*sizeof(gem));											\
		int index=0;																					\
		for (int j=0; j<length ; ++j) {																	\
			if (temp_array[j].grade!=0) {																\
				poolf[i][index] = gemP2gem(temp_array[j]);												\
				index++;																				\
			}																							\
		}																								\
		free(temp_array);																				\
		if (output_options.debug)																		\
			printf("Killgem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);		\
	}

#define KGCOMB_COMPRESSION																				\
	{																									\
		int length = poolc_length[lenc-1];																\
		gemP* temp_array = (gemP*)malloc(length*sizeof(gemP));											\
		for (int j=0; j<length; ++j) {																	\
			temp_array[j]=gem2gemP(poolc[lenc-1][j]);													\
		}																								\
		gem_sort(temp_array, length, gem_less_crit<gemP>);		/* work starts */						\
		float lastcrit=-1;																				\
		int tree_cell=0;																				\
		for (int l=0; l<length; ++l) {																	\
			if (temp_array[l].crit == lastcrit) temp_array[l].place=tree_cell-1;						\
			else {																						\
				temp_array[l].place=tree_cell++;														\
				lastcrit = temp_array[l].crit;															\
			}																							\
		}																								\
		gem_sort(temp_array, length, gem_less_exact<gemP>);												\
		int broken=0;																					\
		int tree_length= 1 << (int)ceil(log2(tree_cell));		/* this is pow(2, ceil()) bitwise */	\
		float* tree = (float*)malloc((tree_length*2)*sizeof(float));									\
		for (int l=0; l<tree_length*2; ++l) tree[l]=0;			/* combines have no gem with bb=0 */	\
		for (int l=length-1;l>=0;--l) {							/* start from large dmg */				\
			gemP* p_gem=temp_array+l;																	\
			if (tree_check_after(tree, tree_length, p_gem->place, p_gem->bbound)) {						\
				tree_add_element(tree, tree_length, p_gem->place, p_gem->bbound);						\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		free(tree);																						\
		double* dtree = (double*)malloc((tree_length*2)*sizeof(double));								\
		for (int l=0; l<tree_length*2; ++l) dtree[l]=0;			/* delete gems with power=0 */			\
		for (int l=0; l<length; ++l) {							/* start from low dmg = high idmg */	\
			gemP* p_gem=temp_array+l;																	\
			if (p_gem->grade==0) continue;																\
			int place = tree_length -1 - p_gem->place;			/* reverse crit order */				\
			if (tree_check_after(dtree, tree_length, place, gem_power(*p_gem))) {						\
				tree_add_element(dtree, tree_length, place, gem_power(*p_gem));							\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		for (int l=0; l<tree_length*2; ++l) dtree[l]=0;			/* bbd - id - c compression */			\
		for (int l=0; l<length; ++l) {							/* start from low dmg = high idmg */	\
			gemP* p_gem=temp_array+l;																	\
			if (p_gem->grade==0) continue;																\
			int place = p_gem->place;							/* regular crit order */				\
			if (tree_check_after(dtree, tree_length, place, gem_bbd(*p_gem))) {							\
				tree_add_element(dtree, tree_length, place, gem_bbd(*p_gem));							\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		for (int l=0; l<tree_length*2; ++l) dtree[l]=0;			/* bbc - d - ic compression */			\
		for (int l=length-1; l>=0; --l) {						/* start from large dmg */				\
			gemP* p_gem=temp_array+l;																	\
			if (p_gem->grade==0) continue;																\
			int place = tree_length -1 - p_gem->place;			/* reverse crit order */				\
			if (tree_check_after(dtree, tree_length, place, gem_bbc(*p_gem))) {							\
				tree_add_element(dtree, tree_length, place, gem_bbc(*p_gem));							\
			}																							\
			else {																						\
				p_gem->grade=0;																			\
				broken++;																				\
			}																							\
		}																								\
		free(dtree);																					\
		poolcf_length=length-broken;																	\
		poolcf = (gem*)malloc(poolcf_length*sizeof(gem));												\
		int index=0;																					\
		for (int j=0; j<length; ++j) {																	\
			if (temp_array[j].grade!=0) {																\
				poolcf[index] = gemP2gem(temp_array[j]);												\
				index++;																				\
			}																							\
		}																								\
		free(temp_array);																				\
	}


#endif // _KGA_UTILS_H
