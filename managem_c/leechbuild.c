#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>


typedef struct Gem_O {
	int grade;			//using short does NOT improve time/memory usage
	double leech;		//using float does NOT improve time/memory usage
	struct Gem_O* father;
	struct Gem_O* mother;
} gem;


int int_max(int a, int b)
{
	if (a > b) return a;
	else return b;
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
	else p_gem_combined->leech = 0.88*p_gem2->leech + 0.5*p_gem1->leech;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)		//bigger is always gem1
{
	p_gem_combined->grade = p_gem1->grade;
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
	else p_gem_combined->leech = 0.89*p_gem2->leech + 0.44*p_gem1->leech;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	if (p_gem1->leech > p_gem2->leech) p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
	else p_gem_combined->leech = 0.9*p_gem2->leech + 0.38*p_gem1->leech;
}

void gem_combine (gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->father=p_gem1;
	p_gem_combined->mother=p_gem2;
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta){
		case 0:
			gem_comb_eq(p_gem1, p_gem2, p_gem_combined);
			break;
		case 1:
			gem_comb_d1(p_gem1, p_gem2, p_gem_combined);
			break;
		case -1:
			gem_comb_d1(p_gem2, p_gem1, p_gem_combined);
			break;
		default:
			gem_comb_gn(p_gem1, p_gem2, p_gem_combined);
			break;
	}
}

void gem_init(gem *p_gem, int grd, double leech)
{
	p_gem->grade=grd;
	p_gem->leech=leech;
	p_gem->father=NULL;
	p_gem->mother=NULL;
}

int gem_better(gem gem1, gem gem2)
{
	return gem1.leech>gem2.leech;
}

void worker(int len, int output_info, int output_quiet)
{
	int i;
	FILE* table;
	table=fopen("leechtable","rb");				// binary to check size
	if(table==NULL) {
		table=fopen("leechtable","w");			// creation
		fprintf(table, "1\n1 1.000000 -1 0 -1 0\n\n");		// printed g1
	}
	else {
		fseek(table, 0, SEEK_END);
		if (ftell(table)==0) {
			table=freopen("leechtable","w", table);					// init
			fprintf(table, "1\n1 1.000000 -1 0 -1 0\n\n");	// printed g1
		}
	}																			// we now have the file with at least g1
	table=freopen("leechtable","r", table);							// read

	gem* gems=malloc(len*sizeof(gem));		// if not malloc-ed 131k is the limit
	gem* pool[len];
	int pool_length[len];
	pool[0]=malloc(sizeof(gem));
	gem_init(gems,1,1);
	gem_init(pool[0],1,1);
	pool_length[0]=1;

	rewind(table);
	for (i=0;i<1+pool_length[0];++i) {						// discard value 0 gems
		fscanf(table, "%*[^\n]\n");
	}
	fscanf(table, "\n");													// discard newline
	int prevmax=0;
	for (i=1;i<len;++i) {
		int eof_check=fscanf(table, "%d\n", pool_length+i);				// get pool length
		if (eof_check==-1) break;
		else {
			pool[i]=malloc(pool_length[i]*sizeof(gem));
			int j;
			for (j=0; j<pool_length[i]; ++j) {
				int value_father, offset_father;
				int value_mother, offset_mother;
				fscanf(table, "%d %la %d %d %d\n", &pool[i][j].grade, &pool[i][j].leech, &value_father, &offset_father, &offset_mother);
				value_mother=i-1-value_father;
				pool[i][j].father=pool[value_father]+offset_father;
				pool[i][j].mother=pool[value_mother]+offset_mother;
			}
			fscanf(table, "\n");						// discard newline
			prevmax++;
		}
	}
	table=freopen("leechtable","a", table);				// append -> updating possible
	
	for (i=prevmax+1; i<len; ++i) {
		int j,k,h;
		int grade_max=(int)(log2(i+1)+1);		// gems with max grade cannot be destroyed, so this is a max, not a sup
		pool_length[i]=grade_max-1;
		pool[i]=malloc(pool_length[i]*sizeof(gem));
		for (j=0; j<pool_length[i]; ++j) gem_init(pool[i]+j,j+2,1);
		int eoc=(i+1)/2;				//end of combining
		int comb_tot=0;

		for (j=0;j<eoc;++j) {										// combine and put istantly in right pool
			if ((i-j)/(j+1) < 10) {								// value ratio < 10
				for (k=0; k< pool_length[j]; ++k) {
					for (h=0; h< pool_length[i-1-j]; ++h) {
						int delta=(pool[j]+k)->grade - (pool[i-1-j]+h)->grade;
						if (abs(delta)<=2) {								// grade difference <= 2
							comb_tot++;
							gem temp;
							gem_combine(pool[j]+k, pool[i-1-j]+h, &temp);
							int grd=temp.grade-2;
							if (gem_better(temp, pool[i][grd])) {
								pool[i][grd]=temp;
							}
						}
					}
				}
			}
		}
		if (!output_quiet) {
			printf("Value:\t%d\n",i+1);
			if (output_info) {
				printf("Raw:\t%d\n",comb_tot);
				printf("Pool:\t%d\n\n",pool_length[i]);
			}
		fflush(stdout);								// forces buffer write, so redirection works well
		}
		
		int broken=0;			// write on file
		if (pool[i][0].father==NULL) broken=1;				// solve false g2 problem
		fprintf(table, "%d\n", pool_length[i]-broken);
		for (j=broken;j<pool_length[i];++j) {
			fprintf(table, "%d %la", pool[i][j].grade, pool[i][j].leech);
			for (k=0; ; k++) {								// print parents
				int place=pool[i][j].father - pool[k];
				if (place < pool_length[k]) {
					fprintf(table, " %d %d", k, place);
					int mom_pool=i-1-k;
					place=pool[i][j].mother - pool[mom_pool];
					fprintf(table, " %d\n", place);
					break;
				}
			}
		}
		fprintf(table, "\n");
		fflush(table);
	}
	fclose(table);			// close
	for (i=0;i<len;++i) free(pool[i]);		// free
}

int main(int argc, char** argv)
{
	int len;
	char opt;
	int output_info=0;
	int output_quiet=0;

	while ((opt=getopt(argc,argv,"iq"))!=-1) {
		switch(opt) {
			case 'i':
				output_info = 1;
				break;
			case 'q':
				output_quiet = 1;
				break;
			case '?':
				return 1;
			default:
				break;
		}
	}
	if (optind+1==argc) {
		len = atoi(argv[optind]);
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
	else worker(len, output_info, output_quiet);
	return 0;
}

