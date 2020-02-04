#ifndef _INTERVAL_TREE_H
#define _INTERVAL_TREE_H

#include <algorithm>

/* place is always 0..N-1 */

template<class T>
void tree_add_element(T* tree, int N, int place, T val)
{
	place+=N;
	while (place!=1) {
		if (val > tree[place]) {
			tree[place]=val;
			place >>= 1;		// place/=2 bitwise, it's a bit faster
		}
		else return;			// no need to go on, they'll sure be even bigger
	}
	if (val > tree[1]) tree[1]=val;
}

template<class T>
bool tree_check_after(T* tree, int N, int place, T val)
{
	place+=N;
	if (val <= tree[place]) return false;
	while (place!=1) {
		if (place%2==0 && val <= tree[place+1]) return false;
		place >>= 1;			// place/=2 bitwise, it's a bit faster
	}
	return true;				// if we never found bigger return true
}

template<class T>
T tree_read_max(T* tree, int N, int place)
{
	place+=N;
	T result = tree[place];
	while (place!=1) {
		if (place%2==0) result=std::max(result,tree[place+1]);
		place >>= 1;			// place/=2 bitwise, it's a bit faster
	}
	return result;
}

template<class T>
bool tree_check_after_debug(T* tree, int N, int place, T val)
{
	printf("\nTree state (baselength %d):\n", N);
	for (int i=1; i<2*N; ++i) {
		printf("%+.4f ", tree[i]);
		if (!(i & (i+1))) printf("\n");
	}
	printf("Trying bb %f at place %d+%d\n", val, N, place);
	place+=N;
	if (val <= tree[place]) {
		printf("Failed base check\n");
		return false;
	}
	while (place!=1) {
		if (place%2==0 && val <= tree[place+1]) {
			printf ("Failed check against place %d\n", place);
			return false;
		}
		place >>= 1;			// place/=2 bitwise, it's a bit faster
	}
	printf("Success, adding\n");
	return true;				// if we never found bigger return true
}

#endif // _INTERVAL_TREE_H
