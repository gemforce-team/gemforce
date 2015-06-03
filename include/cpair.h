#ifndef _CPAIR_H
#define _CPAIR_H

typedef struct Cpair {
	double power;
	double rdmg;
	double rcrit;
	gem*   combg;
	gemY*  comba;
	int    place;
} cpair;

inline int cpair_less_xyz(cpair cpair1, cpair cpair2)
{
	if (cpair1.rdmg != cpair2.rdmg)
		return cpair1.rdmg<cpair2.rdmg;
	if (cpair1.power != cpair2.power)
		return cpair1.power<cpair2.power;
	return cpair1.rcrit<cpair2.rcrit;
}

void ins_sort_xyz (cpair* cpairs, int len)
{
	int i,j;
	cpair element;
	for (i=1; i<len; i++) {
		element=cpairs[i];
		for (j=i; j>0 && cpair_less_xyz(element, cpairs[j-1]); j--) {
			cpairs[j]=cpairs[j-1];
		}
		cpairs[j]=element;
	}
}

void quick_sort_xyz (cpair* cpairs, int len)
{
	if (len > 10)  {
		cpair pivot = cpairs[len/2];
		cpair* beg = cpairs;
		cpair* end = cpairs+len-1;
		while (beg <= end) {
			while (cpair_less_xyz(*beg, pivot)) {
				beg++;
			}
			while (cpair_less_xyz(pivot,*end)) {
				end--;
			}
			if (beg <= end) {
				cpair temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		if (end-cpairs+1 < cpairs-beg+len) {		// sort smaller first
			quick_sort_xyz(cpairs, end-cpairs+1);
			quick_sort_xyz(beg, cpairs-beg+len);
		}
		else {
			quick_sort_xyz(beg, cpairs-beg+len);
			quick_sort_xyz(cpairs, end-cpairs+1);
		}
	}
}

void cpair_sort_xyz (cpair* cpairs, int len)
{
	quick_sort_xyz(cpairs, len);    // partially sort
	ins_sort_xyz(cpairs, len);      // finish the nearly sorted array
}

inline int cpair_less_rcrit(cpair cpair1, cpair cpair2)
{
	return cpair1.rcrit<cpair2.rcrit;
}

void ins_sort_rcrit (cpair* cpairs, int len)
{
	int i,j;
	cpair element;
	for (i=1; i<len; i++) {
		element=cpairs[i];
		for (j=i; j>0 && cpair_less_rcrit(element, cpairs[j-1]); j--) {
			cpairs[j]=cpairs[j-1];
		}
		cpairs[j]=element;
	}
}

void quick_sort_rcrit (cpair* cpairs, int len)
{
	if (len > 10)  {
		cpair pivot = cpairs[len/2];
		cpair* beg = cpairs;
		cpair* end = cpairs+len-1;
		while (beg <= end) {
			while (cpair_less_rcrit(*beg, pivot)) {
				beg++;
			}
			while (cpair_less_rcrit(pivot,*end)) {
				end--;
			}
			if (beg <= end) {
				cpair temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		if (end-cpairs+1 < cpairs-beg+len) {		// sort smaller first
			quick_sort_rcrit(cpairs, end-cpairs+1);
			quick_sort_rcrit(beg, cpairs-beg+len);
		}
		else {
			quick_sort_rcrit(beg, cpairs-beg+len);
			quick_sort_rcrit(cpairs, end-cpairs+1);
		}
	}
}

void cpair_sort_rcrit (cpair* cpairs, int len)
{
	quick_sort_rcrit (cpairs, len);    // partially sort
	ins_sort_rcrit (cpairs, len);      // finish the nearly sorted array
}


#endif // _CPAIR_H
