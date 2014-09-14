#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

typedef struct Gem {
	int grade;
	double leech;
	//double bbound;
} gem;

int int_max(int a, int b) 
{
	if (a > b) return a;
	else return b;
}

void swap (int *p1, int *p2)
{
	int t=*p1;
	*p1=*p2;
	*p2=t;
}

int fact(int n)
{
	if (n<2) return 1;
	int i;
	int fact=1;
	for (i=2;i<=n;++i) fact=fact*i;
	return fact;
}

void gem_print(gem *p_gem) {
	printf("Grade: %d\nLeech: %f\n", p_gem->grade, p_gem->leech);
}

void gem_leech_order(gem *p_gem1, gem *p_gem2)
{
	if (p_gem1->leech < p_gem2->leech) {
	gem gem_temp=*p_gem1;
		*p_gem1=*p_gem2;
		*p_gem2=gem_temp;
	}
}

void gem_comb_eq(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = p_gem1->grade+1;
	gem_leech_order(p_gem1, p_gem2);
	p_gem_combined->leech = 0.88*p_gem1->leech + 0.5*p_gem2->leech;
}

void gem_comb_d1(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	gem_leech_order(p_gem1, p_gem2);
	p_gem_combined->leech = 0.89*p_gem1->leech + 0.44*p_gem2->leech;
}

void gem_comb_gn(gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	p_gem_combined->grade = int_max(p_gem1->grade, p_gem2->grade);
	gem_leech_order(p_gem1, p_gem2);
	p_gem_combined->leech = 0.9*p_gem1->leech + 0.38*p_gem2->leech;
}

void gem_combine (gem *p_gem1, gem *p_gem2, gem *p_gem_combined)
{
	int delta = p_gem1->grade - p_gem2->grade;
	switch (delta){
		case 0:
			gem_comb_eq(p_gem1, p_gem2, p_gem_combined);
			break;
		case -1:
		case 1:
			gem_comb_d1(p_gem1, p_gem2, p_gem_combined);
			break;
		default: 
			gem_comb_gn(p_gem1, p_gem2, p_gem_combined);
			break;
	}
}

void gem_init(gem *p_gem, int grd)
{
	p_gem->grade=grd;
	p_gem->leech= pow(1.38, (double)(grd-1));
}

void order_parser(char* result, int *order, int len)
{
	int ord[len-1];
	char parsed[len][4*len];
	int i;
	for (i=0;i<len-1;++i) ord[i]=order[i];
	for (i=0;i<len;++i)	strcpy(parsed[i], "g");
	char *sum=malloc(sizeof(char[4*len]));
	for (i=0;i<len-1;++i) {
		int num=ord[i];
		char* open_par;
		char* close_par;
		switch (i%3) {
			case 0:
				open_par="(";
				close_par=")";
			break;
			case 1:
				open_par="[";
				close_par="]";
			break;
			case 2:
				open_par="{";
				close_par="}";
			break;
		}
		if (i!=len-2) strcpy(sum, open_par);
		else strcpy(sum, "");
		strcat(sum, parsed[num]);
		strcat(sum, "+");
		strcat(sum, parsed[num+1]);
		if (i!=len-2) strcat(sum, close_par);
		strcpy(parsed[num],sum);
		int j;
		for (j=num+1;j<len-1;++j) strcpy(parsed[j],parsed[j+1]);
		for (j=i+1;j<len-1;++j) if (ord[j]>num) ord[j]--;
	}
	strcpy(parsed[0],sum);
	free(sum);
	strcpy(result, parsed[0]);
} 

void gem_announce(gem* p_gem, int* order, int len)
{
	gem_print(p_gem);
	int i;
	for (i=0;i<len-1;++i) printf("%d ", order[i]);
	char*result=malloc(sizeof(char[4*len]));
	order_parser(result, order,len);
	printf("\n%s",result);
	printf("\n\%dg method.  New cutoff! %f\n",len,p_gem->leech);
	free(result);
}

void reverse (int *order, int len)
{
	if (len==0||len==1) return;
	else {
		swap (order, order+len-1);
		reverse(order+1,len-2);
	}
}

int anti_ord(int* order, int len)
{
	if (len==1) return 1;
	if (order[0]>order[1]) return anti_ord(order+1, len-1);
	else return 0;
}

int find_minover(int* order, int len, int target)
{
	int res=-1;
	if (len>0) {
		int i;
		for (i=0;i<len;++i) if (order[i]>target) res=i;
	}
	return res;
}

int order_seq(int* order, int len)
{
	if (anti_ord(order+1,len-1))
	{
		int minover = find_minover(order,len,*order);
		if (minover==-1) return 1;
		else swap (order,order + minover);
		reverse (order+1,len-1);
		return 0;
	}
	else return order_seq(order+1,len-1);
}

void order_random(int* order, int len)
{
	int i;
	for (i=0;i<len;++i) {
		int rnd=rand()%len;
		int temp=order[rnd];
		order[rnd]=order[i];
		order[i]=temp;
	}
}

void array_reduce(gem *gems, int *order, int len, int comb, int index)
{
	int i;
	for (i=comb+1;i<len-1;++i) gems[i]=gems[i+1];
	for (i=index+1;i<len-1;++i) if (order[i]>comb) order[i]--;
}

void worker(int len, int attempts, int mode, double cut_coeff)
{
	int i;
	int order_orig[len-1];	
	//int order_orig[15]={0,1,2,3,4,5,7,6,9,8,11,13,14,12,10};		//psorek16: 3.902277
	//int order_orig[15]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};		//ieee[len] testing ground: 
	for (i=0;i<len-1;++i) order_orig[i]=i;
	gem *p_gem=malloc(sizeof(gem));
	gem gems[len];
	double cutoff=1;
	double cutoff_best=1;
	int order_temp[len-1];
	int counter=0;
	i=0;
	int j=0;
	switch (mode) {
		case 0:											// random while
		while (i==0) {										
			order_random(order_orig,len-1);
			for (j=0;j<len-1;++j) order_temp[j]=order_orig[j];
			for (j=0;j<len;++j) gem_init(gems+j,1);
			for (j=0;j<len-1;j++) {						//combination cycle			
				int comb=order_temp[j];
				gem_combine(gems+comb,gems+comb+1,p_gem);
				gems[comb]=*p_gem;
				array_reduce(gems, order_temp, len, comb, j);
			}
			if (gems[0].leech>cutoff+0.000001) {
				gem_announce(gems, order_orig, len);
				printf("\n");
				cutoff=gems[0].leech;
			}
			counter++;
			if (counter%2000000==0) {					//2 million
				printf("%d attempts\n\n",counter);
				if (counter/10000000==attempts) i=1;	//10 million
			}
		}
		break;
		
		case 1: ;										// fixed point while
		int randomize=1;
		int comb_point=0;
		int order_points[2];
		int wait=0;
		while (i==0) {
			if (randomize==1) {
				order_random(order_orig,len-1);
				for (j=0;j<len-1;++j) order_temp[j]=order_orig[j];
			}
			else {
				for (j=0;j<len-1;++j) order_temp[j]=order_orig[j];
				for (j=0;j<len-1;++j) {
					if (order_temp[j]==comb_point) order_points[0]=j;
					if (order_temp[j]==comb_point+1) order_points[1]=j;
				}
				swap(order_temp+order_points[0],order_temp+order_points[1]);
			}										
			for (j=0;j<len;++j) gem_init(gems+j,1);
			for (j=0;j<len-1;j++) {						//combination cycle			
				int comb=order_temp[j];
				gem_combine(gems+comb,gems+comb+1,p_gem);
				gems[comb]=*p_gem;
				array_reduce(gems, order_temp, len, comb, j);
			}
			if (gems[0].leech>cutoff+0.000001) {
				gem_announce(gems, order_orig, len);
				cutoff=gems[0].leech;
				if (cutoff>cutoff_best) cutoff_best=cutoff;
				printf("%dg method. Best cutoff: %.6lf\n\n",len,cutoff_best);
				if (randomize==0) {
					comb_point++;
					swap(order_orig+order_points[0],order_orig+order_points[1]);
				}
				else randomize=0;
				wait=0;
			}
			else if (randomize==0 && comb_point<len-2) {
				comb_point++;
				wait=0;
			}
			else {
				randomize = 1;
				comb_point=0;
				if (wait==2000) {
					printf("!!! Best cutoff was %.6lf\n",cutoff_best);
					printf("!!! Cutoff was %.6lf, reduced to %.6lf\n\n", cutoff, (double)cut_coeff*cutoff);
					cutoff=cut_coeff*cutoff;
					wait=0;
				}
			}
			counter++;
			wait++;
			if (counter%2000000==0) {					//2 million
				printf("%d attempts\n\n",counter);
				if (counter/10000000==attempts) i=1;	//10 million
			}
		}
		break;
		
		case 2: ;										// sequential while
		unsigned long long int n_comb=fact(len-1);
		while (i==0) {									
			for (j=0;j<len-1;++j) order_temp[j]=order_orig[j];
			for (j=0;j<len;++j) gem_init(gems+j,1);			
			for (j=0;j<len-1;j++) {						//combination cycle			
				int comb=order_temp[j];
				gem_combine(gems+comb,gems+comb+1,p_gem);
				gems[comb]=*p_gem;
				array_reduce(gems, order_temp, len, comb, j);
			}
			if (gems[0].leech>cutoff+0.000001) {
				gem_announce(gems, order_orig, len);
				printf("\n");
				cutoff=gems[0].leech;
			}
			i=order_seq(order_orig,len-1);
			counter++;
			if (counter%2000000==0) {					//2 million
				printf("%d/%I64u attempts (%.2f%%)\n\n",counter,n_comb,(float)100*counter/n_comb);
				if (counter/10000000==attempts) i=1;	//10 million
			}
		}
		break;
	}		
	free(p_gem);
}

int main(int argc, char **argv)
{
	int len=2;
	int attempts=-1;
	int mode=2;
	double cut_coeff=1;
	char opt;
	while ((opt=getopt(argc,argv,"m:c:a:"))!=-1) {
			switch(opt) {
				case 'm':
					mode=atoi(optarg);
					break;
				case 'c':
					cut_coeff=atof(optarg);
					break;
				case 'a':
					attempts=atoi(optarg);
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
	srand(time(NULL));
	worker(len,attempts,mode,cut_coeff);
	return 0;
}
