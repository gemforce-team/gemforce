#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
typedef struct Gem_OB gem;		// the strange order is so that managem_utils knows which gem type are we defining as "gem"
#include "managem_utils.h"
typedef struct Gem_O gemO;
#include "leech_utils.h"
#include "gfon.h"
const int NT=1048576;			// 2^20 ~ 1m, it's still low, but there's no difference going on (even 10k gives the same results)

double gem_amp_power(gem gem1, gemO amp1, double leech_ratio)
{
	return gem1.bbound*(gem1.leech+leech_ratio*amp1.leech);
}

void print_omnia_table(gem* gems, gemO* amps, double* powers, int len)
{
	printf("Managem\tAmps\tPower (resc. 1k)\n");			// we'll rescale again for 1k, no need to have 10 digits
	int i;
	for (i=0;i<len;i++) printf("%d\t%d\t%.6f\n", i+1, gem_getvalue_O(amps+i), powers[i]/1000);
	printf("\n");
}

void worker(int len, int lenc, int output_options, char* filename, char* filenamec, char* filenameA, int TC, int Namps)
{
	FILE* table=file_check(filename);			// file is open to read
	if (table==NULL) exit(1);						// if the file is not good we exit
	int i;
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(2*sizeof(gem));
	pool_length[0]=2;
	gem_init(pool[0]  ,1,1,0);
	gem_init(pool[0]+1,1,0,1);
	
	int prevmax=pool_from_table(pool, pool_length, len, table);		// managem spec pool filling
	if (prevmax<len-1) {										// if the managems are not enough
		fclose(table);
		for (i=0;i<=prevmax;++i) free(pool[i]);		// free
		printf("Gem table stops at %d, not %d\n",prevmax+1,len);
		exit(1);
	}

	gem* poolf[len];
	int poolf_length[len];
	
	for (i=0;i<len;++i) {								// managem spec compression
		int j;
		gem* temp_pool=malloc(pool_length[i]*sizeof(gem));
		for (j=0; j<pool_length[i]; ++j) {			// copy gems
			temp_pool[j]=pool[i][j];
		}
		gem_sort(temp_pool,pool_length[i]);			// work starts
		int broken=0;
		float lim_bbound=-1;
		for (j=pool_length[i]-1;j>=0;--j) {
			if ((int)(ACC*temp_pool[j].bbound)<=(int)(ACC*lim_bbound)) {
				temp_pool[j].grade=0;
				broken++;
			}
			else lim_bbound=temp_pool[j].bbound;
		}														// unnecessary gems broken
		gem best=(gem){0};							// choosing best i-spec
		for (j=0;j<pool_length[i];++j)
		if (gem_more_powerful(temp_pool[j], best)) {
			best=temp_pool[j];
		}
		for (j=0;j<pool_length[i];++j)							// comparison compression (only for mg):
		if (temp_pool[j].bbound < best.bbound
		&&  temp_pool[j].grade!=0)									// a mg makes sense only if
		{																	// its bbound is bigger than
			temp_pool[j].grade=0;									// the bbound of the best one
			broken++;
		}																	// all the unnecessary gems broked
		poolf_length[i]=pool_length[i]-broken;
		poolf[i]=malloc(poolf_length[i]*sizeof(gem));		// pool init via broken
		int index=0;
		for (j=0; j<pool_length[i]; ++j) {						// copying to subpool
			if (temp_pool[j].grade!=0) {
				poolf[i][index]=temp_pool[j];
				index++;
			}
		}
		free(temp_pool);
		if (output_options & mask_info) printf("Managem value %d speccing compressed pool size:\t%d\n",i+1,poolf_length[i]);
	}
	printf("Gem speccing pool compression done!\n");

	FILE* tablec=file_check(filenamec);		// file is open to read
	if (tablec==NULL) exit(1);					// if the file is not good we exit
	gem** poolc=malloc(lenc*sizeof(gem*));
	int* poolc_length=malloc(lenc*sizeof(int));
	poolc[0]=malloc(sizeof(gem));
	poolc_length[0]=1;
	gem_init(poolc[0],1,1,1);
	
	int prevmaxc=pool_from_table(poolc, poolc_length, lenc, tablec);		// managem comb pool filling
	if (prevmaxc<lenc-1) {												// if the managems are not enough
		fclose(tablec);
		for (i=0;i<=prevmaxc;++i) free(poolc[i]);		// free
		printf("Gem table stops at %d, not %d\n",prevmaxc+1,lenc);
		exit(1);
	}

	gem* poolcf;
	int poolcf_length;
	
	{															// managem pool compression
		gem_sort(poolc[lenc-1],poolc_length[lenc-1]);
		int broken=0;
		float lim_bbound=-1;
		for (i=poolc_length[lenc-1]-1;i>=0;--i) {
			if ((int)(ACC*poolc[lenc-1][i].bbound)<=(int)(ACC*lim_bbound)) {
				poolc[lenc-1][i].grade=0;
				broken++;
			}
			else lim_bbound=poolc[lenc-1][i].bbound;
		}													// unnecessary gems destroyed
		
		gem best=(gem){0};				// choosing best combine
		for (i=0;i<poolc_length[lenc-1];++i)
		if (gem_more_powerful(poolc[lenc-1][i], best)) {
			best=poolc[lenc-1][i];
		}
		for (i=0;i<poolc_length[lenc-1];++i)			// comparison compression (only for mg):
		if (poolc[lenc-1][i].bbound < best.bbound
		&&  poolc[lenc-1][i].grade!=0)					// a mg makes sense only if
		{															// its bbound is bigger than
			poolc[lenc-1][i].grade=0;						// the bbound of the best one
			broken++;
		}															// all the unnecessary gems destroyed
		poolcf_length=poolc_length[lenc-1]-broken;
		poolcf=malloc(poolcf_length*sizeof(gem));		// pool init via broken
		int index=0;
		for (i=0; i<poolc_length[lenc-1]; ++i) {		// copying to subpool
			if (poolc[lenc-1][i].grade!=0) {
				poolcf[index]=poolc[lenc-1][i];
				index++;
			}
		}
	}
	printf("Managem comb compressed pool size:\t%d\n",poolcf_length);

	FILE* tableA=file_check(filenameA);		// fileA is open to read
	if (tableA==NULL) exit(1);					// if the file is not good we exit
	int lena;
	if (lenc > len) lena=lenc;					// see which is bigger between spec len and comb len
	else lena=len;									// and we'll get the amp pool till there
	gemO** poolO=malloc(lena*sizeof(gemO*));
	int poolO_length[lena];
	poolO[0]=malloc(sizeof(gemO));
	poolO_length[0]=1;
	gem_init_O(poolO[0],1,1);
	
	int prevmaxA=pool_from_table_O(poolO, poolO_length, lena, tableA);		// amps pool filling
	if (prevmaxA<lena-1) {
		fclose(tableA);
		for (i=0;i<=prevmaxA;++i) free(poolO[i]);		// free
		printf("Amp table stops at %d, not %d\n",prevmaxA+1,lena);
		exit(1);
	}

	gemO* bestO=malloc(lena*sizeof(gem));		// if not malloc-ed 140k is the limit
	
	for (i=0; i<lena; ++i) {			// amps pool compression
		int j;
		bestO[i]=(gemO){0};
		for (j=0; j<poolO_length[i]; ++j) {
			if (gem_better(poolO[i][j], bestO[i])) {
				bestO[i]=poolO[i][j];
			}
		}
	}
	gemO combO=bestO[lenc-1];			// amps fast access combine
	printf("Amp pool compression done!\n\n");

	int j,k,l;									// let's choose the right gem-amp combo
	gem gems[len];								// for every speccing value
	gemO amps[len];							// we'll choose the best amps
	gem gemsc[len];							// and the best NC combine
	gemO ampsc[len];							// for both
	double powers[len];
	gem_init(gems,1,1,0);
	gem_init_O(amps,0,0);
	gem_init(gemsc,1,0,0);
	gem_init_O(ampsc,0,0);
	powers[0]=0;
	double iloglenc=1/log(lenc);
	double leech_ratio=Namps*0.46*(1+(double)TC*3/100)/(1+(double)TC/30);
	if (!(output_options & mask_quiet)) {
		printf("Managem spec\n");
		gem_print(gems);
		printf("Amplifier spec (x%d)\n", Namps);
		gem_print_O(amps);
	}

	for (i=1;i<len;++i) {													// for every gem value
		gems[i]=(gem){0};														// we init the gems
		amps[i]=(gemO){0};													// to extremely weak ones
		gemsc[i]=(gem){0};
		ampsc[i]=combO;														// <- this is ok only for mg
																					// first we compare the gem alone
		for (l=0; l<poolcf_length; ++l) {								// first search in the NC gem comb pool
			if (gem_power(poolcf[l]) > gem_power(gemsc[i])) {
				gemsc[i]=poolcf[l];
			}
		}
		for (k=0;k<poolf_length[i];++k) {								// and then in the the gem pool
			if (gem_power(poolf[i][k]) > gem_power(gems[i])) {
				gems[i]=poolf[i][k];
			}
		}
		int NS=i+1;
		double c0 = log((double)NT/(i+1))*iloglenc;					// last we compute the combination number
		powers[i] = pow(gem_power(gemsc[i]),c0) * gem_power(gems[i]);
																					// now we compare the whole setup
		for (j=0, NS+=Namps; j<i+1; ++j, NS+=Namps) {				// for every amp value from 1 up to gem_value
			double c = log((double)NT/NS)*iloglenc;					// we compute the combination number
			double Ca= leech_ratio * pow(combO.leech,c);				// <- this is ok only for mg
			double Pa= Ca * bestO[j].leech;								// <- because we already know the best amps
			for (l=0; l<poolcf_length; ++l) {							// then we search in NC gem comb pool
				double Cbg = pow(poolcf[l].bbound,c);
				double Cg  = pow(gem_power(poolcf[l]),c);
				for (k=0;k<poolf_length[i];++k) {						// and in the reduced gem pool
					if (poolf[i][k].leech!=0) {							// if the gem has leech we go on
						double Palone = Cg * gem_power(poolf[i][k]);
						double Pbg = Cbg * poolf[i][k].bbound;
						double power = Palone + Pbg * Pa;  
						if (power>powers[i]) {
							powers[i]=power;
							gems[i]=poolf[i][k];
							amps[i]=bestO[j];
							gemsc[i]=poolcf[l];
						}
					}
				}
			}
		}
		if (!(output_options & mask_quiet)) {
			printf("Managem spec\n");
			printf("Value:\t%d\n",i+1);
			if (output_options & mask_info) printf("Pool:\t%d\n",poolf_length[i]);
			gem_print(gems+i);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_O(amps+i));
			if (output_options & mask_info) printf("Pool:\t1\n");
			gem_print_O(amps+i);
			printf("Managem combine\n");
			printf("Comb:\t%d\n",lenc);
			if (output_options & mask_info) printf("Pool:\t%d\n",poolcf_length);
			gem_print(gemsc+i);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			if (output_options & mask_info) printf("Pool:\t1\n");
			gem_print_O(ampsc+i);
			printf("Spec base power (resc.):\t%f\n", gem_amp_power(gems[i], amps[i], leech_ratio));
			printf("Global power (resc. 1k):\t%f\n\n\n", powers[i]/1000);
		}
	}
	
	if (output_options & mask_quiet) {		// outputs last if we never seen any
		printf("Managem spec\n");
		printf("Value:\t%d\n",len);
		gem_print(gems+len-1);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n",gem_getvalue_O(amps+len-1));
		gem_print_O(amps+len-1);
		printf("Managem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+len-1);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_O(ampsc+len-1);
		printf("Spec base power (resc.):\t%f\n", gem_amp_power(gems[len-1], amps[len-1], leech_ratio));
		printf("Global power (resc. 1k):\t%f\n\n\n", powers[len-1]/1000);
	}

	gem*  gemf = gems+len-1;  // gem  that will be displayed
	gemO* ampf = amps+len-1;  // amp  that will be displayed
	gem*  gemfc=gemsc+len-1;  // gemc that will be displayed
	gemO* ampfc=ampsc+len-1;  // ampc that will be displayed

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
		printf("Managem spec\n");
		printf("Value:\t%d\n", gem_getvalue(gems+best_index));
		gem_print(gems+best_index);
		printf("Amplifier spec (x%d)\n", Namps);
		printf("Value:\t%d\n", gem_getvalue_O(amps+best_index));
		gem_print_O(amps+best_index);
		printf("Managem combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print(gemsc+best_index);
		printf("Amplifier combine\n");
		printf("Comb:\t%d\n",lenc);
		gem_print_O(ampsc+best_index);
		printf("Spec base power (resc.):\t%f\n", gem_amp_power(gems[best_index], amps[best_index], leech_ratio));
		printf("Global power (resc. 1k):\t%f\n\n\n", powers[best_index]/1000);
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
			gemf = gem_putred(poolf[value-1], poolf_length[value-1], value, &red, &gem_array, ampf->leech, leech_ratio);
			printf("Setup with red added:\n\n");
			printf("Managem spec\n");
			printf("Value:\t%d\n", value);		// made to work well with -u
			gem_print(gemf);
			printf("Amplifier spec (x%d)\n", Namps);
			printf("Value:\t%d\n",gem_getvalue_O(ampf));
			gem_print_O(ampf);
			printf("Managem combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print(gemfc);
			printf("Amplifier combine\n");
			printf("Comb:\t%d\n",lenc);
			gem_print_O(ampfc);
			printf("Spec base power with red:\t%f\n\n\n", gem_amp_power(*gemf, *ampf, leech_ratio));
		}
	}

	if (output_options & mask_parens) {
		printf("Managem speccing scheme:\n");
		print_parens_compressed(gemf);
		printf("\n\n");
		printf("Amplifier speccing scheme:\n");
		print_parens_compressed_O(ampf);
		printf("\n\n");
		printf("Managem combining scheme:\n");
		print_parens_compressed(gemfc);
		printf("\n\n");
		printf("Amplifier combining scheme:\n");
		print_parens_compressed_O(ampfc);
		printf("\n\n");
	}
	if (output_options & mask_tree) {
		printf("Managem speccing tree:\n");
		print_tree(gemf, "");
		printf("\n");
		printf("Amplifier speccing tree:\n");
		print_tree_O(ampf, "");
		printf("\n");
		printf("Managem combining tree:\n");
		print_tree(gemfc, "");
		printf("\n");
		printf("Amplifier combining tree:\n");
		print_tree_O(ampfc, "");
		printf("\n");
	}
	if (output_options & mask_table) print_omnia_table(gems, amps, powers, len);
	
	if (output_options & mask_equations) {		// it ruins gems, must be last
		printf("Managem speccing equations:\n");
		print_equations(gemf);
		printf("\n");
		printf("Amplifier speccing equations:\n");
		print_equations_O(ampf);
		printf("\n");
		printf("Managem combining equations:\n");
		print_equations(gemfc);
		printf("\n");
		printf("Amplifier combining equations:\n");
		print_equations_O(ampfc);
		printf("\n");
	}
	
	fclose(table);
	fclose(tablec);
	fclose(tableA);
	for (i=0;i<len;++i) free(pool[i]);			// free gems
	for (i=0;i<len;++i) free(poolf[i]);			// free gems compressed
	for (i=0;i<lenc;++i) free(poolc[i]);		// free gems
	free(poolc);
	free(poolc_length);
	free(poolcf);
	for (i=0;i<lena;++i) free(poolO[i]);		// free amps
	free(poolO);
	free(bestO);										// free amps compressed
	if (output_options & mask_red && len > 2) {
		free(gem_array);
	}
}

int main(int argc, char** argv)
{
	int len;
	int lenc;
	char opt;
	int TC=60;
	int Namps=6;
	int output_options=0;
	char filename[256]="";		// it should be enough
	char filenamec[256]="";		// it should be enough
	char filenameA[256]="";		// it should be enough

	while ((opt=getopt(argc,argv,"iptcequrf:T:N:"))!=-1) {
		switch(opt) {
			case 'i':
				output_options |= mask_info;
				break;
			case 'p':
				output_options |= mask_parens;
				break;
			case 't':
				output_options |= mask_tree;
				break;
			case 'c':
				output_options |= mask_table;
				break;
			case 'e':
				output_options |= mask_equations;
				break;
			case 'q':
				output_options |= mask_quiet;
				break;
			case 'u':
				output_options |= mask_upto;
				break;
			case 'r':
				output_options |= mask_red;
				break;
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
			case 'T':
				TC=atoi(optarg);
				break;
			case 'N':
				Namps=atoi(optarg);
				break;
			case '?':
				return 1;
			default:
				break;
		}
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
		if (optind==argc) printf("No length specified\n");
		else printf("Unknown arguments:\n");
		while (argv[optind]!=NULL) {
			printf("%s ", argv[optind]);
			optind++;
		}
		return 1;
	}
	if (len<1 || lenc<1) {
		printf("Improper gem number\n");
		return 1;
	}
	if (filename[0]=='\0') strcpy(filename, "table_mgspec");
	if (filenamec[0]=='\0') strcpy(filenamec, "table_mgcomb");
	if (filenameA[0]=='\0') strcpy(filenameA, "table_leech");
	worker(len, lenc, output_options, filename, filenamec, filenameA, TC, Namps);
	return 0;
}
