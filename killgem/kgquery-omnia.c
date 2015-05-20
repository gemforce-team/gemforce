#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include "interval_tree.h"
typedef struct Gem_YB gem;
const int ACC=80;					// ACC is for z-axis sorting and for the length of the interval tree
const int ACC_TR=750;			// ACC_TR is for bbound comparisons inside tree
#include "killgem_utils.h"
typedef struct Gem_Y gemY;
#include "crit_utils.h"
#include "kga_utils.h"
#include "gfon.h"

void worker(int len, int lenc, int output_options, char* filename, char* filenamec, char* filenameA, int TC, int As, int GT, int Namps)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);						// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1.000000,1,0);		// grade damage crit bbound
	gem_init(pool[0]+1,1,1.186168,0,1);		// BB has more dmg
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// killgem spec pool filling
	fclose(table);
	if (prevmax<len-1) {										// if the killgems are not enough
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		if (prevmax>0) printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	gem* poolf[len];
	int poolf_length[len];
	
	KGSPEC_COMPRESSION
	printf("Gem speccing pool compression done!\n");

	FILE* tablec=file_check(filenamec);		// file is open to read
	if (tablec==NULL) exit(1);					// if the file is not good we exit
	gem** poolc=malloc(lenc*sizeof(gem*));
	int* poolc_length=malloc(lenc*sizeof(int));
	poolc[0]=malloc(sizeof(gem));
	poolc_length[0]=1;
	gem_init(poolc[0],1,1,1,1);
	
	int prevmaxc=pool_from_table(poolc, poolc_length, lenc, tablec);		// killgem comb pool filling
	fclose(tablec);
	if (prevmaxc<lenc-1) {												// if the killgems are not enough
		for (i=0;i<=prevmaxc;++i) free(poolc[i]);		// free
		if (prevmaxc>0) printf("Gem table stops at %d, not %d\n",prevmaxc+1,lenc);
		exit(1);
	}

	gem* poolcf;
	int poolcf_length;
	
	KGCOMB_COMPRESSION
	printf("Killgem comb compressed pool size:\t%d\n",poolcf_length);

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena=len;									// see which is bigger between spec len and comb len
	if (lenc > len) lena=lenc;					// and we'll get the amp pool till there
	gemY** poolY=malloc(lena*sizeof(gemY*));
	int* poolY_length=malloc(lena*sizeof(int));
	poolY[0]=malloc(sizeof(gemY));
	poolY_length[0]=1;
	gem_init_Y(poolY[0],1,1,1);
	
	int prevmaxA=pool_from_table_Y(poolY, poolY_length, lena, tableA);		// amps pool filling
	fclose(tableA);
	if (prevmaxA<lena-1) {
		for (i=0;i<=prevmaxA;++i) free(poolY[i]);		// free
		if (prevmaxA>0) printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	gemY** poolYf=malloc(lena*sizeof(gemY*));		// if not malloc-ed 140k is the limit
	int poolYf_length[lena];

	AMPS_COMPRESSION
	gemY poolYc[poolYf_length[lenc-1]];
	int poolYc_length=poolYf_length[lenc-1];
	
	for (i=0; i<poolYf_length[lenc-1]; ++i) {		// amps fast access combining pool
		poolYc[i]=poolYf[lenc-1][i];
	}
	printf("Amp combining pool compression done!\n\n");

	int j,k,h,l,m;							// let's choose the right gem-amp combo
	gem gems[len];							// for every speccing value
	gemY amps[len];						// we'll choose the best amps
	gem gemsc[len];						// and the best NC combine
	gemY ampsc[len];						// for both
	double powers[len];
	gem_init(gems,1,1,1,0);
	gem_init_Y(amps,0,0,0);
	gem_init(gemsc,1,1,0,0);
	gem_init_Y(ampsc,0,0,0);
	powers[0]=0;
	double iloglenc=1/log(lenc);
	double crit_ratio  =Namps*(0.15+As/3*0.004)*2*(1+0.03*TC)/(1.0+TC/3*0.1);
	double damage_ratio=Namps*(0.20+As/3*0.004) * (1+0.03*TC)/(1.2+TC/3*0.1);
	double NT=pow(2, GT-1);
	if (!(output_options & mask_quiet)) {
		printf("Killgem spec\n");
		gem_print(gems);
		printf("Amplifier spec (x%d)\n", Namps);
		gem_print_Y(amps);
	}

	for (i=1;i<len;++i) {														// for every gem value
		gems[i]=(gem){0};															// we init the gems
		amps[i]=(gemY){0};														// to extremely weak ones
		gemsc[i]=(gem){0};
		ampsc[i]=(gemY){0};
																						// first we compare the gem alone
		for (l=0; l<poolcf_length; ++l) {									// first search in the NC gem comb pool
			if (gem_power(poolcf[l]) > gem_power(gemsc[i])) {
				gemsc[i]=poolcf[l];
			}
		}
		for (k=0;k<poolf_length[i];++k) {									// and then in the compressed gem pool
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int NS=i+1;
		double c0 = log(NT/(i+1))*iloglenc;									// last we compute the combination number
		powers[i] = pow(gem_power(gemsc[i]),c0) * gem_power(gems[i]);
																						// now we compare the whole setup
		for (j=0, NS+=Namps; j<i+1; ++j, NS+=Namps) {					// for every amp value from 1 to to gem_value
			double c = log(NT/NS)*iloglenc;									// we compute the combination number
			for (l=0; l<poolcf_length; ++l) {								// then we search in the NC gem comb pool
				double Cbg = pow(poolcf[l].bbound,c);
				double Cdg = pow(poolcf[l].damage,c);
				double Ccg = pow(poolcf[l].crit  ,c);
				for (m=0;m<poolYc_length;++m) {								// and in the amp NC pool
					double Cda = damage_ratio* pow(poolYc[m].damage,c);
					double Cca = crit_ratio  * pow(poolYc[m].crit  ,c);
					for (k=0;k<poolf_length[i];++k) {						// then in the gem pool
						if (poolf[i][k].crit!=0) {								// if the gem has crit we go on
							double Pb2 = Cbg * poolf[i][k].bbound * Cbg * poolf[i][k].bbound;
							double Pdg = Cdg * poolf[i][k].damage;
							double Pcg = Ccg * poolf[i][k].crit  ;
							for (h=0;h<poolYf_length[j];++h) {				// and in the reduced amp pool
								double Pdamage = Pdg + Cda * poolYf[j][h].damage ;
								double Pcrit   = Pcg + Cca * poolYf[j][h].crit   ;
								double power   = Pb2 * Pdamage * Pcrit ;
								if (power>powers[i]) {
									powers[i]=power;
									gems[i]=poolf[i][k];
									amps[i]=poolYf[j][h];
									gemsc[i]=poolcf[l];
									ampsc[i]=poolYc[m];
								}
							}
						}
					}
				}
			}
		}
		if (!(output_options & mask_quiet)) {
			printf("Killgem spec\n");
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_debug) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_Y(amps+i));
			if (output_options & mask_debug) printf("Pool:\t%d\n",poolYf_length[gem_getvalue_Y(amps+i)-1]);
			gem_print_Y(amps+i);
			printf("Killgem combine\n");
			printf("Comb:\t%d\n",lenc);
			if (output_options & mask_debug) printf("Pool:\t%d\n",poolcf_length);
			gem_print(gemsc+i);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			if (output_options & mask_debug) printf("Pool:\t%d\n",poolYc_length);
			gem_print_Y(ampsc+i);
			printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[i], amps[i], damage_ratio, crit_ratio));
			printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[i]);
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Killgem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n",gem_getvalue_Y(amps+len-1));
		gem_print_Y(amps+len-1);
		printf("Killgem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+len-1);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_Y(ampsc+len-1);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[len-1], amps[len-1], damage_ratio, crit_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[len-1]);
	}

	gem*  gemf = gems+len-1;  // gem  that will be displayed
	gemY* ampf = amps+len-1;  // amp  that will be displayed
	gem*  gemfc=gemsc+len-1;  // gemc that will be displayed
	gemY* ampfc=ampsc+len-1;  // ampc that will be displayed

	if (output_options & mask_upto) {
		double best_pow=0;
		int best_index=0;
		for (i=0; i<len; ++i) {
			if (powers[i] > best_pow) {
				best_index=i;
				best_pow=powers[i];
			}
		}
		printf("Best setup up to %d:\n\n", len);
		printf("Killgem spec\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue_Y(amps+best_index));
		gem_print_Y(amps+best_index);
		printf("Killgem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+best_index);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_Y(ampsc+best_index);
		printf("Spec base power:    \t%#.7g\n", gem_amp_power(gems[best_index], amps[best_index], damage_ratio, crit_ratio));
		printf("Global power at g%d:\t%#.7g\n\n\n", GT, powers[best_index]);
		gemf = gems+best_index;
		ampf = amps+best_index;
		gemfc = gemsc+best_index;
		ampfc = ampsc+best_index;
	}

	gem* gem_array;
	gem red;
	if (output_options & mask_red) {
		if (len < 3) printf("I could not add red!\n\n");
		else {
			int value=gem_getvalue(gemf);
			int valueA= gem_getvalue_Y(ampf);
			double NS = value + Namps*valueA;
			double c = log(NT/NS)*iloglenc;
			double ampd_resc_coeff = pow((ampfc->damage/gemfc->damage), c);
			double ampc_resc_coeff = pow((ampfc->crit/gemfc->crit), c);
			double amp_damage_scaled = damage_ratio * ampd_resc_coeff * ampf->damage;
			double amp_crit_scaled = crit_ratio * ampc_resc_coeff * ampf->crit;
			gemf = gem_putred(poolf[value-1], poolf_length[value-1], value, &red, &gem_array, amp_damage_scaled, amp_crit_scaled);
			printf("Setup with red added:\n\n");
			printf("Killgem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n", valueA);
			gem_print_Y(ampf);
			printf("Killgem combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print_Y(ampfc);
			if (output_options & mask_debug) printf("Damage rescaling coeff.: \t%f\n", ampd_resc_coeff);
			if (output_options & mask_debug) printf("Crit rescaling coeff.:   \t%f\n", ampc_resc_coeff);
			printf("Spec base power with red:\t%#.7g\n", gem_amp_power(*gemf, *ampf, damage_ratio, crit_ratio));
			double CgP = pow(gem_power(*gemfc), c);
			printf("Global power w. red at g%d:\t%#.7g\n\n\n", GT, CgP*gem_cfr_power(*gemf, amp_damage_scaled, amp_crit_scaled));
		}
	}

	if (output_options & mask_parens) {
		printf("Killgem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed_Y(ampf);
		printf("\n\n");
		printf("Killgem combining scheme:\n");
		print_parens_compressed(gemfc);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed_Y(ampfc);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Killgem speccing tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree_Y(ampf, "");
		printf("\n");
		printf("Killgem combining tree:\n");
		print_tree(gemfc, "");
		printf("\n");
		printf("Amplifier combining tree:\n");
		print_tree_Y(ampfc, "");
		printf("\n");
	}
	if (output_options & mask_table) print_omnia_table(gems, amps, powers, len);
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Killgem speccing equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations_Y(ampf);
		printf("\n");
		printf("Killgem combining equations:\n");
		print_equations(gemfc);
		printf("\n");
		printf("Amplifier combining equations:\n");
		print_equations_Y(ampfc);
		printf("\n");
	}
	
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (i=0;i<lenc;++i) free(poolc[i]);		// free gems
	free(poolc);
	free(poolc_length);
	free(poolcf);
	for (i=0;i<lena;++i) free(poolY[i]);		// free amps
	for (i=0;i<lena;++i) free(poolYf[i]);		// free amps compressed
	free(poolY);
	free(poolY_length);
	free(poolYf);
	if (output_options & mask_red && len > 2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	int lenc;
	char opt;
	int TC=120;
	int As=60;
	int GT=30;    // NT = pow(2, GT-1)
	int Namps=8;  // killgem in tower
	int output_options=0;
	char filename[256]="";		// it should be enough
	char filenamec[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"hptecdqurf:T:A:N:G:"))!=-1) {
		switch(opt) {
			case 'h':
				print_help("hptecdqurf:T:A:N:G:");
				return 0;
			PTECIDCUR_OPTIONS_BLOCK
			case 'f':			// can be "filename,filenamec,filenameA", if missing default is used
				;
				char* p=optarg;
				while (*p != ',' && *p != '\0') p++;
				if (*p==',') *p='\0';			// ok, it's "f,..."
				else p--;							// not ok, it's "f" -> empty string
				char* q=p+1;
				while (*q != ',' && *q != '\0') q++;
				if (*q==',') *q='\0';			// ok, it's "...,fc,fA"
				else q--;							// not ok, it's "...,fc" -> empty string
				strcpy(filename,optarg);
				strcpy(filenamec,p+1);
				strcpy(filenameA,q+1);
				break;
			TAN_OPTIONS_BLOCK
			case 'G':
				GT=atoi(optarg);
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
		lenc= 16;		// 16c as default combine
	}
	else if (optind+2==argc) {
		len = atoi(argv[optind]);
		lenc= atoi(argv[optind+1]);
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
	if (len<1 || lenc<1) {
		printf("Improper gem number\n");
		return 1;
	}
	file_selection(filename, "table_kgspec");
	file_selection(filenamec, "table_kgcomb");
	file_selection(filenameA, "table_crit");
	worker(len, lenc, output_options, filename, filenamec, filenameA, TC, As, GT, Namps);
	return 0;
}
